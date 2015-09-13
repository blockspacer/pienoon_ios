// Copyright 2014 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "precompiled.h"

#include "utilities.h"

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include <cstdlib>
#include <fstream>
#include <stdexcept>

#include <SDL.h>

namespace fpl {

bool LoadFile(const char* filename, std::string* dest) {
  auto handle = SDL_RWFromFile(filename, "rb");
  if (!handle) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "LoadFile fail on %s", filename);
    return false;
  }
  auto len = static_cast<size_t>(SDL_RWseek(handle, 0, RW_SEEK_END));
  SDL_RWseek(handle, 0, RW_SEEK_SET);
  dest->assign(len + 1, 0);
  size_t rlen = static_cast<size_t>(SDL_RWread(handle, &(*dest)[0], 1, len));
  SDL_RWclose(handle);
  return len == rlen && len > 0;
}

#if defined(_WIN32)
inline char* getcwd(char* buffer, int maxlen) {
  return _getcwd(buffer, maxlen);
}

inline int chdir(const char* dirname) { return _chdir(dirname); }
#endif  // defined(_WIN32)

// Search up the directory tree from binary_dir for target_dir, changing the
// working directory to the target_dir and returning true if it's found,
// false otherwise.
bool ChangeToUpstreamDir(const char* const binary_dir,
                         const char* const target_dir) {
#if !defined(__ANDROID__)
  {
    std::string current_dir = binary_dir;

    // Search up the tree from the directory containing the binary searching
    // for target_dir.
    for (;;) {
      size_t separator = current_dir.find_last_of(flatbuffers::kPathSeparator);
      if (separator == std::string::npos) break;
      current_dir = current_dir.substr(0, separator);
      printf("%s\n", current_dir.c_str());
      int success = chdir(current_dir.c_str());
      if (success) break;
      char real_path[256];
      current_dir = getcwd(real_path, sizeof(real_path));
      std::string target = current_dir +
                           std::string(1, flatbuffers::kPathSeparator) +
                           std::string(target_dir);
      success = chdir(target.c_str());
      if (success == 0) return true;
    }
    return false;
  }
#else
  (void)binary_dir;
  (void)target_dir;
  return true;
#endif  // !defined(__ANDROID__)
}

static inline bool IsUpperCase(const char c) { return c == toupper(c); }

std::string CamelCaseToSnakeCase(const char* const camel) {
  // Replace capitals with underbar + lowercase.
  std::string snake;
  for (const char* c = camel; *c != '\0'; ++c) {
    if (IsUpperCase(*c)) {
      const bool is_start_or_end = c == camel || *(c + 1) == '\0';
      if (!is_start_or_end) {
        snake += '_';
      }
      snake += static_cast<char>(tolower(*c));
    } else {
      snake += *c;
    }
  }
  return snake;
}

std::string FileNameFromEnumName(const char* const enum_name,
                                 const char* const prefix,
                                 const char* const suffix) {
  // Skip over the initial 'k', if it exists.
  const bool starts_with_k = enum_name[0] == 'k' && IsUpperCase(enum_name[1]);
  const char* const camel_case_name = starts_with_k ? enum_name + 1 : enum_name;

  // Assemble file name.
  return std::string(prefix) + CamelCaseToSnakeCase(camel_case_name) +
         std::string(suffix);
}

#ifdef __ANDROID__
bool AndroidSystemFeature(const char* feature_name) {
  JNIEnv* env = reinterpret_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
  jobject activity = reinterpret_cast<jobject>(SDL_AndroidGetActivity());
  jclass fpl_class = env->GetObjectClass(activity);
  jmethodID has_system_feature =
      env->GetMethodID(fpl_class, "hasSystemFeature", "(Ljava/lang/String;)Z");
  jstring jfeature_name = env->NewStringUTF(feature_name);
  jboolean has_feature =
      env->CallBooleanMethod(activity, has_system_feature, jfeature_name);
  env->DeleteLocalRef(jfeature_name);
  env->DeleteLocalRef(fpl_class);
  env->DeleteLocalRef(activity);
  return has_feature;
}
#endif

bool TouchScreenDevice() {
#ifdef __ANDROID__
  return AndroidSystemFeature("android.hardware.touchscreen");
#else
  return false;
#endif
}

#ifdef __ANDROID__
bool AndroidCheckDeviceList(const char* device_list[], const int num_devices) {
  // Retrieve device name through JNI.
  JNIEnv* env = reinterpret_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
  jclass build_class = env->FindClass("android/os/Build");
  jfieldID model_id =
      env->GetStaticFieldID(build_class, "MODEL", "Ljava/lang/String;");
  jstring model_obj =
      static_cast<jstring>(env->GetStaticObjectField(build_class, model_id));
  const char* device_name = env->GetStringUTFChars(model_obj, 0);

  // Check if the device name is in the list.
  bool result = true;
  for (int i = 0; i < num_devices; ++i) {
    if (strcmp(device_list[i], device_name) == 0) {
      result = false;
      break;
    }
  }

  // Clean up
  env->ReleaseStringUTFChars(model_obj, device_name);
  env->DeleteLocalRef(model_obj);
  env->DeleteLocalRef(build_class);
  return result;
}
#endif

bool MipmapGeneration16bppSupported() {
#ifdef __ANDROID__
  const char* device_list[] = {"Galaxy Nexus"};
  return AndroidCheckDeviceList(device_list,
                                sizeof(device_list) / sizeof(device_list[0]));
#else
  return true;
#endif
}
  
  using namespace std;
  
  vector<string> FileUtils::List(string directory) {
    if(directory.empty() || directory.back() != '/')
      directory += '/';
    
    vector<string> list;
    
    DIR *dir = opendir(directory.c_str());
    if(!dir)
      return list;
    
    while(true) {
      dirent *ent = readdir(dir);
      if(!ent)
        break;
      // Skip dotfiles (including "." and "..").
      if(ent->d_name[0] == '.')
        continue;
      
      string name = directory + ent->d_name;
      // Don't assume that this operating system's implementation of dirent
      // includes the t_type field; in particular, on Windows it will not.
      struct stat buf;
      stat(name.c_str(), &buf);
      bool isRegularFile = S_ISREG(buf.st_mode);
      
      if(isRegularFile)
        list.push_back(name);
    }
    
    closedir(dir);
    
    return list;
  }
  
  // Get a list of any directories in the given directory.
  vector<string> FileUtils::ListDirectories(string directory) {
    if(directory.empty() || directory.back() != '/')
      directory += '/';
    
    vector<string> list;
    
    DIR *dir = opendir(directory.c_str());
    if(!dir)
      return list;
    
    while(true) {
      dirent *ent = readdir(dir);
      if(!ent)
        break;
      // Skip dotfiles (including "." and "..").
      if(ent->d_name[0] == '.')
        continue;
      
      string name = directory + ent->d_name;
      // Don't assume that this operating system's implementation of dirent
      // includes the t_type field; in particular, on Windows it will not.
      struct stat buf;
      stat(name.c_str(), &buf);
      bool isDirectory = S_ISDIR(buf.st_mode);
      
      if(isDirectory) {
        if(name.back() != '/')
          name += '/';
        list.push_back(name);
      }
    }
    
    closedir(dir);
    
    return list;
  }
  
  vector<string> FileUtils::RecursiveList(const string &directory) {
    vector<string> list;
    RecursiveList(directory, &list);
    return list;
  }
  
  void FileUtils::RecursiveList(string directory, vector<string> *list) {
    if(directory.empty() || directory.back() != '/')
      directory += '/';
    
    DIR *dir = opendir(directory.c_str());
    if(!dir)
      return;
    
    while(true)
    {
      dirent *ent = readdir(dir);
      if(!ent)
        break;
      // Skip dotfiles (including "." and "..").
      if(ent->d_name[0] == '.')
        continue;
      
      string name = directory + ent->d_name;
      // Don't assume that this operating system's implementation of dirent
      // includes the t_type field; in particular, on Windows it will not.
      struct stat buf;
      stat(name.c_str(), &buf);
      bool isRegularFile = S_ISREG(buf.st_mode);
      bool isDirectory = S_ISDIR(buf.st_mode);
      
      if(isRegularFile)
        list->push_back(name);
      else if(isDirectory)
        RecursiveList(name + '/', list);
    }
    
    closedir(dir);
  }
  
  bool FileUtils::Exists(const string &filePath) {
    struct stat buf;
    return !stat(filePath.c_str(), &buf);
  }
  
  void FileUtils::Copy(const string &from, const string &to) {
    ifstream in(from, ios::binary);
    ofstream out(to, ios::binary);
    
    out << in.rdbuf();
  }
  
  void FileUtils::Move(const string &from, const string &to) {
    rename(from.c_str(), to.c_str());
  }
  
  void FileUtils::Delete(const string &filePath) {
    unlink(filePath.c_str());
  }
  
  // Get the filename from a path.
  string FileUtils::Name(const string &path) {
    // string::npos = -1, so if there is no '/' in the path this will
    // return the entire string, i.e. path.substr(0).
    return path.substr(path.rfind('/') + 1);
  }
  
  
  // resource home
  std::string FileUtils::Resource() {
    string resources;
    
    // Find the path to the resource directory. This will depend on the
    // operating system, and can be overridden by a command line argument.
    char *str = SDL_GetBasePath();
    resources = str;
    SDL_free(str);
    
    if(resources.back() != '/') {
      resources += '/';
    }
#if defined __linux__ || defined __FreeBSD__ || defined __DragonFly__
    // Special case, for Linux: the resource files are not in the same place as
    // the executable, but are under the same prefix (/usr or /usr/local).
    static const string LOCAL_PATH = "/usr/local/";
    static const string STANDARD_PATH = "/usr/";
    static const string RESOURCE_PATH = "share/games/endless-sky/";
    if(!resources.compare(0, LOCAL_PATH.length(), LOCAL_PATH))
      resources = LOCAL_PATH + RESOURCE_PATH;
    else if(!resources.compare(0, STANDARD_PATH.length(), STANDARD_PATH))
      resources = STANDARD_PATH + RESOURCE_PATH;
#elif defined __APPLE__ && defined(__MACOSX)
    // Special case for Mac OS X: the resources are in ../Resources relative to
    // the folder the binary is in.
    size_t pos = resources.rfind('/', resources.length() - 2) + 1;
    resources = resources.substr(0, pos) + "Resources/";
#endif
    
    return resources;
  }

}  // namespace fpl

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

#ifndef PIE_NOON_UTILITIES_H
#define PIE_NOON_UTILITIES_H

#include <string>

#include "mathfu/utilities.h"

// TODO: put in alphabetical order when FlatBuffers predeclare bug fixed.
#include "pie_noon_common_generated.h"
#include "audio_config_generated.h"
#include "config_generated.h"

namespace fpl {

bool LoadFile(const char* filename, std::string* dest);

inline const mathfu::vec3 LoadVec3(const pie_noon::Vec3* v) {
  // Note: eschew the constructor that loads contiguous floats. It's faster
  // than the x, y, z constructor we use here, but doesn't account for the
  // endian swap that might occur in pie_noon::Vec3::x().
  return mathfu::vec3(v->x(), v->y(), v->z());
}

inline const mathfu::vec4 LoadVec4(const pie_noon::Vec4* v) {
  return mathfu::vec4(v->x(), v->y(), v->z(), v->w());
}

inline const mathfu::vec2i LoadVec2i(const pie_noon::Vec2i* v) {
  return mathfu::vec2i(v->x(), v->y());
}

inline const mathfu::vec2 LoadVec2(const pie_noon::Vec2* v) {
  return mathfu::vec2(v->x(), v->y());
}

inline const mathfu::vec3 LoadAxis(pie_noon::Axis axis) {
  return axis == pie_noon::Axis_X ? mathfu::kAxisX3f : axis == pie_noon::Axis_Y
                                                           ? mathfu::kAxisY3f
                                                           : mathfu::kAxisZ3f;
}

bool ChangeToUpstreamDir(const char* const binary_dir,
                         const char* const target_dir);

std::string CamelCaseToSnakeCase(const char* const camel);

std::string FileNameFromEnumName(const char* const enum_name,
                                 const char* const prefix,
                                 const char* const suffix);

bool TouchScreenDevice();

bool MipmapGeneration16bppSupported();
  
  class FileUtils {
    
  public:
    
    // Get a list of all regular files in the given directory.
    static std::vector<std::string> List(std::string directory);
    // Get a list of any directories in the given directory.
    static std::vector<std::string> ListDirectories(std::string directory);
    // Get a list of all regular files in the given directory or any directory
    // that it contains, recursively.
    static std::vector<std::string> RecursiveList(const std::string &directory);
    static void RecursiveList(std::string directory, std::vector<std::string> *list);
    
    static bool Exists(const std::string &filePath);
    static void Copy(const std::string &from, const std::string &to);
    static void Move(const std::string &from, const std::string &to);
    static void Delete(const std::string &filePath);
    
    // Get the filename from a path.
    static std::string Name(const std::string &path);
    
    // resource home
    static std::string Resource();
    
  };

}  // namespace fpl

#endif  // PIE_NOON_UTILITIES_H

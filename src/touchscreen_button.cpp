#include "touchscreen_button.h"
#include "utilities.h"

namespace fpl {
namespace pie_noon {

TouchscreenButton::TouchscreenButton()
    : elapsed_time_(0),
      up_current_(0),
      down_material_(nullptr),
      color_(mathfu::kOnes4f),
      is_active_(true),
      is_visible_(true),
      is_highlighted_(false),
      one_over_cannonical_window_height_(0.0f) {}

ButtonId TouchscreenButton::GetId() const {
  if (button_def_ != nullptr) {
    return button_def_->ID();
  } else {
    return ButtonId_Undefined;
  }
}

bool TouchscreenButton::WillCapturePointer(const Pointer& pointer,
                                           vec2 window_size) {
  if (is_visible_ &&
      pointer.mousepos.x() / window_size.x() >= button_def_->top_left()->x() &&
      pointer.mousepos.y() / window_size.y() >= button_def_->top_left()->y() &&
      pointer.mousepos.x() / window_size.x() <=
          button_def_->bottom_right()->x() &&
      pointer.mousepos.y() / window_size.y() <=
          button_def_->bottom_right()->y()) {
    return true;
  }
  return false;
}

void TouchscreenButton::AdvanceFrame(WorldTime delta_time, InputSystem* input,
                                     vec2 window_size) {
  elapsed_time_ += delta_time;
  button_.AdvanceFrame();
  bool down = false;

  for (size_t i = 0; i < input->pointers_.size(); i++) {
    const Pointer& pointer = input->pointers_[i];
    const Button pointer_button = input->GetPointerButton(pointer.id);
    if ((pointer_button.is_down() || pointer_button.went_down()) &&
        WillCapturePointer(pointer, window_size)) {
      down = true;
      break;
    }
  }
  button_.Update(down);
}

bool TouchscreenButton::IsTriggered() {
  return (button_def_->event_trigger() == ButtonEvent_ButtonHold &&
          button_.is_down()) ||
         (button_def_->event_trigger() == ButtonEvent_ButtonPress &&
          button_.went_down());
}

void TouchscreenButton::Render(Renderer& renderer) {
  static const float kButtonZDepth = 0.0f;

  if (!is_visible_) {
    return;
  }
  renderer.color() = color_;

  Material* mat = (button_.is_down() && down_material_ != nullptr)
                      ? down_material_
                      : up_current_ < up_materials_.size()
                            ? up_materials_[up_current_]
                            : nullptr;
  if (!mat) return;  // This is an invisible button.

  const vec2 window_size = vec2(renderer.window_size());
  const float texture_scale =
      window_size.y() * one_over_cannonical_window_height_;

  vec2 base_size = LoadVec2(
      is_highlighted_ ? button_def_->draw_scale_highlighted()
                      : (button_.is_down() ? button_def_->draw_scale_pressed()
                                           : button_def_->draw_scale_normal()));

  auto pulse = sinf(static_cast<float>(elapsed_time_) / 100.0f);
  if (is_highlighted_) {
    base_size += mathfu::kOnes2f * pulse * 0.05f;
  }

  vec3 texture_size =
      texture_scale * vec3(mat->textures()[0]->size().x() * base_size.x(),
                           -mat->textures()[0]->size().y() * base_size.y(), 0);

  vec3 position = vec3(button_def()->texture_position()->x() * window_size.x(),
                       button_def()->texture_position()->y() * window_size.y(),
                       kButtonZDepth);

  if (is_active_ || inactive_shader_ == nullptr) {
    shader_->Set(renderer);
  } else {
    inactive_shader_->Set(renderer);
  }
  mat->Set(renderer);
  Mesh::RenderAAQuadAlongX(position - (texture_size / 2.0f),
                           position + (texture_size / 2.0f), vec2(0, 1),
                           vec2(1, 0));
}

StaticImage::StaticImage()
    : image_def_(nullptr),
      current_material_index_(0),
      shader_(nullptr),
      scale_(mathfu::kZeros2f),
      texture_position_(mathfu::kZeros2f),
      color_(mathfu::kOnes4f),
      one_over_cannonical_window_height_(0.0f),
      is_visible_(true) {}

void StaticImage::Initialize(const StaticImageDef& image_def,
                             std::vector<Material*> materials, Shader* shader,
                             int cannonical_window_height) {
  image_def_ = &image_def;
  materials_ = materials;
  current_material_index_ = 0;
  shader_ = shader;
  scale_ = LoadVec2(image_def_->draw_scale());
  texture_position_ = LoadVec2(image_def_->texture_position());
  color_ = mathfu::kOnes4f;
  one_over_cannonical_window_height_ =
      1.0f / static_cast<float>(cannonical_window_height);
  is_visible_ = image_def_->visible() != 0;
  assert(Valid());
}

bool StaticImage::Valid() const {
  return image_def_ != nullptr &&
         current_material_index_ < static_cast<int>(materials_.size()) &&
         materials_[current_material_index_] != nullptr && shader_ != nullptr;
}

void StaticImage::Render(Renderer& renderer) {
  if (!Valid()) return;
  if (!is_visible_) return;
  renderer.color() = color_;

  Material* material = materials_[current_material_index_];
  const vec2 window_size = vec2(renderer.window_size());
  const float texture_scale =
      window_size.y() * one_over_cannonical_window_height_;
  const vec2 texture_size =
      texture_scale * vec2(material->textures()[0]->size()) * scale_;
  const vec2 position_percent = texture_position_;
  const vec2 position = window_size * position_percent;

  const vec3 position3d(position.x(), position.y(), image_def_->z_depth());
  const vec3 texture_size3d(texture_size.x(), -texture_size.y(), 0.0f);

  shader_->Set(renderer);
  material->Set(renderer);

  Mesh::RenderAAQuadAlongX(position3d - texture_size3d * 0.5f,
                           position3d + texture_size3d * 0.5f, vec2(0, 1),
                           vec2(1, 0));
}

}  // pie_noon
}  // fpl

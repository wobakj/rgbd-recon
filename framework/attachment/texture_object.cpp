#include "texture_object.hpp"
#include "utils.hpp"

std::queue<std::size_t> TextureObject::units_{};
std::size_t TextureObject::active_unit_ = 0;
bool TextureObject::db_inited_ = false;

TextureObject::TextureObject()
 :texture_{}
 ,unit_{0}
 ,handle_{0}
{}

TextureObject::TextureObject(texture tex)
 :texture_{tex}
 ,unit_{0}
 ,handle_{0}
{
  // first, initialize database if necessary
  if (!db_inited_) {
    initDatabase();
  }

  // get current active unit
  std::size_t curr_unit = active_unit_;
  activateUnit(0);

  glGenTextures(1, &handle_);

  // bind new texture handle to current unit for configuration
  glBindTexture(tex.target, handle_);
  // if coordinate is outside texture, use border color
  glTexParameteri(tex.target, GL_TEXTURE_WRAP_S, GLint(GL_CLAMP_TO_EDGE));
  //linear interpolation if texel is smaller/bigger than fragment pixel 
  glTexParameteri(tex.target, GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
  glTexParameteri(tex.target, GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));

  // create texture on gpu at current unit 
  formatTextureObject();

  // unbind texture
  glBindTexture(texture_.target, 0);
  // bind this texture to the next free unit
  bindTo(nextUnit());
  // reactivate previously active unit
  activateUnit(curr_unit);
}

TextureObject::TextureObject(TextureObject const& tex_obj)
 :TextureObject{tex_obj.texture_}
{}

TextureObject::TextureObject(TextureObject&& tex_obj)
 :TextureObject{}
{
  this->swap(tex_obj);
}

TextureObject& TextureObject::operator=(TextureObject tex_obj) {
  this->swap(tex_obj);
  return *this;
}

TextureObject::~TextureObject() {
  // unbind if necessary
  if (bound()) {
    // activate used slot
    if (active_unit_ != unit_) {
      activateUnit(unit_);
    }
    // unbind texture
    glBindTexture(texture_.target, 0);
    units_.push(unit_);
  }
  // free buffers
  glDeleteTextures(1, &handle_);
}

void TextureObject::swap(TextureObject& b) {
  std::swap(texture_, b.texture_);
  std::swap(handle_, b.handle_);
  std::swap(unit_, b.unit_);
}

void TextureObject::formatTextureObject() {
  GLenum internal_format = utils::pixel_type(texture_.channels, texture_.channel_type);
  // create blank texture if struct contains no pixel data
  GLvoid const* data_ptr = nullptr;
  if (!texture_.data.empty()) {
    data_ptr = &texture_.data[0];
  }
  // define & upload texture data
  if (texture_.target == GL_TEXTURE_1D){
    glTexImage1D(texture_.target, 0, GLint(internal_format), texture_.width, 0, texture_.channels, texture_.channel_type, data_ptr);
  }
  else if (texture_.target == GL_TEXTURE_2D) {
    glTexParameteri(texture_.target, GL_TEXTURE_WRAP_T, GLint(GL_CLAMP_TO_EDGE));
    glTexImage2D(texture_.target, 0, GLint(internal_format), texture_.width, texture_.height, 0, texture_.channels, texture_.channel_type, data_ptr);
  }
  else if (texture_.target == GL_TEXTURE_3D){
    glTexParameteri(texture_.target, GL_TEXTURE_WRAP_T, GLint(GL_CLAMP_TO_EDGE));
    glTexParameteri(texture_.target, GL_TEXTURE_WRAP_R, GLint(GL_CLAMP_TO_EDGE));
    glTexImage3D(texture_.target, 0, GLint(internal_format), texture_.width, texture_.height, texture_.depth, 0, texture_.channels, texture_.channel_type, data_ptr);
  }
  else {
    throw std::logic_error("Unsupported Format ");
  }
}

GLint TextureObject::unit() const {
  return unit_;
}

glm::uvec3 TextureObject::dimensions() const {
  return glm::uvec3{texture_.width, texture_.height, texture_.depth};
}

void TextureObject::bindTo(std::size_t unit) {
  // get currently active unit
  std::size_t curr_unit = active_unit_;
  // activate unit and bind texture
  activateUnit(unit);
  glBindTexture(texture_.target, handle_);
  unit_ = unit;

  // reactivate previously active unit
  activateUnit(curr_unit);
}

bool TextureObject::bound() const {
  return unit_ != 0;
}

// static methods
void TextureObject::initDatabase() {
  GLint num_units = 0;
  glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &num_units);

  for (int i = 30; i < num_units - 1; ++i) {
    units_.push(i);
  }
  db_inited_ = true;
} 

void TextureObject::activateUnit(std::size_t unit) {
  glActiveTexture(GL_TEXTURE0 + unit);
  active_unit_ = unit;
}

std::size_t TextureObject::nextUnit() {
  static std::size_t unit = 30;
  ++unit;
  return unit;
}

void swap(TextureObject& a, TextureObject& b) {
  a.swap(b);
}

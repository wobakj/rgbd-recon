#ifndef TEXTURE_OBJECT_HPP
#define TEXTURE_OBJECT_HPP

#include "texture.hpp"

#include <glbinding/gl/gl.h>
// use floats and med precision operations
#define GLM_PRECISION_MEDIUMP_FLOAT
#include <glm/vec3.hpp>

#include <queue>
// use gl definitions from glbinding 
using namespace gl;

class TextureObject {
 public:
  TextureObject();
  TextureObject(texture tex);
  TextureObject(TextureObject const& tex);
  TextureObject(TextureObject&& tex);
  ~TextureObject();

  TextureObject& operator=(TextureObject);

  void swap(TextureObject& b);
  // return used texture unit
  GLint unit() const;

  glm::uvec3 dimensions() const;

  GLuint handle() const {
    return handle_;
  }

 protected:
  // cpu representation
  texture texture_;

  // create/update texture parameters 
  void formatTextureObject();

  static void activateUnit(std::size_t);
  static std::size_t active_unit_;
  // index of the used texture unit 
  std::size_t unit_;

  GLuint handle_;
  
 private:
  bool bound() const;
  void bindTo(std::size_t unit);
  // handle of the opengl object

  // activate unit in gl state
  static void initDatabase(); 
  static std::size_t nextUnit();

  static std::queue<std::size_t> units_;
  // whether the queue was filled
  static bool db_inited_;

  // give framebuffer access to handle
  friend class FrameBuffer;
};
  
void swap(TextureObject& a, TextureObject& b);

#endif
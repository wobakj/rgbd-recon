#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <glbinding/gl/gl.h>
#include <vector>

using namespace gl;

// holds texture data and format information
struct texture {
  texture()
   :data()
   ,width{0}
   ,height{0}
   ,depth{0}
   ,target{GL_NONE}
   ,channels{GL_NONE}
   ,channel_type{GL_NONE}
  {}

  texture(GLenum t, GLenum c, GLenum ty, GLsizei w, GLsizei h = 1, GLsizei d = 1)
   :data(0)
   ,width{w}
   ,height{h}
   ,depth{d}
   ,target{t}
   ,channels{c}
   ,channel_type{ty}
  {}

  texture(std::vector<uint8_t> dat, GLenum t, GLenum c, GLenum ty, GLsizei w, GLsizei h = 0, GLsizei d = 0)
   :data(dat)
   ,width{w}
   ,height{h}
   ,depth{d}
   ,target{GL_TEXTURE_2D}
   ,channels{c}
   ,channel_type{ty}
  {
    // fit target to input dimensions
    if (height < 2) {
      target = GL_TEXTURE_1D;
    }
    else if (depth > 1) {
      target = GL_TEXTURE_3D;
    }
  }

  std::vector<uint8_t> data;
  GLsizei width;
  GLsizei height;
  GLsizei depth;
  // texture purpose
  GLenum target;
  // channel format
  GLenum channels; 
  // pixel format
  GLenum channel_type; 
};

#endif
#ifndef UTILS_HPP
#define UTILS_HPP

#include "model.hpp"

#include <glbinding/gl/gl.h>
// use gl definitions from glbinding 
using namespace gl;

struct texture;

namespace utils {
  // generate texture object form texture struct
  GLuint texture_object(texture const& tex);
  // print bound textures for all texture units
  void print_bound_textures();
  
  // GLSLS error callback
  void glsl_error(int error, const char* description);
  // output current error
  bool query_gl_error();
  // check after every function if error was caused
  void watch_gl_errors(bool activate = true);
  // test program for drawing validity
  void validate_program(GLuint program);
  // return handle of bound vertex array object
  GLint get_bound_VAO();

  // extract filename from path
  std::string file_name(std::string const& file_path);
  // output a gl error log in cerr
  void output_log(GLchar const* log_buffer, std::string const& prefix);
  // read file and write content to string
  std::string read_file(std::string const& name);

  // convert between number of primitives and vertices
  std::size_t prims_to_verts(GLenum primitive, std::size_t count);
  std::size_t verts_to_prims(GLenum primitive, std::size_t count);

  // calculate vertex bytes from attributes
  std::size_t attribs_to_bytes(model::attrib_flag_t attribs);

  // get pixel size from channel number and type
  std::size_t pixel_bytes(GLenum channels, GLenum channel_type);
  // get internal pixel type from channel number and type
  GLenum pixel_type(GLenum channels, GLenum channel_type);
  // get channel enum for given number
  GLenum channel_num_to_type(unsigned num_components);
  // get (non float) channel type for given size
  GLenum channel_bytes_to_type(std::size_t num_bytes, bool sign = false);

  std::size_t type_to_bytes(GLenum type);
}
#endif
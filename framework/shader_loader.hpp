#ifndef SHADER_LOADER_HPP
#define SHADER_LOADER_HPP

#include <glbinding/gl/gl.h>

#include <vector>

using namespace gl;

namespace shader_loader {
  // compile shader
  GLuint shader(std::string const& file_path, GLenum shader_type);

  // create program from given list of stages and activate given varyings for transform feedback
  GLuint program(std::vector<std::pair<std::string, GLenum>> const&, std::vector<char const*> const& feedback_varyings = std::vector<char const*>{});
};

#endif

#ifndef SHADER_PROGRAMM_HPP
#define SHADER_PROGRAMM_HPP

#include "uniform_value.hpp"

#include <glbinding/gl/gl.h>

#include <map>
#include <vector>

// use gl definitions from glbinding 
using namespace gl;

class UniformBuffer;
class StorageBuffer;
// struct to discern uniform buffer index from uint
template<typename T>
struct binding_t {
  binding_t(GLuint i)
   :index{i}
  {}
  GLuint index = 0;
};

// base class for shaders, responsible for managing uniforms
class ShaderProgram {
 public:
  ShaderProgram(std::vector<std::pair<std::string, GLenum>> const& stages);
  ShaderProgram(ShaderProgram const& program);
  ShaderProgram(ShaderProgram&& program);
  ~ShaderProgram();

  ShaderProgram& operator=(ShaderProgram program);

  void bind() const;

  virtual void reload();

  bool hasUniform(std::string const& name) const;

  template<typename T>
  void setUniform(std::string const& name, T const& value) {
    if(uniform_values_.find(name) != uniform_values_.end()) {
      uniform_values_.at(name) = value;
    }
    else {
      uniform_values_.insert(std::make_pair(name, Uniform{value}));
      GLuint location = 0;
      if (typeid(value) == typeid(binding_t<UniformBuffer>)) {
        location = glGetUniformBlockIndex(handle_, name.c_str());
      }
      else if (typeid(value) == typeid(binding_t<StorageBuffer>)) {
        location = glGetProgramResourceIndex(handle_, GL_SHADER_STORAGE_BLOCK, name.c_str());
      }
      else {
        location = glGetUniformLocation(handle_, name.c_str());
      }
      uniform_locations_[name] = location;
    }
    // if shader is currently bound, upload now 
    if(bound()) {
      upload(uniform_locations_.at(name), uniform_values_.at(name));
      uniform_values_.at(name).clean();
    }
  }

  void swap(ShaderProgram& prog);
 protected:
  // helpers to use in child classes
  void replaceProgram(GLuint new_handle);

  std::vector<std::pair<std::string, GLenum>> stages_;
  GLuint handle_;

 private:
  bool bound() const;

  void upload(GLint i, Uniform const& value) const;
  void copyUniforms(ShaderProgram const& program);
  void reloadUniformLocations();
  

  std::map<std::string, GLint> uniform_locations_; 
  std::map<std::string, Uniform> uniform_values_;

  static GLuint bound_handle_;

  friend class UniformBuffer;
  friend class StorageBuffer;
  friend class ShaderBuffer;
};

void swap(ShaderProgram&, ShaderProgram&);

#endif
#ifndef DRAW_PROGRAMM_HPP
#define DRAW_PROGRAMM_HPP

#include "shader_program.hpp"

// use gl definitions from glbinding 
using namespace gl;

// shader program with fragments shader
class DrawProgram : public ShaderProgram {
 public:
  DrawProgram();
  DrawProgram(std::string const& vert_path, std::string const& frag_path);
  DrawProgram(std::string const& vert_path, std::string const& geo_path, std::string const& frag_path);
  DrawProgram(DrawProgram const& program);
  DrawProgram(DrawProgram&& program);

  DrawProgram& operator=(DrawProgram program);

  void swap(DrawProgram& prog);

  // allows calling globally
  friend void swap(DrawProgram& g1, DrawProgram& g2);
};

// allows calling from within classe, otherwise Base::swap(a,b) will be used
void swap(DrawProgram& g1, DrawProgram& g2);

#endif
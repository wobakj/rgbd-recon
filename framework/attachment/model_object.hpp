#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include "model.hpp"

#include <glbinding/gl/gl.h>
// use gl definitions from glbinding 
using namespace gl;

// holds cpu and gpu representation of a geometry
class ModelObject {
 public:
  ModelObject();
  ModelObject(model m, GLenum draw_mode);
  ModelObject(ModelObject const& geo);
  ModelObject(ModelObject&& geo);
  ~ModelObject();

  ModelObject& operator=(ModelObject);

  void draw() const;
  void bind() const;
 private:
  bool bound() const;
  // cpu representation of model
  model model_;
  // mode in which elements are drawn
  GLenum draw_mode_;
  // vertex objects handles
  GLuint vertex_AO_;
  GLuint vertex_BO_;
  GLuint element_BO_;
  
  friend void swap(ModelObject& g1, ModelObject& g2);

  static GLuint bound_VAO_;
};


#endif
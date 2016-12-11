#include "model_object.hpp"
#include "utils.hpp"
#include <cassert>

GLuint ModelObject::bound_VAO_ = 0;

ModelObject::ModelObject()
 :model_{}
 ,draw_mode_{GL_NONE}
 ,vertex_AO_{0}
 ,vertex_BO_{0}
 ,element_BO_{0}
{}

ModelObject::ModelObject(model m, GLenum draw_mode)
 :model_{m}
 ,draw_mode_{draw_mode}
 ,vertex_AO_{0}
 ,vertex_BO_{0}
 ,element_BO_{0}
{
  // generate vertex array object
  glGenVertexArrays(1, &vertex_AO_);
  // bind the array for attaching buffers
  glBindVertexArray(vertex_AO_);

  // generate generic buffer
  glGenBuffers(1, &vertex_BO_);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, vertex_BO_);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model_.data.size(), &model_.data[0], GL_STATIC_DRAW);

  GLuint attrib_index = 0;
  for(model::attribute const& attribute : model::VERTEX_ATTRIBS) {
    if(model_.offsets.find(attribute) != model_.offsets.end()) {
      // activate attribute in vao
      glEnableVertexAttribArray(attrib_index);
      // define attribute characteristics
      glVertexAttribPointer(attrib_index, attribute.components, attribute.type, GL_FALSE, model_.vertex_bytes, model_.offsets[attribute]);

      ++attrib_index;
    }
  }

  if(!model_.indices.empty()) { 
     // generate generic buffer
    glGenBuffers(1, &element_BO_);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_BO_);
    // configure currently bound array buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * model_.indices.size(), &model_.indices[0], GL_STATIC_DRAW);
  }
  else {

  }

  // unbind va to prevent accidental modification
  glBindVertexArray(0);
}

ModelObject::ModelObject(ModelObject const& geo)
 :ModelObject{geo.model_, geo.draw_mode_}
{}

ModelObject::ModelObject(ModelObject&& geo)
 :ModelObject{} 
{
  swap(*this, geo);
}

ModelObject& ModelObject::operator=(ModelObject geo) {
  swap(*this, geo);
  return *this;
}

ModelObject::~ModelObject() {
  // unbind if necessary
  if(bound()) {
    glBindVertexArray(0);
    bound_VAO_ = 0;
  }
  // free buffers
  glDeleteBuffers(1, &vertex_BO_);
  glDeleteVertexArrays(1, &element_BO_);
  glDeleteVertexArrays(1, &vertex_AO_);
}

void swap(ModelObject& g1, ModelObject& g2) {
  std::swap(g1.model_, g2.model_);
  std::swap(g1.draw_mode_, g2.draw_mode_);
  std::swap(g1.vertex_AO_, g2.vertex_AO_);
  std::swap(g1.vertex_BO_, g2.vertex_BO_);
  std::swap(g1.element_BO_, g2.element_BO_);
}

void ModelObject::bind() const {
  // dont do expensive bind if already bound
  // if(!bound()) {
    // bind the array for attaching buffers
    glBindVertexArray(vertex_AO_);

    bound_VAO_ = vertex_AO_;
  // }
}

bool ModelObject::bound() const {
  return bound_VAO_ == vertex_AO_;
}

void ModelObject::draw() const {
  // bind();
  // check if indices are present
  if(!model_.indices.empty()) {
    // draw bound vertex array as indices using bound shader
    glDrawElements(draw_mode_, model_.indices.size(), model::INDEX.type, NULL);
  }
  else {
    // draw vertices starting from 0 to end
    glDrawArrays(draw_mode_, 0, model_.vertex_num);
  }
}
#include "ViewArray.h"

#include <gl_util.h>
#include <stdio.h>
#include <iostream>

namespace mvt{

ViewArray::ViewArray(unsigned width, unsigned height, unsigned numLayers)
  : m_width(width),
    m_height(height),
    m_numLayers(numLayers),
    m_colorArray(m_width, m_height, m_numLayers, GL_RGBA32F, GL_RGBA, GL_FLOAT),
    m_depthArray(m_width, m_height, m_numLayers, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT),
    m_fbo{new globjects::Framebuffer()},
    m_current_fbo(0),
    m_viewport(0,0,width,height),
    m_viewport_current(0,0,width, height)
{
  m_depthArray.setMAGMINFilter(GL_NEAREST);

  m_fbo->setDrawBuffers({GL_COLOR_ATTACHMENT0});
}

ViewArray::~ViewArray() {
  m_fbo->destroy();
}

void ViewArray::init() {

}

void ViewArray::enable(unsigned layer, bool use_vp, unsigned* ox, unsigned* oy, bool clearcolor) {
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &m_current_fbo);

  m_fbo->bind();
  m_fbo->attachTextureLayer(GL_COLOR_ATTACHMENT0, m_colorArray.getTexture(), 0, layer);
  m_fbo->attachTextureLayer(GL_DEPTH_ATTACHMENT, m_depthArray.getTexture(), 0, layer);

  if(use_vp){
    m_viewport.enter();
  }
  else{
    unsigned x;
    unsigned y;
    unsigned w;
    unsigned h;
    getWidthHeight(x,y,w,h);
    if(0 == ox || 0 == oy)
       std::cerr << " ViewArray::enable(unsigned layer, bool use_vp, unsigned* ox, unsigned* oy) ERROR" << std::endl;
    *ox = x;
    *oy = y;
    //std::cerr << x << " " << y << " " << w << " " << h << std::endl;
    m_viewport_current.set(x,y,w,h);
    glViewport(0,0,w,h);
  }

  if(clearcolor) {
    glClearColor(0.0,0.0,0.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }
  else {
    glClear(GL_DEPTH_BUFFER_BIT);
  }
}

void
ViewArray::disable(bool use_vp){
  glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_current_fbo);
  if(use_vp){
    m_viewport.leave();
  }
  else{
    m_viewport_current.enter(false);
  }
}

void
ViewArray::bindToTextureUnits(unsigned start_texture_unit){
  glActiveTexture(GL_TEXTURE0 + start_texture_unit);
  m_colorArray.bind();
  glActiveTexture(GL_TEXTURE0 + start_texture_unit + 1);
  m_depthArray.bind();
}

void
ViewArray::bindToTextureUnitDepth(unsigned start_texture_unit){
  glActiveTexture(GL_TEXTURE0 + start_texture_unit);
  m_depthArray.bind();
}

void
ViewArray::bindToTextureUnitRGBA(unsigned start_texture_unit){
  glActiveTexture(GL_TEXTURE0 + start_texture_unit);
  m_colorArray.bind();
}

unsigned
ViewArray::getWidth(){
  return m_width;
}

unsigned
ViewArray::getHeight(){
  return m_height;
}

unsigned
ViewArray::getNumLayers(){
  return m_numLayers;
}

void
ViewArray::getWidthHeight(unsigned& x, unsigned& y, unsigned& width, unsigned& height){
  GLsizei vp_params[4];
  glGetIntegerv(GL_VIEWPORT,vp_params);
  x = vp_params[0];
  y = vp_params[1];
  width  = vp_params[2];
  height = vp_params[3];
}
}
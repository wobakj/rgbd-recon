#ifndef RECONSTRUCTION_HPP
#define RECONSTRUCTION_HPP

#include "gloost/BoundingBox.h"

namespace kinect{

  class CalibVolumes;
  class CalibrationFiles;

  class Reconstruction{

  public:
    Reconstruction(CalibrationFiles const& cfs, CalibVolumes const* cv, gloost::BoundingBox const&  bbox);

    virtual void draw() = 0;

    virtual void reload();
    // mustnt be implemented by children without fbos
    virtual void resize(std::size_t width, std::size_t height);

  protected:
    CalibVolumes const* m_cv;
    CalibrationFiles const* m_cf;

    unsigned m_tex_width;
    unsigned m_tex_height;
    unsigned m_num_kinects;
    float m_min_length;
    gloost::BoundingBox m_bbox;
  };
}

#endif // #ifndef RECONSTRUCTION_HPP
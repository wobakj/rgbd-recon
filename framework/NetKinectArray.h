#ifndef KINECT_NETKINECTARRAY_H
#define KINECT_NETKINECTARRAY_H




#include <GL/glew.h>
#include <GL/gl.h>

#include <Matrix.h>

#include <string>
#include <vector>
#include <atomic>
#include "DataTypes.h"

namespace boost{
  class thread;
  class mutex;
}

namespace mvt{
  class TextureArray;
}

namespace gloost{
  class Shader;
  class UniformSet;
}

namespace kinect{

  struct double_pbo{
    unsigned size;
    byte* back;
    std::atomic<bool> needSwap;
    unsigned frontID;
    unsigned backID;
    std::vector<gloost::Matrix> current_poses;

    void swap(){
      unsigned tmp = frontID;
      frontID = backID;
      backID = tmp;
    }

  };

  class KinectCalibrationFile;

  class NetKinectArray{


  public:
    NetKinectArray(const char* config, bool readfromfile = false);

    NetKinectArray(std::vector<KinectCalibrationFile*>& calibs);

    virtual ~NetKinectArray();


    virtual void update();

    void bilateralFilter();

    void bindToTextureUnits(unsigned start_texture_unit);
    unsigned getStartTextureUnit() const;

    unsigned getWidth() const;
    unsigned getWidthC() const;

    unsigned getHeight() const;
    unsigned getHeightC() const;

    unsigned getNumLayers() const;

    std::vector<KinectCalibrationFile*> const& getCalibs() const;

    std::vector<gloost::Matrix> current_poses;

    void reloadShader();

    void writeCurrentTexture(std::string prefix);
    void writeBMP(std::string, std::vector<std::uint8_t> const&, unsigned int offset, unsigned int bytesPerPixel);
    
    mvt::TextureArray* getDepthArrayBack();
    mvt::TextureArray* getDepthArray();

  protected:
    void readLoop();
    void readFromFiles();
    bool init();
    unsigned m_width;
    unsigned m_widthc;
    unsigned m_height;
    unsigned m_heightc;

    unsigned m_numLayers;
    std::vector<KinectCalibrationFile*> m_kinectcs;
    mvt::TextureArray* m_colorArray;
    mvt::TextureArray* m_depthArray;

    mvt::TextureArray*  m_colorArray_back;
    mvt::TextureArray*  m_depthArray_back;
    gloost::Shader*     m_shader_bf;
    gloost::UniformSet* m_uniforms_bf;
    unsigned m_fboID;
    unsigned m_gaussID;

    unsigned m_colorsize; // per frame
    unsigned m_depthsize; // per frame
    double_pbo m_colorsCPU3;
    double_pbo m_depthsCPU3;

    boost::mutex* m_mutex;
    boost::thread* m_readThread;
    bool m_running;
    std::string m_serverport;
    static bool s_glewInit;

    std::string m_config;
    unsigned m_start_texture_unit;

  public:
    bool depth_compression_lex;
    float depth_compression_ratio;
  };


}


#endif // #ifndef KINECT_NETKINECTARRAY_H

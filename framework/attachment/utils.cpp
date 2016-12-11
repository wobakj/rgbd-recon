#include "utils.hpp"
#include "texture.hpp"

// load glbinding function type
#include <glbinding/Function.h>
// load meta info extension
#include <glbinding/Meta.h>
// load callback support
#include <glbinding/callbacks.h>

#include <iostream>
#include <sstream>
#include <fstream>

using glbinding::Meta;

namespace utils {
GLuint texture_object(texture const& tex) {
  GLuint texture_object = 0;
  glGenTextures(1, &texture_object);

  // bind new texture handle to current unit for configuration
  glBindTexture(tex.target, texture_object);
  // if coordinate is outside texture, use border color
  glTexParameteri(tex.target, GL_TEXTURE_WRAP_S, GLint(GL_CLAMP_TO_EDGE));
  //linear interpolation if texel is smaller/bigger than fragment pixel 
  glTexParameteri(tex.target, GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR));
  glTexParameteri(tex.target, GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));

  // determine format of image data, internal format should be sized
  GLenum internal_format = GL_NONE;
  if (tex.channels == GL_RED) {
    internal_format = GL_R8;
  }
  else if (tex.channels == GL_RG) {
    internal_format = GL_RG8;
  }
  else if (tex.channels == GL_RGB) {
    internal_format = GL_RGB8;
  }
  else if (tex.channels == GL_RGBA) {
    internal_format = GL_RGBA8;
  }

  // create blank texture if struct contains no pixel data
  GLvoid const* data_ptr = nullptr;
  if (!tex.data.empty()) {
    data_ptr = &tex.data[0];
  }

  // define & upload texture data
  if (tex.target == GL_TEXTURE_1D){
    glTexImage1D(tex.target, 0, GLint(internal_format), tex.width, 0, tex.channels, tex.channel_type, data_ptr);
  }
  else if (tex.target == GL_TEXTURE_2D) {
    glTexParameteri(tex.target, GL_TEXTURE_WRAP_T, GLint(GL_CLAMP_TO_EDGE));
    glTexImage2D(tex.target, 0, GLint(internal_format), tex.width, tex.height, 0, tex.channels, tex.channel_type, data_ptr);
  }
  else if (tex.target == GL_TEXTURE_3D){
    glTexParameteri(tex.target, GL_TEXTURE_WRAP_T, GLint(GL_CLAMP_TO_EDGE));
    glTexParameteri(tex.target, GL_TEXTURE_WRAP_R, GLint(GL_CLAMP_TO_EDGE));
    glTexImage3D(tex.target, 0, GLint(internal_format), tex.width, tex.height, tex.depth, 0, tex.channels, tex.channel_type, data_ptr);
  }
  else {
    throw std::logic_error("Unsupported Format " + Meta::getString(tex.target));
  }

  return texture_object;
}

void print_bound_textures() {
  GLint id1, id2, id3, active_unit, texture_units = 0;
  glGetIntegerv(GL_ACTIVE_TEXTURE, &active_unit);
  std::cout << "Active texture unit: " << active_unit - GLint(GL_TEXTURE0) << std::endl;

  glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);

  for(GLint i = 0; i < texture_units; ++i) {
    glActiveTexture(GL_TEXTURE0 + i);
    glGetIntegerv(GL_TEXTURE_BINDING_3D, &id3);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &id2);
    glGetIntegerv(GL_TEXTURE_BINDING_1D, &id1);
    if(id1 != 0 || id2 != 0 || id3 != 0) {
      std::cout <<"Texture unit " << i << " - ";
      if(id1 != 0) std::cout << "1D: " << id1 << ", ";
      if(id2 != 0) std::cout << "2D: " << id2 << ", ";
      if(id3 != 0) std::cout << "3D: " << id3;
      std::cout << std::endl;
    }
  }
  // reactivate previously active unit
  glActiveTexture(GLenum(active_unit));
}

void glsl_error(int error, const char* description) {
  std::cerr << "GLSL Error " << error << " : "<< description << std::endl;
}

bool query_gl_error() {
  bool error_occured = false;

  GLenum error = glGetError();
  while(error != GL_NO_ERROR) {
    std::cerr << "OpenGL Error: " << Meta::getString(error) << std::endl;
    error = glGetError();

    error_occured = true;
  }

  return error_occured;
}

void watch_gl_errors(bool activate) {
  if(activate) {
    // add callback after each function call
    glbinding::setCallbackMaskExcept(glbinding::CallbackMask::After | glbinding::CallbackMask::ParametersAndReturnValue, { "glGetError" });
    glbinding::setAfterCallback(
      [](glbinding::FunctionCall const& call) {
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
          // print name
          std::cerr <<  "OpenGL Error: " << call.function->name() << "(";
          // parameters
          for (unsigned i = 0; i < call.parameters.size(); ++i)
          {
            std::cerr << call.parameters[i]->asString();
            if (i < call.parameters.size() - 1)
              std::cerr << ", ";
          }
          std::cerr << ")";
          // return value
          if(call.returnValue) {
            std::cerr << " -> " << call.returnValue->asString();
          }
          // error
          std::cerr  << " - " << Meta::getString(error) << std::endl;

          exit(EXIT_FAILURE);
        }
      }
    );
  }
  else {
    glbinding::setCallbackMask(glbinding::CallbackMask::None);
  }
}

void validate_program(GLuint program) {
  glValidateProgram(program);
  // check if validation was successfull
  GLint success = 0;
  glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
  if(success == 0) {
    // get log length
    GLint log_size = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);
    // get log
    GLchar* log_buffer = (GLchar*)malloc(sizeof(GLchar) * log_size);
    glGetProgramInfoLog(program, log_size, &log_size, log_buffer);
    // output errors
    output_log(log_buffer, "program nr. " + program);
    // free broken program
    glDeleteProgram(program);
    free(log_buffer);

    throw std::logic_error("Validation of program nr. " + program);
  }
}

GLint get_bound_VAO() {
  GLint array = -1;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &array);

  return array;
}

std::string file_name(std::string const& file_path) {
  return file_path.substr(file_path.find_last_of("/\\") + 1);
}

void output_log(GLchar const* log_buffer, std::string const& prefix) {
  std::string error{};
  std::istringstream error_stream{log_buffer};
  while(std::getline(error_stream, error)) {
    std::cerr << prefix << " - " << error << std::endl;
  }
}

std::string read_file(std::string const& name) {
  std::ifstream file_in{name};
  if(file_in) {
    std::string contents;
    file_in.seekg(0, std::ios::end);
    contents.resize(file_in.tellg());
    file_in.seekg(0, std::ios::beg);
    file_in.read(&contents[0], contents.size());
    file_in.close();
    return(contents);
  }
  else {
    std::cerr << "File \'" << name << "\' not found" << std::endl;
    
    throw std::invalid_argument(name);
  } 
}

std::size_t prims_to_verts(GLenum primitive, std::size_t count) {
  if (primitive == GL_POINTS || primitive == GL_LINE_LOOP) {
    return count;
  }
  else if (primitive == GL_LINES) {
    return count * 2;
  }
  else if (primitive == GL_LINE_STRIP) {
    return count + 1;
  }
  else if (primitive == GL_TRIANGLES) {
    return count * 3;
  }
  else if (primitive == GL_TRIANGLE_STRIP || primitive == GL_TRIANGLE_FAN) {
    return count + 2;
  }
  else {
    throw std::invalid_argument("primitive type not supported");
  }
  return 0;
}

std::size_t verts_to_prims(GLenum primitive, std::size_t count) {
  if (primitive == GL_POINTS || primitive == GL_LINE_LOOP) {
    return count;
  }
  else if (primitive == GL_LINES) {
    return count / 2;
  }
  else if (primitive == GL_LINE_STRIP) {
    return count - 1;
  }
  else if (primitive == GL_TRIANGLES) {
    return count / 3;
  }
  else if (primitive == GL_TRIANGLE_STRIP || primitive == GL_TRIANGLE_FAN) {
    return count - 2;
  }
  else {
    throw std::invalid_argument("primitive type not supported");
  }
  return 0;
}

std::size_t attribs_to_bytes(model::attrib_flag_t attribs) {
  std::size_t bytes = 0;
  for (auto const& supported_attribute : model::VERTEX_ATTRIBS) {
    if (supported_attribute.flag & attribs) {
      bytes += supported_attribute.size * supported_attribute.components;
    }
  }
  return bytes;
}

std::size_t pixel_bytes(GLenum channels, GLenum channel_type) {
  std::size_t channel_bytes = 0;
  if (channel_type == GL_BYTE || channel_type == GL_UNSIGNED_BYTE) {
    channel_bytes = 1;
  }
  else if (channel_type == GL_UNSIGNED_BYTE_3_3_2 || channel_type == GL_UNSIGNED_BYTE_2_3_3_REV) {
    return 1;
  }
  else if (channel_type == GL_SHORT || channel_type == GL_UNSIGNED_SHORT) {
    channel_bytes = 2;
  }
  // sized formats already specify full texture bytes
  else if( channel_type == GL_UNSIGNED_SHORT_5_6_5 || channel_type == GL_UNSIGNED_SHORT_5_6_5_REV
      || channel_type == GL_UNSIGNED_SHORT_4_4_4_4 || channel_type == GL_UNSIGNED_SHORT_4_4_4_4_REV
      || channel_type == GL_UNSIGNED_SHORT_5_5_5_1 || channel_type == GL_UNSIGNED_SHORT_1_5_5_5_REV) {
    return 2;
  }
  else if (channel_type == GL_INT || channel_type == GL_UNSIGNED_INT || channel_type == GL_FLOAT) {
    channel_bytes = 4;
  }
  // sized formats already specify full texture bytes
  else if (channel_type == GL_UNSIGNED_INT_8_8_8_8 || channel_type == GL_UNSIGNED_INT_8_8_8_8_REV
    || channel_type == GL_UNSIGNED_INT_10_10_10_2 || channel_type == GL_UNSIGNED_INT_2_10_10_10_REV
    || channel_type == GL_UNSIGNED_INT_5_9_9_9_REV || channel_type == GL_UNSIGNED_INT_10F_11F_11F_REV
    || channel_type == GL_UNSIGNED_INT_24_8) {
    return 4;
  }
  else if(channel_type == GL_FLOAT_32_UNSIGNED_INT_24_8_REV) {
    return 5;
  }
  else {
    throw std::logic_error("Channel type '" + Meta::getString(channel_type) + "' not supported.");
  }

  if (channels == GL_RED || channels == GL_GREEN || channels == GL_BLUE
    || channels == GL_RED_INTEGER || channels == GL_GREEN_INTEGER || channels == GL_BLUE_INTEGER
    || channels == GL_DEPTH_COMPONENT) {
    return channel_bytes * 1;
  }
  else if (channels == GL_RG || channels == GL_RG_INTEGER) {
    return channel_bytes * 2;
  }
  else if (channels == GL_RGB || channels == GL_BGR
    || channels == GL_RGB_INTEGER || channels == GL_BGR_INTEGER) {
    return channel_bytes * 3;
  }
  else if (channels == GL_RGBA || channels == GL_BGRA
    || channels == GL_RGBA_INTEGER || channels == GL_BGRA_INTEGER) {
    return channel_bytes * 4;
  }
  else if(channels == GL_STENCIL_INDEX || channels == GL_DEPTH_STENCIL) {
    throw std::logic_error("Format combination of '" + Meta::getString(channels) 
                                         + "' and '" + Meta::getString(channel_type) + "' not supported.");
  }
  else {
    throw std::logic_error("Pixel format '" + Meta::getString(channels) + "' not supported.");
  }
  return 0;
}

GLenum pixel_type(GLenum channels, GLenum channel_type) {
  // determine format of image data, internal format should be sized
  GLenum internal_format = GL_NONE;
  if (channels == GL_RED) {
    // unsigned normalized
    if (channel_type == GL_UNSIGNED_BYTE) {
      internal_format = GL_R8;
    }
    // signed normalized
    else if(channel_type == GL_BYTE) {
      internal_format = GL_R8_SNORM;
    }
    // signed integral
    else if (channel_type == GL_SHORT) {
      internal_format = GL_R16I;
    }
    else if (channel_type == GL_UNSIGNED_SHORT) {
    // unsigned integral
      internal_format = GL_R16UI;
    }
    else if (channel_type == GL_INT) {
      internal_format = GL_R32I;
    }
    else if (channel_type == GL_UNSIGNED_INT) {
      internal_format = GL_R32UI;
    }
    else if (channel_type == GL_HALF_FLOAT) {
      internal_format = GL_R16F;
    }
    else if (channel_type == GL_FLOAT) {
      internal_format = GL_R32F;
    }
    else {
      throw std::logic_error("Unsupported type " + Meta::getString(channel_type));
    }
  }
  else if (channels == GL_RG) {
    if (channel_type == GL_UNSIGNED_BYTE) {
      internal_format = GL_RG8;
    }
    else if(channel_type == GL_BYTE) {
      internal_format = GL_RG8_SNORM;
    }
    else if (channel_type == GL_SHORT) {
      internal_format = GL_RG16I;
    }
    else if (channel_type == GL_UNSIGNED_SHORT) {
      internal_format = GL_RG16UI;
    }
    else if (channel_type == GL_INT) {
      internal_format = GL_RG32I;
    }
    else if (channel_type == GL_UNSIGNED_INT) {
      internal_format = GL_RG32UI;
    }
    else if (channel_type == GL_HALF_FLOAT) {
      internal_format = GL_RG16F;
    }
    else if (channel_type == GL_FLOAT) {
      internal_format = GL_RG32F;
    }
    else {
      throw std::logic_error("Unsupported type " + Meta::getString(channel_type));
    }
  }
  else if (channels == GL_RGB) {
    if (channel_type == GL_UNSIGNED_BYTE) {
      internal_format = GL_RGB8;
    }
    else if (channel_type == GL_BYTE) {
      internal_format = GL_RGB8_SNORM;
    }
    else if (channel_type == GL_SHORT) {
      internal_format = GL_RGB16I;
    }
    else if (channel_type == GL_UNSIGNED_SHORT) {
      internal_format = GL_RGB16UI;
    }
    else if (channel_type == GL_INT) {
      internal_format = GL_RGB32I;
    }
    else if (channel_type == GL_UNSIGNED_INT) {
      internal_format = GL_RGB32UI;
    }
    else if (channel_type == GL_HALF_FLOAT) {
      internal_format = GL_RGB16F;
    }
    else if (channel_type == GL_FLOAT) {
      internal_format = GL_RGB32F;
    }
    else {
      throw std::logic_error("Unsupported type " + Meta::getString(channel_type));
    }
  }
  else if (channels == GL_RGBA) {
    if (channel_type == GL_UNSIGNED_BYTE) {
      internal_format = GL_RGBA8;
    }
    else if (channel_type == GL_BYTE) {
      internal_format = GL_RGBA8_SNORM;
    }
    else if (channel_type == GL_SHORT) {
      internal_format = GL_RGBA16I;
    }
    else if (channel_type == GL_UNSIGNED_SHORT) {
      internal_format = GL_RGBA16UI;
    }
    else if (channel_type == GL_INT) {
      internal_format = GL_RGBA32I;
    }
    else if (channel_type == GL_UNSIGNED_INT) {
      internal_format = GL_RGBA32UI;
    }
    else if (channel_type == GL_HALF_FLOAT) {
      internal_format = GL_RGBA16F;
    }
    else if (channel_type == GL_FLOAT) {
      internal_format = GL_RGBA32F;
    }
    else {
      throw std::logic_error("Unsupported type " + Meta::getString(channel_type));
    }
  }
  else if (channels == GL_DEPTH_COMPONENT) {
    internal_format = GL_DEPTH_COMPONENT24;
  }
  else {
    throw std::logic_error("Unsupported channels " + Meta::getString(channels));
  }
  return internal_format;
}


GLenum channel_num_to_type(unsigned num_components) {
  if (num_components == 1) {
    return GL_RED;
  }
  else if (num_components == 2) {
    return GL_RG;
  }
  else if (num_components == 3) {
    return GL_RGB;
  }
  else if (num_components == 4) {
    return GL_RGBA;
  }
  else {
    throw std::logic_error("Channel number " + std::to_string(num_components) + " not supported");
  }
}

GLenum channel_bytes_to_type(std::size_t num_bytes, bool sign) {
  if (num_bytes == 1) {
    if (sign) {
      return GL_BYTE;
    } 
    else  {
      return GL_UNSIGNED_BYTE;
    }
  }
  else if (num_bytes == 2) {
    if (sign) {
      return GL_SHORT;
    } 
    else  {
      return GL_UNSIGNED_SHORT;
    }
  }
  else if (num_bytes == 4) {
    if (sign) {
      return GL_INT;
    } 
    else  {
      return GL_UNSIGNED_INT;
    }
  }
  else {
    throw std::logic_error("Channel with " + std::to_string(num_bytes) + " bytes not supported");
  }
}

std::size_t type_to_bytes(GLenum type) {
  if (type == GL_BYTE || type == GL_UNSIGNED_BYTE || type == GL_BOOL) {
    return 1;
  }
  else if (type == GL_SHORT || type == GL_UNSIGNED_SHORT || type == GL_BOOL_VEC2) {
    return 2;
  }
  else if (type == GL_BOOL_VEC3) {
    return 3;
  }
  else if (type == GL_INT || type == GL_UNSIGNED_INT || type == GL_FLOAT) {
    return 4;
  }
  else if(type == GL_DOUBLE || type == GL_INT_VEC2 || type == GL_UNSIGNED_INT_VEC2 || type == GL_FLOAT_VEC2) {
    return 8;
  }
  else if(type == GL_INT_VEC3 || type == GL_UNSIGNED_INT_VEC3 || type == GL_FLOAT_VEC3) {
    return 12;
  }
  else if(type == GL_DOUBLE_VEC2 || type == GL_INT_VEC4 || type == GL_UNSIGNED_INT_VEC4 || type == GL_FLOAT_VEC4 || type == GL_FLOAT_MAT2) {
    return 16;
  }
  else if(type == GL_FLOAT_MAT3x2 || type == GL_FLOAT_MAT2x3) {
    return 24;
  }
  else if(type == GL_FLOAT_MAT4x2 || type == GL_FLOAT_MAT2x4 || type == GL_DOUBLE_MAT2) {
    return 32;
  }
  else if(type == GL_FLOAT_MAT3) {
    return 36;
  }
  else if(type == GL_DOUBLE_MAT3x2 || type == GL_DOUBLE_MAT2x3) {
    return 48;
  }
  else if(type == GL_DOUBLE_MAT4x2 || type == GL_DOUBLE_MAT2x4) {
    return 32;
  }
  else if(type == GL_FLOAT_MAT4) {
    return 64;
  }
  else if(type == GL_DOUBLE_MAT3) {
    return 72;
  }
  else if(type == GL_DOUBLE_MAT4) {
    return 144;
  }
  else {
    throw std::logic_error("Channel type '" + Meta::getString(type) + "' not supported.");
  }

  return 0;
}

};
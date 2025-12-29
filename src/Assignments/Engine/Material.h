#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

#include "Application/utils.h"

namespace xe {

    class Material {
    public:
        virtual void bind() = 0;

        virtual void unbind() {};
    };

    class ColorMaterial : public Material {
    public:
        ColorMaterial(const glm::vec4 color, GLuint texture, GLuint texture_unit) : color_(color), texture_(texture), texture_unit_(texture_unit) {}

        ColorMaterial(const glm::vec4 color, GLuint texture)
            : color_(color), texture_(texture), texture_unit_(0), use_texture_(true) {
        }

        ColorMaterial(const glm::vec4 color)
            : color_(color), texture_(0), texture_unit_(0), use_texture_(false) {
        }

        void bind();

        static void init();

        static GLuint program() { return shader_; }

        GLuint texture() { return texture_; }
        void set_texture(GLuint texture) { texture_ = texture; }

        GLuint texture_unit() { return texture_unit_; }
        void set_texture_unit(GLuint texture_unit) { texture_unit_ = texture_unit; }

    private:
        static GLuint shader_;
        static GLuint color_uniform_buffer_;
        static GLint uniform_map_Kd_location_;

        bool use_texture_;

        glm::vec4 color_;
        GLuint texture_;
        GLuint texture_unit_;

    };
    GLuint create_texture(const std::string& name);

}



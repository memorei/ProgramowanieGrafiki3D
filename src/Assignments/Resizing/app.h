#pragma once

#include <vector>

#include "Application/application.h"
#include "Application/utils.h"
#include "glad/gl.h"
#include "XeEngine/Mesh.h"
#include "glm/glm.hpp"

class SimpleShapeApplication : public xe::Application
{
public:
    SimpleShapeApplication(int width, int height, std::string title, bool debug) : Application(width, height, title, debug) {}

    void init() override;

    void frame() override;

    void framebuffer_resize_callback(int w, int h) override;

private:
    GLuint vao_;
    GLuint ebo_;
    GLuint vbo_;
    size_t index_count_;
    GLuint modifier_ubo_;
    GLuint transformations_ubo_;

    float fov_;
    float aspect_;
    float near_;
    float far_;

    glm::mat4 P_;
    glm::mat4 V_;
    GLuint u_pvm_buffer_;

    GLuint program_;

};
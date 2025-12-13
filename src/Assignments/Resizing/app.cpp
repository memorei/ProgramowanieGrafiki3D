#include "app.h"

#include <iostream>
#include <vector>
#include <tuple>

#include "Application/utils.h"
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define STB_IMAGE_IMPLEMENTATION 1
#include "3rdParty/stb/stb_image.h"

void SimpleShapeApplication::init() {

    auto program = xe::utils::create_program(
        { {GL_VERTEX_SHADER,   std::string(PROJECT_DIR) + "/shaders/base_vs.glsl"},
         {GL_FRAGMENT_SHADER, std::string(PROJECT_DIR) + "/shaders/base_fs.glsl"} });

    if (!program) {
        std::cerr << "Invalid program" << std::endl;
        exit(-1);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    float vertices[] = {
    // base (triangle 1)  yellow
    -0.5f, 0.0f, -0.5f,   1.f, 1.f, 0.f,
     0.5f, 0.0f, -0.5f,   1.f, 1.f, 0.f,
    -0.5f, 0.0f,  0.5f,   1.f, 1.f, 0.f,

    // base (triangle 2)  yellow
     0.5f, 0.0f, -0.5f,   1.f, 1.f, 0.f,
     0.5f, 0.0f,  0.5f,   1.f, 1.f, 0.f,
    -0.5f, 0.0f,  0.5f,   1.f, 1.f, 0.f,

    // side 1 (front)     red
    -0.5f, 0.0f,  0.5f,   1.f, 0.f, 0.f,
     0.5f, 0.0f,  0.5f,   1.f, 0.f, 0.f,
     0.0f, 1.0f,  0.0f,   1.f, 0.f, 0.f,

    // side 2 (right)     green
     0.5f, 0.0f,  0.5f,   0.f, 1.f, 0.f,
     0.5f, 0.0f, -0.5f,   0.f, 1.f, 0.f,
     0.0f, 1.0f,  0.0f,   0.f, 1.f, 0.f,

    // side 3 (back)      blue
     0.5f, 0.0f, -0.5f,   0.f, 0.f, 1.f,
    -0.5f, 0.0f, -0.5f,   0.f, 0.f, 1.f,
     0.0f, 1.0f,  0.0f,   0.f, 0.f, 1.f,

    // side 4 (left)      pink
    -0.5f, 0.0f, -0.5f,   1.f, 0.f, 1.f,
    -0.5f, 0.0f,  0.5f,   1.f, 0.f, 1.f,
     0.0f, 1.0f,  0.0f,   1.f, 0.f, 1.f,
    };

    std::vector<GLushort> indices = {
     0,  1,  2,
     3,  4,  5,
     6,  7,  8,
     9, 10, 11,
    12, 13, 14,
    15, 16, 17
    };
    index_count_ = indices.size();

    // ubo
    glGenBuffers(1, &modifier_ubo_);
    glBindBuffer(GL_UNIFORM_BUFFER, modifier_ubo_);
    glBufferData(GL_UNIFORM_BUFFER, 8 * sizeof(float), nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, modifier_ubo_);

    float strength = 0.7f;
    float color[3] = { 1.0f, 0.5f, 0.5f };

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float), &strength);
    glBufferSubData(GL_UNIFORM_BUFFER, 16, sizeof(color), color);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // projection, view
    int width, height;
    std::tie(width, height) = frame_buffer_size();

    aspect_ = (float)width / height;
    fov_ = glm::pi<float>() / 4.0f;
    near_ = 0.1f;
    far_ = 100.0f;

    P_ = glm::perspective(fov_, aspect_, near_, far_);
    V_ = glm::lookAt(
        glm::vec3(1.4f, -0.6f, 2.0f),
        glm::vec3(0.0f, 0.4f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glm::mat4 PVM = P_ * V_;

    // pvm ubo
    glGenBuffers(1, &u_pvm_buffer_);
    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, u_pvm_buffer_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &PVM);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // create VAO, VBO, EBO
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    //bind vao
    glBindVertexArray(vao_);

    // upload vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // upload index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);

    // vertex attribute: position (3 floats)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    // vertex attribute: color (3 floats)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    //unbind vao
    glBindVertexArray(0);

    // background color and viewport
    glClearColor(0.81f, 0.81f, 0.8f, 1.0f);
    auto [w, h] = frame_buffer_size();
    glViewport(0, 0, width, height);

    glUseProgram(program);
}

void SimpleShapeApplication::frame() {
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, index_count_, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);

    auto PVM = P_ * V_;
    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &PVM[0]);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SimpleShapeApplication::framebuffer_resize_callback(int w, int h) {
    Application::framebuffer_resize_callback(w, h);
    glViewport(0, 0, w, h);
    aspect_ = (float)w / h;
    P_ = glm::perspective(fov_, aspect_, near_, far_);
}

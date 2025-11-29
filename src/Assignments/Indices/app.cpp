#include "app.h"

#include <iostream>
#include <vector>
#include <tuple>

#include "Application/utils.h"

#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "spdlog/spdlog.h"
#include "XeEngine/ColorMaterial.h"

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
    glDisable(GL_CULL_FACE);

    float vertices[] = {
    // x,y,z            r,g,b
    -0.8f, -0.5f, 0.f,  0.f, 1.f, 0.f,
     0.8f, -0.5f, 0.f,  0.f, 1.f, 0.f,
    -0.8f,  0.2f, 0.f,  0.f, 1.f, 0.f,
     0.8f,  0.2f, 0.f,  0.f, 1.f, 0.f,

    -0.8f,  0.2f, 0.f,  1.f, 0.f, 0.f,
     0.8f,  0.2f, 0.f,  1.f, 0.f, 0.f,
     0.0f,  0.8f, 0.f,  1.f, 0.f, 0.f
    };

    size_t vertex_count = sizeof(vertices) / (6 * sizeof(float));

    std::vector<GLushort> indices = {
        0, 1, 3,
        0, 3, 2,
        4, 5, 6
    };

    index_count_ = indices.size();

    #if __APPLE__
    auto u_modifiers_index = glGetUniformBlockIndex(program, "Modifiers");
    if (u_modifiers_index == -1) {
        std::cerr << "Cannot find Modifiers uniform block in program" << std::endl;
    }
    else {
        glUniformBlockBinding(program, u_modifiers_index, 0);
    }
    #endif

    // create UBO for projection-view martrix
    glGenBuffers(1, &u_pvm_buffer_);
    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    glBufferData(GL_UNIFORM_BUFFER, 16 * sizeof(float), 0, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, u_pvm_buffer_);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    // create and configure camera
    set_camera(new Camera);

    camera()->look_at(
        glm::vec3(0, 1, 1),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0)
    );

    int w, h;
    std::tie(w, h) = frame_buffer_size();
    camera()->perspective(glm::pi<float>() / 2.0, (float)w / h, 0.1f, 100.f);
    // camera controller
    set_controler(new CameraControler(camera()));


    // create VAO, VBO, EBO
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    // upload index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);

    //unbind index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //bind vao
    glBindVertexArray(vao_);

    // upload vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    //bind index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

    // vertex attribute: position (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // vertex attribute: color (3 floats)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //unbind vao
    glBindVertexArray(0);

    // background color and viewport
    glClearColor(0.81f, 0.81f, 0.8f, 1.0f);
    glViewport(0, 0, w, h);

    // use shader program and bind UBO block
    glUseProgram(program);
    GLuint idx = glGetUniformBlockIndex(program, "Transformations");
    glUniformBlockBinding(program, idx, 1);

    program_ = program;
}

// rendering each frame
void SimpleShapeApplication::frame() {

    // clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // compute projection * view matrix and upload it to UBO
    auto PVM = camera()->projection() * camera()->view();
    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &PVM[0]);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // draw
    glUseProgram(program_);
    glBindVertexArray(vao_);

    glDrawElements(GL_TRIANGLES, index_count_, GL_UNSIGNED_SHORT, 0);

}


void SimpleShapeApplication::framebuffer_resize_callback(int w, int h) {
    Application::framebuffer_resize_callback(w, h);
    glViewport(0,0,w,h);
    camera()->set_aspect((float) w / h);
}

void SimpleShapeApplication::mouse_button_callback(int button, int action, int mods) {
    Application::mouse_button_callback(button, action, mods);

    if (controler_) {
        double x, y;
        glfwGetCursorPos(window_, &x, &y);

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
            controler_->LMB_pressed(x, y);

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
            controler_->LMB_released(x, y);
    }

}

void SimpleShapeApplication::cursor_position_callback(double x, double y) {
    Application::cursor_position_callback(x, y);
    if (controler_) {
        controler_->mouse_moved(x, y);
    }
}

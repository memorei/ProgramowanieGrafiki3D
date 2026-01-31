#include "app.h"

#include <iostream>
#include <vector>
#include <tuple>

#include "Application/utils.h"
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "camera_controler.h"

#include "Assignments/Engine/mesh_loader.h"

#define STB_IMAGE_IMPLEMENTATION 1
#include <Assignments/Engine/PhongMaterial.h>
#include "Assignments/Engine/ColorMaterial.h"

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

    xe::ColorMaterial::init();
	xe::PhongMaterial::init();

    // projection, view
    int width, height;
    std::tie(width, height) = frame_buffer_size();

    set_camera(new Camera);
    camera()->look_at(
        glm::vec3(1.4f, -0.6f, 2.0f),  //camera position
        glm::vec3(0.0f, 0.4f, 0.0f),    //centered on-screen position
        glm::vec3(0.0f, 1.0f, 0.0f)     //up axis
    );
    camera()->perspective(
        glm::pi<float>() / 4.0,         //fov
        (float)width / height,          //aspect
        0.1f,                           //near
        100.0f                          //far
    );

    set_controler(new CameraControler(camera()));

    glm::mat4 PVM = camera()->projection() * camera()->view();

    add_ambient(glm::vec3(0.3f, 0.3f, 0.3f));

    add_light(xe::PointLight(
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        5.0f,  
        2.0f
    ));

	// lights ubo
    glGenBuffers(1, &lights_ubo_);
    glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo_);
    glBufferData(
        GL_UNIFORM_BUFFER,
        32 + 48 * 24,
        nullptr,
        GL_DYNAMIC_DRAW
    );
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, lights_ubo_);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    // pvm ubo
    glGenBuffers(1, &u_pvm_buffer_);
    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    glBufferData(
        GL_UNIFORM_BUFFER,
        2 * sizeof(glm::mat4) + 3 * sizeof(glm::vec4),
        nullptr,
        GL_STATIC_DRAW
    );
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, u_pvm_buffer_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &PVM);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    auto square = xe::load_mesh_from_obj(
        std::string(ROOT_DIR) + "/Models/square.obj",
        std::string(ROOT_DIR) + "/Models"
    );

    add_submesh(square);

    // background color and viewport
    glClearColor(0.81f, 0.81f, 0.8f, 1.0f);
    auto [w, h] = frame_buffer_size();
    glViewport(0, 0, width, height);

    glUseProgram(program);
}

void SimpleShapeApplication::frame() {

    glm::mat4 VM = camera()->view();
    glm::mat4 PVM = camera()->projection() * VM;

    glm::mat3 R = glm::mat3(VM);
    glm::mat3 N = glm::mat3(
        glm::cross(R[1], R[2]),
        glm::cross(R[2], R[0]),
        glm::cross(R[0], R[1])
    );

    glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo_);

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec3), &ambient_);
    unsigned int n = (unsigned int)p_lights_.size();
    glBufferSubData(GL_UNIFORM_BUFFER, 16, sizeof(unsigned int), &n);

    size_t offset = 32;

    for (int i = 0; i < (int)p_lights_.size(); ++i) {
        auto& l = p_lights_[i];

        glm::vec4 pos_vs = VM * glm::vec4(l.position_in_ws, 1.0f);
        l.position_in_vs = glm::vec3(pos_vs);

        glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(xe::PointLight), &l);
        offset += sizeof(xe::PointLight);
    }

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &PVM);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &VM);

    glm::vec4 n0(N[0], 0.0f);
    glm::vec4 n1(N[1], 0.0f);
    glm::vec4 n2(N[2], 0.0f);

    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::vec4), &n0);
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4) + sizeof(glm::vec4), sizeof(glm::vec4), &n1);
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4) + 2 * sizeof(glm::vec4), sizeof(glm::vec4), &n2);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    for (auto m : meshes_)
        m->draw();
}


void SimpleShapeApplication::framebuffer_resize_callback(int w, int h) {
    Application::framebuffer_resize_callback(w, h);
    glViewport(0, 0, w, h);
    camera()->set_aspect((float)w / h);
}

void SimpleShapeApplication::mouse_button_callback(int button, int action, int mods) {
    Application::mouse_button_callback(button, action, mods);

    if (controler_) {
        double x, y;
        glfwGetCursorPos(window_, &x, &y);

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
            controler_->LMB_pressed((float)x, (float)y);

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
            controler_->LMB_released((float)x, (float)y);
    }
}

void SimpleShapeApplication::cursor_position_callback(double x, double y) {
    Application::cursor_position_callback(x, y);
    if (controler_) {
        controler_->mouse_moved((float)x, (float)y);
    }
}

void add_light(const xe::PointLight& p_light) {
    std::vector<xe::PointLight> p_lights_;
    p_lights_.push_back(p_light);
}

void add_ambient(glm::vec3 ambient) {
    glm::vec3 ambient_;
    ambient_ = ambient;
}

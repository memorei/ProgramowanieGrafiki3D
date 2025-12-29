#include "app.h"

#include <iostream>
#include <vector>
#include <tuple>

#include "Application/utils.h"
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "camera_controler.h"

#define STB_IMAGE_IMPLEMENTATION 1
#include "3rdParty/stb/stb_image.h"

void SimpleShapeApplication::init() {

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    xe::ColorMaterial::init();

    float vertices[] = {
    // base (triangle 1)
    -0.5f, 0.0f, -0.5f,0.0f, 0.0f,
     0.5f, 0.0f, -0.5f,0.0f, 0.0f,
    -0.5f, 0.0f,  0.5f,0.0f, 0.0f,

    // base (triangle 2)
     0.5f, 0.0f, -0.5f,0.0f, 0.0f,
     0.5f, 0.0f,  0.5f,0.0f, 0.0f,
    -0.5f, 0.0f,  0.5f,0.0f, 0.0f,

    // side 1 (front)
    -0.5f, 0.0f,  0.5f,0.0f, 0.0f,
     0.5f, 0.0f,  0.5f,0.0f, 0.0f,
     0.0f, 1.0f,  0.0f,0.0f, 0.0f,

    // side 2 (right)
     0.5f, 0.0f,  0.5f,0.0f, 0.0f,
     0.5f, 0.0f, -0.5f,0.0f, 0.0f,
     0.0f, 1.0f,  0.0f,0.0f, 0.0f,

    // side 3 (back)
     0.5f, 0.0f, -0.5f,0.0f, 0.0f,
     -0.5f, 0.0f, -0.5f,0.0f, 0.0f,
     0.0f, 1.0f,  0.0f,0.0f, 0.0f,

    // side 4 (left)
    -0.5f, 0.0f, -0.5f,0.0f, 0.0f,
    -0.5f, 0.0f,  0.5f,0.0f, 0.0f,
     0.0f, 1.0f,  0.0f,0.0f, 0.0f,
    };

    std::vector<GLushort> indices = {
     0,  1,  2,
     3,  4,  5,
     6,  7,  8,
     9, 10, 11,
    12, 13, 14,
    15, 16, 17
    };


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

    // pvm ubo
    glGenBuffers(1, &u_pvm_buffer_);
    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, u_pvm_buffer_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &PVM);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// pyramid mesh

    auto pyramid = new xe::Mesh;

    vertices_size = sizeof(vertices);
    size_t stride = 5 * sizeof(float);
    size_t indices_size = indices.size() * sizeof(GLushort);

	// vertices
    pyramid->allocate_vertex_buffer(vertices_size, GL_STATIC_DRAW);
    pyramid->load_vertices(0, vertices_size, vertices);
    pyramid->vertex_attrib_pointer(0, 3, GL_FLOAT, stride, 0);
    pyramid->vertex_attrib_pointer(1, 2, GL_FLOAT, stride, 3 * sizeof(float));


	// indices
    pyramid->allocate_index_buffer(indices_size, GL_STATIC_DRAW);
    pyramid->load_indices(0, indices_size, indices.data());

	// submeshes and materials

    auto yellow = new xe::ColorMaterial({ 1,1,0,1 });
    auto red = new xe::ColorMaterial({ 1,0,0,1 });
    auto green = new xe::ColorMaterial({ 0,1,0,1 });
    auto blue = new xe::ColorMaterial({ 0,0,1,1 });
    auto pink = new xe::ColorMaterial({ 1,0,1,1 });

    pyramid->add_submesh(0, 6, yellow);
    pyramid->add_submesh(6, 9, red);
    pyramid->add_submesh(9, 12, green);
    pyramid->add_submesh(12, 15, blue);
    pyramid->add_submesh(15, 18, pink);

    add_submesh(pyramid);

    // background color and viewport
    glClearColor(0.81f, 0.81f, 0.8f, 1.0f);
    auto [w, h] = frame_buffer_size();
    glViewport(0, 0, width, height);

}

void SimpleShapeApplication::frame() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    for (auto m : meshes_)
        m->draw();

    auto PVM = camera()->projection() * camera()->view();
    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &PVM[0]);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
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

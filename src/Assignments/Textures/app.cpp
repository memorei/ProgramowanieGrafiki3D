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
#include "spdlog/spdlog.h"

#define STB_IMAGE_IMPLEMENTATION 1
#include "3rdParty/stb/stb_image.h"
#include <Assignments/Engine/ColorMaterial.h>

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

    xe::ColorMaterial::init();

    float vertices[] = {
    // base (triangle 1)
    -0.5f, 0.0f, -0.5f,
     0.5f, 0.0f, -0.5f,
    -0.5f, 0.0f,  0.5f,

    // base (triangle 2)
     0.5f, 0.0f, -0.5f,
     0.5f, 0.0f,  0.5f,
    -0.5f, 0.0f,  0.5f,

    // side 1 (front)
    -0.5f, 0.0f,  0.5f,
     0.5f, 0.0f,  0.5f,
     0.0f, 1.0f,  0.0f,

    // side 2 (right)
     0.5f, 0.0f,  0.5f,
     0.5f, 0.0f, -0.5f,
     0.0f, 1.0f,  0.0f,

    // side 3 (back)
     0.5f, 0.0f, -0.5f,
     -0.5f, 0.0f, -0.5f,
     0.0f, 1.0f,  0.0f,

    // side 4 (left)
    -0.5f, 0.0f, -0.5f,
    -0.5f, 0.0f,  0.5f,
     0.0f, 1.0f,  0.0f
    };

    std::vector<GLushort> indices = {
     0,  1,  2,
     3,  4,  5,
     6,  7,  8,
     9, 10, 11,
    12, 13, 14,
    15, 16, 17
    };


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

    // texture

    stbi_set_flip_vertically_on_load(true);
    GLint image_width, image_height, image_channels;
    std::string texture_file = std::string(ROOT_DIR) + "/Models/multicolor.png";
    auto img = stbi_load(texture_file.c_str(), &image_width, &image_height, &image_channels, 0);

    if (!img) {
        spdlog::warn("Could not read image from file `{}'", texture_file);
    }


    GLenum format;
    if (image_channels == 3)
        format = GL_RGB;
    else if (image_channels == 4) {
        format = GL_RGBA;
    }

    GLuint texture_handle;
    glGenTextures(1, &texture_handle);
    glBindTexture(GL_TEXTURE_2D, texture_handle);

    glTexImage2D(
        GL_TEXTURE_2D,      // target: target texture
        0,                  // level: level-of-detail (0 = base image level)
        GL_RGB,             // internalformat: number of color components
        image_width,        // width
        image_height,       // height
        0,                  // border
        format,             // format: format of pixel data
        GL_UNSIGNED_BYTE,   // type: data type of pixel data
        img                 // pixels: data
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(img);

    vertices_size = sizeof(vertices);
    size_t stride = 3 * sizeof(float);
    size_t indices_size = indices.size() * sizeof(GLushort);

	// vertices
    pyramid->allocate_vertex_buffer(vertices_size, GL_STATIC_DRAW);
    pyramid->load_vertices(0, vertices_size, vertices);
    pyramid->vertex_attrib_pointer(0, 3, GL_FLOAT, stride, 0);

	// indices
    pyramid->allocate_index_buffer(indices_size, GL_STATIC_DRAW);
    pyramid->load_indices(0, indices_size, indices.data());

    auto pyramid_texture = xe::load_mesh_from_obj(std::string(ROOT_DIR) + "/Models/pyramid.obj",
        std::string(ROOT_DIR) + "/Models");

    add_submesh(pyramid_texture);

    // background color and viewport
    glClearColor(0.81f, 0.81f, 0.8f, 1.0f);
    auto [w, h] = frame_buffer_size();
    glViewport(0, 0, width, height);

    glUseProgram(program);
}

void SimpleShapeApplication::frame() {
    
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

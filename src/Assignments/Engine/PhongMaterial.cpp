#include "glm/gtx/string_cast.hpp"

#include "PhongMaterial.h"

#include "spdlog/spdlog.h"

#include "3rdParty/stb/stb_image.h"

namespace xe {

GLuint PhongMaterial::color_uniform_buffer_ = 0u;
GLuint PhongMaterial::shader_ = 0u;
GLint  PhongMaterial::uniform_map_Kd_location_ = 0;

void PhongMaterial::bind() {

    glUseProgram(program());

    bool use_map_Kd = texture_ > 0;

    if (use_map_Kd) {
        glUniform1i(uniform_map_Kd_location_, texture_unit_);
        glActiveTexture(GL_TEXTURE0 + texture_unit_);
        glBindTexture(GL_TEXTURE_2D, texture_);
    }

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, color_uniform_buffer_);
    glBindBuffer(GL_UNIFORM_BUFFER, color_uniform_buffer_);

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4), &color_[0]);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::vec4), sizeof(bool), &use_map_Kd);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

}



void PhongMaterial::init() {

    auto program = xe::utils::create_program(
        { {GL_VERTEX_SHADER,   std::string(PROJECT_DIR) + "/shaders/phong_vs.glsl"},
            {GL_FRAGMENT_SHADER, std::string(PROJECT_DIR) + "/shaders/phong_fs.glsl"} });
    if (!program) {
        std::cerr << "Invalid program" << std::endl;
        exit(-1);
    }

    shader_ = program;

    auto lights_index = glGetUniformBlockIndex(shader_, "Lights");
    glUniformBlockBinding(shader_, lights_index, 2);

    uniform_map_Kd_location_ = glGetUniformLocation(shader_, "map_Kd");
    if (uniform_map_Kd_location_ == -1) {
        spdlog::warn("Cannot get uniform {} location", "map_Kd");
    } 
    glGenBuffers(1, &color_uniform_buffer_);

    glBindBuffer(GL_UNIFORM_BUFFER, color_uniform_buffer_);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) + sizeof(bool), nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0u);
#if __APPLE__
    auto u_modifiers_index = glGetUniformBlockIndex(program, "Color");
    if (u_modifiers_index == -1) {
        std::cerr << "Cannot find Color uniform block in program" << std::endl;
    }
    else {
        glUniformBlockBinding(program, u_modifiers_index, 0);
    }
#endif

#if __APPLE__
    auto u_transformations_index = glGetUniformBlockIndex(program, "Transformations");
    if (u_transformations_index == -1) {
        std::cerr << "Cannot find Transformations uniform block in program" << std::endl;
    }
    else {
        glUniformBlockBinding(program, u_transformations_index, 1);
    }
#endif

}

}
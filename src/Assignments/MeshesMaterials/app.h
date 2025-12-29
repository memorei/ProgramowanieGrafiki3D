#pragma once

#include <vector>

#include "Application/application.h"
#include "Application/utils.h"
#include "camera.h"
#include "glad/gl.h"
#include "glm/glm.hpp"
#include "Engine/Mesh.h"


class CameraControler;

class SimpleShapeApplication : public xe::Application
{
public:
    SimpleShapeApplication(int width, int height, std::string title, bool debug) : Application(width, height, title, debug) {}

    void init() override;

    void frame() override;

    void framebuffer_resize_callback(int w, int h) override;

    void scroll_callback(double xoffset, double yoffset) override {
        Application::scroll_callback(xoffset, yoffset);
        camera()->zoom(yoffset / 30.0f);
    }

	// camera and controler 
    void set_camera(Camera* camera) { camera_ = camera; }
    Camera* camera() { return camera_; }

    void set_controler(CameraControler* controler) { controler_ = controler; }

    void add_submesh(xe::Mesh* mesh) {
        meshes_.push_back(mesh);
    }

    ~SimpleShapeApplication() {
        if (camera_) {
            delete camera_;
        }
        if (controler_) {
            delete controler_;
        }
        if (!meshes_.empty()) {
            for (auto p : meshes_)
            {
                delete p;
            }
            meshes_.clear();
        }
    }
    void mouse_button_callback(int button, int action, int mods) override;
    void cursor_position_callback(double x, double y) override;    

private:

    GLuint vao_;
    GLuint ebo_;
    GLuint vbo_;
    size_t index_count_;
    GLuint modifier_ubo_;
    size_t vertices_size;

    GLuint u_pvm_buffer_;

    Camera* camera_;
    CameraControler* controler_;

    std::vector<xe::Mesh*> meshes_;

    GLuint program_;

};
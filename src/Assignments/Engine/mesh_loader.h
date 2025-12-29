#pragma once

#include <string>
#include <memory>

namespace xe {
    class Mesh;

    Mesh *load_mesh_from_obj(std::string path, std::string mtl_dir);
}
#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

#include "Application/utils.h"

#include <string>
#include "Material.h"
#include <stb/stb_image.h>

namespace xe {

GLuint create_texture(const std::string& filename);

}




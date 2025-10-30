#include "shader.h"

#include <cstdint>
#include <spdlog/spdlog.h>

using namespace render::shader;

void render::shader::use(ShaderProgramData data)
{
    glUseProgram(data.programId);
}

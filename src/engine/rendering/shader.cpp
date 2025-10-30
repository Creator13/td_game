#include "shader.h"

using namespace render::shader;

void render::shader::use(ShaderProgramData data)
{
    glUseProgram(data.programId);
}

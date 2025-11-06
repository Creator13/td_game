#include "shader.h"

void graphics::shader::use(ShaderProgramData data)
{
    glUseProgram(data.programId);
}

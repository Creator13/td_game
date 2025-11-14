#include "shader.h"

#include <glad/glad.h>

void graphics::shader::use(ShaderProgramData data)
{
    glUseProgram(data.programId);
}

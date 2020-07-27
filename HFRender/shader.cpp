#include "Shader.h"
#include <fstream>
#include <sstream>

#include <iostream>

Shader* Shader::Use()
{
    glUseProgram(m_id);
    return this;
}

bool Shader::Init(const std::string& vs_path, const std::string& fs_path, const std::string& gs_path)
{
    // 1. Retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    try
    {
        // Open files
        std::ifstream vertexShaderFile(vs_path);
        std::ifstream fragmentShaderFile(fs_path);
        if (vertexShaderFile.fail() || fragmentShaderFile.fail())
        {
            std::cout << "ERROR::SHADER: Failed to open vertex or fragment shader files" << std::endl;
            return false;
        }

        std::stringstream vShaderStream, fShaderStream;
        // Read file's buffer contents into streams
        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();
        // close file handlers
        vertexShaderFile.close();
        fragmentShaderFile.close();
        // Convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        // If geometry shader path is present, also load a geometry shader
        if (!gs_path.empty())
        {
            std::ifstream geometryShaderFile(gs_path);
            if (geometryShaderFile.fail())
            {
                std::cout << "ERROR::SHADER: Failed to open geometry shader files" << std::endl;
                return false;
            }

            std::stringstream gShaderStream;
            gShaderStream << geometryShaderFile.rdbuf();
            geometryShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    }
    catch (std::exception e)
    {
        std::cout << "ERROR::SHADER: Failed to read shader files" << std::endl;
    }

    const GLchar* vShaderCode = vertexCode.c_str();
    const GLchar* fShaderCode = fragmentCode.c_str();
    const GLchar* gShaderCode = geometryCode.c_str();

    GLuint sVertex, sFragment, gShader;

    // Vertex Shader
    sVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(sVertex, 1, &vShaderCode, NULL);
    glCompileShader(sVertex);
    if (!checkCompileErrors(sVertex, "VERTEX"))
    {
        return false;
    }

    // Fragment Shader
    sFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(sFragment, 1, &fShaderCode, NULL);
    glCompileShader(sFragment);
    if (!checkCompileErrors(sFragment, "FRAGMENT"))
    {
        return false;
    }

    // If geometry shader source code is given, also compile geometry shader
    if (!geometryCode.empty())
    {
        gShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(gShader, 1, &gShaderCode, NULL);
        glCompileShader(gShader);
        if (!checkCompileErrors(gShader, "GEOMETRY"))
        {
            return false;
        }
    }

    // Shader Program
    this->m_id = glCreateProgram();
    glAttachShader(this->m_id, sVertex);
    glAttachShader(this->m_id, sFragment);
    if (!geometryCode.empty())
        glAttachShader(this->m_id, gShader);
    glLinkProgram(this->m_id);
    if (!checkCompileErrors(this->m_id, "PROGRAM"))
    {
        return false;
    }

    // Delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(sVertex);
    glDeleteShader(sFragment);
    if (!geometryCode.empty())
        glDeleteShader(gShader);

    return true;
}

void Shader::SetBool(const std::string& name, bool value) const
{
    glUniform1i(GetUniformLocation(name), (int)value);
    GL_CHECK_ERROR;
}

void Shader::SetFloat(const std::string& name, GLfloat value) const
{
    glUniform1f(GetUniformLocation(name), value);
    GL_CHECK_ERROR;
}
void Shader::SetInteger(const std::string& name, GLint value) const
{
    glUniform1i(GetUniformLocation(name), value);
    GL_CHECK_ERROR;
}
void Shader::SetVector2f(const std::string& name, GLfloat x, GLfloat y) const
{
    glUniform2f(GetUniformLocation(name), x, y);
    GL_CHECK_ERROR;
}
void Shader::SetVector2f(const std::string& name, const glm::vec2& value) const
{
    glUniform2f(GetUniformLocation(name), value.x, value.y);
    GL_CHECK_ERROR;
}
void Shader::SetVector3f(const std::string& name, GLfloat x, GLfloat y, GLfloat z) const
{
    glUniform3f(GetUniformLocation(name), x, y, z);
    GL_CHECK_ERROR;
}
void Shader::SetVector3f(const std::string& name, const glm::vec3& value) const
{
    glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
    GL_CHECK_ERROR;
}
void Shader::SetVector4f(const std::string& name, GLfloat x, GLfloat y, GLfloat z, GLfloat w) const
{
    glUniform4f(GetUniformLocation(name), x, y, z, w);
    GL_CHECK_ERROR;
}
void Shader::SetVector4f(const std::string& name, const glm::vec4& value) const
{
    glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
    GL_CHECK_ERROR;
}
void Shader::SetMatrix4(const std::string& name, const glm::mat4& matrix) const
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
    GL_CHECK_ERROR;
}

void Shader::SetMatrix3(const std::string& name, const glm::mat3& matrix) const
{
    glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
    GL_CHECK_ERROR;
}

void Shader::SetMatrix2(const std::string& name, const glm::mat2& matrix) const
{
    glUniformMatrix2fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
    GL_CHECK_ERROR;
}

void Shader::SetIntegerV(const std::string& name, const GLint* values, int num) const
{
    glUniform1iv(GetUniformLocation(name), num, values);
    GL_CHECK_ERROR;
}

void Shader::SetFloatV(const std::string& name, const GLfloat* values, int num) const
{
    glUniform1fv(GetUniformLocation(name), num, values);
    GL_CHECK_ERROR;
}

void Shader::Set2FloatV(const std::string& name, const GLfloat values[][2], int num) const
{
    glUniform2fv(GetUniformLocation(name), num, (GLfloat*)values);
    GL_CHECK_ERROR;
}

void Shader::SetTexture(const std::string& name, const ITexturePtr& texture)const
{
    texture->Activate();
    glUniform1i(GetUniformLocation(name), texture->GetTextureUnit());
}

bool Shader::checkCompileErrors(GLuint object, std::string type)
{
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(object, 1024, NULL, infoLog);
            std::cout << "| ERROR::SHADER: Compile-time error: Type: " << type << "\n"
                << infoLog << "\n -- --------------------------------------------------- -- "
                << std::endl;
        }
    }
    else
    {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(object, 1024, NULL, infoLog);
            std::cout << "| ERROR::Shader: Link-time error: Type: " << type << "\n"
                << infoLog << "\n -- --------------------------------------------------- -- "
                << std::endl;
        }
    }
    return success;
}
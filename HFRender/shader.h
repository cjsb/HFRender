#pragma once
#include <unordered_map>
#include <memory>
#include "opengl_common.h"
#include "texture.h"

class Shader
{
public:

    // Constructor
    Shader(){ }

    ~Shader()
    {
        glDeleteProgram(m_id);
    }

    // Sets the current shader as active
    Shader* Use();
    GLuint GetId() { return m_id; }
    // Compiles the shader from given source code
    bool Init(const std::string& vs_path, const std::string& fs_path, const std::string& gs_path); // Note: geometry source code is optional 
    // Utility functions
    void SetFloat(const std::string& name, GLfloat value)const;
    void SetBool(const std::string& name, bool value) const;
    void SetInteger(const std::string& name, GLint value)const;
    void SetVector2f(const std::string& name, GLfloat x, GLfloat y)const;
    void SetVector2f(const std::string& name, const glm::vec2& value)const;
    void SetVector3f(const std::string& name, GLfloat x, GLfloat y, GLfloat z)const;
    void SetVector3f(const std::string& name, const glm::vec3& value)const;
    void SetVector4f(const std::string& name, GLfloat x, GLfloat y, GLfloat z, GLfloat w)const;
    void SetVector4f(const std::string& name, const glm::vec4& value)const;
    void SetMatrix4(const std::string& name, const glm::mat4& matrix)const;
    void SetMatrix3(const std::string& name, const glm::mat3& matrix)const;
    void SetMatrix2(const std::string& name, const glm::mat2& matrix)const;
    void SetIntegerV(const std::string& name, const GLint* values, int num)const;
    void SetFloatV(const std::string& name, const GLfloat* values, int num)const;
    void Set2FloatV(const std::string& name, const GLfloat values[][2], int num)const;
    void SetTexture(const std::string& name, const ITexturePtr& texture)const;
    void SetImage(const std::string& name, const ITexturePtr& texture)const;
    
private:
    // Checks if compilation or linking failed and if so, print the error logs
    bool checkCompileErrors(GLuint object, std::string type);

    inline GLuint GetUniformLocation(const std::string& name) const
    {
        auto it = m_uniform_loc_map.find(name);
        if (it != m_uniform_loc_map.end())
        {
            return it->second;
        }
        auto ret = m_uniform_loc_map.emplace(name, glGetUniformLocation(m_id, name.c_str()));
        GL_CHECK_ERROR;
        return ret.first->second;
    }

    mutable std::unordered_map<std::string, GLint> m_uniform_loc_map;

    GLuint m_id;
};

typedef std::shared_ptr<Shader> ShaderPtr;


class ShaderLoader
{
public:
    static ShaderLoader* Instance() { return s_inst; };
    ShaderPtr LoadShader(const std::string& vs_path, const std::string& fs_path, const std::string& gs_path);

private:
    std::unordered_map<std::string, ShaderPtr> m_shader_cache;
    static ShaderLoader* s_inst;
};
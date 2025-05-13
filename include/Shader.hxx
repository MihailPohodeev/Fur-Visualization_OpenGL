#ifndef _SHADER_H_
#define _SHADER_H_

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Shader
{
public:
    GLuint ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr,
           const char* tcsPath = nullptr, const char* tesPath = nullptr)
    {
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::string geometryCode;
        std::string tcsCode;
        std::string tesCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        std::ifstream gShaderFile;
        std::ifstream tcsShaderFile;
        std::ifstream tesShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        gShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        tcsShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        tesShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try 
        {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);            
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode   = vShaderStream.str();
            fragmentCode = fShaderStream.str();
            
            if (geometryPath)
            {
              gShaderFile.open(geometryPath);
              std::stringstream gShaderStream;
              gShaderStream << gShaderFile.rdbuf();
              gShaderFile.close();
              geometryCode = gShaderStream.str();
            }
            
            if (tcsPath && tesPath)
            {
              tcsShaderFile.open(tcsPath);
              tesShaderFile.open(tesPath);
              std::stringstream tcsShaderStream;
              std::stringstream tesShaderStream;
              tcsShaderStream << tcsShaderFile.rdbuf();
              tesShaderStream << tesShaderFile.rdbuf();
              tcsShaderFile.close();
              tesShaderFile.close();
              tcsCode = tcsShaderStream.str();
              tesCode = tesShaderStream.str();
            }
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char * fShaderCode = fragmentCode.c_str();
        const char * gShaderCode = geometryCode.c_str();
        const char * tcsShaderCode = tcsCode.c_str();
        const char * tesShaderCode = tesCode.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment, geometry, tcs, tes;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        
        if (geometryPath)
        {
          geometry = glCreateShader(GL_GEOMETRY_SHADER);
          glShaderSource(geometry, 1, &gShaderCode, NULL);
          glCompileShader(geometry);
          checkCompileErrors(geometry, "GEOMETRY");
        }
        
        if (tcsPath)
        {
          tcs = glCreateShader(GL_TESS_CONTROL_SHADER);
          glShaderSource(tcs, 1, &tcsShaderCode, NULL);
          glCompileShader(tcs);
          checkCompileErrors(tcs, "TESS_CONTROL_SHADER");
        }
        
        if (tesPath)
        {
          tes = glCreateShader(GL_TESS_EVALUATION_SHADER);
          glShaderSource(tes, 1, &tesShaderCode, NULL);
          glCompileShader(tes);
          checkCompileErrors(tes, "TESS_EVALUATION_SHADER");
        }
        
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        if (geometryPath)
          glAttachShader(ID, geometry);
        if (tcsPath)
          glAttachShader(ID, tcs);
        if (tesPath)
          glAttachShader(ID, tes);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (geometryPath)
          glDeleteShader(geometry);
        if (tcsPath)
          glDeleteShader(tcs);
        if (tesPath)
          glDeleteShader(tes);
    }
    // activate the shader
    // ------------------------------------------------------------------------
    void use() 
    { 
        glUseProgram(ID); 
    }
    
    GLuint getProgramID() const
    {
      return ID;
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string &name, bool value) const
    {         
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const
    { 
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const
    { 
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
    }
    
    void setVec3( const std::string& name, const glm::vec3& vec )
    {
        glUniform3f( glGetUniformLocation( ID, name.c_str() ), vec.x, vec.y, vec.z );
    }
    
    void setVec4( const std::string& name, const glm::vec4& vec )
    {
        glUniform4f( glGetUniformLocation( ID, name.c_str() ), vec.x, vec.y, vec.z, vec.w );
    }
    
    void setMat4( const std::string& name, const glm::mat4& mat )
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};
#endif

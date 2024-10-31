#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
    public:
        unsigned int ID;
        Shader(const char* vertexPath, const char* fragmentPath){
            std::string vertexCode;
            std::string fragmentCode;
            std::ifstream vShaderFile;
            std::ifstream fShaderFile; 

            vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
            fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);

            try{
                vShaderFile.open(vertexPath);
                fShaderFile.open(fragmentPath);
                std::stringstream vShaderStream, fShaderStream;
                vShaderStream  << vShaderFile.rdbuf();
                fShaderStream << fShaderFile.rdbuf();
                vShaderFile.close();
                fShaderFile.close();
                vertexCode = vShaderStream.str();
                fragmentCode = fShaderStream.str();
            }
            catch(std::ifstream::failure e){
                std::cout << "Shader file not read successfully" << e.what() << std::endl;
            }
            const char* vShaderCode = vertexCode.c_str();
            const char* fShaderCode = fragmentCode.c_str();

            // Create vertex shader
            unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertexShader, 1, &vShaderCode, NULL);
            glCompileShader(vertexShader);
            checkCompileErrors(vertexShader, "VERTEX");

            // Create fragment shader
            unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
            glCompileShader(fragmentShader);
            checkCompileErrors(fragmentShader, "FRAGMENT");

            ID = glCreateProgram();
            glAttachShader(ID, vertexShader);
            glAttachShader(ID, fragmentShader);
            glLinkProgram(ID);
            checkCompileErrors(ID, "PROGRAM");

            // Delete shaders
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
        }

        void use(){
            glUseProgram(ID);
        }

        void setBool(const std::string &name, bool value) const{
            glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
        }
        void setInt(const std::string &name, int value) const{
            glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
        }
        void setFloat(const std::string &name, float value) const{
            glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
        }
        void setMat4(const std::string &name, const glm::mat4 &mat) const
        {
            glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }
    
    private:
        void checkCompileErrors(unsigned int shader, std::string type){

            int success;
            char infoLog[1024];
            if(type != "PROGRAM"){
                glGetShaderiv(shader, GL_LINK_STATUS, &success);
                if(!success){
                    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                    std::cout << "Compilation error of type " << type << "\n" << infoLog << std::endl;
                }
            }  
            else{
                glGetProgramiv(shader, GL_LINK_STATUS, &success);
                if(!success){
                    glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                    std::cout << "Program linking error of type " << type << "\n" << infoLog << std::endl;
                }
            }
        }
};

#endif

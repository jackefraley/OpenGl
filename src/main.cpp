#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>

#include <iostream>
#include <fstream>

// Varaibles
int width = 800;
int height = 600;
float pi = 3.14;

// Function prototype
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

int main(){

    glfwInit();
    
    // Specify OpenGl 3.3 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Create OpenGl window with core profile
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // GLFW forward compatibility for macos 
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Create glfw window with size: 800x600
    GLFWwindow * window = glfwCreateWindow(width, height, "LearnOpenGl", NULL, NULL);

    // Window error message
    if(window == NULL){
        std::cout << "Failed to create window";
        glfwTerminate();
        return -1;
    }

    // Make window current context
    glfwMakeContextCurrent(window);

    // Resize viewport when window is resized
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Glad error message
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD";
        return -1;
    }

    Shader shader("3.3.shader.vs", "3.3.shader.fs");

    // Set viewport to size of window
    glfwGetFramebufferSize(window, &width, &height);

    // Set the viewport to window size
    glViewport(0, 0, width, height);

    // Coordinates for the vertices (center origin)
float vertices[] = {
    // Front Face (-Z), Side Texture
    -0.5f, -0.5f, -0.5f,  0.25f, 0.75f, // Bottom-left
     0.5f, -0.5f, -0.5f,  0.50f, 0.75f, // Bottom-right
     0.5f,  0.5f, -0.5f,  0.50f, 1.00f, // Top-right
     0.5f,  0.5f, -0.5f,  0.50f, 1.00f, // Top-right
    -0.5f,  0.5f, -0.5f,  0.25f, 1.00f, // Top-left
    -0.5f, -0.5f, -0.5f,  0.25f, 0.75f, // Bottom-left

    // Back Face (+Z), Side Texture
    -0.5f, -0.5f,  0.5f,  0.25f, 0.75f, // Bottom-left
     0.5f, -0.5f,  0.5f,  0.50f, 0.75f, // Bottom-right
     0.5f,  0.5f,  0.5f,  0.50f, 1.00f, // Top-right
     0.5f,  0.5f,  0.5f,  0.50f, 1.00f, // Top-right
    -0.5f,  0.5f,  0.5f,  0.25f, 1.00f, // Top-left
    -0.5f, -0.5f,  0.5f,  0.25f, 0.75f, // Bottom-left

    // Left Face (-X), Side Texture
    -0.5f,  0.5f,  0.5f,  0.25f, 1.00f, // Top-front
    -0.5f,  0.5f, -0.5f,  0.50f, 1.00f, // Top-back
    -0.5f, -0.5f, -0.5f,  0.50f, 0.75f, // Bottom-back
    -0.5f, -0.5f, -0.5f,  0.50f, 0.75f, // Bottom-back
    -0.5f, -0.5f,  0.5f,  0.25f, 0.75f, // Bottom-front
    -0.5f,  0.5f,  0.5f,  0.25f, 1.00f, // Top-front

    // Right Face (+X), Side Texture
     0.5f,  0.5f,  0.5f,  0.25f, 1.00f, // Top-front
     0.5f,  0.5f, -0.5f,  0.50f, 1.00f, // Top-back
     0.5f, -0.5f, -0.5f,  0.50f, 0.75f, // Bottom-back
     0.5f, -0.5f, -0.5f,  0.50f, 0.75f, // Bottom-back
     0.5f, -0.5f,  0.5f,  0.25f, 0.75f, // Bottom-front
     0.5f,  0.5f,  0.5f,  0.25f, 1.00f, // Top-front

    // Bottom Face (-Y), Bottom Texture
    -0.5f, -0.5f, -0.5f,  0.50f, 0.75f, // Back-left
     0.5f, -0.5f, -0.5f,  0.75f, 0.75f, // Back-right
     0.5f, -0.5f,  0.5f,  0.75f, 0.50f, // Front-right
     0.5f, -0.5f,  0.5f,  0.75f, 0.50f, // Front-right
    -0.5f, -0.5f,  0.5f,  0.50f, 0.50f, // Front-left
    -0.5f, -0.5f, -0.5f,  0.50f, 0.75f, // Back-left

    // Top Face (+Y), Top Texture
    -0.5f,  0.5f, -0.5f,  0.00f, 1.00f, // Back-left
     0.5f,  0.5f, -0.5f,  0.25f, 1.00f, // Back-right
     0.5f,  0.5f,  0.5f,  0.25f, 0.75f, // Front-right
     0.5f,  0.5f,  0.5f,  0.25f, 0.75f, // Front-right
    -0.5f,  0.5f,  0.5f,  0.00f, 0.75f, // Front-left
    -0.5f,  0.5f, -0.5f,  0.00f, 1.00f  // Back-left
};



    glm::vec3 cubePositions[] = {
        glm::vec3(  1.0f, -1.0f,  0.0f),
        glm::vec3(  1.0f, -1.0f, -1.0f),
        glm::vec3(  1.0f, -1.0f, -2.0f),
        glm::vec3(  0.0f, -1.0f,  0.0f),
        glm::vec3(  0.0f, -1.0f, -1.0f),
        glm::vec3(  0.0f, -1.0f, -2.0f),
        glm::vec3( -1.0f, -1.0f,  0.0f),
        glm::vec3( -1.0f, -1.0f, -1.0f),
        glm::vec3( -1.0f, -1.0f, -2.0f)       
    };

    // Four indices for rectangle
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &VBO);

    // Square
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind vertex array and buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 

    float borderColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    unsigned int shapeTexture;

    glGenTextures(1, &shapeTexture);
    glBindTexture(GL_TEXTURE_2D, shapeTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);
    unsigned char *data = stbi_load("textureMap.png", &width, &height, &nrChannels, 0);
    
    if(data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else{
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);
    shader.use();
    shader.setInt("shapeTexture", 0);

    // Turn on wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Keep displaying window while open
    while(!glfwWindowShouldClose(window)){
        // Process input
        processInput(window);

        // Rendering commands
        glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shapeTexture);

        // Use the shader program (vertex and fragment)
        shader.use();

        float time = glfwGetTime() * 50.0f;

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        glm::mat4 proj = glm::mat4(1.0f);
        proj = glm::perspective(pi/2, aspectRatio, 0.1f, 100.0f);

        int viewLoc = glGetUniformLocation(shader.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        shader.setMat4("proj", proj);

        // Bind the VAO vertex array and draw square
        glBindVertexArray(VAO);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        for(int x = 0; x < 9; x++){
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[x]);
            //model = glm::rotate(model, pi/4, glm::vec3(1.0f, 0.0f, 0.0f));

            int modelLoc = glGetUniformLocation(shader.ID, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Check and call events, swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Terminate window and close program
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    // Set viewport to be the size given
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
}
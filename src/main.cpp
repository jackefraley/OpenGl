#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <shader.h>

#include <iostream>
#include <fstream>

// Varaibles
int width, height;

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

    // Create glfw window with size: 800x600
    GLFWwindow * window = glfwCreateWindow(800, 600, "LearnOpenGl", NULL, NULL);

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


    // Calculate the aspect ratio
    //float aspect = (float)width / (float)height;

    // Coordinates for the vertices (center origin)
    float square[] = {
         0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // Top Right
         0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, // Bottom Right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, // Bottom Left
        -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f// Top Left
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind vertex array and buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 

    // Turn on wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Keep displaying window while open
    while(!glfwWindowShouldClose(window)){
        // Process input
        processInput(window);

        // Rendering commands
        glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the shader program (vertex and fragment)
        shader.use();
        // Bind the VAO vertex array and draw square
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Draw the triangles

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




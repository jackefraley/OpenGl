#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>

#include <random>
#include <string>
#include <iostream>
#include <fstream>

// Varaibles
int width = 800;
int height = 600;
float pi = 3.14;

// Function prototype
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);

glm::vec3 cameraPos = glm::vec3(0.0f, 6.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 worldFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 400, lastY = 300;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

    struct blockData {
        bool occupied;
    };

    blockData worldArray[12][12][12] = {{{{0}}}};

    std::srand(static_cast<unsigned>(std::time(0)));

    for (int i = 1; i < 11; i++) {
        for(int j = 1; j < 11; j++){
            for(int k = 1; k < 11; k++){
                worldArray[i][j][k].occupied = 1;//static_cast<bool>(std::rand() % 2);
            }
        }
    }

    // Vertex data
    float vertices[] = {
        // Front Face (-Z), Texture 2
         -0.5f, -0.5f, -0.5f,  0.25f, 0.75f, // Bottom-left
          0.5f, -0.5f, -0.5f,  0.50f, 0.75f, // Bottom-right
          0.5f,  0.5f, -0.5f,  0.50f, 1.0f,  // Top-right
         -0.5f,  0.5f, -0.5f,  0.25f, 1.0f,  // Top-left

        // Back Face (+Z), Texture 2
        -0.5f, -0.5f,  0.5f,  0.25f, 0.75f,  // Bottom-left
         0.5f, -0.5f,  0.5f,  0.50f, 0.75f,  // Bottom-right
         0.5f,  0.5f,  0.5f,  0.50f, 1.00f,  // Top-right
        -0.5f,  0.5f,  0.5f,  0.25f, 1.00f,  // Top-left

        // Left Face (-X), Texture 2
        -0.5f, -0.5f, -0.5f,  0.25f, 0.75f, // Bottom-left
        -0.5f,  0.5f, -0.5f,  0.25f, 1.0f,  // Top-left
        -0.5f,  0.5f,  0.5f,  0.50f, 1.0f,  // Top-right
        -0.5f, -0.5f,  0.5f,  0.50f, 0.75f, // Bottom-right

        // Right Face (+X), Texture 2
         0.5f, -0.5f, -0.5f,  0.25f, 0.75f, // Bottom-left
         0.5f,  0.5f, -0.5f,  0.25f, 1.0f,  // Top-left
         0.5f,  0.5f,  0.5f,  0.50f, 1.0f,  // Top-right
         0.5f, -0.5f,  0.5f,  0.50f, 0.75f, // Bottom-right

        // Top Face (+Y), Texture 1
        -0.5f,  0.5f, -0.5f,  0.00f, 0.75f,  // Bottom-left
         0.5f,  0.5f, -0.5f,  0.25f, 0.75f,  // Bottom-right
         0.5f,  0.5f,  0.5f,  0.25f, 1.00f,  // Top-right
        -0.5f,  0.5f,  0.5f,  0.00f, 1.00f,  // Top-left

       // Bottom Face (-Y), Texture 3
        -0.5f, -0.5f, -0.5f,  0.50f, 0.75f,  // Bottom-left
         0.5f, -0.5f, -0.5f,  0.75f, 0.75f,  // Bottom-right
         0.5f, -0.5f,  0.5f,  0.75f, 1.00f,  // Top-right
        -0.5f, -0.5f,  0.5f,  0.50f, 1.00f   // Top-left

    };

    // Index data
    unsigned int indices[] = {
        0, 1, 2, 0, 2, 3,       // Front face
        4, 5, 6, 4, 6, 7,       // Back face
        8, 9, 10, 8, 10, 11,    // Left face
        12, 13, 14, 12, 14, 15, // Right face
        16, 17, 18, 16, 18, 19, // Top face
        20, 21, 22, 20, 22, 23  // Bottom face
    };

    const int faceDirections[6][3] = {
    { 0,  0, -1}, // Front face (-Z)
    { 0,  0,  1}, // Back face (+Z)
    {-1,  0,  0}, // Left face (-X)
    { 1,  0,  0}, // Right face (+X)
    { 0,  1,  0}, // Top face (+Y)
    { 0, -1,  0}  // Bottom face (-Y)
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

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input
        processInput(window);

        // Rendering commands
        glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shapeTexture);

        // Use the shader program (vertex and fragment)
        shader.use();

        glm::mat4 view;
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);


        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        glm::mat4 proj = glm::mat4(1.0f);
        proj = glm::perspective(pi/2, aspectRatio, 0.1f, 50.0f);

        int viewLoc = glGetUniformLocation(shader.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        shader.setMat4("proj", proj);

        // Bind the VAO vertex array and draw square
        glBindVertexArray(VAO);

        for(int x = 1; x < 11; x++){
            for(int y = 1; y < 11; y++){
                for(int z = 1; z < 11; z++){
                    if(worldArray[x][y][z].occupied == 1){
                        glm::mat4 model = glm::mat4(1.0f);
                        model = glm::translate(model, glm::vec3(x - 5, y - 5, z - 5));

                        int modelLoc = glGetUniformLocation(shader.ID, "model");
                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

                        for(int face = 0; face < 6; face++){

                            int dx = faceDirections[face][0];
                            int dy = faceDirections[face][1];
                            int dz = faceDirections[face][2];

                            if(worldArray[x + dx][y + dy][z + dz].occupied == 0){
                                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(face * 6 * sizeof(unsigned int)));
                            }
                        }
                    }
                }
            }
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

    const float cameraSpeed = 2.5f * deltaTime;
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        cameraPos +=  cameraFront * cameraSpeed * glm::vec3(1.0f, 0.0f, 1.0f);
    } 
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        cameraPos -=  cameraFront * cameraSpeed * glm::vec3(1.0f, 0.0f, 1.0f);
    } 
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    } 
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
        cameraPos += cameraUp * cameraSpeed;
    } 
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
        cameraPos -= cameraUp * cameraSpeed;
    } 
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn){
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if(firstMouse){

        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    float xoffset = xpos - lastX;
    float yoffset = ypos - lastY;
    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch -= yoffset;

    if(pitch > 89.0f){
        pitch = 89.0f;
    }
    if(pitch < -89.0f){
        pitch = -89.0f;
    }

    glm::vec3 cameraDirection;
    cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraDirection.y = sin(glm::radians(pitch));
    cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    cameraFront = glm::normalize(cameraDirection);
}
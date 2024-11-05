#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <random>
#include <string>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <shader.h>


// Varaibles
int width = 800;
int height = 600;
float pi = 3.14;
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 400, lastY = 300;
unsigned int VBO, VAO, EBO, shapeTexture;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct chunk {
    struct blockData{
        bool occupied;
    };
    blockData blocks[16][32][16];
    std::vector<float> cachedVertices;
    std::vector<int> cachedIndices;
    bool needsUpdate = true;
};

glm::vec3 cameraPos = glm::vec3(0.0f, 7.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 worldFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 playerPos;
glm::vec3 cameraOffset = glm::vec3(0.0f, 2.0f, 0.0f);

glm::vec3 oldPos = glm::vec3(0.0f, 7.0f, 3.0f);

std::vector<std::vector<chunk>> Q1ChunkArray;
std::vector<std::vector<chunk>> Q2ChunkArray;
std::vector<std::vector<chunk>> Q3ChunkArray;
std::vector<std::vector<chunk>> Q4ChunkArray;

std::vector<std::vector<chunk>>* currentArray;

std::vector<float> vertices;
std::vector<int> indices;

// Function prototype
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);
void render(GLFWwindow* window, Shader& shader);
void update_current_chunk_array(int playerX, int playerY, int playerZ);
void generate_chunk_array(std::vector<std::vector<chunk>>& currentArray);
GLFWwindow* initializeGLFW();
void initialize_glad();
void load_texture(const std::string& filepath);
void setup_buffers();
void set_random_spawn();
void calculate_vertex_renders();

int main(){

    generate_chunk_array(Q1ChunkArray);
    generate_chunk_array(Q2ChunkArray);
    generate_chunk_array(Q3ChunkArray);
    generate_chunk_array(Q4ChunkArray);

    GLFWwindow* window = initializeGLFW();

    initialize_glad();

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    Shader shader("3.3.shader.vs", "3.3.shader.fs");

    // Set viewport to size of window
    glfwGetFramebufferSize(window, &width, &height);

    // Set the viewport to window size
    glViewport(0, 0, width, height);

    setup_buffers();
    load_texture("textureMap.png");
    set_random_spawn();
    cameraPos = playerPos + cameraOffset;

    shader.use();
    shader.setInt("shapeTexture", 0);

    // Keep displaying window while open
    while(!glfwWindowShouldClose(window)){

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

            // Process input
        processInput(window);

        update_current_chunk_array(playerPos.x, playerPos.y, playerPos.z);
        calculate_vertex_renders();

        render(window, shader);
     
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

    const float cameraSpeed = 4.5f * deltaTime;
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
    playerPos = cameraPos;

    // X boundary check
    /*if(chunkArray[n][m][static_cast<int>(cameraPos.x + 0.75f)][static_cast<int>(cameraPos.y)][static_cast<int>(cameraPos.z)].occupied == 1){
        cameraPos.x = oldPos.x;
    }

    // Y- boundary check
    if(chunkArray[n][m][static_cast<int>(cameraPos.x)][static_cast<int>(cameraPos.y) - 1][static_cast<int>(cameraPos.z)].occupied == 1){
        cameraPos.y = oldPos.y;
    }
    // Y+ boundary check
    if(chunkArray[n][m][static_cast<int>(cameraPos.x)][static_cast<int>(cameraPos.y + 0.75f)][static_cast<int>(cameraPos.z)].occupied == 1){
        cameraPos.y = oldPos.y;
    }

    // Z boundary check
    if(chunkArray[n][m][static_cast<int>(cameraPos.x)][static_cast<int>(cameraPos.y)][static_cast<int>(cameraPos.z + 0.75f)].occupied == 1){
        cameraPos.z = oldPos.z;
    }

    // Corner Check
    if(chunkArray[n][m][static_cast<int>(cameraPos.x + 0.75f)][static_cast<int>(cameraPos.y)][static_cast<int>(cameraPos.z + 0.75f)].occupied == 1){
        cameraPos.x = oldPos.x;
        cameraPos.z = oldPos.z;
    }

    oldPos = cameraPos;*/
}

void calculate_vertex_renders(){
    vertices.clear();
    indices.clear();
    int n, m;

    int indexOffset = 0;

    // What chunk are we on
    n = abs(static_cast<int>(playerPos.x)) / 16;
    m = abs(static_cast<int>(playerPos.z)) / 16;

    if(n > 1 && n < currentArray->size() - 1 && m > 1 && m < (*currentArray)[0].size() - 1){
        for(int r = -1; r <= 1; r++){
            for(int s = -1; s <= 1; s++){
                chunk& currentChunk = (*currentArray)[n + r][m + s];
                int chunkIndexOffset = 0;

                if(!currentChunk.needsUpdate && !currentChunk.cachedVertices.empty()) {

                    vertices.insert(vertices.end(),  currentChunk.cachedVertices.begin(), currentChunk.cachedVertices.end());
                    for(int idx : currentChunk.cachedIndices){
                        indices.push_back(idx + indexOffset);
                    }
                    indexOffset += currentChunk.cachedVertices.size() / 5;
                    continue;
                }

                std::vector<float> chunkVertices;
                std::vector<int> chunkIndices;

                float chunkWorldX = (n + r) * 16.0f;
                float chunkWorldZ = (m + s) * 16.0f;

                for(int x = 0; x < 16; x++){
                    for(int y = 0; y < 32; y++){
                        for(int z = 0; z < 16; z++){
                            if(currentChunk.blocks[x][y][z].occupied == 1){

                                const int faceDirections[6][3] = {
                                    { 0,  0, -1}, // Front face (-Z)
                                    { 0,  0,  1}, // Back face (+Z)
                                    {-1,  0,  0}, // Left face (-X)
                                    { 1,  0,  0}, // Right face (+X)
                                    { 0,  1,  0}, // Top face (+Y)
                                    { 0, -1,  0}  // Bottom face (-Y)
                                };

                                float blockWorldX = chunkWorldX + x;
                                float blockWorldY = y;
                                float blockWorldZ = chunkWorldZ + z;

                                for(int face = 0; face < 6; face++){

                                    int dx = faceDirections[face][0];
                                    int dy = faceDirections[face][1];
                                    int dz = faceDirections[face][2];

                                    if(currentChunk.blocks[x + dx][y + dy][z + dz].occupied == 0){
                                                            // Vertex data
                                        float faceVertices[6][20] = {
                                                    // Front Face (-Z), Texture 2
                                            {blockWorldX - 0.5f, blockWorldY - 0.5f, blockWorldZ - 0.5f,  0.25f, 0.75f,
                                             blockWorldX + 0.5f, blockWorldY - 0.5f, blockWorldZ - 0.5f,  0.50f, 0.75f,
                                             blockWorldX + 0.5f, blockWorldY + 0.5f, blockWorldZ - 0.5f,  0.50f, 1.00f,
                                             blockWorldX - 0.5f, blockWorldY + 0.5f, blockWorldZ - 0.5f,  0.25f, 1.00f},

                                            // Back Face (+Z), Texture 2
                                            {blockWorldX - 0.5f, blockWorldY - 0.5f, blockWorldZ + 0.5f,  0.25f, 0.75f,
                                             blockWorldX + 0.5f, blockWorldY - 0.5f, blockWorldZ + 0.5f,  0.50f, 0.75f,
                                             blockWorldX + 0.5f, blockWorldY + 0.5f, blockWorldZ + 0.5f,  0.50f, 1.00f,
                                             blockWorldX - 0.5f, blockWorldY + 0.5f, blockWorldZ + 0.5f,  0.25f, 1.00f},

                                            // Left Face (-X), Texture 2
                                            {blockWorldX - 0.5f, blockWorldY - 0.5f, blockWorldZ - 0.5f,  0.25f, 0.75f,
                                             blockWorldX - 0.5f, blockWorldY + 0.5f, blockWorldZ - 0.5f,  0.25f, 1.0f,
                                             blockWorldX - 0.5f, blockWorldY + 0.5f, blockWorldZ + 0.5f,  0.50f, 1.0f,
                                             blockWorldX - 0.5f, blockWorldY - 0.5f, blockWorldZ + 0.5f,  0.50f, 0.75f},

                                            // Right Face (+X), Texture 2
                                            {blockWorldX + 0.5f, blockWorldY - 0.5f, blockWorldZ - 0.5f,  0.25f, 0.75f,
                                             blockWorldX + 0.5f, blockWorldY + 0.5f, blockWorldZ - 0.5f,  0.25f, 1.0f,
                                             blockWorldX + 0.5f, blockWorldY + 0.5f, blockWorldZ + 0.5f,  0.50f, 1.0f,
                                             blockWorldX + 0.5f, blockWorldY - 0.5f, blockWorldZ + 0.5f,  0.50f, 0.75f},

                                            // Top Face (+Y), Texture 1
                                            {blockWorldX - 0.5f, blockWorldY + 0.5f, blockWorldZ - 0.5f,  0.00f, 0.75f,
                                             blockWorldX + 0.5f, blockWorldY + 0.5f, blockWorldZ - 0.5f,  0.25f, 0.75f,
                                             blockWorldX + 0.5f, blockWorldY + 0.5f, blockWorldZ + 0.5f,  0.25f, 1.00f,
                                             blockWorldX - 0.5f, blockWorldY + 0.5f, blockWorldZ + 0.5f,  0.00f, 1.00f},

                                            // Bottom Face (-Y), Texture 3
                                            {blockWorldX - 0.5f, blockWorldY - 0.5f, blockWorldZ - 0.5f,  0.50f, 0.75f,
                                             blockWorldX + 0.5f, blockWorldY - 0.5f, blockWorldZ - 0.5f,  0.75f, 0.75f,
                                             blockWorldX + 0.5f, blockWorldY - 0.5f, blockWorldZ + 0.5f,  0.75f, 1.00f,
                                             blockWorldX - 0.5f, blockWorldY - 0.5f, blockWorldZ + 0.5f,  0.50f, 1.00f}
                                        };

                                        chunkVertices.insert(chunkVertices.end(), std::begin(faceVertices[face]), std::end(faceVertices[face]));
                                        chunkIndices.push_back(chunkIndexOffset);
                                        chunkIndices.push_back(chunkIndexOffset + 1);
                                        chunkIndices.push_back(chunkIndexOffset + 2);
                                        chunkIndices.push_back(chunkIndexOffset);
                                        chunkIndices.push_back(chunkIndexOffset + 2);
                                        chunkIndices.push_back(chunkIndexOffset + 3);

                                        chunkIndexOffset += 4;
                                    }
                                }
                            }
                        }
                    }
                    currentChunk.cachedVertices = chunkVertices;
                    currentChunk.cachedIndices = chunkIndices;
                    currentChunk.needsUpdate = false;

                    vertices.insert(vertices.end(), chunkVertices.begin(), chunkVertices.end());
                    for(int idx : chunkIndices){
                        indices.push_back(idx + indexOffset);
                    }
                    indexOffset += chunkVertices.size() / 5;
                }
            }
        }
    }
    std::cout << "Vertices size: " << vertices.size() << " Indices size: " << indices.size() << std::endl;
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);
}

void render(GLFWwindow* window, Shader& shader){

    // Rendering commands
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shapeTexture);

    // Use the shader program (vertex and fragment)
    shader.use();

    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glm::mat4 proj = glm::perspective(pi/2, static_cast<float>(width) / static_cast<float>(height), 0.1f, 50.0f);

    shader.setMat4("view", view);
    shader.setMat4("proj", proj);
    shader.setInt("texture1", 0);

    // Bind VAO
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    // Unbind VAO
    glBindVertexArray(0);

    // Check and call events, swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
}


void set_random_spawn(){
    //set random spawn point
    playerPos.x = 50;
    playerPos.y = 33;
    playerPos.z = 50;
}

GLFWwindow* initializeGLFW(){
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
        return nullptr;
    }

    // Make window current context
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    return window;
}

void initialize_glad(){
    // Glad error message
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD";
        exit (-1);
    }
}

void load_texture(const std::string& filepath){
    glGenTextures(1, &shapeTexture);
    glBindTexture(GL_TEXTURE_2D, shapeTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filepath.c_str(), &width, &height, &nrChannels, 0);
    if(data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else{
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
}

void setup_buffers(){
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &VBO);

    // Bind VAO
    glBindVertexArray(VAO);

    // Bind and fill VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);


    // Bind and fill EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind vertex array and buffer
    glBindVertexArray(0); 
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void generate_chunk_array(std::vector<std::vector <chunk>>& currentArray){
    currentArray.resize(10, std::vector<chunk>(10));

    for(int n = 0; n < 10; n++){
        for(int m = 0; m < 10; m++){
            chunk newChunk;

            for(int x = 0; x < 16; x++){
                for(int y = 0; y < 31; y++){
                    for(int z = 0; z < 16; z++){
                        newChunk.blocks[x][y][z].occupied = rand() % 2;
                    }
                }
            }
            currentArray[n][m] = newChunk;
        }
    }
}

void update_current_chunk_array(int playerX, int playerY, int playerZ){
    if(playerX < 0 && playerZ > 0){
        currentArray = &Q1ChunkArray;
    }
    if(playerX > 0 && playerZ > 0){
        currentArray = &Q2ChunkArray;
    }
    if(playerX > 0 && playerZ < 0){
        currentArray = &Q3ChunkArray;
    }
    if(playerX < 0 && playerZ < 0){
        currentArray = &Q4ChunkArray;
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
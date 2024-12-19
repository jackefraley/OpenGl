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
#include <cmath>


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
long modulus = 4294967296;
int multiplier = 1664525;
int increment = 1013904223;
int seed = 54;
float blockBreakCounter = 0;
float blockPlaceCounter = 0;

glm::vec3 faceNormal;

struct quadrant {
    struct chunk {
        struct blockData{
            bool occupied = 0;
        };
        blockData blocks[16][32][16];
        std::vector<float> cachedVertices;
        std::vector<int> cachedIndices;
        bool needsUpdate = true;
    };
    std::vector<std::vector<float>> heightMap; 
    std::vector<std::vector<chunk>> chunkArray;  
};

quadrant Q1;
quadrant Q2;
quadrant Q3;
quadrant Q4;

quadrant* currentQuadrant = nullptr;


glm::vec3 cameraPos = glm::vec3(0.0f, 7.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 worldFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 playerPos;
glm::vec3 cameraOffset = glm::vec3(0.0f, 2.0f, 0.0f);

glm::vec3 oldPos = glm::vec3(0.0f, 7.0f, 3.0f);

std::vector<float> vertices;
std::vector<int> indices;

glm::vec2 randomGradient;

// Function prototype
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);
void render(GLFWwindow* window, Shader& shader);
void renderCrosshair(Shader& shader);
void setupCrosshair();
void update_current_chunk_array(int playerX, int playerY, int playerZ);
void generate_chunk_array(quadrant& currentQuadrant);
GLFWwindow* initializeGLFW();
void initialize_glad();
void load_texture(const std::string& filepath);
void setup_buffers();
void set_random_spawn();
void calculate_vertex_renders();
void generate_random_gradient();
float bilinearInterpolation(float V00, float V01, float V10, float V11, float x, float y);
void generate_height_map(quadrant& currentQuadrant);
unsigned int lcg(int seed);
glm::vec3 getSelectedBlock();
void deleteBlock(quadrant& currentQuadrant, int blockX, int blockY, int blockZ);
void placeBlock(quadrant& currentQuadrant, glm::vec3 pickedBlock);
glm::vec3 selectBlock(quadrant& currentQuadrant);

int main(){

    generate_chunk_array(Q1);
    generate_chunk_array(Q2);
    generate_chunk_array(Q3);
    generate_chunk_array(Q4);

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

    setupCrosshair();

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Keep displaying window while open
    while(!glfwWindowShouldClose(window)){

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

            // Process input
        processInput(window);

        update_current_chunk_array(playerPos.x, playerPos.y, playerPos.z);
        if(abs(playerPos.x / 16) > currentQuadrant->chunkArray.size() || abs(playerPos.y / 16) > currentQuadrant->chunkArray[0].size()){
            generate_chunk_array(*currentQuadrant);
        }
        calculate_vertex_renders();

        if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
            blockBreakCounter += 0.05f;
        }
        else{
            blockBreakCounter = 0;
        }

        blockPlaceCounter = blockPlaceCounter - 0.1f;
        if(blockPlaceCounter < 0){
            blockPlaceCounter = 0;
        }

        render(window, shader);

        renderCrosshair(shader);

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

    const float cameraSpeed = 4.5f * deltaTime;
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        cameraPos += glm::normalize(glm::cross(glm::cross(cameraFront, cameraUp), glm::vec3(0.0f, -1.0f,0.0f))) * cameraSpeed;
    } 
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        cameraPos -= glm::normalize(glm::cross(glm::cross(cameraFront, cameraUp), glm::vec3(0.0f, -1.0f,0.0f))) * cameraSpeed;
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
    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && blockPlaceCounter == 0){
        glm::vec3 block = getSelectedBlock();
        deleteBlock((*currentQuadrant), static_cast<int>(block.x), static_cast<int>(block.y), static_cast<int>(block.z));
        blockPlaceCounter = 2.0f;
    }
    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && blockPlaceCounter == 0){
        blockPlaceCounter = 2.0f;
        glm::vec3 block = getSelectedBlock();
        placeBlock((*currentQuadrant), block);
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

    for(int r = -2; r <= 2; r++){
        for(int s = -2; s <= 2; s++){
            int arridxX = n + r;
            int arridxZ = m + s;

            quadrant::chunk &currentChunk = currentQuadrant->chunkArray[arridxX][arridxZ];

                /*if(arridxX >= 0 && arridxZ >= 0){
                    currentChunk = Q2.chunkArray[arridxX][arridxZ];                  
                }
                else if(arridxX >= 0 && arridxZ < 0){
                    currentChunk = Q3.chunkArray[arridxX][-arridxZ];                         
                }
                else if(arridxX < 0 && arridxZ >= 0){
                    currentChunk = Q1.chunkArray[-arridxX][arridxZ];  
                }                       
                else if(arridxX < 0 && arridxZ < 0){
                    currentChunk = Q4.chunkArray[-arridxX][-arridxZ];                         
                }*/

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
            
            std::cout << "chunk updated\n";

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

                            bool isOccupied = false;

                            for(int face = 0; face < 6; face++){

                                int dx = faceDirections[face][0];
                                int dy = faceDirections[face][1];
                                int dz = faceDirections[face][2];

                                int neighborX = x + dx;
                                int neighborY = y + dy;
                                int neighborZ = z + dz;

                                if(neighborX >= 0 && neighborX < 16 && 
                                neighborY >= 0 && neighborY < 32 && 
                                neighborZ >= 0 && neighborZ < 16){

                                    isOccupied = currentChunk.blocks[neighborX][neighborY][neighborZ].occupied;
                                }
                                else {
                                    int neighborChunkX = arridxX;
                                    int neighborChunkZ = arridxZ;

                                    if(neighborX < 0){
                                        neighborChunkX -= 1;
                                        neighborX = 15;
                                    } else if(neighborX >= 16){
                                        neighborChunkX += 1;
                                        neighborX = 0;
                                    }

                                    if(neighborZ < 0){
                                        neighborChunkZ -= 1;
                                        neighborZ = 15;
                                    } else if(neighborZ >= 16){
                                        neighborChunkZ += 1;
                                        neighborZ = 0;
                                    }

                                    if (neighborChunkX >= 0 && neighborChunkX < currentQuadrant->chunkArray.size() &&
                                    neighborChunkZ >= 0 && neighborChunkZ < currentQuadrant->chunkArray[0].size()) {

                                        quadrant::chunk &neighborChunk = currentQuadrant->chunkArray[neighborChunkX][neighborChunkZ];
                                        isOccupied = neighborChunk.blocks[neighborX][neighborY][neighborZ].occupied;
                                    }
                                }

                                if(!isOccupied){
                                                        // Vertex data
                                    float faceVertices[6][20] = {
                                                // Front Face (-Z), Texture 2
                                        {blockWorldX,       blockWorldY,        blockWorldZ,        0.25f, 0.75f,
                                         blockWorldX + 1,   blockWorldY,        blockWorldZ,        0.50f, 0.75f,
                                         blockWorldX + 1,   blockWorldY + 1,    blockWorldZ,        0.50f, 1.00f,
                                         blockWorldX,       blockWorldY + 1,    blockWorldZ,        0.25f, 1.00f},

                                                // Back Face (+Z), Texture 2
                                        {blockWorldX + 1,   blockWorldY + 1,    blockWorldZ + 1,    0.25f, 1.00f,
                                         blockWorldX + 1,   blockWorldY,        blockWorldZ + 1,    0.25f, 0.75f,
                                         blockWorldX,       blockWorldY,        blockWorldZ + 1,    0.50f, 0.75f,
                                         blockWorldX,       blockWorldY + 1,    blockWorldZ + 1,    0.50f, 1.00f},

                                                // Left Face (-X), Texture 2
                                        {blockWorldX,       blockWorldY,        blockWorldZ,        0.50f, 0.75f,
                                         blockWorldX,       blockWorldY + 1,    blockWorldZ,        0.50f, 1.00f,
                                         blockWorldX,       blockWorldY + 1,    blockWorldZ + 1,    0.25f, 1.00f,
                                         blockWorldX,       blockWorldY,        blockWorldZ + 1,    0.25f, 0.75f},

                                                // Right Face (+X), Texture 2
                                        {blockWorldX + 1,   blockWorldY + 1,    blockWorldZ + 1,    0.50f, 1.00f,
                                         blockWorldX + 1,   blockWorldY + 1,    blockWorldZ,        0.25f, 1.00f,
                                         blockWorldX + 1,   blockWorldY,        blockWorldZ,        0.25f, 0.75f,
                                         blockWorldX + 1,   blockWorldY,        blockWorldZ + 1,    0.50f, 0.75f},

                                                // Top Face (+Y), Texture 1
                                        {blockWorldX + 1,   blockWorldY + 1,    blockWorldZ + 1,    0.25f, 1.00f,
                                         blockWorldX,       blockWorldY + 1,    blockWorldZ + 1,    0.00f, 1.00f,
                                         blockWorldX,       blockWorldY + 1,    blockWorldZ,        0.00f, 0.75f,
                                         blockWorldX + 1,   blockWorldY + 1,    blockWorldZ,        0.25f, 0.75f},

                                                // Bottom Face (-Y), Texture 3
                                        {blockWorldX,       blockWorldY,        blockWorldZ,        0.50f, 1.00f,
                                         blockWorldX,       blockWorldY,        blockWorldZ + 1,    0.50f, 0.75f, 
                                         blockWorldX + 1,   blockWorldY,        blockWorldZ + 1,    0.75f, 0.75f,
                                         blockWorldX + 1,   blockWorldY,        blockWorldZ,        0.75f, 1.00f}};

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
    shader.setBool("useTexture", true);

    // Bind VAO
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    // Unbind VAO
    glBindVertexArray(0);
}

float crosshairVertices[] = {
    -0.02f,  0.0f, 0.0f,
     0.02f,  0.0f, 0.0f,

     0.0f, -0.025f, 0.0f,
     0.0f,  0.025f, 0.0f
};

unsigned int crosshairVAO, crosshairVBO;

void setupCrosshair(){
    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);

    glBindVertexArray(crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVertices), crosshairVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void renderCrosshair(Shader& shader){

    glDisable(GL_DEPTH_TEST);

    shader.use();

    float aspect = width / height;

    glm::mat4 identity = glm::mat4(1.0f);
    glm::mat4 proj = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);

    shader.setMat4("view", identity);
    shader.setMat4("proj", proj);
    shader.setBool("useTexture", false);
    shader.setVec4("overrideColor", glm::vec4(1.0, 1.0, 1.0, 1.0));

    glBindVertexArray(crosshairVAO);
    glLineWidth(5.0f);
    glDrawArrays(GL_LINES, 0, 4); 
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}

void set_random_spawn(){
    //set random spawn point
    playerPos.x = 50;
    playerPos.y = 20;
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

void generate_chunk_array(quadrant& currentQuadrant){
    generate_height_map(currentQuadrant);
    
    int j = 0;
    int i = currentQuadrant.chunkArray.size();
    if(!currentQuadrant.chunkArray.empty()){
        j = currentQuadrant.chunkArray.size();
    }

    currentQuadrant.chunkArray.resize(i + 16, std::vector<quadrant::chunk>(j + 16));

    for(int n = i; n < (i + 16); n++){
        for(int m = j; m < (j + 16); m++){
            quadrant::chunk newChunk;
            for(int x = 0; x < 16; x++){
                for(int y = 0; y < 31; y++){
                    for(int z = 0; z < 16; z++){
                        if(y < currentQuadrant.heightMap[(n * 16) + x][(m * 16) + z]){
                            newChunk.blocks[x][y][z].occupied = 1;
                        }
                    }
                }
            }
            currentQuadrant.chunkArray[n][m] = newChunk;
        }
    }
}

void generate_random_gradient(){
    float angle = lcg(seed) / modulus * 2 * pi;
    randomGradient.x = cos(angle);
    randomGradient.y = sin(angle);
}

unsigned int lcg(int seed){
    seed = (multiplier * seed - increment) % modulus;
    return seed;
}

void generate_height_map(quadrant& currentQuadrant){
    int j = 0;
    int i = currentQuadrant.heightMap.size();
    if(!currentQuadrant.heightMap.empty()){
        j = currentQuadrant.heightMap.size();
    }
    currentQuadrant.heightMap.resize(i + 256, std::vector<float>(j + 256));
    int xSize = i + 256;
    int ySize = j + 256;

    for(int octave = 8; octave > 4; octave--){

        int octaveStep = pow(2, octave);

        for(int gridX = 0; gridX < xSize; gridX += octaveStep){
            for(int gridY = 0; gridY < ySize; gridY += octaveStep){
                float V00 = static_cast<float>(rand() % 500) / 100.0f;
                float V01 = static_cast<float>(rand() % 500) / 100.0f;
                float V10 = static_cast<float>(rand() % 500) / 100.0f;
                float V11 = static_cast<float>(rand() % 500) / 100.0f;

                for(int n = gridX; n < gridX + octaveStep && n < xSize; n++){
                    for(int m = gridY; m < gridY + octaveStep && m < ySize; m++){

                        float x = static_cast<float>(n - gridX) / (octaveStep - 1);
                        float y = static_cast<float>(m - gridY) / (octaveStep - 1);
                        currentQuadrant.heightMap[n][m] += bilinearInterpolation(V00, V01, V10, V11, x, y);
                    }
                }   
            }  
        }
    }
}

float bilinearInterpolation(float V00, float V01, float V10, float V11, float x, float y){
    float Vx0 = (1 - x) * V00 + x + V01;
    float Vx1 = (1 - x) * V10 + x + V11;

    return (1 - y) * Vx0 + y * Vx1;
}

void update_current_chunk_array(int playerX, int playerY, int playerZ){
    if(playerX < 0 && playerZ > 0){
        currentQuadrant = &Q1;
    }
    if(playerX > 0 && playerZ > 0){
        currentQuadrant = &Q2;
    }
    if(playerX > 0 && playerZ < 0){
        currentQuadrant = &Q3;
    }
    if(playerX < 0 && playerZ < 0){
        currentQuadrant = &Q4;
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

glm::vec3 selectBlock(quadrant& currentQuadrant){
    glm::vec3 selectBlock = glm::vec3(0);
    glm::vec3 rayDirection = glm::normalize(cameraFront);
    float closestT = 5.0f;

    glm::vec3 minChunk = playerPos - glm::vec3(5.0f);
    glm::vec3 maxChunk = playerPos + glm::vec3(5.0f);

    int minChunkX = minChunk.x / 16;
    int maxChunkX = maxChunk.x / 16;
    int minChunkZ = minChunk.z / 16;
    int maxChunkZ = maxChunk.z / 16;

    for(int chunkX = minChunkX; chunkX < maxChunkX; chunkX++){
        for(int chunkZ = minChunkZ; chunkZ < maxChunkZ; chunkZ++){
            if(chunkX >= 0 && chunkZ >= 0 && chunkX < currentQuadrant.chunkArray.size() && chunkZ < currentQuadrant.chunkArray[chunkX].size()){

                const auto& chunk = currentQuadrant.chunkArray[chunkX][chunkZ];

                for(int x = 0; x < 16; x++){
                    for(int y = 0; y < 32; y++){
                        for(int z = 0; z < 16; z++){
                            if(chunk.blocks[x][y][z].occupied){
                                glm::vec3 blockWorldPos = glm::vec3(chunkX * 16 + x, y, chunkZ * 16 + z);
                                float tmin = 0.0f;
                                float tmax = MAXFLOAT;

                                glm::vec3 minBlockCorner = blockWorldPos;
                                glm::vec3 maxBlockCorner = blockWorldPos + glm::vec3(1.0f);
                                bool skipBlock = false;

                                for(int i = 0; i < 3; i++){
                                    if(rayDirection[i] != 0){
                                        float nearPlaneDist = (minBlockCorner[i] - cameraPos[i]) / rayDirection[i];
                                        float farPlaneDist = (maxBlockCorner[i] - cameraPos[i]) / rayDirection[i];

                                        if(nearPlaneDist > farPlaneDist) std::swap(nearPlaneDist, farPlaneDist);

                                        tmin = glm::max(tmin, nearPlaneDist);
                                        tmax = glm::min(tmax, farPlaneDist);

                                        if(tmin > tmax){ 
                                            skipBlock = true;
                                            break;
                                        }
                                    } else {
                                        if(cameraPos[i] < minBlockCorner[i] || cameraPos[i] > maxBlockCorner[i]){
                                            skipBlock = true;
                                            break;
                                        }
                                    }
                                }
                                if(skipBlock) continue;

                                if (tmin < closestT) {
                                    closestT = tmin;
                                    selectBlock = blockWorldPos;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return selectBlock;
}

glm::vec3 getSelectedBlock(){
    glm::vec3 selectedBlock;
    
    float u, v, t;
    float closestT = 5.0f;

    for(int k = 0; k < vertices.size() / 20; k++){
        glm::vec3 rayDirection = glm::normalize(cameraFront);

        glm::vec3 v0(vertices[k * 20], vertices[(k * 20) + 1], vertices[(k * 20) + 2]); 
        glm::vec3 v1(vertices[(k * 20) + 5], vertices[(k * 20) + 6], vertices[(k * 20) + 7]); 
        glm::vec3 v2(vertices[(k * 20) + 10], vertices[(k * 20) + 11], vertices[(k * 20) + 12]); 
        glm::vec3 v3(vertices[(k * 20) + 15], vertices[(k * 20) + 16], vertices[(k * 20) + 17]); 


        glm::vec3 diagonal, edge;

        // First triangle
        edge = v1 - v0;
        diagonal = v2 - v0;

        glm::vec3 viewRay = glm::cross(rayDirection, diagonal);
        float determinant = glm::dot(edge, viewRay);

        if(determinant != 0){
            glm::vec3 triangleToOrigin = cameraPos - v0;
            u = glm::dot(triangleToOrigin, viewRay) / determinant;
            if(u < 1 && u > 0){
                glm::vec3 orthoVector = glm::cross(triangleToOrigin, edge);
                v = glm::dot(rayDirection, orthoVector) / determinant;
                if(v > 0 && (v + u) < 1){
                    t = glm::dot(diagonal, orthoVector) / determinant;
                    if(t > 0 && t < closestT){
                        closestT = t;
                        selectedBlock = v0;
                        faceNormal = glm::normalize(glm::cross(edge, diagonal));
                        if(faceNormal.x < 0 || faceNormal.y < 0 || faceNormal.z < 0){
                            glm::vec3 offset = {1.0f, 1.0f, 1.0f};
                            selectedBlock = selectedBlock - offset;
                        }
                    }
                }
            }
        }
        // Second triangle
        edge = v2 - v0;
        diagonal = v3 - v0;

        viewRay = glm::cross(rayDirection, diagonal);

        determinant = glm::dot(edge, viewRay);
        if(determinant != 0){
            glm::vec3 triangleToOrigin = cameraPos - v0;
            u = glm::dot(triangleToOrigin, viewRay) / determinant;
            if(u < 1 && u > 0){
                glm::vec3 orthoVector = glm::cross(triangleToOrigin, edge);
                v = glm::dot(rayDirection, orthoVector) / determinant;
                if(v > 0 && (v + u) < 1){
                    t = glm::dot(diagonal, orthoVector) / determinant;
                    if(t > 0 && t < closestT){
                        closestT = t;
                        selectedBlock = v0;
                        faceNormal = glm::normalize(glm::cross(edge, diagonal));
                        if(faceNormal.x < 0 || faceNormal.y < 0 || faceNormal.z < 0){
                            glm::vec3 offset = {1.0f, 1.0f, 1.0f};
                            selectedBlock = selectedBlock - offset;
                        }
                    }
                }
            }
        }
    }
    
    return selectedBlock;
}

void deleteBlock(quadrant& currentQuadrant, int blockX, int blockY, int blockZ){
    currentQuadrant.chunkArray[blockX / 16][blockZ / 16].blocks[blockX % 16][blockY % 32][blockZ % 16].occupied = 0;
    currentQuadrant.chunkArray[blockX / 16][blockZ / 16].needsUpdate = true;
    std::cout << "Selected Block" << blockX << ", " << blockY << ", " << blockZ << std::endl;
}

void placeBlock(quadrant& currentQuadrant, glm::vec3 pickedBlock){
    glm::vec3 newBlock = pickedBlock - faceNormal;

    currentQuadrant.chunkArray[static_cast<int>(newBlock.x) / 16][static_cast<int>(newBlock.z) / 16].blocks[static_cast<int>(newBlock.x) % 16][static_cast<int>(newBlock.y) % 32][static_cast<int>(newBlock.z) % 16].occupied = 1;
    currentQuadrant.chunkArray[static_cast<int>(newBlock.x) / 16][static_cast<int>(newBlock.z) / 16].needsUpdate = true;

    std::cout << "Selected Block" << newBlock.x << ", " << newBlock.y << ", " << newBlock.z << std::endl;
}
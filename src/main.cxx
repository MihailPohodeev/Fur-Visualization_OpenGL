#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Shader.hxx>
#include <Camera.hxx>
#include <Model.hxx>

#include <iostream>
#include <vector>
#include <cmath>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


// Генерация простой меховой текстуры
// Генерация меховой текстуры с заданным размером точек
GLuint generateFurTexture(int width, int height, float dotSize) {
    srand(0);
    std::vector<unsigned char> data(width * height, 0);
    
    // Количество точек
    int numDots = (width * height) / 100; // Примерное количество
    
    for (int i = 0; i < numDots; ++i) {
        int centerX = rand() % width;
        int centerY = rand() % height;
        
        // Размер точки с небольшими вариациями
        float currentDotSize = dotSize * (0.8f + 0.4f * (rand() % 100) / 100.0f);
        int radius = static_cast<int>(currentDotSize * std::min(width, height) / 2);
        
        // Рисуем круглую точку
        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                if (dx*dx + dy*dy <= radius*radius) {
                    int px = (centerX + dx + width) % width;
                    int py = (centerY + dy + height) % height;
                    
                    // Плавное затухание к краям
                    float dist = sqrtf(dx*dx + dy*dy) / radius;
                    float value = 1.0f - dist * dist;
                    
                    // Учитываем направленность (имитация роста волос)
                    value *= 1.0f - (float)py / height * 0.5f;
                    
                    // Смешиваем с уже существующим значением
                    float oldValue = data[py * width + px] / 255.0f;
                    value = std::max(oldValue, value);
                    data[py * width + px] = static_cast<unsigned char>(value * 255);
                }
            }
        }
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    return textureID;
}

// Создание простой сферы для демонстрации
void createSphere(std::vector<float>& vertices, std::vector<unsigned int>& indices, 
                 float radius = 1.0f, int sectors = 36, int stacks = 18) {
    const float PI = 3.1415926f;
    
    float x, y, z, xy;
    float nx, ny, nz, lengthInv = 1.0f / radius;
    float s, t;
    
    float sectorStep = 2 * PI / sectors;
    float stackStep = PI / stacks;
    float sectorAngle, stackAngle;
    
    for(int i = 0; i <= stacks; ++i) {
        stackAngle = PI / 2 - i * stackStep;
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);
        
        for(int j = 0; j <= sectors; ++j) {
            sectorAngle = j * sectorStep;
            
            // Позиция вершины
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            
            // Нормаль
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            
            // Текстурные координаты
            s = (float)j / sectors;
            t = (float)i / stacks;
            
            // Добавляем вершину
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);
            vertices.push_back(s);
            vertices.push_back(t);
        }
    }
    
    // Генерация индексов
    for(int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;
        
        for(int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if(i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            
            if(i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);



    // Настройка OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    // Создание меховой текстуры
    const int NUM_FUR_TEXTURES = 5;
    GLuint* furTextures = new GLuint[NUM_FUR_TEXTURES];

    // Размеры точек для каждой текстуры (от самой крупной до самой мелкой)
    float dotSizes[NUM_FUR_TEXTURES] = {0.008f, 0.006f, 0.004f, 0.002f, 0.0002f};

    for (int i = 0; i < NUM_FUR_TEXTURES; ++i) {
        furTextures[i] = generateFurTexture(2048, 2048, dotSizes[i]);
    }

    // Компиляция шейдеров
    Shader shader("fur_shader.verx", "fur_shader.frag");
    
    // Создание сферы
    std::vector<float> sphereVertices;
    std::vector<unsigned int> sphereIndices;
    createSphere(sphereVertices, sphereIndices);
    
    // Настройка VAO/VBO для сферы
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);
    
    // Позиции
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Нормали
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Текстурные координаты
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
    
    // Параметры рендеринга
    const int SHELL_LAYERS = 128;
    const float FUR_LENGTH = 0.2f;
    
    // Основной цикл рендеринга
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        shader.use();
        
        // Вращение модели
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 300.0f);
        glm::mat4 view = camera.GetViewMatrix();
        // model = glm::rotate(model, (float)glfwGetTime() * 0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
        
        // Настройка освещения
        glm::vec3 lightPos(5.0f, 5.0f, 5.0f);
        glm::vec3 viewPos(3.0f, 3.0f, 3.0f);
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
        glm::vec3 objectColor(0.8f, 0.6f, 0.4f); // Цвет меха
        
        
        
        // Передача uniform-переменных
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("viewPos", viewPos);
        shader.setVec3("lightColor", lightColor);
        shader.setVec3("objectColor", objectColor);
        shader.setFloat("furLength", FUR_LENGTH);
        
        // Привязка текстуры
        glActiveTexture(GL_TEXTURE0);
        
        for (int i = 0; i < NUM_FUR_TEXTURES; ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, furTextures[i]);
            shader.setInt(("furTextures[" + std::to_string(i) + "]").c_str(), i);
        }

        // Рендеринг слоев меха (shell-texturing)
        glBindVertexArray(VAO);
        for (int i = 0; i < SHELL_LAYERS; ++i) {
            float shellHeight = (float)i / SHELL_LAYERS;
            shader.setFloat("shellHeight", shellHeight);
            shader.setMat4("model", model);
            
            glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
        }
        glBindVertexArray(0);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Очистка
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    for (int i = 0; i < NUM_FUR_TEXTURES; ++i) {
      glDeleteTextures(1, &furTextures[i]);
    }
    delete [] furTextures;
    
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

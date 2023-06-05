#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "PlayerClass.h"
#include "Block.h"

#include<cstdlib>
#include<ctime>
#include <vector>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

const unsigned int SCREEN_WIDTH = 1200;
const unsigned int SCREEN_HEIGHT = 800;

//global variables
Player Me;

glm::vec3 move(0.0f);
float rot_angle = glm::radians(-90.0f);
glm::vec3 camera_move(0.0f, 5.0f, -10.0f);
glm::vec3 up_vector = glm::vec3(0.0f, 1.0f, 0.0f);
float g = -9.98f;
float currentY = 0.0f;
float timeSinceLastMove = 0.0f;
float timeSinceBulletCreated = 0.0f;
bool hasEnemyMovedYet = false;
int score = 0;
float timeThatBulletExisted = 0.0f;

//shaders
const char* vertexShaderSource = R"glsl(
#version 430 core

layout (location = 0) in vec3 Pos;
layout (location = 1) in vec2 UVPos;
layout (location = 2) uniform mat4 u_ProjMatrix;
layout (location = 3) uniform mat4 u_ViewMatrix;
layout (location = 4) uniform mat4 u_ModelMatrix;

out vec2 UV;

void main()
{
   gl_Position = u_ProjMatrix * u_ViewMatrix * u_ModelMatrix * vec4(Pos, 1.0);
   UV = UVPos;
}
)glsl";

const char* fragmentShaderSource = R"glsl(
#version 430 core

out vec4 FragColor;
in vec2 UV;

uniform sampler2D TextureSampler;
layout (location = 6) uniform vec4 CubeColor;

void main()
{
    FragColor = texture(TextureSampler, UV) * CubeColor;
}
)glsl";

std::vector<Block*> blocks;
float cooldown = 0;

int main()
{
    srand(time(NULL));
    for (int i = 0; i < 6; i++)
    {
        blocks.push_back(new Block(
            glm::vec3((int)rand() % 40 - 20, 0.0f, (int)rand() % 70 - 20), 
            glm::vec3(1, 0.9, 1), 
            glm::vec3(1.0f, 0.0f, 0.0f)
            ));
    }
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Przeksztalcenia", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);


    // build and compile our shader program
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    static const float Objects[] = {
        //ground
        -50.0f, -0.5f, -50.0f, 0.0f, 0.0f,
        50.0f, -0.5f, -50.0f, 5.0f, 0.0f,
        50.0f, -0.5f,  50.0f, 5.0f, 5.0f,

        50.0f, -0.5f,  50.0f, 5.0f, 5.0f,
        -50.0f, -0.5f,  50.0f, 0.0f, 5.0f,
        -50.0f, -0.5f, -50.0f, 0.0f,  0.0f,

        //cube
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, //bottom
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 1.0f,

         0.5f, -0.5f,  0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, //top
         0.5f, 0.5f, -0.5f, 1.0f, 0.0f,
         0.5f, 0.5f,  0.5f, 1.0f, 1.0f,

         0.5f, 0.5f,  0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f,  0.5f, 0.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, //front
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f,

         0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, //back
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f,

         0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, //left
        -0.5f,  0.5f, -0.5f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 1.0f, 1.0f,

        -0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, //right
         0.5f,  0.5f, -0.5f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f,

         0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
    };

    unsigned int VertexBufferId, VertexArrayId;
    glGenVertexArrays(1, &VertexArrayId);
    glGenBuffers(1, &VertexBufferId);
    glBindVertexArray(VertexArrayId);

    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Objects), Objects, GL_STATIC_DRAW);
    
    //position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //load and create a first texture
    unsigned int textureFloorId;

    glGenTextures(1, &textureFloorId);
    glBindTexture(GL_TEXTURE_2D, textureFloorId);
    
    //set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //load image, create texture and generate mipmaps
    int tex_width, tex_height, tex_nr_Channels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* data = stbi_load("floor.jpg", &tex_width, &tex_height, &tex_nr_Channels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    //load and create a second texture 
    unsigned int textureCubeId;

    glGenTextures(1, &textureCubeId);
    glBindTexture(GL_TEXTURE_2D, textureCubeId);
    //set the texture wrapping parameters

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //set texture filtering parameters

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //load image, create texture and generate mipmaps

    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    data = stbi_load("player.jpg", &tex_width, &tex_height, &tex_nr_Channels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    //load and create a third texture 
    unsigned int textureEnemyId;

    glGenTextures(1, &textureEnemyId);
    glBindTexture(GL_TEXTURE_2D, textureEnemyId);
    //set the texture wrapping parameters

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //set texture filtering parameters

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //load image, create texture and generate mipmaps

    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    data = stbi_load("enemy.jpg", &tex_width, &tex_height, &tex_nr_Channels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);


    glfwSetTime(0);
    glfwSwapInterval(1);

    //game loop
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        //glfwSetKeyCallback(window, key_callback);

        glClearColor(0.6f, 0.6f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VertexArrayId);

        //macierz rzutowania

        glm::mat4 ProjMatrix = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.01f, 100.0f);
        glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(ProjMatrix));

        //View matrix

        glm::vec3 camera_destination = Me.position;
        glm::vec3 camera_position = Me.position + camera_move;
        glm::mat2 rotation_matrix;
        rotation_matrix[0][0] = glm::cos(rot_angle);
        rotation_matrix[0][1] = -glm::sin(rot_angle);
        rotation_matrix[1][0] = glm::sin(rot_angle);
        rotation_matrix[1][1] = glm::cos(rot_angle);
        glm::vec2 temp_camera_pos(camera_position.z - Me.position.z, camera_position.x - Me.position.x);
        temp_camera_pos = rotation_matrix * temp_camera_pos;
        camera_position[0] = temp_camera_pos[0] + Me.position.x;
        camera_position[2] = temp_camera_pos[1] + Me.position.z;

        glm::vec3 camera_up = up_vector;

        glm::mat4 ViewMatrix = glm::lookAt(camera_position, camera_destination, camera_up);
      
        //Model matrix

        glm::mat4 ModelMatrix = glm::mat4(1.0f);
        glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(ViewMatrix));
        glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(ModelMatrix));

        //ground
        glBindTexture(GL_TEXTURE_2D, textureFloorId);
        glUseProgram(shaderProgram);
        glBindVertexArray(VertexArrayId);

        glUniform4f(6, 1.0f, 1.0f, 1.0f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //qube

        Me.CheckJump((float)glfwGetTime(), g, currentY);

        ModelMatrix = glm::translate(glm::mat4(1.0f), Me.position);
        ModelMatrix = glm::rotate(ModelMatrix, rot_angle, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(ModelMatrix));
       
        glBindTexture(GL_TEXTURE_2D, textureCubeId);
        glUseProgram(shaderProgram);

        glUniform4f(4, 1.0f, 1.0f, 1.0f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 6, 36);

        for (auto b : blocks)
        {
            b->location.x += b->velocityx;
            b->location.z += b->velocityy;
            ModelMatrix = glm::scale(glm::translate(glm::mat4(1.0f), b->GetLocation()), b->GetScale());
            glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(ModelMatrix));
            glUniform4f(1, b->GetColor().x, b->GetColor().y, b->GetColor().z, 0.0f);
            glBindTexture(GL_TEXTURE_2D, textureEnemyId);
            glUseProgram(shaderProgram);
            glDrawArrays(GL_TRIANGLES, 6, 36);

        }

        //enemy movement

        float enemyMoveTimeout = float(glfwGetTime()) - timeSinceLastMove;
        if (enemyMoveTimeout > 1.0f || hasEnemyMovedYet == false)
        {
            hasEnemyMovedYet = true;
            timeSinceLastMove = float(glfwGetTime());
            for (auto b : blocks)
            {
                if (!b->isBullet)
                {
                    int direction = (int)rand() % 4;
                    switch (direction)
                    {
                        case 0:
                            b->velocityx = 0.05;
                            b->velocityy = 0;
                            break;
                        case 1:
                            b->velocityx = -0.05;
                            b->velocityy = 0;
                            break;
                        case 2:
                            b->velocityy = 0.05;
                            b->velocityx = 0;
                            break;
                        case 3:
                            b->velocityy = -0.05;
                            b->velocityx = 0;
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        for (int i = 0; i < blocks.size(); i++)
        {
            glm::vec3 distance = blocks[i]->location - Me.position;
            float distLength = glm::length(distance);
            if (distLength < 1)
            {
                system("cls");
                std::cout << "\nYou Lost!\nYour final score is : " << score;
                glDeleteProgram(shaderProgram);
                glfwDestroyWindow(window);
                glfwTerminate();
                exit(EXIT_SUCCESS);
            }
        }
        
        //killing bullets
        if (blocks.size() > 6)
        {
            float timeNow = (float)glfwGetTime();
            if (timeNow < timeSinceBulletCreated)
            {
                float temp = timeNow;
                timeNow = timeSinceBulletCreated;
                timeSinceBulletCreated = temp;
            }
            timeThatBulletExisted = timeNow - timeSinceBulletCreated;
            if (timeThatBulletExisted > 0.5f)
            {
                blocks.pop_back();
            }
        }
        std::cout << timeSinceBulletCreated <<"  " << timeThatBulletExisted<< std::endl;
        
        //killing enemies
        if (blocks.size() >= 7)
        {
            for (int i = 0; i < blocks.size(); i++)
            {
                glm::vec3 distance = blocks[i]->location - blocks[blocks.size() - 1]->location;
                float distLength = glm::length(distance);
                if (distLength < 1 && i < blocks.size() - 1)
                {
                    blocks.erase(blocks.begin() + i);
                    system("cls");
                    if (score < 5)
                    {
                        std::cout << "Your score is: " << ++score << "!";
                    }
                    else
                    {
                        std::cout << "Congratulations! You Won!"; 
                        glDeleteProgram(shaderProgram);
                        glfwDestroyWindow(window);
                        glfwTerminate();
                        exit(EXIT_SUCCESS);
                    }
                }

               
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    
    glDeleteVertexArrays(1, &VertexArrayId);
    glDeleteBuffers(1, &VertexBufferId);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    cooldown += 0.1;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        rot_angle -= glm::radians(2.0f);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        rot_angle += glm::radians(2.0f);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        Me.Move(glm::vec3(Me.speed * glm::cos(rot_angle), 0.0f, -Me.speed * glm::sin(rot_angle)));
    }
    
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
    {
        camera_move = glm::vec3(0.0f, 40.0f, 0.0f);
        up_vector = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        camera_move = glm::vec3(0.0f, 5.0f, -10.0f);
        up_vector = glm::vec3(0.0f, 1.0f, 0.0f);
    }
    //bullet generation
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    {
        if (cooldown >= 2 && blocks.size() < 7)
        {
            cooldown = 0;
            blocks.push_back((new Block(
                glm::vec3(
                    Me.position.x + glm::cos(rot_angle) * 2,
                    Me.position.y,
                    Me.position.z - glm::sin(rot_angle) * 2
                ), 
                glm::vec3(0.2f, 0.2f, 0.2f), 
                glm::vec3(0.3f, 0.3f, 0.3f)))->SetAsBullet(glm::cos(rot_angle) * 2, -glm::sin(rot_angle) * 2));
                timeSinceBulletCreated = (float)glfwGetTime();
        }
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !(Me.jump_active)) {
        glfwSetTime(0);
        Me.jump_active = true;
        currentY = Me.position.y;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
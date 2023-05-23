﻿#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "PlayerClass.h"

#include <iostream>

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

//shaders
const char* vertexShaderSource = R"glsl(
#version 430 core

layout (location = 0) in vec3 Pos;
layout (location = 2) uniform mat4 u_ProjMatrix;
layout (location = 3) uniform mat4 u_ViewMatrix;
layout (location = 4) uniform mat4 u_ModelMatrix;

void main()
{
   gl_Position = u_ProjMatrix * u_ViewMatrix * u_ModelMatrix * vec4(Pos, 1.0);
}
)glsl";

const char* fragmentShaderSource = R"glsl(
#version 430 core

layout (location = 1) uniform vec4 TriangleColor;

out vec4 FragColor;
void main()
{
   FragColor = TriangleColor;
}
)glsl";

int main()
{
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


    static const float Qube[] = {
      -0.5f, -0.5f, -0.5f, //bottom
       0.5f, -0.5f, -0.5f,
       0.5f,  0.5f, -0.5f,
      -0.5f,  0.5f, -0.5f,

      -0.5f, -0.5f,  0.5f, //top
       0.5f, -0.5f,  0.5f,
       0.5f,  0.5f,  0.5f,
      -0.5f,  0.5f,  0.5f,

      -20.0f, -0.5f, 50.0f, //ground
      -20.0f, -0.5f, -50.0f,
       20.0f, -0.5f, -50.0f,
       20.0f, -0.5f, 50.0f
    };

    unsigned int Indices[] = {
      0, 1, 2, //bottom
      2, 3, 0,
      4, 5, 6, //top
      6, 7, 4,
      0, 1, 5, //first side
      5, 4, 0,
      1, 2, 6, //second side
      6, 5, 1,
      2, 3, 7, //third side
      7, 6, 2,
      3, 0, 4, //fourth side
      4, 7, 3,

      8, 9, 10, //ground
      10, 11, 8
    };

    unsigned int VertexBufferId, VertexArrayId, ElementBufferId;
    glGenVertexArrays(1, &VertexArrayId);
    glGenBuffers(1, &VertexBufferId);
    glGenBuffers(1, &ElementBufferId);
    glBindVertexArray(VertexArrayId);

    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Qube), Qube, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glBindVertexArray(0);

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
        system("cls");
        std::cout << "camera"<<std::endl;
        std::cout << "x: " << camera_position.x << std::endl;
        std::cout << "y: " << camera_position.y << std::endl;
        std::cout << "z: " << camera_position.z << std::endl;
        std::cout << "player" << std::endl;
        std::cout << "x: " << Me.position.x << std::endl;
        std::cout << "y: " << Me.position.y << std::endl;
        std::cout << "z: " << Me.position.z << std::endl;

        glm::vec3 camera_up = up_vector;

        glm::mat4 ViewMatrix = glm::lookAt(camera_position, camera_destination, camera_up);
      
        //Model matrix

        glm::mat4 ModelMatrix = glm::mat4(1.0f);
        glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(ViewMatrix));
        glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(ModelMatrix));

        //floor

        glUniform4f(1, 0.3f, 0.6f, 0.3f, 1.0f);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(36 * sizeof(unsigned int)));

        //qube

        Me.CheckJump((float)glfwGetTime(), g, currentY);
        Me.colorCheck();

        ModelMatrix = glm::translate(glm::mat4(1.0f), Me.position);
        ModelMatrix = glm::rotate(ModelMatrix, rot_angle, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(ModelMatrix));

        glUniform4f(1, Me.color.x, Me.color.y, Me.color.z, 1.0f);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VertexArrayId);
    glDeleteBuffers(1, &VertexBufferId);
    glDeleteBuffers(1, &ElementBufferId);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        rot_angle -= glm::radians(2.0f);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        rot_angle += glm::radians(2.0f);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        Me.Move(glm::vec3(Me.speed * glm::cos(rot_angle), 0.0f, -Me.speed * glm::sin(rot_angle)));
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !(Me.jump_active)) {
        glfwSetTime(0);
        Me.jump_active = true;
        currentY = Me.position.y;
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
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
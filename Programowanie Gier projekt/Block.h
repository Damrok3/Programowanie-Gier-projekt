#pragma once

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <cmath>

#define PI 3.141592653589793238462643383279502884197

class Block
{
private:

    glm::vec3 scale;
    glm::vec3 color;
    float moveRangeHorizontal;
    float moveRangeVertical;
    float orbitRadius;
    float orbitOffset;
    float RPM;
    float lastFrameX;
public:
    glm::vec3 location;
    float velocityx;
    float velocityy;
    bool isBullet;
    float deltaHorizontalSpeed;

    Block(glm::vec3 _location, glm::vec3 _scale, glm::vec3 _color)
    {
        location = _location;
        scale = _scale;
        color = _color;
        moveRangeHorizontal = 0;
        moveRangeVertical = 0;
        orbitRadius = 0;
        float orbitOffset = 0;
        RPM = 0;
        isBullet = false;
        lastFrameX = location.x;
        deltaHorizontalSpeed = 0;
        velocityx = 0;
        velocityy = 0;
    }

    Block* SetMovingRange(float _moveRangeHorizontal, float _moveRangeVertical)
    {
        moveRangeHorizontal = _moveRangeHorizontal;
        moveRangeVertical = _moveRangeVertical;
        return this;
    }

    void SetRotation(float _orbitRadius, float _RPM, float _orbitOffset = 0)
    {
        orbitRadius = _orbitRadius;
        RPM = _RPM;
        orbitOffset = _orbitOffset;
    }

    glm::vec3 GetLocation() // PI * 2 * (glfwGetTime() * RPM /60)
    {
        glm::vec3 newPosition = location
            + glm::vec3(moveRangeHorizontal * cos(glfwGetTime()), moveRangeVertical * sin(glfwGetTime()), 0.0f)
            + orbitRadius * glm::vec3(sin(PI * 2 * (glfwGetTime() * RPM / 60) + orbitOffset), -cos(PI * 2 * (glfwGetTime() * RPM / 60) + orbitOffset), 0.0f);
        deltaHorizontalSpeed = newPosition.x - lastFrameX;
        lastFrameX = newPosition.x;
        return newPosition;
    }

    glm::vec3 GetScale()
    {
        return scale;
    }

    glm::vec3 GetColor()
    {
        return color;
    }

    Block* SetAsBullet(float _velocityx, float _velocityy)
    {
        isBullet = true;
        velocityx = _velocityx;
        velocityy = _velocityy;
        return this;
    }
};
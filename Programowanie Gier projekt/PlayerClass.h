#pragma once

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <cmath>


class Player {
public:
	glm::vec3 position;
	glm::vec3 color;
	float speed;
	float jump_speed;
	bool jump_active;

	Player()
	{
		jump_active = false;
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		color = glm::vec3(0.0f, 0.0f, 1.0f);
		speed = 0.5f;
		jump_speed = 2.0f;
	}

	Player(glm::vec3 pos)
	{
		jump_active = false;
		position = pos;
		color = glm::vec3(0.0f, 0.0f, 1.0f);
		speed = 0.5f;
		jump_speed = 2.0f;
	}

	void Move(glm::vec3 change)
	{
		position = position + speed * change;
	}

	void colorCheck()
	{
		float distance = sqrt(position.x * position.x + position.y * position.y + position.z * position.z); //jest w glm metoda do obliczania dlugosci
		distance /= 10;
		distance /= 3;
		color.x = distance;
		color.z = 1 - distance;
	}

	void CheckJump(float t, float g, float yPos)
	{
		if (jump_active == true)
		{
			position.y += jump_speed * t + 0.5f * g * t * t;
			if (position.y < yPos)
			{
				jump_active = false;
				position.y = yPos;
			}
		}
	}
};
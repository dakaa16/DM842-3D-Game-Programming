#ifndef SNOWMAN_H
#define SNOWMAN_H

#include <glad/glad.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <Model.h>

#include <vector>

class Snowman
{
public:
	// Snowman Attributes
	glm::vec3 position;
	float orientation = 0;
	float velocity = 8;
	bool dead = false;
	glm::vec3 boxMaxCoord;
	glm::vec3 boxMinCoord;
	Model model;

	Snowman() {}

	// Constructor with vector
	Snowman(glm::vec3 cameraPos)
	{
		Model temp("Resources/Models/Magic Snowman/MagicSnowman.obj");
		model = temp;
		int negOrPos = rand() % 4 + 1;
		float randomX = rand() % 80 + 65;
		float randomZ = rand() % 80 + 65;
		if (negOrPos >= 3)
			randomX *= -1;
		if (negOrPos % 2 == 0)
			randomZ *= -1;
		position = glm::vec3(randomX + cameraPos.x,-2.0f, randomZ + cameraPos.z);
		boxMaxCoord = position + model.maxCoordinates;
		boxMinCoord = position + model.minCoordinates;
	}

	glm::mat4 getSeekMatrix(glm::vec3 cameraPos, float time)
	{
		glm::mat4 model;
		glm::vec3 move = (glm::normalize(glm::vec3(cameraPos.x, 0, cameraPos.z) - glm::vec3(position.x, 0, position.z))) * (velocity * time);
		position += move;
		model = glm::translate(model, position);
		orientation = getNewOrientation(orientation, move);
		model = glm::rotate(model, orientation, glm::vec3(0, 1, 0));


		return model;
	}

	void updateAABB()
	{
		boxMaxCoord = position + model.maxCoordinates;
		boxMinCoord = position + model.minCoordinates;
	}

private:
	float getNewOrientation(float current, glm::vec3 position)
	{
		if (glm::length(position) > 0)
		{
			return atan2(position.x, position.z);
		}
		return current;
	}

};
#endif


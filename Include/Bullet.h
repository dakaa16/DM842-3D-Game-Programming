#ifndef BULLET_H
#define BULLET_H

#include <glad/glad.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <Model.h>

class Bullet
{
public:
	// Bullet Attributes
	glm::vec3 position;
	float orientation;
	float velocity = 50;
	float scale = 0.05;
	Model model;
	glm::vec3 boxMaxCoord;
	glm::vec3 boxMinCoord;
	glm::vec3 initFront;

	Bullet() {}

	Bullet(glm::vec3 gunPos, glm::vec3 front)
	{
		Model temp("Resources/Models/bullet/bullet.obj");
		model = temp;
		position = gunPos;
		initFront = front;
		boxMaxCoord = position + (model.maxCoordinates * scale);
		boxMinCoord = position + (model.minCoordinates * scale);
	}

	glm::mat4 getShootingMatrix(float time)
	{
		glm::mat4 model;
		glm::vec3 move = initFront * (velocity * time);
		position += move;
		model = glm::translate(model, glm::vec3(position.x, position.y - 0.17, position.z + 0.07));
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		orientation = getNewOrientation(orientation, move);
		model = glm::rotate(model, orientation - 80.0f, glm::vec3(0, 1, 0));
		return model;
	}

	void updateAABB()
	{
		boxMaxCoord = position + (model.maxCoordinates * scale);
		boxMinCoord = position + (model.minCoordinates * scale);
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
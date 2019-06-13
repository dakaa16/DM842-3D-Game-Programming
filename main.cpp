
//Includes--------------------------------------
#include <iostream>
#include <list>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
//------ GLM - Open GL Mathematics: https://glm.g-truc.net/0.9.8/index.html
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//------ stb_image - loads images as data to use for textures: https://github.com/nothings/stb/blob/master/stb_image.h
//------ | Credit goes to Sean Barett as well as the other contributors
#include <stb_image.h>
//------ shader_s header. Largely based on guide: https://learnopengl.com/#!Getting-started/Shaders
#include <Shader.h>
//------ Camera header. Largely based on guide: https://learnopengl.com/#!Getting-started/Camera
#include <Camera.h>
//------ Model header. Largely based on guide: https://learnopengl.com/#!Model-Loading/Model
#include <Model.h>
//------ Model header. Represents a snowman
#include <Snowman.h>
//------ Bullet header. Represents a bullet
#include <Bullet.h>

// functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
int main();
void processInput(GLFWwindow *window);
bool checkCollision(glm::vec3 aMax, glm::vec3 aMin, glm::vec3 bMax, glm::vec3 bMin);
// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 600;

//Setting up camera variables
Camera camera(glm::vec3(0.0f, 0.0f, 0.3f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
const glm::vec3 playerAABBmax(0.5, 2, 0.5);
const glm::vec3 playerAABBmin(-0.5, -2, -0.5);

bool playerDead = false;
int points = 0;
bool pointsWritten = false;

Bullet bullets[300];
int bulletCounter = 0;

Snowman snowmen[200];
int snowmanCounter = 0;
float lastSnowSpawn = 0;
float spawnRate = 6;

float deltaTime = 0.0f; //Time between current and last frame
float lastFrame = 0.0f; //Time of last frame


int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Snowman Shooter", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//Capture the cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//sets mouse_callback to cursor position callback
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//Enabling depth buffer
	glEnable(GL_DEPTH_TEST);

	//Initialize shader
	Shader modelShader("Resources/Shaders/model_loading_vs.txt", "Resources/Shaders/model_loading_fs.txt");
	//Initialize models
	Model skyboxModel("Resources/Models/SkyBox/SkyBox.obj"); //Texture: https://gamebanana.com/textures/1875
	Model gunModel("Resources/Models/Gun/gun.obj"); // Texture https://www.models-resource.com/pc_computer/counterstrikesource/model/15934/?source=genre
	Model groundModel("Resources/Models/Ground/ground.obj"); //Texture: http://bgfons.com/download/1556
	Model gameOver("Resources/Models/GameOver/gameOver.obj");

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		//Calculate deltatime for each frame
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;


		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//WireFrame mode:
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		modelShader.use();
		//Projection matrix
		//----------------------------------------------------------------------------------
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		modelShader.setMat4("projection", projection);

		//View for camera
		//----------------------------------------------------------------------------------
		glm::mat4 view;
		view = camera.GetViewMatrix();
		modelShader.setMat4("view", view);

		if (!playerDead)
		{
			//Drawing skybox
			//----------------------------------------------------------------------------------
			glm::mat4 skyMat;
			skyMat = glm::translate(skyMat, camera.Position);
			skyMat = glm::scale(skyMat, glm::vec3(50.0f, 50.0f, 50.0f));
			modelShader.setMat4("model", skyMat);
			skyboxModel.Draw(modelShader);

			//Drawing snow ground
			//----------------------------------------------------------------------------------
			glm::mat4 mod;
			mod = glm::translate(mod, glm::vec3(0.0f, -1.9f, 0.0f));
			mod = glm::scale(mod, glm::vec3(5.0f, 5.0f, 5.0f));
			modelShader.setMat4("model", mod);
			groundModel.Draw(modelShader);

			//Drawing gun
			//----------------------------------------------------------------------------------
			glm::mat4 model;
			model = glm::translate(model, glm::vec3(0, -0.35, -1));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
			model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
			glm::mat4 MVP = glm::inverse(camera.GetViewMatrix()) * model;
			modelShader.setMat4("model", MVP);
			gunModel.Draw(modelShader);

			//Drawing bullets
			//----------------------------------------------------------------------------------
			for (int i = 0; i < bulletCounter; i++)
			{
				//Exclude all bullets that can't be seen
				if (bullets[i].position.x > 60 + camera.Position.x || bullets[i].position.x < -60 + camera.Position.x ||
					bullets[i].position.y > 60 + camera.Position.y || bullets[i].position.y < -10 + camera.Position.y ||
					bullets[i].position.z > 60 + camera.Position.z || bullets[i].position.y < -60 + camera.Position.z)
				{
					continue;
				}
				modelShader.setMat4("model", bullets[i].getShootingMatrix(deltaTime));
				bullets[i].model.Draw(modelShader);
			}

			//Drawing snowman + checking for collisions
			//----------------------------------------------------------------------------------
			if (glfwGetTime() > lastSnowSpawn + spawnRate)
			{
				lastSnowSpawn = glfwGetTime();
				spawnRate -= spawnRate / glfwGetTime();
				Snowman snowman(camera.Position);
				snowmen[snowmanCounter] = snowman;
				snowmanCounter++;
				if (snowmanCounter == 200)
					snowmanCounter = 0;
			}
			for (int i = 0; i < snowmanCounter; i++)
			{
				//Exclude all bullets that can't be seen
				if (snowmen[i].dead)
				{
					continue;
				}
				modelShader.setMat4("model", snowmen[i].getSeekMatrix(camera.Position, deltaTime));
				snowmen[i].model.Draw(modelShader);
				//Update snowman and player AABB
				snowmen[i].updateAABB();
				glm::vec3 playerBoxMaxCoord(camera.Position.x + playerAABBmax.x, camera.Position.y + playerAABBmax.y, camera.Position.z + playerAABBmax.z);
				glm::vec3 playerBoxMinCoord(camera.Position.x + playerAABBmin.x, camera.Position.y + playerAABBmin.y, camera.Position.z + playerAABBmin.z);
				// player - snowman collision check
				if (checkCollision(playerBoxMaxCoord, playerBoxMinCoord, snowmen[i].boxMaxCoord, snowmen[i].boxMinCoord))
				{
					playerDead = true;
				}
				//bullet - snowman collision check
				for (int j = 0; j < bulletCounter; j++)
				{
					//Update bullet AABB
					bullets[j].updateAABB();
					if (checkCollision(bullets[j].boxMaxCoord, bullets[j].boxMinCoord, snowmen[i].boxMaxCoord, snowmen[i].boxMinCoord))
					{
						snowmen[i].dead = true;
						points++;
					}
				}
			}
		}
		else
		{
			//Drawing Game Over Plane
			//----------------------------------------------------------------------------------
			glm::mat4 gameOverMat;
			gameOverMat = glm::translate(gameOverMat, glm::vec3(0, 0, -1));
			gameOverMat = glm::rotate(gameOverMat, glm::radians(90.0f), glm::vec3(1, 0, 0));
			gameOverMat = glm::scale(gameOverMat, glm::vec3(0.3f, 0.3f, 0.3f));
			glm::mat4 MVP = glm::inverse(camera.GetViewMatrix()) * gameOverMat;
			modelShader.setMat4("model", MVP);
			gameOver.Draw(modelShader);
			if (!pointsWritten)
			{
				std::cout << "You killed " << points << " snowmen!";
				pointsWritten = true;
			}
				
		}
		
		

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
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
	{	
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		camera.Zoom = 30.0f;
	}
	else
	{
		camera.Zoom = 45.0f;
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	//Avoid jump in camera movement
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		//Create new bullet
		Bullet bullet(camera.Position, camera.Front);
		bullets[bulletCounter] = bullet;
		bulletCounter++;
		if (bulletCounter == 300)
		{
			bulletCounter = 0;
		}
	}
}

bool checkCollision(glm::vec3 aMax, glm::vec3 aMin, glm::vec3 bMax, glm::vec3 bMin)
{
	//Axis Alligned Bounding Boxes (AABB) - not suited for rotations
	return (aMax.x > bMin.x && aMin.x < bMax.x &&
			aMax.y > bMin.y && aMin.y < bMax.y &&
			aMax.z > bMin.z && aMin.z < bMax.z);
}




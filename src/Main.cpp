#include "ShapeGenerator.h"
#include "ShapeData.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "shader.h"
#include "camera.h"
#include "cylinder.h"

#include <iostream>

/* Dynamic error handling */
#define ASSERT(x) if(!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

static void GLClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall(const char* function, const char* file, int line)
{
	while (GLenum error = glGetError())
	{
		std::cout << "[OpenGL Error] (" << error << ")\n"
			<< function << " " << file << ":" << line << std::endl;
		return false;
	}
	return true;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(1.0f, 3.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

bool perspective = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// offset variables for plane, sphere
const uint NUM_VERTICES_PER_TRI = 3;
const uint NUM_FLOATS_PER_VERTICE = 9;
const uint VERTEX_BYTE_SIZE = NUM_FLOATS_PER_VERTICE * sizeof(float);

// plane
GLuint planeNumIndices;
GLuint planeVertexArrayObjectID;
GLuint planeIndexByteOffset;

// sphere
GLuint sphereNumIndices;
GLuint sphereVertexArrayObjectID;
GLuint sphereIndexByteOffset;

// projection matrix
glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

int main()
{
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Module 6: OpenGL Lights", NULL, NULL);
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
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	GLCall(glEnable(GL_DEPTH_TEST));

	Shader lightingShader("res/shaders/multiple_lights.vs", "res/shaders/multiple_lights.fs");
	Shader lightCubeShader("res/shaders/light_cube.vs", "res/shaders/light_cube.fs");

	float vertices[] = {
		// positions          // normals           // texture coords
		//Front Face
		-1.8f,	0.0f, -0.70f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, 
		 1.8f,  0.0f, -0.70f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		 1.8f, 0.45f, -0.70f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f, 
		 1.8f, 0.45f, -0.70f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-1.8f, 0.45f, -0.70f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
		-1.8f,  0.0f, -0.70f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, 
		// Back face
		-1.8f,  0.0f,  0.70f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 1.8f,  0.0f,  0.70f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		 1.8f, 0.45f,  0.70f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		 1.8f, 0.45f,  0.70f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-1.8f, 0.45f,  0.70f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-1.8f,  0.0f,  0.70f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-1.8f, 0.45f,  0.70f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-1.8f, 0.45f, -0.70f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-1.8f,  0.0f, -0.70f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-1.8f,  0.0f, -0.70f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-1.8f,  0.0f,  0.70f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-1.8f, 0.45f,  0.70f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		 1.8f, 0.45f,  0.70f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		 1.8f, 0.45f, -0.70f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		 1.8f,  0.0f, -0.70f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 1.8f,  0.0f, -0.70f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 1.8f,  0.0f,  0.70f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		 1.8f, 0.45f,  0.70f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
	
		-1.8f, -0.0f, -0.70f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		 1.8f, -0.0f, -0.70f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		 1.8f, -0.0f,  0.70f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		 1.8f, -0.0f,  0.70f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-1.8f, -0.0f,  0.70f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-1.8f, -0.0f, -0.70f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		-1.8f, 0.45f, -0.70f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 1.8f, 0.45f, -0.70f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 1.8f, 0.45f,  0.70f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 1.8f, 0.45f,  0.70f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-1.8f, 0.45f,  0.70f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-1.8f, 0.45f, -0.70f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
	};

	// positions of the point lights
	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.8f,  2.8f,  -1.2f),
		glm::vec3(2.5f, 1.0f, -1.0f)
	};

	glm::vec3 pointLightColors[] = {
		glm::vec3(1.0f, 0.6f, 0.0f),
		glm::vec3(0.0f, 0.5f, 1.0f)
	};

	// cup handle cube's VAO and VBO
	unsigned int cubeVBO, cubeVAO;
	GLCall(glGenVertexArrays(1, &cubeVAO));
	GLCall(glGenBuffers(1, &cubeVBO));

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(cubeVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);


	// position attribute for cube
	GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0));
	GLCall(glEnableVertexAttribArray(0));
	// texture attribute for cube
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	GLCall(glEnableVertexAttribArray(1));
	// normal attribute for cube
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	GLCall(glEnableVertexAttribArray(2));


	// lightCube's VAO and VBO
	unsigned int lightCubeVAO, lightCubeVBO;
	glGenVertexArrays(1, &lightCubeVAO);
	glGenBuffers(1, &lightCubeVBO);

	glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLCall(glGenVertexArrays(1, &lightCubeVAO));
	GLCall(glBindVertexArray(lightCubeVAO));

	GLCall(glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO));
	// note that we update the lamp's position attribute's stride to reflect the updated buffer data
	GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0));
	GLCall(glEnableVertexAttribArray(0));

	// creates plane object
	ShapeData plane = ShapeGenerator::makePlane(10);

	unsigned int planeVBO{}, planeVAO;
	GLCall(glGenVertexArrays(1, &planeVAO));
	GLCall(glGenBuffers(1, &planeVBO));

	GLCall(glBindVertexArray(planeVAO));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, planeVBO));
	GLCall(glBufferData(GL_ARRAY_BUFFER, plane.vertexBufferSize() + plane.indexBufferSize(), 0, GL_STATIC_DRAW));

	GLsizeiptr currentOffset = 0;

	glBufferSubData(GL_ARRAY_BUFFER, currentOffset, plane.vertexBufferSize(), plane.vertices);
	currentOffset += plane.vertexBufferSize();

	planeIndexByteOffset = currentOffset;

	glBufferSubData(GL_ARRAY_BUFFER, currentOffset, plane.indexBufferSize(), plane.indices);

	planeNumIndices = plane.numIndices;

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTE_SIZE, (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTE_SIZE, (void*)(sizeof(float) * 3));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTE_SIZE, (void*)(sizeof(float) * 6));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeVBO);

	// Creating of the sphere object 
	ShapeData sphere = ShapeGenerator::makeSphere();
	unsigned int sphereVBO{}, sphereVAO;
	glGenVertexArrays(1, &sphereVAO);
	glGenBuffers(1, &sphereVBO);
	glBindVertexArray(sphereVAO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sphere.vertexBufferSize() + sphere.indexBufferSize(), 0, GL_STATIC_DRAW);
	currentOffset = 0;
	glBufferSubData(GL_ARRAY_BUFFER, currentOffset, sphere.vertexBufferSize(), sphere.vertices);
	currentOffset += sphere.vertexBufferSize();
	sphereIndexByteOffset = currentOffset;
	glBufferSubData(GL_ARRAY_BUFFER, currentOffset, sphere.indexBufferSize(), sphere.indices);
	sphereNumIndices = sphere.numIndices;
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTE_SIZE, (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTE_SIZE, (void*)(sizeof(float) * 3));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, VERTEX_BYTE_SIZE, (void*)(sizeof(float) * 6));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereVBO);

	// load textures using utility function
	unsigned int cubeDiffuseMap = loadTexture("images/metal.jpg");
	unsigned int planeDiffuseMap = loadTexture("images/wrinkle_paper.jpg");
	unsigned int sphereDiffuseMap = loadTexture("images/red-stock.jpg");
	unsigned int cylinderDiffuseMap = loadTexture("images/metal.jpg");

	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	lightingShader.use();
	lightingShader.setInt("material.diffuse", 0);
	lightingShader.setInt("material.specular", 1);

	// Render loop
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		processInput(window);

		// Sets the background color of the window to black (it will be implicitely used by glClear)
		GLCall(glClearColor(0.1f, 0.1f, 0.1f, 1.0f));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		lightingShader.use();
		lightingShader.setVec3("viewPos", camera.Position);
		lightingShader.setFloat("material.shininess", 32.0f);

		// key light
		lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
		lightingShader.setVec3("pointLights[0].ambient", pointLightColors[0]);
		lightingShader.setVec3("pointLights[0].diffuse", 0.5f, 0.5f, 0.5f);
		lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("pointLights[0].constant", 1.0f);
		lightingShader.setFloat("pointLights[0].linear", 0.09);
		lightingShader.setFloat("pointLights[0].quadratic", 0.032);
		// fill light
		lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
		lightingShader.setVec3("pointLights[1].ambient", pointLightColors[1]);
		lightingShader.setVec3("pointLights[1].diffuse", 0.1f, 0.1f, 0.1f);
		lightingShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
		lightingShader.setFloat("pointLights[1].constant", 1.0f);
		lightingShader.setFloat("pointLights[1].linear", 0.09);
		lightingShader.setFloat("pointLights[1].quadratic", 0.032);

		// view/projection transformations
		glm::mat4 view = camera.GetViewMatrix();
		lightingShader.setMat4("projection", projection);
		lightingShader.setMat4("view", view);

		// world transformation
		glm::mat4 model = glm::mat4(1.0f);
		lightingShader.setMat4("model", model);

		// bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		GLCall(glBindTexture(GL_TEXTURE_2D, cubeDiffuseMap));
		GLCall(glBindVertexArray(cubeVAO));
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(4.0f, -0.43f, -2.0f));
		lightingShader.setMat4("model", model);

		glDrawArrays(GL_TRIANGLES, 0, 36);

		// setup to draw plane
		GLCall(glActiveTexture(GL_TEXTURE0));
		GLCall(glBindTexture(GL_TEXTURE_2D, planeDiffuseMap));
		GLCall(glBindVertexArray(planeVAO));
		model = model = glm::mat4(2.0f);
		model = glm::translate(model, glm::vec3(2.5f, -0.22f, 0.0f));
		lightingShader.setMat4("model", model);

		// draw plane
		GLCall(glDrawElements(GL_TRIANGLES, planeNumIndices, GL_UNSIGNED_SHORT, (void*)planeIndexByteOffset));



		// setup to draw sphere
		GLCall(glBindTexture(GL_TEXTURE_2D, sphereDiffuseMap));
		GLCall(glBindVertexArray(sphereVAO));
		model = model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.1f, -2.0f));
		model = glm::scale(model, glm::vec3(0.5f)); // Make it a smaller sphere
		lightingShader.setMat4("model", model);

		// draw sphere
		GLCall(glDrawElements(GL_TRIANGLES, sphereNumIndices, GL_UNSIGNED_SHORT, (void*)sphereIndexByteOffset));

		// setup to draw cylinders (battery)
		GLCall(glBindTexture(GL_TEXTURE_2D, cylinderDiffuseMap));
		GLCall(glBindVertexArray(sphereVAO));
		model = model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(4.0f, 0.35f, 3.0f));
		model = glm::scale(model, glm::vec3(0.5f));
		lightingShader.setMat4("model", model);
		static_meshes_3D::Cylinder C(1, 500, 3, true, true, true);
		C.render();
		model = glm::translate(model, glm::vec3(0.0f, 1.32f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f)); // Make it a smaller cylinder
		static_meshes_3D::Cylinder D(0.7, 500, 1, true, true, true);
		lightingShader.setMat4("model", model);
		D.render();

		/* All the rendering of the pin */
		GLCall(glBindTexture(GL_TEXTURE_2D, sphereDiffuseMap));
		GLCall(glBindVertexArray(sphereVAO));
		model = model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(1.5f, -0.31f, 1.0f));
		model = glm::scale(model, glm::vec3(0.5f)); // Make it a smaller cylinder
		static_meshes_3D::Cylinder PinBase(0.60, 200, 0.5, true, true, true);
		lightingShader.setMat4("model", model);
		PinBase.render();
		model = glm::translate(model, glm::vec3(0.0f, 0.75f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f));
		lightingShader.setMat4("model", model);
		static_meshes_3D::Cylinder PinHandle(0.60, 200, 2.25, true, true, true);
		PinHandle.render();

		GLCall(glBindVertexArray(sphereVAO));
		model = model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(1.5f, 0.45f, 1.0f));
		model = glm::scale(model, glm::vec3(0.5f));
		lightingShader.setMat4("model", model);
		static_meshes_3D::Cylinder TopCylinder(0.75, 200, 0.5, true, true, true);
		TopCylinder.render();
		GLCall(glBindTexture(GL_TEXTURE_2D, cylinderDiffuseMap));
		model = glm::translate(model, glm::vec3(0.0f, 0.85f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f)); // Make it a smaller cylinder
		static_meshes_3D::Cylinder PinShaft(0.1, 200, 2.4, true, true, true);
		lightingShader.setMat4("model", model);
		PinShaft.render();


		// draw the lamp object(s)
		lightCubeShader.use();
		lightCubeShader.setMat4("projection", projection);
		lightCubeShader.setMat4("view", view);

		// we now draw as many light bulbs as we have point lights.
		glBindVertexArray(lightCubeVAO);

		// setup to draw sphere
		GLCall(glBindVertexArray(sphereVAO));
		model = model = glm::mat4(1.0f);
		model = glm::translate(model, pointLightPositions[0]);
		model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller sphere
		lightCubeShader.setMat4("model", model);
		GLCall(glDrawElements(GL_TRIANGLES, sphereNumIndices, GL_UNSIGNED_SHORT, (void*)sphereIndexByteOffset));

		// setup to draw sphere
		GLCall(glBindVertexArray(sphereVAO));
		model = model = glm::mat4(1.0f);
		model = glm::translate(model, pointLightPositions[1]);
		model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller sphere
		lightCubeShader.setMat4("model", model);
		GLCall(glDrawElements(GL_TRIANGLES, sphereNumIndices, GL_UNSIGNED_SHORT, (void*)sphereIndexByteOffset));

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	GLCall(glDeleteVertexArrays(1, &cubeVAO));
	GLCall(glDeleteBuffers(1, &cubeVBO));

	GLCall(glDeleteVertexArrays(1, &planeVAO));
	GLCall(glDeleteBuffers(1, &planeVBO));

	GLCall(glDeleteVertexArrays(1, &sphereVAO));
	GLCall(glDeleteBuffers(1, &sphereVBO));

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window)
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
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);

	// change view between perspective and orthographics
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		if (perspective) {
			projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
			perspective = false;
		}
		else {
			projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			perspective = true;
		}
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
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
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		GLCall(glBindTexture(GL_TEXTURE_2D, textureID));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data));
		GLCall(glGenerateMipmap(GL_TEXTURE_2D));

		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
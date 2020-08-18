

#include <iostream>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>

#include "dashboard.h"
DashBoard *dashboard = nullptr;
Screen *screen = nullptr;
Window *TransferFunctionWindow;
Window *modelPositioningWindow;
#include "camera.h"
Camera* camera;

#include "ViewAlignedSlicer.h"
//shader
#include "shader.h"

#include "SOIL.h"

bool reSlice = false;

GLuint VBO;
GLuint VAO;

GLuint textureID;
GLuint colorTextureID;
//modelview and projection matrices
glm::mat4 Model, View, Projection;
glm::mat4 MV = glm::mat4(1.0);


//screen dimensions
const int WIDTH = 1280;
const int HEIGHT = 960;
int width, height;


int state = 0, oldX = 0, oldY = 0;

//load 3d data into data byte
GLubyte * load_3d_raw_data(std::string texture_path, glm::vec3 dimension) {
	size_t size = dimension[0] * dimension[1] * dimension[2];

	FILE *fp;
	GLubyte *data = new GLubyte[size];			  // 8 bit
	if (!(fp = fopen(texture_path.c_str(), "rb"))) {
		std::cout << "Error: opening .raw file failed" << std::endl;
		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "OK: open .raw file success" << std::endl;
	}
	if (fread(data, sizeof(char), size, fp) != size) {
		std::cout << "Error: read .raw file failed" << std::endl;
		exit(1);
	}
	else {
		std::cout << "OK: read .raw file success" << std::endl;
	}
	fclose(fp);
	return data;
}

//Load the 3D texture
void load3DTexture() {
		GLubyte* texture3DData = load_3d_raw_data(dashboard->selectedRawFile->filename
														,dashboard->selectedRawFile->dimension);

		//generate OpenGL texture
		glGenTextures(1, &textureID);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, textureID);
		// set the texture parameters
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		//set the mipmap levels (base and max)
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 4);

		glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, dashboard->selectedRawFile->dimension.x, 
											dashboard->selectedRawFile->dimension.y,
										dashboard->selectedRawFile->dimension.z
													, 0, GL_RED, GL_UNSIGNED_BYTE, texture3DData);

		//generate mipmaps
		glGenerateMipmap(GL_TEXTURE_3D);
		delete[] texture3DData;
		dashboard->reload = false;
}



//load color texture, use only the the first row of the image
void loadColorTexture() {
	
	glGenTextures(1, &colorTextureID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, colorTextureID);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	int width, height;

	//load the image in byte array
	GLubyte *image = SOIL_load_image(dashboard->colorBarImagePath.c_str(),
		
											&width, &height, 0, SOIL_LOAD_RGB);
	//read and map the first row for transfer function mapping
	float transferFunction[256][4];
	int count = 256;
	for (int i = 0; i < 256 * 3; i+=3) {

		int colorR = (int)image[i];
		int colorG = (int)image[i+1];
		int colorB = (int)image[i+2];
		
		count--;
		transferFunction[count][0] = (float)colorR / (float)255;
		transferFunction[count][1] = (float)colorG / (float)255;
		transferFunction[count][2] = (float)colorB / (float)255;
		transferFunction[count][3] = 0;
	}


	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_FLOAT, transferFunction);
}

void initializeGLFW() {

	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	
}

void bindGLFWEvents(GLFWwindow* window) {
	glfwSetCursorPosCallback(window,
		[](GLFWwindow *, double x, double y) {
		screen->cursorPosCallbackEvent(x, y);

	});

	glfwSetMouseButtonCallback(window,
		[](GLFWwindow *window, int button, int action, int modifiers) {
		screen->mouseButtonCallbackEvent(button, action, modifiers);

		
	});

	glfwSetKeyCallback(window,
		[](GLFWwindow *, int key, int scancode, int action, int mods) {
		screen->keyCallbackEvent(key, scancode, action, mods);
	});

	glfwSetCharCallback(window,
		[](GLFWwindow *, unsigned int codepoint) {
		screen->charCallbackEvent(codepoint);
	});

	glfwSetDropCallback(window,
		[](GLFWwindow *, int count, const char **filenames) {
		screen->dropCallbackEvent(count, filenames);
	});

	glfwSetScrollCallback(window,
		[](GLFWwindow *, double x, double y) {
		screen->scrollCallbackEvent(x, y);
		camera->processMouseScroll(y);
	});

	glfwSetFramebufferSizeCallback(window,
		[](GLFWwindow *, int awidth, int aheight) {
		screen->resizeCallbackEvent(awidth, aheight);
		if (awidth != 0)
		{
			glViewport(0, 0, (GLsizei)awidth, (GLsizei)awidth);
			width = awidth;
			height = aheight;
			Projection = glm::perspective(glm::radians(60.0f), (float)awidth / aheight, 0.1f, 1000.0f);
			if (TransferFunctionWindow != NULL) {
				TransferFunctionWindow->setPosition(Vector2i(width - 480, 10));
			}
		}
	});
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}


// The MAIN function, from here we start the application and run the game loop
int main()
{
	initializeGLFW();

	// Create a GLFWwindow object
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "CS6366 - Assignment 4", nullptr, nullptr);
	if (window == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	
	
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	Projection = glm::perspective(glm::radians(60.0f), (float)width / height, 0.1f, 1000.0f);
	glfwSwapInterval(0);
	glfwSwapBuffers(window);

	//Create a nanogui dahsboard 
		screen = new Screen();
		bindGLFWEvents(window);
		
		camera = new Camera(glm::vec3(0.5, 0.5, 2.5), glm::vec3(0.0f, 1.0f, 0.0f));

		dashboard = new DashBoard(screen, window, camera);
		dashboard->createConfigurationForm(screen);
		TransferFunctionWindow = dashboard->createTransferFunctionForm(screen);
		TransferFunctionWindow->setPosition(Vector2i(width - 480, 10));
		modelPositioningWindow = dashboard->createModelPositioningForm(screen);
		modelPositioningWindow->setPosition(Vector2i(width - 480, height- 250));

		dashboard->reload = true;
		
	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

		
		Shader shader("shaders/cubeTexture.vert", "shaders/cubeTexture.frag");
		shader.use();
		glUniform1i(glGetUniformLocation(shader.program, "texture_3d"), 0);
		glUniform1i(glGetUniformLocation(shader.program, "texture_color"), 1);


	ViewAlignedSlicer slicer;

		glm::mat4 Rx = glm::rotate(glm::mat4(1.0f), glm::radians(dashboard->objRotX), glm::vec3(1.0f, 0.0f, 0.0f));
		Model =   glm::rotate(Rx, glm::radians(dashboard->objRotY), glm::vec3(0.0f, 1.0f, 0.0f));
		View = camera->GetViewMatrix();
		MV = View * Model;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glm::vec3 *slicedVertices = slicer.sliceCube(dashboard->sampleRate,
														-glm::vec3(MV[0][2], MV[1][2], MV[2][2]),
															dashboard->viewSliderValue);

		//pass the sliced vertices vector to buffer object memory
				glBufferData(GL_ARRAY_BUFFER, slicer.bufferSize, 0, GL_DYNAMIC_DRAW);
					glBufferSubData(GL_ARRAY_BUFFER, 0, slicer.bufferSize, &(slicedVertices[0].x));


		//enable vertex attribute array for position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindVertexArray(0);

		loadColorTexture();

	//Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		processInput(window);

		glm::mat4 R = glm::yawPitchRoll(glm::radians(dashboard->objRotX)
											, glm::radians(dashboard->objRotY)
													, glm::radians(dashboard->objRotZ));

		//Rx = glm::rotate(glm::mat4(1.0f), glm::radians(dashboard->objRotX), glm::vec3(1.0f, 0.0f, 0.0f));
		Model = glm::mat4(1.0f) * R;//glm::rotate(Rx, glm::radians(dashboard->objRotY), glm::vec3(0.0f, 1.0f, 0.0f));
		View = camera->GetViewMatrix();
		MV = View * Model;

		if (dashboard->reload)
			load3DTexture();

		if (dashboard->reslice) {
			delete[] slicedVertices;
			slicedVertices = slicer.sliceCube(dashboard->sampleRate, 
									-glm::vec3(MV[0][2], MV[1][2], MV[2][2]), dashboard->viewSliderValue);
			glBindVertexArray(VAO);
				glBindBuffer(GL_ARRAY_BUFFER, VBO);
					glBufferData(GL_ARRAY_BUFFER, slicer.bufferSize, 0, GL_DYNAMIC_DRAW);
						glBufferSubData(GL_ARRAY_BUFFER, 0, slicer.bufferSize, &(slicedVertices[0].x));
			dashboard->reslice = false;
		}
		
		//background color
		glClearColor(0.0, 0.0, 0.0, 1.0f);

		// Clear the color and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (dashboard->addTransferFunction)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		//set vertex array object
		glBindVertexArray(VAO);
		glm::mat4 MVP = Projection * View * Model;

		// Render
		glDepthMask(true);

		
		//bind shader
		shader.use();
		//set the shader uniforms
		glUniform1i(glGetUniformLocation(shader.program, "s"), 
													dashboard->sampleRate);
		glUniform1i(glGetUniformLocation(shader.program, "addTransferFunction"),
														dashboard->addTransferFunction);
		shader.setColor("ObjectColor", dashboard->objectColor);
		shader.setMatrix4("MVP", MVP);
		glUniform1fv(glGetUniformLocation(shader.program, "alphaTransferFunction"),
													256, dashboard->alphaTransferFunction);
		
		if(!dashboard->addTransferFunction)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
		glDrawArrays(GL_TRIANGLES, 0,  slicer.totalVertices);

		//render model

		glBindVertexArray(0);
		glDisable(GL_BLEND);
	
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//render dashboard
		dashboard->render(screen);

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteTextures(1, &textureID);
	delete[] slicedVertices;
	
	//Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}



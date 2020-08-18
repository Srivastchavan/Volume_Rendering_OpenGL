#pragma once
#include "nanogui/nanogui.h"
#include "camera.h"
#include <nanogui/slider.h>

using namespace nanogui;

enum Model_Name {
	TEAPOT = 0,
	BUCKY = 1,
	BONSAI = 2,
	HEAD = 3
};
struct RawFile{
	std::string filename;
	glm::vec3 dimension;
	glm::vec3 initialRotation;
	int initialIsoValue;
};
	
enum Render_Modes {
	LINES = GL_LINE,
	POINTS = GL_POINT,
	TRIANGLES = GL_TRIANGLES
};

class DashBoard
{
	public:
		DashBoard(Screen *screen,GLFWwindow* window,Camera *windowCamera);
		void createConfigurationForm(Screen *screen);
		Window* createTransferFunctionForm(Screen *screen);
		Window* createModelPositioningForm(Screen *screen);
		void render(Screen *screen);
		
		RawFile *selectedRawFile;
		std::vector<RawFile> rawfiledata;
	
		Camera *camera;
		GLint rotValue = 5;
		int modelRotValue = 5;
		GLuint sampleRate =10;
		float objRotX = 0, objRotY = 0, objRotZ;
		bool reload;
		bool reslice;
		GLfloat viewSliderValue = 1.0f;
		Color objectColor = Color(0.5f, 0.2f, 0.3f, 1.0f);
		bool addTransferFunction = false;
		std::string colorBarImagePath;
		static const int num_alpha_sliders = 8;
		GLfloat alphaDataPoints[num_alpha_sliders];
		GLfloat alphaTransferFunction[256];
		~DashBoard();
	private:
		GLuint minSampleRate = 10;
		GLuint maxSampleRate = 2000;
		void initModels();
		Model_Name selectedModel;
		Render_Modes renderMode;
		void resetCamera();
		void redrawGraph(VectorXf &func);
};



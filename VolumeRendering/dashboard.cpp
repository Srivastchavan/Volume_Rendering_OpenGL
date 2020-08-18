#include "dashboard.h"


//constructor
DashBoard::DashBoard(Screen *screen,GLFWwindow* window, Camera* windowCamera)
{
	initModels();
	screen->initialize(window, true);
	camera = windowCamera;
	colorBarImagePath = "images/colorbar.png";
}

//initialize model information 
void DashBoard::initModels()
{
	RawFile rawfile;

	rawfile.filename = "objs/BostonTeapot_256_256_178.raw";
	rawfile.dimension = glm::vec3(256, 256, 178);
	rawfile.initialRotation = glm::vec3(180, 0, 0);
	rawfiledata.push_back(rawfile);

	rawfile.filename = "objs/Bucky_32_32_32.raw";
	rawfile.dimension = glm::vec3(32, 32, 32);
	rawfile.initialIsoValue = 30;
	rawfile.initialRotation = glm::vec3(0, -30, -180);
	rawfiledata.push_back(rawfile);

	rawfile.filename = "objs/Bonsai_512_512_154.raw";
	rawfile.dimension = glm::vec3(512, 512, 154);
	rawfile.initialRotation = glm::vec3(70, 0, 0);
	rawfiledata.push_back(rawfile);

	rawfile.filename = "objs/Head_256_256_225.raw";
	rawfile.dimension = glm::vec3(256, 256, 225);
	rawfile.initialIsoValue = 40;
	rawfile.initialRotation = glm::vec3(5, 20, 0);
	rawfiledata.push_back(rawfile);

	selectedRawFile = &rawfiledata[0];
}

//reset camera to default position
void DashBoard::resetCamera() {
	
	camera->resetCamera(glm::vec3(0.5, 0.5, 2.5), glm::vec3(0.0f, 1.0f, 0.0f));

	reslice = true;
}


//Configuration form
void DashBoard::createConfigurationForm(Screen *screen)
{
	FormHelper *gui = new FormHelper(screen);
	ref<Window> nanoguiWindow = gui->addWindow(Eigen::Vector2i(10, 10), "Configuration Control Bar");
	//Position controls
	gui->addGroup("Position");
		gui->addVariable("X", camera->move.x)->setSpinnable(true);
		gui->addVariable("Y", camera->move.y)->setSpinnable(true);
		gui->addVariable("Z", camera->move.z)->setSpinnable(true);

	//Rotation controls 
	gui->addGroup("Rotation");
		gui->addVariable("Rotate value", rotValue)->setSpinnable(true);
		//(reslice after every rotation)
			gui->addButton("Rotate right+", [this]() {
				camera->addPitch(rotValue);
				reslice = true;
			});
			gui->addButton("Rotate right-", [this]() {
				camera->addPitch(-rotValue);
				reslice = true;
			});
			gui->addButton("Rotate up+", [this]() {
				camera->addYaw(rotValue);
				reslice = true;
			});
			gui->addButton("Rotate up-", [this]() {
				camera->addYaw(-rotValue);
				reslice = true;
			});
			gui->addButton("Rotate front+", [this]() {
				camera->addRoll(rotValue);
				reslice = true;
			});
			gui->addButton("Rotate front-", [this]() {
				camera->addRoll(-rotValue);
				reslice = true;
			});
	
	gui->addGroup("Configuration");

		detail::FormWidget<Model_Name,
			std::integral_constant<bool, true>> *widgetRenderFile = gui->addVariable("Model name", selectedModel, true);
	
	//Model loder control
		widgetRenderFile->setItems({ "TEAPOT ", "BUCKY " , "BONSAI ", "HEAD " });
		widgetRenderFile->setCallback([this,gui](const int &selectedIndex)
		{
			switch (selectedIndex)
			{
			case 0:
				selectedModel = TEAPOT;
				break;
			case 1:
				selectedModel = BUCKY;
				break;
			case 2:
				selectedModel = BONSAI;
				break;
			case 3:
				selectedModel = HEAD;
				break;
			default:
				break;
			}
			selectedRawFile = &rawfiledata[selectedIndex];
			gui->refresh();
		});

		//Render Type
		detail::FormWidget<Render_Modes,
			std::integral_constant<bool, true>> *widgetRender = gui->addVariable("Render type", renderMode, true);
	
		widgetRender->setItems({ "TRIANGLES ", "LINES ", "POINTS " });
		widgetRender->setCallback([this](const int &selectedIndex)
		{
			switch (selectedIndex)
			{
			case 0:
				renderMode = TRIANGLES;
				break;
			case 1:
				renderMode = LINES;
				break;
			case 2:
				renderMode = POINTS;
				break;
			default:
				break;
			}
		});

		//Color bar image
		gui->addVariable("Colorbar image path", colorBarImagePath);
		gui->addButton("Reload Model", [this,gui]() {
			reload = true;
			resetCamera();
			gui->refresh();});

		gui->addButton("Reset Camera", [this, gui]() {
			resetCamera();});

	gui->addGroup("Volume Rendering");
	gui->addVariable("Color", objectColor);
	gui->addVariable("Transfer function sign", addTransferFunction);

	//max sample rate is 2000, dont let the user go beyond that
	detail::FormWidget<GLuint,
		std::integral_constant<bool, true>> *widgetRenderText = gui->addVariable("Sampling rate s(unit slice number)", sampleRate);
	widgetRenderText->setCallback([this,gui]
	(const int &input) {
		if (input < minSampleRate)
			sampleRate = minSampleRate;
		else if (input > maxSampleRate)
			sampleRate = maxSampleRate;
		else 
			sampleRate = input;

		reslice = true;
		gui->refresh();
	});
	widgetRenderText->setSpinnable(true);;

	screen->setVisible(true);
	screen->performLayout();
}

//Transfer function form
Window* DashBoard::createTransferFunctionForm(Screen *screen) {
	
	Window *window = new Window(screen, "Transfer function controls");
	window->setPosition(Vector2i(1205, 15));
	GridLayout *layout =
		new GridLayout(Orientation::Horizontal, 2,
			Alignment::Middle, 15, 5);

	layout->setColAlignment({ Alignment::Fill, Alignment::Fill });
	layout->setSpacing(0, 10);
	window->setLayout(layout);

	/* No need to store a pointer, the data structure will be automatically
	freed when the parent window is deleted */
	{
		new Label(window, "View Slider", "sans-bold");

		Widget *panel = new Widget(window);
		panel->setLayout(new GridLayout(Orientation::Horizontal,2,
			Alignment::Fill, 0, 20));

		Slider *slider = new Slider(panel);
		slider->setValue(1.0f);
		slider->setFixedWidth(200);
		FloatBox<float>  *textBox = new FloatBox<float>(panel,viewSliderValue);
		slider->setCallback([textBox,this](float value) {
			viewSliderValue = value;
			textBox->setValue((value));
			reslice = true; //reslice 
		});
	
		textBox->setFixedSize(Vector2i(100, 15));
		textBox->setAlignment(TextBox::Alignment::Right);
	}
	
		new Label(window, "Transfer function");
		Widget *panel = new Widget(window);
		panel->setLayout(new BoxLayout(Orientation::Horizontal,
			Alignment::Fill, 0, 200));

		Graph *graph = new Graph(panel, "Alpha: (0,1)");
		graph->setHeader("");
		graph->setFooter("Intensity");
		graph->setFixedWidth(290);
		graph->setFixedHeight(280);
		VectorXf &func = graph->values();
		func.resize(256);
		redrawGraph(func);
	
	//alpha sliders - use alpha sliders to draw graph
	for(int i=0;i<num_alpha_sliders;i++)
	{
		new Label(window, "Slider" + std::to_string(i));

		Widget *panel = new Widget(window);
		panel->setLayout(new GridLayout(Orientation::Horizontal, 2,
			Alignment::Fill, 0, 20));
		Slider *slider = new Slider(panel);
		slider->setValue(0.0f);
		slider->setRange(std::make_pair(0.0f, 0.999f));
		slider->setFixedWidth(200);
		const int number = i;

		FloatBox<float>  *textBox = new FloatBox<float>(panel, alphaDataPoints[number]);
		slider->setCallback([textBox, this, number, &func](float value) {
			alphaDataPoints[number] = value;
			textBox->setValue((value));
			
			redrawGraph(func);
			
		});

		textBox->setFixedSize(Vector2i(100, 15));
		textBox->setAlignment(TextBox::Alignment::Right);
	}

	screen->setVisible(true);

	screen->performLayout();
	return window;
}



//model postioning form
Window* DashBoard::createModelPositioningForm(Screen *screen)
{
	FormHelper *gui = new FormHelper(screen);
	ref<Window> nanoguiWindow = gui->addWindow(Eigen::Vector2i(10, 10), "Rotate Model");
	//Rotation controls 
	gui->addGroup("Model Rotation");
	gui->addVariable("Rotate value", modelRotValue)->setSpinnable(true);
	//(reslice after every rotation)
	gui->addButton("Rotate right+", [this]() {
		objRotX += modelRotValue;
		reslice = true;
	});
	gui->addButton("Rotate right-", [this]() {
		objRotX -= modelRotValue;
		reslice = true;
	});
	gui->addButton("Rotate up+", [this]() {
		objRotY += modelRotValue;
		reslice = true;
	});
	gui->addButton("Rotate up-", [this]() {
		objRotY -= modelRotValue;
		reslice = true;
	});
	gui->addButton("Rotate front+", [this]() {
		objRotZ += modelRotValue;

		reslice = true;
	});
	gui->addButton("Rotate front-", [this]() {
		objRotZ -= modelRotValue;

		reslice = true;
	});

	screen->setVisible(true);
	screen->performLayout();

	return nanoguiWindow;
}

void DashBoard::redrawGraph(VectorXf &func)
{  
	int colorIndices[num_alpha_sliders];

	//start and end limits
	colorIndices[num_alpha_sliders - 1] = 255;
	colorIndices[0] = 0;
	func[colorIndices[0]] = alphaDataPoints[0];
	func[colorIndices[num_alpha_sliders - 1]] = alphaDataPoints[num_alpha_sliders - 1];

	//set data points in the function 
	for (int i = 1; i < num_alpha_sliders - 1; i++)
	{
		colorIndices[i] = i * (255 / num_alpha_sliders);
		func[colorIndices[i]] = alphaDataPoints[i];
	}

	//interpolate intermediate points
	for (int i = 0; i < num_alpha_sliders - 1; i++)
	{
		float diff = func[colorIndices[i + 1]] - func[colorIndices[i]];
		float distance = (colorIndices[i + 1] - colorIndices[i]);

		float perUnitDistance = diff / distance;

		for (int j = colorIndices[i] + 1; j < colorIndices[i + 1]; j++) {
			func[j] = func[j - 1] + perUnitDistance;
		}
	}
	for (int i = 0; i < 256; i++)
		alphaTransferFunction[i] = func[i];

}

void DashBoard::render(Screen *screen)
{
	screen->drawWidgets();
}

DashBoard::~DashBoard()
{

}

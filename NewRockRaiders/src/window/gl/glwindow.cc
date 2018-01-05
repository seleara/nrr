#include <iostream>
#include <codecvt>
#include <locale>

#include <GL/glew.h>
#include <imgui/imgui.h>
#include <imgui/examples/opengl3_example/imgui_impl_glfw_gl3.h>
#include <glm/glm.hpp>

#include <nrr/window/gl/glwindow.h>

bool GLWindowImpl::create(const std::string &title, uint32_t width, uint32_t height) {
	if (!glfwInit()) {
		std::cerr << "Unable to initialize GLFW." << std::endl;
		return false;
	}

	size_ = glm::vec2(width, height);

	glfwSetErrorCallback(&GLWindowImpl::errorCallback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
	window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if (!window_) {
		std::cerr << "Unable to initialize GLFW." << std::endl;
		return false;
	}

	glfwMakeContextCurrent(window_);
	//glfwSwapInterval(1);


	ImGui_ImplGlfwGL3_Init(window_, true);

	auto glewErr = glewInit();
	if (glewErr != GLEW_OK) {
		std::cerr << "Unable to initialize GLEW:" << glewGetErrorString(glewErr) << std::endl;
		return false;
	}

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glFrontFace(GL_CCW);
	//glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glfwSetWindowUserPointer(window_, this);
	glfwSetWindowSizeCallback(window_, &GLWindowImpl::resizeCallback_);
	glfwSetCharCallback(window_, &GLWindowImpl::charCallback_);
	glfwSetKeyCallback(window_, &GLWindowImpl::keyCallback_);
	glfwSetMouseButtonCallback(window_, &GLWindowImpl::mouseButtonCallback_);
	glfwSetCursorPosCallback(window_, &GLWindowImpl::mouseMovedCallback_);
	glfwSetScrollCallback(window_, &GLWindowImpl::mouseScrolledCallback_);
	glfwSetWindowFocusCallback(window_, &GLWindowImpl::windowFocusCallback_);

	return true;
}

bool GLWindowImpl::isOpen() const {
	return !glfwWindowShouldClose(window_);
}

bool GLWindowImpl::pollEvents(WindowEvent &event) {
	glfwPollEvents();
	return popEvent(event);
}

void GLWindowImpl::clear(const glm::vec4 &color) {
	glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLWindowImpl::swapBuffers() {
	glfwSwapBuffers(window_);
}

void GLWindowImpl::update() {
	double xPos, yPos;
	glfwGetCursorPos(window_, &xPos, &yPos);
	mousePosition_.x = xPos;
	mousePosition_.y = yPos;
}

void GLWindowImpl::errorCallback(int error, const char *description) {
	std::cerr << "Error: " << error << " | " << description << std::endl;
}

void GLWindowImpl::resizeCallback(int width, int height) {
	if (width == 0 || height == 0)  return;
	size_ = glm::vec2(width, height);
	WindowEvent e;
	e.type = WindowEvent::Type::Resized;
	e.size = size_;

	glViewport(0, 0, width, height);

	/*int maxSamples;
	glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);

	int samples = glm::min(maxSamples, Config::instance().samples());
	//if (samples > 0) --samples;

	//if (Config::instance().samples() > maxSamples) {
	//std::cerr << "Warning: MSAA specified with " << Config::instance().samples() << " samples, only " << maxSamples << " supported." << std::endl;
	//}

	// Multisampling
	glBindRenderbuffer(GL_RENDERBUFFER, multisampleDepthRb_);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT24, width, height);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisampleTex_);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA8, width, height, true);

	// Singlesampling
	glBindRenderbuffer(GL_RENDERBUFFER, singlesampleDepthRb_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glBindTexture(GL_TEXTURE_2D, singlesampleTex_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);*/

	pushEvent(std::move(e));
}

void GLWindowImpl::charCallback(unsigned int codePoint) {
	WindowEvent e;
	e.type = WindowEvent::Type::Text;
	std::wstring_convert<std::codecvt_utf8<unsigned int>, unsigned int> converter;
	e.text = converter.to_bytes(codePoint);
	pushEvent(std::move(e));
}

void GLWindowImpl::keyCallback(int key, int scancode, int action, int mods) {
	WindowEvent e;
	if (action == GLFW_PRESS)
		e.type = WindowEvent::Type::KeyPressed;
	else if (action == GLFW_RELEASE)
		e.type = WindowEvent::Type::KeyReleased;
	else if (action == GLFW_REPEAT)
		e.type = WindowEvent::Type::KeyRepeated;
	else
		return;
	e.key = static_cast<KeyCode>(key);
	pushEvent(std::move(e));
}

void GLWindowImpl::mouseButtonCallback(int button, int action, int mods) {
	WindowEvent e;
	if (action == GLFW_PRESS)
		e.type = WindowEvent::Type::MousePressed;
	else if (action == GLFW_RELEASE)
		e.type = WindowEvent::Type::MouseReleased;
	else
		return;
	e.button = static_cast<MouseButton>(button);
	e.position = mousePosition_;
	pushEvent(std::move(e));
}

void GLWindowImpl::mouseMovedCallback(double xPos, double yPos) {
	WindowEvent e;
	e.type = WindowEvent::Type::MouseMoved;
	e.position = glm::vec2(xPos, yPos);
	e.delta = glm::vec2(xPos - mousePosition_.x, yPos - mousePosition_.y);
	mousePosition_ = e.position;
	//std::cout << "Mouse Moved (" << xPos << ", " << yPos << ")\n";
	pushEvent(std::move(e));
}

void GLWindowImpl::mouseScrolledCallback(double xOffset, double yOffset) {
	WindowEvent e;
	e.type = WindowEvent::Type::MouseScrolled;
	e.delta = glm::vec2(xOffset, yOffset);
	pushEvent(std::move(e));
}

void GLWindowImpl::windowFocusCallback(int focused) {
	isFocused_ = focused == GLFW_TRUE;
}
#include <iostream>

#include <GL/glew.h>
#include <imgui/imgui.h>
#include <imgui/examples/opengl3_example/imgui_impl_glfw_gl3.h>

#include <nrr/window/gl/glwindow.h>

bool GLWindowImpl::create(const std::string &title, uint32_t width, uint32_t height) {
	if (!glfwInit()) {
		std::cerr << "Unable to initialize GLFW." << std::endl;
		return false;
	}

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
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

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
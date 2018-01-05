#pragma once

#include <iostream>

#include <nrr/window/window.h>

#include <GLFW/glfw3.h>

class GLWindowImpl : public WindowImpl {
public:
	bool create(const std::string &title, uint32_t width, uint32_t height) override;
	bool isOpen() const override;
	bool pollEvents(WindowEvent &event) override;
	void clear(const glm::vec4 &color) override;
	void swapBuffers() override;
	void update() override;
private:
	static void errorCallback(int error, const char *description);

	static void resizeCallback_(GLFWwindow *window, int width, int height) {
		auto *ptr = (static_cast<GLWindowImpl *>(glfwGetWindowUserPointer(window)));
		ptr->resizeCallback(width, height);
	}
	void resizeCallback(int width, int height);

	static void charCallback_(GLFWwindow *window, unsigned int codePoint) {
		auto *ptr = (static_cast<GLWindowImpl *>(glfwGetWindowUserPointer(window)));
		ptr->charCallback(codePoint);
	}
	void charCallback(unsigned int codePoint);

	static void keyCallback_(GLFWwindow *window, int key, int scancode, int action, int mods) {
		auto *ptr = (static_cast<GLWindowImpl *>(glfwGetWindowUserPointer(window)));
		ptr->keyCallback(key, scancode, action, mods);
	}
	void keyCallback(int key, int scancode, int action, int mods);

	static void mouseButtonCallback_(GLFWwindow *window, int button, int action, int mods) {
		auto *ptr = (static_cast<GLWindowImpl *>(glfwGetWindowUserPointer(window)));
		ptr->mouseButtonCallback(button, action, mods);
	}
	void mouseButtonCallback(int button, int action, int mods);

	static void mouseMovedCallback_(GLFWwindow *window, double xPos, double yPos) {
		auto *ptr = (static_cast<GLWindowImpl *>(glfwGetWindowUserPointer(window)));
		ptr->mouseMovedCallback(xPos, yPos);
	}
	void mouseMovedCallback(double xPos, double yPos);

	static void mouseScrolledCallback_(GLFWwindow *window, double xOffset, double yOffset) {
		auto *ptr = (static_cast<GLWindowImpl *>(glfwGetWindowUserPointer(window)));
		ptr->mouseScrolledCallback(xOffset, yOffset);
	}
	void mouseScrolledCallback(double xOffset, double yOffset);

	static void windowFocusCallback_(GLFWwindow *window, int focused) {
		auto *ptr = (static_cast<GLWindowImpl *>(glfwGetWindowUserPointer(window)));
		ptr->windowFocusCallback(focused);
	}
	void windowFocusCallback(int focused);

	GLFWwindow *window_;
};
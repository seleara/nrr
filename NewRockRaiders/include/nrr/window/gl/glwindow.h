#pragma once

#include <nrr/window/window.h>

#include <GLFW/glfw3.h>

class GLWindowImpl : public WindowImpl {
public:
	bool create(const std::string &title, uint32_t width, uint32_t height) override;
	bool isOpen() const override;
	bool pollEvents(WindowEvent &event) override;
	void clear(const glm::vec4 &color) override;
	void swapBuffers() override;
private:
	GLFWwindow *window_;
};
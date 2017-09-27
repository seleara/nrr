#pragma once

#include <cstdint>
#include <deque>
#include <string>

#include <glm/glm.hpp>

enum class GraphicsBackend {
	OpenGL,
	Vulkan,
	DirectX
};

struct WindowEvent {
	enum class Type {
		Closed
	} type;
};

class WindowImpl {
public:
	virtual bool create(const std::string &title, uint32_t width, uint32_t height) = 0;
	virtual bool isOpen() const = 0;
	virtual bool pollEvents(WindowEvent &event) = 0;
	virtual void clear(const glm::vec4 &color) = 0;
	virtual void swapBuffers() = 0;
protected:
	void pushEvent(WindowEvent &&event);
	bool popEvent(WindowEvent &event);
private:
	std::deque<WindowEvent> eventQueue_;
};

class Window {
public:
	bool create(const std::string &title, uint32_t width, uint32_t height, GraphicsBackend backend);
	bool isOpen() const;
	bool pollEvents(WindowEvent &event);
	void clear(const glm::vec4 &color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	void swapBuffers();
private:
	GraphicsBackend backend_;
	WindowImpl *ptr_;
};
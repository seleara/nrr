#pragma once

#include <cstdint>
#include <deque>
#include <string>

#include <glm/glm.hpp>

#include <nrr/window/input.h>

enum class GraphicsBackend {
	OpenGL,
	Vulkan,
	DirectX
};

struct WindowEvent {
	enum class Type {
		None,
		Resized,
		MouseMoved,
		MouseScrolled,
		MousePressed,
		MouseReleased,
		KeyPressed,
		KeyReleased,
		KeyRepeated,
		Text
	} type;
	union {
		glm::vec2 size;
		glm::vec2 position;
	};
	glm::vec2 delta;
	union {
		KeyCode key;
		MouseButton button;
	};
	std::string text;
	WindowEvent() {}
};

class WindowImpl {
public:
	virtual bool create(const std::string &title, uint32_t width, uint32_t height) = 0;
	virtual bool isOpen() const = 0;
	virtual bool pollEvents(WindowEvent &event) = 0;
	virtual void clear(const glm::vec4 &color) = 0;
	virtual void swapBuffers() = 0;
	virtual void update() = 0;
protected:
	void pushEvent(WindowEvent &&event);
	bool popEvent(WindowEvent &event);
	glm::vec2 size_;
	glm::vec2 mousePosition_;
	bool isFocused_ = true;
private:
	friend class Window;
	std::deque<WindowEvent> eventQueue_;
};

class Window {
public:
	bool create(const std::string &title, uint32_t width, uint32_t height, GraphicsBackend backend);
	bool isOpen() const;
	bool pollEvents(WindowEvent &event);
	void clear(const glm::vec4 &color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	void swapBuffers();
	void update();
	const glm::vec2 size() const {
		return ptr_->size_;
	}

	const glm::vec2 mousePosition() const {
		return ptr_->mousePosition_;
	}

	bool isFocused() const {
		return ptr_->isFocused_;
	}

	static Window &main() {
		if (!main_) throw std::runtime_error("Main window not yet set.");
		return *main_;
	}

	void setMain() {
		main_ = this;
	}
private:
	GraphicsBackend backend_;
	WindowImpl *ptr_;
	static Window *main_;
};
#include <nrr/window/window.h>

#include <nrr/window/gl/glwindow.h>

void WindowImpl::pushEvent(WindowEvent &&event) {
	eventQueue_.push_back(std::move(event));
}

bool WindowImpl::popEvent(WindowEvent &event) {
	if (eventQueue_.size() > 0) {
		event = eventQueue_.front();
		eventQueue_.pop_front();
		return true;
	}
	return false;
}

bool Window::create(const std::string &title, uint32_t width, uint32_t height, GraphicsBackend backend) {
	backend_ = backend;
	switch (backend_) {
	case GraphicsBackend::OpenGL:
		ptr_ = new GLWindowImpl();
		break;
	}
	return ptr_->create(title, width, height);
}

bool Window::isOpen() const {
	return ptr_->isOpen();
}

bool Window::pollEvents(WindowEvent &event) {
	return ptr_->pollEvents(event);
}

void Window::clear(const glm::vec4 &color) {
	ptr_->clear(color);
}

void Window::swapBuffers() {
	ptr_->swapBuffers();
}
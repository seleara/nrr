#pragma once

#include <nrr/window/window.h>

class Engine {
public:
	int run();
private:
	bool initialize();

	Window window_;
};
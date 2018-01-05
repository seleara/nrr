#pragma once

#include <nrr/window/window.h>
#include <nrr/ecs/ecs.h>
#include <nrr/resource/wadarchive.h>

class Engine {
public:
	int run();
private:
	bool initialize();

	bool preloadShaders();
	bool preloadTextures();
	bool setupUniforms();
	bool setupCamera();

	Window window_;
	WadArchive archive_;
	EntityManager em_;
};

#include <iostream>
#include <sstream>

#include <nrr/debug/archiveviewer.h>
#include <nrr/debug/glviewer.h>
#include <nrr/debug/ecsviewer.h>

#include <nrr/engine.h>

#include <nrr/math/transform.h>
#include <nrr/math/uniformbuffer.h>
#include <nrr/sprite/sprite.h>
#include <nrr/sprite/spriterenderingsystem.h>
#include <nrr/model/modelrenderingsystem.h>
#include <nrr/level/levelsystem.h>
#include <nrr/level/level.h>
#include <nrr/math/camera.h>
#include <nrr/math/camerasystem.h>
#include <nrr/math/clock.h>
#include <nrr/math/time.h>

#include <nrr/resource/shader/shader.h>
#include <nrr/util/configparser.h>

#include <nrr/model/model.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui/imgui.h>
#include <imgui/examples/opengl3_example/imgui_impl_glfw_gl3.h>


int Engine::run() {
	if (!initialize()) return -1;

	ArchiveViewer archiveViewer(archive_);
	OpenGLViewer glViewer;

	ECSViewer ecsViewer(em_);

	em_.registerSystem<SpriteRenderingSystem>();
	em_.registerSystem<ModelRenderingSystem>();
	em_.registerSystem<LevelSystem>();
	em_.registerSystem<CameraSystem>();

	auto test = em_.create();
	test.add<TransformComponent>();
	//test.add<SpriteComponent>()->texture.rectangle().load(archive, "languages/loading.bmp");

	if (!setupCamera()) return -1;

	ConfigParser cfg;
	cfg.parse(archive_, "lego.cfg");
	std::cout << "LegoRR/Main/MusicOn = " << cfg.get<bool>("LegoRR/Main/MusicOn") << std::endl;
	auto col = cfg.get<glm::vec3, true>("Lego*/Main/FPLightRGB");
	auto col2 = cfg.get<glm::ivec3>("Lego*/Main/FPLightRGB");
	std::cout << "Lego*/Main/FPLightRGB = " << col.x << ", " << col.y << ", " << col.z << std::endl;
	std::cout << "Lego*/Main/FPLightRGB = " << col2.x << ", " << col2.y << ", " << col2.z << std::endl;

	auto raider1 = em_.create();
	//auto raider2 = em_.create();
	raider1.add<TransformComponent>()->position = glm::vec3(4, 0, 0);
	//raider2.add<TransformComponent>()->position = glm::vec3(-4, 0, 0);
	auto model = raider1.add<ModelComponent>();
	//auto model2 = raider2.add<ModelComponent>();
	//model.load(archive, "buildings/barracks/barracks.ae");
	//model.load(archive, "buildings/powerstation/powerstation.ae");
	model->load(archive_, "mini-figures/pilot/pilot.ae");
	model->play("Activity_Drill");
	//model2->load(archive, "mini-figures/pilot/pilot.ae");
	//model2->play("Activity_Eat");

	auto captain = em_.create();
	auto capT = captain.add<TransformComponent>();
	capT->position = glm::vec3(0, 0, 0);
	capT->scale = glm::vec3(10, 10, 10);
	capT->rotation = glm::angleAxis(3.1415f, glm::vec3(1, 0, 0)) * glm::angleAxis(1.58f, glm::vec3(1, 0, 0));
	auto capModel = captain.add<ModelComponent>();
	capModel->create(archive_, "Captain");
	capModel->playExternal("mini-figures/captain/new_captain_point_talking.lws");

	auto level = em_.create();
	level.add<Level>()->load(cfg, archive_, 1);

	//auto barracks = em_.create();
	//barracks.add<TransformComponent>()->position = glm::vec3(-20, 0, 0);
	//auto barracksModel = barracks.add<ModelComponent>(archive, "buildings/toolstation/toolstation.ae", "Activity_Stand");
	//barracksModel->load(archive, "buildings/barracks/barracks.ae");
	//barracksModel->load(archive, "vehicles/lmlp/lmlp.ae");
	//barracksModel->play("Activity_Stand");

	/*for (int i = 0; i < 20; ++i) {
		auto raider = em_.create();
		raider.add<TransformComponent>()->position = glm::vec3(4, 0, 4 * i);
		auto model = raider.add<ModelComponent>();
		model->load(archive_, "mini-figures/pilot/pilot.ae");
		model->play("Activity_Drill");
	}*/

	if (!preloadTextures()) return -1;

	Time::fixedDeltaTime_ = 0.01;

	Clock clock;
	double frameTime = 0;
	double accumulator = 0;

	auto debugWindowOpen = true;

	WindowEvent event;
	while (window_.isOpen()) {
		while (window_.pollEvents(event)) {
			em_.handleEvent(event);
		}

		frameTime = clock.reset();
		if (frameTime > 0.25) {
			frameTime = 0.25;
		}
		Time::totalTime_ += frameTime;
		Time::deltaTime_ = frameTime;
		accumulator += frameTime;

		window_.update();

		while (accumulator >= Time::fixedDeltaTime_) {
			accumulator -= Time::fixedDeltaTime_;
			em_.fixedUpdate();
		}
		em_.update();

		window_.clear(glm::vec4(0.4f, 0.4f, 0.4f, 1.0f));
		em_.render();
		em_.render2d();

		ImGui_ImplGlfwGL3_NewFrame();
		archiveViewer.draw();
		glViewer.draw();
		ecsViewer.draw();

		ImGui::Begin("Coordinates", &debugWindowOpen);
		ImGui::BeginChild("coordinates", ImVec2(0, 0), false);
		auto ct = CameraComponent::main.entity().get<TransformComponent>();
		ImGui::Text("X: %f  Y: %f  Z: %f", ct->position.x, ct->position.y, ct->position.z);
		ImGui::Text("RX: %f  RY: %f  RZ: %f  RW: %f", ct->rotation.x, ct->rotation.y, ct->rotation.z, ct->rotation.w);
		ImGui::EndChild();
		ImGui::End();

		ImGui::Render();

		window_.swapBuffers();
	}

	return 0;
}

bool Engine::initialize() {
	if (!window_.create("NewRR", 1280, 720, GraphicsBackend::OpenGL)) {
		return false;
	}

	archive_.append("data/LegoRR0.wad");
	archive_.append("data/LegoRR1.wad");

	if (!preloadShaders()) return false;

	if (!setupUniforms()) return false;

	return true;
}

bool Engine::preloadShaders() {
	Shader modelShader;
	modelShader.load("data/shaders/model.glsl");
	modelShader.saveCache("model");

	Shader levelShader;
	levelShader.load("data/shaders/level.glsl");
	levelShader.saveCache("level");

	Shader spriteShader;
	spriteShader.load("data/shaders/sprite.glsl");
	spriteShader.saveCache("sprite");

	return true;
}

bool Engine::preloadTextures() {
	Texture whiteTexture;
	std::vector<unsigned char> whitePixel = { 0xff, 0xff, 0xff, 0xff };
	whiteTexture.create(1, 1, &whitePixel[0]);
	whiteTexture.saveCache("white");

	return true;
}

bool Engine::setupUniforms() {
	UniformBuffer::createUniformBuffer<Matrices>("mvp", 0);
	UniformBuffer::bindUniformBuffer("mvp");
	auto mvp = UniformBuffer::uniformBuffer<Matrices>("mvp");
	mvp->projection = glm::perspectiveFov(90.0f, 1280.0f, 720.0f, 0.1f, 1000.0f);
	mvp.update();

	UniformBuffer::createUniformBuffer<Matrices>("mvp2d", 1);
	UniformBuffer::bindUniformBuffer("mvp2d");
	auto mvp2d = UniformBuffer::uniformBuffer<Matrices>("mvp2d");
	mvp2d->projection = glm::ortho(0.0f, 1280.0f, 720.0f, 0.0f);
	mvp2d.update();

	return true;
}

bool Engine::setupCamera() {
	auto camera = em_.create();
	//camera.add<TransformComponent>()->position = glm::vec3(-20, 40, 40);
	auto t = camera.add<TransformComponent>();
	t->position = glm::vec3(0, 160, 0);
	//t->rotation = glm::rotate(t->rotation, 45.0f, glm::vec3(1, 0, 0));
	auto cameraComponent = camera.add<CameraComponent>();
	CameraComponent::main = cameraComponent;
	//cameraComponent->mode = CameraMode::Target;
	//cameraComponent->target = glm::vec3(0, 0, 0);

	return true;
}
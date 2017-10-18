#include <nrr/engine.h>

#include <nrr/resource/wadarchive.h>
#include <nrr/debug/archiveviewer.h>
#include <nrr/debug/glviewer.h>
#include <nrr/debug/ecsviewer.h>

#include <nrr/ecs/ecs.h>
#include <nrr/math/transform.h>
#include <nrr/math/uniformbuffer.h>
#include <nrr/sprite/sprite.h>
#include <nrr/sprite/spriterenderingsystem.h>
#include <nrr/model/modelrenderingsystem.h>
#include <nrr/math/camera.h>
#include <nrr/math/camerasystem.h>
#include <nrr/math/clock.h>
#include <nrr/math/time.h>

#include <imgui/imgui.h>
#include <imgui/examples/opengl3_example/imgui_impl_glfw_gl3.h>

#include <iostream>
#include <sstream>

#include <nrr/resource/shader/shader.h>
#include <nrr/util/configparser.h>

#include <nrr/model/model.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

int Engine::run() {
	if (!initialize()) return -1;

	WadArchive archive;
	archive.append("data/LegoRR0.wad");
	archive.append("data/LegoRR1.wad");

	ArchiveViewer archiveViewer(archive);
	OpenGLViewer glViewer;

	UniformBuffer::createUniformBuffer<Matrices>("mvp", 0);
	UniformBuffer::bindUniformBuffer("mvp");
	auto &mvp = UniformBuffer::uniformBuffer<Matrices>("mvp");
	mvp.projection = glm::perspectiveFov(90.0f, 1280.0f, 720.0f, 0.1f, 1000.0f);
	UniformBuffer::updateUniformBuffer("mvp");

	UniformBuffer::createUniformBuffer<Matrices>("mvp2d", 1);
	UniformBuffer::bindUniformBuffer("mvp2d");
	auto &mvp2d = UniformBuffer::uniformBuffer<Matrices>("mvp2d");
	mvp2d.projection = glm::ortho(0.0f, 1280.0f, 720.0f, 0.0f);
	UniformBuffer::updateUniformBuffer("mvp2d");

	Shader spriteShader;
	spriteShader.load("data/shaders/sprite.glsl");

	EntityManager em;

	ECSViewer ecsViewer(em);

	em.registerSystem<SpriteRenderingSystem>();
	em.registerSystem<ModelRenderingSystem>();
	em.registerSystem<CameraSystem>();

	auto test = em.create();
	test.add<TransformComponent>();
	//test.add<SpriteComponent>()->texture.rectangle().load(archive, "languages/loading.bmp");

	auto camera = em.create();
	camera.add<TransformComponent>()->position = glm::vec3(-8, 16, 16);
	auto cameraComponent = camera.add<CameraComponent>();
	CameraComponent::main = cameraComponent.ptr();
	cameraComponent->mode = CameraMode::Target;
	cameraComponent->target = glm::vec3(0, 0, 0);

	ConfigParser cfg;
	cfg.parse(archive, "lego.cfg");
	std::cout << "LegoRR/Main/MusicOn = " << cfg.get<bool>("LegoRR/Main/MusicOn") << std::endl;
	auto col = cfg.get<glm::vec3, true>("Lego*/Main/FPLightRGB");
	auto col2 = cfg.get<glm::ivec3>("Lego*/Main/FPLightRGB");
	std::cout << "Lego*/Main/FPLightRGB = " << col.x << ", " << col.y << ", " << col.z << std::endl;
	std::cout << "Lego*/Main/FPLightRGB = " << col2.x << ", " << col2.y << ", " << col2.z << std::endl;

	auto raider1 = em.create();
	auto raider2 = em.create();
	raider1.add<TransformComponent>()->position = glm::vec3(4, 0, 0);
	raider2.add<TransformComponent>()->position = glm::vec3(-4, 0, 0);
	auto model = raider1.add<ModelComponent>();
	auto model2 = raider2.add<ModelComponent>();
	//model.load(archive, "buildings/barracks/barracks.ae");
	//model.load(archive, "buildings/powerstation/powerstation.ae");
	model->load(archive, "mini-figures/pilot/pilot.ae");
	model->play("Activity_Drill");
	model2->load(archive, "mini-figures/pilot/pilot.ae");
	model2->play("Activity_Eat");

	auto barracks = em.create();
	barracks.add<TransformComponent>()->position = glm::vec3(-20, 0, 0);
	auto barracksModel = barracks.add<ModelComponent>();
	//barracksModel->load(archive, "buildings/barracks/barracks.ae");
	barracksModel->load(archive, "vehicles/smalldigger/smalldigger.ae");
	barracksModel->play("Activity_Stand");

	Texture whiteTexture;
	std::vector<unsigned char> whitePixel = { 0xff, 0xff, 0xff, 0xff };
	whiteTexture.create(1, 1, &whitePixel[0]);
	whiteTexture.saveCache("white");

	Time::fixedDeltaTime_ = 0.01;

	Clock clock;
	double frameTime = 0;
	double accumulator = 0;

	WindowEvent event;
	while (window_.isOpen()) {
		while (window_.pollEvents(event)) {

		}

		frameTime = clock.reset();
		if (frameTime > 0.25) {
			frameTime = 0.25;
		}
		Time::totalTime_ += frameTime;
		Time::deltaTime_ = frameTime;
		accumulator += frameTime;

		while (accumulator >= Time::fixedDeltaTime_) {
			accumulator -= Time::fixedDeltaTime_;
			em.fixedUpdate();
		}
		em.update();

		window_.clear(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
		em.render();
		em.render2d();

		ImGui_ImplGlfwGL3_NewFrame();
		archiveViewer.draw();
		glViewer.draw();
		ecsViewer.draw();
		ImGui::Render();

		window_.swapBuffers();
	}

	return 0;
}

bool Engine::initialize() {
	if (!window_.create("NewRR", 1280, 720, GraphicsBackend::OpenGL)) {
		return false;
	}
	return true;
}
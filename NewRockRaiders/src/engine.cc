#include <nrr/engine.h>

#include <nrr/resource/wadarchive.h>
#include <nrr/debug/archiveviewer.h>
#include <nrr/debug/glviewer.h>

#include <nrr/ecs/ecs.h>
#include <nrr/math/transform.h>
#include <nrr/math/uniformbuffer.h>
#include <nrr/sprite/sprite.h>
#include <nrr/sprite/spriterenderingsystem.h>
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

	em.registerSystem<SpriteRenderingSystem>();
	em.registerSystem<CameraSystem>();

	auto test = em.create();
	test.add<TransformComponent>();
	//test.add<SpriteComponent>()->texture.rectangle().load(archive, "languages/loading.bmp");

	auto camera = em.create();
	camera.add<TransformComponent>()->position = glm::vec3(-8, 16, 16);
	auto &cameraComponent = camera.add<CameraComponent>();
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

	Model model;
	//model.load(archive, "buildings/barracks/barracks.ae");
	//model.load(archive, "buildings/powerstation/powerstation.ae");
	model.load(archive, "mini-figures/pilot/pilot.ae");

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
			model.fixedUpdate();
		}

		em.update();

		window_.clear(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

		em.render();
		model.render();

		em.render2d();

		ImGui_ImplGlfwGL3_NewFrame();

		/*const char *items[] = { "1 - Driller Night!", "2 - ???" };
		static int currentItem = 0;


		static int selected = 0;
		ImGui::BeginChild("left pane", ImVec2(150, 0), true);
			for (int i = 0; i < 30; i++) {
				std::stringstream label;
				label << "Level " << i;
				if (ImGui::Selectable(label.str().c_str(), selected == i)) {
					selected = i;
				}
			}
		ImGui::EndChild();
		ImGui::SameLine();

		// right
		ImGui::BeginGroup();
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing())); // Leave room for 1 line below us
				ImGui::Text("Level %d", selected);
				ImGui::Separator();
				ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. ");
			ImGui::EndChild();
			ImGui::BeginChild("buttons");
				if (ImGui::Button("Load")) {}
				//ImGui::SameLine();
				//if (ImGui::Button("Save")) {}
			ImGui::EndChild();
		ImGui::EndGroup();*/

		archiveViewer.draw();
		glViewer.draw();

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
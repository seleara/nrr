#include <nrr/engine.h>

#include <nrr/resource/wadarchive.h>
#include <nrr/debug/archiveviewer.h>
#include <nrr/debug/glviewer.h>

#include <nrr/ecs/ecs.h>
#include <nrr/math/transform.h>
#include <nrr/math/uniformbuffer.h>
#include <nrr/sprite/sprite.h>
#include <nrr/sprite/spriterenderingsystem.h>

#include <imgui/imgui.h>
#include <imgui/examples/opengl3_example/imgui_impl_glfw_gl3.h>

#include <iostream>
#include <sstream>

#include <nrr/resource/shader/shader.h>

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
	//auto &mvp = UniformBuffer::uniformBuffer<Matrices>("mvp");
	//mvp.projection = glm::ortho(0, 1280, 720, 0);
	//UniformBuffer::updateUniformBuffer("mvp");

	UniformBuffer::createUniformBuffer<Matrices>("mvp2d", 1);
	UniformBuffer::bindUniformBuffer("mvp2d");
	auto &mvp = UniformBuffer::uniformBuffer<Matrices>("mvp2d");
	mvp.projection = glm::ortho(0.0f, 1280.0f, 720.0f, 0.0f);
	UniformBuffer::updateUniformBuffer("mvp2d");

	Shader spriteShader;
	spriteShader.load("data/shaders/sprite.glsl");

	EntityManager em;

	em.registerSystem<SpriteRenderingSystem>();

	auto test = em.create();
	test.add<Transform>();
	test.add<Sprite>()->texture.rectangle().load(archive, "languages/loading.bmp");

	WindowEvent event;
	while (window_.isOpen()) {
		while (window_.pollEvents(event)) {

		}

		em.update();

		window_.clear(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

		em.render();

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
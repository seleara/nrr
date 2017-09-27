#include <nrr/engine.h>

#include <nrr/resource/wadarchive.h>
#include <nrr/debug/archiveviewer.h>

#include <imgui/imgui.h>
#include <imgui/examples/opengl3_example/imgui_impl_glfw_gl3.h>

#include <iostream>
#include <sstream>

int Engine::run() {
	if (!initialize()) return -1;

	WadArchive archive;
	archive.append("data/LegoRR0.wad");
	archive.append("data/LegoRR1.wad");

	ArchiveViewer archiveViewer(archive);

	WindowEvent event;
	while (window_.isOpen()) {
		while (window_.pollEvents(event)) {

		}

		window_.clear(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

		ImGui_ImplGlfwGL3_NewFrame();

		const char *items[] = { "1 - Driller Night!", "2 - ???" };
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
		ImGui::EndGroup();

		archiveViewer.draw();

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
#pragma once

#include <imgui/imgui.h>

#include <nrr/resource/texture/texture.h>
#include <nrr/resource/shader/shader.h>
#include <nrr/model/model.h>

#include <nrr/ecs/ecs.h>

class ECSViewer {
public:
	ECSViewer(EntityManager &em) : em_(em) {}
	void draw() {
		static bool windowOpen = true;
		ImGui::Begin("ECS Debug Viewer", &windowOpen);
		ImGui::BeginChild("left pane", ImVec2(250, 0), true);
		ImGui::PushID(1236);

		bool entityNodeOpen = ImGui::TreeNode("Entities");
		if (entityNodeOpen) {
			ImGui::NextColumn();

			std::stringstream ss;
			for (const auto &kv : em_.entityComponents_) {
				ss << "Entity " << kv.first;
				if (ImGui::Selectable(ss.str().c_str(), selectedEntity_ == kv.first)) {
					selectedEntity_ = kv.first;
				}
				ss.str("");
			}
			ImGui::TreePop();
		}

		ImGui::PopID();
		ImGui::EndChild();
		ImGui::SameLine();
		// right
		ImGui::BeginChild("right pane", ImVec2(0, 0), false);
		if (selectedEntity_ != (EntityID)-1) {
			//ImGui::Text(selectedTexture_->baseName.c_str());
			//ImGui::Separator();
			/*ImGui::Text("Size: %dx%d", selectedTexture_->size().x, selectedTexture_->size().y);
			ImGui::Text("Mag/Min: %s/%s", "Nearest", "Nearest");
			ImGui::Separator();
			ImGui::BeginChild("preview", ImVec2(0, 0), false);
			//ImGui::TextWrapped(filePreview_.c_str());
			auto *img = (GLTextureResource *)selectedTexture_.get();
			ImGui::Image((void *)(img), ImVec2(img->size().x, img->size().y));
			ImGui::EndChild();*/
			for (auto cid : em_.entityComponents_[selectedEntity_]) {
				//BaseComponent *c = (BaseComponent *)em_.componentPools_[cid]->get(selectedEntity_);
				ImGui::Text(em_.componentNames_[cid].c_str());
			}
		}
		ImGui::EndChild();
		ImGui::End();
	}
private:
	EntityManager &em_;

	EntityID selectedEntity_ = -1;
	//const char *filterNames_[] = { "Nearest", "Linear" };
};
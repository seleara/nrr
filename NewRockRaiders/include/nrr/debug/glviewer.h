#pragma once

#include <imgui/imgui.h>

#include <nrr/resource/texture/texture.h>
#include <nrr/resource/shader/shader.h>

class OpenGLViewer {
public:
	void draw() {
		static bool windowOpen = true;
		ImGui::Begin("OpenGL Debug Viewer", &windowOpen);
		ImGui::BeginChild("left pane", ImVec2(250, 0), true);
		ImGui::PushID(1235);

		bool textureNodeOpen = ImGui::TreeNode("Textures");
		if (textureNodeOpen) {
			ImGui::NextColumn();

			for (const auto &kv : TextureLoader::textures_) {
				if (ImGui::Selectable(kv.first.c_str(), selectedTexture_ == kv.second)) {
					selectedTexture_ = kv.second;
					selectedShader_ = nullptr;
					//previewTexture();
				}
			}
			ImGui::TreePop();
		}

		bool shaderNodeOpen = ImGui::TreeNode("Shaders");
		if (shaderNodeOpen) {
			ImGui::NextColumn();

			for (const auto &kv : ShaderLoader::resources_) {
				if (ImGui::Selectable(kv.first.c_str(), selectedShader_ == kv.second)) {
					selectedShader_ = kv.second;
					selectedTexture_ = nullptr;
					//previewTexture();
				}
			}
			ImGui::TreePop();
		}

		ImGui::PopID();
		ImGui::EndChild();
		ImGui::SameLine();
		// right
		ImGui::BeginChild("right pane", ImVec2(0, 0), false);
		if (selectedTexture_) {
			//ImGui::Text(selectedTexture_->baseName.c_str());
			//ImGui::Separator();
			ImGui::Text("Size: %dx%d", selectedTexture_->size().x, selectedTexture_->size().y);
			ImGui::Text("Mag/Min: %s/%s", "Nearest", "Nearest");
			ImGui::Separator();
			ImGui::BeginChild("preview", ImVec2(0, 0), false);
			//ImGui::TextWrapped(filePreview_.c_str());
			auto img = (GLTextureResource *)selectedTexture_.get();
			ImGui::Image((void *)(img->id()), ImVec2(img->size().x, img->size().y));
			ImGui::EndChild();
		} else if (selectedShader_) {
			//ImGui::Text(selectedTexture_->baseName.c_str());
			//ImGui::Separator();
			//ImGui::Text("Size: %dx%d", selectedTexture_->size().x, selectedTexture_->size().y);
			//ImGui::Text("Mag/Min: %s/%s", "Nearest", "Nearest");
			ImGui::Separator();
			ImGui::BeginChild("preview", ImVec2(0, 0), false);
			ImGui::TextWrapped(selectedShader_->source_.c_str());
			ImGui::EndChild();
		}
		ImGui::EndChild();
		ImGui::End();
	}
private:
	std::shared_ptr<TextureResource> selectedTexture_;
	std::shared_ptr<ShaderResource> selectedShader_;
	//const char *filterNames_[] = { "Nearest", "Linear" };
};
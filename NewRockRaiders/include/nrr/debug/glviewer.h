#pragma once

#include <imgui/imgui.h>

#include <nrr/resource/texture/texture.h>
#include <nrr/resource/shader/shader.h>
#include <nrr/model/model.h>

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
					selectedModel_ = nullptr;

					textureInfo_.references = kv.second.use_count() - 2; // Minus two for the copy in the cache and the copy used here
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
					selectedModel_ = nullptr;
					//previewTexture();
				}
			}
			ImGui::TreePop();
		}
		
		bool modelNodeOpen = ImGui::TreeNode("Models");
		if (modelNodeOpen) {
			ImGui::NextColumn();

			for (const auto &kv : LightwaveModelLoader::resources_) {
				if (ImGui::Selectable(kv.first.c_str(), selectedModel_ == kv.second)) {
					selectedModel_ = kv.second;
					selectedTexture_ = nullptr;
					selectedShader_ = nullptr;

					modelInfo_.references = kv.second.use_count() - 2; // Minus two for the copy in the cache and the copy used here
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
			ImGui::Text("Referenced by %d objects.", textureInfo_.references);
			ImGui::Separator();
			ImGui::BeginChild("preview", ImVec2(0, 0), false);
			//ImGui::TextWrapped(filePreview_.c_str());
			auto *img = (GLTextureResource *)selectedTexture_.get();
			ImGui::Image((void *)(img), ImVec2(img->size().x, img->size().y));
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
		} else if (selectedModel_) {
			auto ae = (AnimatedEntityResource *)selectedModel_.get();
			ImGui::Text(ae->name().c_str());
			ImGui::Text("Meshes: %d, Animations: %d", ae->meshes_.size(), ae->animations_.size());
			ImGui::Text("Referenced by %d objects.", modelInfo_.references);
			ImGui::Separator();
			//ImGui::PushID(1236);
			if (ImGui::TreeNode("Meshes")) {
				ImGui::NextColumn();
				for (auto &kv : ae->meshes_) {
					ImGui::Text(kv.second->name().c_str());
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Animations")) {
				ImGui::NextColumn();
				for (auto &kv : ae->animations_) {
					auto str = kv.first + ": " + kv.second->name();
					ImGui::Text(str.c_str());
				}
				ImGui::TreePop();
			}
			//ImGui::PopID();
		}
		ImGui::EndChild();
		ImGui::End();
	}
private:
	std::shared_ptr<TextureResource> selectedTexture_;
	std::shared_ptr<ShaderResource> selectedShader_;
	std::shared_ptr<ModelResource> selectedModel_;
	struct DebugTextureInfo {
		int references;
	} textureInfo_;
	struct DebugModelInfo {
		int references;
	} modelInfo_;
	//const char *filterNames_[] = { "Nearest", "Linear" };
};
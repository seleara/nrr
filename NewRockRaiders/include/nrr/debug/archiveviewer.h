#pragma once

#include <iostream>
#include <sstream>

#include <nrr/resource/wadarchive.h>
#include <nrr/resource/texture/texture.h>
#include <nrr/resource/texture/gl/gltexture.h>

#include <imgui/imgui.h>

struct ArchiveNode {
	std::string baseName;
	std::string fullName;
	enum Type {
		File, Directory
	} type;
	std::map<std::string, ArchiveNode> children;
};

class ArchiveViewer {
public:
	ArchiveViewer(WadArchive &archive) : archive_(archive) {
		root_.baseName = "Root";
		root_.type = ArchiveNode::Directory;
		for (int i = 0; i < archive_.filePaths_.size(); ++i) {
			const auto &path = archive_.filePaths_[i];
			std::vector<std::pair<int, std::string>> pathSplit = split(path, { '/', '\\' });
			ArchiveNode *current = &root_;
			for (int j = 0; j < pathSplit.size(); ++j) {
				ArchiveNode child;
				child.baseName = pathSplit[j].second;
				child.fullName = path.substr(0, pathSplit[j].first);
				child.type = (j < pathSplit.size() - 1) ? ArchiveNode::Directory : ArchiveNode::File;
				current->children.insert({ pathSplit[j].second, child });
				current = &current->children[pathSplit[j].second];
			}
		}
	}

	void draw() {
		static bool windowOpen = true;
		ImGui::Begin("Archive Viewer", &windowOpen);
			ImGui::BeginChild("left pane", ImVec2(250, 0), true);
				ImGui::PushID(1234);
				populateTree(root_);
				ImGui::PopID();
			ImGui::EndChild();
			ImGui::SameLine();
			// right
			ImGui::BeginChild("right pane", ImVec2(0, 0), false);
				if (selectedFile_) {
					ImGui::Text(selectedFile_->baseName.c_str());
					ImGui::Separator();
					ImGui::Text("Version: %02d   Offset: %08x   Size: %08x", fileInfo_->version, fileInfo_->offset, fileInfo_->size);
					ImGui::Separator();
					ImGui::BeginChild("preview", ImVec2(0, 0), false);
						//ImGui::TextWrapped(filePreview_.c_str());
					switch (previewType_) {
					case 0:
						break;
					case 1:
						if (filePreview_.size() > 0)
							ImGui::InputTextMultiline("##preview text", filePreview_.data(), filePreview_.size(), ImVec2(-1, -1));
						break;
					case 2: {
						auto img = imagePreview_;
						if (img->id() != 0) {
							ImGui::Image((void *)(img), ImVec2(img->size().x, img->size().y));
						}
						break; }
					}
					ImGui::EndChild();
				}
			ImGui::EndChild();
		ImGui::End();
	}
private:
	void populateTree(const ArchiveNode &node) {
		if (node.type == ArchiveNode::File) {
			if (ImGui::Selectable(node.baseName.c_str(), selectedFile_ == &node)) {
				selectedFile_ = &node;
				std::cout << "Selected '" << selectedFile_ << "'." << std::endl;
				previewFile();
			}
		} else {
			bool nodeOpen = ImGui::TreeNode(node.baseName.c_str());
			if (nodeOpen) {
				ImGui::NextColumn();
				for (const auto &c : node.children) {
					populateTree(c.second);
				}
				ImGui::TreePop();
			}
		}
	}

	void previewFile() {
		const auto &node = *selectedFile_;
		if (node.type != ArchiveNode::File) {
			filePreview_.clear();
			previewType_ = 0;
			return;
		}
		const auto &entry = archive_.get(node.fullName);
		auto ext = node.baseName.substr(node.baseName.rfind('.') + 1);
		if (ext == "bmp" || ext == "BMP") {
			filePreview_.clear();
			fileInfo_ = &entry;
			imagePreview_ = (GLTextureResource *)TextureLoader::load(archive_, node.fullName, TextureType::Rectangle).get();
			previewType_ = 2;
			return;
		}
		auto &ifs = archive_.getStream(entry);
		filePreview_.resize(entry.size);
		ifs.read(filePreview_.data(), entry.size);
		fileInfo_ = &entry;
		previewType_ = 1;
	}

	std::vector<std::pair<int, std::string>> split(const std::string &path, const std::vector<char> &seps) {
		std::vector<std::pair<int, std::string>> output;
		std::stringstream buffer;
		const char *c = path.data();
		int index = 0;
		while (*c != 0) {
			bool found = false;
			for (auto s : seps) {
				if (s == *c) {
					if (buffer.str().size() > 0)
						output.push_back({ index, buffer.str() });
					buffer.str("");
					found = true;
					break;
				}
			}
			if (!found)
				buffer << *c;
			++c;
			++index;
		}
		if (buffer.str().size() > 0)
			output.push_back({ index, buffer.str() });
		return output;
	}

	ArchiveNode &find(const std::string &path) {

	}
	WadArchive &archive_;

	ArchiveNode root_;
	const ArchiveNode *selectedFile_ = nullptr;
	const ArchiveEntry *fileInfo_ = nullptr;
	std::vector<char> filePreview_;
	GLTextureResource *imagePreview_;

	int previewType_ = 0;
};
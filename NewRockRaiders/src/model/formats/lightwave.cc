#include <nrr/model/formats/lightwave.h>

#include <nrr/resource/wadarchive.h>
#include <nrr/util/configparser.h>
#include <nrr/resource/shader/shader.h>
#include <nrr/math/time.h>
#include <nrr/math/uniformbuffer.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

void LightwaveAnimation::load(WadArchive &archive, const std::string &path) {
	std::cout << "Loading animation \"" << path << "\".\n";

	const auto &entry = archive.get(path);
	auto &stream = archive.getStream(entry);

	name_ = path;
	
	std::string line;
	auto end = (uint64_t)stream.tellg() + entry.size;
	while (stream.tellg() < end) {
		std::getline(stream, line);
		auto commentPos = line.find(';');
		if (commentPos != std::string::npos)
			line.erase(commentPos);
		StringUtil::trim(line);
		auto tokens = StringUtil::splitRef(line);//StringUtil::split(line);
		if (tokens.size() == 0) continue;
		//std::cout << tokens[0] << std::endl;
		if (tokens[0] == "FramesPerSecond") {
			std::stringstream ss;
			ss << tokens[1];
			ss >> fps_;
		} else if (tokens[0] == "FirstFrame") {
			std::stringstream ss;
			ss << tokens[1];
			ss >> firstFrame_;
			//currentFrame_ = firstFrame_;
		} else if (tokens[0] == "LastFrame") {
			std::stringstream ss;
			ss << tokens[1];
			ss >> lastFrame_;
		} else if (tokens[0] == "LoadObject") {
			std::string baseName(tokens[1]);
			auto baseNamePos = baseName.rfind('\\');
			if (baseNamePos != std::string::npos) {
				baseName.erase(0, baseNamePos + 1);
			}
			
			ObjectInfo objInfo;
			objInfo.name = baseName;

			//auto &meshes = ((AnimatedEntityResource *)model_)->meshes_;
			//auto iter = meshes.find(baseName);
			//if (iter == meshes.end()) {

			//auto mesh = std::make_unique<LightwaveMesh>(model_);

			auto slashPos = path.rfind('/');
			std::string folder;
			if (slashPos)
				folder = path.substr(0, slashPos + 1);

			std::string sharedFolder = "world/shared/";

			auto curPos = stream.tellg();
			std::shared_ptr<ModelMesh> mesh;
			if (archive.exists(folder + baseName)) {
				//mesh->load(archive, folder + baseName);
				mesh = LightwaveMeshLoader::load(archive, folder + baseName);
			} else if (archive.exists(sharedFolder + baseName)) {
				//mesh->load(archive, sharedFolder + baseName);
				mesh = LightwaveMeshLoader::load(archive, sharedFolder + baseName);
			} else {
				// Unable to find mesh in the usual places
				std::cerr << "Warning: Mesh \"" << baseName << "\" not found in LWS folder or world/shared/, skipping.\n";
				continue;
			}
			stream.seekg(curPos);
			objInfo.mesh = mesh.get();
			//meshes.insert({ baseName, mesh });

			/*} else {
				objInfo.mesh = iter->second.get();
			}*/
			objects_.push_back(objInfo);
		} else if (tokens[0] == "AddNullObject") {
			ObjectInfo objInfo;
			objInfo.name = tokens[1];
			objInfo.mesh = nullptr;
			objects_.push_back(objInfo);
		} else if (tokens[0] == "ObjectMotion") {
			std::string name(tokens[1]); // Usually (unnamed)
			auto &obj = objects_.back();
			auto &keyframes = obj.keyframes;

			int channels;
			stream >> channels;
			if (channels != 9) {
				throw std::runtime_error("Keyframe channels != 9 in file \"" + path + "\".");
			}
			int kfCount;
			stream >> kfCount;
			for (int i = 0; i < kfCount; ++i) {
				LightwaveKeyframe kf;
				glm::vec3 angles;
				stream >> kf.position.x >> kf.position.y >> kf.position.z >> angles.y >> angles.x >> angles.z >> kf.scale.x >> kf.scale.y >> kf.scale.z >> kf.frame >> (int &)kf.linearValue >> kf.splineAdjustments.x >> kf.splineAdjustments.y >> kf.splineAdjustments.z;
				angles /= 180.0f / glm::pi<float>();
				/*kf.rotation = glm::rotate(kf.rotation, -angles.x, glm::vec3(1, 0, 0));
				kf.rotation = glm::rotate(kf.rotation, angles.y, glm::vec3(0, 1, 0));
				kf.rotation = glm::rotate(kf.rotation, -angles.z, glm::vec3(0, 0, 1));*/
				kf.rotation = glm::quat(angles);
				keyframes.push_back(kf);
			}
		} else if (tokens[0] == "ParentObject") {
			auto &obj = objects_.back();
			std::stringstream ss;
			ss << tokens[1];
			ss >> obj.parentIndex;
		}
	}
	for (auto &obj : objects_) {
		if (obj.parentIndex > 0) {
			obj.parent = &objects_[obj.parentIndex - 1];
		}
	}
}

void LightwaveAnimation::fixedUpdate() {
	/*time_ += Time::fixedDeltaTime();
	while (time_ >= (1.0 / fps_)) {
		time_ -= (1.0 / fps_);
		++currentFrame_;
		if (currentFrame_ > lastFrame_) {
			currentFrame_ = firstFrame_;
		}
	}
	for (auto &obj : objects_) {
		if (obj.mesh) {
			obj.mesh->fixedUpdate();
		}
	}*/
}

void LightwaveAnimation::render() {
	/*for (auto &obj : objects_) {
		if (obj.mesh) {
			auto &mvp = UniformBuffer::uniformBuffer<Matrices>("mvp");
			mvp.model = calculateMatrix(obj, currentFrame_);
			UniformBuffer::updateUniformBuffer("mvp");
			obj.mesh->render();
		}
	}*/
}

const std::string &LightwaveAnimation::name() const {
	return name_;
}

glm::mat4 LightwaveAnimation::calculateMatrix(ObjectInfo &obj, int frame) {
	glm::mat4 m;
	if (obj.parent) {
		m = calculateMatrix(*obj.parent, frame);
	}
	LightwaveKeyframe *kf1 = nullptr, *kf2 = nullptr;
	for (int i = 0; i < obj.keyframes.size(); ++i) {
		if (obj.keyframes[i].frame <= frame && obj.keyframes[(i + 1) % obj.keyframes.size()].frame > frame) {
			kf1 = &obj.keyframes[i];
			kf2 = &obj.keyframes[i + 1];
		} else if (obj.keyframes[i].frame <= frame && obj.keyframes[(i + 1) % obj.keyframes.size()].frame == firstFrame_ - 1) {
			kf1 = &obj.keyframes[i];
			kf2 = &obj.keyframes[(i + 1) % obj.keyframes.size()];
		}
	}
	if (kf1 == nullptr) {
		kf1 = &obj.keyframes[0];
		kf2 = kf1;
	}
	auto kf = interpolateFrames(*kf1, *kf2, frame);
	auto rotM = glm::toMat4(kf.rotation);
	auto transM = glm::translate(glm::mat4(), kf.position);
	auto scaleM = glm::scale(glm::mat4(), kf.scale);
	m = m * transM * scaleM * rotM;
	return m;
}

LightwaveKeyframe LightwaveAnimation::interpolateFrames(const LightwaveKeyframe &kf1, const LightwaveKeyframe &kf2, int frame) const {
	if (kf1.frame == kf2.frame) return kf1;
	LightwaveKeyframe kf;
	float t;
	if (kf1.frame > kf2.frame) { // Looping back to first frame
		auto newFrame = lastFrame_ + 1;
		t = (float)(frame - kf1.frame) / (float)(newFrame - kf1.frame);
	} else {
		t = (float)(frame - kf1.frame) / (float)(kf2.frame - kf1.frame);
	}
	kf.position = kf1.position * (1.0f - t) + kf2.position * t;
	kf.rotation = glm::lerp(kf1.rotation, kf2.rotation, t);
	kf.scale = kf1.scale * (1.0f - t) + kf2.scale * t;
	kf.linearValue = kf1.linearValue;
	kf.splineAdjustments = kf1.splineAdjustments * (1.0f - t) + kf2.splineAdjustments * t;
	kf.frame = frame;
	return kf;
}

void AnimatedEntityResource::load(WadArchive &archive, const std::string &path) {
	ae_.parse(archive, path);

	archive_ = &archive;

	name_ = path;

	auto slashPos = path.rfind('/');
	if (slashPos)
		folder_ = path.substr(0, slashPos + 1);

	//loadAnimation("Activity_Drill");
}

ModelAnimation *AnimatedEntityResource::loadAnimation(const std::string &animationName) {
	//auto activityStand = ae.get("Lego*/Activities/Activity_Stand");
	auto activity = ae_.get("Lego*/Activities/" + animationName);
	auto animationFile = folder_ + ae_.get("Lego*/" + activity + "/FILE") + ".lws";

	auto anim = LightwaveAnimationLoader::load(*archive_, animationFile);

	/*auto anim = std::make_unique<LightwaveAnimation>(this);
	anim->load(*archive_, animationFile);*/

	animations_.insert({ activity, anim });

	return anim.get();
}

const std::string &AnimatedEntityResource::name() const {
	return name_;
}
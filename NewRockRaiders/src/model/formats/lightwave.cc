#include <nrr/model/formats/lightwave.h>

#include <nrr/resource/wadarchive.h>
#include <nrr/util/binaryreader.h>
#include <nrr/util/configparser.h>
#include <nrr/resource/shader/shader.h>
#include <nrr/math/time.h>
#include <nrr/math/uniformbuffer.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

void LightwaveMesh::load(WadArchive &archive, const std::string &path) {
	const auto &entry = archive.get(path);
	auto &stream = archive.getStream(entry);
	BinaryReader br(stream);

	uint64_t end = (uint64_t)br.tellg() + entry.size;

	auto formTag = br.readString(4);
	if (formTag != "FORM") {
		throw std::runtime_error("Invalid LWO file.");
	}
	auto fileLength = br.readBE<uint32_t>();
	auto lwob = br.readString(4);
	if (lwob != "LWOB") {
		throw std::runtime_error("Invalid LWO file.");
	}

	while (br.tellg() < end) {
		auto idTag = br.readString(4);
		auto chunkSize = br.readBE<uint32_t>();
		if (idTag == "PNTS") {
			points.reserve(chunkSize / 12);
			for (int i = 0; i < chunkSize / 12; ++i) {
				Vertex vert;
				vert.point.x = br.readBE<float>();
				vert.point.y = br.readBE<float>();
				vert.point.z = br.readBE<float>();
				points.push_back(vert);
			}
		} else if (idTag == "SRFS") {
			uint32_t read = 0;
			while (read < chunkSize) {
				auto surfName = br.readString();
				read += surfName.size() + 1;
				if (surfName.size() % 2 == 0) {
					++read;
					br.skip(1);
				}
				surfNames.push_back(surfName);
			}
		} else if (idTag == "POLS") {
			uint32_t read = 0;
			while (read < chunkSize) {
				auto numVerts = br.readBE<uint16_t>();
				if (numVerts == 3) {
					glm::ivec3 face;
					for (int i = 0; i < numVerts; ++i) {
						face[i] = br.readBE<uint16_t>();
					}
					auto surf = br.readBE<int16_t>();
					if (surf < 0) throw std::runtime_error("Detail polygons not supported.");
					/*if (faces.size() < surf + 1) {
						faces.resize(surf + 1);
					}*/
					faces.push_back(face);
					faceSurfIndex.push_back(surf);
				} else if (numVerts == 4) {
					glm::ivec4 face;
					glm::ivec3 face1, face2;
					for (int i = 0; i < numVerts; ++i) {
						face[i] = br.readBE<uint16_t>();
					}
					face1.x = face.x;
					face1.y = face.y;
					face1.z = face.z;
					face2.x = face.x;
					face2.y = face.z;
					face2.z = face.w;
					auto surf = br.readBE<int16_t>();
					if (surf < 0) throw std::runtime_error("Detail polygons not supported.");
					/*if (faces.size() < surf + 1) {
						faces.resize(surf + 1);
					}*/
					faces.push_back(face1);
					faces.push_back(face2);
					faceSurfIndex.push_back(surf);
					faceSurfIndex.push_back(surf);
				} else {
					throw std::runtime_error("Only tris and quads supported at the moment.");
				}
				read += numVerts * 2 + 4;
			}
		} else if (idTag == "SURF") {
			auto name = br.readString();
			uint32_t surfRead = name.size() + 1;
			if (name.size() % 2 == 0) {
				++surfRead;
				br.skip(1);
			}
			LightwaveSurface surf;
			surf.name = name;
			while (surfRead < chunkSize) {
				auto subIdTag = br.readString(4);
				auto subChunkSize = br.readBE<uint16_t>();
				if (subIdTag == "COLR") {
					surf.color.r = br.read<uint8_t>() / 255.0f;
					surf.color.g = br.read<uint8_t>() / 255.0f;
					surf.color.b = br.read<uint8_t>() / 255.0f;
					surf.color.a = br.read<uint8_t>() / 255.0f;
					//if (surf.color.a == 0) surf.color.a = 1.0f;
					surf.color.a = 1.0f;
				} else if (subIdTag == "FLAG") {
					surf.flags = br.readBE<uint16_t>();
				} else if (subIdTag == "LUMI") {
					surf.lumi = br.readBE<uint16_t>() / 256.0f;
				} else if (subIdTag == "VLUM") {
					surf.lumi = br.readBE<float>();
				} else if (subIdTag == "DIFF") {
					surf.diff = br.readBE<uint16_t>() / 256.0f;
				} else if (subIdTag == "VDIF") {
					surf.diff = br.readBE<float>();
				} else if (subIdTag == "SPEC") {
					surf.spec = br.readBE<uint16_t>() / 256.0f;
				} else if (subIdTag == "VSPC") {
					surf.spec = br.readBE<float>();
				} else if (subIdTag == "REFL") {
					surf.refl = br.readBE<uint16_t>() / 256.0f;
				} else if (subIdTag == "VRFL") {
					surf.refl = br.readBE<float>();
				} else if (subIdTag == "TRAN") {
					surf.tran = br.readBE<uint16_t>() / 256.0f;
				} else if (subIdTag == "VTRN") {
					surf.tran = br.readBE<float>();
				} else if (subIdTag == "CTEX") {
					auto type = br.readString();
					if (type.size() % 2 == 0) br.skip(1);
					std::cout << "Texture Type = " << type << "\n";
					surf.ctex.type = type == "Planar Image Map" ? LightwaveTextureType::Planar : LightwaveTextureType::Planar; // TODO
				} else if (subIdTag == "TIMG") { // Hack
					auto timgStr = br.readString();
					if (timgStr.size() % 2 == 0) br.skip(1);
					timgStr = StringUtil::baseName(timgStr);
					auto tokens = StringUtil::split(timgStr);
					auto &timg = tokens[0];
					if (tokens.size() > 1) {
						if (tokens[1] == "(sequence)") {
							surf.ctex.animation = LightwaveTextureAnimation::Sequence;
						}
					}
					auto folder = StringUtil::folder(path);
					auto pos = br.tellg();
					std::cout << "Texture Path = " << (folder + timg) << "\n";
					std::string sharedFolder = "world/shared/";
					if (archive.exists(folder + timg)) {
						surf.ctex.texture.load(archive, folder + timg);
					} else if (archive.exists(sharedFolder + timg)) {
						surf.ctex.texture.load(archive, sharedFolder + timg);
					} else {
						// Not found
						std::cerr << "Warning: Texture \"" << timg << "\" not found in LWO folder or world/shared/, creating dummy texture.\n";
						surf.ctex.texture.create(1, 1, nullptr);
					}
					br.seekg(pos);
				} else if (subIdTag == "TFLG") {
					surf.ctex.flag = br.readBE<uint16_t>();
					std::cout << "Texture Flag = " << surf.ctex.flag << "\n";
				} else if (subIdTag == "TSIZ") {
					surf.ctex.size.x = br.readBE<float>();
					surf.ctex.size.y = br.readBE<float>();
					surf.ctex.size.z = br.readBE<float>();
				} else if (subIdTag == "TCTR") {
					surf.ctex.center.x = br.readBE<float>();
					surf.ctex.center.y = br.readBE<float>();
					surf.ctex.center.z = br.readBE<float>();
				} else {
					br.skip(subChunkSize);
				}
				surfRead += subChunkSize + 6;
			}
			surfs.push_back(surf);
		}
	}

	// Check if there is a UV file; if so, load texcoords from there instead of generating them
	auto uvPath = path.substr(0, path.rfind('.')) + ".uv";
	std::cout << uvPath << std::endl;
	if (archive.exists(uvPath)) {
		const auto &entry = archive.get(uvPath);
		auto &uvStream = archive.getStream(entry);
		uint64_t end = (uint64_t)uvStream.tellg() + entry.size;
		int unknown;
		uvStream >> unknown;
		int surfaces;
		uvStream >> surfaces;
		std::cout << "Unknown = " << unknown << ", Surfs = " << surfaces << std::endl;
		uvStream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::vector<std::string> uvSurfaces, texturePaths;
		uvSurfaces.reserve(surfaces);
		for (int i = 0; i < surfaces; ++i) {
			std::string uvSurfaceName;
			std::getline(uvStream, uvSurfaceName);
			uvSurfaces.push_back(uvSurfaceName);
			std::cout << "Surf: " << uvSurfaceName << "\n";
		}
		for (int i = 0; i < surfaces; ++i) {
			std::string texturePath;
			std::getline(uvStream, texturePath);
			StringUtil::trim(texturePath);
			texturePaths.push_back(texturePath);
			texturePath = StringUtil::baseName(texturePath);
			auto folder = StringUtil::folder(path);
			auto pos = uvStream.tellg();
			std::cout << "Texture Path = " << (folder + texturePath) << "\n";
			std::string sharedFolder = "world/shared/";
			auto &surf = surfs[i];
			surf.ctex.flag |= 0x20;
			std::cout << "Texture Shared Path = " << (sharedFolder + texturePath) << "\n";
			if (archive.exists(folder + texturePath)) {
				surf.ctex.texture.load(archive, folder + texturePath);
			} else if (archive.exists(sharedFolder + texturePath)) {
				surf.ctex.texture.load(archive, sharedFolder + texturePath);
			} else {
				// Not found
				std::cerr << "Warning: Texture \"" << texturePath << "\" not found in LWO folder or world/shared/, creating dummy texture.\n";
				surf.ctex.texture.create(1, 1, nullptr);
			}
			uvStream.seekg(pos);
		}
		int polys;
		stream >> polys;
		for (int i = 0; i < polys; ++i) {
			int polyIndex, vertices;
			stream >> polyIndex >> vertices;
			if (vertices != 3) {
				throw std::runtime_error("Need to delay quad->tri step.");
			}
			for (int j = 0; j < vertices; ++j) {
				float u, v, w;
				stream >> u >> v >> w;
				points[faces[i][j]].uv = glm::vec2(u, v);
			}
		}
	} else {
		// Generate texcoords
		for (int i = 0; i < faces.size(); ++i) {
			const auto &tri = faces[i];
			const auto &surf = surfs[faceSurfIndex[i] - 1];
			const auto &ctr = surf.ctex.center;
			for (int j = 0; j < 3; ++j) {
				auto &v = points[tri[j]];
				switch (surf.ctex.flag & 0x7) {
				case 1: // X Axis
					v.uv.s = (v.point.z - ctr.z) / surf.ctex.size.z + 0.5f;
					v.uv.t = -(v.point.y - ctr.y) / surf.ctex.size.y + 0.5f;
					break;
				case 2: // Y Axis
						// Order?
					v.uv.s = -(v.point.x - ctr.x) / surf.ctex.size.x + 0.5f;
					v.uv.t = -(v.point.z - ctr.z) / surf.ctex.size.z + 0.5f;
					break;
				case 4: // Z Axis
						// Order?
					v.uv.s = (v.point.x - ctr.x) / surf.ctex.size.x + 0.5f;
					v.uv.t = -(v.point.y - ctr.y) / surf.ctex.size.y + 0.5f;
					break;
				}
			}
		}
	}

	// Sort faces by surface
	polygons.resize(surfs.size() + 1);
	for (int i = 0; i < faces.size(); ++i) {
		auto surfIndex = faceSurfIndex[i];
		polygons[surfIndex].push_back(faces[i]);
	}

	buffer_.upload(&points[0], points.size() * 5);
	indexBuffer_.setType(VertexBufferType::Element);
	size_t totalFaceSize = 0;// faces.size();
	for (auto &vec : polygons) {
		totalFaceSize += vec.size();
	}
	indexBuffer_.allocate(totalFaceSize * 3);
	size_t destOffset = 0;
	for (int i = 0; i < polygons.size(); ++i) {
		if (polygons[i].size() == 0) continue;
		indexBuffer_.copy(&polygons[i][0][0], 0, polygons[i].size() * 3, destOffset);
		destOffset += polygons[i].size() * 3;
	}
	//indexBuffer_.copy(&faces[0][0], 0, totalFaceSize, 0);
	indexBuffer_.upload();

	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);
	//mesh.buffer_.allocate(bufferSize);
	//mesh.buffer_.copy(buffer, 0, bufferSize, 0);
	//mesh.buffer_.upload(buffer, bufferSize);
	//mesh.buffer_.upload(mesh.geometry_.data(), mesh.geometry_.size() * sizeof(Mesh::GeometryData) / sizeof(float));
	buffer_.bind();
	auto stride = 5;
	buffer_.setAttribute(0, 3, stride, 0);
	buffer_.setAttribute(1, 2, stride, 3);
	//buffer_.setAttribute(2, 2, stride, 6);
	//buffer_.setAttribute(3, 4, stride, 8);

	//mesh.indexBuffer_.setType(VertexBufferType::Element);
	//mesh.indexBuffer_.upload(mesh.faces_.data(), mesh.faces_.size() * 3);
	indexBuffer_.bind();
	glBindVertexArray(0);
}

void LightwaveMesh::render() {
	Shader shader;
	shader.load("data/shaders/model.glsl");
	shader.bind();

	glm::vec4 white(1.0f, 1.0f, 1.0f, 1.0f);
	glUniform4fv(shader.uniformLocation("color"), 1, &white[0]);

	Texture whiteTexture;
	whiteTexture.loadCache("white");

	glBindVertexArray(vao_);
	int start = 0;
	bool invalid = false;
	for (int i = 0; i < polygons.size(); ++i) {
		if (polygons[i].size() == 0) continue;
		if (i != 0) {
			auto &surf = surfs[i - 1];
			//glUniform4fv(shader.uniformLocation("color"), 1, &surfs[i - 1].color[0]);
			if (surf.ctex.texture.valid()) {
				surf.ctex.texture.bind();
			} else {
				whiteTexture.bind();
				glUniform4fv(shader.uniformLocation("color"), 1, &surf.color[0]);
				invalid = true;
			}
			if (surf.ctex.flag & 32) {
				glUniform1i(shader.uniformLocation("pixelBlending"), true);
				auto colorKey = surf.ctex.texture.pixel(0, surf.ctex.texture.size().y - 1);
				glUniform4fv(shader.uniformLocation("pixelBlendingColor"), 1, &colorKey[0]);
				//std::cout << "pixel blending on surf \"" << surfs[i - 1].name << "\".\n";
				//glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
			} else {
				glUniform1i(shader.uniformLocation("pixelBlending"), false);
				//std::cout << "NO pixel blending on surf \"" << surfs[i - 1].name << "\".\n";
				//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			}
		}
		buffer_.draw(Primitives::Triangles, start, polygons[i].size() * 3, indexBuffer_);
		start += polygons[i].size() * 3;
		if (invalid) {
			glUniform4fv(shader.uniformLocation("color"), 1, &white[0]);
			invalid = false;
		}
	}
	glBindVertexArray(0);
}

void LightwaveAnimation::load(WadArchive &archive, const std::string &path) {
	std::cout << "Loading animation \"" << path << "\".\n";

	const auto &entry = archive.get(path);
	auto &stream = archive.getStream(entry);
	
	std::string line;
	auto end = (uint64_t)stream.tellg() + entry.size;
	while (stream.tellg() < end) {
		std::getline(stream, line);
		auto commentPos = line.find(';');
		if (commentPos != std::string::npos)
			line.erase(commentPos);
		StringUtil::trim(line);
		auto tokens = StringUtil::split(line);
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
			currentFrame_ = firstFrame_;
		} else if (tokens[0] == "LastFrame") {
			std::stringstream ss;
			ss << tokens[1];
			ss >> lastFrame_;
		} else if (tokens[0] == "LoadObject") {
			auto baseName = tokens[1];
			auto baseNamePos = baseName.rfind('\\');
			if (baseNamePos != std::string::npos) {
				baseName.erase(0, baseNamePos + 1);
			}
			
			ObjectInfo objInfo;
			objInfo.name = baseName;

			auto &meshes = ((AnimatedEntityResource *)model_)->meshes_;
			auto iter = meshes.find(baseName);
			if (iter == meshes.end()) {
				auto mesh = std::make_unique<LightwaveMesh>(model_);

				auto slashPos = path.rfind('/');
				std::string folder;
				if (slashPos)
					folder = path.substr(0, slashPos + 1);

				std::string sharedFolder = "world/shared/";

				auto curPos = stream.tellg();
				if (archive.exists(folder + baseName)) {
					mesh->load(archive, folder + baseName);
				} else if (archive.exists(sharedFolder + baseName)) {
					mesh->load(archive, sharedFolder + baseName);
				} else {
					// Unable to find mesh in the usual places
					std::cerr << "Warning: Mesh \"" << baseName << "\" not found in LWS folder or world/shared/, skipping.\n";
					continue;
				}
				stream.seekg(curPos);
				objInfo.mesh = mesh.get();
				meshes.insert({ baseName, std::move(mesh) });
			} else {
				objInfo.mesh = iter->second.get();
			}
			objects_.push_back(objInfo);
		} else if (tokens[0] == "AddNullObject") {
			ObjectInfo objInfo;
			objInfo.name = tokens[1];
			objInfo.mesh = nullptr;
			objects_.push_back(objInfo);
		} else if (tokens[0] == "ObjectMotion") {
			auto name = tokens[1]; // Usually (unnamed)
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
	time_ += Time::fixedDeltaTime();
	while (time_ >= (1.0 / fps_)) {
		time_ -= (1.0 / fps_);
		++currentFrame_;
		if (currentFrame_ > lastFrame_) {
			currentFrame_ = firstFrame_;
		}
	}
}

void LightwaveAnimation::render() {
	for (auto &obj : objects_) {
		if (obj.mesh) {
			auto &mvp = UniformBuffer::uniformBuffer<Matrices>("mvp");
			mvp.model = calculateMatrix(obj, currentFrame_);
			UniformBuffer::updateUniformBuffer("mvp");
			obj.mesh->render();
		}
	}
}

glm::mat4 LightwaveAnimation::calculateMatrix(ObjectInfo &obj, int frame) {
	glm::mat4 m;
	if (obj.parent) {
		m = calculateMatrix(*obj.parent, frame);
	}
	LightwaveKeyframe *kf1 = nullptr, *kf2 = nullptr;
	for (int i = 0; i < obj.keyframes.size() - 1; ++i) {
		if (obj.keyframes[i].frame <= frame && obj.keyframes[i + 1].frame > frame) {
			kf1 = &obj.keyframes[i];
			kf2 = &obj.keyframes[i + 1];
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
	float t = (float)(frame - kf1.frame) / (float)(kf2.frame - kf1.frame);
	kf.position = kf1.position * (1.0f - t) + kf2.position * t;
	kf.rotation = glm::lerp(kf1.rotation, kf2.rotation, t);
	kf.scale = kf1.scale * (1.0f - t) + kf2.scale * t;
	kf.linearValue = kf1.linearValue;
	kf.splineAdjustments = kf1.splineAdjustments * (1.0f - t) + kf2.splineAdjustments * t;
	kf.frame = frame;
	return kf;
}

void AnimatedEntityResource::load(WadArchive &archive, const std::string &path) {
	ConfigParser ae;
	ae.parse(archive, path);

	auto slashPos = path.rfind('/');
	std::string folder;
	if (slashPos)
		folder = path.substr(0, slashPos + 1);

	//auto activityStand = ae.get("Lego*/Activities/Activity_Stand");
	auto activityStand = ae.get("Lego*/Activities/Activity_Drill");
	auto standFile = folder + ae.get("Lego*/" + activityStand + "/FILE") + ".lws";

	auto anim = std::make_unique<LightwaveAnimation>(this);
	anim->load(archive, standFile);

	animations_.insert({ activityStand, std::move(anim) });
}
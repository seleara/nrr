#include <nrr/model/formats/lightwave.h>

#include <iomanip>

#include <nrr/util/binaryreader.h>
#include <nrr/util/string.h>
#include <nrr/math/time.h>
#include <nrr/resource/shader/shader.h>
#include <nrr/resource/texture/texture.h>

void LightwaveMesh::load(WadArchive &archive, const std::string &path) {
	const auto &entry = archive.get(path);
	auto &stream = archive.getStream(entry);
	BinaryReader br(stream);

	name_ = path;

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

	// Read chunks
	while (br.tellg() < end) {
		auto idTag = br.readString(4);
		auto chunkSize = br.readBE<uint32_t>();
		if (idTag == "PNTS") {
			readPoints(chunkSize, br);
		} else if (idTag == "SRFS") {
			readSurfaceNames(chunkSize, br);
		} else if (idTag == "POLS") {
			readPolygons(chunkSize, br);
		} else if (idTag == "SURF") {
			readSurface(chunkSize, br, archive, path);
		}
	}

	// Check if there is a UV file; if so, load texcoords from there instead of generating them
	auto uvPath = path.substr(0, path.rfind('.')) + ".uv";
	//std::cout << uvPath << std::endl;
	if (archive.exists(uvPath)) {
		loadExternalUV(archive, path, uvPath);
	} else {
		generateTexcoords();
	}

	// Split faces into triangles and sort them by surface
	polygons.resize(surfs.size() + 1);
	for (int i = 0; i < faces.size(); ++i) {
		auto surfIndex = faceSurfIndex[i];
		auto tris = polygonToTriangles(faces[i]);
		for (const auto &tri : tris) {
			polygons[surfIndex].push_back(tri);
		}
	}

	// Create buffers with the vertex and index data, making it available to the GPU
	buffer_.upload(&points[0], points.size() * 5);
	indexBuffer_.setType(VertexBufferType::Element);
	size_t totalFaceSize = 0;// faces.size();
	for (const auto &vec : polygons) {
		totalFaceSize += vec.size();
	}
	indexBuffer_.allocate(totalFaceSize * 3);
	size_t destOffset = 0;
	for (const auto &poly : polygons) {
		if (poly.size() == 0) continue;
		indexBuffer_.copy(&poly[0][0], 0, poly.size() * 3, destOffset);
		destOffset += poly.size() * 3;
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

void LightwaveMesh::readPoints(int chunkSize, BinaryReader &br) {
	points.reserve(chunkSize / 12);
	for (int i = 0; i < chunkSize / 12; ++i) {
		Vertex vert;
		vert.point.x = br.readBE<float>();
		vert.point.y = br.readBE<float>();
		vert.point.z = br.readBE<float>();
		points.push_back(vert);
	}
}

void LightwaveMesh::readSurfaceNames(int chunkSize, BinaryReader &br) {
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
}

void LightwaveMesh::readPolygons(int chunkSize, BinaryReader &br) {
	uint32_t read = 0;
	while (read < chunkSize) {
		auto numVerts = br.readBE<uint16_t>();
		std::vector<int> face;
		face.reserve(numVerts);
		for (int i = 0; i < numVerts; ++i) {
			face.push_back(br.readBE<uint16_t>());
		}
		auto surf = br.readBE<int16_t>();
		if (surf < 0) throw std::runtime_error("Detail polygons not supported.");
		faces.push_back(face);
		faceSurfIndex.push_back(surf);
		/*if (numVerts == 3) {
			glm::ivec3 face;
			for (int i = 0; i < numVerts; ++i) {
				face[i] = br.readBE<uint16_t>();
			}
			auto surf = br.readBE<int16_t>();
			if (surf < 0) throw std::runtime_error("Detail polygons not supported.");
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
			faces.push_back(face1);
			faces.push_back(face2);
			faceSurfIndex.push_back(surf);
			faceSurfIndex.push_back(surf);
		} else {
			throw std::runtime_error("Only tris and quads supported at the moment.");
		}*/
		read += numVerts * 2 + 4;
	}
}

void LightwaveMesh::readSurface(int chunkSize, BinaryReader &br, WadArchive &archive, const std::string &path) {
	auto name = br.readString();
	uint32_t surfRead = name.size() + 1;
	if (name.size() % 2 == 0) {
		++surfRead;
		br.skip(1);
	}
	LightwaveSurface surf = {};
	surf.name = name;
	std::cout << surf.name << std::endl;
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
			//std::cout << "Texture Type = " << type << "\n";
			surf.ctex.type = type == "Planar Image Map" ? LightwaveTextureType::Planar : LightwaveTextureType::Planar; // TODO
		} else if (subIdTag == "TIMG") { // Hack
			auto timgStr = br.readString();
			if (timgStr.size() % 2 == 0) br.skip(1);
			timgStr = StringUtil::baseName(timgStr);
			auto tokens = StringUtil::splitRef(timgStr);
			auto timg = tokens[0];
			if (tokens.size() > 1) {
				if (tokens[1] == "(sequence)") {
					surf.ctex.animation = LightwaveTextureAnimation::Sequence;
				}
			}
			auto folder = StringUtil::folder(path);
			auto pos = br.tellg();
			std::cout << "Texture Path = " << (folder + timg) << "\n";
			std::string sharedFolder = "world/shared/";
			std::string folderPath(folder + timg);
			std::string sharedFolderPath(sharedFolder + timg);
			auto &texture = surf.ctex.textures.emplace_back();
			std::string *foundFolder;
			if (archive.exists(folderPath)) {
				texture.load(archive, folderPath);
				foundFolder = &folder;
			} else if (archive.exists(sharedFolderPath)) {
				texture.load(archive, sharedFolderPath);
				foundFolder = &sharedFolder;
			} else {
				// Not found
				std::cerr << "Warning: Texture \"" << timg << "\" not found in LWO folder or world/shared/, creating dummy texture.\n";
				texture.create(1, 1, nullptr);
				foundFolder = nullptr;
			}

			if (surf.ctex.animation == LightwaveTextureAnimation::Sequence && foundFolder) {
				int sequenceId;
				std::stringstream startId;
				auto firstZero = timg.find('0');
				if (firstZero != std::string::npos) {
					startId << timg.substr(firstZero, timg.rfind('.') - firstZero);
					startId >> sequenceId;
					++sequenceId;
					std::stringstream seqName;
					seqName << timg.substr(0, timg.rfind('.') - 3) << std::setfill('0') << std::setw(3) << sequenceId << '.' << timg.substr(timg.rfind('.') + 1);
					//std::cout << "Seq: " << seqName.str() << "\n";
					while (archive.exists(*foundFolder + seqName.str())) {
						auto &seqTex = surf.ctex.textures.emplace_back();
						seqTex.load(archive, *foundFolder + seqName.str());
						++sequenceId;
						seqName.str("");
						seqName << timg.substr(0, timg.rfind('.') - 3) << std::setfill('0') << std::setw(3) << sequenceId << '.' << timg.substr(timg.rfind('.') + 1);
						//std::cout << "Seq: " << seqName.str() << "\n";
					}
				}
			}

			if (surf.ctex.textures.size() > maxSequenceFrames_) {
				maxSequenceFrames_ = surf.ctex.textures.size() - 1;
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

std::vector<glm::ivec3> LightwaveMesh::polygonToTriangles(const std::vector<int> &polygon) const {
	std::vector<glm::ivec3> tris;
	tris.reserve(polygon.size() - 2);
	if (polygon.size() == 3) {
		tris.emplace_back(polygon[0], polygon[1], polygon[2]);
	} else if (polygon.size() == 4) { // Technically could be concave but assume it's not for now I guess
		tris.emplace_back(polygon[0], polygon[1], polygon[2]);
		tris.emplace_back(polygon[0], polygon[2], polygon[3]);
	} else if (polygon.size() > 4) {
		std::cerr << "Warning: Polygon might be concave, in which case this algorithm will fail.\n";
		for (int i = 0; i < polygons.size() - 2; ++i) {
			tris.emplace_back(polygon[0], polygon[i + 1], polygon[i + 2]);
		}
	}
	return tris;
}

void LightwaveMesh::loadExternalUV(WadArchive &archive, const std::string &path, const std::string &uvPath) {
	const auto &entry = archive.get(uvPath);
	auto &uvStream = archive.getStream(entry);
	uint64_t end = (uint64_t)uvStream.tellg() + entry.size;
	int unknown;
	uvStream >> unknown;
	int surfaces;
	uvStream >> surfaces;
	//std::cout << "Unknown = " << unknown << ", Surfs = " << surfaces << std::endl;
	uvStream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::vector<std::string> uvSurfaces, texturePaths;
	uvSurfaces.reserve(surfaces);
	for (int i = 0; i < surfaces; ++i) {
		std::string uvSurfaceName;
		std::getline(uvStream, uvSurfaceName);
		uvSurfaces.push_back(uvSurfaceName);
		//std::cout << "Surf: " << uvSurfaceName << "\n";
	}
	for (int i = 0; i < surfaces; ++i) {
		std::string texturePath;
		std::getline(uvStream, texturePath);
		auto trimmedTexturePath = StringUtil::trim(texturePath);
		texturePaths.push_back(std::string(trimmedTexturePath));
		trimmedTexturePath = StringUtil::baseName(trimmedTexturePath);
		auto folder = StringUtil::folder(path);
		auto pos = uvStream.tellg();
		//std::cout << "Texture Path = " << (folder + texturePath) << "\n";
		std::string sharedFolder = "world/shared/";
		auto &surf = surfs[i];
		surf.ctex.flag |= 0x20;
		//std::cout << "Texture Shared Path = " << (sharedFolder + texturePath) << "\n";
		auto &texture = surf.ctex.textures.emplace_back();
		if (archive.exists(folder + trimmedTexturePath)) {
			texture.load(archive, folder + trimmedTexturePath);
		} else if (archive.exists(sharedFolder + trimmedTexturePath)) {
			texture.load(archive, sharedFolder + trimmedTexturePath);
		} else {
			// Not found
			std::cerr << "Warning: Texture \"" << trimmedTexturePath << "\" not found in LWO folder or world/shared/, creating dummy texture.\n";
			texture.create(1, 1, nullptr);
		}
		uvStream.seekg(pos);
	}
	int polys;
	uvStream >> polys;
	for (int i = 0; i < polys; ++i) {
		int polyIndex, vertices;
		uvStream >> polyIndex >> vertices;
		/*if (vertices != 3) {
			throw std::runtime_error("Need to delay quad->tri step.");
		}*/
		for (int j = 0; j < vertices; ++j) {
			float u, v, w;
			uvStream >> u >> v >> w;
			points[faces[i][j]].uv = glm::vec2(u, v);
		}
	}
}

void LightwaveMesh::generateTexcoords() {
	// Generate texcoords
	for (int i = 0; i < faces.size(); ++i) {
		const auto &pol = faces[i];
		const auto &surf = surfs[faceSurfIndex[i] - 1];
		const auto &ctr = surf.ctex.center;
		for (int j = 0; j < pol.size(); ++j) {
			auto &v = points[pol[j]];
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

void LightwaveMesh::fixedUpdate() {
	for (auto &surf : surfs) {
		if (surf.ctex.animation == LightwaveTextureAnimation::Sequence) {
			surf.ctex.sequenceTime += Time::fixedDeltaTime();
			while (surf.ctex.sequenceTime >= 1.0 / 25.0) { // Temp value
				surf.ctex.sequenceTime -= 1.0 / 25.0;
				++surf.ctex.sequenceFrame;
				if (surf.ctex.sequenceFrame >= surf.ctex.textures.size()) {
					surf.ctex.sequenceFrame = 0;
				}
				std::cout << "";
			}
		}
	}
}

void LightwaveMesh::render() {
	//render(0);
}

void LightwaveMesh::render(int sequenceFrame) {
}

void LightwaveMesh::render(int sequenceFrame, Shader &shader, Texture &whiteTexture) {
	// Ugly hack to avoid constructing these string every time this function is called
	static const std::string modelShaderName = "model";
	static const std::string whiteTextureName = "white";
	static const std::string colorLocationName = "color";
	static const std::string pixelBlendingLocationName = "pixelBlending";
	static const std::string pixelBlendingColorLocationName = "pixelBlendingColor";
	// Ugly hack end

	//Shader shader;
	//shader.loadCache(modelShaderName);
	//shader.bind();

	//Texture whiteTexture;
	//whiteTexture.loadCache(whiteTextureName);

	glBindVertexArray(vao_);
	int start = 0;
	bool invalid = false, blending = false;

	constexpr unsigned int colorLocation = 0; //shader.uniformLocation(colorLocationName);
	constexpr unsigned int pixelBlendingLocation = 1; //shader.uniformLocation(pixelBlendingLocationName);
	constexpr unsigned int pixelBlendingColorLocation = 2; //shader.uniformLocation(pixelBlendingColorLocationName);

	glm::vec4 white(1.0f, 1.0f, 1.0f, 1.0f);
	//glUniform4fv(colorLocation, 1, &white[0]);
	struct RenderSettings {
		TextureResource *texture = nullptr;
		bool pixelBlending = false;
		bool invalid = false;
		glm::vec4 color;
		bool operator==(const RenderSettings &other) {
			return other.texture == texture && other.pixelBlending == pixelBlending && other.invalid == invalid && other.color == color;
		}
	} renderSettings, lastRenderSettings;

	int renderCount = 0;

	for (int i = 1; i < polygons.size(); ++i) {
		if (polygons[i].size() == 0) continue;
		if (i != 0) {
			auto &surf = surfs[i - 1];
			//glUniform4fv(shader.uniformLocation("color"), 1, &surfs[i - 1].color[0]);
			if (surf.ctex.textures.size() != 0) {
				if (surf.ctex.animation == LightwaveTextureAnimation::None) {
					if (surf.ctex.textures[0].valid()) {
						renderSettings.texture = surf.ctex.textures[0].get();
						renderSettings.invalid = false;
						/*if (invalid) {
							glUniform4fv(colorLocation, 1, &white[0]);
							invalid = false;
						}*/
					} else {
						renderSettings.texture = whiteTexture.get();
						renderSettings.invalid = true;
						renderSettings.color = surf.color;
						/*if (!invalid) {
							glUniform4fv(colorLocation, 1, &surf.color[0]);
							invalid = true;
						}*/
					}
				} else if (surf.ctex.animation == LightwaveTextureAnimation::Sequence) {
					// Temporary frame hack
					//texture = &surf.ctex.textures[sequenceFrame % surf.ctex.textures.size()];
					renderSettings.texture = surf.ctex.textures[sequenceFrame % surf.ctex.textures.size()].get();
				}
				if (surf.ctex.flag & 32 && !renderSettings.invalid) {
					renderSettings.pixelBlending = true;
					/*if (!blending) {
						glUniform1i(pixelBlendingLocation, true);
						blending = true;
					}
					auto colorKey = texture->pixel(0, texture->size().y - 1);
					glUniform4fv(pixelBlendingColorLocation, 1, &colorKey[0]);*/
					//std::cout << "pixel blending on surf \"" << surfs[i - 1].name << "\".\n";
					//glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
				} else {
					renderSettings.pixelBlending = false;
					/*_if (blending) {
						glUniform1i(pixelBlendingLocation, false);
						blending = false;
					}*/
					//std::cout << "NO pixel blending on surf \"" << surfs[i - 1].name << "\".\n";
					//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				}
			} else {
				renderSettings.texture = whiteTexture.get();
				renderSettings.invalid = true;
				renderSettings.color = surf.color;
			}
		}
		renderCount += polygons[i].size() * 3;
		if (renderSettings == lastRenderSettings || i < 2) {
			lastRenderSettings = renderSettings;
			continue;
		}
		lastRenderSettings = renderSettings;
		lastRenderSettings.texture->bind();
		if (!lastRenderSettings.invalid) {
			shader.setUniform(colorLocation, white);
		} else {
			shader.setUniform(colorLocation, lastRenderSettings.color);
		}
		if (lastRenderSettings.pixelBlending) {
			shader.setUniform(pixelBlendingLocation, true);
			auto colorKey = lastRenderSettings.texture->pixel(0, lastRenderSettings.texture->size().y - 1);
			shader.setUniform(pixelBlendingColorLocation, colorKey);
		} else {
			shader.setUniform(pixelBlendingLocation, false);
		}
		buffer_.draw(Primitives::Triangles, start, renderCount, indexBuffer_);
		start += renderCount;
		renderCount = 0;
	}
	lastRenderSettings = renderSettings;
	if (renderCount > 0) {
		lastRenderSettings.texture->bind();
		if (!lastRenderSettings.invalid) {
			shader.setUniform(colorLocation, white);
		} else {
			shader.setUniform(colorLocation, lastRenderSettings.color);
		}
		if (lastRenderSettings.pixelBlending) {
			//shader.setUniform(pixelBlendingLocation, true);
			// TODO: Fix this. Basic color key transparency isn't decided by anything in the LWO file but instead by the prefix A###_ in the bmp file itself
			// where ### is the index in the color palette.
			shader.setUniform(pixelBlendingLocation, false);
			auto colorKey = lastRenderSettings.texture->pixel(0, lastRenderSettings.texture->size().y - 1);
			shader.setUniform(pixelBlendingColorLocation, colorKey);
		} else {
			shader.setUniform(pixelBlendingLocation, false);
		}
		buffer_.draw(Primitives::Triangles, start, renderCount, indexBuffer_);
	}
	//glBindVertexArray(0);
}

const std::string &LightwaveMesh::name() const {
	return name_;
}
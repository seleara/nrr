#pragma once

#include <string>
#include <vector>

#include <nrr/model/model.h>
#include <nrr/resource/texture/texture.h>
#include <nrr/util/configparser.h>
#include <nrr/resource/shader/shader.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

enum class LightwaveTextureType {
	Planar
};

enum class LightwaveTextureAnimation {
	None,
	Sequence
};

struct LightwaveTexture {
	LightwaveTextureType type = LightwaveTextureType::Planar;
	LightwaveTextureAnimation animation = LightwaveTextureAnimation::None;
	std::vector<Texture> textures;
	uint16_t flag = 0;
	glm::vec3 size;
	glm::vec3 center;
	double sequenceTime = 250;
	int sequenceFrame = 0; // Used if the texture is a sequence
	bool hasColorKey = false;
	glm::ivec4 colorKey;
};

struct LightwaveSurface {
	std::string name;
	glm::vec4 color;
	uint16_t flags = 0;
	float lumi = 0.0f, diff = 0.0f, spec = 0.0f, refl = 0.0f, tran = 0.0f;
	float glos = 0.0f;
	uint16_t rflt = 0;

	LightwaveTexture ctex;
};

struct Vertex {
	glm::vec3 point;
	glm::vec2 uv;
};

class BinaryReader;

class LightwaveMesh : public ModelMesh {
public:
	//LightwaveMesh(ModelResource *model) : ModelMesh(model) {}
	void load(WadArchive &archive, const std::string &path) override;
	void fixedUpdate() override;
	void render() override;
	void render(int sequenceFrame);
	void renderAdditive(int sequenceFrame, Shader &shader, Texture &whiteTexture);
	void renderOpaque(int sequenceFrame, Shader &shader, Texture &whiteTexture);
	const std::string &name() const override;
private:
	// When rendering a mesh, several polygons can share parameters, in which case we can render them all together with a single draw call
	// Using the RenderSettings struct, we keep track of state changes and only render polygons in batch if the next one processed would
	// need a state change.
	//
	// TODO: A possibly better approach would be to create a texture atlas when loading a mesh and preprocess it (color key, additivity, etc.)
	// resulting in only a single draw call being needed per mesh. However, this would result in an increased load time and would result in
	// premature optimization at this stage, so it will be put on hold for now.
	struct RenderSettings {
		TextureResource *texture = nullptr;
		bool hasColorKey = false;
		glm::ivec4 colorKey;
		bool additive = false;
		bool invalid = false;
		glm::vec4 color;
		bool operator==(const RenderSettings &other) {
			return other.texture == texture && other.hasColorKey == hasColorKey && other.invalid == invalid && other.color == color && other.colorKey == colorKey;
		}
	};

	// When a state change is about to occur, the shader is updated with the render settings
	// of the previous batch of polygons by this function being called
	void updateShader(const RenderSettings &settings, Shader &shader) {
		settings.texture->bind();
		if (!settings.invalid) {
			shader.setUniform(colorLocation_, white_);
		} else {
			shader.setUniform(colorLocation_, settings.color);
		}
		if (settings.hasColorKey) {
			shader.setUniform(hasColorKeyLocation_, true);
			shader.setUniform(colorKeyLocation_, (glm::vec4)settings.colorKey / 255.0f);
		} else {
			shader.setUniform(hasColorKeyLocation_, false);
		}
		if (settings.additive) {
			glBlendFunc(GL_ONE, GL_SRC_ALPHA);
			glDepthMask(GL_FALSE);
		} else {
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			glDepthMask(GL_TRUE);
		}
	};

	// For rendering performance
	static constexpr const char *modelShaderName_ = "model";
	static constexpr const char *whiteTextureName_ = "white";
	static constexpr const char *colorLocationName_ = "color";
	static constexpr const char *pixelBlendingLocationName_ = "pixelBlending";
	static constexpr const char *pixelBlendingColorLocationName_ = "pixelBlendingColor";

	static const glm::vec4 white_;

	static constexpr unsigned int colorLocation_ = 0; //shader.uniformLocation(colorLocationName);
	static constexpr unsigned int hasColorKeyLocation_ = 1; //shader.uniformLocation(pixelBlendingLocationName);
	static constexpr unsigned int colorKeyLocation_ = 2; //shader.uniformLocation(pixelBlendingColorLocationName);

	void loadExternalUV(WadArchive &archive, const std::string &path, const std::string &uvPath);
	void generateTexcoords();

	// LWO read methods
	void readPoints(int chunkSize, BinaryReader &br);
	void readSurfaceNames(int chunkSize, BinaryReader &br);
	void readPolygons(int chunkSize, BinaryReader &br);
	void readSurface(int chunkSize, BinaryReader &br, WadArchive &archive, const std::string &path);

	std::vector<glm::ivec3> polygonToTriangles(const std::vector<int> &polygon) const;

	friend class ModelResource;
	friend class AnimatedEntityResource;
	friend class ModelRenderingSystem;
	std::vector<Vertex> points;
	std::vector<std::string> surfNames;
	std::vector<std::vector<int>> faces;
	std::vector<std::vector<glm::ivec3>> polygons;
	std::vector<int> faceSurfIndex;
	//std::vector<int> faceSurfs;
	std::vector<LightwaveSurface> surfs;
	int maxSequenceFrames_ = 1;

	std::string name_;
};

struct LightwaveKeyframe {
	glm::vec3 position;
	glm::quat rotation; // heading, pitch, bank
	glm::vec3 scale;
	int frame;
	enum LinearValue : int {
		Curved = 0,
		Linear = 1
	} linearValue;
	glm::vec3 splineAdjustments; // tension, continuity, bias (-1.0 ~ 1.0)
};

struct ObjectInfo {
	std::string name;
	ModelMesh *mesh = nullptr; // If mesh is null, this is a NullObject
	int parentIndex = 0;
	ObjectInfo *parent = nullptr;
	std::vector<LightwaveKeyframe> keyframes;
};

class LightwaveAnimation : public ModelAnimation {
public:
	//LightwaveAnimation(ModelResource *model) : ModelAnimation(model) {}
	void load(WadArchive &archive, const std::string &path) override;
	void fixedUpdate() override;
	void render() override;
	const std::string &name() const override;
private:
	std::string name_;
	double fps_ = 0;
	int firstFrame_ = 0;
	int lastFrame_ = 0;
	glm::mat4 calculateMatrix(ObjectInfo &obj, int frame);
	LightwaveKeyframe interpolateFrames(const LightwaveKeyframe &kf1, const LightwaveKeyframe &kf2, int frame) const;
	std::vector<ObjectInfo> objects_;

	friend class AnimationWrapper;
	friend class ModelRenderingSystem;
};

class AnimatedEntityResource : public ModelResource {
public:
	void create(WadArchive &archive, const std::string &name) override;
	void load(WadArchive &archive, const std::string &path) override;

	ModelAnimation *loadAnimation(const std::string &animationName) override;
	ModelAnimation *loadExternalAnimation(const std::string &path) override;
	const std::string &name() const override;
private:
	WadArchive *archive_;
	ConfigParser ae_;
	std::string name_;
	std::string folder_;

	std::string wheelNull, fireNull, drillNull, depositNull;

	friend class OpenGLViewer;
	friend class LightwaveMesh;
	friend class LightwaveAnimation;
};

class AnimatedRaiderResource : public AnimatedEntityResource {
public:
	void load(WadArchive &archive, const std::string &path) override;
};

typedef ResourceLoader<ModelResource, AnimatedEntityResource> LightwaveModelLoader;
typedef ResourceLoader<ModelAnimation, LightwaveAnimation> LightwaveAnimationLoader;
typedef ResourceLoader<ModelMesh, LightwaveMesh> LightwaveMeshLoader;
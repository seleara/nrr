#pragma once

#include <nrr/ecs/ecs.h>

#include <nrr/model/model.h>

class ModelComponent : public Component<ModelComponent>, public ModelWrapper {
public:
	ModelComponent() : Component<ModelComponent>(), ModelWrapper() {}
	ModelComponent(WadArchive &archive, const std::string &path, const std::string &anim) : Component<ModelComponent>(), ModelWrapper() {
		load(archive, path);
		play(anim);
	}
};

class ModelRenderingSystem : public System<ModelRenderingSystem> {
public:
	void fixedUpdate() override;
	void update() override;
	void render() override;
private:
};
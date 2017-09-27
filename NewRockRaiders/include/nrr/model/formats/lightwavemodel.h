#pragma once

#include <nrr/model/model.h>
#include <nrr/resource/resourceloader.h>

class LightwaveModelLoader : public ResourceLoader {
public:
	void load(const std::string &path) override;
private:
};
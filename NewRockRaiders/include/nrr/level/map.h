#pragma once

#include <cstdint>
#include <fstream>
#include <vector>

#include <glm/glm.hpp>

#include <nrr/resource/wadarchive.h>
#include <nrr/util/binaryreader.h>

class Map {
public:
	void load(WadArchive &archive, const std::string &path) {
		const auto &entry = archive.get(path);
		auto &stream = archive.getStream(entry);
		BinaryReader br(stream);
		auto magic = br.readString(4);
		auto fileSize = br.read<uint32_t>();
		size_.x = br.read<uint32_t>();
		size_.y = br.read<uint32_t>();
		data_.reserve(size_.x * size_.y);
		for (int y = 0; y < size_.y; ++y) {
			for (int x = 0; x < size_.x; ++x) {
				data_.push_back(br.read<uint16_t>());
			}
		}
	}
	uint8_t get(int x, int y) const {
		return data_[y * size_.x + x];
	}

	uint8_t get(const glm::ivec2 &position) const {
		return data_[position.y * size_.x + position.x];
	}

	const glm::ivec2 &size() const {
		return size_;
	}
private:
	glm::ivec2 size_;
	std::vector<uint16_t> data_;
};
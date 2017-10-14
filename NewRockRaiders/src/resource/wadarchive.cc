#include <nrr/resource/wadarchive.h>

#include <algorithm>
#include <iostream>
#include <vector>

void WadArchive::append(const std::string &path) {
	++currentStreamIndex_;
	streams_.emplace_back();
	auto &ifs = streams_.back();
	ifs.open(path, std::ios_base::binary);
	if (!ifs.is_open()) {
		throw std::runtime_error("Unable to find archive '" + path + "'.");
	}

	struct WadHeader {
		char signature[4];
		uint32_t fileCount;
	} header;
	ifs.read(reinterpret_cast<char *>(&header), sizeof(header));
	if (strncmp(header.signature, "WWAD", 4) != 0) {
		throw std::runtime_error("Archive '" + path + "' is not a valid WAD archive.");
	}

	filePaths_.reserve(header.fileCount);

	files_.reserve(files_.size() + header.fileCount);
	auto oldEntriesSize = entries_.size();
	entries_.reserve(entries_.size() + header.fileCount);
	std::string filePath;
	for (int i = 0; i < header.fileCount; ++i) {
		std::getline(ifs, filePath, '\0');
		/*auto entry = tryGet(filePath);
		if (!entry) {
			entry = std::make_shared<ArchiveEntry>();
			entries_.push_back(entry);
		}*/
		auto entry = std::make_shared<ArchiveEntry>();
		entries_.push_back(entry);
		std::transform(filePath.begin(), filePath.end(), filePath.begin(), ::tolower);
		bool replaced = insertOrReplace(files_, filePath, std::weak_ptr<ArchiveEntry>(entry));
		if (replaced) {
			std::cout << "Warning: File '" << filePath << "' in archive '" << path << "' replaced previously existing file.\n";
		}
		filePaths_.push_back(filePath);
	}
	originalFilePaths_.reserve(originalFilePaths_.size() + header.fileCount);
	for (int i = 0; i < header.fileCount; ++i) {
		std::getline(ifs, filePath, '\0');
		std::transform(filePath.begin(), filePath.end(), filePath.begin(), ::tolower);
		bool replaced = insertOrReplace(originalFilePaths_, filePath, filePaths_[i]);
		if (replaced) {
			std::cout << "Warning: File '" << filePath << "' in archive '" << path << "' replaced previously existing file.\n";
		}
	}

	char *infoBuffer = new char[header.fileCount * 0x10];
	ifs.read(infoBuffer, header.fileCount * 0x10);
	for (int i = 0; i < header.fileCount; ++i) {
		int eid = oldEntriesSize + i;
		entries_[eid]->version = *(uint32_t *)(infoBuffer + (i * 0x10) + 0x0);
		entries_[eid]->size = *(uint32_t *)(infoBuffer + (i * 0x10) + 0x4);
		entries_[eid]->size = *(uint32_t *)(infoBuffer + (i * 0x10) + 0x8);
		entries_[eid]->offset = *(uint32_t *)(infoBuffer + (i * 0x10) + 0xc);
		entries_[eid]->streamIndex = currentStreamIndex_;
	}

	delete[] infoBuffer;
}

const ArchiveEntry &WadArchive::get(const std::string &path) const {
	auto it = files_.find(path);
	if (it == files_.end()) {
		throw std::runtime_error("Cannot find file '" + path + "'.");
	}
	auto ptr = it->second.lock();
	return *ptr;
}

std::istream &WadArchive::getStream(const ArchiveEntry &entry) {
	auto &ifs = streams_[entry.streamIndex];
	ifs.seekg(entry.offset);
	return ifs;
}
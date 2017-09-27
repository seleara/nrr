#pragma once

#include <map>
#include <memory>
#include <unordered_map>
#include <string>
#include <fstream>

struct ArchiveEntry {
	uint32_t version;
	uint32_t offset;
	uint32_t size;
	int streamIndex;
};

class WadArchive {
public:
	void append(const std::string &path);
	const ArchiveEntry &get(const std::string &path) const;
	std::istream &getStream(const ArchiveEntry &entry);
private:
	friend class ArchiveViewer;

	template <typename S, typename T>
	bool insertOrReplace(std::unordered_map<S, T> &m, S a, T b) {
		auto it = m.find(a);
		if (it != m.end()) {
			it->second = b;
			return true;
		}
		m.insert({ a, b });
		return false;
	}

	int currentStreamIndex_ = -1;
	std::vector<std::ifstream> streams_;
	std::vector<std::string> filePaths_;
	std::vector<std::shared_ptr<ArchiveEntry>> entries_;
	std::unordered_map<std::string, std::weak_ptr<ArchiveEntry>> files_;
	std::unordered_map<std::string, std::string> originalFilePaths_;
};
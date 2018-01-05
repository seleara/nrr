#pragma once

class WadArchive;

class Resource {
public:
	virtual void create(WadArchive &archive, const std::string &name) {};
	virtual void load(WadArchive &archive, const std::string &path) {};
	virtual void load(const std::string &path) {};
};
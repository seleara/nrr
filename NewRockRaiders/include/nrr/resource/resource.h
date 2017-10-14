#pragma once

class WadArchive;

class Resource {
public:
	virtual void load(WadArchive &archive, const std::string &path) = 0;
};
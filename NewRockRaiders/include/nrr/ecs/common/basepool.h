#pragma once

#include <vector>

class BasePool {
public:
	BasePool(size_t objectSize, size_t objectsPerBlock);

	virtual ~BasePool();

	void allocate(size_t count);

	void *get(int n);

	const void *get(int n) const;

	virtual void destroy(int n) = 0;
private:
	size_t objectSize_;
	size_t objectsPerBlock_;
	size_t blockCount_;
	std::vector<char *> blocks_;
};
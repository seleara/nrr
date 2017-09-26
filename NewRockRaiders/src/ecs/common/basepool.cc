#include <nrr/ecs/common/basepool.h>

BasePool::BasePool(size_t objectSize, size_t objectsPerBlock) : objectSize_(objectSize), objectsPerBlock_(objectsPerBlock) {}

BasePool::~BasePool() {
	for (auto *block : blocks_) {
		delete[] block;
	}
}

void BasePool::allocate(size_t count) {
	if (count < blockCount_) return;
	for (int i = blockCount_; i < count; ++i) {
		char *block = new char[objectSize_ * objectsPerBlock_];
		blocks_.push_back(block);
	}
	blockCount_ = count;
}

void *BasePool::get(int n) {
	if (n < 0 || n > blockCount_) return nullptr;
	return blocks_[n / objectsPerBlock_] + (n % objectsPerBlock_) * objectSize_;
}

const void *BasePool::get(int n) const {
	if (n < 0 || n > blockCount_) return nullptr;
	return blocks_[n / objectsPerBlock_] + (n % objectsPerBlock_) * objectSize_;
}
#pragma once

#include <nrr/ecs/common/basepool.h>

template <typename T>
class Pool : public BasePool {
public:
	explicit Pool(size_t objectsPerBlock) : BasePool(sizeof(T), objectsPerBlock) {}
	virtual ~Pool() {}

	void destroy(int n) override {
		if (n < 0 || n > blockCount_) return;
		T *obj = static_cast<T *>(get(n));
		if (obj == nullptr) return;
		obj->~T();
	}
};
#pragma once

#include <nrr/ecs/common/basepool.h>

template <typename T>
class Pool : public BasePool {
	virtual ~Pool() {}

	void destroy(int n) override {
		if (n < 0 || n > blockCount_) return;
		T *obj = static_cast<T *>(get(n));
		if (obj == nullptr) return;
		obj->~T();
	}
};
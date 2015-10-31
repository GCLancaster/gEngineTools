
#pragma once

#include <vector>
#include "System\Logging\gLogger.h"

namespace GENG
{
	template<typename T>
	void _removeObjectFromVector(std::vector<T> &vec, T obj, bool bDelete = false)
	{
		auto f = std::find(vec.begin(), vec.end(), obj);
		if (f != vec.end())
		{
			if (bDelete)
				delete *f;
			vec.erase(f);
		}
	}
}

#pragma once

#include <utility>
#include <functional>
#include <thread>

namespace GENG
{
	namespace Threads
	{
		typedef std::pair<std::function<void(void*)>, void*> tyFuncPair;

		static unsigned int SGetNumberOfCores()
		{
			return std::thread::hardware_concurrency();
		}
	}
}
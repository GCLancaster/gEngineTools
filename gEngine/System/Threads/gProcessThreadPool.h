
#pragma once

#include <condition_variable>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include "..\..\Useful.h"
#include ".\UsefulThreads.h"

namespace GENG
{
	namespace Threads
	{
		class gProcessThreadPool
		{
		protected:
			std::recursive_mutex m_poolMutex;
			std::recursive_mutex m_quitMutex;

			uint32_t m_numThreads = 0;
			uint32_t m_maxThreads = -1;
			bool m_bQuit = false;
			bool m_bSetup = false;
			bool m_bAtMaxThreads = false;

			std::vector<std::unique_ptr<std::thread>> m_workers;

		public:
			gProcessThreadPool();
			gProcessThreadPool(const uint32_t numThreads, const uint32_t maxNumThreads = -1);
			~gProcessThreadPool();
			
			void Init(const uint32_t numThreads, const uint32_t maxNumThreads = -1);
			void Destroy();
			void AddTask(const std::string & name, const std::vector<tyFuncPair> & funcPairs);
			std::chrono::milliseconds WaitToFinish();

		protected:
			bool _AddThread(const uint32_t numToAdd = 1);
			void _ClearThreads();
			bool _GetQuit();
			void _SetQuit(const bool & setup);
			bool _GetSetup();
			void _SetSetup(const bool & setup);

			static void _RunTask(gProcessThreadPool * pThis, tyFuncPair & pair);

		public:
			static void ScriptRegisterClass(chaiscript::ModulePtr pModule)
			{
				chaiscript::utility::add_class<gProcessThreadPool>(*pModule, "gProcessThreadPool",
					// Constructors
					{
						chaiscript::constructor<gProcessThreadPool()>(),
						chaiscript::constructor<gProcessThreadPool(const uint32_t numThreads, const uint32_t maxNumThreads)>(),
					},
					// Functions
					{
						{ chaiscript::fun(&gProcessThreadPool::Init), "Init" },
						{ chaiscript::fun(&gProcessThreadPool::Destroy), "Destroy" },
						{ chaiscript::fun(&gProcessThreadPool::AddTask), "AddTask" },
						{ chaiscript::fun(&gProcessThreadPool::WaitToFinish), "WaitToFinish" }
					}
				);
			};
		};
	}
};
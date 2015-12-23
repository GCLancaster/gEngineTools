
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
#include "chaiscript\dispatchkit\bad_boxed_cast.hpp"

//GENG::Threads::gProcessThreadPool th;
//th.Init(8);
//
//std::vector<GENG::Threads::tyFuncPair> tasks;
//for (int i = 0; i < 8; i++)
//{
//	int * p = new int;
//	*p = i;
//	GENG::Threads::tyFuncPair pair;
//	pair.first = [](void * data)
//	{
//		int * pi = reinterpret_cast<int*>(data);
//		DBG(*pi);
//	};
//	pair.second = p;
//	tasks.push_back(pair);
//}
//th.AddTask("testTasks", tasks);
//
//th.WaitToFinish();

namespace GENG
{
	namespace Threads
	{
		class gTaskThreadPool
		{
		protected:
			// Task Item Class
			struct gWorkerTask
			{
				std::string m_name;
				tyFuncPair m_task;
				
				gWorkerTask(const std::string & name, const tyFuncPair & pair) : m_name(name), m_task(pair) { };
				~gWorkerTask() {};
			};

			enum gWorkerState
			{
				gwsRunning,
				gwsWaiting,

				gwsNULL
			};

			struct gWorkerInfo
			{
				std::string name;
				gWorkerState state;

				gWorkerInfo() : name(""), state(gwsNULL){};
				~gWorkerInfo(){};
			};

			std::recursive_mutex m_poolMutex;
			std::recursive_mutex m_quitMutex;

			std::recursive_mutex m_taskMutex;
			std::condition_variable_any m_taskCond;
			std::deque<std::shared_ptr<gWorkerTask>> m_taskQueue;

			uint32_t m_numThreads     = 0;
			uint32_t m_maxThreads = -1;
			bool m_bQuit         = false;
			bool m_bAtMaxThreads = false;
			
			std::vector<std::unique_ptr<std::thread>> m_workers;
			std::map<std::thread::id, gWorkerInfo> m_workerInfo;			

		public:
			gTaskThreadPool();
			gTaskThreadPool(const uint32_t numThreads, const uint32_t maxNumThreads = -1);
			~gTaskThreadPool();

			void Init(const uint32_t numThreads, const uint32_t maxNumThreads = -1);
			void Destroy();
			bool AddThread(const uint32_t numToAdd = 1);
			void AddTask(const std::string & name, const tyFuncPair & funcPair);
			void ClearThreads();
			void ClearTaskQueue();
			std::chrono::milliseconds WaitToFinish();

		protected:
			bool _GetNextTask(std::shared_ptr<gWorkerTask> & task);
			bool _GetQuit();
			void _SetQuit(const bool & quit);

			static void _RunTask(std::shared_ptr<gWorkerTask> & taskPtr, gWorkerInfo * pInfo);
			static int _RunWorkerThread(void * data);

		public:
			static void ScriptRegisterClass(chaiscript::ModulePtr pModule)
			{
				chaiscript::utility::add_class<gTaskThreadPool>(*pModule, "gTaskThreadPool",
					// Constructors
					{
						chaiscript::constructor<gTaskThreadPool()>(),
						chaiscript::constructor<gTaskThreadPool(const uint32_t numThreads, const uint32_t maxNumThreads)>(),
					},
					// Functions
					{
						{ chaiscript::fun(&gTaskThreadPool::Init), "Init" },
						{ chaiscript::fun(&gTaskThreadPool::Destroy), "Destroy" },
						{ chaiscript::fun(&gTaskThreadPool::AddThread), "AddThread" },
						{ chaiscript::fun(&gTaskThreadPool::AddTask), "AddTask" },
						{ chaiscript::fun(&gTaskThreadPool::ClearThreads), "ClearThreads" },
						{ chaiscript::fun(&gTaskThreadPool::ClearTaskQueue), "ClearTaskQueue" }
					}
				);
			};
		};
	};
};
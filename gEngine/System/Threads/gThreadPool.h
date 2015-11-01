
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

namespace GENG
{
	namespace Threads
	{
		class gThreadPool
		{
		protected:
			// Task Item Class
			struct gWorkerTask
			{
				std::string m_name;
				std::function<void(void *)> m_task;
				void * m_data;
				
				gWorkerTask(const std::string & name, std::function<void(void *)> task, void * data) : m_name(name), m_task(task), m_data(data) { };
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

			int m_numThreads     = 0;
			int m_maxThreads     = -1;
			bool m_bQuit         = false;
			bool m_bAtMaxThreads = false;
			
			std::vector<std::unique_ptr<std::thread>> m_workers;
			std::map<std::thread::id, gWorkerInfo> m_workerInfo;
			

		public:
			gThreadPool();
			gThreadPool(const int numThreads, const int maxNumThreads = -1);
			~gThreadPool();

			void Init(const int numThreads, const int maxNumThreads = -1);
			void Destroy();
			bool AddThread(const int numToAdd = 1);
			void AddTask(const std::string & name, std::function<void(void *)> task, void * data);
			void ClearThreads();
			void ClearTaskQueue();
			void WaitToFinish();

		protected:
			bool _GetNextTask(std::shared_ptr<gWorkerTask> & task);
			bool _GetQuit();
			void _SetQuit(const bool & quit);

			static void _RunTask(std::shared_ptr<gWorkerTask> & taskPtr, gWorkerInfo * pInfo);
			static int _RunWorkerThread(void * data);
		};
	};
};
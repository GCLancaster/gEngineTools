
#include "gThreadPool.h"
#include <algorithm>
#include <xrefwrap>

GENG::Threads::gThreadPool::gThreadPool()
{
}

GENG::Threads::gThreadPool::gThreadPool(const int numThreads, const int maxNumThreads /*= -1*/)
{
	Init(numThreads, maxNumThreads);
}

GENG::Threads::gThreadPool::~gThreadPool()
{
	Destroy();
}

void GENG::Threads::gThreadPool::Init(const int numThreads, const int maxNumThreads /*= -1*/)
{
	Destroy();

	_SetQuit(false);

	m_numThreads = numThreads;
	m_maxThreads = maxNumThreads;
	
	AddThread(numThreads);

	LOG("Initalised NumThreads(" << m_numThreads << ") MaxThreads(" << m_maxThreads << ")");
}

void GENG::Threads::gThreadPool::Destroy()
{
	WaitToFinish();

	_SetQuit(true);

	ClearTaskQueue();
	
	ClearThreads();
}

bool GENG::Threads::gThreadPool::AddThread(const int numToAdd /*= 1*/)
{
	std::lock_guard<std::recursive_mutex> guard(m_poolMutex);

	for (int i = 0; i < numToAdd; i++)
	{
		if (m_maxThreads != -1 && m_workers.size() >= m_maxThreads)
		{
			DBG("Cannot add any more threads: Max(" << m_maxThreads << ")");
			return GENG_EXIT_FAILURE;
		}
		m_workers.emplace_back(std::make_unique<std::thread>(_RunWorkerThread, &*this));
		m_numThreads = m_workers.size();
	}

	return GENG_EXIT_SUCCESS;
}

void GENG::Threads::gThreadPool::AddTask(const std::string & name, std::function<void(void *)> task, void * data)
{
	bool bWorkerAvailable = false;

	{
		std::lock_guard<std::recursive_mutex> guard(m_poolMutex);

		for (auto & info : m_workerInfo)
		{
			if (info.second.state == gwsWaiting || info.second.state == gwsNULL)
			{
				bWorkerAvailable = true;
				break;
			}
		}

		if (!bWorkerAvailable)
		{
			if (AddThread()) // Can't any more threads
				bWorkerAvailable = true;
		}
	}

	{
		std::lock_guard<std::recursive_mutex> guard(m_taskMutex);

		m_taskQueue.push_back(std::make_shared<gWorkerTask>(name, task, data));

		DBG("Added task (" << name << ")");

		if (bWorkerAvailable)
			m_taskCond.notify_one();
	}
}

void GENG::Threads::gThreadPool::ClearThreads()
{
	std::lock_guard<std::recursive_mutex> guard(m_poolMutex);
	
	if (m_workers.size() > 0)
	{
		DBG("Joining threads");
		std::for_each(m_workers.begin(), m_workers.end(), [&](std::unique_ptr<std::thread> &thread){ m_taskCond.notify_one(); if (thread->joinable()) thread->join(); });

		m_workers.clear();
		m_workerInfo.clear();
	}

	m_numThreads = 0;
}

void GENG::Threads::gThreadPool::ClearTaskQueue()
{
	std::lock_guard<std::recursive_mutex> guard(m_taskMutex);
	m_taskQueue.clear();
	m_taskCond.notify_all();
}

void GENG::Threads::gThreadPool::WaitToFinish()
{
	int tasksLeft = 0;
	{
		std::lock_guard<std::recursive_mutex> guard(m_taskMutex);
		tasksLeft = m_taskQueue.size();
	}

	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

	while (tasksLeft > 0)
	{
		std::lock_guard<std::recursive_mutex> guard(m_taskMutex);
		tasksLeft = m_taskQueue.size();
	}

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	DBG("Finished waiting. Tasks took " << time.count() << "ms");
}

bool GENG::Threads::gThreadPool::_GetNextTask(std::shared_ptr<gWorkerTask> & task)
{
	std::lock_guard<std::recursive_mutex> guard(m_taskMutex);

	if (m_taskQueue.size() > 0)
	{
		task = m_taskQueue.front();
		m_taskQueue.pop_front();

		return GENG_EXIT_SUCCESS;
	}

	return GENG_EXIT_FAILURE;
}

bool GENG::Threads::gThreadPool::_GetQuit()
{
	std::lock_guard<std::recursive_mutex> guard(m_quitMutex);

	return m_bQuit;
}

void GENG::Threads::gThreadPool::_SetQuit(const bool & quit)
{
	std::lock_guard<std::recursive_mutex> guard(m_quitMutex);

	m_bQuit = quit;
}

void GENG::Threads::gThreadPool::_RunTask(std::shared_ptr<gWorkerTask> & taskPtr, gWorkerInfo * pInfo)
{
	// Run the task
	if (taskPtr != nullptr)
	{
		try
		{
			pInfo->state = gwsRunning;

			pInfo->name = taskPtr->m_name;

			taskPtr->m_task(taskPtr->m_data);

			DBG("Completed task (" << taskPtr->m_name << ")");

			taskPtr.reset();

			pInfo->name = "";
		}
		catch (std::exception * e)
		{
			DBG("Exception! " << e->what());
		}
	}
}

int GENG::Threads::gThreadPool::_RunWorkerThread(void * data)
{
	gThreadPool * m_pool = reinterpret_cast<gThreadPool *>(data);
	if (m_pool == nullptr)
		return 0;

	gWorkerInfo * pInfo = nullptr;
	{
		std::lock_guard<std::recursive_mutex> guard_1(m_pool->m_poolMutex);
		pInfo = &m_pool->m_workerInfo[std::this_thread::get_id()];
	}
	if (pInfo == nullptr)
		return 0;

	std::shared_ptr<gWorkerTask> taskPtr;

	DBG(std::this_thread::get_id() << " initalised");
	
	while (!m_pool->_GetQuit())
	{
		{
			std::unique_lock<std::recursive_mutex> guard_2(m_pool->m_taskMutex);

			// Wait for an task to be added					
			pInfo->state = gwsWaiting;
			while (!m_pool->_GetQuit() && m_pool->m_taskQueue.empty())
			{
				m_pool->m_taskCond.wait(guard_2);
			}

			if (m_pool->_GetQuit())
				break;

			m_pool->_GetNextTask(taskPtr);
		}

		_RunTask(taskPtr, pInfo);
	}

	DBG(std::this_thread::get_id() << " destroyed");

	return 1;
}

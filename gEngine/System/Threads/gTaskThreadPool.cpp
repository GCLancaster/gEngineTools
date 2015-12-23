
#include "gTaskThreadPool.h"
#include <algorithm>
#include <xrefwrap>

GENG::Threads::gTaskThreadPool::gTaskThreadPool()
{
}

GENG::Threads::gTaskThreadPool::gTaskThreadPool(const uint32_t numThreads, const uint32_t maxNumThreads /*= -1*/)
{
	Init(numThreads, maxNumThreads);
}

GENG::Threads::gTaskThreadPool::~gTaskThreadPool()
{
	Destroy();
}

void GENG::Threads::gTaskThreadPool::Init(const uint32_t numThreads, const uint32_t maxNumThreads /*= -1*/)
{
	Destroy();

	_SetQuit(false);

	m_numThreads = numThreads;
	m_maxThreads = maxNumThreads;
	
	AddThread(numThreads);

	LOG("Initalised NumThreads(" << m_numThreads << ") MaxThreads(" << m_maxThreads << ")");
}

void GENG::Threads::gTaskThreadPool::Destroy()
{
	WaitToFinish();

	_SetQuit(true);

	ClearTaskQueue();
	
	ClearThreads();
}

bool GENG::Threads::gTaskThreadPool::AddThread(const uint32_t numToAdd /*= 1*/)
{
	std::lock_guard<std::recursive_mutex> guard(m_poolMutex);

	for (uint32_t i = 0; i < numToAdd; i++)
	{
		if (m_maxThreads != -1 && m_workers.size() >= m_maxThreads)
		{
			DBG("Cannot add any more threads: Max(" << m_maxThreads << ")");
			return GENG_EXIT_FAILURE;
		}
		m_workers.emplace_back(std::make_unique<std::thread>(_RunWorkerThread, &*this));
		m_numThreads = static_cast<uint32_t>(m_workers.size());
	}

	return GENG_EXIT_SUCCESS;
}

void GENG::Threads::gTaskThreadPool::AddTask(const std::string & name, const tyFuncPair & funcPair)
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

		m_taskQueue.push_back(std::make_shared<gWorkerTask>(name, funcPair));

		DBG("Added task (" << name << ")");

		if (bWorkerAvailable)
			m_taskCond.notify_one();
	}
}

void GENG::Threads::gTaskThreadPool::ClearThreads()
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

void GENG::Threads::gTaskThreadPool::ClearTaskQueue()
{
	std::lock_guard<std::recursive_mutex> guard(m_taskMutex);
	m_taskQueue.clear();
	m_taskCond.notify_all();
}

std::chrono::milliseconds GENG::Threads::gTaskThreadPool::WaitToFinish()
{
	size_t tasksLeft = 0;
	{
		std::lock_guard<std::recursive_mutex> guard(m_taskMutex);
		tasksLeft = m_taskQueue.size();
	}

	if (tasksLeft > 0)
	{
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

		while (tasksLeft > 0)
		{
			std::lock_guard<std::recursive_mutex> guard(m_taskMutex);
			tasksLeft = m_taskQueue.size();
		}

		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		return time;
	}
	return std::chrono::milliseconds(0);
}

bool GENG::Threads::gTaskThreadPool::_GetNextTask(std::shared_ptr<gWorkerTask> & task)
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

bool GENG::Threads::gTaskThreadPool::_GetQuit()
{
	std::lock_guard<std::recursive_mutex> guard(m_quitMutex);

	return m_bQuit;
}

void GENG::Threads::gTaskThreadPool::_SetQuit(const bool & quit)
{
	std::lock_guard<std::recursive_mutex> guard(m_quitMutex);

	m_bQuit = quit;
}

void GENG::Threads::gTaskThreadPool::_RunTask(std::shared_ptr<gWorkerTask> & taskPtr, gWorkerInfo * pInfo)
{
	// Run the task
	if (taskPtr != nullptr)
	{
		try
		{
			pInfo->state = gwsRunning;

			pInfo->name = taskPtr->m_name;

			taskPtr->m_task.first(taskPtr->m_task.second);

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

int GENG::Threads::gTaskThreadPool::_RunWorkerThread(void * data)
{
	gTaskThreadPool * m_pool = reinterpret_cast<gTaskThreadPool *>(data);
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

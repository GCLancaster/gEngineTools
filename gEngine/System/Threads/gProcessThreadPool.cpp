
#include "gProcessThreadPool.h"

#include <algorithm>
#include <xrefwrap>

GENG::Threads::gProcessThreadPool::gProcessThreadPool()
{
}

GENG::Threads::gProcessThreadPool::gProcessThreadPool(const uint32_t numThreads, const uint32_t maxNumThreads /*= -1*/)
{
	Init(numThreads, maxNumThreads);
}

GENG::Threads::gProcessThreadPool::~gProcessThreadPool()
{
	Destroy();
}

void GENG::Threads::gProcessThreadPool::Init(const uint32_t numThreads, const uint32_t maxNumThreads /*= -1*/)
{
	Destroy();

	_SetQuit(false);

	m_numThreads = numThreads;
	m_maxThreads = maxNumThreads;

	_AddThread(numThreads);

	LOG("Initalised NumThreads(" << m_numThreads << ") MaxThreads(" << m_maxThreads << ")");
}

void GENG::Threads::gProcessThreadPool::Destroy()
{
	WaitToFinish();

	_SetQuit(true);
	
	_ClearThreads();
}

bool GENG::Threads::gProcessThreadPool::_AddThread(const uint32_t numToAdd /*= 1*/)
{
	std::lock_guard<std::recursive_mutex> guard(m_poolMutex);

	for (uint32_t i = 0; i < numToAdd; i++)
	{
		if (m_maxThreads != -1 && m_workers.size() >= m_maxThreads)
		{
			DBG("Cannot add any more threads: Max(" << m_maxThreads << ")");
			return GENG_EXIT_FAILURE;
		}
		m_workers.emplace_back(std::make_unique<std::thread>());
		m_numThreads = static_cast<uint32_t>(m_workers.size());
	}

	return GENG_EXIT_SUCCESS;
}

void GENG::Threads::gProcessThreadPool::AddTask(const std::string & name, const std::vector<tyFuncPair> & funcPairs)
{
	_SetSetup(false);
		
	const uint32_t numPairs = static_cast<uint32_t>(funcPairs.size());
	if (numPairs > m_numThreads)
		DERROR("Invalid number of function-pairs passed in. Please add more threads.");
	
	int counter = 0;
	for_each(m_workers.begin(), m_workers.end(), [&](std::unique_ptr<std::thread> &thread)
	{
		DBG(counter);
		auto pair = funcPairs[counter];
		thread = std::make_unique<std::thread>(_RunTask, &*this, pair);
		counter++;
	});

	_SetSetup(true);
}

void GENG::Threads::gProcessThreadPool::_ClearThreads()
{
	std::lock_guard<std::recursive_mutex> guard(m_poolMutex);

	if (m_workers.size() > 0)
	{
		std::for_each(m_workers.begin(), m_workers.end(), [&](std::unique_ptr<std::thread> &thread){ if (thread->joinable()) thread->join(); });

		m_workers.clear();
	}

	m_numThreads = 0;
}

std::chrono::milliseconds GENG::Threads::gProcessThreadPool::WaitToFinish()
{	
	if (m_workers.size() > 0)
	{
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
		
		std::for_each(m_workers.begin(), m_workers.end(), [&](std::unique_ptr<std::thread> &thread){ if (thread->joinable()) thread->join(); });

		_SetSetup(false);

		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		return time;
	}
	return std::chrono::milliseconds(0);
}

bool GENG::Threads::gProcessThreadPool::_GetQuit()
{
	std::lock_guard<std::recursive_mutex> guard(m_quitMutex);

	return m_bQuit;
}

void GENG::Threads::gProcessThreadPool::_SetQuit(const bool & quit)
{
	std::lock_guard<std::recursive_mutex> guard(m_quitMutex);

	m_bQuit = quit;
}

bool GENG::Threads::gProcessThreadPool::_GetSetup()
{
	std::lock_guard<std::recursive_mutex> guard(m_quitMutex);

	return m_bSetup;
}

void GENG::Threads::gProcessThreadPool::_SetSetup(const bool & setup)
{
	std::lock_guard<std::recursive_mutex> guard(m_quitMutex);

	m_bSetup = setup;
}

void GENG::Threads::gProcessThreadPool::_RunTask(gProcessThreadPool * pThis, tyFuncPair & pair)
{
	if (pThis == nullptr)
		DERROR("Invalid gProcessThreadPool passed in!");

	while (!pThis->_GetSetup())
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

	pair.first(pair.second);
}
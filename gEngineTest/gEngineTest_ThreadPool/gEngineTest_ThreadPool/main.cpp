
#include "gEngine\System\Logging\gLogger.h"
#include "gEngine\System\Threads\gThreadPool.h"
#include <array>
#undef main


int main(int argc, char* args[])
{
	std::unique_ptr<GENG::Threads::gThreadPool> m_threadPool = std::make_unique<GENG::Threads::gThreadPool>(std::thread::hardware_concurrency(), 16);
	//m_threadPool.Init(8, 16);

	struct funcData
	{
		std::string name;
		int number;
		int rand;
	};
	const int num = 150;
	std::array<funcData, num> intArray;
	std::string tName("ThreadName_");
	for (int i = 0; i < intArray.size(); i++)
	{
		std::string n(tName);
		n += std::to_string(i);
		intArray[i].name = n;
		intArray[i].number = i;
		intArray[i].rand = rand() % (intArray.size() * 10);
	}
	for (int i = 0; i < intArray.size() ; i++)
	{
		m_threadPool->AddTask(intArray[i].name, [&](void * data){
			funcData * pData = reinterpret_cast<funcData*>(data);
			if (pData != nullptr)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(pData->rand));
			}
		}, &intArray[i]);
	}
	
	m_threadPool->WaitToFinish();

	for (int i = intArray.size() - 1; i >= 0 ; i--)
	{
		m_threadPool->AddTask(intArray[i].name, [&](void * data){
			funcData * pData = reinterpret_cast<funcData*>(data);
			if (pData != nullptr)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds((intArray.size() * 10) - pData->rand));
			}
		}, &intArray[i]);
	}

	m_threadPool->Destroy();
	
	return true;
}
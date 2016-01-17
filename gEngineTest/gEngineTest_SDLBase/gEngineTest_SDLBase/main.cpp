
#include "gEngine\Useful.h"
#include "gEngine\System\Logging\gLogger.h"
#include "gEngine\System\DisplayDevice\gDisplayWindow.h"
#include "gEngine\System\DisplayDevice\gEventManager.h"
#include "gEngine\System\DisplayDevice\gLoopTimer.h"
#include "gEngine\System\Messaging\gMessagePoolMaster.h"
#include "gEngine\System\Objects\gHandleContainer.h"
#include "gEngine\System\Objects\gObjectComponents.h"
#include "gEngine\System\Resources\gFileHandling.h"
#include "gEngine\System\Resources\gMemoryManagement.h"
#include "gEngine\System\Scripting\gScriptingState.h"
#include "gEngine\System\Threads\gProcessThreadPool.h"
#include "gEngine\System\Threads\gTaskThreadPool.h"
#include <array>
#include <memory>
#include <vector>
#include <typeindex>
#include <iosfwd>

#undef main

#define TEST_FILEHANDLING
#define _CHECK_HASH_
#define TEST_COMPONENTSTRUCTRUE
#define TEST_LUA_SCRIPTLOADING

void TestComponentStructure()
{
#ifdef TEST_COMPONENTSTRUCTRUE
	class TestObject : public GENG::Object::ObjectBase
	{
	public: int i = 100;
	};
	class TestComponent_1 : public GENG::Object::ComponentBase
	{
	public: 
		int value = 100; 
		TestComponent_1(const int & v) : value(v) 
		{ 
			DBGINIT; 
			DBG(value); 
		}; 
		~TestComponent_1()
		{ 
			DBGDEST; 
		};

		int GetValue() { return value; };
	};
	class TestComponent_2 : public GENG::Object::ComponentBase
	{
	public: float value = 99.99f; TestComponent_2(const float & v) : value(v) { DBGINIT; DBG(value); }; ~TestComponent_2(){ DBGDEST; }
	};

	std::shared_ptr<GENG::Object::gEntryManager> m_manager = std::make_shared<GENG::Object::gEntryManager>();
	m_manager->AddEntryType<TestComponent_1, 500>();
	m_manager->AddEntryType<TestComponent_2, 500>();

	GENG::Object::gHandle handle_comp1 = m_manager->AddEntry<TestComponent_1>(25);
	GENG::Object::gHandle handle_comp2 = m_manager->AddEntry<TestComponent_2>(24.99f);

	// Handle components externally via manager
	m_manager->RunFunc<TestComponent_1>([](std::shared_ptr<TestComponent_1> data)
	{
		DBG("External: TestComponent_1 " << data->value);
		data->value *= 2;
	});
	m_manager->RunFunc<TestComponent_2>([](std::shared_ptr<TestComponent_2> data)
	{
		DBG("External: TestComponent_2 " << data->value);
		data->value *= 2;
	});

	// Attach components to an object
	TestObject testObject;
	testObject.AttachComponent(handle_comp1);
	testObject.AttachComponent(handle_comp2);

	// Handle components internally to the object
	for (auto component : testObject.m_components)
	{
		if (SHandleValid(component))
		{
			auto comp1 = m_manager->GetSharedEntry<TestComponent_1>(component);
			if (comp1 != nullptr)
			{
				DBG(comp1->value);
			}

			auto comp2 = m_manager->GetSharedEntry<TestComponent_2>(component);
			if (comp2 != nullptr)
			{
				DBG(comp2->value);
			}
		}
	}
#endif // TEST_COMPONENTSTRUCTRUE
}

void TestFileHandling()
{
#ifdef TEST_FILEHANDLING
	auto string1 = R"(F:\Videos\Films\Foodfight![2012].mp4)";
	auto string2 = R"(F:\Program Files (x86)\Games\WildStar\Patch\ClientData.archive)";

	std::string data1;
	std::vector<char> data2;
	//std::array<char, MB(2000)> data3;
	//std::array<std::array<char, MB(1000)>, 2> data4;
	auto file = GENG::FileHandling::SGetFileString(string1, data1);
	auto buff = GENG::FileHandling::SGetFileBuffer(string1, data2);
	//auto array = GENG::FileHandling::SGetFileArray(string1, data3);
	//auto arrays = GENG::FileHandling::SGetFileArrays(string1, data4);

	std::vector<char> data;
	GENG::FileHandling::SLoadMultithreaded<1>(string2, data);
	GENG::FileHandling::SLoadMultithreaded<1>(string2, data, 1024 * 1024);

	GENG::FileHandling::SLoadMultithreaded<2>(string2, data);
	GENG::FileHandling::SLoadMultithreaded<2>(string2, data, 1024 * 1024);

	GENG::FileHandling::SLoadMultithreaded<4>(string2, data);
	GENG::FileHandling::SLoadMultithreaded<4>(string2, data, 1024 * 1024);

	GENG::FileHandling::SLoadMultithreaded<8>(string2, data);
	GENG::FileHandling::SLoadMultithreaded<8>(string2, data, 1024 * 1024);
#endif // TEST_FILEHANDLING
}

int main(int argc, char* args[])
{
	Init();
		
	//////////////////////////////////////////////////////////////////////////
	// Test Memory Stack-Allocator
	struct Size256
	{
		uint8_t data[256];

		Size256(){};
		Size256(const int & v) { memset(&data, v, 256); };
		~Size256(){};
	};
	struct Size512
	{
		uint8_t data[512];

		Size512(){};
		Size512(const int & v) { memset(&data, v, 512); };
		~Size512(){};
	};

	GENG::Memory::gStackAllocator<256*8> allocator;

	auto pSize256_1 = allocator.AllocLower<Size256>(1);
	auto pSize256_2 = allocator.AllocLower<Size256>(2);
	auto pSize256_3 = allocator.AllocHigher<Size256>(3);
	auto pSize256_4 = allocator.AllocHigher<Size256>(4);
	auto pSize256_5 = allocator.AllocHigher<Size256>(6);
	auto pSize512_6 = allocator.AllocLower<Size512>(7);
	auto pSize256_7 = allocator.AllocHigher<Size256>(8);
	
	allocator.DebugOutput();

	//////////////////////////////////////////////////////////////////////////
	// Settings
	const std::string settingsFile = "F:\\My Documents\\GitHub\\gEngineTools\\gEngineTest\\gEngineTest_SDLBase\\gEngineTest_SDLBase\\settings.chai";
	std::shared_ptr<GENG::Scripting::ScriptState> scriptState = std::make_shared<GENG::Scripting::ScriptState>();
	std::string m_windowName = "gEngine - Window";
	uint32_t m_windowWidth;
	uint32_t m_windowHeight;
	bool m_windowIsMaximised;
	uint32_t maxFPS = 60;

	scriptState->RegisterValue("WindowName", m_windowName);
	scriptState->RegisterValue("WindowWidth", m_windowWidth);
	scriptState->RegisterValue("WindowHeight", m_windowHeight);
	scriptState->RegisterValue("WindowIsMaximised", m_windowIsMaximised);
	scriptState->RegisterValue("MaxFPS", maxFPS);

	scriptState->LoadFile(settingsFile);
	scriptState->EvaluateFile();
	
	//////////////////////////////////////////////////////////////////////////
	// Initalise
	std::shared_ptr<GENG::DisplayDevice::gLoopTimer> timer = std::make_shared<GENG::DisplayDevice::gLoopTimer>();

	std::shared_ptr<GENG::DisplayDevice::gGLWindow> window = std::make_shared<GENG::DisplayDevice::gGLWindow>();
	window->SetWindowSize(m_windowWidth, m_windowHeight);
	window->SetWindowTitle(m_windowName);

	GENG::DisplayDevice::gEventManager displayDevice;
	displayDevice.SetWindow(window);

	std::shared_ptr<GENG::Messaging::gMessagePoolMaster> messageMaster = GENG::Messaging::gMessagePoolMaster::Get();

	// VSync
	if (SDL_GL_SetSwapInterval(1) < 0)
	{
		DERROR("Could not set VSync:\n" << SDL_GetError());
		return GENG_EXIT_FAILURE;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// Setup game-loop
	const std::string ingameScript = "F:\\My Documents\\GitHub\\gEngineTools\\gEngineTest\\gEngineTest_SDLBase\\gEngineTest_SDLBase\\ingame.chai";
	scriptState->LoadFile(ingameScript);
	scriptState->AddVariable("gameloop", timer);

	timer->Init(maxFPS,
	[&]() -> void {
	},
	{
		GENG::DisplayDevice::gLoopTimer::InternalBlock(1.0f / 10.0f, "ScriptEvaluation", [&]() -> void {
			scriptState->SetReload(true);
		}),
		GENG::DisplayDevice::gLoopTimer::InternalBlock(0.5, "ScriptEvaluation", [&]() -> void {
			scriptState->EvaluateFile();
		}),
		GENG::DisplayDevice::gLoopTimer::InternalBlock(120, "EventHandling", [&]() -> void {
			displayDevice.HandleEvents();
		}),
		GENG::DisplayDevice::gLoopTimer::InternalBlock(maxFPS, "Main Loop", [&]() -> void {
			messageMaster->UpdateAll();
		}),
		GENG::DisplayDevice::gLoopTimer::InternalBlock(60, "Screen Clearing", [&]() -> void {
			
		})
	},
	[&]() -> void {
	}
	);

	//////////////////////////////////////////////////////////////////////////
	// Run game loop
	while (!displayDevice.GetQuit())
	{
		timer->Loop();
	}

	Destroy();
}
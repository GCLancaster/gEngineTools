
#include "gEngine\Useful.h"
#include "gEngine\System\Logging\gLogger.h"
#include "gEngine\System\DisplayDevice\gDisplayWindow.h"
#include "gEngine\System\DisplayDevice\gEventManager.h"
#include "gEngine\System\DisplayDevice\gLoopTimer.h"
#include "gEngine\System\Messaging\gMessagePoolMaster.h"
#include "gEngine\System\Objects\gHandleContainer.h"
#include "gEngine\System\Objects\gObjectComponents.h"
#include "gEngine\System\Resources\gFileHandling.h"
#include "gEngine\System\Resources\gResourcesUseful.h"
#include "gEngine\System\Resources\gMemoryManagement.h"
#include "gEngine\System\Scripting\gScriptingState.h"
#include "gEngine\System\Scripting\gScriptingRegister.h"
#include "gEngine\System\Threads\gProcessThreadPool.h"
#include "gEngine\System\Threads\gTaskThreadPool.h"
#include <array>
#include <memory>
#include <vector>
#include <typeindex>
#include <iosfwd>
#include <initializer_list>

#undef main

#define TEST_FILEHANDLING
#define _CHECK_HASH_
#define TEST_COMPONENTSTRUCTRUE
#define TEST_LUA_SCRIPTLOADING

void TestComponentStructure()
{
#ifdef TEST_COMPONENTSTRUCTRUE
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
	class TestObject : public GENG::Object::ObjectBase
	{
	public: 
		int i = 100;
	protected:
			virtual void RunComponent(GENG::Object::gHandle & component, std::shared_ptr<GENG::Object::gEntryManager> manager)
			{
				auto comp1 = manager->GetSharedEntry<TestComponent_1>(component);
				if (comp1 != nullptr)
				{
					DBG(comp1->value);
				}

				auto comp2 = manager->GetSharedEntry<TestComponent_2>(component);
				if (comp2 != nullptr)
				{
					DBG(comp2->value);
				}
			}

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
	testObject.RunComponents(m_manager);
#endif // TEST_COMPONENTSTRUCTRUE
}

void TestFileHandling()
{
#ifdef TEST_FILEHANDLING
	auto string1 = R"(F:\Videos\Films\Foodfight![2012].mp4)";
	auto string2 = R"(F:\Program Files (x86)\Games\WildStar\Patch\ClientData.archive)";
	auto string3 = R"(F:\Pictures\512_bmp_MildlyLoco.bmp)";

	//std::string data1;
	//std::vector<uint8_t> data2;
	//auto file = GENG::FileHandling::SGetFileString(string1, data1);
	//auto buff = GENG::FileHandling::SGetFileBuffer(string1, data2);

	//std::vector<uint8_t> data;
	//GENG::FileHandling::SLoadMultithreaded(string2, data);
	//GENG::FileHandling::SLoadMultithreaded(string2, data, 2, 1024 * 1024);

	//GENG::FileHandling::SLoadMultithreaded(string2, data);
	//GENG::FileHandling::SLoadMultithreaded(string2, data, 4, 1024 * 1024);

	//GENG::FileHandling::SLoadMultithreaded(string2, data);
	//GENG::FileHandling::SLoadMultithreaded(string2, data, 8, 1024 * 1024);

	//GENG::FileHandling::SLoadMultithreaded(string2, data);
	//GENG::FileHandling::SLoadMultithreaded(string2, data, 16, 1024 * 1024);
	
	std::vector<uint8_t> encoded;
	std::vector<uint8_t> decoded;
	std::vector<uint8_t> data3;

	GENG::FileHandling::SLoadMultithreaded(string3, data3, 4);

	GENG::FileHandling::Compression::SEncode_RunLengthEncoding<1, uint8_t>(data3, encoded);
	GENG::FileHandling::Compression::SDecode_RunLengthEncoding<1, uint8_t>(encoded, decoded);

	for (int i = 0; i < std::min(data3.size(), decoded.size()) ; i++)
	{
		assert(data3[i] == decoded[i]);
	}
#endif // TEST_FILEHANDLING
}

void TestMemoryAllocation()
{
	//////////////////////////////////////////////////////////////////////////
	// Test Memory Stack-Allocator
	GENG::Memory::stack<2048> stack;
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

	auto pObj1 = stack.AllocateObject<uint16_t>(5);
	auto pObj2a = stack.AllocateObject<uint32_t>(10);
	{
		auto pObj2b = stack.AllocateObject<uint64_t>(10);
	}
	auto pObj2c = stack.AllocateObject<uint8_t>(10);
	auto pObj3 = stack.AllocateObject<Size256>(15);
	//GENG::Memory::gStackAllocator<256 * 8> allocator;

	//auto pSize256_1 = allocator.AllocLower<uint8_t>(UINT8_MAX);
	//auto pSize256_2 = allocator.AllocLower<uint16_t>(UINT16_MAX);
	//
	//allocator.DebugOutput();
	//allocator.Dealloc(pSize256_1);

	//auto pSize256_3 = allocator.AllocHigher<Size256>(3);
	//auto pSize256_4 = allocator.AllocHigher<Size256>(4);

	//allocator.DebugOutput();

	//auto pSize256_5 = allocator.AllocHigher<Size256>(5);
	//auto pSize512_6 = allocator.AllocLower<Size512>(6);

	//allocator.DebugOutput();

	//auto pSize256_7 = allocator.AllocHigher<Size256>(7);

	//allocator.DebugOutput();

	//allocator.DeallocLower();
	//allocator.DeallocHigher();

	//allocator.DebugOutput();
}

void TestComponentFileHandling()
{	
	std::shared_ptr<GENG::Object::gEntryManager> m_manager = std::make_shared<GENG::Object::gEntryManager>();
	m_manager->AddEntryType<GENG::FileHandling::gResource_Buffer, 32>();
	
	class TestObject : public GENG::Object::ObjectBase
	{
	public:
		int i = 100;

		std::string m_bufferFile;
	protected:
		virtual void RunComponent(GENG::Object::gHandle & component, std::shared_ptr<GENG::Object::gEntryManager> manager)
		{
			auto comp1 = manager->GetSharedEntry<GENG::FileHandling::gResource_Buffer>(component);
			if (comp1 != nullptr)
			{
				comp1->LoadResource(m_bufferFile);
			}
		}

	};

	TestObject testObject;
	testObject.AttachComponent(m_manager->AddEntry<GENG::FileHandling::gResource_Buffer>());

	testObject.m_bufferFile = R"("F:\My Documents\GitHub\gEngineTools\gEngine\System\Resources\gResourcesUseful.h")";
	testObject.RunComponents(m_manager);
};

#include <emmintrin.h>

struct CDIB_PIXEL
{
	float R;
	float G;
	float B;
	float A;
};
struct CDIB_PIXEL_MM4
{
	__m128 R;
	__m128 G;
	__m128 B;
	__m128 A;
};
class CDIB_RGBA
{
	std::vector<float> m_data;
	float * m_R;
	float * m_G;
	float * m_B;
	float * m_A;

	int64_t m_width;
	int64_t m_height;

	CDIB_RGBA()
	{
		SetupRGBA(512 * 512);
	}
	~CDIB_RGBA(){}

	void SetupRGBA(const size_t & filesize)
	{
		const size_t quarter = filesize / 4;
		m_data.resize(filesize);
		m_R = m_data.data() + (quarter * 0);
		m_G = m_data.data() + (quarter * 1);
		m_B = m_data.data() + (quarter * 2);
		m_A = m_data.data() + (quarter * 3);
	}
	inline CDIB_PIXEL GetPixel(const int64_t & x, const int64_t & y)
	{
		const auto offset = x + (x * y);
		return { *(m_R + offset), *(m_G + offset), *(m_B + offset), *(m_A + offset) };
	}
	inline CDIB_PIXEL_MM4 GetPixel_MM4(const int64_t & x, const int64_t & y)
	{
		const auto offset = x + (x * y);
		return { _mm_load_ps(m_R + offset), _mm_load_ps(m_G + offset), _mm_load_ps(m_B + offset), _mm_load_ps(m_A + offset) };
	}

	inline void SetPixel(const int64_t & x, const int64_t & y, const CDIB_PIXEL & pixel)
	{
		const auto offset = x + (x * y);
		*(m_R + offset) = pixel.R;
		*(m_G + offset) = pixel.G;
		*(m_B + offset) = pixel.B;
		*(m_A + offset) = pixel.A;
	}
	inline void SetPixel_MM4(const int64_t & x, const int64_t & y, const CDIB_PIXEL_MM4 & pixel)
	{
		const auto offset = x + (x * y);
		_mm_store_ps((m_R + offset), pixel.R);
		_mm_store_ps((m_G + offset), pixel.G);
		_mm_store_ps((m_B + offset), pixel.B);
		_mm_store_ps((m_A + offset), pixel.A);
	}

	void EditImage()
	{
		for (int64_t y = 0; y < m_width; y++)
		{
			int64_t x = 0;
			for (; x < m_height; x += 4)
			{
				auto pixel4 = GetPixel_MM4(x, y);
			}
			for (; x < m_height; x += 1)
			{
				auto pixel = GetPixel(x, y);
			}
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// Carcassonne
namespace Carcassonne
{
	enum EdgeType
	{
		ctFarm,
		ctCity,
		ctRoad,
		ctMonk,

		ctCOUNT,
		ctNULL
	};
	enum EdgeSide
	{
		csTop,
		csLeft,
		csBottom,
		csRight,

		csCOUNT,
		csNULL
	};
	class Edge
	{
	private:
		EdgeType m_type = ctNULL;

	public:
		std::wstring RenderSingle() const
		{
			switch (m_type)
			{
			case ctFarm:
				return L"F";
				break;
			case ctCity:
				return L"C";
				break;
			case ctRoad:
				return L"R";
				break;
			case ctMonk:
				return L"M";
				break;
			case ctCOUNT:
			case ctNULL:
			default:
				return L"-";
				break;
			}
		}

		Edge(){};
		Edge(const EdgeType & type) : m_type(type) {};
	};
	class Tile
	{
	private:
		Edge m_edges[csCOUNT];
		Edge m_center;

		Tile * m_pEdgeTiles[csCOUNT];

	public:
		Tile(){};
		Tile(std::initializer_list<EdgeType> edges)
		{
			if (edges.size() == csCOUNT)
			{
				memcpy(&m_edges[0], edges.begin(), (sizeof(EdgeType) * csCOUNT));
			}
			else if (edges.size() == csCOUNT + 1)
			{
				memcpy(&m_edges[0], edges.begin(), (sizeof(EdgeType) * csCOUNT));
				m_center = *(edges.begin() + (csCOUNT + 1));
			}
			else
			{
				DERROR("Invalid number of tiles!");
			}
		}

		auto RenderSingle() const -> decltype(m_center.RenderSingle())
		{		
			using sstringType = decltype(m_center.RenderSingle());
			sstringType text = L"";

			// ###[-]###
			// [-][-][-]
			// ###[-]###
			auto topLeft = L'\u250F';
			auto topRight = L'\u2523';
			auto bottomLeft = L'\u2517';
			auto bottomRight = L'\u251B';

			// Top row
			text += topLeft;
			text += m_edges[csTop].RenderSingle();
			text += topRight;

			// Mid row
			text += m_edges[csLeft].RenderSingle();
			text += m_center.RenderSingle();
			text += m_edges[csRight].RenderSingle();

			text += bottomLeft;
			text += m_edges[csBottom].RenderSingle();
			text += bottomRight;

			return text;
		}
		auto Render() const -> decltype(RenderSingle())
		{
			using sstringType = decltype(RenderSingle());
			sstringType text = L"";

			text += RenderSingle();

			if (m_pEdgeTiles[csTop] != nullptr) { text += m_pEdgeTiles[csTop]->RenderSingle(); }
			if (m_pEdgeTiles[csLeft] != nullptr) { text += m_pEdgeTiles[csLeft]->RenderSingle(); }
			if (m_pEdgeTiles[csBottom] != nullptr) { text += m_pEdgeTiles[csBottom]->RenderSingle(); }
			if (m_pEdgeTiles[csRight] != nullptr) { text += m_pEdgeTiles[csRight]->RenderSingle(); }

			return text;
		}
		Tile * SetSide(const EdgeSide & side, Tile * pTile)
		{
			if (m_pEdgeTiles[side] != nullptr)
				DERROR("Cannot place tile!");
			m_pEdgeTiles[side] = pTile;
			return m_pEdgeTiles[side];
		}
	};
	class Board
	{
		static const size_t NUM_TILES = 60;

		size_t m_poolPos = NUM_TILES - 1;
		std::array<Tile, NUM_TILES> m_tilePool;
		
	public:
		Board()
		{
			m_tilePool[NUM_TILES - 1] = { EdgeType::ctRoad, EdgeType::ctFarm, EdgeType::ctCity, EdgeType::ctFarm };
		}

		Tile * GetTopOfStack() { return &m_tilePool[--m_poolPos]; }
		Tile * GetStartTile() { return &m_tilePool[NUM_TILES - 1]; };

		auto Render() const -> decltype(m_tilePool[NUM_TILES - 1].Render())
		{
			return m_tilePool[NUM_TILES - 1].Render();
		}
	};
};

int main(int argc, char* args[])
{
	Init();

	Carcassonne::Board m_board;
	auto s = m_board.GetStartTile();
	LOG(m_board.Render());

	auto s_l = s->SetSide(Carcassonne::EdgeSide::csLeft, m_board.GetTopOfStack());
	auto s_b = s->SetSide(Carcassonne::EdgeSide::csBottom, m_board.GetTopOfStack());
	auto s_r = s->SetSide(Carcassonne::EdgeSide::csRight, m_board.GetTopOfStack());

	s_l->SetSide(Carcassonne::EdgeSide::csTop, m_board.GetTopOfStack());

	LOG(m_board.Render());
		
	//TestComponentStructure();

	//TestFileHandling();

	//TestMemoryAllocation();

	//TestComponentFileHandling();

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
	const std::string ingameScript = R"(F:\My Documents\GitHub\gEngineTools\gEngineTest\gEngineTest_SDLBase\gEngineTest_SDLBase\settings.chai)";
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
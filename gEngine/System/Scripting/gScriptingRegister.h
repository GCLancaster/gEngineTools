
#pragma once

#include <array>
#include <memory>
#include <vector>
#include <typeindex>
#include "gEngine\Useful.h"
#include "gScriptingState.h"

#include "..\Logging\gLogger.h"

#include "..\Resources\gFileHandling.h"

#include "..\Threads\UsefulThreads.h"
#include "..\Threads\gProcessThreadPool.h"
#include "..\Threads\gTaskThreadPool.h"

#include "..\DisplayDevice\gDisplayWindow.h"
#include "..\DisplayDevice\gEventManager.h"
#include "..\DisplayDevice\gLoopTimer.h"

#define MCR_REGISTER_FUNC(pState, Name, Func, Rtn, Params, ...) \
	pState->RegisterFunction<Rtn(Params, ##__VA_ARGS__)>(Name, std::function<Rtn(Params, ##__VA_ARGS__)>(static_cast<Rtn(*)(Params, ##__VA_ARGS__)>(Func)))

#define MCR_REGISTER_STATIC_FUNCS(space) space::ScriptRegisterStaticFunctions(pScriptState)

namespace GENG
{
	namespace Logging
	{
		static void ScriptRegisterStaticFunctions(Scripting::ScriptState * pScriptState)
		{
			chaiscript::ModulePtr pModule = std::make_shared<chaiscript::Module>();

			pScriptState->RegisterFunction<std::string()>("GetAppFolder", GetAppFolder);
			pScriptState->RegisterFunction<std::string()>("GetLoggingPath", GetLoggingPath);

			auto filePath = pScriptState->GetFilepath();

#define MCR_RegsiterLoggingFuncs(type, conversionToString) \
			pScriptState->RegisterFunction<void(const type &)>("DBG", [](const type & msg) -> void { std::ostringstream os; os << "[ChaiScript] : " << conversionToString; DbgMsg(os.str()); }); \
			pScriptState->RegisterFunction<void(const type &)>("LOG", [](const type & msg) -> void { std::ostringstream os; os << "[ChaiScript] : " << conversionToString; LogMsg(os.str()); });

			MCR_RegsiterLoggingFuncs(std::string, msg);
			MCR_RegsiterLoggingFuncs(uint32_t, std::to_string(msg));
			MCR_RegsiterLoggingFuncs(bool, ((msg == 0) ? "0" : "1"));
		};
	};
	namespace FileHandling
	{
		static void ScriptRegisterStaticFunctions(Scripting::ScriptState * pScriptState)
		{
			MCR_REGISTER_FUNC(pScriptState, "GetFilesize", FileHandling::SGetFileSize, unsigned int, const std::string &);
			MCR_REGISTER_FUNC(pScriptState, "GetFilestring", FileHandling::SGetFileString, bool, const std::string &, std::string &, const unsigned int &);
			MCR_REGISTER_FUNC(pScriptState, "GetFilebuffer", FileHandling::SGetFileBuffer, bool, const std::string &, std::vector<char> &, const unsigned int &);
		};
	};
	namespace Threads
	{
		static void ScriptRegisterStaticFunctions(Scripting::ScriptState * pScriptState)
		{
			pScriptState->RegisterFunction<unsigned int()>("SGetNumberOfCores", &SGetNumberOfCores);

			pScriptState->RegisterClass<gTaskThreadPool>();
			pScriptState->RegisterClass<gProcessThreadPool>();
		};
	};
	namespace Scripting
	{
		static void RegisterAll(Scripting::ScriptState * pScriptState)
		{
			MCR_REGISTER_STATIC_FUNCS(Logging);
			MCR_REGISTER_STATIC_FUNCS(FileHandling);
			MCR_REGISTER_STATIC_FUNCS(Threads);
			pScriptState->RegisterClass<GENG::DisplayDevice::gGLWindow>();
			pScriptState->RegisterClass<GENG::DisplayDevice::gEventManager>();
			pScriptState->RegisterClass<GENG::DisplayDevice::gLoopTimer>();

			MCR_REGISTER_FUNC(pScriptState, "to_string", std::to_string, std::string, int);
			MCR_REGISTER_FUNC(pScriptState, "to_string", std::to_string, std::string, float);
			MCR_REGISTER_FUNC(pScriptState, "to_string", std::to_string, std::string, double);
		}
	}
};
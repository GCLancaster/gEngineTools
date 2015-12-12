
#pragma once

#include <mutex>
#include <string>
#include <windows.h>
#include <sstream>
#include <ostream>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include "SDL\SDL_filesystem.h"

namespace GENG
{
	namespace Logging
	{
		static bool g_loggingActive = true;
		static std::recursive_mutex g_loggingMutex;
		static std::ofstream g_logFile;
		static std::thread g_loggingAutoSave;

		static std::string GetAppFolder()
		{
			std::unique_lock<std::recursive_mutex> guard(g_loggingMutex);

			static std::string appFolder;
			if (appFolder.empty())
			{
				char * p = SDL_GetBasePath();
				appFolder = SDL_strdup(p);
				SDL_free(p);
			}
			return appFolder;
		};

		static std::string GetLoggingPath()
		{
			std::unique_lock<std::recursive_mutex> guard(g_loggingMutex);

			static std::string logFolder;
			auto now = time(0);
			tm timeS;
			char buff[80];
			localtime_s(&timeS, &now);
			strftime(buff, sizeof(buff), "%Y_%m_%d", &timeS);

			logFolder = GetAppFolder() + "main_" + buff + "_log.txt";

			return logFolder;
		};

		static void DbgMsg(const std::string & msg)
		{
			std::unique_lock<std::recursive_mutex> guard(g_loggingMutex);

			std::ostringstream out;
			out << "[" << __TIME__ << "]" << msg.c_str() << std::endl;
			auto c = out.str();
			auto d = std::string(c);
			OutputDebugString(d.c_str());
			std::cout << d << std::endl;
		};

		static void LogMsg(const std::string & msg)
		{
			std::unique_lock<std::recursive_mutex> guard(g_loggingMutex);

			DbgMsg(msg);
			
			g_logFile.open(GetLoggingPath(), std::ofstream::out | std::ofstream::app);
			if (g_logFile.is_open())
			{
				std::ostringstream out;
				out << "[" << __TIME__ << "]" << msg.c_str() << std::endl;
				auto c = out.str();
				auto d = std::string(c);

				g_logFile << d;

				if (g_logFile.is_open())
					g_logFile.close();
			}
		};

		static void InitaliseLogging()
		{
			std::unique_lock<std::recursive_mutex> guard(g_loggingMutex);
			g_loggingActive = true;
			g_logFile.open(GetLoggingPath(), std::ofstream::out | std::ofstream::app);

			LogMsg("Initalise Logging");
		};

		static void DestroyLogging()
		{
			std::unique_lock<std::recursive_mutex> guard(g_loggingMutex);

			LogMsg("Destroy Logging");

			if (g_logFile.is_open())
				g_logFile.close();

			g_loggingActive = false;
		};
	};
};

#define GENG_CLASSNAME std::string(typeid(this).name()).c_str()
#define DBG(s) do { std::ostringstream os; os << "[" << __LINE__ << "][" << __FUNCTION__ << "] : " << s; GENG::Logging::DbgMsg(os.str()); } while(false)
#define LOG(s) do { std::ostringstream os; os << "[" << __LINE__ << "][" << __FUNCTION__ << "] : " << s; GENG::Logging::LogMsg(os.str()); } while(false)
#define DBGINIT DBG("Constructed")
#define DBGDEST DBG("Destroyed")
#define DERROR(s) do { DBG(s); std::ostringstream os; os << "[" << __LINE__ << "][" << __FUNCTION__ << "] : " << s; throw std::logic_error(os.str()); } while(false)
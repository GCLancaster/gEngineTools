
#pragma once
#include <mutex>
#include <string>
#include <windows.h>
#include <sstream>
#include <ostream>
#include <fstream>
#include <iostream>

namespace GENG
{
	namespace Logging
	{
		static std::recursive_mutex g_loggingMutex;

		static std::string GetAppFolder()
		{
			std::unique_lock<std::recursive_mutex> guard(g_loggingMutex);

			static std::string appFolder = "C:\\Logging\\";
			//if (appFolder.empty())
			//{
			//	char *p = SDL_GetBasePath();
			//	appFolder = SDL_strdup(p);
			//	SDL_free(p);
			//}
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
			out << "[" << __TIME__ << "]" << msg.c_str() << "\r\n";
			auto c = out.str();
			auto d = std::string(c);
			OutputDebugString(d.c_str());
			std::cout << d << std::endl;
		};

		static void LogMsg(const std::string & msg)
		{
			std::unique_lock<std::recursive_mutex> guard(g_loggingMutex);
			
			std::ostringstream out;
			out << "[" << __TIME__ << "]" << msg.c_str() << "\r\n";

			std::ofstream file;
			file.open(GetLoggingPath());
			file << out.rdbuf();
			file.close();
		};
	};
};

#define GENG_CLASSNAME std::string(typeid(this).name()).c_str()
#define DBG(s) do { std::ostringstream os; os << "[" << __LINE__ << "][" << __FUNCTION__ << "] : " << s; GENG::Logging::DbgMsg(os.str()); } while(false)
#define LOG(s) do { std::ostringstream os; os << "[" << __LINE__ << "][" << __FUNCTION__ << "] : " << s; GENG::Logging::LogMsg(os.str()); } while(false)
#define DBGINIT DBG("Constructed")
#define DBGDEST DBG("Destroyed")
#define DERROR(s) do { DBG(s); throw std::logic_error(s); } while(false)
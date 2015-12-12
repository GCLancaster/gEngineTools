
#pragma once

#ifdef LIB_SDL
	// Simple DirectMedia Layer is a cross - platform development library designed to provide 
	// low level access to audio, keyboard, mouse, joystick, and graphics hardware via OpenGL and Direct3D.
	#include <SDL\SDL.h>
	#include <SDL\SDL_opengl.h>
#endif // LIB_SDL

#ifdef LIB_XML
	#include <rapidxml.hpp>
#endif // LIB_XML

#include <vector>
#include <array>
#include <utility>
#include <memory>
#include "System\Logging\gLogger.h"

static void Init()
{
#ifdef LIB_SDL
	LOG("Using SDL2");
	SDL_Init(SDL_INIT_EVERYTHING);
#endif // LIB_SDL
#ifdef LIB_XML
	LOG("Using RapidXML");
#endif // LIB_XML
	GENG::Logging::InitaliseLogging();
};
static void Destroy()
{
#ifdef LIB_SDL
	SDL_Quit();
#endif // LIB_SDL
	GENG::Logging::DestroyLogging();
};
namespace GENG
{
#define GENG_EXIT_SUCCESS true;
#define GENG_EXIT_FAILURE false;

	template<typename T>
	using deleted_unique_ptr = std::unique_ptr<T, std::function<void(T*)>>;

	template<typename T>
	void _removeObjectFromVector(std::vector<T> &vec, T obj, bool bDelete = false)
	{
		auto f = std::find(vec.begin(), vec.end(), obj);
		if (f != vec.end())
		{
			if (bDelete)
				delete *f;
			vec.erase(f);
		}
	}

	template<int BuffSize> struct factorial { enum { value = BuffSize * factorial<BuffSize - 1>::value }; };
	template<> struct factorial<1> { enum { value = 1 }; };

	static std::vector<std::string> _getDelimited(const std::string & string, const std::string & delimiter = ",")
	{
		std::vector<std::string> rtn;
		std::string s = string;
		size_t pos = 0;
		std::string token;
		while ((pos = s.find(delimiter)) != std::string::npos)
		{
			rtn.push_back(s.substr(0, pos));
			s.erase(0, pos + delimiter.length());
		}
		rtn.push_back(s.substr(0));
		return rtn;
	};

	static std::pair<std::string, int> _getStringOrder(const std::string & string)
	{
		int endOfName = string.find('[');
		int endOfNum = string.find(']');
		if (endOfName >= 0 && endOfNum > 0)
		{
			std::string name = string.substr(0, endOfName);
			std::string num = string.substr((endOfName + 1), endOfNum - (endOfName + 1));
			return std::make_pair<std::string, int>(std::string(name), std::atoi(num.c_str()));
		}

		return std::make_pair<std::string, int>(std::string(string), 0);
	};

	namespace DisplayDevice
	{
		struct GLWindow
		{
		private:
			deleted_unique_ptr<SDL_Window> m_glWindow;
		public:
			GLWindow(const std::string & winName)
			{
				m_glWindow = deleted_unique_ptr<SDL_Window>(
					SDL_CreateWindow(winName.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL),
					[](SDL_Window * pWindow) { SDL_DestroyWindow(pWindow); });
				if (m_glWindow == nullptr)
					DERROR("Could not initialise Window: " << SDL_GetError());	
			};
			~GLWindow() {};
			SDL_Window * Get() { return m_glWindow.get(); };
		};
		struct GLContext
		{
		private:
			SDL_GLContext m_glContext;
		public:
			GLContext(SDL_Window * window) 
			{ 
				m_glContext = SDL_GL_CreateContext(window); 
				if (m_glContext == nullptr)
					DERROR("Could not initialise OpenGL Context: " << SDL_GetError());
			};
			~GLContext() { SDL_GL_DeleteContext(m_glContext); };
			SDL_GLContext Get() { return m_glContext; };
		};
	};

	//static inline void GetBounds(const std::vector<Vec2> & tightFit, std::array<Vec2, 2> & bounds)
	//{
	//	bounds[0].x = bounds[0].y = FLT_MAX;
	//	bounds[1].x = bounds[1].y = FLT_MIN;
	//	for (auto p : tightFit)
	//	{
	//		bounds[0].x = std::min(bounds[0].x, p.x);
	//		bounds[0].y = std::min(bounds[0].y, p.y);
	//		bounds[1].x = std::max(bounds[1].x, p.x);
	//		bounds[1].y = std::max(bounds[1].y, p.y);
	//	}
	//};

	struct Vec2
	{
		float x = -1;
		float y = -1;

		Vec2() {};
		Vec2(const int & x, const int & y) : x(x), y(y) {};
		~Vec2() {};
	};
}
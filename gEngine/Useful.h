
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

#ifdef LIB_CHAISCRIPT
#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_stdlib.hpp>
#endif // LIB_CHAISCRIPT

#include <string>
#include <vector>
#include <array>
#include <utility>
#include <memory>
#include <thread>
#include "System\Logging\gLogger.h"

static void Init()
{
	GENG::Logging::InitaliseLogging();
#ifdef LIB_SDL
	LOG("Using SDL2");
	SDL_Init(SDL_INIT_EVERYTHING);
#endif // LIB_SDL
#ifdef LIB_CHAISCRIPT
	LOG("Using ChaiScript");
#endif // LIB_CHAISCRIPT
#ifdef LIB_XML
	LOG("Using RapidXML");
#endif // LIB_XML
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

#define KB(x)((x) << 10)
#define MB(x)((x) << 20)
#define GB(x)((x) << 30)
	
	template<typename T>
	using deleted_unique_ptr = std::unique_ptr<T, std::function<void(T*)>>;

	template<typename T>
	void _removeObjectFromVector(std::vector<T> &vec, const T & obj)
	{
		auto f = std::find(vec.begin(), vec.end(), obj);
		if (f != vec.end())
		{
			vec.erase(f);
		}
	}
	template<typename T>
	void _removeObjectFromVectorAndDelete(std::vector<T> &vec, const T & obj)
	{
		auto f = std::find(vec.begin(), vec.end(), obj);
		if (f != vec.end())
		{
			delete *f;
			vec.erase(f);
		}
	}

	namespace Maths
	{
		template<int NUM> struct factorial { enum { value = NUM * factorial<NUM - 1>::value }; };
		template<> struct factorial<1> { enum { value = 1 }; };
	}

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
		size_t endOfName = string.find('[');
		size_t endOfNum = string.find(']');
		if (endOfName >= 0 && endOfNum > 0)
		{
			std::string name = string.substr(0, endOfName);
			std::string num = string.substr((endOfName + 1), endOfNum - (endOfName + 1));
			return std::make_pair<std::string, int>(std::string(name), std::atoi(num.c_str()));
		}

		return std::make_pair<std::string, int>(std::string(string), 0);
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
	
	template<typename T>
	T fixedLength(const int & value, int digits = 3) 
	{
		unsigned int uvalue = (value < 0) ? -value : value;
		if (value < 0)
			digits--;
		T result;
		while (digits-- > 0) 
		{
			result += ('0' + uvalue % 10);
			uvalue /= 10;
		}
		if (value < 0) {
			result += '-';
		}
		std::reverse(result.begin(), result.end());
		return result;
	}

	template <typename T> 
	T clamp(const T& n, const T& lower, const T& upper) { return std::max(lower, std::min(n, upper)); }

	//////////////////////////////////////////////////////////////////////////

	struct Vec2
	{
		float x = -1;
		float y = -1;

		Vec2() {};
		Vec2(const uint32_t & x, const uint32_t & y) : x(static_cast<float>(x)), y(static_cast<float>(y)) {};
		Vec2(const float & x, const float & y) : x(x), y(y) {};
		~Vec2() {};

		static void ScriptRegisterClass(chaiscript::ModulePtr pModule)
		{
			chaiscript::utility::add_class<Vec2>(*pModule, "Vec2",
				// Constructors
				{
					chaiscript::constructor<Vec2()>(),
					chaiscript::constructor<Vec2(const uint32_t & x, const uint32_t & y)>(),
					chaiscript::constructor<Vec2(const float & x, const float & y)>()
				},
				// Functions
				{
					{ chaiscript::fun(&Vec2::x), "x" },
					{ chaiscript::fun(&Vec2::y), "y" }
				}
			);
		};
	};
}
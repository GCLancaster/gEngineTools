
#pragma once

#include <memory>
#include <mutex>
#include <vector>
#include <map>

#include "..\..\Useful.h"
#include "..\Scripting\gScriptingState.h"

namespace GENG
{
	namespace DisplayDevice
	{
		class gGLWindow
		{
		protected:
			Vec2 m_dimensons;
			deleted_unique_ptr<SDL_Window> m_glWindow;
			SDL_GLContext m_glContext;

		public:
			gGLWindow(const std::string & winName = "")
			{
				m_glWindow = deleted_unique_ptr<SDL_Window>(
					SDL_CreateWindow(winName.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL),
					[](SDL_Window * pWindow) { SDL_DestroyWindow(pWindow); });
				if (m_glWindow == nullptr)
					DERROR("Could not initialise Window: " << SDL_GetError());

				m_glContext = SDL_GL_CreateContext(m_glWindow.get());
			};
			~gGLWindow() 
			{
				SDL_GL_DeleteContext(m_glContext);
			};
			SDL_Window * Get() { return m_glWindow.get(); };

			std::string SetWindowTitle(const std::string & windowName)
			{
				SDL_SetWindowTitle(m_glWindow.get(), windowName.c_str());
				return windowName;
			};

			Vec2 SetWindowSize(const uint32_t & width, const uint32_t & height)
			{
				m_dimensons.x = width;
				m_dimensons.y = height;
				SDL_SetWindowSize(m_glWindow.get(), width, height);
				return Vec2(width, height);
			}
			Vec2 GetWindowSize() const { return m_dimensons; }

			
			static void ScriptRegisterClass(chaiscript::ModulePtr pModule)
			{
				chaiscript::utility::add_class<gGLWindow>(*pModule, "gGLWindow",
					// Constructors
					{
						chaiscript::constructor<gGLWindow(const std::string & winName)>(),
					},
					// Functions
					{
						{ chaiscript::fun(&gGLWindow::SetWindowTitle), "SetWindowTitle" },
						{ chaiscript::fun(&gGLWindow::SetWindowSize), "SetWindowSize" },
						{ chaiscript::fun(&gGLWindow::GetWindowSize), "GetWindowSize" },
					}
				);
			};
		};
	};
};
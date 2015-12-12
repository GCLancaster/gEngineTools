
#pragma once

#include <memory>
#include <mutex>
#include <vector>
#include <map>

#include "..\..\Useful.h"

namespace GENG
{
	namespace DisplayDevice
	{
		class gEventManager;
		struct gSKeyTimings;

		class gEventHandler
		{
			friend class gEventManager;
		public:
			gEventHandler();
			virtual ~gEventHandler();
		protected:
			virtual void HandleEvent(const SDL_Event & eventHandler, const std::map<SDL_KeyTypes, gSKeyTimings> & keys) = 0;
		};

		struct gSKeyTimings
		{
			// Time to a button has to be pressed for until it registers as "held-down".
			std::chrono::duration<long long, std::milli> m_triggerTillHoldDown = std::chrono::milliseconds(300);

			bool m_bPressed = false;
			bool m_bHeldDown = false;
			std::chrono::high_resolution_clock::time_point m_timePoint = std::chrono::high_resolution_clock::now();

			std::chrono::duration<long long, std::milli> TimeSincePress()
			{
				return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_timePoint);
			}
		};

		class gEventManager
		{
		private:
			bool m_bQuit = false;
			bool m_bPaused = false;

			bool m_bWindowFocus = true;
			bool m_bMouseFocus = true;
			bool m_bKeyboardFocus = true;

			bool m_bWindowMaximised = false;

			std::map<SDL_KeyTypes, gSKeyTimings> m_keyHeldDownTime;

			std::shared_ptr<GLWindow> m_pWindow;
			std::shared_ptr<GLContext> m_pOpenGLCTX;

			static std::vector<gEventHandler *> g_eventHandlers;
			static std::mutex g_eventMutex;

		public:
			static void SAddEventHandler(gEventHandler * pEventHandler);
			static void SRemoveEventHandler(gEventHandler * pEventHandler);
			static SDL_KeyTypes SGetEventKeyID(const SDL_Event & event) 
			{ 
				if (!(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP))
					return SDLK_UNKNOWN;
				return static_cast<SDL_KeyTypes>(event.key.keysym.sym);
			};
			

			gEventManager() {};
			~gEventManager() {};
			
			void HandleEvents();

			bool GetQuit() const { return m_bQuit; }

			void SetWindow(std::shared_ptr<GLWindow> window) { m_pWindow = window; }
			void SetOpenGLCTX(std::shared_ptr<GLContext> openGLCTX) { m_pOpenGLCTX = openGLCTX; }
		private:
			void HandleWindowEvent(const SDL_Event & event);
			void HandleKeyboardEvent(const SDL_Event & event, const SDL_EventType & keyboardType);
			void HandleEventHandlers(const SDL_Event & event, const std::vector<gEventHandler *> & eventHandlers);
		};
	};
};
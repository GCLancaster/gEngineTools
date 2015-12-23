
#pragma once

#include <memory>
#include <mutex>
#include <vector>
#include <map>

#include "..\..\Useful.h"
#include "gDisplayWindow.h"

//class obj : public GENG::DisplayDevice::gEventHandler
//{
//	virtual void HandleEvent(const SDL_Event & eventHandler, const std::map<SDL_KeyTypes, GENG::DisplayDevice::gSKeyTimings> & keys)
//	{
//		auto id = GENG::DisplayDevice::gEventManager::SGetEventKeyID(eventHandler);
//		if (id == SDLK_o)
//			DBG("Do event");
//	}
//};

namespace GENG
{
	namespace DisplayDevice
	{
		class gEventManager;
		struct gSKeyTimings;

		typedef std::function<void(const SDL_Event &, const std::map<SDL_KeyTypes, gSKeyTimings> &)> tyEventFunction;

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

			std::shared_ptr<gGLWindow> m_pWindow;

			std::vector<tyEventFunction> m_eventBehaviours;

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
			
			inline void AddEventBehaviour(tyEventFunction func)
			{
				m_eventBehaviours.push_back(func);
			}

			inline void SetWindow(std::shared_ptr<gGLWindow> window) { m_pWindow = window; }

			inline bool GetKeyboardFocus() const { return m_bKeyboardFocus; }
			inline bool GetMouseFocus() const { return m_bMouseFocus; }
			inline bool GetPaused() const { return m_bPaused; }
			inline bool GetQuit() const { return m_bQuit; }
			inline bool GetWindowFocus() const { return m_bWindowFocus; }
			inline bool GetWindowMaximised() const { return m_bWindowMaximised; }

			inline void SetPaused(bool paused) { m_bPaused = paused; }
			inline void SetQuit(bool quit) { m_bQuit = quit; }
			inline void SetWindowMaximised(bool windowMaximised) 
			{ 
				m_bWindowMaximised = windowMaximised; 
				if (m_pWindow != nullptr)
					SDL_SetWindowFullscreen(m_pWindow->Get(), (m_bWindowMaximised) ? SDL_TRUE : SDL_FALSE);
			}

			std::shared_ptr<gGLWindow> GetWindow() const { return m_pWindow; }
		private:
			void HandleWindowEvent(const SDL_Event & event);
			void HandleKeyboardEvent(const SDL_Event & event, const SDL_EventType & keyboardType);
			void HandleEventHandlers(const SDL_Event & event, const std::vector<gEventHandler *> & eventHandlers);
			void HandleEventBehaviours(const SDL_Event & event);

		public:
			static void ScriptRegisterClass(chaiscript::ModulePtr pModule)
			{
				chaiscript::utility::add_class<gEventManager>(*pModule, "gEventManager",
					// Constructors
					{
						chaiscript::constructor<gEventManager()>(),
					},
					// Functions
					{
						{ chaiscript::fun(&gEventManager::GetKeyboardFocus), "GetKeyboardFocus" },
						{ chaiscript::fun(&gEventManager::GetMouseFocus), "GetMouseFocus" },
						{ chaiscript::fun(&gEventManager::GetPaused), "GetPaused" },
						{ chaiscript::fun(&gEventManager::GetQuit), "GetQuit" },
						{ chaiscript::fun(&gEventManager::GetWindowFocus), "GetWindowFocus" },
						{ chaiscript::fun(&gEventManager::GetWindowMaximised), "GetWindowMaximised" },
						{ chaiscript::fun(&gEventManager::SetPaused), "SetPaused" },
						{ chaiscript::fun(&gEventManager::SetQuit), "SetQuit" },
						{ chaiscript::fun(&gEventManager::SetWindowMaximised), "SetWindowMaximised" },

						{ chaiscript::fun(&gEventManager::GetWindow), "GetWindow" },
					}
				);
			};
		};
	};
};
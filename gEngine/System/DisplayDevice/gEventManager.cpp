
#include <mutex>
#include <vector>
#include <string>

#include "gEventManager.h"

GENG::DisplayDevice::gEventHandler::gEventHandler()
{
	gEventManager::SAddEventHandler(this);
}

GENG::DisplayDevice::gEventHandler::~gEventHandler()
{
	gEventManager::SRemoveEventHandler(this);
}

std::vector<GENG::DisplayDevice::gEventHandler *> GENG::DisplayDevice::gEventManager::g_eventHandlers;
std::mutex GENG::DisplayDevice::gEventManager::g_eventMutex;

void GENG::DisplayDevice::gEventManager::SAddEventHandler(gEventHandler * pEventHandler)
{
	std::lock_guard<std::mutex> guard(g_eventMutex);
	g_eventHandlers.push_back(pEventHandler);
}

void GENG::DisplayDevice::gEventManager::SRemoveEventHandler(gEventHandler * pEventHandler)
{
	std::lock_guard<std::mutex> guard(g_eventMutex);
	_removeObjectFromVector<gEventHandler*>(g_eventHandlers, pEventHandler);
}

void GENG::DisplayDevice::gEventManager::HandleEvents()
{
	SDL_Event eventHandler;
	while (SDL_PollEvent(&eventHandler) == 1)
	{
		HandleWindowEvent(eventHandler);
		HandleKeyboardEvent(eventHandler, SDL_KEYDOWN);
		HandleKeyboardEvent(eventHandler, SDL_KEYUP);
		HandleEventHandlers(eventHandler, g_eventHandlers);
	}
}

void GENG::DisplayDevice::gEventManager::HandleWindowEvent(const SDL_Event & event)
{
	if (event.type != SDL_WINDOWEVENT)
		return;

	SDL_WindowEventID id = static_cast<SDL_WindowEventID>(event.window.event);
	switch (id)
	{
	case SDL_WINDOWEVENT_NONE:         // Never used
		DERROR("Invalid WindowEventID");
		break;
	case SDL_WINDOWEVENT_SHOWN:        // Window has been shown
		m_bWindowFocus = true;
		LOG("Window shown");
		break;
	case SDL_WINDOWEVENT_HIDDEN:       // Window has been hidden
		m_bWindowFocus = false;
		LOG("Window hidden");
		break;
	case SDL_WINDOWEVENT_EXPOSED:      // Window has been exposed and should be redrawn
		m_bWindowFocus = true;
		LOG("Window exposed! Redraw needed!");
		break;
	case SDL_WINDOWEVENT_MOVED:        // Window has been moved to data1, data2
		LOG("Window moved to #Data1 and #Data2");
		break;
	case SDL_WINDOWEVENT_RESIZED:      // Window has been resized to data1xdata2
		LOG("Window resized to #Data1 x #Data2");
		break;
	case SDL_WINDOWEVENT_SIZE_CHANGED: // The window size has changed, either as a result of an API call or through the system or user changing the window size.
		LOG("Window resized via API");
		break;
	case SDL_WINDOWEVENT_MINIMIZED:    // Window has been minimized
		m_bWindowMaximised = false;
		m_bWindowFocus = false;
		LOG("Window minimized");
		break;
	case SDL_WINDOWEVENT_MAXIMIZED:    // Window has been maximized
		m_bWindowMaximised = true;
		m_bWindowFocus = true;
		LOG("Window maximized");
		break;
	case SDL_WINDOWEVENT_RESTORED:     // Window has been restored to normal size and position
		m_bWindowFocus = true;
		LOG("Window restored");
		break;
	case SDL_WINDOWEVENT_ENTER:        // Window has gained mouse focus
		m_bMouseFocus = true;
		LOG("Window gained mouse focus");
		break;
	case SDL_WINDOWEVENT_LEAVE:        // Window has lost mouse focus
		m_bMouseFocus = false;
		LOG("Window lost mouse focus");
		break;
	case SDL_WINDOWEVENT_FOCUS_GAINED: // Window has gained keyboard focus
		m_bKeyboardFocus = true;
		LOG("Window gained keyboard focus");
		break;
	case SDL_WINDOWEVENT_FOCUS_LOST:   // Window has lost keyboard focus
		m_bKeyboardFocus = false;
		LOG("Window lost keyboard focus");
		break;
	case SDL_WINDOWEVENT_CLOSE:        // The window manager requests that the window be closed
		m_bQuit = true;
		LOG("Window closed");
		break;
	default:
		DERROR("Invalid WindowEventID");
		break;
	}
}

void GENG::DisplayDevice::gEventManager::HandleKeyboardEvent(const SDL_Event & event, const SDL_EventType & keyboardType)
{
	if (!(keyboardType == SDL_KEYDOWN || keyboardType == SDL_KEYUP))
		DERROR("Invalid keyboard type");

	if (event.type != keyboardType)
		return;
	
	SDL_KeyTypes id = SGetEventKeyID(event);
	auto & key = m_keyHeldDownTime[id];

	//////////////////////////////////////////////////////////////////////////
	// Handle key press timings
	if (keyboardType == SDL_KEYDOWN)
	{
		if (!key.m_bPressed)
		{
			key.m_bPressed = true;
			key.m_bHeldDown = false;

			auto timeSince = key.TimeSincePress();
			LOG("Pressed : Key(" << id << ") Time since previous(" << timeSince.count() << ")");

			key.m_timePoint = std::chrono::high_resolution_clock::now();
		}
		else if (key.m_bPressed && key.TimeSincePress() >= key.m_triggerTillHoldDown)
		{
			key.m_bHeldDown = true;

			auto timeSince = key.TimeSincePress();
			LOG("Hold : Key(" << id << ") Time held down(" << timeSince.count() << ")");
		}
	}
	else
	{
		key.m_bPressed = false;
		key.m_bHeldDown = false;

		auto timeSince = key.TimeSincePress();
		LOG("Release : Key(" << id << ") Duration(" << timeSince.count() << ")");

		key.m_timePoint = std::chrono::high_resolution_clock::now();
	}

	//////////////////////////////////////////////////////////////////////////
	// Handle key press events
	if (key.m_bPressed && !key.m_bHeldDown)
	{
		switch (id)
		{
		case SDLK_p:
			m_bPaused = !m_bPaused;
		default:
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Handle key held events
	if (key.m_bHeldDown && key.TimeSincePress() > std::chrono::milliseconds(150))
	{
		switch (id)
		{
		case SDLK_ESCAPE:
			m_bQuit = true;
		default:
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Handle multi-key press events
	if (m_keyHeldDownTime[SDLK_LCTRL].m_bPressed && m_keyHeldDownTime[SDLK_f].m_bPressed)
	{
		if (!m_bWindowMaximised)
		{
			if (m_pWindow != nullptr)
				SDL_SetWindowFullscreen(m_pWindow->Get(), SDL_TRUE);
			m_bWindowMaximised = true;
		}
		else
		{
			if (m_pWindow != nullptr)
				SDL_SetWindowFullscreen(m_pWindow->Get(), SDL_FALSE);
			m_bWindowMaximised = false;
		}
	}
}

void GENG::DisplayDevice::gEventManager::HandleEventHandlers(const SDL_Event & event, const std::vector<gEventHandler *> & eventHandlers)
{
	for (auto pHandler : eventHandlers)
		pHandler->HandleEvent(event, m_keyHeldDownTime);
}

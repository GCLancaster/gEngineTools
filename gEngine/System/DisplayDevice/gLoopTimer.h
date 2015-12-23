
#pragma once

#include "..\..\Useful.h"
#include <chrono>
#include <thread>

namespace GENG
{
	namespace DisplayDevice
	{
		class gLoopTimer
		{
		public:
			struct InternalBlock
			{
				double m_internalFPS = 0;
				std::string m_name = "";
				std::function<void()> m_function;
				std::chrono::high_resolution_clock::duration m_frameDelta;
				std::chrono::high_resolution_clock::duration m_frameAccum;
				std::chrono::high_resolution_clock::duration m_totalTime;

				InternalBlock() = delete;
				InternalBlock(const double & fps, const std::string & name, std::function<void()> func) :
					m_internalFPS(fps), m_name(name), m_function(func)
				{};
				~InternalBlock()
				{};
			};
	
		private:
			std::chrono::steady_clock::time_point m_appStart;

			std::chrono::high_resolution_clock::time_point m_fTimerStart;	// current frame time
			std::chrono::high_resolution_clock::time_point m_fTimerEnd;		// current frame time
			std::chrono::high_resolution_clock::time_point m_fTimerSecondCount;
			std::chrono::high_resolution_clock::duration m_fTimerMaxDiff;

			std::function<void()> m_preLoopFunction;
			std::vector<InternalBlock> m_midLoopFunctions;
			std::function<void()> m_postLoopFunction;

			std::chrono::high_resolution_clock::duration m_curFrameTime;
			std::chrono::high_resolution_clock::duration m_avgFrameTime;
			std::chrono::high_resolution_clock::duration m_minFrameTime;
			std::chrono::high_resolution_clock::duration m_maxFrameTime;
			uint32_t m_frameCounter = 0;
			uint32_t m_maxFPS = 0;

			bool m_bQuit = false;
			bool m_bStabilised = false;
		public:
			gLoopTimer() {}
			~gLoopTimer() {}

			inline void Init(
				const uint32_t & maxFPS,
				const std::function<void()> & preLoopFunction,
				const std::initializer_list<InternalBlock> & internaLoopFunctions,
				const std::function<void()> & postLoopFunction)
			{
				SetMaxFPS(maxFPS);

				m_preLoopFunction = std::move(preLoopFunction);
				m_postLoopFunction = std::move(postLoopFunction);

				m_midLoopFunctions.clear();
				for (auto loopPair : internaLoopFunctions)
					_addMidLoopFunction(loopPair);

				m_appStart = std::chrono::steady_clock::now();
			}
			
			inline void SetMaxFPS(const uint32_t & maxFPS)
			{
				// External Loop Max FPS
				if (m_maxFPS != maxFPS)
				{
					m_maxFPS = maxFPS;
					auto timeI = std::chrono::seconds(1) / static_cast<double>(m_maxFPS / 2);
					m_fTimerMaxDiff = std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(timeI);
				}
			}

			inline bool Loop()
			{
				if (m_bQuit) return false;

				_startFrame(m_preLoopFunction);

				if (m_bQuit) return false;

				for (auto & internal : m_midLoopFunctions)
				{
					if (m_bQuit) return false;
					_midLoop(internal);
				}

				if (m_bQuit) return false;

				_endFrame(m_postLoopFunction);

				return true;
			};
			std::chrono::milliseconds GetRunningTime()
			{
				auto diff = std::chrono::steady_clock::now() - m_appStart;
				return std::chrono::duration_cast<std::chrono::milliseconds>(diff);
			}

		protected:
			inline void _addMidLoopFunction(const InternalBlock & funcPair)
			{
				m_midLoopFunctions.push_back(funcPair);
				auto timeD = std::chrono::seconds(1) / static_cast<double>(funcPair.m_internalFPS);
				m_midLoopFunctions.back().m_frameDelta = std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(timeD);
				m_midLoopFunctions.back().m_frameAccum.zero();
				m_midLoopFunctions.back().m_totalTime.zero();
			};

			inline void _startFrame(const std::function<void()> & preLoopFunction)
			{
				m_frameCounter++;

				// Duration of previous frame
				m_fTimerStart = std::chrono::high_resolution_clock::now();
				m_curFrameTime = m_fTimerStart - m_fTimerEnd;
				m_avgFrameTime += m_curFrameTime;

				if (!m_bStabilised)
					m_bStabilised = (GetRunningTime() > std::chrono::seconds(5));

				// Second counter + per-second debugging
				if (m_bStabilised && (m_fTimerStart > (m_fTimerSecondCount + std::chrono::seconds(1))))
				{
					m_fTimerSecondCount = m_fTimerStart;

					m_avgFrameTime /= m_frameCounter;

					m_minFrameTime = min(m_minFrameTime, m_curFrameTime);
					m_maxFrameTime = max(m_maxFrameTime, m_curFrameTime);

					//DBG("ExternalLoop: " << std::chrono::duration_cast<std::chrono::seconds>(getRunningTime()).count() << "s" << " " << m_frameCounter << "fps"
					//	<< " Cur:" << std::chrono::duration_cast<std::chrono::milliseconds>(m_curFrameTime).count() << "ms"
					//	<< " Avg:" << std::chrono::duration_cast<std::chrono::milliseconds>(m_avgFrameTime).count() << "ms"
					//	<< " Min:" << std::chrono::duration_cast<std::chrono::milliseconds>(m_minFrameTime).count() << "ms"
					//	<< " Max:" << std::chrono::duration_cast<std::chrono::milliseconds>(m_maxFrameTime).count() << "ms");

					for (auto internal: m_midLoopFunctions)
					{
						//DBG(internal.m_name + ": " + std::to_string(internal.m_internalFPS) + "fps");
						internal.m_internalFPS = 0;
					}

					m_frameCounter = 0;
				}

				// Limit Frame Difference
				if (m_curFrameTime > m_fTimerMaxDiff)
					m_curFrameTime = m_fTimerMaxDiff;

				// Frame-rate normalizer
				if (m_curFrameTime < m_fTimerMaxDiff)
				{
					std::chrono::high_resolution_clock::duration sleepTime(m_fTimerMaxDiff - m_curFrameTime);
					std::this_thread::sleep_for(sleepTime);
				}

				preLoopFunction();
			};

			inline void _midLoop(InternalBlock & internal)
			{
				internal.m_frameAccum += m_curFrameTime;

				while (internal.m_frameAccum >= internal.m_frameDelta)
				{
					internal.m_internalFPS++;

					internal.m_totalTime += internal.m_frameDelta;
					internal.m_frameAccum -= internal.m_frameDelta;

					internal.m_function();
				}
			};
			inline void _endFrame(const std::function<void()> & postLoopFunction)
			{
				postLoopFunction();

				m_fTimerEnd = m_fTimerStart;
			};
			
		public:
			static void ScriptRegisterClass(chaiscript::ModulePtr pModule)
			{
				chaiscript::utility::add_class<gLoopTimer>(*pModule, "gLoopTimer",
					// Constructors
					{
						chaiscript::constructor<gLoopTimer()>(),
					},
					// Functions
					{
						{ chaiscript::fun(&gLoopTimer::SetMaxFPS), "SetMaxFPS" },
						{ chaiscript::fun(&gLoopTimer::GetRunningTime), "GetRunningTime" },
					}
				);
			};
		};
	};
};
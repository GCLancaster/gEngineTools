
#include "gEngine\System\gLoopTimer.h"
#include <array>
#undef main


int main(int argc, char* args[])
{
	GENG::gLoopTimer timer;
	bool bQuit = false;

	timer.init
		(
		[&]() -> void {
			//DBG("Preloop");
		},
		{
			GENG::gLoopTimer::InternalBlock(30, "InternalLoop 1", [&]() -> void {
				std::this_thread::sleep_for(std::chrono::microseconds(rand() % 10));
				//DBG("Internal");
			}),
			GENG::gLoopTimer::InternalBlock(120, "InternalLoop 2", [&]() -> void {
				std::this_thread::sleep_for(std::chrono::microseconds(rand() % 10));
				//DBG("Internal");
			})
		},
		[&]() -> void {
				//DBG("Postloop");
		}
	);

	while (!bQuit)
	{
		timer.loop();
	}
}
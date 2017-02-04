
#include "gEngine\Useful.h"
#include "gEngine\System\Resources\gSpriteSheet.h"
#include <array>
#include <memory>
#include <vector>
#undef main

int main(int argc, char* args[])
{
	Init();
	{
		GENG::Resources::gSpriteSheet spriteSheet;
		spriteSheet.load((R"(F:\My Documents\GitHub\gEngineTools\gEngineTest\gEngineTest_SpriteSheet\gEngineTest_SpriteSheet\testSheet.xml)"));
	}

	Destroy();
}

#include "gEngine\System\Logging\gLogger.h"
#include "gEngine\System\Messaging\gMessagePool.h"
#include "gEngine\System\Messaging\gListener.h"
#include "gEngine\System\Messaging\gMessenger.h"
#include "gEngine\System\Messaging\gMessagePoolMaster.h"
#undef main

class PosterClass : public GENG::Messaging::gMessenger<int>, public GENG::Messaging::gMessenger<double>
{
public:
	using GENG::Messaging::gMessenger<int>::Post;
	using GENG::Messaging::gMessenger<double>::Post;
};
class ListenerClass : public GENG::Messaging::gListener<int>, public GENG::Messaging::gListener<double>
{
public:
	virtual void RunMessage(GENG::Messaging::gMessage<int> msg)
	{
		GENG_DBG("Received int message: " << msg.data);
	}

	virtual void RunMessage(GENG::Messaging::gMessage<double> msg)
	{
		GENG_DBG("Received double message: " << msg.data);
	}
};

int main(int argc, char* args[])
{
	PosterClass a;
	ListenerClass b;

	GENG::Messaging::gMessage<int> msgInt;
	msgInt.data = 2;
	GENG::Messaging::gMessage<double> msgDbl;
	msgDbl.data = 2.5f;
	a.Post(msgInt);
	a.Post(msgDbl);

	GENG::Messaging::gMessagePoolMaster::Get()->UpdateAll();

	return true;
}
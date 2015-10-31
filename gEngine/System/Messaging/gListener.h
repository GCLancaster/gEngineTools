
#pragma once
#include <memory>
#include "gMessagePool.h"

namespace GENG
{
	namespace Messaging
	{
		class gListenerBase
		{
		protected:
			gListenerBase() {};
			virtual ~gListenerBase() {};
		};

		template<typename MsgType>
		class gListener : private gListenerBase
		{
		public:
			gListener()
			{
				gMessagePool<MsgType>::Get()->AddListener(this);
			}
			virtual ~gListener()
			{
				gMessagePool<MsgType>::Get()->RemoveListener(this);
			};

			virtual void RunMessage(gMessage<MsgType> msg) = 0;
		};
	};
};
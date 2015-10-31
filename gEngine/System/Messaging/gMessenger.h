
#pragma once
#include <memory>
#include "..\..\Useful.h"
#include "gMessagePool.h"

namespace GENG
{
	namespace Messaging
	{
		template<typename MsgType>
		class gMessenger
		{
		public:
			gMessenger()
			{
				gMessagePool<MsgType>::Get()->AddMessager(this);
			}
			virtual ~gMessenger()
			{
				gMessagePool<MsgType>::Get()->RemoveMessager(this);
			}

			void Post(gMessage<MsgType> & msg)
			{
				msg.sender = this;
				gMessagePool<MsgType>::Get()->AddMessage(msg);
			};
		};
	};
};
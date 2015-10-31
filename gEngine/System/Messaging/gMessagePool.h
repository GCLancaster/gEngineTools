
#pragma once
#include <memory>
#include <mutex>
#include <vector>
#include "..\..\Useful.h"

namespace GENG
{
	namespace Messaging
	{
		template<typename MsgType>
		class gListener;
		template<typename MsgType>
		class gMessenger;

		//////////////////////////////////////////////////////////////////////////
		// Message Class
		// - Sender
		// - Data 
		template<typename MsgType>
		struct gMessage
		{
			gMessenger<MsgType> *sender = nullptr;
			MsgType data;
		};

		//////////////////////////////////////////////////////////////////////////
		// MessengerPool base class
		class gMessagePoolBase
		{
		public:
			gMessagePoolBase() {};
			virtual ~gMessagePoolBase() {};

			virtual void Update() = 0;
		};
						
		//////////////////////////////////////////////////////////////////////////
		// MessengerPool class
		template<typename MsgType>
		class gMessagePool : public gMessagePoolBase
		{
		private:
			static gMessagePool<MsgType> * instance;
			
			std::recursive_mutex m_poolMutex;

			std::vector<gMessage<MsgType>> m_messages;

			std::vector<gMessenger<MsgType> *> m_messagers;
			std::vector<gListener<MsgType> *> m_listeners;

		public:
			gMessagePool() { DBGINIT; };
			~gMessagePool() { DBGDEST; };

			static gMessagePool<MsgType> * Get()
			{
				if (instance == nullptr)
				{
					gMessagePoolMaster::Get()->CreatePool<gMessagePool<MsgType>>(&instance);
				}
				return instance;
			}

			void Update()
			{
				std::lock_guard<std::recursive_mutex> guard(m_poolMutex);

				for (auto message : m_messages)
				{
					for (auto listener : m_listeners)
					{
						listener->RunMessage(message);
					}
				}
				m_messages.clear();
			}

			void AddMessage(const gMessage<MsgType> & message)
			{
				std::lock_guard<std::recursive_mutex> guard(m_poolMutex);

				m_messages.push_back(message);
			}
			void AddMessager(gMessenger<MsgType> * messager)
			{
				std::lock_guard<std::recursive_mutex> guard(m_poolMutex);

				m_messagers.push_back(messager);
			}
			void AddListener(gListener<MsgType> * listener)
			{
				std::lock_guard<std::recursive_mutex> guard(m_poolMutex);

				m_listeners.push_back(listener);
			}
			void RemoveMessager(gMessenger<MsgType> * messager)
			{
				std::lock_guard<std::recursive_mutex> guard(m_poolMutex);

				GENG::_removeObjectFromVector<gMessenger<MsgType>*>(m_messagers, messager);
			}
			void RemoveListener(gListener<MsgType> * listener)
			{
				std::lock_guard<std::recursive_mutex> guard(m_poolMutex);

				GENG::_removeObjectFromVector<gListener<MsgType>*>(m_listeners, listener);
			}
		};

		template<typename MsgType>
		__declspec(selectany) GENG::Messaging::gMessagePool<MsgType> * GENG::Messaging::gMessagePool<MsgType>::instance = nullptr;
	};
};
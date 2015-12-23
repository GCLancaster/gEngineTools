
#pragma once
#include <algorithm>
#include <memory>
#include <mutex>
#include <vector>
#include "..\..\Useful.h"
#include "gMessagePool.h"

namespace GENG
{
	namespace Messaging
	{
		class gMessagePoolMaster
		{
			static std::shared_ptr<gMessagePoolMaster> instance;

			std::vector<std::unique_ptr<gMessagePoolBase>> m_pools;

		public:
			gMessagePoolMaster() { DBGINIT; };
			~gMessagePoolMaster() { m_pools.clear(); DBGDEST; };

			static std::shared_ptr<gMessagePoolMaster> Get()
			{
				if (instance == nullptr)
					instance = std::make_shared<gMessagePoolMaster>();
				return instance;
			}

			template <typename MsgPoolType>
			void CreatePool(MsgPoolType ** pool)
			{
				m_pools.push_back(std::make_unique<MsgPoolType>());
				*pool = reinterpret_cast<MsgPoolType*>(m_pools.back().get());
			}

			void UpdateAll()
			{
				for (auto & pool : m_pools)
				{
					pool->Update();
				}
			};

			void ClearAll()
			{
				m_pools.clear();
			}
		};

		__declspec(selectany) std::shared_ptr<gMessagePoolMaster> gMessagePoolMaster::instance;
	};
};
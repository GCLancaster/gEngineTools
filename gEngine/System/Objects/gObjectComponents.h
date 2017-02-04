
#pragma once

#include <memory>
#include <vector>
#include <array>
#include <map>
#include <typeindex>
#include <type_traits>
#include "..\..\Useful.h"
#include "gHandleContainer.h"

namespace GENG
{
	namespace Object
	{
		class ObjectBase
		{
		public:
			//protected:
			std::vector<gHandle> m_components;

		public:
			ObjectBase(){};
			virtual ~ObjectBase(){};

			void AttachComponent(const gHandle & handle)
			{
				if (SHandleValid(handle))
					m_components.push_back(handle);
			}

			//template<typename T>
			//void AttachComponent(std::shared_ptr<gEntryManager> m_manager, std::shared_ptr<T> pObj)
			//{
			//	GENG::Object::gHandle handle = pObj->GetHandle();
			//	if (!SHandleValid(handle))
			//	{
			//		if (m_manager->AddEntry(handle, pObj))
			//			AttachComponent(handle);
			//	}
			//}

			//template<typename T, typename... Args>
			//void AttachComponent(std::shared_ptr<gEntryManager> m_manager, Args&&... args)
			//{
			//	GENG::Object::gHandle handle;
			//	if (m_manager->AddEntry<T>(handle, std::forward<Args>(args)...))
			//	{
			//		if (SHandleValid(handle))
			//			AttachComponent(handle);
			//	}
			//}

			void RemoveComponent(const gHandle & handle)
			{
				_removeObjectFromVector(m_components, handle);
			}

			//void RemoveComponent(std::shared_ptr<gEntryManager> m_manager, gHandle & handle)
			//{
			//	if (SHandleValid(handle))
			//	{
			//		if (m_manager->RemoveEntry(handle))
			//			RemoveComponent(handle);
			//	}
			//}

			//template<typename T>
			//void RemoveComponent(std::shared_ptr<gEntryManager> m_manager, std::shared_ptr<T> pObj)
			//{
			//	gHandle handle = pObj->GetHandle();
			//	RemoveComponent(m_manager, handle);
			//}

			void RunComponents(std::shared_ptr<GENG::Object::gEntryManager> manager)
			{
				if (manager == nullptr)
					DERROR("Invalid manager passed in!");

				for (auto component : m_components)
				{
					if (SHandleValid(component))
					{
						RunComponent(component, manager);
					}
				}
			}

		protected:
			virtual void RunComponent(gHandle & component, std::shared_ptr<GENG::Object::gEntryManager> manager) = 0;
		};

		class ComponentBase
		{
		protected:
			gHandle m_handle;

		public:
			ComponentBase(){};
			virtual ~ComponentBase(){};

			GENG::Object::gHandle GetHandle() const { return m_handle; }
			void SetHandle(GENG::Object::gHandle handle) { m_handle = handle; }
		};
	};
};
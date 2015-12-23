
#pragma once

#include <memory>
#include <vector>
#include <array>
#include <map>
#include <typeindex>
#include <type_traits>
#include "..\..\Useful.h"

namespace GENG
{
	namespace Object
	{
		struct gHandle
		{
			gHandle() {};
			gHandle(const uint32_t & index, const uint32_t & type) :
				m_index(index), m_type(type)
			{};
			~gHandle() {};

			uint32_t m_index = UINT32_MAX;
			uint32_t m_type = UINT32_MAX;
#ifdef _DEBUG
			std::string m_typeName = "";
#endif // _DEBUG

			bool operator == (const gHandle & rhs) const
			{
				return m_index == rhs.m_index && m_type == rhs.m_type;
			}
		};

		static bool SHandleValid(const gHandle & handle)
		{
			return (handle.m_index != UINT32_MAX) && (handle.m_index != UINT32_MAX);
		}

		template<typename T, uint32_t MAX_ENTRIES>
		class gHandleContainer : public gHandleContainerBase
		{
		protected:
			struct EntryEntry
			{
				EntryEntry() {};
				EntryEntry(const uint32_t & nextFreeIndex) :
					m_nextIndex(nextFreeIndex)
				{};

				uint32_t m_index = UINT32_MAX;
				uint32_t m_nextIndex = UINT32_MAX;
				bool m_bActive = false;
				bool m_bEndOfList = false;

				// Pointer to the entity.
				std::shared_ptr<T> m_dataEntry;
			};

			std::recursive_mutex m_mutex;

			bool bInitiated = false;
			uint32_t m_firstFreeEntry = 0;
			uint32_t m_activeEntries = 0;
			uint32_t m_MAX_ENTRIES = MAX_ENTRIES;
			std::array<EntryEntry, MAX_ENTRIES> m_handles;
		public:
			gHandleContainer()
			{
				DBGINIT;
				m_typeInfo = typeid(T);
				m_type = m_typeInfo.hash_code();
				m_typeName = m_typeInfo.name();
				Initalise();
			}
			~gHandleContainer()
			{
				DBGDEST;
				Destroy();
			}

			virtual void Initalise()
			{
				// Reset the handles list - have them all point to the next one
				for (unsigned int i = 0; i < m_MAX_ENTRIES; ++i)
					m_handles[i] = EntryEntry(i + 1);

				m_handles[m_MAX_ENTRIES - 1].m_nextIndex = UINT32_MAX;
				m_handles[m_MAX_ENTRIES - 1].m_bEndOfList = true;
			}
			virtual void Destroy()
			{
				for (size_t i = 0; i < m_handles.size(); i++)
				{
					if (m_handles[i].m_dataEntry.use_count() > 1)
					{
						throw std::logic_error("This handle is in use elsewhere!");
					}
				}
			}

		protected:
			virtual bool _canAddEntry(const gHandle & handle)
			{
				if (handle.m_type != m_type)
					return GENG_EXIT_FAILURE;
				if (m_MAX_ENTRIES <= 0 || m_activeEntries >= m_MAX_ENTRIES)
					return GENG_EXIT_FAILURE;
				if (m_firstFreeEntry == UINT32_MAX || m_firstFreeEntry >= m_MAX_ENTRIES || m_handles[m_firstFreeEntry].m_bActive)
					return GENG_EXIT_FAILURE;
				return GENG_EXIT_SUCCESS;
			}
			virtual bool _canUpdateEntry(const gHandle & handle)
			{
				if (handle.m_type != m_type)
					return GENG_EXIT_FAILURE;
				if ((handle.m_index >= m_handles.size()) || (!m_handles[handle.m_index].m_bActive))
					return GENG_EXIT_FAILURE;
				return GENG_EXIT_SUCCESS;
			}
			virtual bool _canGetEntry(const gHandle & handle)
			{
				if (handle.m_type != m_type)
					return GENG_EXIT_FAILURE;
				if ((handle.m_index >= m_handles.size()) || (!m_handles[handle.m_index].m_bActive))
					return GENG_EXIT_FAILURE;
				return GENG_EXIT_SUCCESS;
			}
			virtual bool _canRemoveEntry(const gHandle & handle)
			{
				if (handle.m_type != m_type)
					return GENG_EXIT_FAILURE;
				if ((handle.m_index >= m_handles.size()) || (!m_handles[handle.m_index].m_bActive))
					return GENG_EXIT_FAILURE;
				return GENG_EXIT_SUCCESS;
			}
			virtual bool _addEntry(gHandle & handle, void ** pItem)
			{
				std::lock_guard<std::recursive_mutex> guard(m_mutex);

				if (!_canAddEntry(handle))
					return GENG_EXIT_FAILURE;

				const auto newIndex = m_firstFreeEntry;

				m_firstFreeEntry = m_handles[newIndex].m_nextIndex;

				// Add the handle.
				m_handles[newIndex].m_index = newIndex;
				m_handles[newIndex].m_nextIndex = 0;
				m_handles[newIndex].m_bActive = true;
				m_handles[newIndex].m_bEndOfList = (newIndex == (m_MAX_ENTRIES - 1));

				void * pEntryAddress = &m_handles[newIndex].m_dataEntry;
				(*pItem) = pEntryAddress;

				m_activeEntries++;

				// Update the handle
				handle.m_index = newIndex;

				return GENG_EXIT_SUCCESS;
			}
			virtual bool _updateEntry(gHandle & handle, void ** pItem)
			{
				std::lock_guard<std::recursive_mutex> guard(m_mutex);

				if (!_canUpdateEntry(handle))
					return GENG_EXIT_FAILURE;
				
				// Update the handle.
				void * pEntryAddress = &m_handles[handle.m_index].m_dataEntry;
				(*pItem) = pEntryAddress;

				return GENG_EXIT_SUCCESS;
			}
			virtual bool _getEntry(gHandle & handle, void ** pItem)
			{
				std::lock_guard<std::recursive_mutex> guard(m_mutex);

				if (!_canGetEntry(handle))
					return GENG_EXIT_FAILURE;
				
				// Update the item pointer
				(*pItem) = &m_handles[handle.m_index].m_dataEntry;

				return GENG_EXIT_SUCCESS;
			}
			virtual bool _removeEntry(gHandle & handle)
			{
				std::lock_guard<std::recursive_mutex> guard(m_mutex);

				if (!_canRemoveEntry(handle))
					return GENG_EXIT_FAILURE;

				const auto index = handle.m_index;

				// Set Handle to null
				handle = gHandle();

				// Reset the index location.
				m_handles[index].m_nextIndex = m_firstFreeEntry;
				m_handles[index].m_bActive = false;
				m_handles[index].m_dataEntry.reset();
				m_firstFreeEntry = index;

				m_activeEntries--;

				return GENG_EXIT_SUCCESS;
			}
			virtual bool _runFunc(std::function<void(void*)> func)
			{
				for (auto entry : m_handles)
				{
					if (entry.m_bActive && entry.m_dataEntry != nullptr)
					{
						func(&entry.m_dataEntry);
					}
				}
				return true;
			}
		};

		class gHandleContainerBase
		{
		protected:
			std::type_index m_typeInfo = typeid(this);
			std::string m_typeName = m_typeInfo.name();
			size_t m_type = 0;

		public:
			gHandleContainerBase() { Initalise(); };
			virtual ~gHandleContainerBase() { Destroy(); };

			virtual void Initalise() {};
			virtual void Destroy() {};

		public:
			template<typename T, typename... Args>
			bool AddEntry(gHandle & handle, Args&&... args)
			{
				void * pData = nullptr;
				void ** pDataPtr = &pData;
				bool rtn = _addEntry(handle, pDataPtr);
				if (rtn)
				{
					std::shared_ptr<T> * pEntry = (std::shared_ptr<T> *)(*pDataPtr);
					if (pEntry != nullptr)
						(*pEntry) = std::make_shared<T>(std::forward<Args>(args)...);
					(*pEntry)->SetHandle(handle);
				}
				return rtn;
			};

			template<typename T>
			bool AddEntry(gHandle & handle, std::shared_ptr<T> pObj)
			{
				void * pData = nullptr;
				void ** pDataPtr = &pData;
				bool rtn = _addEntry(handle, pDataPtr);
				if (rtn)
				{
					std::shared_ptr<T> * pEntry = (std::shared_ptr<T> *)(*pDataPtr);
					if (pEntry != nullptr)
						(*pEntry) = pObj;
					(*pEntry)->SetHandle(handle);
				}
				return rtn;
			};

			template<typename T, typename... Args>
			bool UpdateEntry(gHandle & handle, Args&&... args)
			{
				void * pData = nullptr;
				void ** pDataPtr = &pData;
				bool rtn = _updateEntry(handle, pDataPtr);
				if (rtn)
				{
					std::shared_ptr<T> * pEntry = (std::shared_ptr<T> *)(*pDataPtr);
					if (pEntry != nullptr)
						(*pEntry) = std::make_shared<T>(std::forward<Args>(args)...);
					(*pEntry)->SetHandle(handle);
				}
				return rtn;
			};

			template<typename T>
			bool UpdateEntry(gHandle & handle, std::shared_ptr<T> pObj)
			{
				void * pData = nullptr;
				void ** pDataPtr = &pData;
				bool rtn = _updateEntry(handle, pDataPtr);
				if (rtn)
				{
					std::shared_ptr<T> * pEntry = (std::shared_ptr<T> *)(*pDataPtr);
					if (pEntry != nullptr)
						(*pEntry) = pObj;
					(*pEntry)->SetHandle(handle);
				}
				return rtn;
			};

			template<typename T>
			std::shared_ptr<T> GetSharedEntry(gHandle & handle)
			{
				void * ppVoidItem = nullptr;
				std::shared_ptr<T> item;

				_getEntry(handle, &ppVoidItem);

				if (ppVoidItem != nullptr)
					item = *(reinterpret_cast<std::shared_ptr<T>*>(ppVoidItem));
				
				return item;
			}
		
			void * GetSharedEntry(gHandle & handle)
			{
				void * ppVoidItem = nullptr;

				_getEntry(handle, &ppVoidItem);

				return ppVoidItem;
			}

			bool RemoveEntry(gHandle & handle)
			{
				return _removeEntry(handle);
			};

			template<typename T>
			bool RemoveEntry(std::shared_ptr<T> pObj)
			{
				gHandle handle = pObj->GetHandle();
				bool rtn = RemoveEntry(handle);
				if (rtn)
					pObj->SetHandle(handle);
				return rtn;
			}

			std::string GetTypeName() const { return m_typeName; };

			void RunFunc(std::function<void(void*)> func)
			{
				_runFunc(func);
			};
		protected:
			virtual bool _canAddEntry(const gHandle & handle) = 0;
			virtual bool _canUpdateEntry(const gHandle & handle) = 0;
			virtual bool _canGetEntry(const gHandle & handle) = 0;
			virtual bool _canRemoveEntry(const gHandle & handle) = 0;
			virtual bool _addEntry(gHandle & handle, void ** pItem) = 0;
			virtual bool _updateEntry(gHandle & handle, void ** pItem) = 0;
			virtual bool _getEntry(gHandle & handle, void ** pItem) = 0;
			virtual bool _removeEntry(gHandle & handle) = 0;
			virtual bool _runFunc(std::function<void(void*)> func) = 0;
		};

		class gEntryManager
		{
		protected:
			uint32_t m_defualtMaxSize = 128;
			std::map<uint32_t, std::unique_ptr<gHandleContainerBase>> m_containers;

		public:
			gEntryManager()
			{
				DBGINIT;
				Initalise();
			}
			~gEntryManager()
			{
				DBGDEST;
				m_containers.clear();
			}

			void Initalise()
			{
			};
			void Destroy()
			{
			};

			template<typename T, uint32_t MAX_ENTRIES = m_defualtMaxSize>
			void AddEntryType()
			{
				m_containers[static_cast<uint32_t>(typeid(T).hash_code())] = std::make_unique<gHandleContainer<T, MAX_ENTRIES>>();
			};

			template<typename T, typename... Args>
			gHandle AddEntry(Args&&... args)
			{
				gHandle handle(UINT32_MAX, static_cast<uint32_t>(typeid(T).hash_code()));

				auto & pManager = _getManager(handle.m_type);
#ifdef _DEBUG
				handle.m_typeName = pManager->GetTypeName();
#endif // _DEBUG
				if(!pManager->AddEntry<T>(handle, std::forward<Args>(args)...))
					return gHandle();
				return handle;
			};

			template<typename T>
			gHandle AddEntry(std::shared_ptr<T> pObj)
			{
				gHandle handle(UINT32_MAX, typeid(T).hash_code());

				auto & pManager = _getManager(handle.m_type);
#ifdef _DEBUG
				handle.m_typeName = pManager->GetTypeName();
#endif // _DEBUG
				if (!pManager->AddEntry<T>(handle, pObj))
					return gHandle();
				return handle;
			}

			template<typename T, typename... Args>
			bool UpdateEntry(gHandle & handle, Args&&... args)
			{
				if (!SHandleValid(handle))
					return false;

				auto & pManager = _getManager(typeid(T).hash_code());
				return pManager->UpdateEntry<T>(handle, std::forward<Args>(args)...);
			};

			template<typename T>
			bool UpdateEntry(gHandle & handle, std::shared_ptr<T> pObj)
			{
				if (!SHandleValid(handle))
					return false;

				auto & pManager = _getManager(typeid(T).hash_code());
				return pManager->UpdateEntry<T>(handle, pObj);
			};

			template<typename T>
			std::shared_ptr<T> GetSharedEntry(gHandle & handle)
			{
				if (!SHandleValid(handle))
					return false;

				auto & pManager = _getManager(static_cast<uint32_t>(typeid(T).hash_code()));
				return pManager->GetSharedEntry<T>(handle);
			}

			void * GetSharedEntry(gHandle & handle)
			{
				if (!SHandleValid(handle))
					return false;
				
				auto & pManager = _getManager(handle.m_type);
				return pManager->GetSharedEntry(handle);
			}

			bool RemoveEntry(gHandle & handle)
			{
				if (!SHandleValid(handle))
					return false;

				auto & pManager = _getManager(handle.m_type);
				return pManager->RemoveEntry(handle);
			};

			template<typename T>
			bool RemoveEntry(std::shared_ptr<T> pObj)
			{
				gHandle handle = pObj->GetHandle();
				if (!SHandleValid(handle))
					return false;

				auto & pManager = _getManager(typeid(T).hash_code());
				return pManager->RemoveEntry<T>(pObj);
			};

			template<typename T>
			void RunFunc(std::function<void(std::shared_ptr<T>)> func)
			{
				auto function = [&func](void * pData)
				{
					std::shared_ptr<T> pObj = *reinterpret_cast<std::shared_ptr<T>*>(pData);
					if (pObj != nullptr)
						func(pObj);
				};
				_getManager(static_cast<uint32_t>(typeid(T).hash_code()))->RunFunc(function);
			}
		protected:
			std::unique_ptr<gHandleContainerBase> & _getManager(const uint32_t & type)
			{
				if (m_containers.find(type) == m_containers.end())
					DERROR("Cannot retrieve invalid type-container!");
				return m_containers[type];
			};
		};
	};
};
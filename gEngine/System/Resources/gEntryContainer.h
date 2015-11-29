
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
	};
	template<typename T, uint32_t MAX_ENTRIES>
	class gEntryContainer : public gEntryContainerBase
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
		gEntryContainer()
		{
			DBGINIT;
			m_typeInfo = typeid(T);
			m_type = m_typeInfo.hash_code();
			m_typeName = m_typeInfo.name();
			initalise();
		}
		~gEntryContainer()
		{
			DBGDEST;
			destroy();
		}

		virtual void initalise()
		{
			// Reset the handles list - have them all point to the next one
			for (unsigned int i = 0; i < m_MAX_ENTRIES; ++i)
				m_handles[i] = EntryEntry(i + 1);

			m_handles[m_MAX_ENTRIES - 1].m_nextIndex = UINT32_MAX;
			m_handles[m_MAX_ENTRIES - 1].m_bEndOfList = true;
		}
		virtual void destroy()
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
			const auto index = handle.m_index;
			if ((index >= m_handles.size()) || (!m_handles[index].m_bActive))
				return GENG_EXIT_FAILURE;
			return GENG_EXIT_SUCCESS;
		}
		virtual bool _canGetEntry(const gHandle & handle)
		{
			const auto index = handle.m_index;
			if ((index >= m_handles.size()) || (!m_handles[index].m_bActive))
				return GENG_EXIT_FAILURE;
			return GENG_EXIT_SUCCESS;
		}
		virtual bool _canRemoveEntry(const gHandle & handle)
		{
			const auto index = handle.m_index;
			if ((index >= m_handles.size()) || (!m_handles[index].m_bActive))
				return GENG_EXIT_FAILURE;
			return GENG_EXIT_SUCCESS;
		}
		virtual bool _addEntry(gHandle & handle, void ** pItem)
		{
			std::lock_guard<std::recursive_mutex> guard(m_mutex);

			_canAddEntry(handle);

			const auto newIndex = m_firstFreeEntry;

			m_firstFreeEntry = m_handles[newIndex].m_nextIndex;

			// Add the handle.
			m_handles[newIndex].m_index = newIndex;
			m_handles[newIndex].m_nextIndex = 0;
			m_handles[newIndex].m_bActive = true;
			m_handles[newIndex].m_bEndOfList = (newIndex == (m_MAX_ENTRIES - 1));

			//std::shared_ptr<T> * pSharedPtr = (std::shared_ptr<T> *)(pItem);
			//m_handles[newIndex].m_dataEntry = *pSharedPtr;
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

			_canUpdateEntry(handle);

			const auto index = handle.m_index;

			// Update the handle.
			//std::shared_ptr<T> * pSharedPtr = (std::shared_ptr<T> *)(pItem);
			//m_handles[index].m_dataEntry = *pSharedPtr;
			void * pEntryAddress = &m_handles[index].m_dataEntry;
			(*pItem) = pEntryAddress;

			return GENG_EXIT_SUCCESS;
		}
		virtual bool _getEntry(gHandle & handle, void ** pItem)
		{
			std::lock_guard<std::recursive_mutex> guard(m_mutex);

			_canGetEntry(handle);

			const auto index = handle.m_index;

			// Update the item pointer
			(*pItem) = &m_handles[index].m_dataEntry;

			return GENG_EXIT_SUCCESS;
		}
		virtual bool _removeEntry(gHandle & handle)
		{
			std::lock_guard<std::recursive_mutex> guard(m_mutex);

			_canRemoveEntry(handle);

			const auto index = handle.m_index;

			// Set Handle to null
			handle.m_index = UINT32_MAX;
			handle.m_type = 0;

			// Reset the index location.
			m_handles[index].m_nextIndex = m_firstFreeEntry;
			m_handles[index].m_bActive = false;
			m_handles[index].m_dataEntry.reset();
			m_firstFreeEntry = index;

			m_activeEntries--;

			return GENG_EXIT_SUCCESS;
		}
	};

	class gEntryContainerBase
	{
	protected:
		std::type_index m_typeInfo = typeid(this);
		std::string m_typeName = m_typeInfo.name();
		uint32_t m_type = 0;

	public:
		gEntryContainerBase() { initalise(); };
		virtual ~gEntryContainerBase() { destroy(); };

		virtual void initalise() {};
		virtual void destroy() {};

	public:
		template<typename T, typename... Args>
		bool addEntry(gHandle & handle, Args&&... args)
		{
			if (!_checkType<T>(handle) || !_canAddEntry(handle))
				return GENG_EXIT_FAILURE;
			
			void * pData = nullptr;
			void ** pDataPtr = &pData;
			bool rtn = _addEntry(handle, pDataPtr);
			std::shared_ptr<T> * pEntry = (std::shared_ptr<T> *)(*pDataPtr);
			if (pEntry != nullptr)
				(*pEntry) = std::make_shared<T>(std::forward<Args>(args)...);
			return rtn;
		};

		template<typename T, typename... Args>
		bool updateEntry(gHandle & handle, Args&&... args)
		{
			if (!_checkType<T>(handle) || !_canUpdateEntry(handle))
				return GENG_EXIT_FAILURE;
			return _updateEntry(handle, &std::make_shared<T>(std::forward<Args>(args)...));
		};

		template<typename T>
		std::shared_ptr<T> getSharedEntry(gHandle & handle)
		{
			if (!_checkType<T>(handle) || !_canGetEntry(handle))
				return std::shared_ptr<T>();

			void * ppVoidItem = nullptr;
			if (_getEntry(handle, &ppVoidItem))
			{
				std::shared_ptr<T> * pItem = reinterpret_cast<std::shared_ptr<T>*>(ppVoidItem);
				if (pItem != nullptr)
					return *pItem;
			}
			return std::shared_ptr<T>();
		}
		void * getEntry(gHandle & handle)
		{
			if (!_checkType(handle) || !_canGetEntry(handle))			
				return nullptr;

			void * ppVoidItem = nullptr;
			if (_getEntry(handle, &ppVoidItem) && (ppVoidItem != nullptr))
				return ppVoidItem;

			return nullptr;
		}

		bool removeEntry(gHandle & handle)
		{
			if (!_canRemoveEntry(handle))
				return GENG_EXIT_FAILURE;
			return _removeEntry(handle);
		};

		gHandle createHandle()
		{
#ifdef _DEBUG
			std::string m_typeName = "";
			gHandle handle(UINT32_MAX, m_type);
			handle.m_typeName = m_typeInfo.name();
			return handle;
#else
			return gHandle(UINT32_MAX, m_type);
#endif // _DEBUG
		}

		std::string getTypeName() const { return m_typeName; };

	protected:
		template<typename T = gEntryContainerBase>
		bool _checkType(gHandle & handle)
		{
			if ((typeid(T) != typeid(gEntryContainerBase)) && (m_typeInfo != typeid(T)))
				return GENG_EXIT_FAILURE;//DERROR("Incorrect Type specified!");
			if (m_type != handle.m_type)
				return GENG_EXIT_FAILURE;//DERROR("Incorrect Handle argument!");
			return GENG_EXIT_SUCCESS;
		}

		virtual bool _canAddEntry(const gHandle & handle) = 0;
		virtual bool _canUpdateEntry(const gHandle & handle) = 0;
		virtual bool _canGetEntry(const gHandle & handle) = 0;
		virtual bool _canRemoveEntry(const gHandle & handle) = 0;
		virtual bool _addEntry(gHandle & handle, void ** pItem) = 0;
		virtual bool _updateEntry(gHandle & handle, void ** pItem) = 0;
		virtual bool _getEntry(gHandle & handle, void ** pItem) = 0;
		virtual bool _removeEntry(gHandle & handle) = 0;
	};
		
	class gEntryManager
	{
	protected:
		uint32_t m_defualtMaxSize = 128;
		std::map<uint32_t, std::unique_ptr<gEntryContainerBase>> m_containers;

	public:
		gEntryManager()
		{
			DBGINIT;
			initalise();
		}
		~gEntryManager()
		{
			DBGDEST;
			m_containers.clear();
		}

		void initalise()
		{
		};
		void destroy()
		{
		};

		template<typename T, uint32_t MAX_ENTRIES = m_defualtMaxSize>
		void addEntryType()
		{
			m_containers[typeid(T).hash_code()] = std::make_unique<gEntryContainer<T, MAX_ENTRIES>>();
		};
		
		template<typename T, typename... Args>
		gHandle addEntry(Args&&... args)
		{
			gHandle handle(UINT32_MAX, typeid(T).hash_code());

			auto & pManager = _getManager(handle.m_type);
#ifdef _DEBUG
			handle.m_typeName = pManager->getTypeName();
#endif // _DEBUG
			pManager->addEntry<T>(handle, std::forward<Args>(args)...);
			return handle;
		};
		
		template<typename T, typename... Args>
		bool updateEntry(gHandle & handle, Args&&... args)
		{
			return _getManager(handle.m_type)->updateEntryFromArgs<T>(handle, std::forward<Args>(args)...);
		};

		template<typename T>
		std::shared_ptr<T> getSharedEntry(gHandle & handle)
		{
			return _getManager(handle.m_type)->getSharedEntry<T>(handle);
		}
		void * getSharedEntry(gHandle & handle)
		{
			return _getManager(handle.m_type)->getEntry(handle);
		}

		bool removeEntry(gHandle & handle)
		{
			return _getManager(handle.m_type)->removeEntry(handle);
		};

	protected:
		std::unique_ptr<gEntryContainerBase> & _getManager(const uint32_t & type)
		{
			if (m_containers.find(type) == m_containers.end())
				DERROR("Cannot retrieve invalid type-container!");
			return m_containers[type];
		};
	};
};

#pragma once

#include <memory>
#include <vector>
#include <array>
#include <map>
#include <typeindex>
#include <type_traits>
#include <functional>
#include "..\..\Useful.h"
#include "..\Scripting\gScriptingState.h"

namespace GENG
{
	namespace Memory
	{
		typedef uint64_t stack_t;
		struct stackObject_n
		{
			uint32_t m_id = UINT32_MAX;
			uint32_t m_allocationID = 0;
			void * m_object = nullptr;

			stackObject_n(){};
			stackObject_n(stackObject_n && rhs) : m_id(std::move(rhs.m_id)), m_allocationID(std::move(rhs.m_allocationID)), m_object(std::move(rhs.m_object)) {};
			stackObject_n(const uint32_t & id, const uint32_t & allocationID, void * pObject) : m_id(id), m_allocationID(allocationID), m_object(pObject) {};
		};

		//struct stackObject_n
		//{
		//	uint32_t m_id = UINT32_MAX;
		//	uint32_t m_allocationID = 0;
		//	deleted_unique_ptr<void> m_object = deleted_unique_ptr<void>(nullptr, nullptr);

		//	stackObject_n(){};
		//	stackObject_n(stackObject_n && rhs)
		//	{
		//		m_id = rhs.m_id;
		//		m_object = std::move(rhs.m_object);
		//	};

		//	~stackObject_n()
		//	{
		//		LOG("Deleting stackObject");
		//	}
		//};
		struct stackAllocation_n
		{
			union
			{
				struct { stack_t left; stack_t right; } s;
				stack_t p[2];
			} m_bounds;

			stackAllocation_n() {};
			stackAllocation_n(const stack_t & left, const stack_t & right) { m_bounds.s.left = left; m_bounds.s.right = right; };
			inline static stackAllocation_n CreateLeftSide(const stack_t & pos, const stack_t & size) { return stackAllocation_n(pos, pos + size); };
			inline static stackAllocation_n CreateRightSide(const stack_t & pos, const stack_t & size) { return stackAllocation_n(pos - size, pos); };
		};

		template<uint64_t SIZE>
		struct stack
		{
			using tObjectAllocation = std::pair<stackObject_n, stackAllocation_n>;
			std::array<uint8_t, SIZE> m_pBuffer;
			uint8_t * m_pMarker;

			std::vector<stackAllocation_n> m_allocSpace;
			std::vector<stackAllocation_n> m_freeSpace;			
			std::map<uint32_t, deleted_unique_ptr<stackObject_n>> m_objects;

			uint64_t m_ids;

			size_t m_startingPosition = 0;
			size_t m_currentPosition = 0;
			size_t m_totalSpace = SIZE;

			stack() : m_pMarker(m_pBuffer.data()), m_allocSpace(), m_freeSpace(0), m_ids(0)
			{
				m_freeSpace.push_back(stackAllocation_n::CreateLeftSide(m_startingPosition, m_startingPosition + m_totalSpace));
			}

			~stack()
			{
				LOG("Deleting stack object");
			}
			
			void DebugOutput()
			{
				uint32_t l = 0;
				uint32_t r = SIZE;// [1] - m_stackBounds[0];
				double ratio = 100.0f / r;

				uint32_t lPos = (m_stackMarker[0].m_movePosition - m_stackBounds[0]) * ratio;
				uint32_t rPos = (m_stackMarker[1].m_movePosition - m_stackBounds[0]) * ratio;

				std::string header;
				std::string internalSides;
				std::string internalAlloc;
				for (int i = 0; i < 100; i++)
				{
					header += "-";
					if (i < (lPos - 1))			internalSides += "_";
					else if (i == lPos - 1)		internalSides += "L";
					else if (i >= (rPos + 1))	internalSides += "_";
					else if (i == rPos)			internalSides += "R";
					else						internalSides += "-";
				}

				int count = 0;
				for (auto aIter : m_allocSpace)
				{
					uint32_t lPosIter = (aIter.m_bounds[eSSLeft] - m_stackBounds[0]) * ratio;
					uint32_t rPosIter = (aIter.m_bounds[eSSRight] - m_stackBounds[0]) * ratio;
					uint32_t length = rPosIter - lPosIter;

					for (int i = 0; i < length; i++)
					{
						internalAlloc += std::to_string(count);
					}
					count++;
				}

				for (int i = 0; i < (rPos - lPos); i++)
				{
					internalAlloc += " ";
				}

				count = 0;
				for (auto aIter : m_stackMarker[1].m_allocations)
				{
					uint32_t lPosIter = (aIter.m_bounds[eSSLeft] - m_stackBounds[0]) * ratio;
					uint32_t rPosIter = (aIter.m_bounds[eSSRight] - m_stackBounds[0]) * ratio;
					uint32_t length = rPosIter - lPosIter;

					for (int i = 0; i < length; i++)
					{
						internalAlloc += std::to_string(count);
					}
					count++;
				}


				LOG(header);
				LOG(internalSides);
				LOG(internalAlloc);
				LOG(internalAlloc);
				LOG(internalAlloc);
				LOG(internalSides);
				LOG(header);
			}

			bool CalculateFreeSpace(const size_t & neededSize)
			{
				size_t totalFreeSpace = 0;
				for (auto free : m_freeSpace)
					totalFreeSpace += (free.m_bounds.p[1] - free.m_bounds.p[0]);

				if (totalFreeSpace < neededSize)
					return false;

				size_t remainingFreeSpace = totalFreeSpace - neededSize;
				m_currentPosition += neededSize;
				m_freeSpace.clear();				

				if (remainingFreeSpace > 0)
					m_freeSpace.emplace_back(stackAllocation_n::CreateLeftSide(m_currentPosition, remainingFreeSpace));
			};

			template<typename T, typename... Args>
			std::shared_ptr<T> AllocateObject(Args&&... args)
			{
				using tObjectPtr = std::shared_ptr<T>;
				const size_t sizeofPTR = sizeof(std::shared_ptr<T>);
				const size_t sizeofOBJ = sizeof(T);
				const auto id = m_ids++;
				uint8_t * pMarkerStart = m_pMarker;

				// Determine if free space is available
				if (!CalculateFreeSpace(sizeofOBJ))
					return nullptr;

				// Allocate object space
				LOG("Allocating memory space" << sizeof(T));
				T * pObject = reinterpret_cast<T*>(m_pMarker);
				*pObject = std::move(T(std::forward<Args>(args)...));
				m_pMarker += sizeofOBJ;

				auto newAllocation = (stackAllocation_n::CreateLeftSide(reinterpret_cast<stack_t>(pMarkerStart), sizeof(T)));

				// Document the allocation
				m_allocSpace.emplace_back(std::move(newAllocation));
				m_objects[id] = deleted_unique_ptr<stackObject_n>(new stackObject_n(id, m_allocSpace.size() - 1, reinterpret_cast<void*>(pObject)), 
				[this](stackObject_n * ptr)
				{
					LOG("Clearing memory space" << sizeof(T));
					m_allocSpace.erase((m_allocSpace.begin() + ptr->m_allocationID));
					delete ptr;
				});

				return std::shared_ptr<T>(pObject, [this](T * ptr)
				{
					auto iterFnd = m_objects.begin();
					for (; iterFnd != m_objects.end(); ++iterFnd)
					{
						if (iterFnd->second.get()->m_object == reinterpret_cast<void*>(ptr))
							break;
					}

					if (iterFnd != m_objects.end())
					{
						m_objects.erase(iterFnd);
					}
				});
			};
		};


		template<typename T>
		struct gStackObject
		{
			std::shared_ptr<T> m_pObject;

			T * operator->() const
			{
				return (m_pObject->get());
			}
		};
		template<>
		struct gStackObject<void>
		{
			void * m_pObject;

			void * operator->() const
			{
				return (m_pObject);
			}
		};

		template<uint64_t SIZE>
		class gStackAllocator
		{
			enum eStackSide
			{
				eSSLeft,
				eSSRight,

				eSSCount
			};

			struct stackAllocation
			{
				std::type_index m_type;
				stack_t m_size;
				stack_t m_bounds[eSSCount];
				stackAllocation() : m_type(typeid(stackAllocation)), m_size(0)
				{
					m_bounds[eSSLeft] = 0;
					m_bounds[eSSRight] = 0;
				}
				stackAllocation(const eStackSide & side, const stack_t & position, const stack_t & size) : m_type(typeid(stackAllocation)), m_size(size)
				{
					if (side == eSSLeft)
					{
						m_bounds[eSSLeft] = position;
						m_bounds[eSSRight] = position + m_size;
					}
					else
					{
						m_bounds[eSSLeft] = position - m_size;
						m_bounds[eSSRight] = position;
					}
				}
			};
			struct stackSide
			{
				eStackSide m_side = eSSCount;
				stack_t m_holdPosition = 0;
				stack_t m_movePosition = 0;
				stack_t * m_opPosition = nullptr;
				std::vector<stackAllocation> m_allocations;
				uint32_t m_marker;

				stackSide(){};
				stackSide(const eStackSide & side, const stack_t & staticLocation, stack_t * opPosition) : 
					m_side(side), m_holdPosition(staticLocation), m_movePosition(staticLocation), m_opPosition(opPosition),
					m_marker(0)
				{
				};
				~stackSide() {};

				bool AllocateStackObject(const stack_t & size, stackAllocation & outputAllocation)
				{
					stack_t currPos = m_movePosition;
					stack_t nextPos = (m_side == eSSLeft) ? (m_movePosition + size) : (m_movePosition - size);
					
					bool bInvalid =
						((m_side == eSSLeft) ? (currPos >= *m_opPosition) : (currPos <= *m_opPosition)) &&
						((m_side == eSSLeft) ? (nextPos > *m_opPosition) : (nextPos < *m_opPosition));
						
					if (bInvalid)
					{
						LOG("Allocation invalid");
						return false;
					}

					m_allocations.emplace_back(m_side, currPos, size);
					m_marker = m_allocations.size();
					
					m_movePosition = nextPos;
					outputAllocation = m_allocations.back();

					return true;
				}
				bool Deallocate(stackAllocation & outputAllocation)
				{
					if (m_marker <= 0)
						return false;
					
					stack_t currPos = m_movePosition;
					stack_t nextPos = m_allocations.back().m_bounds[m_side];

					stackAllocation toDeallocate = m_allocations.back();
					m_allocations.pop_back();
					m_marker = m_allocations.size();

					m_movePosition = nextPos;

					return true;
				}
			};

		protected:
			std::array<uint8_t, SIZE> m_buffer;

			stack_t m_alignment;
			stack_t m_stackBounds[2];
			std::array<stackSide, 2> m_stackMarker;

			/*
			||      m_stackMarker[0]	|	   Free Memory		|				m_stackMarker[1]				  ||
			--------------------------------------------------------------------------------------------------------
			||				|		|	|						|		|	|			|						  ||
			||		0		|	1	| 2	|.......................|	3	| 2	|	  1		|			  0			  ||
			||				|		|	|						|		|	|			|						  ||
			--------------------------------------------------------------------------------------------------------

			^ - m_stackBounds[0]																m_stackBounds[1] - ^
			*/
			
			template<typename T, typename... Args>
			gStackObject<T> Alloc(const eStackSide & side, Args&&... args)
			{
				stackAllocation newAllocation;
				newAllocation.m_type = typeid(T);
				if (!m_stackMarker[side].AllocateStackObject(sizeof(std::shared_ptr<T>), newAllocation))
					return gStackObject<T>();

				T * pObject = reinterpret_cast<T*>(newAllocation.m_bounds[eSSLeft]);
				*pObject = T(std::forward<Args>(args)...);

				gStackObject<T> newStackObject;
				newStackObject.m_pObject = std::shared_ptr<T>(pObject, [=](T * pObj)
				{
					delete pObj;
				});

				return newStackObject;
			};

			gStackObject<void> Alloc(const eStackSide & side, const uint32_t & size)
			{
				stackAllocation newAllocation;
				if (!m_stackMarker[side].AllocateStackObject(sizeof(T), newAllocation))
					return nullptr;

				gStackObject<void> newStackObject;
				newStackObject.m_pObject = reinterpret_cast<void*>(newAllocation.m_bounds[eSSLeft]);
				return newStackObject;
			}

			bool Deallocate(const eStackSide & side)
			{
				stackAllocation oldAllocation;
				if (!m_stackMarker[side].Deallocate(oldAllocation))
					return false;

				std::memset(reinterpret_cast<void*>(oldAllocation.m_bounds[eSSLeft]), 0, oldAllocation.m_size);
			}

			bool DeallocateToMarker(const eStackSide & side, const uint32_t & marker)
			{
				uint32_t numMarkers = m_stackMarker[side].m_marker;
				if (marker > numMarkers)
					return false;

				for (int i = 0; i < numMarkers - marker ; i++)
					Deallocate(side);
			}

		public:
			gStackAllocator()
			{
				m_buffer.assign(UINT8_MAX);
				stack_t dBuffer = reinterpret_cast<stack_t>(m_buffer.data());
				m_stackBounds[eSSLeft] = dBuffer;
				m_stackBounds[eSSRight] = dBuffer + m_buffer.size();

				m_stackMarker[eSSLeft] = stackSide(eSSLeft, m_stackBounds[eSSLeft], &m_stackMarker[eSSRight].m_holdPosition);
				m_stackMarker[eSSRight] = stackSide(eSSRight, m_stackBounds[eSSRight], &m_stackMarker[eSSLeft].m_holdPosition);
			};
			~gStackAllocator()
			{
			}

			gStackObject<void> AllocLower(const uint32_t & size) { return Alloc(eSSLeft, size); };
			gStackObject<void> AllocHigher(const uint32_t & size) { return Alloc(eSSRight, size); };

			template<typename T, typename... Args>
			gStackObject<T> AllocLower(Args&&... args) { return Alloc<T>(eSSLeft, std::forward<Args>(args)...); };

			template<typename T, typename... Args>
			gStackObject<T> AllocHigher(Args&&... args) { return Alloc<T>(eSSRight, std::forward<Args>(args)...); };


			template<typename T>
			void Dealloc(std::shared_ptr<T> & pObject)
			{

			}

			void DeallocLower() { Deallocate(eSSLeft); };
			void DeallocToMarkerLower(const uint32_t & marker) { DeallocateToMarker(eSSLeft, marker); };
			void DeallocHigher() { Deallocate(eSSRight); };
			void DeallocToMarkerHigher(const uint32_t & marker) { DeallocateToMarker(eSSRight, marker); };

			void DebugOutput()
			{
				uint32_t l = 0;
				uint32_t r = m_stackBounds[1] - m_stackBounds[0];
				double ratio = 100.0f / r;
				
				uint32_t lPos = (m_stackMarker[0].m_movePosition - m_stackBounds[0]) * ratio;
				uint32_t rPos = (m_stackMarker[1].m_movePosition - m_stackBounds[0]) * ratio;

				std::string header;
				std::string internalSides;
				std::string internalAlloc;
				for (int i = 0; i < 100; i++)
				{
					header += "-";
					if (i < (lPos - 1))		internalSides += "_";
					else if (i == lPos - 1)			internalSides += "L";
					else if (i >= (rPos + 1))	internalSides += "_";
					else if (i == rPos)			internalSides += "R";
					else						internalSides += "-";
				}

				int count = 0;
				for (auto aIter : m_stackMarker[0].m_allocations)
				{
					uint32_t lPosIter = (aIter.m_bounds[eSSLeft] - m_stackBounds[0]) * ratio;
					uint32_t rPosIter = (aIter.m_bounds[eSSRight] - m_stackBounds[0]) * ratio;
					uint32_t length = rPosIter - lPosIter;

					for (int i = 0; i < length; i++)
					{
						internalAlloc += std::to_string(count);
					}
					count++;
				}

				for (int i = 0; i < (rPos - lPos); i++)
				{
					internalAlloc += " ";
				}

				count = 0;
				for (auto aIter : m_stackMarker[1].m_allocations)
				{
					uint32_t lPosIter = (aIter.m_bounds[eSSLeft] - m_stackBounds[0]) * ratio;
					uint32_t rPosIter = (aIter.m_bounds[eSSRight] - m_stackBounds[0]) * ratio;
					uint32_t length = rPosIter - lPosIter;

					for (int i = 0; i < length; i++)
					{
						internalAlloc += std::to_string(count);
					}
					count++;
				}


				LOG(header);
				LOG(internalSides);
				LOG(internalAlloc);
				LOG(internalAlloc);
				LOG(internalAlloc);
				LOG(internalSides);
				LOG(header);
			}
		};

		class gFrameAllocator
		{
		public:
			struct Frame
			{
				static size_t g_size;
				std::array<uint8_t, MB(32)> m_buffer;
			};
		protected:
			gStackAllocator<(sizeof(Frame) * 2)> m_frameAllocation;
			std::array<gStackObject<Frame> , 2> m_frameBuffer;
			uint32_t m_currentFrame;

			gFrameAllocator(){};

		public:
			void Init()
			{
				m_frameBuffer[0] = m_frameAllocation.AllocLower<Frame>();
				m_frameBuffer[1] = m_frameAllocation.AllocLower<Frame>();

				LOG("Created Frame buffers");
				m_frameAllocation.DebugOutput();
			};

			void Swap()
			{
				++m_currentFrame %= m_frameBuffer.size();
			};
			void Set(decltype(Frame::m_buffer) & buffer)
			{
				//m_frameBuffer[m_currentFrame]->m_buffer = buffer;
			}
			void Clear()
			{
				//std::memset(&m_frameBuffer[m_currentFrame]->m_buffer[0], '\0', Frame::g_size);
			};
		};

		size_t gFrameAllocator::Frame::g_size = MB(32);

	}
};
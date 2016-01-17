
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
		template<uint64_t SIZE>
		class gStackAllocator
		{
			enum eStackSide
			{
				eSSLeft,
				eSSRight,

				eSSCount
			};
			typedef uint64_t stack_t;
			struct stackAllocation
			{
				stack_t m_bounds[eSSCount];
				stackAllocation(const eStackSide & side, const stack_t & position, const stack_t & size)
				{
					if (side == eSSLeft)
					{
						m_bounds[eSSLeft] = position;
						m_bounds[eSSRight] = position + size;
					}
					else
					{
						m_bounds[eSSLeft] = position - size;
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
				std::vector<stackAllocation> allocations;

				stackSide(){};
				stackSide(const eStackSide & side, const stack_t & staticLocation, stack_t * opPosition) : 
					m_side(side), m_holdPosition(staticLocation), m_movePosition(staticLocation), m_opPosition(opPosition)
				{
				};
				~stackSide() {};

				bool Alloc(const stack_t & size, stack_t & outputPosition)
				{
					stack_t currPos = m_movePosition;
					stack_t nextPos = (m_side == eSSLeft) ? (m_movePosition + size) : (m_movePosition - size);
					
					bool bInvalid =
						((m_side == eSSLeft) ? (currPos >= *m_opPosition) : (currPos <= *m_opPosition)) &&
						((m_side == eSSLeft) ? (nextPos > *m_opPosition) : (nextPos < *m_opPosition));
						
					if (bInvalid)
						return false;

					allocations.emplace_back(m_side, currPos, size);
					
					m_movePosition = nextPos;
					outputPosition = (m_side == eSSLeft) ? currPos : nextPos;

					return true;
				}
			};

		protected:
			std::array<uint8_t, SIZE> m_buffer;

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
			T * Alloc(const eStackSide & side, Args&&... args)
			{
				stack_t buffPos = 0;
				if (!m_stackMarker[side].Alloc(sizeof(T), buffPos))
					return nullptr;

				T * pObject = reinterpret_cast<T*>(buffPos);
				*pObject = T(std::forward<Args>(args)...);
				return pObject;
			};


			void * Alloc(const eStackSide & side, const uint32_t & size)
			{
				stack_t buffPos = 0;
				if (!m_stackMarker[side].Alloc(sizeof(T), buffPos))
					return nullptr;
				return reinterpret_cast<void*>(buffPos);
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

			void * AllocLower(const uint32_t & size) { return Alloc(eSSLeft, size); };
			void * AllocHigher(const uint32_t & size) { return Alloc(eSSRight, size); };

			template<typename T, typename... Args>
			T * AllocLower(Args&&... args) { return Alloc<T>(eSSLeft, std::forward<Args>(args)...); };

			template<typename T, typename... Args>
			T * AllocHigher(Args&&... args) { return Alloc<T>(eSSRight, std::forward<Args>(args)...); };
			

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
				for (auto aIter : m_stackMarker[0].allocations)
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
				for (auto aIter : m_stackMarker[1].allocations)
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
	}
};
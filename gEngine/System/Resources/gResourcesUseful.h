
#pragma once
#include "..\Objects\gObjectComponents.h"
#include "gFileHandling.h"

namespace GENG
{
	namespace FileHandling
	{
		class gResource : public Object::ComponentBase
		{
		public:
			virtual void LoadResource(const std::string & filePath) = 0;
		};

		class gResource_Buffer : public gResource
		{
		protected:
			std::vector<uint8_t> m_buffer;

		public:
			virtual void LoadResource(const std::string & filePath)
			{
				FileHandling::SGetFileBuffer(filePath, m_buffer, 0);
			}
		};
	};
};
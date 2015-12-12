
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
	unsigned int SGetFileSize(std::ifstream * pFile)
	{
		if (pFile == nullptr || !pFile->is_open())
		{
			DERROR("File-size could not be returned. Invalid file!");
			return 0;
		}
		pFile->seekg(0, std::ios::end);
		unsigned int size = static_cast<unsigned int>(pFile->tellg());
		pFile->seekg(0, std::ios::beg);
		return size;
	}

	std::string SGetFileString(const std::string & filename)
	{
		std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
		std::string rtn;
		if (in.is_open())
		{
			rtn.resize(SGetFileSize(&in));
			in.read(&rtn[0], rtn.size());
			in.close();
		}
		return rtn;
	}

	std::vector<char> SGetFileBuffer(const std::string & filename)
	{
		std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
		std::vector<char> rtn;
		if (in.is_open())
		{
			rtn.resize(SGetFileSize(&in));
			in.read(rtn.data(), rtn.size());
			in.close();
		}
		return rtn;
	}

	template<uint32_t BuffSize>
	std::array<char, BuffSize> SGetFileArray(const std::string & filename)
	{
		std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
		std::array<char, BuffSize> rtn; rtn.assign(CHAR_MAX);
		if (in.is_open())
		{
			auto size = SGetFileSize(&in);
			if (size > BuffSize)
				DERROR("Cannot read complete file, NumArrays specified is too small! Size(" << std::to_string(size) << ") > BuffSize(" << std::to_string(BuffSize) << ")");
			in.read(rtn.data(), size);
			in.close();
		}
		return rtn;
	}

	template<uint32_t BuffSize, uint8_t NumArray>
	std::array<std::array<char, BuffSize>, NumArray> SGetFileArrays(const std::string & filename)
	{
		std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
		std::array<std::array<char, BuffSize>, NumArray> rtn;
		for (auto buff : rtn) 
			buff.assign(CHAR_MAX);

		auto size = SGetFileSize(&in);
		if (in.is_open())
		{
			auto size = SGetFileSize(&in);
			if (size > (BuffSize * NumArray))
				DERROR("Cannot read complete file, NumArrays specified is too small! Size(" << std::to_string(size) << ") > BuffSize*NumArray(" << std::to_string(BuffSize * NumArray) << ")");

			float num = ceil(static_cast<float>(size) / static_cast<float>(BuffSize));

			for (auto buff : rtn)
			{
				if (in.eof())
					break;
				in.read(buff.data(), BuffSize);
			}
			in.close();
		}
		return rtn;
	}
};
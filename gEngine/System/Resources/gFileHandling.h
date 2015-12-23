
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

namespace std
{
	template<>
	struct hash<std::vector<char>>
	{
		typedef std::vector<char> argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const& s) const
		{
			return std::hash<std::string>() (std::string(s.cbegin(), s.cend()));
		};
	};
};

namespace GENG
{
	namespace FileHandling
	{
		static unsigned int SGetFileSize(std::ifstream * pFile)
		{
			if (pFile == nullptr || !pFile->is_open())
			{
				DERROR("File-size could not be returned. Invalid file!");
				return 0;
			}
			pFile->seekg(0, std::ios::end);
			unsigned int rtn = static_cast<unsigned int>(pFile->tellg());
			pFile->seekg(0, std::ios::beg);

			return rtn;
		};
		static unsigned int SGetFileSize(const std::string & filename)
		{
			std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
			unsigned int rtn = 0;
			if (file.is_open())
			{
				file.seekg(0, std::ios::end);
				rtn = static_cast<unsigned int>(file.tellg());
				file.close();
			}
			return rtn;
		};

		static bool SGetFileString(std::ifstream * pFile, std::string & rtn, const unsigned int & offset = 0)
		{
			if (pFile == nullptr || !pFile->is_open())
			{
				DERROR("String could not be returned. Invalid file!");
				return false;
			}

			rtn.resize(SGetFileSize(pFile) - offset);

			pFile->seekg(offset, std::ios::beg);
			pFile->read(&rtn[0], rtn.size());
			pFile->seekg(0, std::ios::beg);

			return true;
		}
		static bool SGetFileString(const std::string & filename, std::string & data, const unsigned int & offset = 0)
		{
			std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
			if (!file.is_open())
				return false;

			data.resize(SGetFileSize(&file) - offset);

			file.seekg(offset, std::ios::beg);
			file.read(&data[0], data.size());

			file.close();

			return true;
		}

		static bool SGetFileBuffer(std::ifstream * pFile, std::vector<char> & data, const unsigned int & offset = 0)
		{
			if (pFile == nullptr || !pFile->is_open())
			{
				DERROR("Buffer-Vector could not be returned. Invalid file!");
				return false;
			}

			data.resize(SGetFileSize(pFile) - offset);

			pFile->seekg(offset, std::ios::beg);
			pFile->read(data.data(), data.size());
			pFile->seekg(0, std::ios::beg);

			return true;
		}

		static bool SGetFileBuffer(const std::string & filename, std::vector<char> & data, const unsigned int & offset = 0)
		{
			std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
			if (!file.is_open())
				return false;

			data.resize(SGetFileSize(&file) - offset);

			file.seekg(offset, std::ios::beg);
			file.read(data.data(), data.size());

			file.close();

			return true;
		}

		template<uint32_t BuffSize>
		static bool SGetFileArray(std::ifstream * pFile, std::array<char, BuffSize> & data, const unsigned int & offset = 0)
		{
			if (pFile == nullptr || !pFile->is_open())
			{
				DERROR("Buffer-Array could not be returned. Invalid file!");
				return;
			}

			data.assign(CHAR_MAX);

			auto size = SGetFileSize(pFile) - offset;
			if (size > BuffSize)
			{
				DERROR("Cannot read complete file, BuffSize specified is too small! Size(" << std::to_string(size) << ") > BuffSize(" << std::to_string(BuffSize) << ")");
				return false;
			}

			pFile->seekg(offset, std::ios::beg);
			pFile->read(data.data(), size);
			pFile->seekg(0, std::ios::beg);

			return data;
		}

		template<uint32_t BuffSize>
		static bool SGetFileArray(const std::string & filename, std::array<char, BuffSize> & data, const unsigned int & offset = 0)
		{
			std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
			if (!file.is_open())
				return false;

			data.assign(CHAR_MAX);

			auto size = SGetFileSize(&file) - offset;
			if (size > BuffSize)
			{
				DERROR("Cannot read complete file, BuffSize specified is too small! Size(" << std::to_string(size) << ") > BuffSize(" << std::to_string(BuffSize) << ")");
				return false;
			}

			file.seekg(offset, std::ios::beg);
			file.read(data.data(), size);

			file.close();

			return true;
		}

		template<uint32_t BuffSize, uint8_t NumArray>
		static bool SGetFileArrays(std::ifstream * pFile, std::array<std::array<char, BuffSize>, NumArray> & data, const unsigned int & offset = 0)
		{
			if (pFile == nullptr || !pFile->is_open())
			{
				DERROR("Buffer-Arrays could not be returned. Invalid file!");
				return false;
			}

			for (auto buff : data)
				buff.assign(CHAR_MAX);

			auto size = SGetFileSize(pFile) - offset;
			if (size > (BuffSize * NumArray))
				DERROR("Cannot read complete file, NumArrays specified is too small! Size(" << std::to_string(size) << ") > BuffSize*NumArray(" << std::to_string(BuffSize * NumArray) << ")");

			float num = ceil(static_cast<float>(size) / static_cast<float>(BuffSize));
			pFile->seekg(offset, std::ios::beg);
			for (auto buff : data)
			{
				if (pFile->eof())
					break;
				pFile->read(buff.data(), BuffSize);
			}

			pFile->seekg(0, std::ios::beg);

			return true;
		};

		template<uint32_t BuffSize, uint8_t NumArray>
		static bool SGetFileArrays(const std::string & filename, std::array<std::array<char, BuffSize>, NumArray> & data, const unsigned int & offset = 0)
		{
			std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
			if (!file.is_open())
				return false;

			for (auto buff : data)
				buff.assign(CHAR_MAX);

			auto size = SGetFileSize(&file) - offset;
			if (size > (BuffSize * NumArray))
			{
				DERROR("Cannot read complete file, NumArrays specified is too small! Size(" << std::to_string(size) << ") > BuffSize*NumArray(" << std::to_string(BuffSize * NumArray) << ")");
				return false;
			}

			float num = ceil(static_cast<float>(size) / static_cast<float>(BuffSize));
			file.seekg(offset, std::ios::beg);
			for (auto buff : data)
			{
				if (file.eof())
					break;
				file.read(buff.data(), BuffSize);
			}

			file.close();

			return true;
		};

		template<unsigned int NumThreads = 1, typename bufferType = std::vector<char>>
		static bool SLoadMultithreaded(const std::string & m_fileName, bufferType & data, unsigned int chunkSize = 0)
		{
			struct threadData
			{
				std::ifstream file;
				unsigned int offset = 0;
				unsigned int chunkSize = 0;
				unsigned int threadSize = 0;
				bufferType * pBuffer;
			};

			std::array<threadData, NumThreads> multiThreadData;
			GENG::Threads::gProcessThreadPool pool;
			pool.Init(NumThreads);

			std::ifstream file = std::ifstream(m_fileName.c_str(), std::ios::in | std::ios::binary);

			if (!file.is_open())
				return false;

			const unsigned int fileSize = GENG::FileHandling::SGetFileSize(&file);

			auto sizePerThread = static_cast<unsigned int>(std::ceil(static_cast<float>(fileSize) / static_cast<float>(NumThreads)));

			data.clear();
			data.resize(fileSize);

			std::vector<GENG::Threads::tyFuncPair> pairs;
			for (int i = 0; i < NumThreads; i++)
			{
				multiThreadData[i].file = std::ifstream(m_fileName.c_str(), std::ios::in | std::ios::binary);
				multiThreadData[i].offset = sizePerThread * i;
				multiThreadData[i].pBuffer = &data;
				if (chunkSize > 1)
				{
					auto cSize = std::min(chunkSize, sizePerThread);
					multiThreadData[i].chunkSize = cSize;
					multiThreadData[i].threadSize = sizePerThread;
				}
				else
				{
					multiThreadData[i].chunkSize = sizePerThread;
					multiThreadData[i].threadSize = sizePerThread;
				}

				auto loaderTask = [](void * data) -> void
				{
					threadData * pData = reinterpret_cast<threadData*>(data);
					if ((pData != nullptr) && (pData->file.is_open()) && (pData->pBuffer != nullptr))
					{
						pData->file.seekg(pData->offset, std::ifstream::beg);

						auto pos = &(*pData->pBuffer)[0] + pData->offset;
						auto cSize = pData->chunkSize;

						while (pData->threadSize > 0)
						{
							if (pData->threadSize < cSize)
								cSize = pData->threadSize;
							pData->file.read(pos, cSize);
							pData->threadSize -= cSize;
							pos += cSize;
						}

						pData->file.close();
					}
				};

				GENG::Threads::tyFuncPair pair;
				pair.first = loaderTask;
				pair.second = &multiThreadData[i];
				pairs.push_back(pair);
			}
			pool.AddTask(m_fileName, pairs);

			auto timeTaken = pool.WaitToFinish();
			
			LOG("Time to load \'" << m_fileName << "\' : " << timeTaken.count() << " milliseconds "
				<< "NumThreads(" << NumThreads << ") ChunkSize(" << chunkSize << ")");

#ifdef _CHECK_HASH_
			const unsigned numMB = 500;
			if (data.size() > MB(numMB))
			{
				std::vector<char> first(data.begin(), data.begin() + MB(numMB / 2));
				std::vector<char> last(data.end() - MB(numMB / 2), data.end());
				LOG("Hash(Split)(" << std::hash<bufferType>{}(first) << "|" << std::hash<bufferType>{}(last) << ")");
			}
			else
			{
				LOG("Hash(" << std::hash<bufferType>{}(data) << ")");
			}
#endif // _CHECK_HASH_

			file.close();

			return true;
		};
	}
};
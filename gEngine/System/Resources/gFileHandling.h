
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

	template<>
	struct hash<std::vector<uint8_t>>
	{
		typedef std::vector<uint8_t> argument_type;
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
		template<typename T = char>
		static uint32_t SGetFileSize(std::basic_ifstream<T> * pFile)
		{
			if (pFile == nullptr || !pFile->is_open())
			{
				DERROR("File-size could not be returned. Invalid file!");
				return 0;
			}
			pFile->seekg(0, std::ios::end);
			uint32_t rtn = static_cast<uint32_t>(pFile->tellg());
			pFile->seekg(0, std::ios::beg);

			return rtn;
		};
		
		template<typename T = char>
		static uint32_t SGetFileSize(const std::string & filename)
		{
			std::basic_ifstream<T> file(filename.c_str(), std::ios::in | std::ios::binary);
			uint32_t rtn = 0;
			if (file.is_open())
			{
				file.seekg(0, std::ios::end);
				rtn = static_cast<uint32_t>(file.tellg());
				file.close();
			}
			return rtn;
		};

		template<typename T = char>
		static bool SGetFileString(std::basic_ifstream<T> * pFile, std::string & rtn, const uint32_t & offset = 0)
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
		
		template<typename T = char>
		static bool SGetFileString(const std::string & filename, std::string & data, const uint32_t & offset = 0)
		{
			std::basic_ifstream<T> file(filename.c_str(), std::ios::in | std::ios::binary);
			if (!file.is_open())
				return false;

			data.resize(SGetFileSize(&file) - offset);

			file.seekg(offset, std::ios::beg);
			file.read(&data[0], data.size());

			file.close();

			return true;
		}
		
		template<typename T = char>
		static bool SGetFileBuffer(std::basic_ifstream<T> * pFile, std::vector<T> & data, const uint32_t & offset = 0)
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

		template<typename T = char>
		static bool SGetFileBuffer(const std::string & filename, std::vector<T> & data, const uint32_t & offset = 0)
		{
			std::basic_ifstream<T> file(filename.c_str(), std::ios::in | std::ios::binary);
			if (!file.is_open())
				return false;

			data.resize(SGetFileSize(&file) - offset);

			file.seekg(offset, std::ios::beg);
			file.read(data.data(), data.size());

			file.close();

			return true;
		}

		template<uint32_t BuffSize, typename T = uint8_t>
		static bool SGetFileArray(std::basic_ifstream<T> * pFile, std::array<T, BuffSize> & data, const uint32_t & offset = 0)
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

		template<uint32_t BuffSize, typename T = uint8_t>
		static bool SGetFileArray(const std::string & filename, std::array<T, BuffSize> & data, const uint32_t & offset = 0)
		{
			std::basic_ifstream<T> file(filename.c_str(), std::ios::in | std::ios::binary);
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

		template<uint32_t BuffSize, uint8_t NumArray, typename T = uint8_t>
		static bool SGetFileArrays(std::basic_ifstream<T> * pFile, std::array<std::array<T, BuffSize>, NumArray> & data, const uint32_t & offset = 0)
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

		template<uint32_t BuffSize, uint8_t NumArray, typename T = uint8_t>
		static bool SGetFileArrays(const std::string & filename, std::array<std::array<T, BuffSize>, NumArray> & data, const uint32_t & offset = 0)
		{
			std::basic_ifstream<T> file(filename.c_str(), std::ios::in | std::ios::binary);
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

		template<typename T = uint8_t>
		static bool SLoadMultithreaded(const std::string & m_fileName, std::vector<T> & data, const uint32_t & numThreads = 1, const uint32_t & chunkSize = 0)
		{
			using bufferType = std::vector<T>;
			struct threadData
			{
				std::basic_ifstream<T> file;
				uint32_t offset = 0;
				uint32_t chunkSize = 0;
				uint32_t threadSize = 0;
				bufferType * pBuffer;
			};

			std::vector<threadData> multiThreadData(numThreads);
			GENG::Threads::gProcessThreadPool pool;
			pool.Init(numThreads);

			std::basic_ifstream<T> file(m_fileName.c_str(), std::ios::in | std::ios::binary);

			if (!file.is_open())
				return false;

			const uint32_t fileSize = GENG::FileHandling::SGetFileSize(&file);

			auto sizePerThread = static_cast<uint32_t>(std::ceil(static_cast<float>(fileSize) / static_cast<float>(numThreads)));

			data.clear();
			data.resize(fileSize);

			auto loaderTask = [](void * data) -> void
			{
				threadData * pData = reinterpret_cast<threadData*>(data);
				if ((pData != nullptr) && (pData->file.is_open()) && (pData->pBuffer != nullptr))
				{
					pData->file.seekg(pData->offset, std::basic_ifstream<T>::beg);

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

			std::vector<GENG::Threads::tyFuncPair> pairs;
			for (int i = 0; i < numThreads; i++)
			{
				multiThreadData[i].file = std::basic_ifstream<T>(m_fileName.c_str(), std::ios::in | std::ios::binary);
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

				GENG::Threads::tyFuncPair pair;
				pair.first = loaderTask;
				pair.second = &multiThreadData[i];
				pairs.push_back(pair);
			}
			pool.AddTask(m_fileName, pairs);

			auto timeTaken = pool.WaitToFinish();
			
			LOG("Time to load \'" << m_fileName << "\' : " << timeTaken.count() << " milliseconds "
				<< "NumThreads(" << numThreads << ") ChunkSize(" << chunkSize << ")");

#ifdef _CHECK_HASH_
			const unsigned numMB = 500;
			if (data.size() > MB(numMB))
			{
				std::vector<T> first(data.begin(), data.begin() + MB(numMB / 2));
				std::vector<T> last(data.end() - MB(numMB / 2), data.end());
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

		namespace Compression
		{			
			template<uint32_t TypeSet = 1, typename T = uint8_t>
			static bool SEncode_RunLengthEncoding(const T * inputBuffer, const size_t & inputSize, std::vector<T> & outputBuffer)
			{
				if (inputSize <= 0)
					return false;

				struct RunSet
				{
					T * set[TypeSet];
					bool operator==(const RunSet & rhs)
					{
						for (int i = 0; i < TypeSet; i++)
						{
							if ((*set[i]) != (*rhs.set[i]))
								return false;
						}
						return true;
					}
				};
				using bufferType = std::vector<T>;
				T runCount = 0;
				RunSet runValueA;
				RunSet runValueB;

				outputBuffer.clear();
				outputBuffer.reserve(inputSize / 2);

				auto readValue = [&inputBuffer](size_t & position, RunSet & value)
				{
					for (int i = 0; i < TypeSet; i++)
						value.set[i] = const_cast<T*>(&inputBuffer[position++]);
				};
				auto writeValue = [&outputBuffer, &runCount, &runValueA, &runValueB]()
				{
					outputBuffer.emplace_back(runCount + 1);

					for (int i = 0; i < TypeSet; i++)
						outputBuffer.emplace_back((*runValueA.set[i]));

					runValueA = runValueB;
					runCount = 0;
				};

				size_t runIter = 0;

				readValue(runIter, runValueA);

				bool bEOF = false;
				while (!bEOF)
				{
					readValue(runIter, runValueB);
					
					if (runValueA == runValueB)
					{
						if (runCount <= (std::numeric_limits<T>::max() - 1))
							runCount++;
						else
							writeValue();
					}
					else
						writeValue();
				
					bEOF = (runIter == inputSize);
				}

				writeValue();

				return true;
			}

			template<uint32_t TypeSet = 1, typename T = uint8_t>
			static bool SDecode_RunLengthEncoding(const T * inputBuffer, const size_t & inputSize, std::vector<T> & outputBuffer)
			{
				if (inputSize < 2)
					return false;

				using bufferType = std::vector<T>;

				outputBuffer.clear();
				outputBuffer.reserve(inputSize * 2);

				uint32_t i = 0;
				while (i < inputSize)
				{
					const T * runCount = &inputBuffer[i++];
					T * set[TypeSet];
					for (int s = 0; s < TypeSet; s++)
					{
						if (i >= inputSize)
							break;
						set[s] = const_cast<T*>(&inputBuffer[i++]);
					}

					for (T c = 0; c < (*runCount); c++)
					{
						for (int s = 0; s < TypeSet; s++)
							outputBuffer.emplace_back((*set[s]));
					}
				}

				return outputBuffer.size() > 2;
			}

			template<uint32_t TypeSet = 1, typename T = uint8_t>
			static bool SEncode_RunLengthEncoding(const std::vector<T> & inputBuffer, std::vector<T> & outputBuffer)
			{
				return SEncode_RunLengthEncoding<TypeSet, T>(inputBuffer.data(), inputBuffer.size(), outputBuffer);
			}

			template<uint32_t TypeSet = 1, typename T = uint8_t>
			static bool SDecode_RunLengthEncoding(const std::vector<T> & inputBuffer, std::vector<T> & outputBuffer)
			{
				return SDecode_RunLengthEncoding<TypeSet, T>(inputBuffer.data(), inputBuffer.size(), outputBuffer);
			}

			template<uint32_t TypeSet = 1, typename T = uint8_t>
			static bool SEncode_RunLengthEncodingMultiThreaded(const std::vector<T> & inputBuffer, std::vector<T> & outputBuffer, const uint32_t numThreads = 1)
			{
				if (inputBuffer.size() < 2)
					return false;

				using bufferType = std::vector<T>;
				struct threadData
				{
					T * pBuffer = nullptr;
					uint32_t bufferSize = 0;
					bufferType outputBuffer;
				};

				std::vector<threadData> multiThreadData(numThreads);
				GENG::Threads::gProcessThreadPool pool;
				pool.Init(numThreads);
				
				const size_t fileSize = inputBuffer.size();

				auto sizePerThread = static_cast<uint32_t>(std::ceil(static_cast<float>(fileSize) / static_cast<float>(numThreads)));

				outputBuffer.clear();
				outputBuffer.reserve(fileSize / 2);
				
				auto loaderTask = [](void * data) -> void
				{
					threadData * pData = reinterpret_cast<threadData*>(data);

					if ((pData != nullptr) && (pData->pBuffer != nullptr))
					{
						SEncode_RunLengthEncoding<TypeSet, T>(pData->pBuffer, pData->bufferSize, pData->outputBuffer);
					}
				};

				std::vector<GENG::Threads::tyFuncPair> pairs;
				for (int i = 0; i < numThreads; i++)
				{
					uint32_t offset = sizePerThread * i;
					T * pData = const_cast<T*>(&inputBuffer[0]) + offset;
					multiThreadData[i].pBuffer = pData;
					multiThreadData[i].bufferSize = sizePerThread;

					GENG::Threads::tyFuncPair pair;
					pair.first = loaderTask;
					pair.second = &multiThreadData[i];
					pairs.push_back(pair);
				}
				pool.AddTask("", pairs);

				auto timeTaken = pool.WaitToFinish();

				for (int i = 0; i < numThreads; i++)
				{
					outputBuffer.insert(outputBuffer.end(), multiThreadData[i].outputBuffer.begin(), multiThreadData[i].outputBuffer.end());
				}

				return true;
			}
		}
	}
};

#pragma once

#include <array>
#include <memory>
#include <vector>
#include <typeindex>
#include "gEngine\Useful.h"

namespace GENG
{
	namespace Scripting
	{
		enum AddVarType
		{
			adtVariable,
			adtConstVariable,
			adtGlobal,
			adtConstGlobal
		};
		
		struct ScriptModule
		{
			std::string m_id;
			chaiscript::ModulePtr m_module;

			ScriptModule(const std::string & id, chaiscript::ModulePtr module) : m_module(module), m_id(id) {};
			~ScriptModule() {};
		};

		class ScriptState
		{
			typedef chaiscript::ChaiScript _scriptState;

			std::unique_ptr<_scriptState> m_state;
			std::vector<std::type_index> m_registeredClasses;
			std::vector<std::type_index> m_registeredTypes;
			std::vector<std::string> m_registeredFunctions;
			std::vector<std::string> m_registeredVariables;
			std::vector<std::string> m_registeredModules;
			
			std::string m_filepath;
			std::string m_file;

			bool m_bReload = false;
		public:
			ScriptState();
			~ScriptState();
			_scriptState * Get();

			void SetReload(bool reload) { m_bReload = reload; }
			std::string GetFilepath() const { return m_filepath; }

			void LoadFile(const std::string filepath);
			void EvaluateFile();

			template<typename T>
			void RegisterClass()
			{
				std::type_index typeIndex = std::type_index(typeid(T));
				//if (std::find(m_registeredClasses.begin(), m_registeredClasses.end(), typeIndex) == m_registeredClasses.end())
				{
					chaiscript::ModulePtr pModule = std::make_shared<chaiscript::Module>();
					T::ScriptRegisterClass(pModule);
					m_state->add(pModule);
					m_registeredClasses.push_back(typeIndex);
				}
			}

			template<typename T>
			void RegisterFunction(const std::string & funcName, std::function<T> func)
			{
				//if (std::find(m_registeredFunctions.begin(), m_registeredFunctions.end(), funcName) == m_registeredFunctions.end())
				{
					m_state->add(chaiscript::fun(func), funcName);
					m_registeredFunctions.push_back(funcName);
				}
			}

			template<typename T>
			void RegisterValue(const std::string & funcName, T & value)
			{
				//if (std::find(m_registeredVariables.begin(), m_registeredVariables.end(), funcName) == m_registeredVariables.end())
				{
					std::function<T(const T &)> func = [&value](const T & v) -> T { value = v; return value; };
					m_state->add(chaiscript::fun(func), funcName);
					m_registeredVariables.push_back(funcName);
				}
			};

			template<typename T>
			inline void RegisterTypeInfo(const std::string & typeName)
			{
				std::type_index typeIndex = std::type_index(typeid(T));
				if (std::find(m_registeredTypes.begin(), m_registeredTypes.end(), typeIndex) == m_registeredTypes.end())
				{
					m_state->add(chaiscript::user_type<T>(typeName));
				}
			};

			inline void RegisterModule(const ScriptModule & module)
			{
				//if (std::find(m_registeredModules.begin(), m_registeredModules.end(), module.m_id) == m_registeredModules.end())
				{
					m_state->add(module.m_module);
				}
			};

			template<typename T>
			inline void AddVariable(const std::string & varName, T & value, const AddVarType & type = adtVariable)
			{
				switch (type)
				{
				case adtVariable:
					m_state->add(chaiscript::var(value), varName);
					break;
				case adtConstVariable:
					m_state->add(chaiscript::const_var(value), varName);
					break;
				case adtGlobal:
					m_state->add_global(chaiscript::var(value), varName);
					break;
				case adtConstGlobal:
					m_state->add_global_const(chaiscript::const_var(value), varName);
					break;
				default:
					break;
				}
			};

			void DisplayDebugData();
		};
	};
};
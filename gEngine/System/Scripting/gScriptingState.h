
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
		
		using _ChaiConstructor = chaiscript::Proxy_Function;
		using _ChaiFunction = std::pair<chaiscript::Proxy_Function, std::string>;

		struct ScriptModule
		{
			std::string m_id;
			chaiscript::ModulePtr m_module;
			ScriptModule(const std::string & id, chaiscript::ModulePtr module) : m_module(module), m_id(id) {};
			~ScriptModule() {};
		};

		struct ScriptState_Values
		{
			typedef chaiscript::ChaiScript _scriptState;

			std::vector<std::type_index> m_classes;
			std::vector<std::type_index> m_types;
			std::vector<std::string> m_functions;
			std::vector<std::string> m_variables;
			std::vector<std::string> m_modules;

			std::unique_ptr<_scriptState> m_state;
		};
		class ScriptState
		{
			typedef chaiscript::ChaiScript _scriptState;

			static ScriptState_Values g_staticRegister;
			ScriptState_Values m_register;
						
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

					m_register.m_state->add(pModule);
					m_register.m_classes.push_back(typeIndex);
				}
			}

			template<typename T>
			void RegisterFunction(const std::string & funcName, std::function<T> func)
			{
				//if (std::find(m_registeredFunctions.begin(), m_registeredFunctions.end(), funcName) == m_registeredFunctions.end())
				{
					m_register.m_state->add(chaiscript::fun(func), funcName);
					m_register.m_functions.push_back(funcName);
				}
			}

			template<typename T>
			void RegisterValue(const std::string & funcName, T & value)
			{
				//if (std::find(m_registeredVariables.begin(), m_registeredVariables.end(), funcName) == m_registeredVariables.end())
				{
					std::function<T(const T &)> func = [&value](const T & v) -> T { value = v; return value; };

					m_register.m_state->add(chaiscript::fun(func), funcName);
					m_register.m_variables.push_back(funcName);
				}
			};

			template<typename T>
			inline void RegisterTypeInfo(const std::string & typeName)
			{
				std::type_index typeIndex = std::type_index(typeid(T));
				if (std::find(m_registeredTypes.begin(), m_registeredTypes.end(), typeIndex) == m_registeredTypes.end())
				{
					m_register.m_state->add(chaiscript::user_type<T>(typeName));
					m_register.m_types.push_back(typeName);
				}
			};

			inline void RegisterModule(const ScriptModule & module)
			{
				//if (std::find(m_registeredModules.begin(), m_registeredModules.end(), module.m_id) == m_registeredModules.end())
				{
					m_register.m_state->add(module.m_module);
					m_register.m_modules.push_back(module.m_id);
				}
			};

			template<typename T>
			inline void AddVariable(const std::string & varName, T & value, const AddVarType & type = adtVariable)
			{
				switch (type)
				{
				case adtVariable:
					m_register.m_state->add(chaiscript::var(value), varName);
					break;
				case adtConstVariable:
					m_register.m_state->add(chaiscript::const_var(value), varName);
					break;
				case adtGlobal:
					m_register.m_state->add_global(chaiscript::var(value), varName);
					break;
				case adtConstGlobal:
					m_register.m_state->add_global_const(chaiscript::const_var(value), varName);
					break;
				default:
					break;
				}
			};

			void DisplayDebugData();

			static void ScriptRegisterClass(chaiscript::ModulePtr pModule)
			{
				chaiscript::utility::add_class<ScriptState>(*pModule, "ScriptState",
					// Constructors
					{
						chaiscript::constructor<ScriptState()>(),
					},
					// Functions
					{
						{ chaiscript::fun(&ScriptState::LoadFile), "LoadFile" },
						{ chaiscript::fun(&ScriptState::EvaluateFile), "EvaluateFile" },
					}
				);
			};

			static ScriptState_Values & SGetStaticScriptStateRegisters()
			{
				if (g_staticRegister.m_state == nullptr)
					g_staticRegister.m_state = std::make_unique<_scriptState>(chaiscript::Std_Lib::library());
				return ScriptState::g_staticRegister;
			}
		};
	};

#define STRING(s) #s		
//#define CHAI_REG_CONSTRUCTOR(C, ...) const int y##Constructor = C##_Register::AddConstructors<##__VA_ARGS__>();
//#define CHAI_REG_VAR(C, y) const int y##Register = C##_Register::AddVar(&C::y, STRING(y));
//#define CHAI_REG_STRUCT(C) \
//	struct C; \
//	struct C##_Register \
//	{ \
//		static std::vector<chaiscript::Proxy_Function> _CONSRTUCTS; \
//		static std::vector<std::pair<chaiscript::Proxy_Function, std::string>> _VARS; \
//		template<typename T> \
//		static int AddConstructors() { _CONSRTUCTS.push_back( chaiscript::constructor<T>() ); return 0; }; \
//		template<typename T> \
//		static int AddVar(T m, const std::string & name) { _VARS.push_back( { chaiscript::fun(m), name } ); return 0; }; \
//		static int RegisterToStatic() \
//		{ \
//			chaiscript::ModulePtr pModule = std::make_shared<chaiscript::Module>(); \
//			chaiscript::utility::add_class<C>(*pModule, "name", _CONSRTUCTS, _VARS); \
//			\
//			DBG("Added object"); \
//			\
//			GENG::Scripting::ScriptModule module("name", pModule); \
//			auto & state = GENG::Scripting::ScriptState::SGetStaticScriptStateRegisters(); \
//			\
//			state.m_state->add(module.m_module); \
//			state.m_modules.push_back(module.m_id); \
//			\
//			return 0; \
//		}; \
//	} \
//	static C##; \
//	__declspec(selectany) std::vector<chaiscript::Proxy_Function> C##_Register::_CONSRTUCTS; \
//	__declspec(selectany) std::vector<std::pair<chaiscript::Proxy_Function, std::string>> C##_Register::_VARS;
//
//#define CHAI_REG_END(C) const int y##Close = C##_Register::RegisterToStatic();
//
//struct _ChaiRegister
//{
//	static std::vector<chaiscript::Proxy_Function> _CONSRTUCTS;
//	static std::vector<std::pair<chaiscript::Proxy_Function, std::string>> _VARS;
//
//	template<typename C, typename T>
//	static int AddConstructors() 
//	{ 
//		DBG("Added constructor");
//		_CONSRTUCTS.push_back(chaiscript::constructor<T>()); 
//		return 0; 
//	};
//
//	template<typename C, typename T>
//	static int AddVar(T m, const std::string & name) 
//	{
//		DBG("Added function");
//		//chaiscript::fun(m);
//		//std::pair<chaiscript::Proxy_Function, std::string> pair;
//		//pair.first = chaiscript::fun(m);
//		//pair.second = name;
//		//_VARS.push_back( pair ); 
//		return 0; 
//	};
//
//	template<typename C>
//	static int RegisterToStatic()
//	{
//		chaiscript::ModulePtr pModule = std::make_shared<chaiscript::Module>();
//		chaiscript::utility::add_class<C>(*pModule, "name", _CONSRTUCTS, _VARS);
//
//		DBG("Added object");
//
//		GENG::Scripting::ScriptModule module("name", pModule);
//		auto & state = GENG::Scripting::ScriptState::SGetStaticScriptStateRegisters();
//
//		state.m_state->add(module.m_module);
//		state.m_modules.push_back(module.m_id);
//
//		return 0;
//	};
//};
//
//template<typename C>
//__declspec(selectany) std::vector<std::pair<chaiscript::Proxy_Function, std::string>> GENG::_ChaiRegister<C>::_VARS;
//
//template<typename C> 
//__declspec(selectany) std::vector<chaiscript::Proxy_Function> GENG::_ChaiRegister<C>::_CONSRTUCTS;
//
//#define CHAI_REG_STRUCT2(C) : public _ChaiRegister<C>
//#define CHAI_REG_CONSTRUCTOR2(...) const int constructor = AddConstructors<##__VA_ARGS__>();
//#define CHAI_REG_VAR2(C, y) const int y##Register = AddVar(&C::y, STRING(y));
//#define CHAI_REG_END2() const int y##Close = RegisterToStatic();
};

#include "gScriptingState.h"
#include "gScriptingRegister.h"

GENG::Scripting::ScriptState_Values GENG::Scripting::ScriptState::g_staticRegister;

GENG::Scripting::ScriptState::ScriptState()
{
	m_register.m_classes = g_staticRegister.m_classes;
	m_register.m_types = g_staticRegister.m_types;
	m_register.m_functions = g_staticRegister.m_functions;
	m_register.m_variables = g_staticRegister.m_variables;
	m_register.m_modules = g_staticRegister.m_modules;
	
	m_register.m_state = std::make_unique<_scriptState>(chaiscript::Std_Lib::library());
	//m_register.m_state = std::make_unique<_scriptState>(*g_staticRegister.m_state.get());

	RegisterAll(this);
	DisplayDebugData();
}

GENG::Scripting::ScriptState::~ScriptState()
{

}

void GENG::Scripting::ScriptState::LoadFile(const std::string filepath)
{
	m_filepath = filepath;
	GENG::FileHandling::SGetFileString(filepath, m_file);
	LOG("Loaded : " << m_filepath);
}

void GENG::Scripting::ScriptState::EvaluateFile()
{
	if (m_bReload)
	{
		LoadFile(m_filepath);
		m_bReload = false;
	}
	try
	{
		auto locals = m_register.m_state->get_locals();
		m_register.m_state->eval(m_file);
		m_register.m_state->set_locals(locals);
	}
	catch (std::exception e)
	{
		LOG("Chaiscript exception: " << std::string(e.what()));
		LoadFile(m_filepath);
	}
}

void GENG::Scripting::ScriptState::DisplayDebugData()
{
	DBG("Registered classes:");
	for (auto reg : m_register.m_classes)
	{
		DBG(reg.name());
	}
	DBG("Registered types:");
	for (auto reg : m_register.m_types)
	{
		DBG(reg.name());
	}
	DBG("Registered functions:");
	for (auto reg : m_register.m_functions)
	{
		DBG(reg);
	}
	DBG("Registered variables:");
	for (auto reg : m_register.m_variables)
	{
		DBG(reg);
	}
	DBG("Registered modules:");
	for (auto reg : m_register.m_modules)
	{
		DBG(reg);
	}
}


#include "gScriptingState.h"
#include "gScriptingRegister.h"

GENG::Scripting::ScriptState::ScriptState()
{
	m_state = std::make_unique<_scriptState>(chaiscript::Std_Lib::library());

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
		auto locals = m_state->get_locals();
		m_state->eval(m_file);
		m_state->set_locals(locals);
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
	for (auto reg : m_registeredClasses)
	{
		DBG(reg.name());
	}
	DBG("Registered types:");
	for (auto reg : m_registeredTypes)
	{
		DBG(reg.name());
	}
	DBG("Registered functions:");
	for (auto reg : m_registeredFunctions)
	{
		DBG(reg);
	}
	DBG("Registered variables:");
	for (auto reg : m_registeredVariables)
	{
		DBG(reg);
	}
	DBG("Registered modules:");
	for (auto reg : m_registeredModules)
	{
		DBG(reg);
	}
}

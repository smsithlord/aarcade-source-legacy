#include "cbase.h"
#include "c_windowinstance.h"
//#include "aa_globals.h"
#include "c_anarchymanager.h"
//#include "c_embeddedinstance.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_WindowInstance::C_WindowInstance()
{
	DevMsg("WindowInstance: Constructor\n");
	m_title = "";
	m_id = "";
	m_hwnd = null;
	m_bHidden = false;
	m_bPresetHidden = false;
}

C_WindowInstance::~C_WindowInstance()
{
	DevMsg("WindowInstance: Destructor\n");
}


void C_WindowInstance::SelfDestruct()
{
	DevMsg("WindowInstance: SelfDestruct %s\n", m_id.c_str());
	delete this;
}

void C_WindowInstance::Close()
{
	g_pAnarchyManager->GetWindowManager()->DestroyWindowInstance(this);
}

void C_WindowInstance::Init(std::string id, HWND hwnd, std::string title, std::string className)
{
	std::string goodTitle = (title != "") ? title : "Untitled Window Instance";
	m_title = title;
	m_className = className;
	m_id = id;

	if (m_id == "")
		m_id = g_pAnarchyManager->GenerateUniqueId();

	m_hwnd = hwnd;

	// automatically hide instances listed in the "autoHideWindows.txt" file
	m_bHidden = g_pAnarchyManager->GetWindowManager()->IsPresetHiddenWindow(m_className, m_title);
	m_bPresetHidden = m_bHidden;

	g_pAnarchyManager->GetWindowManager()->AddInstance(this);
}
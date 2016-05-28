#include "cbase.h"

#include "c_webtab.h"
#include "c_anarchymanager.h"
//#include "aa_globals.h"
//#include "Filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_WebTab::C_WebTab(std::string url)
{
	DevMsg("WebTab: Constructor\n");
	m_iState = 0;

	m_id = g_pAnarchyManager->GenerateUniqueId();
	m_initialUrl = url;

	// initialize
	m_iState = 1;

	g_pAnarchyManager->GetWebManager()->GetWebBrowser()->CreateWebView(this);
}

C_WebTab::~C_WebTab()
{
	DevMsg("WebTab: Destructor\n");
}
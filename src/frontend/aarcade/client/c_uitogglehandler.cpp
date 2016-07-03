#include "cbase.h"
#include "aa_globals.h"
#include "c_uitogglehandler.h"
#include "c_anarchymanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_UIToggleHandler g_UIToggleHandler;
extern C_UIToggleHandler* g_pUIToggleHandler(&g_UIToggleHandler);

C_UIToggleHandler::C_UIToggleHandler()
{
	DevMsg("UIToggleHandler: Constructor\n");
}

C_UIToggleHandler::~C_UIToggleHandler()
{
	DevMsg("UIToggleHandler: Destructor\n");
}

bool C_UIToggleHandler::HandleUiToggle()
{
	return g_pAnarchyManager->HandleUiToggle();
}
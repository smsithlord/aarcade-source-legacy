#include "cbase.h"

#include "aa_globals.h"
#include "c_inputlistener.h"
#include "c_anarchymanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_InputListener::C_InputListener()
{
	DevMsg("InputListener: Constructor\n");
}

C_InputListener::~C_InputListener()
{
	DevMsg("InputListener: Destructor\n");
}
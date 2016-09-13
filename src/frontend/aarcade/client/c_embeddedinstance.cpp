#include "cbase.h"

//#include "aa_globals.h"
#include "c_embeddedinstance.h"
//#include "c_anarchymanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_EmbeddedInstance::C_EmbeddedInstance()
{
	DevMsg("EmbeddedInstance: Constructor\n");
//	m_iLastRenderedFrame = 0;
}

C_EmbeddedInstance::~C_EmbeddedInstance()
{
	DevMsg("EmbeddedInstance: Destructor\n");
}
#include "cbase.h"
#include "c_systemtime.h"

#include "vgui/ISystem.h"
#include "vgui_controls/Controls.h"


//#include <vgui/IInput.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_SystemTime::C_SystemTime()
{
	//DevMsg("SystemTime: Constructor\n");
}

C_SystemTime::~C_SystemTime()
{
	//DevMsg("MetaverseManager: Destructor\n");
}

long C_SystemTime::GetTime()
{
	DevMsg("Time: %i\n", vgui::system()->GetTimeMillis());
	//system()->GetTimeMillis();
	return 420L;
}
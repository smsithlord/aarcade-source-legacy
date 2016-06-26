#include "cbase.h"

#include "c_prop_shortcut.h"
//#include "vgui/ISystem.h"
//#include "boost/regex.h"
//#include "baseviewport.h"
//#include "ienginevgui.h"
//#include "inetchannelinfo.h"
//#include "ICreateHotlink.h"
//#include "ienginevgui.h"
//#include <cassert>
//#include <string>

#include <KeyValues.h>
#include "Filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_PropShortcutEntity, DT_PropShortcutEntity, CPropShortcutEntity)
	RecvPropString(RECVINFO(m_itemId))
END_RECV_TABLE()

C_PropShortcutEntity::C_PropShortcutEntity()
{
}

C_PropShortcutEntity::~C_PropShortcutEntity()
{
}

void C_PropShortcutEntity::OnPreDataChanged(DataUpdateType_t updateType)
{
	//DevMsg("PreDataChangedUpdate: ")
	BaseClass::OnPreDataChanged(updateType);
}

void C_PropShortcutEntity::Release()
{
	BaseClass::Release();
}

void C_PropShortcutEntity::Precache(void)
{
	PrecacheModel("models\\cabinets\\two_player_arcade.mdl");
	BaseClass::Precache();
}

void C_PropShortcutEntity::Spawn()
{
	DevMsg("SPAWNING!!\n");

	Precache();
	SetModel("models\\cabinets\\two_player_arcade.mdl");// VarArgs("%s", this->GetModelName()));
	//SetSolid(SOLID_NONE);
	SetSolid(SOLID_VPHYSICS);
	SetSize(-Vector(100, 100, 100), Vector(100, 100, 100));
	SetMoveType(MOVETYPE_VPHYSICS);

	//	SetRenderMode(kRenderNormal);

	//SetUse(&CPropHotlinkEntity::UseFunc);

	if (CreateVPhysics())
	{
		IPhysicsObject *pPhysics = this->VPhysicsGetObject();
		if (pPhysics)
		{
			pPhysics->EnableMotion(false);
		}
	}

	BaseClass::Spawn();
}

std::string C_PropShortcutEntity::GetItemId()
{
	return std::string(m_itemId);
}
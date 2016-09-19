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

#include "c_anarchymanager.h"

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

void C_PropShortcutEntity::GetEmbeddedInstances(std::vector<C_EmbeddedInstance*>& embeddedInstances)
{
	// DETECT DYNAMIC TEXTURES
	const model_t* model = this->GetModel();

	IMaterial* pMaterials[1024];
	for (int x = 0; x < 1024; x++)
		pMaterials[x] = NULL;

	modelinfo->GetModelMaterials(model, 1024, &pMaterials[0]);

	//auto it;
	std::vector<C_EmbeddedInstance*>::iterator it;
	IMaterial* pMaterial;
	C_EmbeddedInstance* pEmbeddedInstance;
	for (int x = 0; x < 1024; x++)
	{
		if (pMaterials[x] && pMaterials[x]->HasProxy())
		{
			pMaterial = pMaterials[x];
			pEmbeddedInstance = g_pAnarchyManager->GetCanvasManager()->FindEmbeddedInstance(pMaterial);

			if (pEmbeddedInstance)
			{
				// make sure it isn't already on the list
				it = std::find(embeddedInstances.begin(), embeddedInstances.end(), pEmbeddedInstance);
				if (it == embeddedInstances.end())
					embeddedInstances.push_back(pEmbeddedInstance);
			}
		}
		//	Material: vgui/websurfacealt2
		//	Material: vgui/websurfacealt
		//	Material: vgui/websurfacealt5
		//	Material: vgui/websurfacealt5
		//	Material: vgui/websurfacealt7
	}
}
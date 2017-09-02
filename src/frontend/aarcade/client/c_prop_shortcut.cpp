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
	RecvPropBool(RECVINFO(m_bSlave)),
	RecvPropString(RECVINFO(m_objectId)),
	RecvPropString(RECVINFO(m_itemId)),
	RecvPropString(RECVINFO(m_modelId)),
END_RECV_TABLE()

C_PropShortcutEntity::C_PropShortcutEntity()
{
	m_bInitialized = false;
	m_bAlreadySetObjectEntity = false;
}

C_PropShortcutEntity::~C_PropShortcutEntity()
{
}

void C_PropShortcutEntity::Initialize()
{
	Vector absOrigin = this->GetAbsOrigin();
	object_t* pSpawningObject = g_pAnarchyManager->GetMetaverseManager()->GetSpawningObject();
	if (pSpawningObject)
		DevMsg("Abs origin (dist %f) is: %f %f %f vs %f %f %f\n", pSpawningObject->origin.DistTo(absOrigin), absOrigin.x, absOrigin.y, absOrigin.z, absOrigin.x, absOrigin.y, absOrigin.z);

	if (pSpawningObject && pSpawningObject->origin.DistTo(absOrigin) < 1.0)// GEEZ! not even position is set yet.  defer //&& !Q_strcmp(pSpawningObject->itemId.c_str(), m_itemId) )	// FIXME: accounts for rounding errors, but can produce false-positives!! // FIXME2: m_itemId isn't set until the 1st data update, der.  so this check is probably very unqualified.  too many false positive posibilities.
	{
		// THIS IS THE OBJECT THE CURRENT USER IS SPAWNING!!
		//Precache();
		//SetModel("models\\cabinets\\two_player_arcade.mdl");// VarArgs("%s", this->GetModelName()));
		std::string modelId = (pSpawningObject->modelId != "") ? pSpawningObject->modelId : pSpawningObject->itemId;// g_pAnarchyManager->GenerateLegacyHash("models/cabinets/two_player_arcade.mdl");
		std::string modelFile;

		KeyValues* entryModel = g_pAnarchyManager->GetMetaverseManager()->GetLibraryModel(modelId);
		KeyValues* activeModel = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(entryModel);
		if (activeModel)
			modelFile = activeModel->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID), "models\\cabinets\\two_player_arcade.mdl");	// uses default model if key value read fails
		else
			modelFile = "models\\icons\\missing.mdl";

		SetModel(modelFile.c_str());
		SetSolid(SOLID_NONE);
		SetSize(-Vector(100, 100, 100), Vector(100, 100, 100));
		//SetRenderMode(kRenderTransTexture);
		SetRenderMode(kRenderTransColor);
		SetRenderColorA(160);
		g_pAnarchyManager->ActivateObjectPlacementMode(this);
	}
	else
	{
		// This is a regular object that already existed or somebody else spawned
		//Precache();
		SetModel("models\\icons\\missing.mdl");// VarArgs("%s", this->GetModelName()));
		//SetSolid(SOLID_NONE);
		SetSolid(SOLID_VPHYSICS);
		SetSize(-Vector(100, 100, 100), Vector(100, 100, 100));
		SetMoveType(MOVETYPE_VPHYSICS);

		SetRenderColorA(255);
		SetRenderMode(kRenderNormal);

		if (CreateVPhysics())
		{
			IPhysicsObject *pPhysics = this->VPhysicsGetObject();
			if (pPhysics)
			{
				pPhysics->EnableMotion(false);
			}
		}
	}

	//this->ConfirmNotError();

	m_bInitialized = true;
}

void C_PropShortcutEntity::ConfirmNotError()
{
	// Check if the model's material is an error
	const model_t* TheModel = this->GetModel();
	if (TheModel)
	{
		std::string modelName = modelinfo->GetModelName(TheModel);
		if( m_oldModel != modelName)
		{
			m_oldModel = modelName;

			IMaterial* pMaterial;
			modelinfo->GetModelMaterials(TheModel, 1, &pMaterial);

			bool bIsMultiplayer = false;
			bool bIsMultiplayerHost = false;
			if (!Q_stricmp(pMaterial->GetName(), "models/error/new light1") && (!bIsMultiplayer || bIsMultiplayerHost == -1))
			{
				DevMsg("WARNING: The following model failed to load. Reverting to placeholder: %s\n", modelName.c_str());

				std::string modelId = g_pAnarchyManager->GetMetaverseManager()->ResolveLegacyModel("models/icons/missing.mdl");
				//std::string modelId = g_pAnarchyManager->GetMetaverseManager()->GetMissingModelId();
				if (modelId != "")
					engine->ServerCmd(VarArgs("switchmodel \"%s\" \"models/icons/missing.mdl\" %i;\n", modelId.c_str(), this->entindex()));
			}
		}
	}
}

void C_PropShortcutEntity::OnPreDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnPreDataChanged(updateType);
}

void C_PropShortcutEntity::OnDataChanged(DataUpdateType_t updateType)
{
	//DevMsg("PreDataChangedUpdate: ")//DATA_UPDATE_DATATABLE_CHANGED
	if (!m_bInitialized )
		this->Initialize();
	else
	{
		// do nada
	}

	this->ConfirmNotError();

	if (!m_bAlreadySetObjectEntity && Q_strcmp(m_objectId, ""))
	{
		// associated this entity with this object
		g_pAnarchyManager->GetInstanceManager()->SetObjectEntity(std::string(m_objectId), this);
		m_bAlreadySetObjectEntity = true;

		// handle nodes
		object_t* pObject = g_pAnarchyManager->GetInstanceManager()->GetInstanceObject(m_objectId);
		if (pObject)
		{
			KeyValues* pItemKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(pObject->itemId));
			if (pItemKV)
			{
				if (!Q_strcmp(pItemKV->GetString("info/id"), ""))
				{
					DevMsg("Skipping item with no ID.\n");
				}
				else
				{
					//DevMsg("tester file field: %s\n", pItemKV->GetString("file"));
					std::string fileValue = pItemKV->GetString("file");
					if (fileValue.find_first_of("/\\") == std::string::npos && fileValue.find(".") == std::string::npos)
					{
						// is this the node item we're expecting?
						std::string incomingNodeId = g_pAnarchyManager->GetInstanceManager()->GetIncomingNodeId();
						if (incomingNodeId != "" && !Q_strcmp(incomingNodeId.c_str(), pItemKV->GetString("info/id")))
						{
							// we are CREATING a node instance
							// ================================

							// (this should actually all be done on the SERVER side. for correct parenting.)
							// TODO: work

							// attach all entities listed in the nodevolume to us.
							// TODO: work

							/*
								EntityMatrix matrix, childMatrix;
								matrix.InitFromEntity( const_cast<CBaseEntity *>(pParentEntity), m_iParentAttachment ); // parent->world
								childMatrix.InitFromEntityLocal( this ); // child->world
								Vector localOrigin = (useLocalSpace) ? matrix.WorldToLocal(GetLocalOrigin()) : GetLocalOrigin();
		
								// I have the axes of local space in world space. (childMatrix)
								// I want to compute those world space axes in the parent's local space
								// and set that transform (as angles) on the child's object so the net
								// result is that the child is now in parent space, but still oriented the same way
								VMatrix tmp = matrix.Transpose(); // world->parent
								tmp.MatrixMul( childMatrix, matrix ); // child->parent
								QAngle angles;
								if (useLocalSpace)
								{
									MatrixToAngles(matrix, angles);
									SetLocalAngles(angles);
								}
								UTIL_SetOrigin( this, localOrigin );
							*/

							// create a node instance using the RELATIVE origin & angles of all children
							// TODO: work

							// give it the correct nodestyle too!
							// TODO: work

							// save it with out w/ the correct Id
							// TODO: work

							// now continue loading it & adding all children objects like normal
							// TODO: work

							// finally, make sure they all spawn right away.
							// TODO: work

							// reset the incoming node id
							g_pAnarchyManager->GetInstanceManager()->SetIncomingNodeId("");
							//engine->ServerCmd(VarArgs("showhubsavemenu %i;", this->entindex()));	// DISABLED FOR NOW
						}
						else
						{
							// potentially a node instance ID
							instance_t* pNodeInstance = g_pAnarchyManager->GetInstanceManager()->FindInstance(fileValue);
							if (pNodeInstance)
							{
								DevMsg("Bengo! (However, node spawning is disabled for right now.)\n");	// From here, all the objects get added (knowing their parent entity index), but still use the regular arse object spawning system...
								//g_pAnarchyManager->GetInstanceManager()->LoadInstance(this, fileValue); // DISABLED FOR NOW, as stated above.

								// spawn all the objects too, if needed...
								// FIXME: do it.
								// TODO: work
							}
						}
					}
				}
			}
		}
	}

	BaseClass::OnDataChanged(updateType);
}

void C_PropShortcutEntity::Release()
{
	BaseClass::Release();
}

void C_PropShortcutEntity::Precache(void)
{
	//PrecacheModel("models\\cabinets\\two_player_arcade.mdl");	// FIXME: Probably can precache the correct model here.
	BaseClass::Precache();
}

void C_PropShortcutEntity::Spawn()
{
	m_bInitialized = false;
	Precache();
	//SetModel("models\\cabinets\\two_player_arcade.mdl");// VarArgs("%s", this->GetModelName()));
	//if (engine->IsInGame() && Q_strcmp(g_pAnarchyManager->MapName(), ""))

	object_t* pSpawningObject = g_pAnarchyManager->GetMetaverseManager()->GetSpawningObject();
	if ( !pSpawningObject)	// FIXME: Simply checking if the local user is spawning an entity is NOT enough to determine if THIS entity is the one he is spawning...
		this->Initialize();

	//	DevMsg("SPAWNING!!\n");
	/*
	object_t* pSpawningObject = g_pAnarchyManager->GetMetaverseManager()->GetSpawningObject();
	if (false && pSpawningObject->origin.DistTo(this->GetAbsOrigin()) < 1.0 )// GEEZ! not even position is set yet.  defer //&& !Q_strcmp(pSpawningObject->itemId.c_str(), m_itemId) )	// FIXME: accounts for rounding errors, but can produce false-positives!! // FIXME2: m_itemId isn't set until the 1st data update, der.  so this check is probably very unqualified.  too many false positive posibilities.
	{
		// THIS IS THE OBJECT THE CURRENT USER IS SPAWNING!!
		Precache();
		SetModel("models\\cabinets\\two_player_arcade.mdl");// VarArgs("%s", this->GetModelName()));
		SetSolid(SOLID_NONE);
		SetSize(-Vector(100, 100, 100), Vector(100, 100, 100));
		SetRenderMode(kRenderTransTexture);
		g_pAnarchyManager->ActivateObjectPlacementMode(this);
		BaseClass::Spawn();
	}
	else
	{
		// This is a regular object that already existed or somebody else spawned
		Precache();
		SetModel("models\\cabinets\\two_player_arcade.mdl");// VarArgs("%s", this->GetModelName()));
		//SetSolid(SOLID_NONE);
		SetSolid(SOLID_VPHYSICS);
		SetSize(-Vector(100, 100, 100), Vector(100, 100, 100));
		SetMoveType(MOVETYPE_VPHYSICS);

		if (CreateVPhysics())
		{
			IPhysicsObject *pPhysics = this->VPhysicsGetObject();
			if (pPhysics)
			{
				pPhysics->EnableMotion(false);
			}
		}
	}
	*/
	//	SetRenderMode(kRenderNormal);
	//SetUse(&CPropHotlinkEntity::UseFunc);

	BaseClass::Spawn();
}

std::string C_PropShortcutEntity::GetItemId()
{
	return std::string(m_itemId);
}

std::string C_PropShortcutEntity::GetObjectId()
{
	return std::string(m_objectId);
}

std::string C_PropShortcutEntity::GetModelId()
{
	return std::string(m_modelId);
}

bool C_PropShortcutEntity::GetSlave()
{
	//DevMsg("Returning %i\n", m_bSlave);
	return m_bSlave;
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
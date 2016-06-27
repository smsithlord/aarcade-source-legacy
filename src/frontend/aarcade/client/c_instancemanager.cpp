#include "cbase.h"

#include "aa_globals.h"
#include "c_instancemanager.h"
#include "c_prop_shortcut.h"
#include "filesystem.h"
#include "icliententitylist.h"
#include "c_anarchymanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_InstanceManager::C_InstanceManager()
{
	DevMsg("WebManager: Constructor\n");
}

C_InstanceManager::~C_InstanceManager()
{
	DevMsg("ShortcutManager: Destructor\n");
}

void C_InstanceManager::AddObject(std::string objectId, std::string itemId, std::string modelId, Vector origin, QAngle angles)
{
	std::string goodObjectId = (objectId != "") ? objectId : g_pAnarchyManager->GenerateUniqueId();

	object_t* pObject = new object_t();
	pObject->objectId = goodObjectId;
	pObject->itemId = itemId;
	pObject->modelId = modelId;
	pObject->origin.Init(origin.x, origin.y, origin.z);
	pObject->angles.Init(angles.x, angles.y, angles.z);
	pObject->spawned = false;

	m_objects[goodObjectId] = pObject;

	m_unspawnedObjects.push_back(pObject);
}

bool C_InstanceManager::SpawnNearestObject()
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	
	Vector playerPos = pPlayer->GetAbsOrigin();

	object_t* pNearObject = null;
	object_t* pTestObject = null;
	float fMinDist = -1;
	float fTestDist;
	std::vector<object_t*>::iterator nearestIt;
	std::vector<object_t*>::iterator it = m_unspawnedObjects.begin();
	while (it != m_unspawnedObjects.end())
	{
		pTestObject = *it;
		fTestDist = pTestObject->origin.DistTo(playerPos);
		if (fMinDist == -1 || fTestDist < fMinDist)
		{
			fMinDist = fTestDist;
			pNearObject = pTestObject;
			nearestIt = it;
		}

		it++;
	}
	/*
	unsigned int uNumObjects = m_unspawnedObjects.size();
	unsigned int i;
	for (i = 0; i < uNumObjects; i++)
	{
		pTestObject = m_unspawnedObjects[i];
		fTestDist = pTestObject->origin.DistTo(playerPos);
		if (fMinDist == -1 || fTestDist < fMinDist)
		{
			fMinDist = fTestDist;
			pNearObject = pTestObject;
		}
	}
	*/

	if (pNearObject)
	{
		m_unspawnedObjects.erase(nearestIt);

		pNearObject->spawned = true;	// FIXME: This really shouldn't be set to true until after it exists on the client. there should be a different state for waiting to spawn.

		KeyValues* model = g_pAnarchyManager->GetMetaverseManager()->GetLibraryModel(pNearObject->modelId);
		if (model)
		{
			DevMsg("Spawning an object w/ %s\n", pNearObject->modelId.c_str());

			KeyValues* active = model->FindKey("current");
			if (!active)
				active = model->FindKey("local", true);

			std::string modelFile = active->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID));
			std::string msg = VarArgs("spawnshortcut \"%s\" \"%s\" %.10f %.10f %.10f %.10f %.10f %.10f\n", pNearObject->itemId.c_str(), modelFile.c_str(), pNearObject->origin.x, pNearObject->origin.y, pNearObject->origin.z, pNearObject->angles.x, pNearObject->angles.y, pNearObject->angles.z);
			engine->ServerCmd(msg.c_str(), false);
		}
		else
		{
			DevMsg("Could not spawn object because it's model was not found: %s\n", pNearObject->modelId.c_str());
		}
	}

	return true;
}
/*
void C_InstanceManager::SpawnItem(std::string id)
{
	// Figure out where to place it
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	Vector forward;
	pPlayer->EyeVectors(&forward);

	trace_t tr;
	UTIL_TraceLine(pPlayer->EyePosition(),
		pPlayer->EyePosition() + forward * MAX_TRACE_LENGTH, MASK_NPCSOLID,
		pPlayer, COLLISION_GROUP_NONE, &tr);

	// No hit? We're done.
	if (tr.fraction == 1.0)
		return;

	VMatrix entToWorld;
	Vector xaxis;
	Vector yaxis;

	if (tr.plane.normal.z == 0.0f)
	{
		yaxis = Vector(0.0f, 0.0f, 1.0f);
		CrossProduct(yaxis, tr.plane.normal, xaxis);
		entToWorld.SetBasisVectors(tr.plane.normal, xaxis, yaxis);
	}
	else
	{
		Vector ItemToPlayer;
		VectorSubtract(pPlayer->GetAbsOrigin(), Vector(tr.endpos.x, tr.endpos.y, tr.endpos.z), ItemToPlayer);

		xaxis = Vector(ItemToPlayer.x, ItemToPlayer.y, ItemToPlayer.z);

		CrossProduct(tr.plane.normal, xaxis, yaxis);
		if (VectorNormalize(yaxis) < 1e-3)
		{
			xaxis.Init(0.0f, 0.0f, 1.0f);
			CrossProduct(tr.plane.normal, xaxis, yaxis);
			VectorNormalize(yaxis);
		}
		CrossProduct(yaxis, tr.plane.normal, xaxis);
		VectorNormalize(xaxis);

		entToWorld.SetBasisVectors(xaxis, yaxis, tr.plane.normal);
	}

	QAngle angles;
	MatrixToAngles(entToWorld, angles);

	//char buf[512];
	//Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", tr.endpos.x, tr.endpos.y, tr.endpos.z);
	//pObjectKV->SetString("origin", buf);
	//Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", angles.x, angles.y, angles.z);
	//pObjectKV->SetString("angles", buf);

	//pClientArcadeResources->AddObjectToArrangement(pObjectKV);
	//pClientArcadeResources->SaveArrangements(true);

//	ConVar* NextToMakeTypeVar = cvar->FindVar("NextToMakeType");
//	NextToMakeTypeVar->SetValue(pItemKV->GetString("type", "other"));

	std::string msg = VarArgs("spawnshortcut \"%s\" \"%s\" %.10f %.10f %.10f %.10f %.10f %.10f\n", id.c_str(), "models/cabinets/two_player_arcade.mdl", tr.endpos.x, tr.endpos.y, tr.endpos.z, angles.x, angles.y, angles.z);
	engine->ServerCmd(msg.c_str(), false);
}
*/

void C_InstanceManager::LoadLegacyInstance()
{
	DevMsg("Load the instance!!!\n");

	FileFindHandle_t pFileFindHandle;
	const char *pInstanceFileName = g_pFullFileSystem->FindFirstEx("maps\\dm_peachs_castle*.set", "GAME", &pFileFindHandle);
	while (pInstanceFileName != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(pFileFindHandle))
		{
			pInstanceFileName = g_pFullFileSystem->FindNext(pFileFindHandle);
			continue;
		}

		break;
		
//		pInstanceFileName = g_pFullFileSystem->FindNext(pFileFindHandle);
	}
	g_pFullFileSystem->FindClose(pFileFindHandle);

	if (pInstanceFileName)
	{
		bool spawnedOne = false;
		KeyValues* item;
		KeyValues* activeItem;
		//KeyValues* modelSearchInfo = new KeyValues("search");
		std::string modelId;
		KeyValues* model;
		KeyValues* activeModel;
		//std::string modelFile;
		std::string itemId;
		std::string fileName = "maps/" + std::string(pInstanceFileName);
		KeyValues* legacyKv = new KeyValues("instance");
		std::string testBuf;
		if (legacyKv->LoadFromFile(g_pFullFileSystem, fileName.c_str(), "GAME"))
		{
			for (KeyValues *sub = legacyKv->FindKey("objects", true)->GetFirstSubKey(); sub; sub = sub->GetNextKey())
			{
				itemId = g_pAnarchyManager->ExtractLegacyId(sub->GetString("itemfile"));

				if (itemId == "")
				{
					// don't generate item id's for legacy prop objects
					testBuf = sub->GetString("itemfile");
					size_t foundExt = testBuf.find(".mdl");
					if (foundExt != testBuf.length() - 4)
						itemId = g_pAnarchyManager->GenerateLegacyHash(sub->GetString("itemfile"));
				}

				modelId = g_pAnarchyManager->GenerateLegacyHash(sub->GetString("model"));
				//DevMsg("Item title: %s w/ %s\n", activeItem->GetString("title"), modelId.c_str());

				// alright, spawn this object
				Vector origin;
				UTIL_StringToVector(origin.Base(), sub->GetString("origin", "0 0 0"));

				QAngle angles;
				UTIL_StringToVector(angles.Base(), sub->GetString("angles", "0 0 0"));

				this->AddObject("", itemId, modelId, origin, angles);
				/*
				item = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(itemId);
				if (item)
				{
					activeItem = item->FindKey("current");
					if (!activeItem)
						activeItem = item->FindKey("local", true);
				}
				*/
			}
		}
		legacyKv->deleteThis();
		//modelSearchInfo->deleteThis();
	}
}
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
	m_fNearestSpawnDist = 0;
	m_fLastSpawnActionPressed = 0;
	m_uNextFlashedObject = 0;
}

C_InstanceManager::~C_InstanceManager()
{
	DevMsg("ShortcutManager: Destructor\n");
	m_instances.clear();
}

void C_InstanceManager::SpawnObject(object_t* object)
{
	auto it = std::find(m_unspawnedObjects.begin(), m_unspawnedObjects.end(), object);
	if (it != m_unspawnedObjects.end())
		m_unspawnedObjects.erase(it);
	else
		return;
	
	object->spawned = true;	// FIXME: This really shouldn't be set to true until after it exists on the client. there should be a different state for waiting to spawn.
	KeyValues* model = g_pAnarchyManager->GetMetaverseManager()->GetLibraryModel(object->modelId);
	if (model)
	{
	//	DevMsg("Spawning an object w/ %s\n", object->modelId.c_str());

		KeyValues* active = model->FindKey("current");
		if (!active)
			active = model->FindKey("local", true);

		std::string modelFile = active->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID));
		std::string msg = VarArgs("spawnshortcut \"%s\" \"%s\" %.10f %.10f %.10f %.10f %.10f %.10f %.10f %i\n", object->itemId.c_str(), modelFile.c_str(), object->origin.x, object->origin.y, object->origin.z, object->angles.x, object->angles.y, object->angles.z, object->scale, object->slave);
		engine->ServerCmd(msg.c_str(), false);
	}
	else
	{
		DevMsg("Could not spawn object because it's model was not found: %s\n", object->modelId.c_str());
	}
}

object_t* C_InstanceManager::AddObject(std::string objectId, std::string itemId, std::string modelId, Vector origin, QAngle angles, float scale, bool slave)
{
	std::string goodObjectId = (objectId != "") ? objectId : g_pAnarchyManager->GenerateUniqueId();

	object_t* pObject = new object_t();
	pObject->objectId = goodObjectId;
	pObject->itemId = itemId;
	pObject->modelId = modelId;
	pObject->origin.Init(origin.x, origin.y, origin.z);
	pObject->angles.Init(angles.x, angles.y, angles.z);
	pObject->spawned = false;
	pObject->scale = scale;
	pObject->slave = slave;

	m_objects[goodObjectId] = pObject;

	m_unspawnedObjects.push_back(pObject);
	return pObject;
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
		if (fTestDist < m_fNearestSpawnDist && (fMinDist == -1 || fTestDist < fMinDist))
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
		this->SpawnObject(pNearObject);
		return true;
	}

	return false;
}

void C_InstanceManager::AddInstance(std::string instanceId, std::string mapId, std::string title, std::string file, std::string workshopIds, std::string mountIds)
{
	//DevMsg("Add instance for mapid %s with title %s and with file %s\n", mapId.c_str(), title.c_str(), file.c_str());
	instance_t* pInstance = new instance_t();
	pInstance->id = instanceId;
	pInstance->mapId = mapId;
	pInstance->title = title;
	pInstance->file = file;
	pInstance->workshopIds = workshopIds;
	pInstance->mountIds = mountIds;

	m_instances[instanceId] = pInstance;
}

instance_t* C_InstanceManager::GetInstance(std::string id)
{
	std::map<std::string, instance_t*>::iterator it = m_instances.find(id);
	if (it != m_instances.end())
		return it->second;
	
	return null;
}

instance_t* C_InstanceManager::FindInstance(std::string mapId)
{
	std::map<std::string, instance_t*>::iterator it = m_instances.begin();
	while (it != m_instances.end())
	{
		if ( !Q_stricmp(it->second->mapId.c_str(), mapId.c_str()) )
			return it->second;

		it++;
	}

	return null;
}

void C_InstanceManager::FindAllInstances(std::string mapId, std::vector<instance_t*> &instances)
{
	std::map<std::string, instance_t*>::iterator it = m_instances.begin();
	while (it != m_instances.end())
	{
		//if (it->second->mapId == mapId)
		if (!Q_stricmp(it->second->mapId.c_str(), mapId.c_str()))
			instances.push_back(it->second);

		it++; 
	}
}

void C_InstanceManager::LegacyMapIdFix(std::string legacyMapName, std::string mapId)
{
	std::map<std::string, instance_t*>::iterator it = m_instances.begin();
	while (it != m_instances.end())
	{
		//if (it->second->mapId == legacyMapName)
		if (!Q_stricmp(it->second->mapId.c_str(), legacyMapName.c_str()))
			it->second->mapId = mapId;

		it++;
	}
}

//#include "../../../public/engine/ivdebugoverlay.h"
#include "../../../game/shared/debugoverlay_shared.h"
void C_InstanceManager::SpawnActionPressed()
{
	if (!g_pAnarchyManager->IsInitialized())
		return;

	float fMinDist = 420.0;
	float fDuration = 4.0f;
	DevMsg("Time is: %f\n", gpGlobals->realtime - m_fLastSpawnActionPressed);

	// determine if there is anything to spawn
	Vector testPlayerPos = C_BasePlayer::GetLocalPlayer()->GetAbsOrigin();

	bool bHasOne = false;
	//object_t* pTestNearObject = null;
	object_t* pTestTestObject = null;
	//float fTestMinDist = -1;
	float fTestTestDist;
	std::vector<object_t*>::iterator testIt = m_unspawnedObjects.begin();
	while (testIt != m_unspawnedObjects.end())
	{
		pTestTestObject = *testIt;
		fTestTestDist = pTestTestObject->origin.DistTo(testPlayerPos);
		if (fMinDist == -1 || fTestTestDist < fMinDist)
		{
			bHasOne = true;
			break;
		}

		testIt++;
	}

	if (!bHasOne)
		return;

	//if (m_fLastSpawnActionPressed != 0 && gpGlobals->realtime - m_fLastSpawnActionPressed < fDuration)
	if ( true)	// turn off double-tap loading if unspawned item title previews are on while walking around
	{
		m_fLastSpawnActionPressed = gpGlobals->realtime;

		std::string instanceId = g_pAnarchyManager->GetInstanceId();
		if (instanceId != "")
		{
			std::string uri = "asset://ui/spawnItems.html?max=" + std::string("420");

			C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
			g_pAnarchyManager->GetAwesomiumBrowserManager()->SelectAwesomiumBrowserInstance(pHudBrowserInstance);
			pHudBrowserInstance->SetUrl(uri);
			g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, false, pHudBrowserInstance);
		}
	}
	else
	{
		m_fLastSpawnActionPressed = gpGlobals->realtime;

	//	while (true)
		//{
			C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();

			Vector playerPos = pPlayer->GetAbsOrigin();

			//object_t* pNearObject = null;
			object_t* pTestObject = null;
			float fTestDist;
			//std::vector<object_t*>::iterator nearestIt;
			std::vector<object_t*>::iterator it = m_unspawnedObjects.begin();
			while (it != m_unspawnedObjects.end())
			{
				pTestObject = *it;
				fTestDist = pTestObject->origin.DistTo(playerPos);
				//if (fTestDist < m_fNearestSpawnDist && (fMinDist == -1 || fTestDist < fMinDist))
				if (fMinDist == -1 || fTestDist < fMinDist)
				{
					//fMinDist = fTestDist;
					//pNearObject = pTestObject;
					//nearestIt = it;

					KeyValues* item = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(pTestObject->itemId.c_str());
					if (item)
					{
						KeyValues* active = item->FindKey("current");
						if (!active)
							active = item->FindKey("local", true);

						if (active)
							NDebugOverlay::Text(pTestObject->origin, active->GetString("title"), true, fDuration);
							//debugoverlay->AddTextOverlay(pTestObject->origin, 3.0, "%s", active->GetString("title"));
					}
					//std::string modelFile = active->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID));
					//std::string msg = VarArgs("spawnshortcut \"%s\" \"%s\" %.10f %.10f %.10f %.10f %.10f %.10f %.10f\n", pNearObject->itemId.c_str(), modelFile.c_str(), pNearObject->origin.x, pNearObject->origin.y, pNearObject->origin.z, pNearObject->angles.x, pNearObject->angles.y, pNearObject->angles.z, pNearObject->scale);
					//engine->ServerCmd(msg.c_str(), false);
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

			/*
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
					std::string msg = VarArgs("spawnshortcut \"%s\" \"%s\" %.10f %.10f %.10f %.10f %.10f %.10f %.10f\n", pNearObject->itemId.c_str(), modelFile.c_str(), pNearObject->origin.x, pNearObject->origin.y, pNearObject->origin.z, pNearObject->angles.x, pNearObject->angles.y, pNearObject->angles.z, pNearObject->scale);
					engine->ServerCmd(msg.c_str(), false);
				}
				else
				{
					DevMsg("Could not spawn object because it's model was not found: %s\n", pNearObject->modelId.c_str());
				}

				return true;
			}
			*/
	//	}
	}
}

void C_InstanceManager::ChangeModel(C_BaseEntity* pEntity, std::string in_modelFile)
{
	std::string modelFile = in_modelFile;
	if (!g_pFullFileSystem->FileExists(modelFile.c_str()))
		modelFile = "models/icons/missing.mdl";

	engine->ServerCmd(VarArgs("switchmodel \"%s\" %i;\n", modelFile.c_str(), pEntity->entindex()));
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

void C_InstanceManager::LevelShutdownPostEntity()
{
	DevMsg("FIXME: C_InstanceManager::LevelShutdownPostEntity needs to actually clear shit out (or does it?)!!\n");
	m_uNextFlashedObject = 0;
	m_fLastSpawnActionPressed = 0;
	m_fNearestSpawnDist = 0;

	// clear out objects and unspanwed objects
	auto it = m_objects.begin();
	while (it != m_objects.end())
	{
		delete it->second;
		it++;
	}

	m_objects.clear();
	m_unspawnedObjects.clear();
}

void C_InstanceManager::Update()
{
	if (engine->IsInGame() && !g_pAnarchyManager->GetSuspendEmbedded() && g_pAnarchyManager->GetInstanceId() != "")
	{
	//	DevMsg("Letters callback\n");

		// grab an iterator to the last flashed object and increment it, otherwise just use .begin()
		unsigned int max = m_unspawnedObjects.size();
		if (m_uNextFlashedObject >= max)
			m_uNextFlashedObject = 0;

		//m_uNextFlashedObject++;
		//if (m_uNextFlashedObject >= max)
//			m_uNextFlashedObject = 0;

		if (max > 0)
		{
			C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
			Vector playerPos = pPlayer->GetAbsOrigin();

			object_t* pTestObject = null;
			float fDuration = 3.0f;
			float fMinDist = 420.0;
			float fTestDist;
			//std::vector<object_t*>::iterator nearestIt;
			//std::vector<object_t*>::iterator it = m_unspawnedObjects.begin();
			while (m_uNextFlashedObject < max)
			{
				pTestObject = m_unspawnedObjects[m_uNextFlashedObject];
				fTestDist = pTestObject->origin.DistTo(playerPos);
				//if (fTestDist < m_fNearestSpawnDist && (fMinDist == -1 || fTestDist < fMinDist))
				if (fMinDist == -1 || fTestDist < fMinDist)
				{
					//fMinDist = fTestDist;
					//pNearObject = pTestObject;
					//nearestIt = it;

					KeyValues* kv = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(pTestObject->itemId);
					if (!kv)	// load the model instead if no item exists
						kv = g_pAnarchyManager->GetMetaverseManager()->GetLibraryModel(pTestObject->modelId);

					if (kv)
					{
						KeyValues* active = kv->FindKey("current");
						if (!active)
							active = kv->FindKey("local", true);

						if (active)
						{
							NDebugOverlay::Text(pTestObject->origin, active->GetString("title"), true, fDuration);
							break;
						}
						//debugoverlay->AddTextOverlay(pTestObject->origin, 3.0, "%s", active->GetString("title"));
					}
					//std::string modelFile = active->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID));
					//std::string msg = VarArgs("spawnshortcut \"%s\" \"%s\" %.10f %.10f %.10f %.10f %.10f %.10f %.10f\n", pNearObject->itemId.c_str(), modelFile.c_str(), pNearObject->origin.x, pNearObject->origin.y, pNearObject->origin.z, pNearObject->angles.x, pNearObject->angles.y, pNearObject->angles.z, pNearObject->scale);
					//engine->ServerCmd(msg.c_str(), false);
				}

				break;
				//m_uNextFlashedObject++;
			}

			m_uNextFlashedObject++;
			if (m_uNextFlashedObject >= max)
				m_uNextFlashedObject = 0;
		}
	}
}

void C_InstanceManager::LoadLegacyInstance(std::string instanceId)
{
	DevMsg("Load the instance!!!\n");

	instance_t* pInstance = this->GetInstance(instanceId);
	
	bool spawnedOne = false;
	KeyValues* item;
	KeyValues* activeItem;
	//KeyValues* modelSearchInfo = new KeyValues("search");
	std::string modelId;
	KeyValues* model;
	KeyValues* activeModel;
	//std::string modelFile;
	std::string itemId;
	std::string fileName = pInstance->file;
	KeyValues* legacyKv = new KeyValues("instance");
	std::string testBuf;
	if (legacyKv->LoadFromFile(g_pFullFileSystem, fileName.c_str(), ""))
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
				{
					KeyValues* pSearchInfo = new KeyValues("search");
					pSearchInfo->SetString("file", sub->GetString("itemfile"));

					KeyValues* foundActive = null;
					KeyValues* foundItem = g_pAnarchyManager->GetMetaverseManager()->FindLibraryItem(pSearchInfo);
					if (foundItem)
					{
						foundActive = foundItem->FindKey("current");
						if (!foundActive)
							foundActive = foundItem->FindKey("local");

						if (foundActive)
							itemId = foundActive->GetString("info/id");
					}

					if (!foundActive)
					{
						DevMsg("Instance failed to resolve item with itemfile: %s\n", sub->GetString("itemfile"));
						itemId = g_pAnarchyManager->GenerateLegacyHash(sub->GetString("itemfile"));
					}
				//	else
				//		DevMsg("Resolved it to: %s\n", itemId.c_str());
				}
			}

			modelId = g_pAnarchyManager->GenerateLegacyHash(sub->GetString("model"));
			//DevMsg("Item title: %s w/ %s\n", activeItem->GetString("title"), modelId.c_str());

			// alright, spawn this object
			Vector origin;
			UTIL_StringToVector(origin.Base(), sub->GetString("origin", "0 0 0"));

			QAngle angles;
			UTIL_StringToVector(angles.Base(), sub->GetString("angles", "0 0 0"));

			float scale = sub->GetFloat("scale", 1.0f);

			bool slave = (sub->GetInt("slave") > 0);

			this->AddObject("", itemId, modelId, origin, angles, scale, slave);
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
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
	m_pInstanceKV = null;

	m_pTransform = new transform_t();
	m_pTransform->offX = 0;
	m_pTransform->offY = 0;
	m_pTransform->offZ = 0;
	m_pTransform->rotP = 0;
	m_pTransform->rotY = 0;
	m_pTransform->rotR = 0;
	m_pTransform->scale = 1.0;

	m_pRecentModelIdConVar = cvar->FindVar("recent_model_id");//null;

	m_incomingNodeId = "";

	m_iUnspawnedWithinRangeEstimate = 0;

	SetDefLessFunc(m_instances);
}

C_InstanceManager::~C_InstanceManager()
{
	DevMsg("ShortcutManager: Destructor\n");
	//m_instances.clear();
	m_instances.RemoveAll();	// NOTE: Might need to do a Purge&Delete instead!!
}

void C_InstanceManager::ResetObjectChanges(C_PropShortcutEntity* pShortcut)
{
	KeyValues* pEntryKV = m_pInstanceKV->FindKey(VarArgs("objects/%s", pShortcut->GetObjectId().c_str()));

	// check if this object has been saved to the KV yet
	if (pEntryKV)
	{
		// yes, it already exists and has values to revert to
		object_t* pObject = this->GetInstanceObject(pShortcut->GetObjectId());
		// revert
		// TODO: everything should be condensed into a single server-command

		// FOR NOW: start by setting object ID's
		std::string modelId = pObject->modelId;// pShortcut->GetModelId();
		std::string modelFile;

		KeyValues* entryModel = g_pAnarchyManager->GetMetaverseManager()->GetLibraryModel(modelId);
		KeyValues* activeModel = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(entryModel);
		if (activeModel)
			modelFile = activeModel->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID), "models\\cabinets\\two_player_arcade.mdl");	// uses default model if key value read fails
		else
			modelFile = "models\\cabinets\\two_player_arcade.mdl";

		pShortcut->PrecacheModel(modelFile.c_str());	// not needed.  handled server-side?
		pShortcut->SetModel(modelFile.c_str());	// not needed.  handled server-side?
		//engine->ServerCmd(VarArgs("setobjectids %i \"%s\" \"%s\" \"%s\";\n", pShortcut->entindex(), modelId.c_str(), modelId.c_str(), modelFile.c_str()), false);

		// FOR NOW: then do position, rotation, and scale
		//Vector origin = pShortcut->GetAbsOrigin();
		//QAngle angles = pShortcut->GetAbsAngles();
		//engine->ServerCmd(VarArgs("setcabpos %i %f %f %f %f %f %f;\n", pShortcut->entindex(), pObject->origin.x, pObject->origin.y, pObject->origin.z, pObject->angles.x, pObject->angles.y, pObject->angles.z), false);	// FIXME: Other calls to setcabpos in client code may have an additional unused blank string param at the end that the server-side code doesn't ask for.  Fix that.  No extra param should be sent.

		// FOR NOW: lastly, do scale
//		engine->ServerCmd(VarArgs("switchmodel \"%s\" \"%s\" %i;\n", modelId.c_str(), modelFile.c_str(), pShortcut->entindex()));	// TODO: This shouldn't be required cu setobjectids is supposed to do everything this does...
		engine->ServerCmd(VarArgs("setobjectids %i \"%s\" \"%s\" \"%s\";\n", pShortcut->entindex(), pObject->itemId.c_str(), modelId.c_str(), modelFile.c_str()), false);
		engine->ServerCmd(VarArgs("setcabpos %i %f %f %f %f %f %f;\n", pShortcut->entindex(), pObject->origin.x, pObject->origin.y, pObject->origin.z, pObject->angles.x, pObject->angles.y, pObject->angles.z), false);	// FIXME: Other calls to setcabpos in client code may have an additional unused blank string param at the end that the server-side code doesn't ask for.  Fix that.  No extra param should be sent.
		engine->ServerCmd(VarArgs("setscale %i %f;\n", pShortcut->entindex(), pObject->scale), false);

		//engine->ServerCmd(VarArgs("setobjectids %i \"%s\" \"%s\" \"%s\"; setcabpos %i %f %f %f %f %f %f; setscale %i %f;", pShortcut->entindex(), pObject->itemId.c_str(), modelId.c_str(), modelFile.c_str(), pShortcut->entindex(), pObject->origin.x, pObject->origin.y, pObject->origin.z, pObject->angles.x, pObject->angles.y, pObject->angles.z, pShortcut->entindex(), pObject->scale), false);
		
		// undo build mode FX
		engine->ServerCmd(VarArgs("makenonghost %i;\n", pShortcut->entindex()), false);
		/*
		pShortcut->SetRenderColorA(255);
		pShortcut->SetRenderMode(kRenderNormal);

		// make the prop solid
		pShortcut->SetSolid(SOLID_VPHYSICS);
		pShortcut->SetSize(-Vector(100, 100, 100), Vector(100, 100, 100));
		pShortcut->SetMoveType(MOVETYPE_VPHYSICS);

		if (pShortcut->CreateVPhysics())
		{
			IPhysicsObject *pPhysics = pShortcut->VPhysicsGetObject();
			if (pPhysics)
			{
				pPhysics->EnableMotion(false);
			}
		}
		*/
	}
	else
	{
		// nothing to revert to, just remove it.
		this->RemoveEntity(pShortcut);
	}
}

void C_InstanceManager::ModelFileChanged(std::string modelId)
{
	// 1. find every object using this model
	//std::map<std::string, object_t*> m_objects;
	std::string modelFile;
	KeyValues* pModelKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->GetLibraryModel(modelId));
	if (pModelKV)
		modelFile = pModelKV->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID));

	if (modelFile != "")
	{
		C_BaseEntity* pBaseEntity;
		C_PropShortcutEntity* pShortcut;
		object_t* pObject;
		int entityIndex;
		auto it = m_objects.begin();
		while (it != m_objects.end())
		{
			pObject = it->second;

			if (pObject->modelId == modelId)
			{
				entityIndex = pObject->entityIndex;

				if (entityIndex >= 0)
				{
					pBaseEntity = C_BaseEntity::Instance(entityIndex);
					if (pBaseEntity)
					{
						pShortcut = dynamic_cast<C_PropShortcutEntity*>(pBaseEntity);
						this->ChangeModel(pBaseEntity, modelId, modelFile, false);
					}
				}
			}

			it++;
		}
	}

		// 2. re-initialize that object so it re-reads the model file name from it's model.
		// 3. fin
}

void C_InstanceManager::ResetTransform()
{
	m_pTransform->offX = 0;
	m_pTransform->offY = 0;
	m_pTransform->offZ = 0;
	m_pTransform->rotP = 0;
	m_pTransform->rotY = 0;
	m_pTransform->rotR = 0;
	m_pTransform->scale = 1.0;
}

void C_InstanceManager::AdjustObjectOffset(float x, float y, float z)
{
	m_pTransform->offX = x;
	m_pTransform->offY = y;
	m_pTransform->offZ = z;
}

void C_InstanceManager::AdjustObjectRot(float p, float y, float r)
{
	m_pTransform->rotP = p;
	m_pTransform->rotY = y;
	m_pTransform->rotR = r;
}

void C_InstanceManager::AdjustObjectScale(float scale)
{
	if (scale < 0.05)
		m_pTransform->scale = -0.05;
	else
		m_pTransform->scale = scale;
}

void C_InstanceManager::SaveActiveInstance(KeyValues* pInstanceKV)
{
	if (!pInstanceKV)
		pInstanceKV = m_pInstanceKV;

	std::string instanceId = pInstanceKV->GetString("info/local/id");

	DevMsg("Saving instance ID %s\n", instanceId.c_str());

	// now save out to the SQL
	g_pAnarchyManager->GetMetaverseManager()->SaveSQL(null, "instances", instanceId.c_str(), pInstanceKV);
}

void C_InstanceManager::ApplyChanges(C_PropShortcutEntity* pShortcut, bool bShouldSave)
{
	if (!m_pInstanceKV)
	{
		//DevMsg("WARNING: This is the 1st time this instance has been modified by the local user.  It's KeyValues structure must be generated first, before changes can be saved.\n");

		DevMsg("ERROR: No KEY for this instance!\n");
		return;
	}

	// untag as legacy, because any instance that gets saved out gets saved out in REDUX format
	instance_t* instance = this->GetCurrentInstance();
	if (instance && instance->legacy)
	{
		//KeyValues* pLegacyKV = m_pInstanceKV->FindKey("legacy");

		//if (pLegacyKV)
			//pLegacyKV->SetInt("", 0);
//			m_pInstanceKV->RemoveSubKey(pLegacyKV);
		m_pInstanceKV->SetInt("legacy", 0);
		instance->legacy = 0;
		instance->file = "";
	}

	std::string objectId = pShortcut->GetObjectId();

	// update this object's object_t
	object_t* object = g_pAnarchyManager->GetInstanceManager()->GetInstanceObject(objectId);
	if (!object)
		DevMsg("FATAL ERROR: This shortcut has no object data struct!\n");

	object->scale = pShortcut->GetModelScale();
	object->itemId = pShortcut->GetItemId();
	object->modelId = pShortcut->GetModelId();
	/*
	object->origin = (object->child) ? pShortcut->GetLocalOrigin() : pShortcut->GetAbsOrigin();
	object->angles = (object->child) ? pShortcut->GetLocalAngles() : pShortcut->GetAbsAngles();
	*/
	object->origin = pShortcut->GetAbsOrigin();
	object->angles = pShortcut->GetAbsAngles();
	object->slave = pShortcut->GetSlave();

	KeyValues* pObjectKV = m_pInstanceKV->FindKey(VarArgs("objects/%s", objectId.c_str()), true);

	// position
	char buf[AA_MAX_STRING];
	Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", object->origin.x, object->origin.y, object->origin.z);
	std::string position = buf;

	// rotation
	Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", object->angles.x, object->angles.y, object->angles.z);
	std::string rotation = buf;

	int slave = (object->slave) ? 1 : 0;
	int child = (object->child) ? 1 : 0;

	this->CreateObject(pObjectKV, objectId, object->itemId, object->modelId, position, rotation, object->scale, slave, child);

	if (bShouldSave)
	{
		this->SaveActiveInstance();
		//DevMsg("Saving instance ID %s vs %s\n", m_pInstanceKV->GetString("info/local/id"), instance->id.c_str());

		// now save out to the SQL
		//g_pAnarchyManager->GetMetaverseManager()->SaveSQL(null, "instances", m_pInstanceKV->GetString("info/local/id"), m_pInstanceKV);
	}
}

void C_InstanceManager::SetObjectEntity(std::string objectId, C_BaseEntity* pEntity)
{
	object_t* pObject = this->GetInstanceObject(objectId);
	pObject->entityIndex = pEntity->entindex();
}

C_BaseEntity* C_InstanceManager::GetObjectEntity(std::string objectId)
{
	object_t* pObject = this->GetInstanceObject(objectId);
	if (pObject->entityIndex != -1)
		return C_BaseEntity::Instance(pObject->entityIndex);
	else
		return null;
}

void C_InstanceManager::CreateObject(KeyValues* pObjectKV, std::string objectId, std::string itemId, std::string modelId, std::string position, std::string rotation, float scale, int slave, int child)
{
	////bool slave = (sub->GetInt("local/slave") > 0);	// FIXME: slave mode needs to be determined somehow
	//bool physics = (sub->GetInt("slave") > 0); // FIXME: TODO: Use the physics stuff too!

	// assume "local" user key.
	pObjectKV->SetString("local/info/id", objectId.c_str());
	pObjectKV->SetString("local/info/created", VarArgs("%llu", g_pAnarchyManager->GetTimeNumber()));
	pObjectKV->SetString("local/info/owner", "local");
	pObjectKV->SetString("local/item", itemId.c_str());
	pObjectKV->SetString("local/model", modelId.c_str());
	pObjectKV->SetString("local/position", position.c_str());
	pObjectKV->SetString("local/rotation", rotation.c_str());
	pObjectKV->SetFloat("local/scale", scale);
	pObjectKV->SetInt("local/slave", slave);
	pObjectKV->SetInt("local/child", child);
}

std::string C_InstanceManager::CreateBlankInstance(int iLegacy, KeyValues* pInstanceKV, std::string instanceId, std::string mapId, std::string title, std::string file, std::string workshopIds, std::string mountIds, std::string style, C_Backpack* pBackpack)
{
	// DO WORK
	// (create a new instance, save it to the db, then overwrite the instanceId with the newly created id.)

	if (instanceId == "")
		instanceId = g_pAnarchyManager->GenerateUniqueId();

	if (title == "")
	{
		if (workshopIds != "")
		{
			SteamWorkshopDetails_t* pDetails = g_pAnarchyManager->GetWorkshopManager()->GetWorkshopSubscription((PublishedFileId_t)Q_atoui64(workshopIds.c_str()));
			if (pDetails)
				title = "Unnamed (" + pDetails->title + ")";
		}

		if (title == "")
			title = "Unnamed (" + instanceId + ")";
	}

	bool bNeedCleanup = false;
	if (!pInstanceKV)
	{
		// If we are creating the KV here, then save it out & clean it up right away as well.
		bNeedCleanup = true;
		pInstanceKV = new KeyValues("instance");
	}
	else
		pInstanceKV->Clear();	// Clear out the KV if we are given one (because we are making it a BLANK instance KV)

	// set workshop & legacy versioning
	pInstanceKV->SetInt("generation", 3);
	pInstanceKV->SetInt("legacy", iLegacy);

	// set local 
	KeyValues* pInstanceInfoKV = pInstanceKV->FindKey("info/local", true);
	pInstanceInfoKV->SetString("id", instanceId.c_str());
	pInstanceInfoKV->SetString("created", VarArgs("%llu", g_pAnarchyManager->GetTimeNumber()));
	pInstanceInfoKV->SetString("owner", "local");
	pInstanceInfoKV->SetString("title", title.c_str());
	pInstanceInfoKV->SetString("map", mapId.c_str());
	pInstanceInfoKV->SetString("style", style.c_str());	// for nodes
	pInstanceInfoKV->SetString(VarArgs("platforms/%s/workshopIds", AA_PLATFORM_ID), workshopIds.c_str());
	pInstanceInfoKV->SetString(VarArgs("platforms/%s/mountIds", AA_PLATFORM_ID), mountIds.c_str());
	//pInstanceInfoKV->SetString(VarArgs("platforms/%s/backpackIds", AA_PLATFORM_ID), backpackId.c_str());

	if (!pBackpack)
		g_pAnarchyManager->GetInstanceManager()->AddInstance(iLegacy, instanceId, mapId, title, file, workshopIds, mountIds, style);

	if (bNeedCleanup)
	{
		g_pAnarchyManager->GetMetaverseManager()->SaveSQL(null, "instances", instanceId.c_str(), pInstanceKV);
		pInstanceKV->deleteThis();
		pInstanceKV = null;
	}

	return instanceId;
}

void C_InstanceManager::SpawnObject(object_t* object, bool bShouldGhost)
{
	auto it = std::find(m_unspawnedObjects.begin(), m_unspawnedObjects.end(), object);
	if (it != m_unspawnedObjects.end())
		m_unspawnedObjects.erase(it);
	else
		return;
	
	object->spawned = true;	// FIXME: This really shouldn't be set to true until after it exists on the client. there should be a different state for waiting to spawn.
	//DevMsg("Here it is: %s vs %s\n", object->modelId.c_str(), object->itemId.c_str());
	std::string goodModelId = (object->modelId != "") ? object->modelId : object->itemId;

	KeyValues* model = g_pAnarchyManager->GetMetaverseManager()->GetLibraryModel(goodModelId);
	if (!model)
	{
		DevMsg("WARNING: Model not found in library with given ID! Using default cabinet instead. %s\n", goodModelId.c_str());
		goodModelId = g_pAnarchyManager->GenerateLegacyHash("models/cabinets/two_player_arcade.mdl");
		model = g_pAnarchyManager->GetMetaverseManager()->GetLibraryModel(goodModelId);
	}

	KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(model);
	std::string modelFile = active->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID));

	//if( !g_pFullFileSystem->FileExists(modelFile.c_str(), "GAME")

	int ghost = (bShouldGhost) ? 1 : 0;
	std::string msg = VarArgs("spawnshortcut \"%s\" \"%s\" \"%s\" \"%s\" %.10f %.10f %.10f %.10f %.10f %.10f %.10f %i %i %i\n", object->objectId.c_str(), object->itemId.c_str(), goodModelId.c_str(), modelFile.c_str(), object->origin.x, object->origin.y, object->origin.z, object->angles.x, object->angles.y, object->angles.z, object->scale, object->slave, object->parentEntityIndex, ghost);
	engine->ServerCmd(msg.c_str(), false);
}

object_t* C_InstanceManager::GetNearestObjectToPlayerLook(object_t* pStartingObject)
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return false;

	if (pPlayer->GetHealth() <= 0)
		return false;

	// fire a trace line
	trace_t tr;
	Vector forward;
	pPlayer->EyeVectors(&forward);
	UTIL_TraceLine(pPlayer->EyePosition(), pPlayer->EyePosition() + forward * MAX_COORD_RANGE, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

	Vector startPos = tr.endpos;

	object_t* pNearObject = null;
	object_t* pTestObject = null;
	float fMinDist = 300;
	float fTestDist;

	float fStartingDist = 0.0f;
	if (pStartingObject)
		fStartingDist = pStartingObject->origin.DistTo(startPos);

	std::map<std::string, object_t*>::iterator it = m_objects.begin();
	while (it != m_objects.end())
	{
		pTestObject = it->second;
		fTestDist = pTestObject->origin.DistTo(startPos);
		if (fTestDist > fStartingDist && (fMinDist == -1 || fTestDist < fMinDist))
		{
			fMinDist = fTestDist;
			pNearObject = pTestObject;
		}

		it++;
	}

	return pNearObject;
}

object_t* C_InstanceManager::AddObject(std::string objectId, std::string itemId, std::string modelId, Vector origin, QAngle angles, float scale, bool slave, unsigned int created, std::string owner, unsigned int removed, std::string remover, unsigned int modified, std::string modifier, bool isChild, int iParentEntityIndex)
{
	std::string goodObjectId = (objectId != "") ? objectId : g_pAnarchyManager->GenerateUniqueId();

	DevMsg("Object ID here is: %s\n", goodObjectId.c_str());

	object_t* pObject = new object_t();
	pObject->objectId = goodObjectId;
	pObject->created = created;
	pObject->owner = owner;
	pObject->removed = removed;
	pObject->remover = remover;
	pObject->modified = modified;
	pObject->modifier = modifier;
	pObject->itemId = itemId;
	pObject->modelId = modelId;
	pObject->origin.Init(origin.x, origin.y, origin.z);
	pObject->angles.Init(angles.x, angles.y, angles.z);
	pObject->child = (iParentEntityIndex >= 0) ? true : isChild;
	pObject->spawned = false;
	pObject->scale = scale;
	pObject->slave = slave;
	pObject->entityIndex = -1;
	pObject->parentEntityIndex = iParentEntityIndex;

	m_objects[goodObjectId] = pObject;

	m_unspawnedObjects.push_back(pObject);
	return pObject;
}

unsigned int C_InstanceManager::GetInstanceObjectCount()
{
	unsigned int count = 0;
	std::map<std::string, object_t*>::iterator it = m_objects.begin();
	while (it != m_objects.end())
	{
		count++;
		it++;
	}

	return count;
}

object_t* C_InstanceManager::GetInstanceObject(std::string objectId)
{
	auto it = m_objects.find(objectId);
	if (it != m_objects.end())
		return it->second;
	
	return null;
}

void C_InstanceManager::RemoveEntity(C_PropShortcutEntity* pShortcutEntity, bool bBulkRemove)
{
	int iEntityIndex = pShortcutEntity->entindex();
	C_EmbeddedInstance* pTesterInstance = null;

	// 1. build a vector of this object, & all its children
	std::vector<object_t*> victims;
	auto masterIt = m_objects.begin();
	while (masterIt != m_objects.end())
	{
		if (masterIt->second->entityIndex == iEntityIndex || masterIt->second->parentEntityIndex == iEntityIndex)
		{
			victims.push_back(masterIt->second);

			pTesterInstance = g_pAnarchyManager->GetSteamBrowserManager()->FindSteamBrowserInstanceByEntityIndex(iEntityIndex);
			if (pTesterInstance)
				g_pAnarchyManager->GetSteamBrowserManager()->DestroySteamBrowserInstance(dynamic_cast<C_SteamBrowserInstance*>(pTesterInstance));

			pTesterInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstanceByEntityIndex(iEntityIndex);
			if (pTesterInstance)
				g_pAnarchyManager->GetLibretroManager()->DestroyLibretroInstance(dynamic_cast<C_LibretroInstance*>(pTesterInstance));

			pTesterInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstanceByEntityIndex(iEntityIndex);
			if (pTesterInstance)
				g_pAnarchyManager->GetAwesomiumBrowserManager()->DestroyAwesomiumBrowserInstance(dynamic_cast<C_AwesomiumBrowserInstance*>(pTesterInstance));
		}

		masterIt++;
	}

	// 2. loop through the vector, removing the entities in it.
	KeyValues* pObjectsKV = m_pInstanceKV->FindKey("objects", true);

	C_PropShortcutEntity* pVictimEntity;
	for (unsigned int i = 0; i < victims.size(); i++)
	{
		if (victims[i]->spawned)
		{
			pVictimEntity = dynamic_cast<C_PropShortcutEntity*>(C_BaseEntity::Instance(victims[i]->entityIndex));
			if (pVictimEntity)
			{
				// Entry should be removed from the instance KV also, or at least flag it as "removed" so deleted objects can be undone.
				if (pObjectsKV)
				{
					KeyValues* pEntryKV = pObjectsKV->FindKey(pVictimEntity->GetObjectId().c_str());
					if (pEntryKV)
					{
						pObjectsKV->RemoveSubKey(pEntryKV);

						// save out the instance KV
						if (!bBulkRemove)
							g_pAnarchyManager->GetMetaverseManager()->SaveSQL(null, "instances", m_pInstanceKV->GetString("info/local/id"), m_pInstanceKV);
					}
				}

				// FIXME: What if the object is still unspawned?  Could this ever happen?  If it could, then it'd need to be removed from the unspawned objects vector too.
				auto it = m_objects.find(pVictimEntity->GetObjectId());
				if (it != m_objects.end())
				{
					delete it->second;
					m_objects.erase(it);
				}
			}
		}
	}
	victims.clear();

	if (pObjectsKV && !bBulkRemove)
	{
		// save out the instance KV
		g_pAnarchyManager->GetMetaverseManager()->SaveSQL(null, "instances", m_pInstanceKV->GetString("info/local/id"), m_pInstanceKV);
	}

	if (!bBulkRemove)
		engine->ServerCmd(VarArgs("removeobject %i;\n", iEntityIndex), false);
}

int C_InstanceManager::SetNearestSpawnDist(double maxDist)
{
	m_fNearestSpawnDist = (float)maxDist;
	
	int count = 0;
	// only do distance calculations if we need to
	if (m_fNearestSpawnDist == 99999999999.9)
		count = (int)m_unspawnedObjects.size();
	else
	{
		C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
		Vector playerPos = pPlayer->GetAbsOrigin();

		// figure out how many unspawned objects are within that range
		object_t* pTestObject = null;

		unsigned int uNumObjects = m_unspawnedObjects.size();
		float fTestDist;
		unsigned int i;
		for (i = 0; i < uNumObjects; i++)
		{
			pTestObject = m_unspawnedObjects[i];
			fTestDist = pTestObject->origin.DistTo(playerPos);
			if (fTestDist < m_fNearestSpawnDist)
				count++;
		}
	}

	m_iUnspawnedWithinRangeEstimate = count;
	return count;
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

void C_InstanceManager::AddInstance(int iLegacy, std::string instanceId, std::string mapId, std::string title, std::string file, std::string workshopIds, std::string mountIds, std::string style)
{
	/*
	auto it = m_instances.find(instanceId);
	if (it != m_instances.end())
	{
		DevMsg("WARNING: Instance already exists with id %s, aborting.\n", instanceId.c_str());
		return;
	}
	*/

	DevMsg("Added instance w/ ID %s\n", instanceId.c_str());

	instance_t* pInstance;
	int index = m_instances.Find(instanceId);
	if (index != m_instances.InvalidIndex())
		pInstance = m_instances.Element(index);
	else
		pInstance = new instance_t();

	//DevMsg("Adding instance for:\n\tID: %s\n\tMapID: %s\n\tTitle: %s\n\tStyle: %s\n\tFile: %s\n\tWorkshopIds: %s\n\tMountIds: %s\n\tLegacy: %i\n", instanceId.c_str(), mapId.c_str(), title.c_str(), style.c_str(), file.c_str(), workshopIds.c_str(), mountIds.c_str(), iLegacy);
	//instance_t* pInstance = new instance_t();
	pInstance->id = instanceId;
	pInstance->mapId = mapId;
	pInstance->title = title;
	pInstance->style = style;
	pInstance->file = file;
	pInstance->workshopIds = workshopIds;
	pInstance->mountIds = mountIds;
	//pInstance->backpackId = backpackId;
	pInstance->legacy = iLegacy;

	//m_instances[instanceId] = pInstance;
	m_instances.InsertOrReplace(instanceId, pInstance);
}

instance_t* C_InstanceManager::GetCurrentInstance()
{
	int iInstancesMapIndex = m_instances.Find(g_pAnarchyManager->GetInstanceId());
	if (iInstancesMapIndex != m_instances.InvalidIndex())
		return m_instances.Element(iInstancesMapIndex);

	return null;

	// OLD STD::MAP STUFF HERE
	/*
	std::map<std::string, instance_t*>::iterator it = m_instances.find(g_pAnarchyManager->GetInstanceId());
	if (it != m_instances.end())
		return it->second;

	return null;
	*/
}

instance_t* C_InstanceManager::GetInstance(std::string id)
{
	int iInstancesMapIndex = m_instances.Find(id);
	if (iInstancesMapIndex != m_instances.InvalidIndex())
		return m_instances.Element(iInstancesMapIndex);

	return null;
	/*
	std::map<std::string, instance_t*>::iterator it = m_instances.find(id);
	if (it != m_instances.end())
		return it->second;
	
	return null;
	*/
}

instance_t* C_InstanceManager::FindInstance(std::string instanceId)
{
	instance_t* pInstance;
	for (int iInstancesMapIndex = m_instances.FirstInorder(); iInstancesMapIndex != m_instances.InvalidIndex(); iInstancesMapIndex = m_instances.NextInorder(iInstancesMapIndex))
	{
		pInstance = m_instances.Element(iInstancesMapIndex);
		if (!Q_stricmp(pInstance->id.c_str(), instanceId.c_str()))
			return pInstance;
	}

	return null;
}

void C_InstanceManager::FindAllInstances(std::string mapId, std::vector<instance_t*> &instances)
{
	instance_t* pInstance;
	for (int iInstancesMapIndex = m_instances.FirstInorder(); iInstancesMapIndex != m_instances.InvalidIndex(); iInstancesMapIndex = m_instances.NextInorder(iInstancesMapIndex))
	{
		pInstance = m_instances.Element(iInstancesMapIndex);
		if (pInstance->mapId == mapId)
			instances.push_back(pInstance);
	}
	/*
	std::map<std::string, instance_t*>::iterator it = m_instances.begin();
	while (it != m_instances.end())
	{
		//DevMsg("Map iiiiiiiiiid: %s\n", it->second->mapId.c_str());
		//if (it->second->mapId == mapId)
		//if (!Q_stricmp(it->second->mapId.c_str(), mapId.c_str()))
		if (it->second->mapId == mapId)
			instances.push_back(it->second);

		it++; 
	}
	*/
}

void C_InstanceManager::LegacyMapIdFix(std::string legacyMapName, std::string mapId)
{
	instance_t* pInstance;
	for (int iInstancesMapIndex = m_instances.FirstInorder(); iInstancesMapIndex != m_instances.InvalidIndex(); iInstancesMapIndex = m_instances.NextInorder(iInstancesMapIndex))
	{
		pInstance = m_instances.Element(iInstancesMapIndex);

		if (!Q_stricmp(pInstance->mapId.c_str(), legacyMapName.c_str()))
		{
			DevMsg("LegacyMapIdFixing: %s to %s\n", pInstance->mapId.c_str(), mapId.c_str());
			pInstance->mapId = mapId;
		}
	}

	/*
	std::map<std::string, instance_t*>::iterator it = m_instances.begin();
	while (it != m_instances.end())
	{
		//if (it->second->mapId == legacyMapName)
		if (!Q_stricmp(it->second->mapId.c_str(), legacyMapName.c_str()))
		{
			DevMsg("LegacyMapIdFixing: %s to %s\n", it->second->mapId.c_str(), mapId.c_str());
			it->second->mapId = mapId;
		}

		it++;
	}
	*/
}

// TODO: This will replace LoadLegacyInstance, because legacy instances will be consumed before loaded.
bool C_InstanceManager::ConsumeLegacyInstance(std::string instanceId, std::string filename, std::string path, std::string searchPath, std::string workshopIds, std::string mountIds, C_Backpack* pBackpack)
{
	DevMsg("Consuming legacy instance from %s%s\n", path.c_str(), filename.c_str());

	/* These fields must be determined
	pInstance->id = instanceId; (already given)
	pInstance->workshopIds = workshopIds; (already given)
	pInstance->mountIds = mountIds; (already given)
	TODO: pInstance->backpackId = mountIds; (should be already given)
	pInstance->file = file;
	pInstance->title = title;
	pInstance->style = style;
	pInstance->mapId = mapId;	(but uses legacyMapId instead, and blank if just a node.)
	*/

	// file
	std::string file = path + filename;

	// Load up the SET file, it has useful info.
	KeyValues* pLegacyInstanceKV = new KeyValues("instance");
	if (!pLegacyInstanceKV->LoadFromFile(g_pFullFileSystem, file.c_str(), searchPath.c_str()))
	{
		DevMsg("ERROR: Failed to load legacy instance for consumption: %s\n", file.c_str());
		return false;
	}

	// title
	std::string title = pLegacyInstanceKV->GetString("title");	// gen2 nodes (there were no gen1 nodes) have titles in their SET file

	// style
	bool bIsNode = false;
	std::string style = pLegacyInstanceKV->GetString("map");
	if (style.find("node_") == 0)
		bIsNode = true;
	else
		style = "";

	// mapId
	std::string legacyMapId;
	std::string goodMapName;
	if (!bIsNode)
	{
		goodMapName = filename.substr(0, filename.find("."));
		legacyMapId = goodMapName;	// map ID's can only be found after all maps have been detected.  So use a legacyMapId right now, which gets fixed when Redux detects all BSP files.
	}

	// PRAISE TOM CRUISE!  There exists a function that gets called later on that fixes all the instance ID's of shit, outside of the KeyValues.
	// The function gets called as part of "detecting all maps", which is scanning for BSP files, then adjusts the ID's of any instances that just blantantly reference the map file name (no extension).
//	this->LegacyMapIdFix()

	// Build a GEN3 instance KeyValues structure here and save it out
	KeyValues* pInstanceKV = new KeyValues("instance");

	// TODO:
	//	1. Make THESE legacy functions the STANDARD for EXPORTING legacy stuff.
	//	2. Improve/refactor the LoadLegacyInstance function (where ever it may be) to resolve these kinds of things when it loads, and it only needs to work with things w/ legacy = 1
	//	3. Remember that mapId gets updated upon map detection.

	// Note: We CANNOT assume this is a legacy GEN2 workshop item in the future.  Each GEN of workshop addon will likely need its own constumption method actually.
	g_pAnarchyManager->GetInstanceManager()->CreateBlankInstance(1, pInstanceKV, instanceId, legacyMapId, title, file, workshopIds, mountIds, style);

	KeyValues* pObjectsKV = pInstanceKV->FindKey("objects", true);
	KeyValues* pLegacyObjectsKV = pLegacyInstanceKV->FindKey("objects", true);
	
	std::string legacyItemId;
	std::string legacyModelId;
	std::string objectId;
	std::string position;
	std::string rotation;
	float scale;
	bool slave;
	bool child;
	KeyValues* pObjectKV;
	for (KeyValues *pLegacyObjectKV = pLegacyObjectsKV->GetFirstSubKey(); pLegacyObjectKV; pLegacyObjectKV = pLegacyObjectKV->GetNextKey())
	{
		legacyItemId = pLegacyObjectKV->GetString("itemfile");

		/* THIS SHOULD BE DONE UPON LOADING THE INSTANCE NOW, when legacy = 1 is detected in its KV!
		itemId = g_pAnarchyManager->ExtractLegacyId(pLegacyObjectKV->GetString("itemfile"));
		if (itemId == "")
		{
			// don't generate item id's for legacy prop objects
			testBuf = pLegacyObjectKV->GetString("itemfile");
			size_t foundExt = testBuf.find(".mdl");
			if (foundExt != testBuf.length() - 4)
			{
				KeyValues* pSearchInfo = new KeyValues("search");
				pSearchInfo->SetString("file", pInstanceKV->GetString("itemfile"));

				KeyValues* foundActive = null;
				KeyValues* foundItem = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->FindLibraryItem(pSearchInfo));
				if (foundItem)
					itemId = foundActive->GetString("info/id");
				else
				{
					DevMsg("Instance failed to resolve item with itemfile: %s\n", pInstanceKV->GetString("itemfile"));
					itemId = g_pAnarchyManager->GenerateLegacyHash(pInstanceKV->GetString("itemfile"));
				}
			}
		}
		*/

		legacyModelId = pLegacyObjectKV->GetString("model");

		/* This should be handled just like itemId, and resolved with nearly the same logic
		modelId = g_pAnarchyManager->GenerateLegacyHash(pLegacyObjectKV->GetString("model"));
		*/

		scale = pLegacyObjectKV->GetFloat("scale", 1.0f);
		slave = (pLegacyObjectKV->GetInt("slave") > 0);
		child = pLegacyObjectKV->GetBool("isChild");

		objectId = g_pAnarchyManager->GenerateUniqueId();

		position = pLegacyObjectKV->GetString("origin", "0 0 0");
		rotation = pLegacyObjectKV->GetString("angles", "0 0 0");

		// create an object in the KV
		KeyValues* objectKV = pObjectsKV->FindKey(objectId.c_str(), true);
		g_pAnarchyManager->GetInstanceManager()->CreateObject(objectKV, objectId, legacyItemId, legacyModelId, position, rotation, scale, slave, child);
	}

	// FIXME: This really should NOT be saved into the local library so early!!
	// Saving the workshop instance should not happen until the user makes a change to it, however, until
	// GEN2 workshop addons can be CONSUMED properly, we'll continue to instantly save workshop instances out
	// to the user library to avoid any weird systems that will become obsolete once consuming GEN2 workshop items properly is finished.
	// The only side-effect to doing it this way is that instances get saved to the user's library IMMEDIATELY upon consuming a workshop instance,
	// instead of not until the user makes changes to them, which is how it will be eventually.

	sqlite3** pDb = (pBackpack && pBackpack->GetSQLDbPointer()) ? pBackpack->GetSQLDbPointer() : null;
	g_pAnarchyManager->GetMetaverseManager()->SaveSQL(pDb, "instances", instanceId.c_str(), pInstanceKV);

	char fullFilePath[AA_MAX_STRING];
	PathTypeQuery_t pathTypeQuery;
	g_pFullFileSystem->RelativePathToFullPath(file.c_str(), searchPath.c_str(), fullFilePath, AA_MAX_STRING, FILTER_NONE, &pathTypeQuery);
	std::string fullpath = fullFilePath;

	g_pAnarchyManager->GetInstanceManager()->AddInstance(true, instanceId, legacyMapId, title, fullpath, workshopIds, mountIds, style);
	pLegacyInstanceKV->deleteThis();
	pLegacyInstanceKV = null;
	pInstanceKV->deleteThis();
	pInstanceKV = null;
}

void C_InstanceManager::ClearNodeSpace()
{
	DevMsg("And here, we gotta clear that node space buddy guy pal!\n");

	KeyValues* pNodeInfoKV = new KeyValues("node");
	if (!pNodeInfoKV->LoadFromFile(g_pFullFileSystem, "nodevolume.txt", "DEFAULT_WRITE_PATH"))
	{
		DevMsg("ERROR: Could not load nodevolume.txt!\n");
		return;
	}

	// Find the info shortcut for the node.
	KeyValues* pInstanceKV = null;
	C_PropShortcutEntity* pInfoShortcut = null;
	C_BaseEntity* pBaseEntity;
	C_PropShortcutEntity* pPropShortcutEntity;
	for (KeyValues *sub = pNodeInfoKV->FindKey("setup/objects", true)->GetFirstSubKey(); sub; sub = sub->GetNextKey())
	{
		// loop through it adding all the info to the response object.
		pBaseEntity = C_BaseEntity::Instance(sub->GetInt());
		if (!pBaseEntity)
			continue;

		pPropShortcutEntity = dynamic_cast<C_PropShortcutEntity*>(pBaseEntity);
		if (!pPropShortcutEntity)
			continue;

		KeyValues* pItemKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(pPropShortcutEntity->GetItemId()));
		if (pItemKV)
		{
			KeyValues* pTypeKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->GetLibraryType(pItemKV->GetString("type")));
			if (pTypeKV)
			{
				if (!Q_strcmp(pTypeKV->GetString("info/id"), g_pAnarchyManager->GetMetaverseManager()->GetSpecialTypeId("node").c_str()))
				{
					pInfoShortcut = pPropShortcutEntity;
					break;
				}
			}
		}
	}

	KeyValues* pNodeInfoSetupKV = pNodeInfoKV->FindKey("setup", true);

	KeyValues* pNodeObjectsKV = pNodeInfoSetupKV->FindKey("objects", true);
	for (KeyValues *sub = pNodeObjectsKV->GetFirstSubKey(); sub; sub = sub->GetNextKey())
	{
		// loop through it adding all the info to the response object.
		pBaseEntity = C_BaseEntity::Instance(sub->GetInt());
		if (!pBaseEntity)
			continue;

		pPropShortcutEntity = dynamic_cast<C_PropShortcutEntity*>(pBaseEntity);
		if (!pPropShortcutEntity)
			continue;

		if (pInfoShortcut && (pPropShortcutEntity == pInfoShortcut || pPropShortcutEntity->GetMoveParent() == pInfoShortcut->GetBaseEntity()))
			continue;

		this->RemoveEntity(pPropShortcutEntity, true);
	}

	if (pInfoShortcut)
		this->RemoveEntity(pInfoShortcut, true);

	// finished bulk removing...
	int iInfoShortcutIndex = (pInfoShortcut) ? pInfoShortcut->entindex() : -1;
	this->SaveActiveInstance();
	engine->ServerCmd(VarArgs("bulkremovedobjects %i;\n", iInfoShortcutIndex), false);
}

void C_InstanceManager::CreateNewNode(std::string nodeName)
{
	KeyValues* pNodeInfoKV = new KeyValues("node");
	if (!pNodeInfoKV->LoadFromFile(g_pFullFileSystem, "nodevolume.txt", "DEFAULT_WRITE_PATH"))
	{
		DevMsg("ERROR: Could not load nodevolume.txt!\n");
		return;
	}

	// Find the info shortcut for the node.
	KeyValues* pNodeInfoSetupKV = pNodeInfoKV->FindKey("setup");

	std::string nodeTypeId = g_pAnarchyManager->GetMetaverseManager()->GetSpecialTypeId("node");
	std::string nodeModelId = g_pAnarchyManager->GetMetaverseManager()->GetSpecialModelId("node");// "models/cabinets/node.mdl";
	std::string itemId = g_pAnarchyManager->GenerateUniqueId();

	std::string title = nodeName;
	std::string description = "";
	std::string file = itemId;
	std::string type = nodeTypeId;
	std::string app = "";
	std::string reference = "";
	std::string preview = "";
	std::string download = "";
	std::string stream = "";
	std::string screen = "";
	std::string marquee = "";
	std::string model = nodeModelId;

	KeyValues* pItemKV = new KeyValues("item");
	bool bCreated = g_pAnarchyManager->GetMetaverseManager()->CreateItem(0, itemId, pItemKV, title, description, file, type, app, reference, preview, download, stream, screen, marquee, model);
	if (pItemKV)
	{
		// push this onto the active library & save it
		g_pAnarchyManager->GetMetaverseManager()->AddItem(pItemKV);
		g_pAnarchyManager->GetMetaverseManager()->SaveItem(pItemKV);

		// Remember this itemID so that the item can identify itself while its spawning as a "currently building" node item.
		m_incomingNodeId = itemId;

		// Add this object to the instance
		Vector origin;
		UTIL_StringToVector(origin.Base(), pNodeInfoSetupKV->GetString("origin", "0 0 0"));

		QAngle angles;
		UTIL_StringToVector(angles.Base(), pNodeInfoSetupKV->GetString("angles", "0 0 0"));

		//std::string modelId = g_pAnarchyManager->GenerateLegacyHash("models/cabinets/two_player_arcade.mdl");
		object_t* pObject = this->AddObject("", itemId, nodeModelId, origin, angles, 1.0f, false);
		this->SpawnNearestObject();
		//object_t* pObject = this->AddObject("", itemId, modelId, origin, angles, 1.0f, false, 0, 0, 0, "", 0, "", true);
		//g_pAnarchyManager->GetMetaverseManager()->SetSpawningObject(pObject);
		//this->SpawnObject(pObject);

		// Spawn the item. At THAT point, the node will parent all children to itself (instead of spawning its children) and CREATE the node instance (instead of LOADING the node instance).
		// TODO: work this->ApplyChanges([the_entity]);

		// we are finished from here.  the rest of the logic happens as the node item spawns itself in.
	}
}

//#include "../../../public/engine/ivdebugoverlay.h"
#include "../../../game/shared/debugoverlay_shared.h"
void C_InstanceManager::SpawnActionPressed()
{
	if (!g_pAnarchyManager->IsInitialized())
		return;

	std::string loadingScreenshotId = g_pAnarchyManager->GetMetaverseManager()->GetLoadingScreenshotId();
	//if ( this->GetInstanceObjectCount() > 0)
	//{
	std::string screenshotPostfix = (loadingScreenshotId != "") ? "&screenshot=" + loadingScreenshotId : "";
	std::string uri = "asset://ui/spawnItems.html?max=" + std::string("99999999999.9") + screenshotPostfix;

	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	g_pAnarchyManager->GetAwesomiumBrowserManager()->SelectAwesomiumBrowserInstance(pHudBrowserInstance);
	pHudBrowserInstance->SetUrl(uri);
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, false, pHudBrowserInstance);
	return;

	// end of shit

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
						KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(item);
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

void C_InstanceManager::ChangeModel(C_BaseEntity* pEntity, std::string modelId, std::string in_modelFile, bool bMakeGhost)
{
	std::string modelFile = in_modelFile;
	if (!g_pFullFileSystem->FileExists(modelFile.c_str()))
		modelFile = "models/icons/missing.mdl";

	//DevMsg("Here the model id is: %s\n", modelId.c_str());

	m_pRecentModelIdConVar->SetValue(modelId.c_str());

	int iMakeGhost = (bMakeGhost) ? 1 : 0;
	engine->ServerCmd(VarArgs("switchmodel \"%s\" \"%s\" %i %i;\n", modelId.c_str(), modelFile.c_str(), pEntity->entindex(), iMakeGhost));
}

void C_InstanceManager::RemoveInstance(instance_t* pInstance)
{
	int it = m_instances.Find(pInstance->id);
	if (it != m_instances.InvalidIndex())
		m_instances.RemoveAt(it);

	delete pInstance;
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
	m_iUnspawnedWithinRangeEstimate = 0;

	// clear out objects and unspanwed objects
	auto it = m_objects.begin();
	while (it != m_objects.end())
	{
		delete it->second;
		it++;
	}

	m_objects.clear();
	m_unspawnedObjects.clear();

	if (m_pInstanceKV)
	{
		m_pInstanceKV->deleteThis();
		m_pInstanceKV = null;
	}

	m_incomingNodeId = "";
}

void C_InstanceManager::Update()
{
	return;	// FIXME: Re-enable later.  Disabled for stablity of public beta.

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
						KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(kv);
						NDebugOverlay::Text(pTestObject->origin, active->GetString("title"), true, fDuration);
						break;
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

// TODO: Generalize this function so that nodes can use it to spawn in their objects. :)
/*
	x. read this method carefully and re-learn its logic.
	x. re-read this method again, mentally separating the logic that is required for loading an instance vs loading a node.
	x. re-read this method again, figuring in the logic that would follow the calls to this method in the TWO cases it is used from.
	x. make an algo for what the new version of this method will actually do, including any other sub-routines it'll be broken up into.
	5. dowork.
*/
void C_InstanceManager::LoadInstance(C_PropShortcutEntity* pParentNodeEntity, std::string instanceId, std::string position, std::string rotation)
{
	// Legacy saves un-legacy themselves after they are loaded for the 1st time!!
	// Legacy saves are detected, converted, and saved, all in 1 swoop when being loaded for use.

	DevMsg("Load the instance!!!\n");
	instance_t* pInstance = this->GetInstance(instanceId);
	
	DevMsg("Attempting to load instance w/ ID: %s\n", instanceId.c_str());

	KeyValues* instanceKV = new KeyValues("instance");
	if ( !g_pAnarchyManager->GetMetaverseManager()->LoadSQLKevValues("instances", pInstance->id.c_str(), instanceKV))
	{
		// if this wasn't in our library, try other librarys.
		// check all backpacks...
		C_Backpack* pBackpack = g_pAnarchyManager->GetBackpackManager()->FindBackpackWithInstanceId(pInstance->id);
		if (pBackpack)
		{
			// we found the backpack containing this instance ID
			DevMsg("Loading from instance backpack w/ ID %s...\n", pBackpack->GetId().c_str());
			pBackpack->OpenDb();
			sqlite3* pDb = pBackpack->GetSQLDb();
			if (!pDb || !g_pAnarchyManager->GetMetaverseManager()->LoadSQLKevValues("instances", pInstance->id.c_str(), instanceKV, pDb))
			{
				DevMsg("CRITICAL ERROR: Failed to load instance from library!\n");
				pBackpack->CloseDb();
				pBackpack = null;
			}
			else
				pBackpack->CloseDb();
		}

		//instanceKV->deleteThis();

		if ( !pBackpack )
		{
			DevMsg("WARNING: Could not load instance!");// Attempting to load as legacy instance...\n");
			instanceKV->deleteThis();
			instanceKV = null;

			//if (pInstance->file != "")
				//this->LoadLegacyInstance(instanceId, instanceKV);

			//m_pInstanceKV = instanceKV;	// FIXME: if this fails to load the instance, the KeyValues instanceKV is NOT deleted or cleaned up anywhere.
		}
	}

	if (instanceKV)
	{
		std::string objectId;
		std::string itemId;
		std::string modelId;
		std::string testItemId;
		std::string testModelId;
		std::vector<KeyValues*> badObjectKeys;
		for (KeyValues *sub = instanceKV->FindKey("objects", true)->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		{
			objectId = sub->GetString("local/info/id");

			if (objectId == "")
			{
				badObjectKeys.push_back(sub);
				continue;
			}

			// if this is a node object, each object needs a unique id.
			if (pParentNodeEntity)
				objectId = g_pAnarchyManager->GenerateUniqueId();

			itemId = sub->GetString("local/item");
			if (pInstance->legacy)
			{
				testItemId = g_pAnarchyManager->ExtractLegacyId(itemId);

				if (testItemId == "")
				{
					// don't generate item id's for legacy prop objects
					testItemId = itemId;
					size_t foundExt = testItemId.find(".mdl");
					if (foundExt != testItemId.length() - 4)
					{
						KeyValues* pSearchInfo = new KeyValues("search");
						pSearchInfo->SetString("file", itemId.c_str());

						KeyValues* foundItem = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->FindLibraryItem(pSearchInfo));
						if (foundItem)
							testItemId = foundItem->GetString("info/id");
						else
						{
							DevMsg("Instance failed to resolve item with itemfile: %s\n", itemId.c_str());
							testItemId = g_pAnarchyManager->GenerateLegacyHash(itemId.c_str());
						}

						if (testItemId != "" && testItemId != "ffffffff" )
							itemId = testItemId;
					}
				}
				else
					itemId = testItemId;

				// write the resolved ID to the KV.  (don't forget to un-legacy tag it when saving out the instance!)
				sub->SetString("local/item", itemId.c_str());
			}

			modelId = sub->GetString("local/model");
			if (pInstance->legacy)
			{
				// model gets resolved on-the-fly for legacy saves
				//modelId = g_pAnarchyManager->GenerateLegacyHash(modelId.c_str());

				testModelId = g_pAnarchyManager->ExtractLegacyId(modelId);

				if (testModelId == "")
				{
					// don't generate model id's for legacy prop objects
					testModelId = modelId;
					size_t foundExt = testModelId.find(".mdl");
					if (foundExt == testModelId.length() - 4)
					{
						KeyValues* pSearchInfo = new KeyValues("search");
						pSearchInfo->SetString("file", modelId.c_str());

						KeyValues* foundModel = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->FindLibraryModel(pSearchInfo));
						if (foundModel)
							testModelId = foundModel->GetString("info/id");
						else
						{
							// If no model exists for this MDL, but this MDL DOES exist, then create an MDL entry for it!

							// TODO: DO THIS SHIT NEXT TIME!! AS DESCRIBED ABOVE!!!!!!!!!!!!!!!!
							DevMsg("Instance failed to resolve model with modelfile: %s\n", modelId.c_str());
							testModelId = g_pAnarchyManager->GenerateLegacyHash(modelId.c_str());
						}

						if (testModelId != "" && testModelId != "ffffffff")
							modelId = testModelId;
					}
				}
				else
					modelId = testModelId;

				// write the resolved ID to the KV.  (don't forget to un-legacy tag it when saving out the instance!)
				sub->SetString("local/model", modelId.c_str());
			}

			// alright, spawn this object
			Vector origin;
			UTIL_StringToVector(origin.Base(), sub->GetString("local/position", "0 0 0"));	// FIXME: TODO: Failing to properly read a position at this point might indicate a broken item KV.  should try and detect it instead of having a fallback of "0 0 0"

			QAngle angles;
			UTIL_StringToVector(angles.Base(), sub->GetString("local/rotation", "0 0 0"));

			float scale = sub->GetFloat("local/scale", 1.0f);

			// DO MAGIC:
			// Here's a couple of possible ways to relatively attach to the parent.
			// Keep in mind, Legacy had to use a weird way to make it work, so the obvious way is likely going to fail.
			// A: Attach the entity to the parent with a magic built-in Source engine attach method that takes a relative offset. (Maybe a UTIL.)
			// B: Zero-out the parentEntity's matrix (except use 1.0 for scale) and attach the child objects to the parentEntity,
			//		then restore the parentEntity's original matrix.  (This is how Legacy spawned nodes.)

			// FIXME: TODO: OBSOLETE: this isn't needed after the exporter in legacy is fixed!
			if (scale == 0)
				scale = 1.0f;

			bool slave = (sub->GetInt("local/slave") > 0);
			bool child = sub->GetBool("local/child");
			//bool physics = (sub->GetInt("slave") > 0); // FIXME: TODO: Use the physics stuff too!


			int iParentEntityIndex = (pParentNodeEntity) ? pParentNodeEntity->entindex() : -1;
			//DevMsg("IDs here are: %s %s %s\n", objectId.c_str(), itemId.c_str(), modelId.c_str());
			this->AddObject(objectId, itemId, modelId, origin, angles, scale, slave, 0, "", 0, "", 0, "", child, iParentEntityIndex);
		}

		unsigned int max = badObjectKeys.size();
		KeyValues* pObjectsKey = instanceKV->FindKey("objects", true);
		for (unsigned int i = 0; i < max; i++)
			pObjectsKey->RemoveSubKey(badObjectKeys[i]);

		if (max > 0)
			badObjectKeys.clear();

		if (!pParentNodeEntity)
		{
			m_pInstanceKV = instanceKV;

			// check if we should teleport
			if (position != "" && rotation != "")
			{
				C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
				engine->ServerCmd(VarArgs("teleport_player %i %s %s\n", pPlayer->entindex(), position.c_str(), rotation.c_str()), true);
			}


			std::string loadingScreenshotId = g_pAnarchyManager->GetMetaverseManager()->GetLoadingScreenshotId();
			//if ( this->GetInstanceObjectCount() > 0)
			//{
			std::string screenshotPostfix = (loadingScreenshotId != "") ? "&screenshot=" + loadingScreenshotId : "";
			std::string uri = "asset://ui/spawnItems.html?max=" + std::string("99999999999.9") + screenshotPostfix;

			C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
			g_pAnarchyManager->GetAwesomiumBrowserManager()->SelectAwesomiumBrowserInstance(pHudBrowserInstance);
			pHudBrowserInstance->SetUrl(uri);
			//g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, false, pHudBrowserInstance);
			//}
			//else
			//	g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);

			g_pAnarchyManager->GetMetaverseManager()->SetLoadingScreenshotId("");
		}
	}
}

void C_InstanceManager::LoadLegacyInstance(std::string instanceId, KeyValues* instanceKV)
{
	instance_t* pInstance = this->GetInstance(instanceId);
	DevMsg("Load the LEGACYinstance!!! %s\n", pInstance->file.c_str());
	
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
		DevMsg("Creating KV for changes to this instance.\n");

		//KeyValues* kv = new KeyValues("instance");
		instanceKV->SetInt("generation", 3);

		instance_t* pInstance = this->GetInstance(g_pAnarchyManager->GetInstanceId());

		KeyValues* info = instanceKV->FindKey("info", true);
		info->SetString("id", pInstance->id.c_str());
		info->SetString("title", pInstance->title.c_str());
		info->SetString("map", pInstance->mapId.c_str());
		info->SetString("style", pInstance->style.c_str());	// for nodes
		//info->SetString("created", "0");	// store unsigned ints as strings, then %llu them
		info->SetString("owner", "local");


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
					KeyValues* foundItem = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->FindLibraryItem(pSearchInfo));
					if (foundItem)
						itemId = foundActive->GetString("info/id");
					else
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
			bool child = sub->GetBool("isChild");

			object_t* pObject = this->AddObject("", itemId, modelId, origin, angles, scale, slave, 0, "", 0, "", 0, "", child);	// isChild cuz this is a LEGACY instance that is BEING imported, so it's using gen 2 shit.

			// create an object in the KV
			KeyValues* objectKV = instanceKV->FindKey(VarArgs("objects/%s", pObject->objectId.c_str()), true);
			objectKV->SetString("local/info/id", pObject->objectId.c_str());
			objectKV->SetString("local/item", pObject->itemId.c_str());
			objectKV->SetString("local/model", pObject->modelId.c_str());
			objectKV->SetString("local/position", sub->GetString("origin", "0 0 0"));
			objectKV->SetString("local/rotation", sub->GetString("angles", "0 0 0"));
			objectKV->SetFloat("local/scale", pObject->scale);
			objectKV->SetInt("local/slave", pObject->slave);
			objectKV->SetInt("local/child", pObject->child);
						
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
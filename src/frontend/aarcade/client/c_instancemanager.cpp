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
}

C_InstanceManager::~C_InstanceManager()
{
	DevMsg("ShortcutManager: Destructor\n");
	m_instances.clear();
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

void C_InstanceManager::ApplyChanges(std::string id, C_PropShortcutEntity* pShortcut)
{
	if (!m_pInstanceKV)
	{
		//DevMsg("WARNING: This is the 1st time this instance has been modified by the local user.  It's KeyValues structure must be generated first, before changes can be saved.\n");

		DevMsg("ERROR: No KEY for this instance!\n");
		return;
	}

	KeyValues* pObjectKV = m_pInstanceKV->FindKey(VarArgs("objects/%s", id.c_str()));
	if (!pObjectKV)
	{
		//KeyValues* pInstanceObjectsKV = m_pInstanceKV->FindKey(VarArgs("objects/%s", id.c_str()), true);
		pObjectKV = m_pInstanceKV->FindKey(VarArgs("objects/%s", id.c_str()), true);//pInstanceObjectsKV->FindKey(VarArgs("%s", id.c_str()), true);

		KeyValues* pObjectInfoKV = pObjectKV->FindKey("local/info", true);
		pObjectInfoKV->SetString("id", id.c_str());
		//pObjectInfoKV->SetString("style", kv->GetString("nodestyle"));	// for nodes
		//pObjectInfoKV->SetString("created", "0");
		pObjectInfoKV->SetString("creator", "local");
		//pObjectInfoKV->SetString("removed", "0");
		//pObjectInfoKV->SetString("remover", "0");
		//pObjectInfoKV->SetString("modified", "0");
		//pObjectInfoKV->SetString("modifier", "0");

		/*
		std::string modelId = this->GenerateHashX(sub->GetString("model"));

		pObjectKV->SetString("item", itemId.c_str());
		pObjectKV->SetString("model", modelId.c_str());
		pObjectKV->SetString("position", sub->GetString("origin"));
		pObjectKV->SetString("rotation", sub->GetString("angles"));

		float bufFloat = sub->GetFloat("scale", 1.0f);
		if (bufFloat != 1.0f)
			pObjectKV->SetFloat("scale", bufFloat);

		int bufInt = sub->GetInt("slave");
		if (bufInt != 0)
			pObjectKV->SetInt("slave", bufInt);

		bufInt = (sub->GetInt("nophysics") == 0) ? 1 : 0;
		if (bufInt != 0)
			pObjectKV->SetInt("physics", bufInt);

		bufInt = sub->GetInt("ischild");
		if (bufInt != 0)
			pObjectKV->SetInt("child", bufInt);
			*/
	}
	//pObjectKV->SetString("local/")
	//object
	//object_t* pObject = g_pAnarchyManager->GetInstanceManager()->AddObject("", id, modelId, tr.endpos, angles, 1.0f, false);

	//DevMsg("Object ID IS: %s\n", id.c_str());
	//DevMsg("Model ID IS: %s\n", pShortcut->GetModelId().c_str());

	// also update this object's object_t
	object_t* object = g_pAnarchyManager->GetInstanceManager()->GetInstanceObject(pShortcut->GetObjectId());
	if (!object)
		DevMsg("FATAL ERROR: This shortcut has no object data struct!\n");

	object->scale = pShortcut->GetModelScale();
	object->itemId = pShortcut->GetItemId();
	object->modelId = pShortcut->GetModelId();
	DevMsg("Aaaaand here the model ID is: %s\n", object->modelId.c_str());
	object->origin = pShortcut->GetAbsOrigin();
	object->angles = pShortcut->GetAbsAngles();
	object->slave = pShortcut->GetSlave();

	pObjectKV->SetString("local/item", object->itemId.c_str());
	pObjectKV->SetString("local/model", object->modelId.c_str());

	//Vector origin = pShortcut->GetAbsOrigin();
	//QAngle angles = pShortcut->GetAbsAngles();

	char buf[AA_MAX_STRING];
	Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", object->origin.x, object->origin.y, object->origin.z);
	pObjectKV->SetString("local/position", buf);

	Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", object->angles.x, object->angles.y, object->angles.z);
	pObjectKV->SetString("local/rotation", buf);

	pObjectKV->SetFloat("local/scale", object->scale);

	int slave = (object->slave) ? 1 : 0;
	pObjectKV->SetInt("local/slave", slave);
	////bool slave = (sub->GetInt("local/slave") > 0);	// FIXME: slave mode needs to be determined somehow
	//bool physics = (sub->GetInt("slave") > 0); // FIXME: TODO: Use the physics stuff too!

	// now save out to the SQL
	//g_pAnarchyManager->GetMetaverseManager()->SaveItem()
	g_pAnarchyManager->GetMetaverseManager()->SaveSQL("instances", m_pInstanceKV->GetString("info/id"), m_pInstanceKV);
}

void C_InstanceManager::SpawnObject(object_t* object, bool bShouldGhost)
{
	auto it = std::find(m_unspawnedObjects.begin(), m_unspawnedObjects.end(), object);
	if (it != m_unspawnedObjects.end())
		m_unspawnedObjects.erase(it);
	else
		return;
	
	object->spawned = true;	// FIXME: This really shouldn't be set to true until after it exists on the client. there should be a different state for waiting to spawn.
	std::string goodModelId = (object->modelId != "") ? object->modelId : object->itemId;

	KeyValues* model = g_pAnarchyManager->GetMetaverseManager()->GetLibraryModel(goodModelId);
	if (!model)
	{
		DevMsg("WARNING: Model not found in library with given ID! Using default cabinet instead.\n");
		goodModelId = g_pAnarchyManager->GenerateLegacyHash("models/cabinets/two_player_arcade.mdl");
		model = g_pAnarchyManager->GetMetaverseManager()->GetLibraryModel(goodModelId);
	}

	KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(model);
	std::string modelFile = active->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID));

	//if( !g_pFullFileSystem->FileExists(modelFile.c_str(), "GAME")

	int ghost = (bShouldGhost) ? 1 : 0;
	std::string msg = VarArgs("spawnshortcut \"%s\" \"%s\" \"%s\" \"%s\" %.10f %.10f %.10f %.10f %.10f %.10f %.10f %i %i\n", object->objectId.c_str(), object->itemId.c_str(), goodModelId.c_str(), modelFile.c_str(), object->origin.x, object->origin.y, object->origin.z, object->angles.x, object->angles.y, object->angles.z, object->scale, object->slave, ghost);
	engine->ServerCmd(msg.c_str(), false);
}

object_t* C_InstanceManager::AddObject(std::string objectId, std::string itemId, std::string modelId, Vector origin, QAngle angles, float scale, bool slave, unsigned int created, std::string creator, unsigned int removed, std::string remover, unsigned int modified, std::string modifier, bool isChild)
{
	std::string goodObjectId = (objectId != "") ? objectId : g_pAnarchyManager->GenerateUniqueId();
	DevMsg("Object ID here is: %s\n", goodObjectId.c_str());
	object_t* pObject = new object_t();
	pObject->objectId = goodObjectId;
	pObject->created = created;
	pObject->creator = creator;
	pObject->removed = removed;
	pObject->remover = remover;
	pObject->modified = modified;
	pObject->modifier = modifier;
	pObject->itemId = itemId;
	pObject->modelId = modelId;
	pObject->origin.Init(origin.x, origin.y, origin.z);
	pObject->angles.Init(angles.x, angles.y, angles.z);
	pObject->child = isChild;
	pObject->spawned = false;
	pObject->scale = scale;
	pObject->slave = slave;

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

void C_InstanceManager::RemoveEntity(C_PropShortcutEntity* pShortcutEntity)
{
	// FIXME: Entry should be removed from the instance KV also, or at least flag it as "removed" so deleted objects can be undone.
	KeyValues* pObjectsKV = m_pInstanceKV->FindKey("objects", true);
	if (pObjectsKV)
	{
		KeyValues* pEntryKV = pObjectsKV->FindKey(pShortcutEntity->GetObjectId().c_str());
		if (pEntryKV)
		{
			pObjectsKV->RemoveSubKey(pEntryKV);

			// save out the instance KV
			g_pAnarchyManager->GetMetaverseManager()->SaveSQL("instances", m_pInstanceKV->GetString("info/id"), m_pInstanceKV);
		}
	}

	// 1. find the object associated with this entity
	// 2. get rdy to delete the object
	// 3. remove the entity
	// 4. remove the object

	object_t* pObject;
	// FIXME: What if the object is still unspawned?  Could this ever happen?  If it could, then it'd need to be removed from the unspawned objects vector too.
	auto it = m_objects.find(pShortcutEntity->GetObjectId());
	if (it != m_objects.end())
	{
		delete it->second;
		m_objects.erase(it);
	}

	engine->ServerCmd(VarArgs("removeobject %i;\n", pShortcutEntity->entindex()), false);
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

void C_InstanceManager::AddInstance(std::string instanceId, std::string mapId, std::string title, std::string file, std::string workshopIds, std::string mountIds, std::string style)
{
	auto it = m_instances.find(instanceId);
	if (it != m_instances.end())
	{
		DevMsg("WARNING: Instance already exists with id %s, aborting.\n", instanceId.c_str());
		return;
	}

	//DevMsg("Add instance for mapid %s with title %s and with file %s\n", mapId.c_str(), title.c_str(), file.c_str());
	instance_t* pInstance = new instance_t();
	pInstance->id = instanceId;
	pInstance->mapId = mapId;
	pInstance->title = title;
	pInstance->style = style;
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
		//DevMsg("Map iiiiiiiiiid: %s\n", it->second->mapId.c_str());
		//if (it->second->mapId == mapId)
		//if (!Q_stricmp(it->second->mapId.c_str(), mapId.c_str()))
		if (it->second->mapId == mapId)
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
		{
			DevMsg("LegacyMapIdFixing: %s to %s\n", it->second->mapId.c_str(), mapId.c_str());
			it->second->mapId = mapId;
		}

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

void C_InstanceManager::ChangeModel(C_BaseEntity* pEntity, std::string modelId, std::string in_modelFile)
{
	std::string modelFile = in_modelFile;
	if (!g_pFullFileSystem->FileExists(modelFile.c_str()))
		modelFile = "models/icons/missing.mdl";

	DevMsg("Here the model id is: %s\n", modelId.c_str());

	engine->ServerCmd(VarArgs("switchmodel \"%s\" \"%s\" %i 1;\n", modelId.c_str(), modelFile.c_str(), pEntity->entindex()));
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

	if (m_pInstanceKV)
	{
		m_pInstanceKV->deleteThis();
		m_pInstanceKV = null;
	}
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

void C_InstanceManager::LoadInstance(std::string instanceId)
{
	DevMsg("Load the instance!!!\n");
	instance_t* pInstance = this->GetInstance(instanceId);
	
	DevMsg("Attempting to load instance w/ ID: %s\n", instanceId.c_str());

	KeyValues* instanceKV = new KeyValues("instance");
	//if (!instanceKV->LoadFromFile(g_pFullFileSystem, VarArgs("library\\instances\\%s.key", pInstance->id.c_str())))
	if ( !g_pAnarchyManager->GetMetaverseManager()->LoadSQLKevValues("instances", pInstance->id.c_str(), instanceKV))
	{
		//instanceKV->deleteThis();

		DevMsg("WARNING: Could not load instance! Attempting to load as legacy instance...\n");
		if (pInstance->file != "")
			this->LoadLegacyInstance(instanceId, instanceKV);	// FIXME: if this fails to load the instance, the KeyValues instanceKV is NOT deleted or cleaned up anywhere.

		m_pInstanceKV = instanceKV;
	}
	else
	{
		std::string objectId;
		std::string itemId;
		std::string modelId;
		for (KeyValues *sub = instanceKV->FindKey("objects", true)->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		{
			objectId = sub->GetString("local/info/id");
			itemId = sub->GetString("local/item");
			modelId = sub->GetString("local/model");

			// alright, spawn this object
			Vector origin;
			UTIL_StringToVector(origin.Base(), sub->GetString("local/position", "0 0 0"));

			QAngle angles;
			UTIL_StringToVector(angles.Base(), sub->GetString("local/rotation", "0 0 0"));

			float scale = sub->GetFloat("local/scale", 1.0f);

			// FIXME: TODO: OBSOLETE: this isn't needed after the exporter in legacy is fixed!
			if (scale == 0)
				scale = 1.0f;

			bool slave = (sub->GetInt("local/slave") > 0);
			bool child = sub->GetBool("local/child");
			//bool physics = (sub->GetInt("slave") > 0); // FIXME: TODO: Use the physics stuff too!


			DevMsg("IDs here are: %s\n", objectId.c_str());// , sub->GetName());
			this->AddObject(objectId, itemId, modelId, origin, angles, scale, slave, 0, "", 0, "", 0, "", child);
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
		m_pInstanceKV = instanceKV;
		//instanceKV->deleteThis();
	}

	// after the instance is loaded, spawn everything in it.
	//std::string instanceId = g_pAnarchyManager->GetInstanceId();
	//if (instanceId != "")
	//{

	if (this->GetInstanceObjectCount() > 0)
	{
		std::string uri = "asset://ui/spawnItems.html?max=" + std::string("99999999999.9");

		C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
		g_pAnarchyManager->GetAwesomiumBrowserManager()->SelectAwesomiumBrowserInstance(pHudBrowserInstance);
		pHudBrowserInstance->SetUrl(uri);
		g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, false, pHudBrowserInstance);
	}
	//}
	//this->SpawnNearestObject();
}

void C_InstanceManager::LoadLegacyInstance(std::string instanceId, KeyValues* instanceKV)
{
	DevMsg("Load the LEGACYinstance!!!\n");

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
		info->SetString("creator", "local");


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
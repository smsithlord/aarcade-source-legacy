#include "cbase.h"
#include "prop_shortcut_entity.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void AddGlowEffect(const CCommand &args)
{
	CBaseEntity* pEntity = CBaseEntity::Instance(Q_atoi(args[1]));
	if (pEntity)
		pEntity->AddGlowEffect();
}
ConCommand addgloweffect("addgloweffect", AddGlowEffect, "Adds a glow around the entity.", FCVAR_HIDDEN);

void RemoveGlowEffect(const CCommand &args)
{
	CBaseEntity* pEntity = CBaseEntity::Instance(Q_atoi(args[1]));
	if (pEntity)
		pEntity->RemoveGlowEffect();
}
ConCommand removegloweffect("removegloweffect", RemoveGlowEffect, "Removes a glow around the entity.", FCVAR_HIDDEN);

void SpawnShortcut(const CCommand &args)
{
//	DevMsg("yaarbelzzzzzzzzz\n");

	/*
	// Figure out where to place it
	CBasePlayer* pPlayer = UTIL_GetCommandClient();
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
	*/

	Vector origin(Q_atof(args[5]), Q_atof(args[6]), Q_atof(args[7]));
	QAngle angles(Q_atof(args[8]), Q_atof(args[9]), Q_atof(args[10]));

	// Now spawn it
	CPropShortcutEntity *pShortcut = dynamic_cast<CPropShortcutEntity*>(CreateEntityByName("prop_shortcut"));

	// Pass in standard key values
	char buf[512];
	// Pass in standard key values
	Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", origin.x, origin.y, origin.z);
	pShortcut->KeyValue("origin", buf);
	Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", angles.x, angles.y, angles.z);
	pShortcut->KeyValue("angles", buf);

	pShortcut->KeyValue("model", args[4]);
	pShortcut->KeyValue("solid", "6");		// for 	V_PHYSICS
	//pShortcut->KeyValue("solid", "0");
	pShortcut->KeyValue("fademindist", "-1");
	pShortcut->KeyValue("fademaxdist", "0");
	pShortcut->KeyValue("fadescale", "1");
	pShortcut->KeyValue("MinAnimTime", "5");
	pShortcut->KeyValue("MaxAnimTime", "10");
	pShortcut->KeyValue("spawnflags", "8");
	pShortcut->KeyValue("objectId", args[1]);
	pShortcut->KeyValue("itemId", args[2]);
	pShortcut->KeyValue("modelId", args[3]);
	pShortcut->KeyValue("slave", args[12]);

	pShortcut->Precache();
	DispatchSpawn(pShortcut);
	pShortcut->Activate();

	pShortcut->SetSolid(SOLID_VPHYSICS);

	pShortcut->SetModelScale(Q_atof(args[11]), 0);

	IPhysicsObject* pPhysics = pShortcut->VPhysicsGetObject();
	if (!pPhysics && pShortcut->CreateVPhysics())
		pPhysics = pShortcut->VPhysicsGetObject();

	if (pPhysics)
	{
		if (pShortcut->GetSpawnFlags() == 8)
			pPhysics->EnableMotion(false);
		else
			pPhysics->EnableMotion(true);
	}

	if (Q_atoi(args[13]) == 1)
		engine->ServerCommand(UTIL_VarArgs("makeghost %i;\n", pShortcut->entindex()));	// lazy way to make transparent & stuff

//	pShortcut->SetModelScale(Q_atof(args[9]), 0);
	/*	// from server-side code...
	float flScale = Q_atof(args[2]);
	float flMin = Q_atof(args[3]);
	float flMax = Q_atof(args[4]);

	CBaseEntity* pEntity = CBaseEntity::Instance(Q_atoi(args[1]));
	if (pEntity)
	{
		C_PropHotlinkEntity* pHotlink = dynamic_cast<CPropHotlinkEntity*>(pEntity);
		pHotlink->SetModelScale(flScale, 0.0f);
		*/
}
ConCommand spawnshortcut("spawnshortcut", SpawnShortcut, "Spawns a shortcut.", FCVAR_NONE);

void SpawnInstance(const CCommand &args)
{
	DevMsg("Spawn Instance: %s\n", args[1]);
}
ConCommand spawninstance("spawninstance", SpawnInstance, "Spawns an entire instance.  Many shortcuts.", FCVAR_NONE);

/*
// WE are given: [1]itemfile [2]model [3]origin [4]angles [5]nophysics [6]allowplacement
void Create_Hotlink(const CCommand &args)
{
	// arg 1: item
	// arg 2: model
	bool bAbsoluteLocation = false;

	if (args.ArgC() > 3)
	{
		bAbsoluteLocation = true;
	}

	char modelFile[256];
	if (args.ArgC() > 2 && Q_strcmp(args.Arg(2), ""))
		strcpy(modelFile, args.Arg(2));
	else if (args.ArgC() > 1 && Q_strcmp(args.Arg(1), ""))
		strcpy(modelFile, args.Arg(1));
	else
		strcpy(modelFile, "models/cabinets/two_player_arcade.mdl");

	if (!g_pFullFileSystem->FileExists(modelFile))
		strcpy(modelFile, "models/icons/missing.mdl");

	if (!engine->IsModelPrecached(modelFile))
		engine->PrecacheModel(modelFile, true);

	if (!bAbsoluteLocation)
	{
		// Figure out where to place it
		CBasePlayer* pPlayer = UTIL_GetCommandClient();
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

		// Now spawn it
		CPropHotlinkEntity *pProp = dynamic_cast<CPropHotlinkEntity*>(CreateEntityByName("prop_hotlink"));

		CArcadeResources* pServerArcadeResources = CArcadeResources::GetSelf();

		// Pass in standard key values
		char buf[512];
		// Pass in standard key values
		Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", tr.endpos.x, tr.endpos.y, tr.endpos.z);
		pProp->KeyValue("origin", buf);
		Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", angles.x, angles.y, angles.z);
		pProp->KeyValue("angles", buf);

		pProp->KeyValue("model", modelFile);
		//pProp->KeyValue("solid", "6");		// for 	V_PHYSICS
		pProp->KeyValue("solid", "0");
		pProp->KeyValue("fademindist", "-1");
		pProp->KeyValue("fademaxdist", "0");
		pProp->KeyValue("fadescale", "1");
		pProp->KeyValue("MinAnimTime", "5");
		pProp->KeyValue("MaxAnimTime", "10");
		pProp->KeyValue("spawnflags", "8");
		pProp->KeyValue("IndexOfKV", UTIL_VarArgs("%i", pServerArcadeResources->GetHotlinkCount()));

		DispatchSpawn(pProp);

		pServerArcadeResources->IncrementHotlinkCount();
	}
	else
	{
		// Now spawn it
		CPropHotlinkEntity *pProp = dynamic_cast<CPropHotlinkEntity*>(CreateEntityByName("prop_hotlink"));

		CArcadeResources* pServerArcadeResources = CArcadeResources::GetSelf();

		// Pass in standard key values
		pProp->KeyValue("origin", args[3]);
		pProp->KeyValue("angles", args[4]);
		pProp->KeyValue("model", modelFile);
		//		pProp->KeyValue("solid", "6");		// for 	V_PHYSICS
		pProp->KeyValue("solid", "0");
		pProp->KeyValue("fademindist", "-1");
		pProp->KeyValue("fademaxdist", "0");
		pProp->KeyValue("fadescale", "1");
		pProp->KeyValue("MinAnimTime", "5");
		pProp->KeyValue("MaxAnimTime", "10");

		//pProp->KeyValue("spawnflags", "8");

		if (Q_atoi(args[5]) == 1)
			pProp->KeyValue("spawnflags", "8");

		pProp->KeyValue("IndexOfKV", UTIL_VarArgs("%i", pServerArcadeResources->GetHotlinkCount()));

		DispatchSpawn(pProp);

		pServerArcadeResources->IncrementHotlinkCount();

		if (Q_atoi(args[6]) == 0)
		{
			pProp->VPhysicsInitNormal(SOLID_VPHYSICS, 0, false);

			IPhysicsObject* pPhysics = pProp->VPhysicsGetObject();
			if (pPhysics)
			{
				if (pProp->GetSpawnFlags() == 8)
					pPhysics->EnableMotion(false);
				else
					pPhysics->EnableMotion(true);
			}

			//pProp->SetCollisionGroup(COLLISION_GROUP_NONE);

			//////////////////
			pProp->SetSolid(SOLID_VPHYSICS);

			IPhysicsObject* pPhysics = pProp->VPhysicsGetObject();
			if (!pPhysics && pProp->CreateVPhysics())
			pPhysics = pProp->VPhysicsGetObject();

			if (pPhysics)
			{
			if (pProp->GetSpawnFlags() == 8)
			pPhysics->EnableMotion(false);
			else
			pPhysics->EnableMotion(true);
			}
			/////////////////////////
		}
	}

	//	if( args.ArgC() > 2 )
	//	{
	//		edict_t *pClient = engine->PEntityOfEntIndex( pPlayer->entindex() );
	//		engine->ClientCommand( pClient, UTIL_VarArgs( "new_item \"%s\" \"%s\"\n", args.Arg(1), args.Arg(2) ) );
	//	}
	//	else if( args.ArgC() > 1 )
	//	{
	//		edict_t *pClient = engine->PEntityOfEntIndex( pPlayer->entindex() );
	//		engine->ClientCommand( pClient, UTIL_VarArgs( "new_item \"%s\"\n", args.Arg(1) ) );
	//	}
}

ConCommand prop_hotlink_create("prop_hotlink_create", Create_Hotlink, "Create a dynamic item under your crosshairs.");
*/

void SetCabPos(const CCommand &args)
{
	int TheEntity = Q_atoi(args.Arg(1));

	edict_t *pEdict = INDEXENT(TheEntity);
	if (pEdict && !pEdict->IsFree())
	{
		CBaseEntity* pEntity = GetContainingEntity(pEdict);
		//CPropHotlinkEntity* pEntity = (CPropHotlinkEntity*)GetContainingEntity(pHotlinkEdict);
		Vector origin = Vector(Q_atof(args.Arg(2)), Q_atof(args.Arg(3)), Q_atof(args.Arg(4)));
		QAngle angles = QAngle(Q_atof(args.Arg(5)), Q_atof(args.Arg(6)), Q_atof(args.Arg(7)));

		UTIL_SetOrigin(pEntity, origin, true);
		pEntity->SetAbsAngles(angles);

		Vector vel = Vector(0, 0, 0);
		pEntity->Teleport(&origin, &angles, &vel);
	}
}

ConCommand setcabpos("setcabpos", SetCabPos, "For internal use only.");

void SwitchModel(const CCommand &args)
{
	const char* modelId = args[1];
	const char *TheModel = args[2];
	int TheEntity = Q_atoi(args.Arg(3));

	edict_t *pHotlinkEdict = INDEXENT(TheEntity);
	if (pHotlinkEdict && !pHotlinkEdict->IsFree())
	{
		//CPropHotlinkEntity* pHotlink = (CPropHotlinkEntity*)GetContainingEntity(pHotlinkEdict);
		CBaseEntity* pEntity = GetContainingEntity(pHotlinkEdict);
		CPropShortcutEntity* pHotlink = dynamic_cast<CPropShortcutEntity*>(pEntity);

		//CDynamicProp* pEntity = dynamic_cast<CDynamicProp*>(pHotlink);

		if (!engine->IsModelPrecached(TheModel))
		{
			int result = pEntity->PrecacheModel(TheModel);
			DevMsg("Cache result for %s is: %i\n\n\n", TheModel, result);

			//			IMaterial* pMaterial;
			//		modelinfo->GetModelMaterials(modelinfo->FindOrLoadModel(TheModel), 1, &pMaterial);
		}

		if (pHotlink)
			pHotlink->SetModelId(std::string(modelId));

		UTIL_SetModel(pEntity, TheModel);
		pEntity->SetModel(TheModel);

		if (args.ArgC() > 4 && Q_atoi(args[4]) == 1)
		{
			pEntity->SetSolid(SOLID_NONE);
			pEntity->SetSize(-Vector(100, 100, 100), Vector(100, 100, 100));
			//SetRenderMode(kRenderTransTexture);
			pEntity->SetRenderMode(kRenderTransColor);
			pEntity->SetRenderColorA(160);
		}

		pEntity->NetworkStateChanged();
	}
}

ConCommand switchmodel("switchmodel", SwitchModel, "For internal use only.");

/*
void SwitchModel(const CCommand &args)
{
	const char *TheModel = args[1];
	int TheEntity = Q_atoi(args.Arg(2));
	std::string itemValue = args.Arg(3);
	int itemValueLength = itemValue.length();
	char bufItemValue[1024];
	Q_strcpy(bufItemValue, itemValue.c_str());


	CURL* easyhandle = curl_easy_init();
	std::string commands = curl_easy_escape(easyhandle, bufItemValue, itemValueLength);
	curl_easy_cleanup(easyhandle);


	edict_t *pHotlinkEdict = INDEXENT(TheEntity);
	if (pHotlinkEdict && !pHotlinkEdict->IsFree())
	{
		CPropHotlinkEntity* pHotlink = (CPropHotlinkEntity*)GetContainingEntity(pHotlinkEdict);

		//CDynamicProp* pEntity = dynamic_cast<CDynamicProp*>(pHotlink);

		if (!engine->IsModelPrecached(TheModel))
		{
			int result = pHotlink->PrecacheModel(TheModel);
			DevMsg("Cache result for %s is: %i\n\n\n", TheModel, result);

			//			IMaterial* pMaterial;
			//		modelinfo->GetModelMaterials(modelinfo->FindOrLoadModel(TheModel), 1, &pMaterial);
		}

		ConVar* pMPModeVar = cvar->FindVar("mp_mode");

		if (!pMPModeVar->GetBool())
		{
			pHotlink->CleanupCameraEntities();
			pHotlink->CleanupSequenceEntities();
		}

		UTIL_SetModel(pHotlink, TheModel);
		pHotlink->SetModel(TheModel);

		//pHotlink->ResetSequenceInfo();

		std::string realSequenceName = "activated";
		int index = pHotlink->LookupSequence(realSequenceName.c_str());

		if (index == ACT_INVALID)
		{
			realSequenceName = "activeidle";
			index = pHotlink->LookupSequence(realSequenceName.c_str());
		}

		if (index != ACT_INVALID)
		{
			CBasePlayer* pRequestingPlayer = UTIL_GetCommandClient();

			if (!pRequestingPlayer)
				return;

			edict_t *pClient = engine->PEntityOfEntIndex(pRequestingPlayer->entindex());

			pHotlink->ResetSequenceInfo();

			//engine->ClientCommand(pClient, "nextsequenceready %i \"activated\" 1\n", pHotlink->entindex());
			engine->ClientCommand(pClient, "nextsequenceready %i \"%s\" 1;\n", pHotlink->entindex(), realSequenceName.c_str());
		}
	}
}

ConCommand switchmodel("switchmodel", SwitchModel, "For internal use only.");
*/






/*
void SetCabPos(const CCommand &args)
{
	int TheEntity = Q_atoi(args.Arg(1));

	edict_t *pHotlinkEdict = INDEXENT(TheEntity);
	if (pHotlinkEdict && !pHotlinkEdict->IsFree())
	{
		CPropHotlinkEntity* pEntity = (CPropHotlinkEntity*)GetContainingEntity(pHotlinkEdict);
		Vector origin = Vector(Q_atof(args.Arg(2)), Q_atof(args.Arg(3)), Q_atof(args.Arg(4)));
		QAngle angles = QAngle(Q_atof(args.Arg(5)), Q_atof(args.Arg(6)), Q_atof(args.Arg(7)));

		CArcadeResources* pArcadeResources = CArcadeResources::GetSelf();

		// Cycle through all trigger_multiple and test if our point is inside of them...
		CTriggerMultiple* pTriggerMultiple;
		CNodeInfoEntity* pNodeInfo;

		// Check if we have a most-likely trigger to test first...
		CBaseEntity* pBaseMostRecentTrigger = pArcadeResources->GetMostRecentHubTrigger();
		CBaseEntity* pBaseMostRecentNodeInfo = pArcadeResources->GetMostRecentHubNodeInfo();
		if (pBaseMostRecentTrigger)
		{
			// If we have a most recent trigger, then test against it.  If we are, in fact, inside of it, then just return.
			pTriggerMultiple = dynamic_cast<CTriggerMultiple*>(pBaseMostRecentTrigger);
			if (pTriggerMultiple != NULL && pTriggerMultiple->GetEnabled() && pTriggerMultiple->PointIsWithin(Vector(Q_atof(args.Arg(2)), Q_atof(args.Arg(3)), Q_atof(args.Arg(4)))))
			{
				pNodeInfo = dynamic_cast<CNodeInfoEntity*>(pBaseMostRecentNodeInfo);

				if (pNodeInfo && pEntity->GetAbsOrigin() == pNodeInfo->GetAbsOrigin())
					return;
			}
			else
			{
				// If we aren't in that volume anymore, clear it.
				pArcadeResources->SetMostRecentHubTrigger(NULL);
				pArcadeResources->SetMostRecentHubNodeInfo(NULL);
			}

			// Otherwise, fall through.
		}

		bool bVolumeOccupied = false;	// We don't want to spawn 2 hubs in the same volume!
		bool bVolumeIsMatch = false;
		bool bMatchIsLearnVolume = false;

		std::string nameBuf;

		CBaseEntity* pBaseTriggerMultiple = gEntList.FindEntityByClassname(gEntList.FirstEnt(), "trigger_multiple");
		while (pBaseTriggerMultiple)
		{
			pTriggerMultiple = dynamic_cast<CTriggerMultiple*>(pBaseTriggerMultiple);
			if (pTriggerMultiple != NULL && pTriggerMultiple->GetEnabled() && pTriggerMultiple->PointIsWithin(Vector(Q_atof(args.Arg(2)), Q_atof(args.Arg(3)), Q_atof(args.Arg(4)))))
			{
				// Check if the name of this entity is the match to our nodestyle.
				nameBuf = UTIL_VarArgs("%s", pTriggerMultiple->GetEntityName());

				if (!Q_stricmp(UTIL_VarArgs("%s", pTriggerMultiple->GetEntityName()), args.Arg(8)) || nameBuf.find("snap") == 0 || nameBuf.find("autospawn") != std::string::npos)
				{
					// Cycle through all node_info and test if they are inside of this trigger.
					CBaseEntity* pBaseNodeInfo = gEntList.FindEntityByClassname(gEntList.FirstEnt(), "node_info");
					while (pBaseNodeInfo)
					{
						pNodeInfo = dynamic_cast<CNodeInfoEntity*>(pBaseNodeInfo);
						//if (pNodeInfo && pTriggerMultiple->IsTouching(pNodeInfo))
						if (pNodeInfo && pTriggerMultiple->PointIsWithin(pNodeInfo->GetAbsOrigin()))
						{
							if (nameBuf.find("autospawn") != std::string::npos)
								bMatchIsLearnVolume = true;

							bVolumeIsMatch = true;

							// Cycle through all prop_hotlinks and test if any are at the exact same origin and angles as the NodeInfo...
							CBaseEntity* pBaseHotlink = gEntList.FindEntityByClassname(gEntList.FirstEnt(), "prop_hotlink");
							while (pBaseHotlink)
							{
								CPropHotlinkEntity* pHotlink = dynamic_cast<CPropHotlinkEntity*>(pBaseHotlink);
								//if (pHotlink && pTriggerMultiple->IsTouching(pHotlink) && pHotlink->GetAbsOrigin() == pNodeInfo->GetAbsOrigin())	// This is assuming that IsTouching is less expensive than comparing origins.
								if (pHotlink && pHotlink->GetAbsOrigin() == pNodeInfo->GetAbsOrigin())	// This is assuming that IsTouching is less expensive than comparing origins.
								{
									bVolumeOccupied = true;
									break;
								}

								pBaseHotlink = gEntList.FindEntityByClassname(pBaseHotlink, "prop_hotlink");
							}

							break;
						}

						pBaseNodeInfo = gEntList.FindEntityByClassname(pBaseNodeInfo, "node_info");
					}

					if (bVolumeIsMatch)
						break;
				}
			}

			pBaseTriggerMultiple = gEntList.FindEntityByClassname(pBaseTriggerMultiple, "trigger_multiple");
		}

		if (bVolumeOccupied || !bVolumeIsMatch)
		{
			// Otherwise, proceed as usual...
			//		pEntity->SetLocalOrigin(origin);
			//			pEntity->SetAbsOrigin(origin);

			UTIL_SetOrigin(pEntity, origin, true);
			pEntity->SetAbsAngles(angles);

			Vector vel = Vector(0, 0, 0);
			pEntity->Teleport(&origin, &angles, &vel);
		}
		else
		{
			// Otherwise, do some work with pTriggerMultiple, pNodeInfo, and other stuff.
			pArcadeResources->SetMostRecentHubTrigger((CBaseEntity*)pTriggerMultiple);
			pArcadeResources->SetMostRecentHubNodeInfo((CBaseEntity*)pNodeInfo);

			UTIL_SetOrigin(pEntity, pNodeInfo->GetAbsOrigin(), true);
			pEntity->SetAbsAngles(pNodeInfo->GetAbsAngles());

			Vector vel2 = Vector(0, 0, 0);
			Vector origin2 = pNodeInfo->GetAbsOrigin();
			QAngle angle2 = pNodeInfo->GetAbsAngles();

			pEntity->Teleport(&origin2, &angle2, &vel2);

			if (bMatchIsLearnVolume)
			{
				DevMsg("SHOULD AUTO SPAWN!!\n");

				CBasePlayer* pRequestingPlayer = UTIL_GetCommandClient();

				edict_t *pClient = engine->PEntityOfEntIndex(pRequestingPlayer->entindex());
				engine->ClientCommand(pClient, "doautospawn;\n");
			}
		}
	}
}

ConCommand setcabpos("setcabpos", SetCabPos, "For internal use only.");
*/

void RemoveObject(const CCommand &args)
{
	if (args.ArgC() < 2)
		return;

	//CPropShortcutEntity
	//CDynamicProp* pProp = NULL;
	//pProp = dynamic_cast<CDynamicProp*>(CBaseEntity::Instance(Q_atoi(args[1])));
	CPropShortcutEntity* pProp = dynamic_cast<CPropShortcutEntity*>(CBaseEntity::Instance(Q_atoi(args[1])));
	if (!pProp)
	{
		DevMsg("Invalid entindex specified for \"remove\" command!\n");
		return;
	}

	inputdata_t emptyDummy;
	pProp->InputKillHierarchy(emptyDummy);
}
ConCommand removeobject("removeobject", RemoveObject, "Deletes an object from the game.");

void SetObjectIds(const CCommand &args)
{
	if (args.ArgC() < 3)
		return;

	//CPropShortcutEntity
	//CDynamicProp* pProp = NULL;
	//pProp = dynamic_cast<CDynamicProp*>(CBaseEntity::Instance(Q_atoi(args[1])));
	std::string itemId = args[2];
	std::string modelId = args[3];
	std::string modelFile = args[4];
	CPropShortcutEntity* pProp = dynamic_cast<CPropShortcutEntity*>(CBaseEntity::Instance(Q_atoi(args[1])));
	if (!pProp)
	{
		DevMsg("Invalid entindex specified for \"setobjectitemid\" command!\n");
		return;
	}

	pProp->PrecacheModel(modelFile.c_str());
	pProp->SetModel(modelFile.c_str());	// This might need to be done server-side (maybe in addition)
	// does physics need to be adjusted for the new model??
	pProp->SetItemId(itemId);
	pProp->SetModelId(modelId);

	if (args.ArgC() > 5 && Q_atoi(args[5]) == 1)
	{
		pProp->SetSolid(SOLID_NONE);
		pProp->SetSize(-Vector(100, 100, 100), Vector(100, 100, 100));
		//SetRenderMode(kRenderTransTexture);
		pProp->SetRenderMode(kRenderTransColor);
		pProp->SetRenderColorA(160);
	}
		//engine->ServerCommand(UTIL_VarArgs("makeghost %i 0;\n", pShortcut->entindex()));	// lazy way to make transparent & stuff

	pProp->NetworkStateChanged();
}
ConCommand setobjectids("setobjectids", SetObjectIds, "");

void MakeGhost(const CCommand &args)
{
	CBaseEntity* pShortcut = CBaseEntity::Instance(Q_atoi(args[1]));
	pShortcut->SetSolid(SOLID_NONE);
	pShortcut->SetSize(-Vector(100, 100, 100), Vector(100, 100, 100));
	//SetRenderMode(kRenderTransTexture);
	pShortcut->SetRenderMode(kRenderTransColor);
	pShortcut->SetRenderColorA(160);
	pShortcut->NetworkStateChanged();
}
ConCommand makeghost("makeghost", MakeGhost, "Interal use only.", FCVAR_HIDDEN);

void MakeNonGhost(const CCommand &args)
{
	CBaseEntity* pShortcut = CBaseEntity::Instance(Q_atoi(args[1]));
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
	pShortcut->NetworkStateChanged();
}
ConCommand makenonghost("makenonghost", MakeNonGhost, "Interal use only.", FCVAR_HIDDEN);

void SetScale(const CCommand &args)
{
	float flScale = Q_atof(args[2]);

	CBaseEntity* pEntity = CBaseEntity::Instance(Q_atoi(args[1]));
	if (pEntity)
	{
		CPropShortcutEntity* pShortcutEntity = dynamic_cast<CPropShortcutEntity*>(pEntity);
		pShortcutEntity->SetModelScale(flScale, 0.0f);
	}
}
ConCommand setscale("setscale", SetScale, "Interal use only.", FCVAR_HIDDEN);

void SetAngles(const CCommand &args)
{
	CBaseEntity* pEntity = CBaseEntity::Instance(Q_atoi(args[1]));
	if (pEntity)
	{
		QAngle angles;
		angles.x = Q_atoi(args[2]);
		angles.y = Q_atoi(args[3]);
		angles.z = Q_atoi(args[4]);
		pEntity->SetAbsAngles(angles);
	}
}
ConCommand setangles("setangles", SetAngles, "Internal use only.", FCVAR_HIDDEN);
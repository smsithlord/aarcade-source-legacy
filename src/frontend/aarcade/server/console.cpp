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

	Vector origin(Q_atof(args[3]), Q_atof(args[4]), Q_atof(args[5]));
	QAngle angles(Q_atof(args[6]), Q_atof(args[7]), Q_atof(args[8]));

	// Now spawn it
	CPropShortcutEntity *pShortcut = dynamic_cast<CPropShortcutEntity*>(CreateEntityByName("prop_shortcut"));

	// Pass in standard key values
	char buf[512];
	// Pass in standard key values
	Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", origin.x, origin.y, origin.z);
	pShortcut->KeyValue("origin", buf);
	Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", angles.x, angles.y, angles.z);
	pShortcut->KeyValue("angles", buf);

	pShortcut->KeyValue("model", args[2]);
	pShortcut->KeyValue("solid", "6");		// for 	V_PHYSICS
	//pShortcut->KeyValue("solid", "0");
	pShortcut->KeyValue("fademindist", "-1");
	pShortcut->KeyValue("fademaxdist", "0");
	pShortcut->KeyValue("fadescale", "1");
	pShortcut->KeyValue("MinAnimTime", "5");
	pShortcut->KeyValue("MaxAnimTime", "10");
	pShortcut->KeyValue("spawnflags", "8");
	pShortcut->KeyValue("itemId", args[1]);

	pShortcut->Precache();
	DispatchSpawn(pShortcut);
	pShortcut->Activate();

	pShortcut->SetSolid(SOLID_VPHYSICS);

	pShortcut->SetModelScale(Q_atof(args[9]), 0);

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
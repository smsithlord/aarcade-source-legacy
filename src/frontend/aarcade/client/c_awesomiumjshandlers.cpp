#include "cbase.h"

#include "c_awesomiumjshandlers.h"
#include "c_anarchymanager.h"
#include <algorithm>
//#include "aa_globals.h"
//#include "Filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

JSHandler::JSHandler()
{
	//m_pPreviousLibraryItemIt = null;
}

void JSHandler::OnMethodCall(WebView* caller, unsigned int remote_object_id, const WebString& method_name, const JSArray& args)
{
	if (method_name == WSLit("quit"))
	{
		engine->ClientCmd("quit");
	}
	else if (method_name == WSLit("spawnItem"))
	{
		if (!engine->IsInGame())
		{
			// auto-inspect the item instead
			//g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
			std::string id = WebStringToCharString(args[0].ToString());

			KeyValues* pItemKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(id));
			if (pItemKV)
			{
				g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);

				C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->AutoInspect(pItemKV);
				g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, pBrowserInstance, true);
				/*
				C_SteamBrowserInstance* pBrowserInstance = g_pAnarchyManager->AutoInspect(pItemKV);
				if (pBrowserInstance)
				{
					if (g_pAnarchyManager->GetInputManager()->GetInputMode())
					{
						g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(pBrowserInstance);
						pBrowserInstance->Select();
						pBrowserInstance->Focus();
					}
					else
						g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, pBrowserInstance, true);
				}
				*/
			}

			return;
		}

		//C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
		//C_SteamBrowserInstance* pSteamBrowserInstance = dynamic_cast<C_SteamBrowserInstance*>(pEmbeddedInstance);
		g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
		g_pAnarchyManager->DeselectEntity("", true);

		//if (pSteamBrowserInstance && pSteamBrowserInstance->GetId().find("scrape") == 0)
			//g_pAnarchyManager->GetSteamBrowserManager()->DestroySteamBrowserInstance(pSteamBrowserInstance);

		std::string id = WebStringToCharString(args[0].ToString());

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

		std::string modelId = cvar->FindVar("recent_model_id")->GetString();// g_pAnarchyManager->GenerateLegacyHash("models/cabinets/two_player_arcade.mdl");
		object_t* pObject = g_pAnarchyManager->GetInstanceManager()->AddObject("", id, modelId, tr.endpos, angles, 1.0f, false);
		g_pAnarchyManager->GetMetaverseManager()->SetSpawningObject(pObject);
		g_pAnarchyManager->GetInstanceManager()->SpawnObject(pObject);

		//char buf[512];
		//Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", tr.endpos.x, tr.endpos.y, tr.endpos.z);
		//pObjectKV->SetString("origin", buf);
		//Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", angles.x, angles.y, angles.z);
		//pObjectKV->SetString("angles", buf);

		//pClientArcadeResources->AddObjectToArrangement(pObjectKV);
		//pClientArcadeResources->SaveArrangements(true);

		//	ConVar* NextToMakeTypeVar = cvar->FindVar("NextToMakeType");
		//	NextToMakeTypeVar->SetValue(pItemKV->GetString("type", "other"));

		//std::string msg = VarArgs("spawnshortcut \"%s\" \"%s\" %.10f %.10f %.10f %.10f %.10f %.10f\n", id.c_str(), "models/cabinets/two_player_arcade.mdl", tr.endpos.x, tr.endpos.y, tr.endpos.z, angles.x, angles.y, angles.z);
		//engine->ServerCmd(msg.c_str(), false);

		//DevMsg("SPAWN THE SHIT! %s\n", id.c_str());

		/*
		std::string modelFile = "";
		std::string msg = VarArgs("spawnshortcut \"%s\" \"%s\" %.10f %.10f %.10f %.10f %.10f %.10f\n", pNearObject->itemId.c_str(), modelFile.c_str(), pNearObject->origin.x, pNearObject->origin.y, pNearObject->origin.z, pNearObject->angles.x, pNearObject->angles.y, pNearObject->angles.z);
		engine->ServerCmd(msg.c_str(), false);
		*/
		//g_pAnarchyManager->GetInstanceManager()->Spaw>SpawnItem(id);
	}
	else if (method_name == WSLit("setLibraryBrowserContext"))
	{
		// SEARCH CONTEXT OF THE ENTRY SPAWNED
		std::string category = WebStringToCharString(args[0].ToString());
		std::string id = WebStringToCharString(args[1].ToString());
		std::string search = WebStringToCharString(args[2].ToString());
		std::string filter = WebStringToCharString(args[3].ToString());
		g_pAnarchyManager->GetMetaverseManager()->SetLibraryBrowserContext(category, id, search, filter);
	}
	else if (method_name == WSLit("spawnEntry"))
	{
		// spawnEntry(category, entryId, search, filter)
		g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);

		// SEARCH CONTEXT OF THE ENTRY SPAWNED
		//const char* category = WebStringToCharString(args[0].ToString());

		unsigned int numArgs = args.size();
		std::string category = WebStringToCharString(args[0].ToString());
		std::string id = WebStringToCharString(args[1].ToString());	// used as a starting point for the search // NOTE: Might need a find/getPrevious to go along with find/getNext
		std::string search = WebStringToCharString(args[2].ToString());
		std::string filter = WebStringToCharString(args[3].ToString());
		std::string modelId = (numArgs > 4) ? WebStringToCharString(args[4].ToString()) : "";
		std::string scale = (numArgs > 5) ? WebStringToCharString(args[5].ToString()) : "1.0";
		std::string pitch = (numArgs > 6) ? WebStringToCharString(args[6].ToString()) : "0";
		std::string yaw = (numArgs > 7) ? WebStringToCharString(args[7].ToString()) : "0";
		std::string roll = (numArgs > 8) ? WebStringToCharString(args[8].ToString()) : "0";
		std::string offX = (numArgs > 9) ? WebStringToCharString(args[9].ToString()) : "0";
		std::string offY = (numArgs > 10) ? WebStringToCharString(args[10].ToString()) : "0";
		std::string offZ = (numArgs > 11) ? WebStringToCharString(args[11].ToString()) : "0";

		g_pAnarchyManager->GetMetaverseManager()->SetLibraryBrowserContext(category, id, search, filter);

		// get the point that the local player is looking at
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
			Vector objectToPlayer;
			VectorSubtract(pPlayer->GetAbsOrigin(), Vector(tr.endpos.x, tr.endpos.y, tr.endpos.z), objectToPlayer);

			xaxis = Vector(objectToPlayer.x, objectToPlayer.y, objectToPlayer.z);

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

		// position in tr.endpos
		// orientation in angles

		if (modelId == "")
		{
			if (category == "items")
			{
				std::string testerModelId = cvar->FindVar("recent_model_id")->GetString();
				if (testerModelId != "")
					modelId = testerModelId;// modelId = cvar->FindVar("recent_model_id")->GetString();
				else
					modelId = g_pAnarchyManager->GenerateLegacyHash("models/cabinets/two_player_arcade.mdl");	// TODO: Get the actual default model to use from the item itself, or intellegently figure out which one the user probably wants otherwise.
			}
			else if (category == "models")
			{
				// an empty modelId means the entry IS a model.
				modelId = "";

				/*
				KeyValues* entry = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(id);
				KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(entry);
				if (!active)
				DevMsg("ERROR: Could not find the given ID in the library!\n");

				modelId = active->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID));
				*/
			}
		}

		g_pAnarchyManager->GetInstanceManager()->AdjustObjectRot(Q_atof(pitch.c_str()), Q_atof(yaw.c_str()), Q_atof(roll.c_str()));
		g_pAnarchyManager->GetInstanceManager()->AdjustObjectOffset(Q_atof(offX.c_str()), Q_atof(offY.c_str()), Q_atof(offZ.c_str()));
		g_pAnarchyManager->GetInstanceManager()->AdjustObjectScale(Q_atof(scale.c_str()));

		object_t* pObject = g_pAnarchyManager->GetInstanceManager()->AddObject("", id, modelId, tr.endpos, angles, Q_atof(scale.c_str()), false);
		g_pAnarchyManager->GetMetaverseManager()->SetSpawningObject(pObject);
		g_pAnarchyManager->GetInstanceManager()->SpawnObject(pObject);

		//char buf[512];
		//Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", tr.endpos.x, tr.endpos.y, tr.endpos.z);
		//pObjectKV->SetString("origin", buf);
		//Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", angles.x, angles.y, angles.z);
		//pObjectKV->SetString("angles", buf);

		//pClientArcadeResources->AddObjectToArrangement(pObjectKV);
		//pClientArcadeResources->SaveArrangements(true);

		//	ConVar* NextToMakeTypeVar = cvar->FindVar("NextToMakeType");
		//	NextToMakeTypeVar->SetValue(pItemKV->GetString("type", "other"));

		//std::string msg = VarArgs("spawnshortcut \"%s\" \"%s\" %.10f %.10f %.10f %.10f %.10f %.10f\n", id.c_str(), "models/cabinets/two_player_arcade.mdl", tr.endpos.x, tr.endpos.y, tr.endpos.z, angles.x, angles.y, angles.z);
		//engine->ServerCmd(msg.c_str(), false);

		//DevMsg("SPAWN THE SHIT! %s\n", id.c_str());

		/*
		std::string modelFile = "";
		std::string msg = VarArgs("spawnshortcut \"%s\" \"%s\" %.10f %.10f %.10f %.10f %.10f %.10f\n", pNearObject->itemId.c_str(), modelFile.c_str(), pNearObject->origin.x, pNearObject->origin.y, pNearObject->origin.z, pNearObject->angles.x, pNearObject->angles.y, pNearObject->angles.z);
		engine->ServerCmd(msg.c_str(), false);
		*/
		//g_pAnarchyManager->GetInstanceManager()->Spaw>SpawnItem(id);
	}
	else if (method_name == WSLit("launchItem"))
	{
		std::string id = WebStringToCharString(args[0].ToString());
		launchErrorType_t error = g_pAnarchyManager->LaunchItem(id);

		std::string code = "launchFailed(\"";

		if (error == UNKNOWN_ERROR)
			code += "UNKNOWN_ERROR";
		else if (error == ITEM_NOT_FOUND)
			code += "ITEM_NOT_FOUND";
		else if (error == ITEM_FILE_NOT_FOUND)
			code += "ITEM_FILE_NOT_FOUND";
		else if (error == ITEM_FILE_PATH_RESTRICTED)
			code += "ITEM_FILE_PATH_RESTRICTED";
		else if (error == APP_NOT_FOUND)
			code += "APP_NOT_FOUND";
		else if (error == APP_FILE_NOT_FOUND)
			code += "APP_FILE_NOT_FOUND";
		else if (error == APP_PATH_NOT_FOUND)
			code += "APP_PATH_NOT_FOUND";

		code += "\");";

		if (error != NONE)
		{
			caller->ExecuteJavascript(WSLit(code.c_str()), WSLit(""));
			g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true);
			g_pAnarchyManager->GetInputManager()->SetFullscreenMode(true);
		}
		else
		{
			// TODO: Need to handle launching from instances that are only temporarily selected for input.  They probably keep playing even after launch, while launching from the selected cabinet always deselects the cabinet and ends playback.

			//C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
			//if (pEmbeddedInstance && pEmbeddedInstance->GetId() == "hud" && pEmbeddedInstance != 
			//	pEmbeddedInstance->Close();
		}
	}
	/*
	else if (method_name == WSLit("loadNextLocalAppCallback"))
	{
		// FIXME: This should be done outside of the awesomeium-specific classes!!
		C_MetaverseManager* pMetaverseManager = g_pAnarchyManager->GetMetaverseManager();

		C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
		//C_WebTab* pHudWebTab = g_pAnarchyManager->GetWebManager()->GetHudWebTab();

		KeyValues* app = pMetaverseManager->LoadNextLocalApp();
		if (app)
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading Apps", "locallibraryapps", "", "", "+", "loadNextLocalAppCallback");
		else
		{
			pMetaverseManager->LoadLocalAppClose();
			g_pAnarchyManager->OnLoadAllLocalAppsComplete();
		}
	}
	*/
	//else if (method_name == WSLit("loadLocalAppClose"))	// shouldn't really ever be called!!! (unless user abors the loading process)
	//	g_pAnarchyManager->GetMetaverseManager()->LoadLocalAppClose();
	else if (method_name == WSLit("detectAllModels"))
	{
		g_pAnarchyManager->DetectAllModelsThreaded();// > GetMetaverseManager()->DetectAllModels();
	}
	else if (method_name == WSLit("detectAllMaps"))
	{
		g_pAnarchyManager->GetMetaverseManager()->DetectAllMaps();
	}
	else if (method_name == WSLit("startupCallback"))
	{
		g_pAnarchyManager->OnStartupCallback();
	}
	else if (method_name == WSLit("hostSession"))
	{
		g_pAnarchyManager->GetMetaverseManager()->HostSession();
	}
	else if (method_name == WSLit("objectUpdateReceived"))
	{

		/*
		0 - isLocalUserUpdate (bool)
		1 - isFreshObject (bool)
		2 - id (string)
		3 - item (string)
		4 - model (string)
		5 - slave (bool)
		6 - child (bool)
		7 - parentObject (string)
		8 - scale (float)
		9 - origin (string)
		10 - angles (string)
		*/

		g_pAnarchyManager->GetMetaverseManager()->ObjectUpdateReceived(args[0].ToBoolean(), args[1].ToBoolean(), WebStringToCharString(args[2].ToString()), WebStringToCharString(args[3].ToString()), WebStringToCharString(args[4].ToString()), args[5].ToBoolean(), args[6].ToBoolean(), WebStringToCharString(args[7].ToString()), Q_atof(WebStringToCharString(args[8].ToString())), WebStringToCharString(args[9].ToString()), WebStringToCharString(args[10].ToString()));
	}
	else if (method_name == WSLit("sendLocalChatMsg"))
	{
		std::string chatText = WebStringToCharString(args[0].ToString());
		g_pAnarchyManager->GetMetaverseManager()->SendLocalChatMsg(chatText);
	}/*
	else if (method_name == WSLit("extractOverviewTGA"))
	{
		g_pAnarchyManager->GetMetaverseManager()->ExtractOverviewTGA();
	}*/
	else if (method_name == WSLit("sendEntryUpdate"))
	{
		std::string mode = WebStringToCharString(args[0].ToString());
		std::string id = WebStringToCharString(args[1].ToString());
		g_pAnarchyManager->GetMetaverseManager()->SendEntryUpdate(mode, id);
	}
	else if (method_name == WSLit("joinLobby"))
	{
		std::string lobbyId = WebStringToCharString(args[0].ToString());
		g_pAnarchyManager->JoinLobby(lobbyId);
	}
	else if (method_name == WSLit("followPlayer"))
	{
		std::string userId = WebStringToCharString(args[0].ToString());
		user_t* pUser = g_pAnarchyManager->GetMetaverseManager()->GetInstanceUser(userId);
		if (pUser)
		{
			g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
			g_pAnarchyManager->GetMetaverseManager()->InstanceUserClicked(pUser);
		}
		else
			DevMsg("WARNING: Could NOT find user to follow.\n");
	}
	else if (method_name == WSLit("banPlayer"))
	{
		std::string userId = WebStringToCharString(args[0].ToString());
		g_pAnarchyManager->GetMetaverseManager()->BanSessionUser(userId);
		/*
		user_t* pUser = g_pAnarchyManager->GetMetaverseManager()->GetInstanceUser(userId);
		if (pUser)
		{
			g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
			g_pAnarchyManager->GetMetaverseManager()->InstanceUserClicked(pUser);
		}
		else
			DevMsg("WARNING: Could NOT find user to follow.\n");*/
	}
	else if (method_name == WSLit("unbanPlayer"))
	{
		std::string userId = WebStringToCharString(args[0].ToString());
		g_pAnarchyManager->GetMetaverseManager()->UnbanSessionUser(userId);
	}
	else if (method_name == WSLit("syncPano"))
	{
		g_pAnarchyManager->GetMetaverseManager()->SyncPano();
	}
	else if (method_name == WSLit("networkEvent"))
	{
		std::string eventType = WebStringToCharString(args[0].ToString());

		//DevMsg("Network Event %s:\n\t", eventType.c_str());
		if (eventType == "universeJoin")
		{
			std::string universeId = WebStringToCharString(args[1].ToString());

			DevMsg("ID: %s\n", universeId.c_str());
		}
		else if (eventType == "universeCreate")
		{
			std::string universeId = WebStringToCharString(args[1].ToString());

			DevMsg("ID: %s\n", universeId.c_str());
		}
		else if (eventType == "metaverseConnect")
		{
			std::string address = WebStringToCharString(args[1].ToString());

			DevMsg("Address: %s\n", address.c_str());
		}
		else if (eventType == "userLogin")
		{
			std::string userId = WebStringToCharString(args[1].ToString());
			std::string displayName = WebStringToCharString(args[2].ToString());

			DevMsg("ID: %s\tDisplay Name: %s\n", userId.c_str(), displayName.c_str());
		}
		else if (eventType == "modelCreate")
		{
			std::string modelId = WebStringToCharString(args[1].ToString());
			int iCurrent = args[2].ToInteger();
			int iMax = args[3].ToInteger();

			//DevMsg("ID: %s\tCurrent: %i\tMax: %i\n", modelId.c_str(), iCurrent, iMax);

			C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Syncing Models", "aampProcessingModels", "0", VarArgs("%i", iMax), VarArgs("%i", iCurrent), "");
		}
		else if (eventType == "appCreate")
		{
			std::string appId = WebStringToCharString(args[1].ToString());
			int iCurrent = args[2].ToInteger();
			int iMax = args[3].ToInteger();

			//DevMsg("ID: %s\tCurrent: %i\tMax: %i\n", appId.c_str(), iCurrent, iMax);

			C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Syncing Apps", "aampProcessingApps", "0", VarArgs("%i", iMax), VarArgs("%i", iCurrent), "");
		}
		else if (eventType == "itemCreate")
		{
			std::string itemId = WebStringToCharString(args[1].ToString());
			int iCurrent = args[2].ToInteger();
			int iMax = args[3].ToInteger();

			//DevMsg("ID: %s\tCurrent: %i\tMax: %i\n", itemId.c_str(), iCurrent, iMax);

			C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Syncing Items", "aampProcessingItems", "0", VarArgs("%i", iMax), VarArgs("%i", iCurrent), "");
		}
		else if (eventType == "objectCreate")
		{
			std::string objectId = WebStringToCharString(args[1].ToString());
			int iCurrent = args[2].ToInteger();
			int iMax = args[3].ToInteger();

			//DevMsg("ID: %s\tCurrent: %i\tMax: %i\n", objectId.c_str(), iCurrent, iMax);

			C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Syncing Objects", "aampProcessingObjects", "0", VarArgs("%i", iMax), VarArgs("%i", iCurrent), "");
		}
		else if (eventType == "typeCreate")
		{
			std::string typeId = WebStringToCharString(args[1].ToString());
			int iCurrent = args[2].ToInteger();
			int iMax = args[3].ToInteger();

			//DevMsg("ID: %s\tCurrent: %i\tMax: %i\n", typeId.c_str(), iCurrent, iMax);

			C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Syncing Types", "aampProcessingTypes", "0", VarArgs("%i", iMax), VarArgs("%i", iCurrent), "");
		}
		else if (eventType == "mapCreate")
		{
			std::string mapId = WebStringToCharString(args[1].ToString());

			DevMsg("ID: %s\n", mapId.c_str());
		}
		else if (eventType == "instanceCreate")
		{
			std::string instanceId = WebStringToCharString(args[1].ToString());

			DevMsg("ID: %s\n", instanceId.c_str());
		}
		else if (eventType == "instanceObjectCreate")
		{
			std::string instanceId = WebStringToCharString(args[1].ToString());
			std::string objectId = WebStringToCharString(args[2].ToString());
			int iCurrent = args[3].ToInteger();
			int iMax = args[4].ToInteger();

			//DevMsg("INSTANCE: %s\tOBJECT: %s\tCurrent: %i\tMax: %i\n", instanceId.c_str(), objectId.c_str(), iCurrent, iMax);

			C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Verifying Objects", "aampSyncingObjects", "0", VarArgs("%i", iMax), VarArgs("%i", iCurrent), "");
		}
		else if (eventType == "instanceUserCreate")
		{
			std::string instanceId = WebStringToCharString(args[1].ToString());
			std::string sessionId = WebStringToCharString(args[2].ToString());
			std::string userId = WebStringToCharString(args[3].ToString());

			DevMsg("INSTANCE: %s\tSESSION: %s\tUSER: %s\n", instanceId.c_str(), sessionId.c_str(), userId.c_str());
		}
		else if (eventType == "instanceUserAdded")
		{
			std::string userId = WebStringToCharString(args[1].ToString());
			std::string sessionId = WebStringToCharString(args[2].ToString());
			std::string displayName = WebStringToCharString(args[3].ToString());

			g_pAnarchyManager->GetMetaverseManager()->InstanceUserAddedReceived(userId, sessionId, displayName);
		}
		else if (eventType == "instanceUserRemoved")
		{
			std::string userId = WebStringToCharString(args[1].ToString());
			g_pAnarchyManager->GetMetaverseManager()->InstanceUserRemoved(userId);
		}
		//else if (eventType == "overviewComplete")
		//{
		//	g_pAnarchyManager->GetMetaverseManager()->OverviewSyncComplete();
		//}
		else if (eventType == "panoComplete")
		{
			std::string cachedPanoName = WebStringToCharString(args[1].ToString());
			std::string panoId = WebStringToCharString(args[2].ToString());
			g_pAnarchyManager->GetMetaverseManager()->PanoSyncComplete(cachedPanoName, panoId);
		}
		else if (eventType == "userSessionUpdated")
		{
			int iUpdateMask = args[1].ToInteger();
			std::string userId = WebStringToCharString(args[2].ToString());
			std::string sessionId = WebStringToCharString(args[3].ToString());
			std::string displayName = WebStringToCharString(args[4].ToString());
			std::string itemId = WebStringToCharString(args[5].ToString());
			std::string objectId = WebStringToCharString(args[6].ToString());
			std::string say = WebStringToCharString(args[7].ToString());
			std::string bodyOrigin = WebStringToCharString(args[8].ToString());
			std::string bodyAngles = WebStringToCharString(args[9].ToString());
			std::string headOrigin = WebStringToCharString(args[10].ToString());
			std::string headAngles = WebStringToCharString(args[11].ToString());
			std::string mouseX = WebStringToCharString(args[12].ToString());
			std::string mouseY = WebStringToCharString(args[13].ToString());
			std::string webUrl = WebStringToCharString(args[14].ToString());
			std::string avatarUrl = WebStringToCharString(args[15].ToString());

			g_pAnarchyManager->GetMetaverseManager()->UserSessionUpdated(iUpdateMask, userId, sessionId, displayName, itemId, objectId, say, bodyOrigin, bodyAngles, headOrigin, headAngles, mouseX, mouseY, webUrl, avatarUrl);

			//DevMsg("User session update detected!!\n");
			/*
			std::string instanceId = WebStringToCharString(args[1].ToString());
			std::string sessionId = WebStringToCharString(args[2].ToString());
			std::string userId = WebStringToCharString(args[3].ToString());

			DevMsg("INSTANCE: %s\tSESSION: %s\tUSER: %s\n", instanceId.c_str(), sessionId.c_str(), userId.c_str());*/
			/*
			if (!this->IsMapLoaded() || !this->IsReady() || !m_pPhantomModeReady)
				return;

			//	DevMsg("Update the position for player %s\n", name);

			PhantomPlayer_t* pPhantomPlayer = null;
			C_DynamicProp* pEntity = null;

			unsigned int i;
			for (i = 0; i < m_phantomPlayers.size(); i++)
			{
				pEntity = null;

				pPhantomPlayer = m_phantomPlayers[i];

				if (!Q_strcmp(m_phantomPlayers[i]->id.c_str(), id))
				{
					pPhantomPlayer->name = name;

					if (pPhantomPlayer->say != say && Q_strcmp(say, ""))
					{
						engine->ServerCmd(VarArgs("psay \"%s\" \"%s\";\n", pPhantomPlayer->name.c_str(), say), false);
						//engine->ServerCmd(VarArgs("psay \"%s\" \"%s\";\n", pPhantomPlayer->name.c_str(), say), false);
					}

					pPhantomPlayer->say = say;
					//			pPhantomPlayer->selectedKey = selectedKey;

					// Handle follow behavior
					if (pPhantomPlayer->selectedKey != selectedKey)
					{
						C_PropHotlinkEntity* pSelectedHotlink = this->GetPhantomObjectEntityByKey(selectedKey);

						if (pSelectedHotlink)
						{
							if (pPhantomPlayer->bIsFollowed)
								engine->ClientCmd(VarArgs("setmpfocusnow %i;\n", pSelectedHotlink->entindex()));
						}
					}

					pPhantomPlayer->selectedKey = selectedKey;

					if (pPhantomPlayer->iEntityIndex > 0)
					{
						C_BaseEntity* pBaseEntity = C_BaseEntity::Instance(pPhantomPlayer->iEntityIndex);

						if (pBaseEntity)
						{
							pEntity = dynamic_cast<C_DynamicProp*>(pBaseEntity);

							if (pEntity)
							{
								//						DevMsg("Phantom Update Index: %s\n", pEntity->entindex());	// THE ENTITY MIGHT NTO YET BE VALID!!!! so don't try to use it.
								engine->ServerCmd(VarArgs("set_object_pos %i %f %f %f %f %f %f;\n", pPhantomPlayer->iEntityIndex, position.x, position.y, position.z, rotation.x, rotation.y, rotation.z), false);
								//engine->ServerCmd(VarArgs("set_object_pos %i %f %f %f %f %f %f;\n", pPhantomPlayer->iEntityIndex, position.x, position.y, position.z, rotation.x, rotation.y, rotation.z), false);
							}
						}
					}

					break;
				}

				pPhantomPlayer = null;
			}

			// If we haven't been found yet, create us.
			if (!pPhantomPlayer)
			{
				pPhantomPlayer = new PhantomPlayer_t();
				pPhantomPlayer->iEntityIndex = 0;
				pPhantomPlayer->bIsFollowed = false;
				pPhantomPlayer->selectedKey = selectedKey;
				pPhantomPlayer->id = id;
				pPhantomPlayer->name = name;
				pPhantomPlayer->say = say;

				m_phantomPlayers.push_back(pPhantomPlayer);

				engine->ServerCmd(VarArgs("create_phantom_object_pos \"%s\" %f %f %f %f %f %f \"%s\" \"%s\";\n", modelName, position.x, position.y, position.z, rotation.x, rotation.y, rotation.z, id, name), false);
				//engine->ServerCmd(VarArgs("create_object_pos \"models/players/heads/flipflopfred.mdl\" %f %f %f %f %f %f \"%s\" \"%s\";\n", position.x, position.y, position.z, rotation.x, rotation.y, rotation.z, id, name), false);
			}
			*/
		}
		else if (eventType == "hostReady")
		{
			DevMsg("Multiplayer server is ready.\n");

			std::string address = std::string(WebStringToCharString(args[1].ToString()));
			std::string universe = std::string(WebStringToCharString(args[2].ToString()));
			std::string instance = std::string(WebStringToCharString(args[3].ToString()));
			std::string session = std::string(WebStringToCharString(args[4].ToString()));
			std::string lobby = std::string(WebStringToCharString(args[5].ToString()));
			bool bIsPublic = bool(Q_atoi(WebStringToCharString(args[6].ToString())));
			std::string lobbyPassword = std::string(WebStringToCharString(args[7].ToString()));
			g_pAnarchyManager->SetConnectedUniverse(true, address, universe, instance, session, lobby, bIsPublic, lobbyPassword);

			/*
			std::vector<std::string> args;
			args.push_back(address);
			args.push_back(universe);
			args.push_back(instance);
			args.push_back(session);
			*/

			std::vector<std::string> args;
			C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
			pHudBrowserInstance->AddHudLoadingMessage("", "", "Complete", "complete", "", "", "", "");
			pHudBrowserInstance->DispatchJavaScriptMethod("eventListener", "doneHostingSession", args);
			g_pAnarchyManager->AddToastMessage("Online Session Ready");

			
		}
		else
			DevMsg("WARNING: Unhandled network event.\n");
	}
	else if (method_name == WSLit("disconnected"))
	{
		g_pAnarchyManager->GetMetaverseManager()->Disconnected();
	}
	else if (method_name == WSLit("restartNetwork"))
	{
		g_pAnarchyManager->GetMetaverseManager()->RestartNetwork();
	}
	else if (method_name == WSLit("importNextSteamGameCallback"))
	{
		g_pAnarchyManager->GetMetaverseManager()->ImportNextSteamGame();
	}
	else if (method_name == WSLit("readyToLoadUserLibraryCallback"))
	{
		g_pAnarchyManager->OnReadyToLoadUserLibrary();
	}
	else if (method_name == WSLit("updateLibraryVersionCallback"))
	{
		g_pAnarchyManager->OnUpdateLibraryVersionCallback();
	}
	else if (method_name == WSLit("mountNextWorkshopCallback"))
	{
		C_WorkshopManager* pWorkshopManager = g_pAnarchyManager->GetWorkshopManager();
		pWorkshopManager->MountNextWorkshop();
	}
	else if (method_name == WSLit("loadNextLocalItemLegacyCallback"))
	{
		// FIXME: This should be done outside of the awesomeium-specific classes!!
		C_WorkshopManager* pWorkshopManager = g_pAnarchyManager->GetWorkshopManager();
		C_MetaverseManager* pMetaverseManager = g_pAnarchyManager->GetMetaverseManager();
		//C_WebTab* pHudWebTab = g_pAnarchyManager->GetWebManager()->GetHudWebTab();

		g_pAnarchyManager->GetMetaverseManager()->LoadNextLocalItemLegacy();
//		{
		//	pMetaverseManager->LoadLocalItemLegacyClose();
			//pWorkshopManager->OnMountWorkshopSucceed();
	//	}
	}
	else if (method_name == WSLit("detectNextMapCallback"))
	{
		C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");

		bool bAlreadyExists = false;
		KeyValues* map = g_pAnarchyManager->GetMetaverseManager()->DetectNextMap(bAlreadyExists);
		if (map)
		{
			if( bAlreadyExists )
				pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Detecting Maps", "detectmaps", "", "", "+0", "detectNextMapCallback");
			else
				pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Detecting Maps", "detectmaps", "", "", "+", "detectNextMapCallback");
		}
		else
			g_pAnarchyManager->GetMetaverseManager()->OnDetectAllMapsCompleted();
	}
	else if (method_name == WSLit("addNextDefaultLibraryCallback"))
	{
		g_pAnarchyManager->OnAddNextDefaultLibraryCallback();
	}
	else if (method_name == WSLit("rebuildSoundCacheCallback"))
	{
		g_pAnarchyManager->OnRebuildSoundCacheCallback();
	}
	else if (method_name == WSLit("processAllModelsCallback"))
	{
		g_pAnarchyManager->ProcessAllModels();
	}
	else if (method_name == WSLit("processNextModelCallback"))
	{
		g_pAnarchyManager->ProcessNextModel();
	}
	else if (method_name == WSLit("addNextModelCallback"))
	{
		g_pAnarchyManager->AddNextModel();
	}
	else if (method_name == WSLit("defaultLibraryReadyCallback"))
	{
		g_pAnarchyManager->OnDefaultLibraryReadyCallback();
	}
	//else if (method_name == WSLit("defaultLibraryReadyCallback"))
	//{
	//	g_pAnarchyManager->OnDefaultLibraryReady();
	//}
	else if (method_name == WSLit("loadMap"))
	{

		std::string mapId = WebStringToCharString(args[0].ToString());
		std::string instanceId = (args.size() > 1) ? WebStringToCharString(args[1].ToString()) : "";
		std::string position = (args.size() > 2) ? WebStringToCharString(args[2].ToString()) : "";
		std::string rotation = (args.size() > 3) ? WebStringToCharString(args[3].ToString()) : "";
		std::string screenshotId = (args.size() > 4) ? WebStringToCharString(args[4].ToString()) : "";

		if (!g_pAnarchyManager->LoadMapCommand(mapId, instanceId, position, rotation, screenshotId))
		{
			C_AwesomiumBrowserInstance* pHudInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
			pHudInstance->SetUrl("asset://ui/welcome.html");
		}
	}
	else if (method_name == WSLit("loadMapNow"))
	{
		cvar->FindVar("last_map_loaded")->SetValue(0);
		engine->ClientCmd("host_writeconfig");

		std::string map = WebStringToCharString(args[0].ToString());
		engine->ClientCmd(VarArgs("map \"%s\"\n", map.c_str()));
	}
	else if (method_name == WSLit("deactivateInputMode"))
	{
		g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);

		if (args.size() > 0 && args[0].ToBoolean())
		{
			C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
			pHudBrowserInstance->SetUrl("asset://ui/welcome.html");
			g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true);
		}

		//if (Q_strcmp(g_pAnarchyManager->MapName(), ""))
		//	g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
		//else
//			g_pAnarchyManager->

		// unpause us if we are paused at main menu
	}
	else if (method_name == WSLit("forceInputMode"))
	{
		g_pAnarchyManager->GetInputManager()->ForceInputMode();
	}
	else if (method_name == WSLit("hudMouseUp"))
	{
		//C_WebTab* pWebTab = g_pAnarchyManager->GetWebManager()->GetSelectedWebTab();
		C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
		C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
		if (!pEmbeddedInstance)
			return;

		int code = args[0].ToInteger();
		bool bPassThru = args[1].ToBoolean();

		ButtonCode_t button = MOUSE_LEFT;
		switch (code)
		{
			case 0:
			defaut:
				button = MOUSE_LEFT;
				break;

			case 1:
				button = MOUSE_RIGHT;
				break;

			case 2:
				button = MOUSE_MIDDLE;
				break;
		}


		//if (bPassThru && pWebTab != g_pAnarchyManager->GetWebManager()->GetHudWebTab())
		if (bPassThru && pEmbeddedInstance != (C_EmbeddedInstance*)pHudBrowserInstance)
		{
			C_AwesomiumBrowserInstance* pOtherAwesomiumBrowserInstance = dynamic_cast<C_AwesomiumBrowserInstance*>(pEmbeddedInstance);
			if (pOtherAwesomiumBrowserInstance)
			{
			//	pOtherAwesomiumBrowserInstance->Select();
				pOtherAwesomiumBrowserInstance->Focus();
				pOtherAwesomiumBrowserInstance->OnMouseReleased(button);

			}
			else
			{
				if (!pEmbeddedInstance->HasFocus())
				{
				//	pEmbeddedInstance->Select();
					pEmbeddedInstance->Focus();
				}

				pEmbeddedInstance->GetInputListener()->OnMouseReleased(button);
			}
		}
	}
	else if (method_name == WSLit("hudMouseDown"))
	{
		//C_WebTab* pWebTab = g_pAnarchyManager->GetWebManager()->GetSelectedWebTab();
		C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
		C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
		if (!pEmbeddedInstance)
			return;

		int code = args[0].ToInteger();
		bool bPassThru = args[1].ToBoolean();

		ButtonCode_t button;
		switch (code)
		{
		case 0:
		default:
			button = MOUSE_LEFT;
			break;

		case 1:
			button = MOUSE_RIGHT;
			break;

		case 2:
			button = MOUSE_MIDDLE;
			break;
		}

		if (bPassThru && pEmbeddedInstance != (C_EmbeddedInstance*)pHudBrowserInstance)
		{
			C_AwesomiumBrowserInstance* pOtherAwesomiumBrowserInstance = dynamic_cast<C_AwesomiumBrowserInstance*>(pEmbeddedInstance);
			if (pOtherAwesomiumBrowserInstance)
			{
			//	pOtherAwesomiumBrowserInstance->Select();
				pOtherAwesomiumBrowserInstance->Focus();
				pOtherAwesomiumBrowserInstance->OnMousePressed(button);
			}
			else
			{
				pHudBrowserInstance->Blur();

				if (!pEmbeddedInstance->HasFocus())
				{
					//pEmbeddedInstance->Select();
					pEmbeddedInstance->Focus();
				}

				pEmbeddedInstance->GetInputListener()->OnMousePressed(button);
			}
		}
		else if (!bPassThru)
		{
			//pHudBrowserInstance->Focus();
			//if (g_pAnarchyManager->GetWebManager()->GetFocusedWebTab() != g_pAnarchyManager->GetWebManager()->GetHudWebTab())
			//if (pEmbeddedInstance != (C_EmbeddedInstance*)pHudBrowserInstance)
		//	{
				//pEmbeddedInstance->Select();
			//	pHudBrowserInstance->Focus();
			//}
		}
	}
	else if (method_name == WSLit("requestActivateInputMode"))
	{
//		C_WebTab* pWebTab = g_pAnarchyManager->GetWebManager()->GetWebBrowser()->FindWebTab(caller);	// FIXME This same code is somewhere else also.
//		if (!pWebTab)
//			return;

		//C_AwesomiumBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance(caller);
		



		g_pAnarchyManager->HudStateNotify();

		/*
		std::vector<std::string> params;
		params.push_back(VarArgs("%i", (g_pAnarchyManager->GetInputManager()->GetFullscreenMode())));
		params.push_back(VarArgs("%i", (g_pAnarchyManager->GetInputManager()->GetWasForceInputMode())));
		params.push_back(VarArgs("%i", engine->IsInGame()));
		params.push_back(VarArgs("%i", (g_pAnarchyManager->GetSelectedEntity() != null)));

		int entIndex = -1;
		int isItemSelected = 0;
		C_PropShortcutEntity* pShortcut = null;
		C_BaseEntity* pEntity = g_pAnarchyManager->GetSelectedEntity();
		if (pEntity)
		{
			pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);
			if (pShortcut && pShortcut->GetItemId() != "")
				isItemSelected = 1;
		}
		params.push_back(VarArgs("%i", isItemSelected));
		params.push_back(VarArgs("%i", (g_pAnarchyManager->GetInputManager()->GetMainMenuMode())));

		C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
		C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
		if (pEmbeddedInstance && pEmbeddedInstance != pHudBrowserInstance)
		{
			params.push_back(pEmbeddedInstance->GetURL());

			if (pShortcut && pEmbeddedInstance->GetOriginalEntIndex() == pShortcut->entindex())
				entIndex = pShortcut->entindex();
		}
		else
			params.push_back("");

		params.push_back(VarArgs("%i", entIndex));

		pHudBrowserInstance->DispatchJavaScriptMethod("arcadeHud", "onActivateInputMode", params);
		*/





		//g_pAnarchyManager->GetWebManager()->DispatchJavaScriptMethod(pWebTab, "arcadeHud", "onActivateInputMode", params);
	}
	else if (method_name == WSLit("disconnect"))
	{
		g_pAnarchyManager->Disconnect();
	}
	else if (method_name == WSLit("clearAwesomiumCache"))
	{
		g_pAnarchyManager->GetAwesomiumBrowserManager()->ClearCache();
	}
	/*
	else if (method_name == WSLit("autoInspect"))
	{
		// FIXME: THIS SHOULD JUST CALL A SUBROUTINE OF THE METAVERSE MANAGER!!

		std::string itemId = WebStringToCharString(args[0].ToString());
		KeyValues* item = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(itemId);
		if (item)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(item);

			std::string uri = "file://";
			uri += engine->GetGameDirectory();
			uri += "/resource/ui/html/autoInspectItem.html?mode=" + g_pAnarchyManager->GetAutoInspectMode() + "&id=" + g_pAnarchyManager->encodeURIComponent(itemId) + "&title=" + g_pAnarchyManager->encodeURIComponent(active->GetString("title")) + "&screen=" + g_pAnarchyManager->encodeURIComponent(active->GetString("screen")) + "&marquee=" + g_pAnarchyManager->encodeURIComponent(active->GetString("marquee")) + "&preview=" + g_pAnarchyManager->encodeURIComponent(active->GetString("preview")) + "&reference=" + g_pAnarchyManager->encodeURIComponent(active->GetString("reference")) + "&file=" + g_pAnarchyManager->encodeURIComponent(active->GetString("file"));

			C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
			if (pEmbeddedInstance)
			{
				C_SteamBrowserInstance* pSteamBrowserInstance = dynamic_cast<C_SteamBrowserInstance*>(pEmbeddedInstance);
				if (pSteamBrowserInstance)
				{
					pSteamBrowserInstance->SetActiveScraper("", "", "");
					pSteamBrowserInstance->SetUrl(uri);
				}
			}
		}
	}
	*/
	else if (method_name == WSLit("cabinetSelected"))
	{
		C_BaseEntity* pEntity = g_pAnarchyManager->GetMetaverseManager()->GetSpawningObjectEntity();
		if (pEntity)
		{
			std::string modelId = WebStringToCharString(args[0].ToString());
			KeyValues* model = g_pAnarchyManager->GetMetaverseManager()->GetLibraryModel(modelId);
			if (model)
			{
				KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(model);

				std::string modelFile = active->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID));
				g_pAnarchyManager->GetInstanceManager()->ChangeModel(pEntity, modelId, modelFile, g_pAnarchyManager->UseBuildGhosts());
			}
		}

	}
	else if (method_name == WSLit("playSound"))
	{
		std::string file = WebStringToCharString(args[0].ToString());
		g_pAnarchyManager->PlaySound(file);
	}
	else if (method_name == WSLit("viewObjectInfo"))
	{
		// JUST USES THE OBJECT THAT IS CURRENTLY UNDER THE PLAYER'S CURSOR FOR NOW.
		unsigned int numArgs = args.size();
		int iEntityIndex = (numArgs > 0) ? args[0].ToInteger() : -1;

		C_BaseEntity* pEntity = null;// g_pAnarchyManager->GetSelectedEntity();
		if (iEntityIndex >= 0)
			pEntity = C_BaseEntity::Instance(iEntityIndex);

		C_PropShortcutEntity* pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);
		if (!pShortcut)
		{
			C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
			Vector forward;
			pPlayer->EyeVectors(&forward);

			trace_t tr;
			UTIL_TraceLine(pPlayer->EyePosition(),
				pPlayer->EyePosition() + forward * MAX_TRACE_LENGTH, MASK_NPCSOLID,
				pPlayer, COLLISION_GROUP_NONE, &tr);

			if (tr.fraction != 1.0 && tr.DidHitNonWorldEntity())
			{
				pEntity = tr.m_pEnt;
				if (pEntity)
					pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);
			}
		}

		if (pShortcut)
		{
			//g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
			std::string url = std::string("asset://ui/objectInfo.html?id=") + pShortcut->GetObjectId() + std::string("&entity=") + std::string(VarArgs("%i", pShortcut->entindex()));

			C_AwesomiumBrowserInstance* pHudInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
			pHudInstance->SetUrl(url);

			//g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
			//g_pAnarchyManager->GetInstanceManager()->RemoveEntity(pShortcut);
			//g_pAnarchyManager->GetInstanceManager()->
		}

		/*
		JSObject response;

		// get the objectID of the object under the player's cursor
		KeyValues* pItemKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->GetMap(instance->mapId.c_str()));

		JSObject mapObject;
		AddSubKeys(pMapKV, mapObject);

		KeyValues* stuffKV = g_pAnarchyManager->GetMetaverseManager()->DetectRequiredWorkshopForMapFile(pMapKV->GetString("platforms/-KJvcne3IKMZQTaG7lPo/file"));
		if (stuffKV)
		{
			JSObject stuff;
			AddSubKeys(stuffKV, stuff);
			stuffKV->deleteThis();
			response.SetProperty(WSLit("stuff"), stuff);
		}

		response.SetProperty(WSLit("map"), mapObject);

		//// PLAYERS

		return response;
		*/
	}
	else if (method_name == WSLit("moveObject"))
	{
		// DO WORK
		unsigned int numArgs = args.size();
		std::string scale = (numArgs > 1) ? WebStringToCharString(args[1].ToString()) : "1.0";
		std::string pitch = (numArgs > 2) ? WebStringToCharString(args[2].ToString()) : "0";
		std::string yaw = (numArgs > 3) ? WebStringToCharString(args[3].ToString()) : "0";
		std::string roll = (numArgs > 4) ? WebStringToCharString(args[4].ToString()) : "0";
		std::string offX = (numArgs > 5) ? WebStringToCharString(args[5].ToString()) : "0";
		std::string offY = (numArgs > 6) ? WebStringToCharString(args[6].ToString()) : "0";
		std::string offZ = (numArgs > 7) ? WebStringToCharString(args[7].ToString()) : "0";

		// MOVE
		// FIXME: TODO: Make this a standard helper function to get the object under the player's cursor.
		//C_PropShortcutEntity* pShortcut = null;
		//C_BaseEntity* pEntity = g_pAnarchyManager->GetSelectedEntity();
		//if (pEntity)
			//pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);

		int iEntityIndex = (numArgs > 0) ? args[0].ToInteger() : -1;

		C_BaseEntity* pEntity = null;// g_pAnarchyManager->GetSelectedEntity();
		if (iEntityIndex >= 0)
			pEntity = C_BaseEntity::Instance(iEntityIndex);

		C_PropShortcutEntity* pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);
		if (!pShortcut)
		{
			C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
			Vector forward;
			pPlayer->EyeVectors(&forward);

			trace_t tr;
			UTIL_TraceLine(pPlayer->EyePosition(),
				pPlayer->EyePosition() + forward * MAX_TRACE_LENGTH, MASK_NPCSOLID,
				pPlayer, COLLISION_GROUP_NONE, &tr);

			if (tr.fraction != 1.0 && tr.DidHitNonWorldEntity())
			{
				pEntity = tr.m_pEnt;
				if (pEntity)
					pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);
			}
		}

		if (pShortcut)
		{
			g_pAnarchyManager->GetInstanceManager()->AdjustObjectRot(Q_atof(pitch.c_str()), Q_atof(yaw.c_str()), Q_atof(roll.c_str()));
			g_pAnarchyManager->GetInstanceManager()->AdjustObjectOffset(Q_atof(offX.c_str()), Q_atof(offY.c_str()), Q_atof(offZ.c_str()));
			g_pAnarchyManager->GetInstanceManager()->AdjustObjectScale(Q_atof(scale.c_str()));

			g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
			g_pAnarchyManager->ActivateObjectPlacementMode(pShortcut, "move");
		}
	}
	else if (method_name == WSLit("beginImportSteamGames"))
	{
		g_pAnarchyManager->BeginImportSteamGames();
	}
	else if (method_name == WSLit("startImportSteamGames"))
	{
		g_pAnarchyManager->GetMetaverseManager()->StartImportSteamGames();
	}
	else if (method_name == WSLit("setSlaveScreen"))
	{
		// FIXME: If a map is loaded, input mode can be deactivated, but if at main menu that might be weird.
		g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);

		std::string objectId = WebStringToCharString(args[0].ToString());
		bool val = args[1].ToBoolean();
		g_pAnarchyManager->SetSlaveScreen(objectId, val);
	}
	else if (method_name == WSLit("navigateToURI"))
	{
		std::string uri = WebStringToCharString(args[0].ToString());

		g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);

		// BEHAVIOR: An existing SteamBrowserInstance will get re-used (FIXME: it should get its active scraper reset!!),
		// any other non-hud selected instance gets destroyed and replaced with a SteamBrowserInstance with same ID.
		// If the HUD IS the selected instance, then a brand NEW instance gets created.  and since there is no instanceID
		// to clone, this new SteamBrowserInstance is given a special name so that it gets auto-closed when input mode gets
		// deactivated.

		C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
		C_SteamBrowserInstance* pBrowserInstance = dynamic_cast<C_SteamBrowserInstance*>(pEmbeddedInstance);
		if (!pBrowserInstance)
		{
			std::string oldId;
			if (pEmbeddedInstance && pEmbeddedInstance->GetId() != "hud")
			{
				oldId = pEmbeddedInstance->GetId();

				// close this instance
				DevMsg("Removing embedded Instance ID: %s\n", oldId.c_str());

				pEmbeddedInstance->Blur();
				pEmbeddedInstance->Deselect();
				g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);
				pEmbeddedInstance->Close();
			}
			else	// FIXME: These should probably just take the ID of the currently active instance so it won't need any special treatment. (!!!!!)
				oldId = "scrape" + std::string(g_pAnarchyManager->GenerateUniqueId());	// name starts with scrape even tho its not scraping so that it can auto-close properly when input move is deactivated

			pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->CreateSteamBrowserInstance();
			pBrowserInstance->Init(oldId, uri.c_str(), "Steamworks Browser", null);
		}
		else
			pBrowserInstance->SetUrl(uri);	// reuse the current focused steam browser if it exists

		pBrowserInstance->SetActiveScraper("", "", "");
		pBrowserInstance->Select();
		pBrowserInstance->Focus();

		g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance((C_EmbeddedInstance*)pBrowserInstance);
		g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, pBrowserInstance);
	}
	else if (method_name == WSLit("showEngineOptionsMenu"))
	{
		// FIXME: If a map is loaded, input mode can be deactivated, but if at main menu that might be weird.
		//g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
		g_pAnarchyManager->ShowEngineOptionsMenu();
	}
	else if (method_name == WSLit("adjustObjectOffset"))
	{
		unsigned int numArgs = args.size();
		if (numArgs < 3)
			return;

		g_pAnarchyManager->GetInstanceManager()->AdjustObjectOffset((float)args[0].ToDouble(), (float)args[1].ToDouble(), (float)args[2].ToDouble());
	}
	else if (method_name == WSLit("adjustObjectRot"))
	{
		unsigned int numArgs = args.size();
		if (numArgs < 3)
			return;

		g_pAnarchyManager->GetInstanceManager()->AdjustObjectRot((float)args[0].ToDouble(), (float)args[1].ToDouble(), (float)args[2].ToDouble());
	}
	else if (method_name == WSLit("adjustObjectScale"))
	{
		unsigned int numArgs = args.size();
		if (numArgs < 1)
			return;

		g_pAnarchyManager->GetInstanceManager()->AdjustObjectScale((float)args[0].ToDouble());
	}
	else if (method_name == WSLit("goForward"))
	{
		C_SteamBrowserInstance* pSteamBrowserInstance = dynamic_cast<C_SteamBrowserInstance*>(g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance());
		if (pSteamBrowserInstance )
		{
			pSteamBrowserInstance->GoForward();
		}
	}
	else if (method_name == WSLit("doCopy"))
	{
		caller->Copy();
		g_pAnarchyManager->AddToastMessage("Copied to your clipboard.  CTRL+V to paste.");
	}
	else if (method_name == WSLit("goBack"))
	{
		C_SteamBrowserInstance* pSteamBrowserInstance = dynamic_cast<C_SteamBrowserInstance*>(g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance());
		if (pSteamBrowserInstance)
		{
			pSteamBrowserInstance->GoBack();
		}
	}
	else if (method_name == WSLit("reload"))
	{
		C_SteamBrowserInstance* pSteamBrowserInstance = dynamic_cast<C_SteamBrowserInstance*>(g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance());
		if (pSteamBrowserInstance)
		{
			pSteamBrowserInstance->Reload();
		}
	}
	else if (method_name == WSLit("goHome"))
	{
		C_SteamBrowserInstance* pSteamBrowserInstance = dynamic_cast<C_SteamBrowserInstance*>(g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance());
		if (pSteamBrowserInstance)
		{
			pSteamBrowserInstance->SetUrl(g_pAnarchyManager->GetHomeURL());
		}
	}
	else if (method_name == WSLit("libretroPause"))
	{
		C_LibretroInstance* pLibretroInstance = dynamic_cast<C_LibretroInstance*>(g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance());
		if (pLibretroInstance)
		{
			bool bValue;

			unsigned int numArgs = args.size();
			if (numArgs > 0 && args[0].IsBoolean())
				bValue = args[0].ToBoolean();
			else
				bValue = !pLibretroInstance->GetPause();

			pLibretroInstance->SetPause(bValue);
		}
	}
	else if (method_name == WSLit("setStartWithWindows"))
	{
		bool bValue = (args.size() > 0) ? !Q_strcmp(WebStringToCharString(args[0].ToString()), "1") : true;
		g_pAnarchyManager->SetStartWithWindows(bValue);
	}
	else if (method_name == WSLit("libretroSaveOverlay"))
	{
		std::string type;
		if (args.size() > 0)
			type = WebStringToCharString(args[0].ToString());

		std::string overlayId;
		if (args.size() > 1)
			overlayId = WebStringToCharString(args[1].ToString());

		float x = 0;
		if (args.size() > 2)
			x = (float)args[2].ToDouble();

		float y = 0;
		if (args.size() > 3)
			y = (float)args[3].ToDouble();

		float width = 1;
		if (args.size() > 4)
			width = (float)args[4].ToDouble();

		float height = 1;
		if (args.size() > 5)
			height = (float)args[5].ToDouble();

		C_LibretroInstance* pLibretroInstance = dynamic_cast<C_LibretroInstance*>(g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance());
		if (pLibretroInstance)
		{
			pLibretroInstance->SaveOverlay(type, overlayId, x, y, width, height);
			//pLibretroInstance->SetOverlay(overlayId);
		}
	}
	else if (method_name == WSLit("libretroClearOverlay"))
	{
		std::string type;
		if (args.size() > 0)
			type = WebStringToCharString(args[0].ToString());

		std::string overlayId;
		if (args.size() > 1)
			overlayId = WebStringToCharString(args[1].ToString());

		C_LibretroInstance* pLibretroInstance = dynamic_cast<C_LibretroInstance*>(g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance());
		if (pLibretroInstance)
		{
			pLibretroInstance->ClearOverlay(type, overlayId);
			//pLibretroInstance->SetOverlay(overlayId);
		}
	}
	else if (method_name == WSLit("acquire"))
	{
		std::string query = WebStringToCharString(args[0].ToString());
		if (query.find("http://") != 0 && query.find("https://") != 0)
			query = "http://" + query;

		g_pAnarchyManager->Acquire(query);
	}
	else if (method_name == WSLit("libretroSetOverlay"))
	{
		std::string type;
		if (args.size() > 0)
			type = WebStringToCharString(args[0].ToString());
		else
			type = "core";

		std::string overlayId;
		if (args.size() > 1)
			overlayId = WebStringToCharString(args[1].ToString());

		C_LibretroInstance* pLibretroInstance = dynamic_cast<C_LibretroInstance*>(g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance());
		if (pLibretroInstance)
		{
			g_pAnarchyManager->GetLibretroManager()->SaveOverlaysKV(type, overlayId, pLibretroInstance->GetInfo()->prettycore, pLibretroInstance->GetInfo()->prettygame);
			pLibretroInstance->SetOverlay(overlayId);
		}
	}
	else if (method_name == WSLit("libretroReset"))
	{
		C_LibretroInstance* pLibretroInstance = dynamic_cast<C_LibretroInstance*>(g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance());
		if (pLibretroInstance)
		{
			pLibretroInstance->SetReset(true);
		}
	}
	else if (method_name == WSLit("setLibretroGUIGamepadEnabled"))
	{
		if (g_pAnarchyManager->GetLibretroManager())
		{
			bool bValue;
			if (args.size() > 0 && args[0].IsBoolean())
				bValue = args[0].ToBoolean();
			else
				bValue = !g_pAnarchyManager->GetLibretroManager()->GetGUIGamepadEnabled();
			
			g_pAnarchyManager->GetLibretroManager()->ClearGUIGamepadInputState();
			g_pAnarchyManager->GetLibretroManager()->SetGUIGamepadEnabled(bValue);
		}
	}
	else if (method_name == WSLit("setLibretroGUIGamepadButtonState"))
	{
		if (args.size() > 1 && args[0].IsNumber() && args[1].IsNumber() && g_pAnarchyManager->GetLibretroManager())
		{
			int iButton = args[0].ToInteger();
			int iValue = args[1].ToInteger();
			//float fValue = (float)args[1].ToDouble();
			g_pAnarchyManager->GetLibretroManager()->SetGUIGamepadInputState(0, 1, 0, iButton, iValue);
		}
	}
	else if (method_name == WSLit("clearLibretroGUIGamepadButtonStates"))
	{
		if (g_pAnarchyManager->GetLibretroManager())
			g_pAnarchyManager->GetLibretroManager()->ClearGUIGamepadInputState();
	}
	else if (method_name == WSLit("taskClear"))
	{
		g_pAnarchyManager->TaskClear();
	}
	else if (method_name == WSLit("setTabMenuFile"))
	{
		std::string menuFile;
		if (args.size() > 0)
			menuFile = WebStringToCharString(args[0].ToString());

		if (menuFile == "")
			menuFile = "taskMenu.html";

		g_pAnarchyManager->SetTabMenuFile(menuFile);
	}
	else if (method_name == WSLit("switchToTask"))
	{
		std::string taskId = WebStringToCharString(args[0].ToString());
		g_pAnarchyManager->GetCanvasManager()->SwitchToInstance(taskId);
	}
	else if (method_name == WSLit("closeTask"))
	{
		std::string taskId = WebStringToCharString(args[0].ToString());
		g_pAnarchyManager->GetCanvasManager()->CloseInstance(taskId);
	}
	else if (method_name == WSLit("selectTaskObject"))
	{
		std::string taskId = WebStringToCharString(args[0].ToString());
		C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetCanvasManager()->FindEmbeddedInstance(taskId);
		if (pEmbeddedInstance)
		{
			C_PropShortcutEntity* pEntity = dynamic_cast<C_PropShortcutEntity*>(C_BaseEntity::Instance(pEmbeddedInstance->GetOriginalEntIndex()));
			if (pEntity)
			{
				g_pAnarchyManager->HideTaskMenu();
				g_pAnarchyManager->SelectEntity(pEntity);
			}
		}
	}
	else if (method_name == WSLit("hideTask"))
	{
		std::string taskId = WebStringToCharString(args[0].ToString());
		C_WindowInstance* pWindowInstance = g_pAnarchyManager->GetWindowManager()->FindWindowInstance(taskId);
		if (pWindowInstance)
			g_pAnarchyManager->GetWindowManager()->HideTask(pWindowInstance);
	}
	else if (method_name == WSLit("unhideTask"))
	{
		std::string taskId = WebStringToCharString(args[0].ToString());
		C_WindowInstance* pWindowInstance = g_pAnarchyManager->GetWindowManager()->FindWindowInstance(taskId);
		if (pWindowInstance)
			g_pAnarchyManager->GetWindowManager()->UnhideTask(pWindowInstance);
	}
	else if (method_name == WSLit("displayTask"))
	{
		std::string taskId = WebStringToCharString(args[0].ToString());
		C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetCanvasManager()->FindEmbeddedInstance(taskId);
		if (pEmbeddedInstance)
			g_pAnarchyManager->GetCanvasManager()->SetDisplayInstance(pEmbeddedInstance);
	}
	else if (method_name == WSLit("specialReady"))
	{
		C_AwesomiumBrowserInstance* pBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance(caller);
		if (pBrowserInstance)
		{
			g_pAnarchyManager->SpecialReady(pBrowserInstance);
			//std::string id = pBrowserInstance->GetId();
			//if (id == "hud")	// is this too early??
			//	g_pAnarchyManager->IncrementState();
			//else if (id == "images" && AASTATE_AWESOMIUMBROWSERMANAGERIMAGESWAIT)
			//	g_pAnarchyManager->IncrementState();
		}
	}
	else if (method_name == WSLit("doPause"))
	{
		g_pAnarchyManager->DoPause();
	}
	else if (method_name == WSLit("feedback"))
	{
		std::string type = WebStringToCharString(args[0].ToString());
		g_pAnarchyManager->Feedback(type);
	}
	else if (method_name == WSLit("consoleCommand"))
	{
		std::string command = WebStringToCharString(args[0].ToString());
		engine->ClientCmd(VarArgs("%s\n", command.c_str()));

		if (args.size() > 1 && args[1].ToBoolean() )
			g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
	}
	else if (method_name == WSLit("deleteApp"))
	{
		std::string appId = WebStringToCharString(args[0].ToString());
		g_pAnarchyManager->GetMetaverseManager()->DeleteApp(appId);
	}
	else if (method_name == WSLit("teleportScreenshot"))
	{
		std::string id = WebStringToCharString(args[0].ToString());
		bool bDeactivateInputMode = (args.size() > 1) ? args[1].ToBoolean() : true;
		g_pAnarchyManager->TeleportToScreenshot(id, bDeactivateInputMode);
	}
	else if (method_name == WSLit("saveNewNode"))
	{
		std::string nodeName = WebStringToCharString(args[0].ToString());
		g_pAnarchyManager->GetInstanceManager()->CreateNewNode(nodeName);
	}
	else if (method_name == WSLit("addToastMessage"))
	{
		std::string text = WebStringToCharString(args[0].ToString());
		g_pAnarchyManager->AddToastMessage(text);
	}
	else if (method_name == WSLit("clearNodeSpace"))
	{
		g_pAnarchyManager->GetInstanceManager()->ClearNodeSpace();
	}
	else if (method_name == WSLit("takeScreenshot"))
	{
		bool bCreateBig = (args.size() > 0) ? args[0].ToBoolean() : false;
		std::string id = (args.size() > 1) ? WebStringToCharString(args[1].ToString()) : "";
		g_pAnarchyManager->TakeScreenshot(bCreateBig, id);
	}
	else if (method_name == WSLit("deleteScreenshot"))
	{
		std::string screenshotId = WebStringToCharString(args[0].ToString());
		g_pAnarchyManager->GetMetaverseManager()->DeleteScreenshot(screenshotId);
	}
	else if (method_name == WSLit("deleteObject"))
	{
		// DO WORK


		// CANCEL
		//C_BaseEntity* pEntity = g_pAnarchyManager->GetMetaverseManager()->GetSpawningObjectEntity();
		unsigned int numArgs = args.size();
		int iEntityIndex = (numArgs > 0) ? args[0].ToInteger() : -1;

		C_BaseEntity* pEntity = null;// g_pAnarchyManager->GetSelectedEntity();
		if (iEntityIndex >= 0)
			pEntity = C_BaseEntity::Instance(iEntityIndex);

		C_PropShortcutEntity* pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);
		if (!pShortcut)
		{
			C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
			Vector forward;
			pPlayer->EyeVectors(&forward);

			trace_t tr;
			UTIL_TraceLine(pPlayer->EyePosition(),
				pPlayer->EyePosition() + forward * MAX_TRACE_LENGTH, MASK_NPCSOLID,
				pPlayer, COLLISION_GROUP_NONE, &tr);

			if (tr.fraction != 1.0 && tr.DidHitNonWorldEntity())
			{
				pEntity = tr.m_pEnt;
				if (pEntity)
					pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);
			}
		}

		if (pShortcut)
		{
			g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);

			if (g_pAnarchyManager->GetSelectedEntity() == pShortcut)
				g_pAnarchyManager->DeselectEntity();

			g_pAnarchyManager->GetInstanceManager()->RemoveEntity(pShortcut);
		}
	}
	else if (method_name == WSLit("modelSelected"))
	{
		// MODEL HAS BEEN SELECTED ON THE BUILDMODE MENU!!
		// This means the object must be updated with a new itemId, a new modelId, etc.
		// The entity needs to also be udpated to reflect the changes in the object.

		C_PropShortcutEntity* pEntity = g_pAnarchyManager->GetMetaverseManager()->GetSpawningObjectEntity();
		if (pEntity)
		{
			std::string modelId = WebStringToCharString(args[0].ToString());
			std::string modelFile;

			KeyValues* entryModel = g_pAnarchyManager->GetMetaverseManager()->GetLibraryModel(modelId);
			KeyValues* activeModel = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(entryModel);
			if (activeModel)
				modelFile = activeModel->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID), "models\\cabinets\\two_player_arcade.mdl");	// uses default model if key value read fails
			else
				modelFile = "models\\cabinets\\two_player_arcade.mdl";

			pEntity->PrecacheModel(modelFile.c_str());	// not needed.  handled server-side?
			pEntity->SetModel(modelFile.c_str());	// not needed.  handled server-side?
			engine->ServerCmd(VarArgs("setobjectids %i \"%s\" \"%s\" \"%s\" %i;\n", pEntity->entindex(), modelId.c_str(), modelId.c_str(), modelFile.c_str(), g_pAnarchyManager->UseBuildGhosts()), false);
			//engine->ServerCmd(VarArgs("makeghost %i;\n", pEntity->entindex()), false);	// might be over-kill, all we're trying to do is make sure it stays non-solid
			//pEntity->SetSolid(SOLID_NONE);
		}

		// Swap the current object out for the new one according to the given ID.
		//DevMsg("UNHANDLED CALL TO MODEL SELECTED!!\n");
		/*
		C_BaseEntity* pEntity = g_pAnarchyManager->GetMetaverseManager()->GetSpawningObjectEntity();
		if (pEntity)
		{
			std::string modelId = WebStringToCharString(args[0].ToString());
			KeyValues* model = g_pAnarchyManager->GetMetaverseManager()->GetLibraryModel(modelId);
			if (model)
			{
				KeyValues* active = model->FindKey("current");
				if (!active)
					active = model->FindKey("local", true);

				std::string modelFile = active->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID));
				g_pAnarchyManager->GetInstanceManager()->ChangeModel(pEntity, modelFile, g_pAnarchyManager->UseBuildGhosts());
			}
		}
		*/
	}
	else if (method_name == WSLit("objectHover"))
	{
		// OBJECT HAS BEEN HOVERED ON THE BUILDMODE MENU!!
		// This means the user wants to highlight it.
		std::string objectId = WebStringToCharString(args[0].ToString());
		C_BaseEntity* pEntity = g_pAnarchyManager->GetInstanceManager()->GetObjectEntity(objectId);
		if (pEntity)
			g_pAnarchyManager->AddHoverGlowEffect(pEntity);
	}
	else if (method_name == WSLit("objectSelected"))
	{
		//g_pAnarchyManager->RemoveLastHoverGlowEffects();

		// OBJECT HAS BEEN SELECTED ON THE BUILDMODE MENU!!
		// This means the user wants to select it.
		std::string objectId = WebStringToCharString(args[0].ToString());
		C_BaseEntity* pEntity = g_pAnarchyManager->GetInstanceManager()->GetObjectEntity(objectId);
		if (pEntity)
			g_pAnarchyManager->AttemptSelectEntity(pEntity);
	}
	else if (method_name == WSLit("popout"))
	{
		std::string popoutId = WebStringToCharString(args[0].ToString());
		g_pAnarchyManager->Popout(popoutId);
	}
	else if (method_name == WSLit("runLibretro"))
	{
		bool bLaunched = false;

		C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
		C_LibretroInstance* pLibretroInstance = dynamic_cast<C_LibretroInstance*>(pEmbeddedInstance);
		std::string itemId = pEmbeddedInstance->GetOriginalItemId();
		KeyValues* item = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(itemId);
		if (item)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(item);

			std::string gameFile = "";
			std::string coreFile = "";
			bool bShouldLibretroLaunch = (g_pAnarchyManager->DetermineLibretroCompatible(item, gameFile, coreFile) && (g_pAnarchyManager->GetLibretroManager()->GetInstanceCount() == 0 || pLibretroInstance));

			// auto-libretro
			bool bFileExists = g_pFullFileSystem->FileExists(gameFile.c_str());
			if (!bFileExists)
				g_pAnarchyManager->AddToastMessage("Libretro Aborted - File Not Found");
			else if (bShouldLibretroLaunch)
			{
				if (pLibretroInstance)
				{
					//pLibretroInstance->SetOriginalGame(gameFile);	// We cannot actually change the game this way if we are just doing a retro_reset because that does not re-trigger game loading.
					//pLibretroInstance->SetGame(gameFile);
					pLibretroInstance->SetReset(true);
				}
				else
				{
					std::string oldId = pEmbeddedInstance->GetId();
					//std::string oldTitle = pEmbeddedInstance->GetTitle();
					int iOldEntIndex = pEmbeddedInstance->GetOriginalEntIndex();

					pEmbeddedInstance->Blur();
					pEmbeddedInstance->Deselect();
					g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);
					pEmbeddedInstance->Close();

					pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->CreateLibretroInstance();
					pLibretroInstance->Init(oldId, VarArgs("%s - Libretro", active->GetString("title", "Untitled")), iOldEntIndex);
					DevMsg("Setting game to: %s\n", gameFile.c_str());
					pLibretroInstance->SetOriginalGame(gameFile);
					pLibretroInstance->SetOriginalItemId(itemId);
					if (!pLibretroInstance->LoadCore(coreFile))	// FIXME: elegantly revert back to autoInspect if loading the core failed!
						DevMsg("ERROR: Failed to load core: %s\n", coreFile.c_str());

					//pLibretroInstance->SetActiveScraper("", "", "");
					pLibretroInstance->Select();
					pLibretroInstance->Focus();

					g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance((C_EmbeddedInstance*)pLibretroInstance);
				}

				bLaunched = true;
			}
		}

		if (bLaunched)
		{
			KeyValues* pOverlayKV = pLibretroInstance->GetOverlayKV();
			vgui::CInputSlate* pInputSlate = g_pAnarchyManager->GetInputManager()->GetInputSlate();
			if (pInputSlate)
			{
				if (pOverlayKV)
					pInputSlate->AdjustOverlay(pOverlayKV->GetFloat("current/x", 0), pOverlayKV->GetFloat("current/y", 0), pOverlayKV->GetFloat("current/width", 1), pOverlayKV->GetFloat("current/height", 1), pLibretroInstance->GetOverlayId());
				else
					pInputSlate->AdjustOverlay(0, 0, 1, 1, "");
			}
		}
		else
			DevMsg("Failed to open with Libretro.\n");
	}
	else if (method_name == WSLit("viewStream"))
	{
		std::string itemId = WebStringToCharString(args[0].ToString());
		KeyValues* item = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(itemId);
		if (item)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(item);

			std::string streamUri = active->GetString("stream");
			if (streamUri.find("http") != 0)
				streamUri = active->GetString("file");

			if (streamUri.find("http") != 0)
				streamUri = active->GetString("preview");

			if (streamUri.find("http") != 0)
				return;

			// the active embeded instance might not be the right type, so get rdy to re-make it if needed...
			C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
			C_SteamBrowserInstance* pBrowserInstance = dynamic_cast<C_SteamBrowserInstance*>(pEmbeddedInstance);
			if (!pBrowserInstance)
			{
				std::string oldId;
				int iOldEntityIndex = -1;
				std::string oldItemId;
				if (pEmbeddedInstance)
				{
					oldId = pEmbeddedInstance->GetId();
					iOldEntityIndex = pEmbeddedInstance->GetOriginalEntIndex();
					oldItemId = pEmbeddedInstance->GetOriginalItemId();

					// close this instance
					//DevMsg("Removing embedded Instance ID: %s\n", oldId.c_str());

					pEmbeddedInstance->Blur();
					pEmbeddedInstance->Deselect();
					g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);
					pEmbeddedInstance->Close();

					pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->CreateSteamBrowserInstance();
					pBrowserInstance->Init(oldId, streamUri.c_str(), VarArgs("%s - Stream", active->GetString("title", "Untitled")), null, iOldEntityIndex);
					pBrowserInstance->SetOriginalItemId(oldItemId);
				}
			}
			else
				pBrowserInstance->SetUrl(streamUri);	// reuse the current focused steam browser if it exists

			pBrowserInstance->SetActiveScraper("", "", "");
			pBrowserInstance->Select();
			pBrowserInstance->Focus();

			g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance((C_EmbeddedInstance*)pBrowserInstance);

			vgui::CInputSlate* pInputSlate = g_pAnarchyManager->GetInputManager()->GetInputSlate();
			if (pInputSlate)
				pInputSlate->AdjustOverlay(0, 0, 1, 1, "");
		}
	}
	else if (method_name == WSLit("autoInspect"))
	{
		std::string itemId = WebStringToCharString(args[0].ToString());
		KeyValues* item = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(itemId);
		if (item)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(item);

			std::string uri = "file://";
			uri += engine->GetGameDirectory();
			uri += "/resource/ui/html/autoInspectItem.html?imageflags=" + g_pAnarchyManager->GetAutoInspectImageFlags() + "&id=" + g_pAnarchyManager->encodeURIComponent(itemId) + "&title=" + g_pAnarchyManager->encodeURIComponent(active->GetString("title")) + "&screen=" + g_pAnarchyManager->encodeURIComponent(active->GetString("screen")) + "&marquee=" + g_pAnarchyManager->encodeURIComponent(active->GetString("marquee")) + "&preview=" + g_pAnarchyManager->encodeURIComponent(active->GetString("preview")) + "&reference=" + g_pAnarchyManager->encodeURIComponent(active->GetString("reference")) + "&file=" + g_pAnarchyManager->encodeURIComponent(active->GetString("file"));

			// the active embeded instance might not be the right type, so get rdy to re-make it if needed...
			C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
			C_SteamBrowserInstance* pBrowserInstance = dynamic_cast<C_SteamBrowserInstance*>(pEmbeddedInstance);
			if (!pBrowserInstance)
			{
				std::string oldId;
				int iOldEntityIndex = -1;
				std::string oldItemId;
				if (pEmbeddedInstance)
				{
					oldId = pEmbeddedInstance->GetId();
					iOldEntityIndex = pEmbeddedInstance->GetOriginalEntIndex();
					oldItemId = pEmbeddedInstance->GetOriginalItemId();

					// close this instance
					//DevMsg("Removing embedded Instance ID: %s\n", oldId.c_str());

					pEmbeddedInstance->Blur();
					pEmbeddedInstance->Deselect();
					g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);
					pEmbeddedInstance->Close();

					pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->CreateSteamBrowserInstance();
					pBrowserInstance->Init(oldId, uri, VarArgs("%s - Stream", active->GetString("title", "Untitled")), null, iOldEntityIndex);
					pBrowserInstance->SetOriginalItemId(oldItemId);
				}
			}
			else
				pBrowserInstance->SetUrl(uri);	// reuse the current focused steam browser if it exists

			pBrowserInstance->SetActiveScraper("", "", "");
			pBrowserInstance->Select();
			pBrowserInstance->Focus();

			g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance((C_EmbeddedInstance*)pBrowserInstance);

			vgui::CInputSlate* pInputSlate = g_pAnarchyManager->GetInputManager()->GetInputSlate();
			if (pInputSlate)
				pInputSlate->AdjustOverlay(0, 0, 1, 1, "");
		}
	}
	else if (method_name == WSLit("viewPreview"))
	{
		std::string itemId = WebStringToCharString(args[0].ToString());
		std::string previewURL = WebStringToCharString(args[1].ToString());

		// the active embeded instance might not be the right type, so get rdy to re-make it if needed...
		C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
		C_SteamBrowserInstance* pBrowserInstance = dynamic_cast<C_SteamBrowserInstance*>(pEmbeddedInstance);
		if (!pBrowserInstance)
		{
			std::string oldId;
			int iOldEntityIndex = -1;
			std::string oldItemId;
			std::string oldTitle;
			if (pEmbeddedInstance)
			{
				oldId = pEmbeddedInstance->GetId();
				iOldEntityIndex = pEmbeddedInstance->GetOriginalEntIndex();
				oldItemId = pEmbeddedInstance->GetOriginalItemId();
				oldTitle = pEmbeddedInstance->GetTitle();

				// close this instance
				//DevMsg("Removing embedded Instance ID: %s\n", oldId.c_str());

				pEmbeddedInstance->Blur();
				pEmbeddedInstance->Deselect();
				g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);
				pEmbeddedInstance->Close();

				pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->CreateSteamBrowserInstance();
				pBrowserInstance->Init(oldId, previewURL, oldTitle, null, iOldEntityIndex);
				pBrowserInstance->SetOriginalItemId(oldItemId);
			}
		}
		else
			pBrowserInstance->SetUrl(previewURL);	// reuse the current focused steam browser if it exists

		pBrowserInstance->SetActiveScraper("", "", "");
		pBrowserInstance->Select();
		pBrowserInstance->Focus();

		g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance((C_EmbeddedInstance*)pBrowserInstance);

		vgui::CInputSlate* pInputSlate = g_pAnarchyManager->GetInputManager()->GetInputSlate();
		if (pInputSlate)
			pInputSlate->AdjustOverlay(0, 0, 1, 1,"");
	}
	else if (method_name == WSLit("getDOM"))
	{
		// get the embedded instance
		C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
		
		// if current instance is NOT Steamworks browser, throw error
		C_SteamBrowserInstance* pSteamBrowserInstance = dynamic_cast<C_SteamBrowserInstance*>(pEmbeddedInstance);
		if (!pSteamBrowserInstance)
		{
			DevMsg("ERROR: Active browser is NOT a Steamworks browser.");
			return;
		}

		// let 'er rip
		/* bust due to title limit being 4096
		std::string code = "document.title = \"AAAPICALL_";
		code += WebStringToCharString(args[0].ToString());
		code += ":\" + document.documentElement.innerHTML;";
		pSteamBrowserInstance->InjectJavaScript(code.c_str());
		*/

		// bust due to xmlhttprequests having a callback in the Steamworks browser
		//std::string code = "var xhr = new XMLHttpRequest(); xhr.open(\"POST\", \"http://www.aarcadeapicall.com.net.org/\", true); xhr.send(\"";
		//code += WebStringToCharString(args[0].ToString());
		//code += ":\" + document.documentElement.innerHTML); ";

		//std::string code = "var aaapicallform = document.createElement('form'); aaapicallform.method = 'post'; aaapicallform.action = 'http://www.aarcadeapicall.com.net.org/'; var aaapicallinput = document.createElement('input'); aaapicallinput.type = 'text'; aaapicallinput.value = document.documentElement.innerHTML; aaapicallform.appendChild(aaapicallinput); document.body.appendChild(aaapicallform); aaapicallform.submit();";
		//std::string code = "var aaapicallform = document.createElement('form'); aaapicallform.method = 'post'; aaapicallform.action = 'http://www.aarcadeapicall.com.net.org/'; var aaapicallinput = document.createElement('input'); aaapicallinput.name = 'doc'; aaapicallinput.type = 'text'; aaapicallinput.value = 'tester joint'; aaapicallform.appendChild(aaapicallinput); document.body.appendChild(aaapicallform); aaapicallform.submit();";
		//pSteamBrowserInstance->InjectJavaScript(code.c_str());

		std::string mainCode;
		std::string code;
		std::string requestId = WebStringToCharString(args[0].ToString());
		std::string scraperId = WebStringToCharString(args[1].ToString());
		if (scraperId == "importSteamGames")
		{
			code = g_pAnarchyManager->GetSteamGamesCode(requestId);

			mainCode = code;

			//rgGames
			//code = "document.location = 'http://www.smsithlord.com/';";
		}
		else
		{
			DevMsg("Injecting DOM getting code...\n");
			code = "document.location = 'http://www.aarcadeapicall.com.net.org/?doc=";
			code += requestId;
			code += "AAAPICALL' + encodeURIComponent(document.documentElement.outerHTML);";

			mainCode = "if (document.readyState === 'complete'){";
			mainCode += code;
			mainCode += "}else{document.addEventListener('DOMContentLoaded', function() {";
			mainCode += code;
			mainCode += "});}";
		}

		DevMsg("Main code is: %s\n", mainCode.c_str());
		pSteamBrowserInstance->InjectJavaScript(mainCode);

		//pSteamBrowserInstance->InjectJavaScript("document.title = 'tester joint';");
		//pSteamBrowserInstance->InjectJavaScript("window.status = 'testerrrrrrrrr';");// document.location = 'http://jk.smsithlord.com/'; ");// window.status = document.documentElement.innerHTML; ");
	}
	else if (method_name == WSLit("metaSearch"))
	{
		std::string scraperId = WebStringToCharString(args[0].ToString());
		std::string itemId = WebStringToCharString(args[1].ToString());
		std::string field = WebStringToCharString(args[2].ToString());
		std::string query = WebStringToCharString(args[3].ToString());

		//if (itemId == "")
			g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);

		//KeyValues* pItem = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(itemId);

		//KeyValues* active = pItem->FindKey("current");
		//if (!active)
		//	active = pItem->FindKey("local", true);

		//if (active)
		//{
			//bool bIsEntityInstance = false;


			int oldOriginalEntIndex = -1;
			std::string oldOriginalItemId;

			C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
			C_SteamBrowserInstance* pBrowserInstance = dynamic_cast<C_SteamBrowserInstance*>(pEmbeddedInstance);
			if (!pBrowserInstance)
			{
				std::string oldId;
				if (pEmbeddedInstance && pEmbeddedInstance->GetId() != "hud")
				{
					oldId = pEmbeddedInstance->GetId();
					oldOriginalEntIndex = pEmbeddedInstance->GetOriginalEntIndex();
					oldOriginalItemId = pEmbeddedInstance->GetOriginalItemId();

					// close this instance
					DevMsg("Removing embedded Instance ID: %s\n", oldId.c_str());

					pEmbeddedInstance->Blur();
					pEmbeddedInstance->Deselect();
					g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);
					pEmbeddedInstance->Close();
				}
				else
					oldId = "scrape" + std::string(g_pAnarchyManager->GenerateUniqueId());

				pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->CreateSteamBrowserInstance();
				pBrowserInstance->SetActiveScraper(scraperId, itemId, field);
				pBrowserInstance->Init(oldId, query.c_str(), "Meta Search", null);
			}
			else
			{
				pBrowserInstance->SetActiveScraper(scraperId, itemId, field);
				pBrowserInstance->SetUrl(query);	// reuse the current focused steam browser if it exists
			}

			//pBrowserInstance->SetActiveScraper(scraperId, itemId, field);
				
			pBrowserInstance->Select();
			pBrowserInstance->Focus();

			if (oldOriginalEntIndex >= 0 )
				pBrowserInstance->SetOriginalEntIndex(oldOriginalEntIndex);

			if (oldOriginalItemId != "")
				pBrowserInstance->SetOriginalItemId(oldOriginalItemId);

			g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance((C_EmbeddedInstance*)pBrowserInstance);
			//if (g_pAnarchyManager->GetSelectedEntity())
				//g_pAnarchyManager->DeselectEntity("asset://ui/overlay.html");
			//else

				C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
				pHudBrowserInstance->SetUrl("asset://ui/overlay.html");

		//	g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);

			//if (itemId == "")
				g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, pBrowserInstance);

			g_pAnarchyManager->GetInputManager()->GetInputSlate()->AdjustOverlay(0, 0, 1, 1, "");
		//}
	}
	else if (method_name == WSLit("saveLibretroOption"))
	{
		std::string type = WebStringToCharString(args[0].ToString());
		std::string name_internal = WebStringToCharString(args[1].ToString());
		std::string value = WebStringToCharString(args[2].ToString());

		// get the active libretro instance
		//C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->GetFocusedLibretroInstance();
		C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->GetSelectedLibretroInstance();
		if (pLibretroInstance)
			pLibretroInstance->SaveLibretroOption(type, name_internal, value);
	}
	else if (method_name == WSLit("spawnNearestObject"))
	{
		C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");

		g_pAnarchyManager->GetInstanceManager()->SetNearestSpawnDist(args[0].ToDouble());
		pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Spawning Objects", "spawningobjects", "0", VarArgs("%i", g_pAnarchyManager->GetInstanceManager()->GetUnspawnedWithinRangeEstimate()), "+0", "spawnNextObjectCallback");
	}
	else if (method_name == WSLit("spawnNextObjectCallback"))
	{
		C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
		//C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();

		bool bSpawned = g_pAnarchyManager->GetInstanceManager()->SpawnNearestObject();
		if (bSpawned)
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Spawning Objects", "spawningobjects", "0", VarArgs("%i", g_pAnarchyManager->GetInstanceManager()->GetUnspawnedWithinRangeEstimate()), "+", "spawnNextObjectCallback");
		else
		{
			ConVar* pConVar = cvar->FindVar("engine_no_focus_sleep");
			pConVar->SetValue(g_pAnarchyManager->GetOldEngineNoFocusSleep().c_str());

			// Map has successfully loaded, so remember that.
			cvar->FindVar("last_map_loaded")->SetValue(1);
			engine->ClientCmd("host_writeconfig");

			//pHudBrowserInstance->SetUrl("asset://ui/default.html");
			ConVar* pHostNextMapConVar = cvar->FindVar("host_next_map");
			if (pHostNextMapConVar->GetBool())
			{
				pHostNextMapConVar->SetValue(false);
				pHudBrowserInstance->SetUrl("asset://ui/hostSessionProgress.html");
			}
			else
			{
				g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
			}
		}
	}
	else if (method_name == WSLit("fileBrowse"))
	{
		std::string browseId = WebStringToCharString(args[0].ToString());
		g_pAnarchyManager->ShowFileBrowseMenu(browseId);
	}
	else if (method_name == WSLit("removeAppFilepath"))
	{
		std::string appId = WebStringToCharString(args[0].ToString());
		std::string appFilepathId = WebStringToCharString(args[1].ToString());
		KeyValues* pAppKV = g_pAnarchyManager->GetMetaverseManager()->GetLibraryApp(appId);
		KeyValues* pActiveKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pAppKV);
		if (pActiveKV)
		{
			if (appFilepathId != "undefined")
			{
				KeyValues* pFilepathKV = pActiveKV->FindKey(VarArgs("filepaths/%s", appFilepathId.c_str()));
				if (pFilepathKV)
				{
					pActiveKV->FindKey("filepaths")->RemoveSubKey(pFilepathKV);
					g_pAnarchyManager->GetMetaverseManager()->SaveApp(pAppKV);
				}
			}
			else
			{
				// clean out corrupt KVs
				std::vector<KeyValues*> victims;
				KeyValues* pFilepathsKV = pActiveKV->FindKey("filepaths", true);
				for (KeyValues *sub = pFilepathsKV->GetFirstSubKey(); sub; sub = sub->GetNextKey())
				{
					if (!Q_strcmp(sub->GetString("id"), ""))
						victims.push_back(sub);
				}

				unsigned int max = victims.size();
				for (unsigned int i = 0; i < max; i++)
					pFilepathsKV->RemoveSubKey(victims[i]);
				victims.clear();
			}
		}
	}
	else if (method_name == WSLit("saveLibretroKeybind"))
	{
		// params: type, name, retrokey, steamkey
		std::string type = WebStringToCharString(args[0].ToString());
		unsigned int retroport = args[1].ToInteger();
		unsigned int retrodevice = args[2].ToInteger();// g_pAnarchyManager->GetLibretroManager()->StringToRetroDevice(WebStringToCharString(args[2].ToString()));
		unsigned int retroindex = args[3].ToInteger();
		unsigned int retrokey = args[4].ToInteger();
		//if (retrodevice != 3)
//			retrokey = g_pAnarchyManager->GetLibretroManager()->StringToRetroKey(WebStringToCharString(args[4].ToString()));
	//	else
		//	retrokey = g_pAnarchyManager->GetLibretroManager()->StringToRetroKeyboardKey(WebStringToCharString(args[4].ToString()));

		//std::string retrotype = WebStringToCharString(args[3].ToString());
		//std::string retrokey = WebStringToCharString(args[4].ToString());
		std::string steamkey = WebStringToCharString(args[5].ToString());

		// get the active libretro instance
		//C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->GetFocusedLibretroInstance();
		C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->GetSelectedLibretroInstance();
		if (pLibretroInstance)
			pLibretroInstance->SaveLibretroKeybind(type, retroport, retrodevice, retroindex, retrokey, steamkey);
	}
	else if (method_name == WSLit("simpleImageReady"))
	{
		//DevMsg("Simple image ready msg recieved.\n");
	//	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
		//C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();

		C_AwesomiumBrowserInstance* pImagesBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("images");

//		C_WebTab* pWebTab = g_pAnarchyManager->GetWebManager()->GetWebBrowser()->FindWebTab(caller);
		//if (pWebTab->GetId() != "images")
		if (pImagesBrowserInstance->GetWebView() != caller || g_pAnarchyManager->GetSuspendEmbedded())
			return;// JSValue(false);

		std::string sessionId = WebStringToCharString(args[0].ToString());
		if (sessionId != "ready" && sessionId != pImagesBrowserInstance->GetImagesSessionId())
			return;

		std::string channel = WebStringToCharString(args[1].ToString());
		std::string itemId = WebStringToCharString(args[2].ToString());
		std::string field = WebStringToCharString(args[3].ToString());
		std::string imageUrl = WebStringToCharString(args[4].ToString());
		bool bFromCache = args[5].ToBoolean();

		if (channel == "" && itemId == "" && field == "")
		{
			pImagesBrowserInstance->SetNumImagesLoading(0);
			//pWebTab->DecrementNumImagesLoading();
		}
		else if (channel != "" && itemId != "" && field != "" && imageUrl != "")
		{
		//	DevMsg("Given: %s, %s, %s\n", field.c_str(), itemId.c_str(), channel.c_str());
			KeyValues* item = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(itemId);
			if (item)
			{
				KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(item);

				KeyValues* fieldKv = active->FindKey(field.c_str());
				if (fieldKv)
				{
				//	DevMsg("Entry A\n");
				//	DevMsg("Field val is: %s\n", fieldKv->GetString());
					std::string imageId = g_pAnarchyManager->GenerateLegacyHash(fieldKv->GetString());
				//	DevMsg("Post hash\n");

					if (!bFromCache)
					{
						C_AwesomiumBrowserInstance* pImagesTab = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("images");
						pImagesTab->SaveImageToCache(imageUrl);// fieldKv->GetString());
					}

					// URLs that are already loaded would have been provided in the blacklist, so the rendered URL should always be fresh at this point.
					std::string textureName = "image_";
					textureName += imageId;

					int iWidth = AA_THUMBNAIL_SIZE;
					int iHeight = AA_THUMBNAIL_SIZE;

					int multiplyer = g_pAnarchyManager->GetDynamicMultiplyer();
					ITexture* pTexture = null;
					//ITexture* pTexture = g_pMaterialSystem->CreateProceduralTexture(textureName.c_str(), TEXTURE_GROUP_MODEL, iWidth, iHeight, IMAGE_FORMAT_BGR888, 1);
					if (!g_pMaterialSystem->IsTextureLoaded(textureName.c_str()))
						pTexture = g_pMaterialSystem->CreateProceduralTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, iWidth * multiplyer, iHeight * multiplyer, IMAGE_FORMAT_BGR888, 1);
					else
						pTexture = g_pMaterialSystem->FindTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, false, 1);

					if (!pTexture)
						DevMsg("Failed to create texture!\n");

					// get the regen and assign it
					CCanvasRegen* pRegen = g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen();
					pTexture->SetTextureRegenerator(pRegen);
					
					pRegen->SetEmbeddedInstance(pImagesBrowserInstance);

					//DevMsg("Exit A\n");
				//	DevMsg("Prepare\n");
					pTexture->Download();
				//	DevMsg("Checkpiont A\n");
				//	DevMsg("Download\n");
					
					pImagesBrowserInstance->OnSimpleImageReady(sessionId, channel, itemId, field, pTexture);
				}
			}
		}
		else if (itemId != "")
		{
			pImagesBrowserInstance->OnSimpleImageReady(sessionId, channel, itemId, field, null);
		}

		return;// JSValue(true);
	}
}

void AddSubKeys(KeyValues* kv, JSObject& object, bool bConvertPropertyNamesToLowercase = false)
{
	if (!kv)
		return;

	std::string lowerBuf;
	//bConvertPropertyNamesToLowercase
	for (KeyValues *sub = kv->GetFirstSubKey(); sub; sub = sub->GetNextKey())
	{
		if (sub->GetFirstSubKey())
		{
			JSObject subObject;
			AddSubKeys(sub, subObject, bConvertPropertyNamesToLowercase);

			if (bConvertPropertyNamesToLowercase && Q_strcmp(sub->GetName(), AA_PLATFORM_ID) )	// don't lower-case the platform ID node!
			{
				lowerBuf = sub->GetName();
				std::transform(lowerBuf.begin(), lowerBuf.end(), lowerBuf.begin(), ::tolower);
				object.SetProperty(WSLit(lowerBuf.c_str()), subObject);
			}
			else
				object.SetProperty(WSLit(sub->GetName()), subObject);
		}
		else
		{
			if (bConvertPropertyNamesToLowercase)
			{
				lowerBuf = sub->GetName();
				std::transform(lowerBuf.begin(), lowerBuf.end(), lowerBuf.begin(), ::tolower);
				object.SetProperty(WSLit(lowerBuf.c_str()), WSLit(sub->GetString()));
			}
			else
				object.SetProperty(WSLit(sub->GetName()), WSLit(sub->GetString()));
		}
	}
}

JSValue JSHandler::OnMethodCallWithReturnValue(WebView* caller, unsigned int remote_object_id, const WebString &method_name, const JSArray &args)
{
	
	if (method_name == WSLit("getAllLibraryTypes"))
	{
		int count = 0;
		//JSArray response;
		JSObject types;
		KeyValues* active;
		KeyValues* pType = g_pAnarchyManager->GetMetaverseManager()->GetFirstLibraryType();
		while (pType != null)	// for some reason, GetNextLibraryType was returning non-null even when there was nothing left to return!!  the non-null value was negative, so this line was adjusted.
		{
			count++;

			active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pType);
			JSObject typeObject;
			AddSubKeys(active, typeObject);
			types.SetProperty(WSLit(active->GetString("info/id")), typeObject);

			pType = g_pAnarchyManager->GetMetaverseManager()->GetNextLibraryType();
		}
		return types;
	}
	else if (method_name == WSLit("getLibraryBrowserContext"))
	{
		JSObject response;
		response.SetProperty(WSLit("category"), WSLit(g_pAnarchyManager->GetMetaverseManager()->GetLibraryBrowserContext("category").c_str()));
		response.SetProperty(WSLit("id"), WSLit(g_pAnarchyManager->GetMetaverseManager()->GetLibraryBrowserContext("id").c_str()));
		response.SetProperty(WSLit("filter"), WSLit(g_pAnarchyManager->GetMetaverseManager()->GetLibraryBrowserContext("filter").c_str()));
		response.SetProperty(WSLit("search"), WSLit(g_pAnarchyManager->GetMetaverseManager()->GetLibraryBrowserContext("search").c_str()));
		return response;
	}
	else if (method_name == WSLit("createItem"))
	{
		// this doesn't seem like it is intended to SAVE the item though.  that is done in a subsequent call, after modifications have been made.

		// build the given info structure
		KeyValues* pInfoKV = new KeyValues("info");

		unsigned int numArgs = args.size();
		if (numArgs == 0)
			return JSValue(0);
		else
		{
			std::string fieldName;
			std::string fieldValue;
			for (unsigned int i = 0; i < numArgs - 1; i = i + 2)
			{
				fieldName = WebStringToCharString(args[i].ToString());
				fieldValue = WebStringToCharString(args[i + 1].ToString());
				pInfoKV->SetString(fieldName.c_str(), fieldValue.c_str());
			}
		}

		//g_pAnarchyManager->GetMetaverseManager()->GetDefaultFields();
		std::string title = pInfoKV->GetString("title");
		std::string description = pInfoKV->GetString("description");
		std::string file = pInfoKV->GetString("file");
		std::string type = pInfoKV->GetString("type");
		std::string app = pInfoKV->GetString("app");
		std::string reference = pInfoKV->GetString("reference");
		std::string preview = pInfoKV->GetString("preview");
		std::string download = pInfoKV->GetString("download");
		std::string stream = pInfoKV->GetString("stream");
		std::string screen = pInfoKV->GetString("screen");
		std::string marquee = pInfoKV->GetString("marquee");
		std::string model = pInfoKV->GetString("model");
		pInfoKV->deleteThis();

		KeyValues* pItemKV = new KeyValues("item");
		bool bCreated = g_pAnarchyManager->GetMetaverseManager()->CreateItem(0, "", pItemKV, title, description, file, type, app, reference, preview, download, stream, screen, marquee, model);
		if ( pItemKV )
		{
			KeyValues* pActiveKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pItemKV);
			if (pActiveKV)
			{
				JSObject item;
				AddSubKeys(pActiveKV, item);

				pItemKV->deleteThis();
				return item;
			}
			else
			{
				pItemKV->deleteThis();
				return JSValue(0);
			}
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("createModel"))
	{
		// TODO: This should be generalized because it's also done when model scanning & import.
		KeyValues* pDataKV = new KeyValues("data");

		unsigned int numArgs = args.size();
		if (numArgs == 0)
			return JSValue(0);
		else
		{
			std::string fieldName;
			std::string fieldValue;
			for (unsigned int i = 0; i < numArgs - 1; i = i + 2)
			{
				fieldName = WebStringToCharString(args[i].ToString());
				fieldValue = WebStringToCharString(args[i + 1].ToString());
				pDataKV->SetString(fieldName.c_str(), fieldValue.c_str());
			}
		}

		//g_pAnarchyManager->GetMetaverseManager()->GetDefaultFields();
		// FIXME: Why are all the other fields disabled here?  Probably because modelFile is the only value available at the places in the UI where models can be created??
		//std::string title = pDataKV->GetString("title");
		std::string modelFile = pDataKV->GetString("file");
		//std::string screen = pDataKV->GetString("screen");
		//std::string keywords = pDataKV->GetString("keywords");
		//std::string mountIds = pDataKV->GetString("mountIds");
		//std::string workshopIds = pDataKV->GetString("workshopIds");
		//std::string download = pDataKV->GetString("download");
		//std::string preview = pDataKV->GetString("preview");
		pDataKV->deleteThis();

		KeyValues* pModel = g_pAnarchyManager->GetMetaverseManager()->CreateModelFromFileTarget(modelFile);
		if (pModel)
		{
			g_pAnarchyManager->GetMetaverseManager()->AddModel(pModel);
			g_pAnarchyManager->GetMetaverseManager()->SaveModel(pModel);

			g_pAnarchyManager->AddToastMessage("New Model Added");

			JSObject model;
			AddSubKeys(g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pModel), model);
			return model;
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("createApp"))
	{
		KeyValues* pDataKV = new KeyValues("data");

		unsigned int numArgs = args.size();
		if (numArgs == 0)
			return JSValue(0);
		else
		{
			std::string fieldName;
			std::string fieldValue;
			for (unsigned int i = 0; i < numArgs - 1; i = i + 2)
			{
				fieldName = WebStringToCharString(args[i].ToString());
				fieldValue = WebStringToCharString(args[i + 1].ToString());
				pDataKV->SetString(fieldName.c_str(), fieldValue.c_str());
			}
		}

		//g_pAnarchyManager->GetMetaverseManager()->GetDefaultFields();
		std::string title = pDataKV->GetString("title");
		std::string file = pDataKV->GetString("file");
		std::string commandformat = pDataKV->GetString("commandformat");
		std::string description = pDataKV->GetString("description");
		std::string type = pDataKV->GetString("type");
		std::string reference = pDataKV->GetString("reference");
		std::string download = pDataKV->GetString("download");
		std::string screen = pDataKV->GetString("screen");
		pDataKV->deleteThis();

		std::string appId = g_pAnarchyManager->GenerateUniqueId();

		KeyValues* pAppKV = new KeyValues("app");
		pAppKV->SetInt("generation", 3);

		KeyValues* pActiveKV = pAppKV->FindKey("local", true);
		pActiveKV->SetString("info/owner", "local");
		pActiveKV->SetString("info/id", appId.c_str());
		pActiveKV->SetString("info/created", VarArgs("%llu", g_pAnarchyManager->GetTimeNumber()));

		pActiveKV->SetString("title", title.c_str());
		pActiveKV->SetString("file", file.c_str());
		pActiveKV->SetString("commandformat", commandformat.c_str());
		pActiveKV->SetString("description", description.c_str());
		pActiveKV->SetString("type", type.c_str());
		pActiveKV->SetString("reference", reference.c_str());
		pActiveKV->SetString("download", download.c_str());
		pActiveKV->SetString("screen", screen.c_str());
		pActiveKV->FindKey("filepaths", true);

		g_pAnarchyManager->GetMetaverseManager()->AddApp(pAppKV);
		g_pAnarchyManager->GetMetaverseManager()->SaveApp(pAppKV);

		g_pAnarchyManager->AddToastMessage("New Open-With App Profile Created");

		JSObject app;
		AddSubKeys(pActiveKV, app);
		return app;
	}
	else if (method_name == WSLit("createType"))
	{
		KeyValues* pDataKV = new KeyValues("data");

		unsigned int numArgs = args.size();
		if (numArgs == 0)
			return JSValue(0);
		else
		{
			std::string fieldName;
			std::string fieldValue;
			for (unsigned int i = 0; i < numArgs - 1; i = i + 2)
			{
				fieldName = WebStringToCharString(args[i].ToString());
				fieldValue = WebStringToCharString(args[i + 1].ToString());
				pDataKV->SetString(fieldName.c_str(), fieldValue.c_str());
			}
		}

		//g_pAnarchyManager->GetMetaverseManager()->GetDefaultFields();	// FIXME: Make this default fields thing actually be used more widely than JUST for items!!
		std::string title = pDataKV->GetString("title");
		/*
		std::string titleformat = pDataKV->GetString("titleformat");
		std::string fileformat = pDataKV->GetString("fileformat");
		std::string priority = pDataKV->GetString("priority");
		*/
		pDataKV->deleteThis();

		std::string typeId = g_pAnarchyManager->GenerateUniqueId();

		KeyValues* pTypeKV = new KeyValues("type");
		pTypeKV->SetInt("generation", 3);

		KeyValues* pActiveKV = pTypeKV->FindKey("local", true);
		pActiveKV->SetString("info/owner", "local");
		pActiveKV->SetString("info/id", typeId.c_str());
		pActiveKV->SetString("info/created", VarArgs("%llu", g_pAnarchyManager->GetTimeNumber()));

		pActiveKV->SetString("title", title.c_str());

		/*
		pActiveKV->SetString("titleformat", titleformat.c_str());
		pActiveKV->SetString("fileformat", fileformat.c_str());
		pActiveKV->SetString("priority", priority.c_str());
		*/

		g_pAnarchyManager->GetMetaverseManager()->AddType(pTypeKV);
		g_pAnarchyManager->GetMetaverseManager()->SaveType(pTypeKV);

		g_pAnarchyManager->AddToastMessage("New Type Profile Created");

		JSObject type;
		AddSubKeys(pActiveKV, type);
		return type;
	}
	else if (method_name == WSLit("getAllLibraryApps"))
	{
		int count = 0;
		//JSArray response;
		JSObject apps;
		KeyValues* active;
		KeyValues* pApp = g_pAnarchyManager->GetMetaverseManager()->GetFirstLibraryApp();
		while (pApp != null)	// for some reason, GetNextLibraryType was returning non-null even when there was nothing left to return!!  the non-null value was negative, so this line was adjusted.
		{
			count++;

			active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pApp);
			JSObject appObject;
			AddSubKeys(active, appObject);
			apps.SetProperty(WSLit(active->GetString("info/id")), appObject);

			pApp = g_pAnarchyManager->GetMetaverseManager()->GetNextLibraryApp();
		}
		return apps;
	}
	else if (method_name == WSLit("didCancelPopupMenu"))
	{
		//DevMsg("Disabled for testing\n");
		std::string id = WebStringToCharString(args[0].ToString());

		//C_WebTab* pWebTab;
		C_AwesomiumBrowserInstance* pAwesomiumBrowserInstance = null;
		if (id == "hud")
			pAwesomiumBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");//g_pAnarchyManager->GetWebManager()->GetHudWebTab();
		else
			pAwesomiumBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance(id);
			//pWebTab = g_pAnarchyManager->GetWebManager()->FindWebTab(id);

		//WebView* pWebView = g_pAnarchyManager->GetWebManager()->GetWebBrowser()->FindWebView(pWebTab);
		//pWebView->DidCancelPopupMenu();
		pAwesomiumBrowserInstance->GetWebView()->DidCancelPopupMenu();
		return JSValue(true);
	}
	else if (method_name == WSLit("didSelectPopupMenuItem"))
	{
		//DevMsg("Disabled for testing\n");

		std::string id = WebStringToCharString(args[0].ToString());
		int itemIndex = args[1].ToInteger();

		C_AwesomiumBrowserInstance* pAwesomiumBrowserInstance = null;
		//C_WebTab* pWebTab;
		if (id == "hud")
			pAwesomiumBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");//g_pAnarchyManager->GetWebManager()->GetHudWebTab();
		else
			pAwesomiumBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance(id);
			//pWebTab = g_pAnarchyManager->GetWebManager()->GetHudWebTab();
		//else
		//	pWebTab = g_pAnarchyManager->GetWebManager()->FindWebTab(id);

	//	WebView* pWebView = g_pAnarchyManager->GetWebManager()->GetWebBrowser()->FindWebView(pWebTab);
	//	pWebView->DidSelectPopupMenuItem(itemIndex);
		pAwesomiumBrowserInstance->GetWebView()->DidSelectPopupMenuItem(itemIndex);
		return JSValue(true);
	}
	else if (method_name == WSLit("getLibraryType"))
	{
		std::string id = WebStringToCharString(args[0].ToString());

		KeyValues* pType = g_pAnarchyManager->GetMetaverseManager()->GetLibraryType(id);
		if (pType)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pType);
			JSObject type;
			AddSubKeys(active, type);
			return type;
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("getLibraryApp"))
	{
		std::string id = WebStringToCharString(args[0].ToString());

		KeyValues* pApp = g_pAnarchyManager->GetMetaverseManager()->GetLibraryApp(id);
		if (pApp)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pApp);
			JSObject app;
			AddSubKeys(active, app);
			return app;
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("getNearestObjectToPlayerLook"))
	{
		object_t* pObject = g_pAnarchyManager->GetInstanceManager()->GetNearestObjectToPlayerLook();

		JSObject response;	// response = {"success": BOOL, "entry": OBJECT, "queryId": STRING}
		std::string queryId = "singleton";

		response.SetProperty(WSLit("queryId"), WSLit(queryId.c_str()));

		if (pObject)
		{
			JSObject entry;
			entry.SetProperty(WSLit("id"), WSLit(pObject->objectId.c_str()));
			entry.SetProperty(WSLit("itemId"), WSLit(pObject->itemId.c_str()));
			entry.SetProperty(WSLit("modelId"), WSLit(pObject->modelId.c_str()));

			std::string title = "Unnamed";
			KeyValues* pItemKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(pObject->itemId));
			KeyValues* pModelKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->GetLibraryModel(pObject->modelId));
			if (pItemKV)
				title = pItemKV->GetString("title");
			else
				title = pModelKV->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID));

			entry.SetProperty(WSLit("title"), WSLit(title.c_str()));

			response.SetProperty(WSLit("entry"), entry);
			response.SetProperty(WSLit("success"), JSValue(true));
		}
		else
			response.SetProperty(WSLit("success"), JSValue(false));

		g_pAnarchyManager->SetLastNearestObjectToPlayerLook(pObject);

		return response;
	}
	else if (method_name == WSLit("getNextNearestObjectToPlayerLook"))
	{
		object_t* pLastObject = g_pAnarchyManager->GetLastNearestObjectToPlayerLook();
		object_t* pObject = null;

		/*
		C_PropShortcutEntity* pLastHoverGlowEntity = dynamic_cast<C_PropShortcutEntity*>(g_pAnarchyManager->GetLastHoverGlowEntity());
		if (pLastHoverGlowEntity)
			pLastObject = g_pAnarchyManager->GetInstanceManager()->GetInstanceObject(pLastHoverGlowEntity->GetObjectId());
		*/

		// TODO: Make this a context, like library searching.
		pObject = g_pAnarchyManager->GetInstanceManager()->GetNearestObjectToPlayerLook(pLastObject);

		JSObject response;	// response = {"success": BOOL, "entry": OBJECT, "queryId": STRING}
		std::string queryId = "singleton";

		response.SetProperty(WSLit("queryId"), WSLit(queryId.c_str()));

		if (pObject)
		{
			JSObject entry;
			//AddSubKeys(pActive, entry);
			entry.SetProperty(WSLit("id"), WSLit(pObject->objectId.c_str()));
			entry.SetProperty(WSLit("itemId"), WSLit(pObject->itemId.c_str()));
			entry.SetProperty(WSLit("modelId"), WSLit(pObject->modelId.c_str()));

			std::string title = "Unnamed";
			KeyValues* pItemKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(pObject->itemId));
			KeyValues* pModelKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->GetLibraryModel(pObject->modelId));
			if (pItemKV)
				title = pItemKV->GetString("title");
			else
				title = pModelKV->GetString(VarArgs("platforms/%s/file", AA_PLATFORM_ID));

			entry.SetProperty(WSLit("title"), WSLit(title.c_str()));

			response.SetProperty(WSLit("entry"), entry);
			response.SetProperty(WSLit("success"), JSValue(true));
		}
		else
			response.SetProperty(WSLit("success"), JSValue(false));

		g_pAnarchyManager->SetLastNearestObjectToPlayerLook(pObject);

		return response;
	}
	else if (method_name == WSLit("getNodeSetupInfo"))
	{
		JSObject response;

		JSObject nodeInfo;
		KeyValues* pNodeInfoKV = new KeyValues("node");
		if (!pNodeInfoKV->LoadFromFile(g_pFullFileSystem, "nodevolume.txt", "DEFAULT_WRITE_PATH"))
		{
			DevMsg("ERROR: Could not load nodevolume.txt!\n");
			response.SetProperty(WSLit("success"), JSValue(false));
		}
		else
		{
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

				// First, determine if this is an item with the node's info by:
				//	a. check its model
				//	b. check its item
				// If it is, set pInfoShortcut to pPropShortcutEntity.

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

			// Next, if there is a pInfoShortcut, get the node item assigned to this shortcut.
			// TODO: work

			// Then get the instance KV of the node and save it to pInstanceKV so it can be used to detect changes.
			// TODO: work

			KeyValues* pNodeInfoSetupKV = pNodeInfoKV->FindKey("setup", true);
			nodeInfo.SetProperty(WSLit("style"), WSLit(pNodeInfoSetupKV->GetString("style")));

			bool bHasChanged = false;
			JSArray objects;
			object_t* pObject;
			KeyValues* pObjectInfo;
			KeyValues* pItemInfo;
			KeyValues* pModelInfo;
			KeyValues* pNodeObjectsKV = pNodeInfoSetupKV->FindKey("objects", true);
			int arrayIndex = 0;
			for (KeyValues *sub = pNodeObjectsKV->GetFirstSubKey(); sub; sub = sub->GetNextKey())
			{
				// loop through it adding all the info to the response object.
				pBaseEntity = C_BaseEntity::Instance(sub->GetInt());
				if (!pBaseEntity)
					continue;

				pPropShortcutEntity = dynamic_cast<C_PropShortcutEntity*>(pBaseEntity);
				if (!pPropShortcutEntity)
					continue;

				JSObject entry;
				if (pPropShortcutEntity == pInfoShortcut)
					nodeInfo.SetProperty(WSLit("infoObjectIndex"), JSValue(arrayIndex));

				pObject = g_pAnarchyManager->GetInstanceManager()->GetInstanceObject(pPropShortcutEntity->GetObjectId());
				pObjectInfo = null;
				pItemInfo = null;
				pModelInfo = null;

				g_pAnarchyManager->GetMetaverseManager()->GetObjectInfo(pObject, pObjectInfo, pItemInfo, pModelInfo);

				if (!bHasChanged)
				{
					// Then, using the info just obtained, search through pInstanceKV & find this objects match within it.
					// If it does not have a match, set bHasChanged = true
					// TODO: work
				}

				if (pObjectInfo)
				{
					JSObject object;
					AddSubKeys(pObjectInfo, object);
					entry.SetProperty(WSLit("object"), object);

					pObjectInfo->deleteThis();
				}

				if (pItemInfo)
				{
					JSObject item;
					AddSubKeys(pItemInfo, item);
					entry.SetProperty(WSLit("item"), item);

					pItemInfo->deleteThis();
				}

				if (pModelInfo)
				{
					JSObject model;
					AddSubKeys(pModelInfo, model);
					entry.SetProperty(WSLit("model"), model);

					pModelInfo->deleteThis();
				}

				objects.Push(entry);
				arrayIndex++;
			}

			nodeInfo.SetProperty(WSLit("hasChanged"), JSValue(bHasChanged));
			nodeInfo.SetProperty(WSLit("objects"), objects);
			response.SetProperty(WSLit("success"), JSValue(true));
		}
		pNodeInfoKV->deleteThis();

		response.SetProperty(WSLit("nodeInfo"), nodeInfo);
		return response;
	}
	else if (method_name == WSLit("getAllUserChat"))
	{
		JSArray userChats;
		aampConnection_t* pConnection = g_pAnarchyManager->GetConnectedUniverse();
		if (pConnection && pConnection->connected)
		{
			// go through ALL users & grab the last thing they've said
			std::vector<user_t*> users;
			g_pAnarchyManager->GetMetaverseManager()->GetAllInstanceUsers(users);

			for (unsigned int i = 0; i < users.size(); i++)
			{
				JSObject chatEntry;
				chatEntry.SetProperty(WSLit("userId"), WSLit(users[i]->userId.c_str()));
				chatEntry.SetProperty(WSLit("displayName"), WSLit(users[i]->displayName.c_str()));
				chatEntry.SetProperty(WSLit("say"), WSLit(users[i]->say.c_str()));
				userChats.Push(chatEntry);
			}
		}
		
		return userChats;
	}
	else if (method_name == WSLit("getAllUsers"))
	{
		JSArray responseUsers;

		aampConnection_t* pConnection = g_pAnarchyManager->GetConnectedUniverse();
		if (pConnection && pConnection->connected)
		{
			// go through ALL users & grab the last thing they've said
			std::vector<user_t*> users;
			g_pAnarchyManager->GetMetaverseManager()->GetAllInstanceUsers(users);

			for (unsigned int i = 0; i < users.size(); i++)
			{
				JSObject user;
				user.SetProperty(WSLit("userId"), WSLit(users[i]->userId.c_str()));
				user.SetProperty(WSLit("sessionId"), WSLit(users[i]->sessionId.c_str()));
				user.SetProperty(WSLit("followingId"), WSLit(users[i]->followingId.c_str()));
				user.SetProperty(WSLit("displayName"), WSLit(users[i]->displayName.c_str()));
				user.SetProperty(WSLit("itemId"), WSLit(users[i]->itemId.c_str()));
				user.SetProperty(WSLit("objectId"), WSLit(users[i]->objectId.c_str()));
				user.SetProperty(WSLit("say"), WSLit(users[i]->say.c_str()));
				if (users[i]->entity)
				{
					user.SetProperty(WSLit("exists"), WSLit("1"));
					user.SetProperty(WSLit("bodyOrigin"), WSLit(VarArgs("%.10f %.10f %.10f", users[i]->bodyOrigin.x, users[i]->bodyOrigin.y, users[i]->bodyOrigin.z)));
					user.SetProperty(WSLit("bodyAngles"), WSLit(VarArgs("%.10f %.10f %.10f", users[i]->bodyAngles.x, users[i]->bodyAngles.y, users[i]->bodyAngles.z)));
					user.SetProperty(WSLit("headOrigin"), WSLit(VarArgs("%.10f %.10f %.10f", users[i]->headOrigin.x, users[i]->headOrigin.y, users[i]->headOrigin.z)));
					user.SetProperty(WSLit("headAngles"), WSLit(VarArgs("%.10f %.10f %.10f", users[i]->headAngles.x, users[i]->headAngles.y, users[i]->headAngles.z)));
				}
				else
					user.SetProperty(WSLit("exists"), WSLit("0"));
				if (g_pAnarchyManager->GetMetaverseManager()->GetFollowingId() == users[i]->userId)
					user.SetProperty(WSLit("followed"), WSLit("1"));
				else
					user.SetProperty(WSLit("followed"), WSLit("0"));
				if (g_pAnarchyManager->GetMetaverseManager()->GetLocalUser() != users[i] )//!g_pAnarchyManager->GetConnectedUniverse() || g_pAnarchyManager->GetConnectedUniverse()->user != users[i]->userId)
					user.SetProperty(WSLit("local"), WSLit("0"));
				else
					user.SetProperty(WSLit("local"), WSLit("1"));
				user.SetProperty(WSLit("mouseX"), WSLit(users[i]->mouseX.c_str()));
				user.SetProperty(WSLit("mouseY"), WSLit(users[i]->mouseY.c_str()));
				user.SetProperty(WSLit("webUrl"), WSLit(users[i]->webUrl.c_str()));
				user.SetProperty(WSLit("avatarUrl"), WSLit(users[i]->avatarUrl.c_str()));
				responseUsers.Push(user);
			}
		}

		return responseUsers;
	}
	else if (method_name == WSLit("getConnectedSession"))
	{
		JSObject response;
		aampConnection_t* pConnection = g_pAnarchyManager->GetConnectedUniverse();
		if (pConnection && pConnection->connected)
		{
			response.SetProperty(WSLit("server"), WSLit(pConnection->address.c_str()));
			response.SetProperty(WSLit("universe"), WSLit(pConnection->universe.c_str()));
			response.SetProperty(WSLit("instance"), WSLit(pConnection->instance.c_str()));
			response.SetProperty(WSLit("session"), WSLit(pConnection->session.c_str()));
			response.SetProperty(WSLit("lobby"), WSLit(pConnection->lobby.c_str()));
			response.SetProperty(WSLit("isPublic"), JSValue(pConnection->isPublic));
			response.SetProperty(WSLit("lobbyPassword"), WSLit(pConnection->lobbyPassword.c_str()));
			return response;
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("getNumUsers"))
	{
		return JSValue((int)g_pAnarchyManager->GetMetaverseManager()->GetNumInstanceUsers());
	}
	else if (method_name == WSLit("getSyncOverview"))
	{
		if (cvar->FindVar("sync_overview")->GetBool())
		{
			KeyValues* pOverviewKV = new KeyValues("overview");
			if (pOverviewKV->LoadFromFile(g_pFullFileSystem, VarArgs("resource/overviews/%s.txt", g_pAnarchyManager->MapName()), "GAME"))
			{
				JSObject response;
				response.SetProperty(WSLit("map"), WSLit(g_pAnarchyManager->MapName()));
				response.SetProperty(WSLit("pos_x"), WSLit(VarArgs("%i", pOverviewKV->GetInt("pos_x"))));
				response.SetProperty(WSLit("pos_y"), WSLit(VarArgs("%i", pOverviewKV->GetInt("pos_y"))));
				response.SetProperty(WSLit("scale"), WSLit(VarArgs("%.10f", pOverviewKV->GetFloat("scale"))));

				pOverviewKV->deleteThis();
				return response;
			}

			pOverviewKV->deleteThis();
		}
		
		return JSValue(0);
	}
	else if (method_name == WSLit("hasLibraryEntry"))
	{
		std::string category = WebStringToCharString(args[0].ToString());
		std::string id = WebStringToCharString(args[1].ToString());

		// get the first entry
		KeyValues* pEntry;
		if (category == "instances")
		{
			instance_t* pInstance = g_pAnarchyManager->GetInstanceManager()->FindInstance(id);
			return JSValue(!!pInstance);
		}
		else if (category == "items")
			pEntry = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(id);
		else if (category == "models")
			pEntry = g_pAnarchyManager->GetMetaverseManager()->GetLibraryModel(id);
		else if (category == "types")
			pEntry = g_pAnarchyManager->GetMetaverseManager()->GetLibraryType(id);
		else if (category == "apps")
			pEntry = g_pAnarchyManager->GetMetaverseManager()->GetLibraryApp(id);
		else if (category == "maps")
			pEntry = g_pAnarchyManager->GetMetaverseManager()->GetLibraryMap(id);

		pEntry = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pEntry);

		return JSValue(!!pEntry);
	}
	else if (method_name == WSLit("getFirstLibraryEntry"))
	{
		//const char* 
		std::string category = WebStringToCharString(args[0].ToString());

		JSObject response;	// response = {"success": BOOL, "entry": OBJECT, "queryId": STRING}

		// get the first entry
		KeyValues* pEntry;
		//const char*
		std::string queryId = g_pAnarchyManager->GetMetaverseManager()->GetFirstLibraryEntry(pEntry, category.c_str());
		//DevMsg("The query ID here is: %s\n", queryId.c_str());
		response.SetProperty(WSLit("queryId"), WSLit(queryId.c_str()));

		// put the entry into the response
		KeyValues* pActive = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pEntry);
		if (pActive)
		{
			JSObject entry;
			AddSubKeys(pActive, entry);
			response.SetProperty(WSLit("entry"), entry);
			response.SetProperty(WSLit("success"), JSValue(true));
		}
		else
			response.SetProperty(WSLit("success"), JSValue(false));

		return response;
	}
	else if (method_name == WSLit("getNextLibraryEntry"))
	{
		//const char* queryId = WebStringToCharString(args[0].ToString());
		std::string queryId = WebStringToCharString(args[0].ToString());
		//DevMsg("Hereeee the ID is: %s\n", queryId.c_str());

		// FOR TESTING ONLY!! THIS DATA SHOULD BE SAVED AS PART OF THE ORIGINAL QUERY!!
		//const char* 
		std::string debugCategory = WebStringToCharString(args[1].ToString());

		JSObject response;	// response = {"success": BOOL, "entry": OBJECT, "queryId": STRING}
		response.SetProperty(WSLit("queryId"), WSLit(queryId.c_str()));

		// get the next entry
		KeyValues* pEntry = g_pAnarchyManager->GetMetaverseManager()->GetNextLibraryEntry(queryId.c_str(), debugCategory.c_str());

		// put the entry into the response
		KeyValues* pActive = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pEntry);	// return null if given a null argument to avoid the if-then cluster fuck of error checking each step of this common task
		if (pActive)
		{
			JSObject entry;
			AddSubKeys(pActive, entry);
			response.SetProperty(WSLit("entry"), entry);
			response.SetProperty(WSLit("success"), JSValue(true));
		}
		else
			response.SetProperty(WSLit("success"), JSValue(false));

		return response;
	}
	else if (method_name == WSLit("getFirstLibraryItem"))
	{
		KeyValues* pItem = g_pAnarchyManager->GetMetaverseManager()->GetFirstLibraryItem();

		if (pItem)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pItem);
			JSObject item;
			AddSubKeys(active, item);
			return item;
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("getNextLibraryItem"))
	{
		KeyValues* pItem = g_pAnarchyManager->GetMetaverseManager()->GetNextLibraryItem();
		if (pItem)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pItem);
			JSObject item;
			AddSubKeys(active, item);
			return item;
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("findFirstLibraryEntry"))
	{
		// Grab the variables (watch out for thread safety)
		//const char*
		std::string category = WebStringToCharString(args[0].ToString());
		//std::string catBuf = std::string(category);


		JSObject response;	// response = {"success": BOOL, "entry": OBJECT, "queryId": STRING}

		// build the search info w/ the key & value pairs given in args[1] and beyond
		KeyValues* pSearchInfo = new KeyValues("search");	// this gets deleted by the metaverse manager!!
		DevMsg("Cat is: %s\n", category.c_str());
		std::string fieldName;
		std::string fieldValue;
		unsigned int numArgs = args.size();
		for (unsigned int i = 1; i < numArgs - 1; i = i + 2)
		{
			fieldName = WebStringToCharString(args[i].ToString());
			fieldValue = WebStringToCharString(args[i + 1].ToString());
			std::transform(fieldValue.begin(), fieldValue.end(), fieldValue.begin(), ::tolower);
			pSearchInfo->SetString(fieldName.c_str(), fieldValue.c_str());

			DevMsg("Field name & value: %s %s\n", fieldName.c_str(), fieldValue.c_str());
		}

		// find the first entry that matches the search params
		KeyValues* pEntry;
		std::string queryId = g_pAnarchyManager->GetMetaverseManager()->FindFirstLibraryEntry(pEntry, category.c_str(), pSearchInfo);	// EXPENSIVE
		//DevMsg("The query ID here is: %s\n", queryId.c_str());
		response.SetProperty(WSLit("queryId"), WSLit(queryId.c_str()));

		// add the entry to the response
		KeyValues* pActive = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pEntry);
		if (pActive)
		{
			JSObject entry;
			AddSubKeys(pActive, entry);
			response.SetProperty(WSLit("entry"), entry);
			response.SetProperty(WSLit("success"), JSValue(true));
		}
		else
			response.SetProperty(WSLit("success"), JSValue(false));

		return response;
	}
	else if (method_name == WSLit("findNextLibraryEntry"))
	{
		// Grab the variables
		//const char*
		std::string queryId = WebStringToCharString(args[0].ToString());

		// FOR TESTING ONLY!! THIS DATA SHOULD BE SAVED AS PART OF THE ORIGINAL QUERY CONTEXT!!
		//const char*
		std::string debugCategory = WebStringToCharString(args[1].ToString());

		JSObject response;	// response = {"success": BOOL, "entry": OBJECT, "queryId": STRING}
		response.SetProperty(WSLit("queryId"), WSLit(queryId.c_str()));

		// find the next matching entry
		KeyValues* pEntry = g_pAnarchyManager->GetMetaverseManager()->FindNextLibraryEntry(queryId.c_str(), debugCategory.c_str());

		// add the entry to the response
		KeyValues* pActive = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pEntry);
		if (pActive)
		{
			JSObject entry;
			AddSubKeys(pActive, entry);
			response.SetProperty(WSLit("entry"), entry);
			response.SetProperty(WSLit("success"), JSValue(true));
		}
		else
			response.SetProperty(WSLit("success"), JSValue(false));

		return response;
	}
	else if (method_name == WSLit("getSelectedLibraryItem"))
	{
		C_BaseEntity* pEntity = g_pAnarchyManager->GetSelectedEntity();
		if (pEntity)
		{
			C_PropShortcutEntity* pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);
			if (pShortcut)
			{
				std::string id = pShortcut->GetItemId();
				if (id != "")
				{
					KeyValues* pItem = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(id);
					if (pItem)
					{
						KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pItem);
						JSObject item;
						AddSubKeys(active, item);
						return item;
					}
				}
			}
		}

		return JSValue(0);
	}
	else if (method_name == WSLit("getLibraryModel"))
	{
		std::string id = WebStringToCharString(args[0].ToString());

		KeyValues* pModelKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->GetLibraryModel(id));
		if (pModelKV)
		{
			JSObject model;
			AddSubKeys(pModelKV, model, true);
			return model;
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("getLibraryItem"))
	{
		std::string id = WebStringToCharString(args[0].ToString());

		KeyValues* pItem = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(id);
		if (pItem)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pItem);
			JSObject item;
			AddSubKeys(active, item);
			return item;
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("findFirstLibraryItem"))
	{
		DevMsg("findFirstLibraryItem\n");
		// build the search info
		KeyValues* pSearchInfo = new KeyValues("search");	// this gets deleted by the metaverse manager!!

		unsigned int numArgs = args.size();
		if (numArgs == 0)
			return JSValue(0);
		else
		{
			std::string fieldName;
			std::string fieldValue;
			for (unsigned int i = 0; i < numArgs - 1; i = i + 2)
			{
				fieldName = WebStringToCharString(args[i].ToString());
				fieldValue = WebStringToCharString(args[i + 1].ToString());
				DevMsg("Using %s equals %s as search field.\n", fieldName.c_str(), fieldValue.c_str());
				pSearchInfo->SetString(fieldName.c_str(), fieldValue.c_str());
			}
			/*
			JSObject object = args[0].ToObject();
			JSArray properties = object.GetPropertyNames();
			std::string propertyName;
			for (unsigned int i = 0; i < properties.size(); i++)
			{
				propertyName = WebStringToCharString(properties.At(i).ToString());
				DevMsg("Library search field: %s\n", propertyName.c_str());
				pSearchInfo->SetString(propertyName.c_str(), WebStringToCharString(object.GetProperty(properties.At(i).ToString()).ToString()));
			}
			*/
		}

		// start the search
		KeyValues* pItem = g_pAnarchyManager->GetMetaverseManager()->FindFirstLibraryItem(pSearchInfo);

		if (pItem)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pItem);
			JSObject item;
			AddSubKeys(active, item);
			return item;
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("findNextLibraryItem"))
	{
		KeyValues* pItem = g_pAnarchyManager->GetMetaverseManager()->FindNextLibraryItem();
		if (pItem)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pItem);
			JSObject item;
			AddSubKeys(active, item);
			return item;
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("findLibraryItem"))
	{
		// build the search info
		KeyValues* pSearchInfo = new KeyValues("search");	// this gets deleted by the metaverse manager!!

		unsigned int numArgs = args.size();
		if (numArgs == 0)
			return JSValue(0);
		else
		{
			std::string fieldName;
			std::string fieldValue;
			for (unsigned int i = 0; i < numArgs - 1; i = i + 2)
			{
				fieldName = WebStringToCharString(args[i].ToString());
				fieldValue = WebStringToCharString(args[i + 1].ToString());
				DevMsg("Using %s equals %s as search field.\n", fieldName.c_str(), fieldValue.c_str());
				pSearchInfo->SetString(fieldName.c_str(), fieldValue.c_str());
			}

			//for (unsigned int i = 0; i < numArgs; i += 2)
			//	pSearchInfo->SetString(WebStringToCharString(args[i].ToString()), WebStringToCharString(args[i + 2].ToString()));
			/*
			JSObject object = args[0].ToObject();
			JSArray properties = object.GetPropertyNames();
			std::string propertyName;
			for (unsigned int i = 0; i < properties.size(); i++)
			{
				propertyName = WebStringToCharString(properties.At(i).ToString());
				pSearchInfo->SetString(propertyName.c_str(), WebStringToCharString(object.GetProperty(properties.At(i).ToString()).ToString()));
			}
			*/
		}
		//pSearchInfo->SetString("title", WebStringToCharString(args[0].ToString()));
		
		// start the search
		KeyValues* pItem = g_pAnarchyManager->GetMetaverseManager()->FindLibraryItem(pSearchInfo);

		if (pItem)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pItem);
			JSObject item;
			AddSubKeys(active, item);
			return item;
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("findLibraryModel"))
	{
		// build the search info
		KeyValues* pSearchInfo = new KeyValues("search");	// this gets deleted by the metaverse manager!!

		unsigned int numArgs = args.size();
		if (numArgs == 0)
			return JSValue(0);
		else
		{
			std::string fieldName;
			std::string fieldValue;
			for (unsigned int i = 0; i < numArgs - 1; i = i + 2)
			{
				fieldName = WebStringToCharString(args[i].ToString());
				fieldValue = WebStringToCharString(args[i + 1].ToString());
				//DevMsg("Using %s equals %s as search field.\n", fieldName.c_str(), fieldValue.c_str());
				pSearchInfo->SetString(fieldName.c_str(), fieldValue.c_str());
			}

			//for (unsigned int i = 0; i < numArgs; i += 2)
			//	pSearchInfo->SetString(WebStringToCharString(args[i].ToString()), WebStringToCharString(args[i + 2].ToString()));
			/*
			JSObject object = args[0].ToObject();
			JSArray properties = object.GetPropertyNames();
			std::string propertyName;
			for (unsigned int i = 0; i < properties.size(); i++)
			{
			propertyName = WebStringToCharString(properties.At(i).ToString());
			pSearchInfo->SetString(propertyName.c_str(), WebStringToCharString(object.GetProperty(properties.At(i).ToString()).ToString()));
			}
			*/
		}
		//pSearchInfo->SetString("title", WebStringToCharString(args[0].ToString()));

		// start the search
		KeyValues* pModel = g_pAnarchyManager->GetMetaverseManager()->FindLibraryModel(pSearchInfo);

		if (pModel)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pModel);
			JSObject model;
			AddSubKeys(active, model);
			return model;
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("findLibraryApp"))
	{
		// build the search info
		KeyValues* pSearchInfo = new KeyValues("search");	// this gets deleted by the metaverse manager!!

		unsigned int numArgs = args.size();
		if (numArgs == 0)
			return JSValue(0);
		else
		{
			std::string fieldName;
			std::string fieldValue;
			for (unsigned int i = 0; i < numArgs - 1; i = i + 2)
			{
				fieldName = WebStringToCharString(args[i].ToString());
				fieldValue = WebStringToCharString(args[i + 1].ToString());
				DevMsg("Using %s equals %s as search field.\n", fieldName.c_str(), fieldValue.c_str());
				pSearchInfo->SetString(fieldName.c_str(), fieldValue.c_str());
			}

			//for (unsigned int i = 0; i < numArgs; i += 2)
			//	pSearchInfo->SetString(WebStringToCharString(args[i].ToString()), WebStringToCharString(args[i + 2].ToString()));
			/*
			JSObject object = args[0].ToObject();
			JSArray properties = object.GetPropertyNames();
			std::string propertyName;
			for (unsigned int i = 0; i < properties.size(); i++)
			{
			propertyName = WebStringToCharString(properties.At(i).ToString());
			pSearchInfo->SetString(propertyName.c_str(), WebStringToCharString(object.GetProperty(properties.At(i).ToString()).ToString()));
			}
			*/
		}
		//pSearchInfo->SetString("title", WebStringToCharString(args[0].ToString()));

		// start the search
		KeyValues* pApp = g_pAnarchyManager->GetMetaverseManager()->FindLibraryApp(pSearchInfo);

		if (pApp)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pApp);
			JSObject app;
			AddSubKeys(active, app);
			return app;
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("findLibraryType"))
	{
		// build the search info
		KeyValues* pSearchInfo = new KeyValues("search");	// this gets deleted by the metaverse manager!!

		unsigned int numArgs = args.size();
		if (numArgs == 0)
			return JSValue(0);
		else
		{
			std::string fieldName;
			std::string fieldValue;
			for (unsigned int i = 0; i < numArgs - 1; i = i + 2)
			{
				fieldName = WebStringToCharString(args[i].ToString());
				fieldValue = WebStringToCharString(args[i + 1].ToString());
				//DevMsg("Using %s equals %s as search field.\n", fieldName.c_str(), fieldValue.c_str());
				pSearchInfo->SetString(fieldName.c_str(), fieldValue.c_str());
			}
		}

		// start the search
		KeyValues* pType = g_pAnarchyManager->GetMetaverseManager()->FindLibraryType(pSearchInfo);

		if (pType)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pType);
			JSObject type;
			AddSubKeys(active, type);
			return type;
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("saveItem"))
	{
		// construct a new KeyValues for the item
		std::string id = WebStringToCharString(args[0].ToString());
		if (id == "")
			id = g_pAnarchyManager->GenerateUniqueId();

		KeyValues* pItemKV = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(id);
		if (pItemKV)
		{
			DevMsg("ERROR: Item already exists!  Use updateItem instead!\n");
			return JSValue(0);
		}

		// now loop through our updated fields
		KeyValues* pInfoKV = new KeyValues("info");
		bool bNeedsTextureUpdate = false;
		std::string field;
		std::string value;
		JSArray update = args[1].ToArray();
		unsigned int max = update.size();
		for (unsigned int i = 0; i < max; i += 2)
		{
			field = WebStringToCharString(update.At(i).ToString());
			value = WebStringToCharString(update.At(i + 1).ToString());

			//DevMsg("Updating %s with %s\n", field.c_str(), value.c_str());

			// update field with value
			pInfoKV->SetString(field.c_str(), value.c_str());

			// if any of the following fields were changed, the images on the item should be refreshed:
			if (field == "file" || field == "preview" || field == "screen" || field == "marquee")
				bNeedsTextureUpdate = true;
		}

		pItemKV = new KeyValues("item");
		std::string title = pInfoKV->GetString("title");
		std::string description = pInfoKV->GetString("description");
		std::string file = pInfoKV->GetString("file");
		std::string type = pInfoKV->GetString("type");
		std::string app = pInfoKV->GetString("app");
		std::string reference = pInfoKV->GetString("reference");
		std::string preview = pInfoKV->GetString("preview");
		std::string download = pInfoKV->GetString("download");
		std::string stream = pInfoKV->GetString("stream");
		std::string screen = pInfoKV->GetString("screen");
		std::string marquee = pInfoKV->GetString("marquee");
		std::string model = pInfoKV->GetString("model");

		pInfoKV->deleteThis();
		pInfoKV = null;
		bool bCreated = g_pAnarchyManager->GetMetaverseManager()->CreateItem(0, id, pItemKV, title, description, file, type, app, reference, preview, download, stream, screen, marquee, model);

		// push this onto the active library
		g_pAnarchyManager->GetMetaverseManager()->AddItem(pItemKV);

		DevMsg("Saving item now...\n");

		// now save the item's changes
		g_pAnarchyManager->GetMetaverseManager()->SaveItem(pItemKV);
		DevMsg("Done.\n");

		return WSLit(id.c_str());
	}
	else if (method_name == WSLit("generateUniqueId"))
	{
		std::string id = g_pAnarchyManager->GenerateUniqueId();
		return WSLit(id.c_str());
	}
	else if (method_name == WSLit("updateType"))
	{
		std::string id = WebStringToCharString(args[0].ToString());

		KeyValues* pTypeKV = g_pAnarchyManager->GetMetaverseManager()->GetLibraryType(id);
		KeyValues* pActiveKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pTypeKV);
		if (pActiveKV)
		{
			// now loop through our updated fields
			std::string field;
			std::string value;
			JSArray update = args[1].ToArray();
			unsigned int max = update.size();
			for (unsigned int i = 0; i < max; i += 2)
			{
				field = WebStringToCharString(update.At(i).ToString());
				value = WebStringToCharString(update.At(i + 1).ToString());

				// update field with value
				pActiveKV->SetString(field.c_str(), value.c_str());
			}

			// now save the item's changes
			g_pAnarchyManager->GetMetaverseManager()->SaveType(pTypeKV);
			return JSValue(true);
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("updateModel"))
	{
		std::string id = WebStringToCharString(args[0].ToString());

		KeyValues* pModelKV = g_pAnarchyManager->GetMetaverseManager()->GetLibraryModel(id);
		KeyValues* pActiveKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pModelKV);
		if (pActiveKV)
		{
			// now loop through our updated fields
			bool bNeedsModelUpdate = false;
			std::string field;
			std::string value;
			JSArray update = args[1].ToArray();
			unsigned int max = update.size();
			for (unsigned int i = 0; i < max; i += 2)
			{
				//JSObject object = update.At(i).ToObject();
				//field = WebStringToCharString(object.GetProperty(WSLit("field")).ToString());
				//value = WebStringToCharString(object.GetProperty(WSLit("value")).ToString());
				field = WebStringToCharString(update.At(i).ToString());
				value = WebStringToCharString(update.At(i + 1).ToString());

				// update field with value
				pActiveKV->SetString(field.c_str(), value.c_str());

				// if any of the following fields were changed, the images on the item should be refreshed:
				if (field == std::string(VarArgs("platforms/%s/file", AA_PLATFORM_ID)))
					bNeedsModelUpdate = true;
			}

			// now save the item's changes
			g_pAnarchyManager->GetMetaverseManager()->SaveModel(pModelKV);

			if (bNeedsModelUpdate)
			{
				g_pAnarchyManager->GetInstanceManager()->ModelFileChanged(id);
				//g_pAnarchyManager->GetCanvasManager()->RefreshItemTextures(id, "ALL");
				//g_pAnarchyManager->GetCanvasManager()->RefreshItemTextures(id, "screen");
				//g_pAnarchyManager->GetCanvasManager()->RefreshItemTextures(id, "marquee");
			}

			return JSValue(true);
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("checkStartWithWindows"))
	{
		if (g_pAnarchyManager->CheckStartWithWindows())
			return JSValue(WSLit("1"));
		else
			return JSValue(WSLit("0"));
	}
	else if (method_name == WSLit("libretroUpdateDLL"))
	{
		//libretroUpdateDLL(payload.coreFile, payload.enabled, payload.priority, payload.paths)
		unsigned int numArgs = args.size();
		if (numArgs < 6 || !args[0].IsString() || !args[1].IsInteger() || !args[2].IsInteger() || !args[3].IsInteger() || !args[4].IsInteger() || !args[5].IsArray())
		{
			DevMsg("ERROR: My hair is a bird.  Your argument is invalid. (%u)\n", numArgs);
			return JSValue(false);
		}
		else
		{
			KeyValues* pCoreSettingsKV = g_pAnarchyManager->GetLibretroManager()->GetCoreSettingsKV();

			// Find this coreFile's KV
			std::string coreFile = WebStringToCharString(args[0].ToString());

			KeyValues* pCoreEntryKV = null;
			for (KeyValues *sub = pCoreSettingsKV->GetFirstSubKey(); sub; sub = sub->GetNextKey())
			{
				if (!Q_stricmp(sub->GetString("file"), coreFile.c_str()))
				{
					pCoreEntryKV = sub;
					break;
				}
			}

			if (!pCoreEntryKV)
			{
				DevMsg("ERROR: Could not find core settings for %s\n", coreFile.c_str());
				return JSValue(false);
			}
			else
			{
				bool enabled = (args[1].ToInteger()) ? true : false;
				bool cartsaves = (args[2].ToInteger()) ? true : false;
				bool statesaves = (args[3].ToInteger()) ? true : false;
				int priority = args[4].ToInteger();

				pCoreEntryKV->SetBool("enabled", enabled);
				pCoreEntryKV->SetBool("cartsaves", cartsaves);
				pCoreEntryKV->SetBool("statesaves", statesaves);
				pCoreEntryKV->SetInt("priority", priority);

				// Now update the CONTENT FOLDERS
				KeyValues* pCoreEntryPathsKV = pCoreEntryKV->FindKey("paths", true);
				pCoreEntryPathsKV->Clear();

				KeyValues* pPathKV;
				std::string path;
				std::string extensions;
				JSArray paths = args[5].ToArray();
				unsigned int max = paths.size();
				for (unsigned int i = 0; i < max; i += 2)
				{
					path = WebStringToCharString(paths.At(i).ToString());
					extensions = WebStringToCharString(paths.At(i + 1).ToString());

					if (path != "" || extensions != "")
					{
						pPathKV = pCoreEntryPathsKV->CreateNewKey();
						pPathKV->SetName("path");

						pPathKV->SetString("path", path.c_str());
						pPathKV->SetString("extensions", extensions.c_str());
					}
				}

				g_pAnarchyManager->GetLibretroManager()->SaveCoreSettings();
			}
		}

		return JSValue(true);
	}
	else if (method_name == WSLit("updateItem"))
	{
		std::string id = WebStringToCharString(args[0].ToString());

		KeyValues* pItem = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(id);
		if (pItem)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pItem);
			// now loop through our updated fields
			bool bNeedsTextureUpdate = false;
			std::string field;
			std::string value;
			JSArray update = args[1].ToArray();
			unsigned int max = update.size();
			for (unsigned int i = 0; i < max; i += 2)
			{
				//JSObject object = update.At(i).ToObject();
				//field = WebStringToCharString(object.GetProperty(WSLit("field")).ToString());
				//value = WebStringToCharString(object.GetProperty(WSLit("value")).ToString());
				field = WebStringToCharString(update.At(i).ToString());
				value = WebStringToCharString(update.At(i+1).ToString());

				// update field with value
				active->SetString(field.c_str(), value.c_str());

				// if any of the following fields were changed, the images on the item should be refreshed:
				if (field == "file" || field == "preview" || field == "screen" || field == "marquee")
					bNeedsTextureUpdate = true;
			}

			// now save the item's changes
			g_pAnarchyManager->GetMetaverseManager()->SaveItem(pItem);
				
			if (bNeedsTextureUpdate)
			{
				/*
				C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->GetFocusedLibretroInstance();
				if (pLibretroInstance)
				{
					if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == (C_EmbeddedInstance*)pLibretroInstance)
						g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);

					g_pAnarchyManager->GetLibretroManager()->DestroyLibretroInstance(pLibretroInstance);
				}
				*/
				/*
				C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
				if (pEmbeddedInstance)
				{
					if (pEmbeddedInstance && pEmbeddedInstance->GetId() != "hud")
					{
						pEmbeddedInstance->Blur();
						pEmbeddedInstance->Deselect();
						g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);
						pEmbeddedInstance->Close();
					}
				}
				*/

				//g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
				g_pAnarchyManager->GetCanvasManager()->RefreshItemTextures(id, "ALL");
				//g_pAnarchyManager->GetCanvasManager()->RefreshItemTextures(id, "screen");
				//g_pAnarchyManager->GetCanvasManager()->RefreshItemTextures(id, "marquee");
				//g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
			}

			//if (g_pAnarchyManager->GetConnectedUniverse() && g_pAnarchyManager->GetConnectedUniverse()->connected)
			//g_pAnarchyManager->GetMetaverseManager()->SendItemUpdate(id);

			return JSValue(true);
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("updateApp"))
	{
		std::string id = WebStringToCharString(args[0].ToString());

		KeyValues* pApp = g_pAnarchyManager->GetMetaverseManager()->GetLibraryApp(id);
		if (pApp)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pApp);

			// now loop through our updated fields
			bool bHadFilepaths = false;
			std::string filepathsKey;

			bool bNeedsTextureUpdate = false;
			std::string field;
			std::string value;
			JSArray update = args[1].ToArray();
			unsigned int max = update.size();
			for (unsigned int i = 0; i < max; i += 2)
			{
				field = WebStringToCharString(update.At(i).ToString());
				value = WebStringToCharString(update.At(i + 1).ToString());

				if (!bHadFilepaths && field.find("filepaths") != std::string::npos)
				{
					size_t found = field.find_last_of("/");
					if (found != std::string::npos)
					{
						filepathsKey = field.substr(0, found);
						found = filepathsKey.find("/");
						if (found != std::string::npos)
						{
							filepathsKey = filepathsKey.substr(found + 1);
							active->SetString(VarArgs("filepaths/%s/id", filepathsKey.c_str()), filepathsKey.c_str());
							//active->FindKey(filepathsKey.c_str(), true);
						}
					}
					
					bHadFilepaths = true;
				}

				// update field with value
				active->SetString(field.c_str(), value.c_str());
			}

			// now save the app's changes
			g_pAnarchyManager->GetMetaverseManager()->SaveApp(pApp);

			return JSValue(true);
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("updateInstance"))
	{
		std::string id = WebStringToCharString(args[0].ToString());

		instance_t* pInstance = g_pAnarchyManager->GetInstanceManager()->GetInstance(id);
		if (pInstance)
		{
			// now loop through our updated fields
			bool bNeedsTextureUpdate = false;
			std::string field;
			std::string value;
			JSArray update = args[1].ToArray();
			unsigned int max = update.size();
			for (unsigned int i = 0; i < max; i += 2)
			{
				//JSObject object = update.At(i).ToObject();
				//field = WebStringToCharString(object.GetProperty(WSLit("field")).ToString());
				//value = WebStringToCharString(object.GetProperty(WSLit("value")).ToString());
				field = WebStringToCharString(update.At(i).ToString());
				value = WebStringToCharString(update.At(i + 1).ToString());

				// update field with value
				if (field == "title")
					pInstance->title = std::string(value);
			}

			// now save the instance's changes
			g_pAnarchyManager->GetMetaverseManager()->SaveInstanceTitle(pInstance);
			return JSValue(true);
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("deleteInstance"))
	{
		std::string id = WebStringToCharString(args[0].ToString());

		instance_t* pInstance = g_pAnarchyManager->GetInstanceManager()->GetInstance(id);
		if (pInstance)
		{
			g_pAnarchyManager->GetMetaverseManager()->DeleteInstance(pInstance);
			return JSValue(true);
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("getFirstLibraryModel"))
	{
		KeyValues* pModel = g_pAnarchyManager->GetMetaverseManager()->GetFirstLibraryModel();

		if (pModel)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pModel);
			JSObject model;
			AddSubKeys(active, model);
			return model;
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("getNextLibraryModel"))
	{
		KeyValues* pModel = g_pAnarchyManager->GetMetaverseManager()->GetNextLibraryModel();
		if (pModel)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pModel);
			JSObject model;
			AddSubKeys(active, model);
			return model;
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("findFirstLibraryModel"))
	{
		// build the search info
		KeyValues* pSearchInfo = new KeyValues("search");	// this gets deleted by the metaverse manager!!

		unsigned int numArgs = args.size();
		if (numArgs == 0)
			return JSValue(0);
		else if (numArgs > 0)
			pSearchInfo->SetString("title", WebStringToCharString(args[0].ToString()));

		if (numArgs > 1)
			pSearchInfo->SetString("dynamic", WebStringToCharString(args[1].ToString()));

		// start the search
		KeyValues* pModel = g_pAnarchyManager->GetMetaverseManager()->FindFirstLibraryModel(pSearchInfo);

		if (pModel)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pModel);
			JSObject model;
			AddSubKeys(active, model);
			return model;
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("findNextLibraryModel"))
	{
		KeyValues* pModel = g_pAnarchyManager->GetMetaverseManager()->FindNextLibraryModel();
		if (pModel)
		{
			KeyValues* active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(pModel);
			JSObject model;
			AddSubKeys(active, model);
			return model;
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("getTransformInfo"))
	{
		JSObject response;

		if (!g_pAnarchyManager->GetMetaverseManager()->GetSpawningObject())
			response.SetProperty(WSLit("success"), JSValue(false));
		else
		{
			response.SetProperty(WSLit("success"), JSValue(true));

			/*
			C_PropShortcutEntity* pShortcut = dynamic_cast<C_PropShortcutEntity*>(g_pAnarchyManager->GetSelectedEntity());
			if (pShortcut)
			{

			}
			*/

			transform_t* pTransform = g_pAnarchyManager->GetInstanceManager()->GetTransform();

			JSObject rotation;
			rotation.SetProperty(WSLit("p"), JSValue(pTransform->rotP));
			rotation.SetProperty(WSLit("y"), JSValue(pTransform->rotY));
			rotation.SetProperty(WSLit("r"), JSValue(pTransform->rotR));

			JSObject offset;
			offset.SetProperty(WSLit("x"), JSValue(pTransform->offX));
			offset.SetProperty(WSLit("y"), JSValue(pTransform->offY));
			offset.SetProperty(WSLit("z"), JSValue(pTransform->offZ));

			response.SetProperty(WSLit("rotation"), rotation);
			response.SetProperty(WSLit("offset"), offset);
			response.SetProperty(WSLit("scale"), JSValue(pTransform->scale));
		}

		return response;
	}
	else if (method_name == WSLit("getEntityInfo"))
	{
		C_PropShortcutEntity* pShortcut = dynamic_cast<C_PropShortcutEntity*>(C_BaseEntity::Instance(args[0].ToInteger()));

		std::string id;
		if (pShortcut)
			id = pShortcut->GetObjectId();

		JSObject response;
		if (id != "")
		{
			response.SetProperty(WSLit("success"), JSValue(true));

			object_t* pObject = g_pAnarchyManager->GetInstanceManager()->GetInstanceObject(id);
			KeyValues* pObjectInfo = null;
			KeyValues* pItemInfo = null;
			KeyValues* pModelInfo = null;

			g_pAnarchyManager->GetMetaverseManager()->GetObjectInfo(pObject, pObjectInfo, pItemInfo, pModelInfo);

			if (pObjectInfo)
			{
				JSObject object;
				AddSubKeys(pObjectInfo, object);
				response.SetProperty(WSLit("object"), object);

				pObjectInfo->deleteThis();
			}

			if (pItemInfo)
			{
				JSObject item;
				AddSubKeys(pItemInfo, item);
				response.SetProperty(WSLit("item"), item);

				pItemInfo->deleteThis();
			}

			if (pModelInfo)
			{
				JSObject model;
				AddSubKeys(pModelInfo, model);
				response.SetProperty(WSLit("model"), model);

				pModelInfo->deleteThis();
			}
		}
		else
			response.SetProperty(WSLit("success"), JSValue(false));

		return response;
	}
	else if (method_name == WSLit("getObjectInfo"))
	{
		std::string id = WebStringToCharString(args[0].ToString());
		JSObject response;
		response.SetProperty(WSLit("success"), JSValue(true));

		object_t* pObject = g_pAnarchyManager->GetInstanceManager()->GetInstanceObject(id);
		KeyValues* pObjectInfo = null;
		KeyValues* pItemInfo = null;
		KeyValues* pModelInfo = null;

		g_pAnarchyManager->GetMetaverseManager()->GetObjectInfo(pObject, pObjectInfo, pItemInfo, pModelInfo);
		if (pObjectInfo)
		{
			JSObject object;
			AddSubKeys(pObjectInfo, object);
			response.SetProperty(WSLit("object"), object);

			pObjectInfo->deleteThis();
		}

		if (pItemInfo)
		{
			JSObject item;
			AddSubKeys(pItemInfo, item);
			response.SetProperty(WSLit("item"), item);

			pItemInfo->deleteThis();
		}

		if (pModelInfo)
		{
			JSObject model;
			AddSubKeys(pModelInfo, model);
			response.SetProperty(WSLit("model"), model);

			pModelInfo->deleteThis();
		}

		return response;
	}
	else if (method_name == WSLit("getObject"))
	{
		std::string id = WebStringToCharString(args[0].ToString());

		object_t* pObject = g_pAnarchyManager->GetInstanceManager()->GetInstanceObject(id);
		if (pObject)
		{
			JSObject responseObject;
			responseObject.SetProperty(WSLit("id"), WSLit(pObject->objectId.c_str()));
			responseObject.SetProperty(WSLit("item"), WSLit(pObject->itemId.c_str()));
			responseObject.SetProperty(WSLit("model"), WSLit(pObject->modelId.c_str()));
			responseObject.SetProperty(WSLit("slave"), JSValue(pObject->slave));
			responseObject.SetProperty(WSLit("scale"), JSValue(pObject->scale));

			// origin
			char buf[AA_MAX_STRING];
			Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", pObject->origin.x, pObject->origin.y, pObject->origin.z);
			responseObject.SetProperty(WSLit("origin"), WSLit(buf));

			// angles
			Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", pObject->angles.x, pObject->angles.y, pObject->angles.z);
			responseObject.SetProperty(WSLit("angles"), WSLit(buf));

			responseObject.SetProperty(WSLit("child"), JSValue(pObject->child));

			C_PropShortcutEntity* pParentShortcut = dynamic_cast<C_PropShortcutEntity*>(C_BaseEntity::Instance(pObject->parentEntityIndex));
			std::string parentObjectId = (pParentShortcut) ? pParentShortcut->GetObjectId() : "";
			responseObject.SetProperty(WSLit("parentObject"), WSLit(parentObjectId.c_str()));

			return responseObject;
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("getAllObjectInfos"))
	{
		//JSObject response;
		JSArray responseObjects;

		std::map<std::string, object_t*> objects = g_pAnarchyManager->GetInstanceManager()->GetObjectsMap();
		std::map<std::string, object_t*>::iterator it = objects.begin();
		while (it != objects.end())
		{
			JSObject responseObject;
			object_t* pObject = it->second;
			responseObject.SetProperty(WSLit("objectId"), WSLit(pObject->objectId.c_str()));
			responseObject.SetProperty(WSLit("itemId"), WSLit(pObject->itemId.c_str()));
			responseObject.SetProperty(WSLit("modelId"), WSLit(pObject->modelId.c_str()));

			/*
			KeyValues* pObjectInfo = null;
			KeyValues* pItemInfo = null;
			KeyValues* pModelInfo = null;

			g_pAnarchyManager->GetMetaverseManager()->GetObjectInfo(pObject, pObjectInfo, pItemInfo, pModelInfo);
			if (pObjectInfo)
			{
				JSObject object;
				AddSubKeys(pObjectInfo, object);
				responseObject.SetProperty(WSLit("object"), object);

				pObjectInfo->deleteThis();
			}

			if (pItemInfo)
			{
				JSObject item;
				AddSubKeys(pItemInfo, item);
				responseObject.SetProperty(WSLit("item"), item);

				pItemInfo->deleteThis();
			}

			if (pModelInfo)
			{
				JSObject model;
				AddSubKeys(pModelInfo, model);
				responseObject.SetProperty(WSLit("model"), model);

				pModelInfo->deleteThis();
			}
			*/

			responseObjects.Push(responseObject);
			it++;
		}

		//response.SetProperty(WSLit("objects"), responseObjects);
		//response.SetProperty(WSLit("success"), JSValue(true));
		return responseObjects;
	}
	else if (method_name == WSLit("getBackpack"))
	{
		JSObject response;
		JSObject responseBackpack;
		std::string backpackId = WebStringToCharString(args[0].ToString());

		C_Backpack* pBackpack = g_pAnarchyManager->GetBackpackManager()->GetBackpack(backpackId);
		if (pBackpack)
		{
			responseBackpack.SetProperty(WSLit("id"), WSLit(pBackpack->GetId().c_str()));
			responseBackpack.SetProperty(WSLit("title"), WSLit(pBackpack->GetTitle().c_str()));
			responseBackpack.SetProperty(WSLit("folder"), WSLit(pBackpack->GetBackpackFolder().c_str()));

			// vpks
			JSArray vpks;
			std::vector<std::string> allVPKs;
			pBackpack->GetAllVPKs(allVPKs);
			unsigned int max = allVPKs.size();
			for (unsigned int i = 0; i < max; i++)
				vpks.Push(WSLit(allVPKs[i].c_str()));
			responseBackpack.SetProperty(WSLit("vpks"), vpks);

			// files
			JSArray files;
			std::vector<std::string> allFiles;
			pBackpack->GetAllFiles(allFiles);
			max = allFiles.size();
			for (unsigned int i = 0; i < max; i++)
				files.Push(WSLit(allFiles[i].c_str()));
			responseBackpack.SetProperty(WSLit("files"), files);

			response.SetProperty(WSLit("success"), JSValue(true));
			response.SetProperty(WSLit("backpack"), responseBackpack);
		}
		else
		{
			response.SetProperty(WSLit("success"), JSValue(false));
		}

		return response;
	}
	else if (method_name == WSLit("getRelativeAssetPath"))
	{
		std::string fullPath = WebStringToCharString(args[0].ToString());
		std::string relativePath = g_pAnarchyManager->ExtractRelativeAssetPath(fullPath);
		return WSLit(relativePath.c_str());
	}
	else if (method_name == WSLit("getAllBackpacks"))
	{
		JSObject response;
		JSArray backpacks;

		std::vector<C_Backpack*> allBackpacks;
		g_pAnarchyManager->GetBackpackManager()->GetAllBackpacks(allBackpacks);

		unsigned int max = allBackpacks.size();
		for (unsigned int i = 0; i < max; i++)
		{
			JSObject backpack;
			backpack.SetProperty(WSLit("id"), WSLit(allBackpacks[i]->GetId().c_str()));
			backpack.SetProperty(WSLit("title"), WSLit(allBackpacks[i]->GetTitle().c_str()));
			backpacks.Push(backpack);
		}

		response.SetProperty(WSLit("success"), JSValue(true));
		response.SetProperty(WSLit("backpacks"), backpacks);
		return response;
	}
	else if (method_name == WSLit("getLibretroActiveOverlay"))
	{
		JSObject response;
		float fPositionX = 0;
		float fPositionY = 0;
		float fSizeX = 1;
		float fSizeY = 1;
		std::string overlayId = "";

		C_LibretroInstance* pLibretroInstance = dynamic_cast<C_LibretroInstance*>(g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance());
		if (pLibretroInstance)
			pLibretroInstance->GetFullscreenInfo(fPositionX, fPositionY, fSizeX, fSizeY, overlayId);

		response.SetProperty(WSLit("x"), JSValue(fPositionX));
		response.SetProperty(WSLit("y"), JSValue(fPositionX));
		response.SetProperty(WSLit("width"), JSValue(fSizeX));
		response.SetProperty(WSLit("height"), JSValue(fSizeY));
		//response.SetProperty(WSLit("file"), WSLit(file.c_str()));
		response.SetProperty(WSLit("overlayId"), WSLit(overlayId.c_str()));

		// ALSO figure out the CORE overlayId
		std::string coreOverlayId;
		std::string gameOverlayId;

		if (pLibretroInstance)
		{
			KeyValues* pOverlaysKV = g_pAnarchyManager->GetLibretroManager()->GetOverlaysKV();
			KeyValues* pOverlayEntryKV = null;
			std::string prettyCore = pLibretroInstance->GetInfo()->prettycore;
			std::string prettyGame = pLibretroInstance->GetInfo()->prettygame;
			std::string testerCore;
			std::string testerGame;
			for (KeyValues *sub = pOverlaysKV->GetFirstSubKey(); sub; sub = sub->GetNextKey())
			{
				testerCore = sub->GetString("core");
				testerGame = sub->GetString("game");

				if (testerCore != prettyCore)
					continue;

				if (testerGame == "")
					coreOverlayId = sub->GetString("overlayId");
				else if (testerGame == prettyGame)
					gameOverlayId = sub->GetString("overlayId");
			}
		}
		response.SetProperty(WSLit("coreOverlayId"), WSLit(coreOverlayId.c_str()));
		response.SetProperty(WSLit("gameOverlayId"), WSLit(gameOverlayId.c_str()));

		return response;
	}
	else if (method_name == WSLit("getLibretroOverlays"))
	{
		g_pAnarchyManager->GetLibretroManager()->DetectAllOverlaysPNGs();

		JSArray overlays;
		std::vector<std::string> overlayFiles;
		g_pAnarchyManager->GetLibretroManager()->DetectAllOverlays(overlayFiles);
		for (unsigned int i = 0; i < overlayFiles.size(); i++)
			overlays.Push(WSLit(overlayFiles[i].c_str()));
		return overlays;
	}
	else if (method_name == WSLit("libretroGetAllDLLs"))
	{
		JSObject response;
		C_LibretroManager* pLibretroManager = g_pAnarchyManager->GetLibretroManager();
		if (pLibretroManager)
		{
			JSArray cores;
			response.SetProperty(WSLit("success"), JSValue(true));

			KeyValues* pBlacklistedDLLsKV = g_pAnarchyManager->GetLibretroManager()->GetBlacklistedDLLsKV();
			KeyValues* pCoreSettingsKV = g_pAnarchyManager->GetLibretroManager()->GetCoreSettingsKV();

			bool bBlacklisted;
			std::string coreFile;
			for (KeyValues *pCoreSub = pCoreSettingsKV->GetFirstSubKey(); pCoreSub; pCoreSub = pCoreSub->GetNextKey())
			{
				coreFile = pCoreSub->GetString("file");
				bBlacklisted = false;
				for (KeyValues *pDLLSub = pBlacklistedDLLsKV->GetFirstSubKey(); pDLLSub; pDLLSub = pDLLSub->GetNextKey())
				{
					if (!Q_stricmp(coreFile.c_str(), pDLLSub->GetString("file")))
					{
						bBlacklisted = true;
						break;
					}
				}

				if (!bBlacklisted && pCoreSub->GetBool("exists"))
				{
					JSObject core;
					core.SetProperty(WSLit("file"), WSLit(coreFile.c_str()));
					core.SetProperty(WSLit("enabled"), JSValue(pCoreSub->GetInt("enabled")));
					core.SetProperty(WSLit("cartsaves"), JSValue(pCoreSub->GetInt("cartsaves")));
					core.SetProperty(WSLit("statesaves"), JSValue(pCoreSub->GetInt("statesaves")));
					core.SetProperty(WSLit("priority"), JSValue(pCoreSub->GetInt("priority")));

					std::string pathText;
					std::string extensions;
					JSArray paths;
					for (KeyValues *sub = pCoreSub->FindKey("paths", true)->GetFirstSubKey(); sub; sub = sub->GetNextKey())
					{
						pathText = sub->GetString("path");
						extensions = sub->GetString("extensions");

						if (pathText == "" && extensions == "")
							continue;

						JSObject path;
						path.SetProperty(WSLit("path"), WSLit(pathText.c_str()));
						path.SetProperty(WSLit("extensions"), WSLit(extensions.c_str()));
						paths.Push(path);
					}
					core.SetProperty(WSLit("paths"), paths);
					cores.Push(core);
				}
			}

			response.SetProperty(WSLit("cores"), cores);
		}
		else
			response.SetProperty(WSLit("success"), JSValue(false));

		return response;
	}
	else if (method_name == WSLit("getLibretroGUIGamepadEnabled"))
	{
		bool bValue = false;
		C_LibretroManager* pLibretroManager = g_pAnarchyManager->GetLibretroManager();
		if (pLibretroManager)
			bValue = pLibretroManager->GetGUIGamepadEnabled();

		return JSValue(bValue);
	}
	else if (method_name == WSLit("getAllWorkshopSubscriptions"))
	{
		JSObject response;
		JSArray subscriptions;

		std::vector<SteamWorkshopDetails_t*> details;
		g_pAnarchyManager->GetWorkshopManager()->GetAllWorkshopSubscriptions(details);

		unsigned int max = details.size();
		for (unsigned int i = 0; i < max; i++)
		{
			JSObject subscription;
			subscription.SetProperty(WSLit("publishedFileId"), WSLit(VarArgs("%llu", details[i]->publishedFileId)));
			subscription.SetProperty(WSLit("type"), JSValue((int)details[i]->type));
			subscription.SetProperty(WSLit("title"), WSLit(details[i]->title.c_str()));
			subscription.SetProperty(WSLit("description"), WSLit(details[i]->description.c_str()));
			subscription.SetProperty(WSLit("owner"), WSLit(VarArgs("%llu", details[i]->owner)));
			subscription.SetProperty(WSLit("created"), WSLit(VarArgs("%u", details[i]->created)));
			subscription.SetProperty(WSLit("updated"), WSLit(VarArgs("%u", details[i]->updated)));
			subscription.SetProperty(WSLit("subscribed"), WSLit(VarArgs("%u", details[i]->subscribed)));
			subscription.SetProperty(WSLit("visibility"), JSValue((int)details[i]->visibility));
			subscription.SetProperty(WSLit("banned"), JSValue(details[i]->banned));
			//subscription.SetProperty(WSLit("accepted"), JSValue(details[i]->m_bAcceptedForUse));
			subscription.SetProperty(WSLit("tagsTruncated"), JSValue(details[i]->tagsTruncated));
			subscription.SetProperty(WSLit("tags"), WSLit(details[i]->tags.c_str()));
			// file/url information
			subscription.SetProperty(WSLit("file"), WSLit(VarArgs("%llu", details[i]->file)));
			subscription.SetProperty(WSLit("preview"), WSLit(VarArgs("%llu", details[i]->preview)));
			subscription.SetProperty(WSLit("previewURL"), WSLit(details[i]->previewURL.c_str()));
			subscription.SetProperty(WSLit("filename"), WSLit(details[i]->filename.c_str()));
			subscription.SetProperty(WSLit("fileSize"), JSValue(details[i]->fileSize));
			subscription.SetProperty(WSLit("previewSize"), JSValue(details[i]->previewSize));
			subscription.SetProperty(WSLit("numSubscriptions"), WSLit(VarArgs("%u", details[i]->numSubscriptions)));
			subscription.SetProperty(WSLit("numFavorites"), WSLit(VarArgs("%u", details[i]->numFavorites)));
			subscription.SetProperty(WSLit("url"), WSLit(details[i]->url.c_str()));

			JSArray additionalPreviewURLs;
			unsigned int max = details[i]->additionalPreviewURLs.size();
			for (unsigned int j = 0; j < max; j++)
				additionalPreviewURLs.Push(WSLit(details[i]->additionalPreviewURLs[j].c_str()));
			subscription.SetProperty(WSLit("additionalPreviewURLs"), additionalPreviewURLs);

			JSArray keyValueTags;
			max = details[i]->keyValueTags.size();
			for (unsigned int j = 0; j < max; j++)
			{
				JSObject keyValueTag;
				keyValueTag.SetProperty(WSLit("key"), WSLit(details[i]->keyValueTags[j]->key.c_str()));
				keyValueTag.SetProperty(WSLit("value"), WSLit(details[i]->keyValueTags[j]->value.c_str()));
				keyValueTags.Push(keyValueTag);
			}
			subscription.SetProperty(WSLit("keyValueTags"), keyValueTags);

			// voting information
			subscription.SetProperty(WSLit("votesUp"), WSLit(VarArgs("%u", details[i]->votesUp)));
			subscription.SetProperty(WSLit("votesDown"), WSLit(VarArgs("%u", details[i]->votesDown)));
			subscription.SetProperty(WSLit("score"), JSValue(details[i]->score));
			// collection information
			subscription.SetProperty(WSLit("numChildren"), WSLit(VarArgs("%u", details[i]->numChildren)));

			// now get extra info including:
			/*
				Cache Folder
				Cache Size

				Note the following info gets populared asynchronously:
				Items List
				Models List
				Maps List
				Instances List
				Files List
				Mounts List
				Required List

				Double note that Redux should take advantage of Workshop collections more completely.
			*/
			
			// add us to the response
			subscriptions.Push(subscription);
		}

		response.SetProperty(WSLit("success"), JSValue(true));
		response.SetProperty(WSLit("subscriptions"), subscriptions);
		return response;
	}
	else if (method_name == WSLit("alphabetSafe"))
	{
		std::string text = WebStringToCharString(args[0].ToString());
		std::string alphabet = WebStringToCharString(args[1].ToString());

		bool bResponse = g_pAnarchyManager->AlphabetSafe(text, alphabet);
		return JSValue(bResponse);
	}
	else if (method_name == WSLit("getConVarValue"))
	{
		std::string name = WebStringToCharString(args[0].ToString());
		ConVar* pConVar = cvar->FindVar(name.c_str());
		if ( pConVar )
			return WSLit(pConVar->GetString());
		return JSValue(0);
	}
	else if (method_name == WSLit("getMount"))
	{
		std::string id = WebStringToCharString(args[0].ToString());
		C_Mount* pMount = g_pAnarchyManager->GetMountManager()->GetMount(id);
		if (pMount)
		{
			JSObject responseMount;
			responseMount.SetProperty(WSLit("active"), JSValue(pMount->GetActive()));
			responseMount.SetProperty(WSLit("id"), WSLit(pMount->GetId().c_str()));
			responseMount.SetProperty(WSLit("title"), WSLit(pMount->GetTitle().c_str()));
			responseMount.SetProperty(WSLit("base"), WSLit(pMount->GetBase().c_str()));
			
			JSArray responsePaths;
			std::vector<std::string> paths = pMount->GetPaths();
			unsigned int max2 = paths.size();
			for (unsigned int j = 0; j < max2; j++)
				responsePaths.Push(WSLit(paths[j].c_str()));
			responseMount.SetProperty(WSLit("paths"), responsePaths);

			JSArray responseMountedPaths;
			std::vector<std::string> mountedPaths = pMount->GetMountedPaths();
			max2 = mountedPaths.size();
			for (unsigned int j = 0; j < max2; j++)
				responseMountedPaths.Push(WSLit(mountedPaths[j].c_str()));
			responseMount.SetProperty(WSLit("mountedPaths"), responseMountedPaths);

			return responseMount;
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("getAllMounts"))
	{
		JSObject response;
		JSArray responseMounts;

		std::vector<C_Mount*> mounts;
		g_pAnarchyManager->GetMountManager()->GetAllMounts(mounts);

		unsigned int j;
		unsigned int max2;
		unsigned int max = mounts.size();
		for (unsigned int i = 0; i < max; i++)
		{
			JSObject responseMount;
			responseMount.SetProperty(WSLit("active"), JSValue(mounts[i]->GetActive()));
			responseMount.SetProperty(WSLit("id"), WSLit(mounts[i]->GetId().c_str()));
			responseMount.SetProperty(WSLit("title"), WSLit(mounts[i]->GetTitle().c_str()));
			responseMount.SetProperty(WSLit("base"), WSLit(mounts[i]->GetBase().c_str()));

			JSArray responsePaths;
			std::vector<std::string> paths = mounts[i]->GetPaths();
			max2 = paths.size();
			for (j = 0; j < max2; j++)
				responsePaths.Push(WSLit(paths[j].c_str()));
			responseMount.SetProperty(WSLit("paths"), responsePaths);

			JSArray responseMountedPaths;
			std::vector<std::string> mountedPaths = mounts[i]->GetMountedPaths();
			max2 = mountedPaths.size();
			for (j = 0; j < max2; j++)
				responseMountedPaths.Push(WSLit(mountedPaths[j].c_str()));
			responseMount.SetProperty(WSLit("mountedPaths"), responseMountedPaths);
			
			responseMounts.Push(responseMount);
		}

		response.SetProperty(WSLit("success"), JSValue(true));
		response.SetProperty(WSLit("mounts"), responseMounts);
		return response;
	}
	else if (method_name == WSLit("isInGame"))
	{
		bool bResponse = engine->IsInGame();
		return JSValue(bResponse);
	}
	else if (method_name == WSLit("setNearestObjectDist"))
	{
		int objectCount = g_pAnarchyManager->GetInstanceManager()->SetNearestSpawnDist(args[0].ToDouble());
		return JSValue(objectCount);
	}
	else if (method_name == WSLit("getAllTasks"))
	{
		bool bGetThumbnails = (args.size() > 0) ? args[0].ToBoolean() : false;

		JSObject response;
		JSArray tasks;
		JSArray windowsTasks;

		// get all the tasks
		std::vector<C_EmbeddedInstance*> embeddedInstances;
		g_pAnarchyManager->GetLibretroManager()->GetAllInstances(embeddedInstances);
		g_pAnarchyManager->GetSteamBrowserManager()->GetAllInstances(embeddedInstances);
		g_pAnarchyManager->GetAwesomiumBrowserManager()->GetAllInstances(embeddedInstances);

		C_EmbeddedInstance* pEmbeddedInstance;
		//std::string originalItemId;

		bool bHiddenTask;
		bool bPresetHiddenTask;
		unsigned int i;
		C_PropShortcutEntity* pEntity;
		unsigned int size = embeddedInstances.size();
		for (i = 0; i < size; i++)
		{
			pEmbeddedInstance = embeddedInstances[i];
			if (!pEmbeddedInstance)
				continue;


			// ignore special instances
			bHiddenTask = (pEmbeddedInstance->GetId() == "hud" || pEmbeddedInstance->GetId() == "images" || pEmbeddedInstance->GetId() == "network");
			bPresetHiddenTask = bHiddenTask;

			//if (pEmbeddedInstance->GetId() == "hud" || pEmbeddedInstance->GetId() == "images")
				//continue;

			/*
			// check for an original item id
			originalItemId = pEmbeddedInstance->GetOriginalItemId();
			if (originalItemId != "")
			{
				// WE HAVE FOUND AN EMBEDDED INSTANCE THAT WAS ORIGINALLY CREATED BY AN ITEM!!

				// make sure we can find an item for it
				KeyValues* pItem = m_pMetaverseManager->GetLibraryItem(originalItemId);
				if (pItem)
				{
					active = pItem->FindKey("current");
					if (!active)
						active = pItem->FindKey("local", true);

					if (active)
						break;
				}
			}
			*/

			bool bIsDisplayTask = (pEmbeddedInstance == g_pAnarchyManager->GetCanvasManager()->GetDisplayInstance());
			bool bIsFirstTaskToDisplay = (!g_pAnarchyManager->GetCanvasManager()->GetDisplayInstance() && pEmbeddedInstance == g_pAnarchyManager->GetCanvasManager()->GetFirstInstanceToDisplay());

			// add them to the tasks object
			JSObject task;
			task.SetProperty(WSLit("id"), WSLit(pEmbeddedInstance->GetId().c_str()));
			task.SetProperty(WSLit("title"), WSLit(pEmbeddedInstance->GetTitle().c_str()));
			task.SetProperty(WSLit("isDisplayTask"), JSValue(bIsDisplayTask || bIsFirstTaskToDisplay));
			task.SetProperty(WSLit("isWindowsTask"), JSValue(false));
			task.SetProperty(WSLit("isHiddenTask"), JSValue(bHiddenTask));
			task.SetProperty(WSLit("isPresetHiddenTask"), JSValue(bPresetHiddenTask));

			pEntity = dynamic_cast<C_PropShortcutEntity*>(C_BaseEntity::Instance(pEmbeddedInstance->GetOriginalEntIndex()));
			if (pEntity)
			{
				JSObject item;
				KeyValues* pItemKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(pEntity->GetItemId()));
				if ( pItemKV )
					AddSubKeys(pItemKV, item);
				task.SetProperty(WSLit("item"), item);
			}

			// update the thumbnail if we are supposed to
			//if (bGetThumbnails)
				//g_pAnarchyManager->GetCanvasManager()->CaptureInstanceThumbnail(pEmbeddedInstance);

			tasks.Push(task);
		}

		// get all the Windows tasks
		std::vector<C_EmbeddedInstance*> windowsEmbeddedInstances;
		g_pAnarchyManager->GetWindowManager()->GetAllInstances(windowsEmbeddedInstances);

		size = windowsEmbeddedInstances.size();
		for (i = 0; i < size; i++)
		{
			pEmbeddedInstance = windowsEmbeddedInstances[i];
			if (!pEmbeddedInstance)
				continue;

			bool bIsDisplayTask = false;
			bool bIsFirstTaskToDisplay = false;
			bool bIsWindowsTask = true;

			bHiddenTask = dynamic_cast<C_WindowInstance*>(pEmbeddedInstance)->IsHidden();
			bPresetHiddenTask = dynamic_cast<C_WindowInstance*>(pEmbeddedInstance)->IsPresetHidden();

			// add them to the tasks object
			JSObject task;
			task.SetProperty(WSLit("id"), WSLit(pEmbeddedInstance->GetId().c_str()));
			task.SetProperty(WSLit("title"), WSLit(pEmbeddedInstance->GetTitle().c_str()));
			task.SetProperty(WSLit("isDisplayTask"), JSValue(bIsDisplayTask || bIsFirstTaskToDisplay));
			task.SetProperty(WSLit("isWindowsTask"), JSValue(true));
			task.SetProperty(WSLit("isHiddenTask"), JSValue(bHiddenTask));
			task.SetProperty(WSLit("isPresetHiddenTask"), JSValue(bPresetHiddenTask));

			// update the thumbnail if we are supposed to
			//if (bGetThumbnails)
				//g_pAnarchyManager->GetCanvasManager()->CaptureInstanceThumbnail(pEmbeddedInstance);

			windowsTasks.Push(task);
		}

		// return success
		response.SetProperty(WSLit("success"), JSValue(true));
		response.SetProperty(WSLit("tasks"), tasks);
		response.SetProperty(WSLit("windowsTasks"), windowsTasks);
		return response;
	}
	else if (method_name == WSLit("createDbBackup"))
	{
		return JSValue(g_pAnarchyManager->GetMetaverseManager()->CreateLibraryBackup());
	}
	else if (method_name == WSLit("getDbSize"))
	{
		return JSValue((int)g_pAnarchyManager->GetMetaverseManager()->GetLibraryDbSize());
	}
	else if (method_name == WSLit("getWorldInfo"))
	{
		JSObject response;

		std::string instanceId = (args.size() > 1) ? WebStringToCharString(args[1].ToString()) : g_pAnarchyManager->GetInstanceId();
		instance_t* instance = g_pAnarchyManager->GetInstanceManager()->GetInstance(instanceId);
		if (!instance)
		{
			DevMsg("ERROR: Instance not found w/ ID %s\n", instanceId.c_str());
			response.SetProperty(WSLit("success"), JSValue(false));
			return response;
		}

		response.SetProperty(WSLit("success"), JSValue(true));
		response.SetProperty(WSLit("universe"), WSLit("Personal"));	// UNIVERSE (always PERSONAL for now, but in the future would tell you what server you are connected to.  ie. universe = server)
		response.SetProperty(WSLit("mode"), WSLit("Singleplayer"));	// MODE (either Singleplayer or Multiplayer)

		JSObject instanceObject;
		instanceObject.SetProperty(WSLit("id"), WSLit(instance->id.c_str()));
		instanceObject.SetProperty(WSLit("mapId"), WSLit(instance->mapId.c_str()));
		instanceObject.SetProperty(WSLit("title"), WSLit(instance->title.c_str()));
		instanceObject.SetProperty(WSLit("file"), WSLit(instance->file.c_str()));
		instanceObject.SetProperty(WSLit("workshopIds"), WSLit(instance->workshopIds.c_str()));
		instanceObject.SetProperty(WSLit("mountIds"), WSLit(instance->mountIds.c_str()));
		response.SetProperty(WSLit("instance"), instanceObject);

		KeyValues* pMapKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->GetMap(instance->mapId.c_str()));

		KeyValues* stuffKV = g_pAnarchyManager->GetMetaverseManager()->DetectRequiredWorkshopForMapFile(pMapKV->GetString("platforms/-KJvcne3IKMZQTaG7lPo/file"));
		if (stuffKV)
		{
			pMapKV->SetString("workshopIds", stuffKV->GetString("workshopIds"));
			pMapKV->SetString("mountIds", stuffKV->GetString("mountIds"));

			std::string mountTitle = "";
			C_Mount* pMount = g_pAnarchyManager->GetMountManager()->GetMount(stuffKV->GetString("mountIds"));
			if (pMount)
				mountTitle = pMount->GetTitle();

			pMapKV->SetString("mountTitle", mountTitle.c_str());
		}

		JSObject mapObject;
		AddSubKeys(pMapKV, mapObject);

		/*
		if (stuffKV)
		{
			JSObject stuff;
			AddSubKeys(stuffKV, stuff);
			stuffKV->deleteThis();
			response.SetProperty(WSLit("stuff"), stuff);
		}
		*/

		response.SetProperty(WSLit("map"), mapObject);

		//// PLAYERS

		return response;
	}
	else if (method_name == WSLit("detectAllMapScreenshots"))
	{
		JSObject response;

		std::map<std::string, KeyValues*>& screenshots = g_pAnarchyManager->GetMetaverseManager()->DetectAllMapScreenshots();
		std::map<std::string, KeyValues*>::iterator it = screenshots.begin();
		while (it != screenshots.end())
		{
			JSObject screenshotObject;
			AddSubKeys(it->second, screenshotObject);
			response.SetProperty(WSLit(it->second->GetString("id")), screenshotObject);
			it++;
		}

		return response;
	}
	else if (method_name == WSLit("getScreenshot"))
	{
		JSObject response;

		std::string screenshotId = WebStringToCharString(args[0].ToString());

		std::vector<KeyValues*> screenshots;
		KeyValues* pScreenshotKV = g_pAnarchyManager->GetMetaverseManager()->GetScreenshot(screenshotId);
		if (pScreenshotKV)
		{
			JSObject screenshotObject;
			AddSubKeys(pScreenshotKV, screenshotObject);
			response.SetProperty(WSLit("screenshot"), screenshotObject);
		}

		return response;
	}
	else if (method_name == WSLit("getAllMapScreenshots"))
	{
		JSObject response;

		std::string mapId = (args.size() > 0) ? WebStringToCharString(args[0].ToString()) : "";

		std::vector<KeyValues*> screenshots;
		g_pAnarchyManager->GetMetaverseManager()->GetAllMapScreenshots(screenshots, mapId);

		KeyValues* pScreenshotKv;
		unsigned int max = screenshots.size();
		for (unsigned int i = 0; i < max; i++)
		{
			pScreenshotKv = screenshots[i];
			JSObject screenshotObject;
			AddSubKeys(pScreenshotKv, screenshotObject);
			response.SetProperty(WSLit(pScreenshotKv->GetString("id")), screenshotObject);
		}

		/*
		std::map<std::string, KeyValues*>& screenshots = g_pAnarchyManager->GetMetaverseManager()->GetAllMapScreenshots(mapId);
		std::map<std::string, KeyValues*>::iterator it = screenshots.begin();
		while (it != screenshots.end())
		{
			JSObject screenshotObject;
			AddSubKeys(it->second, screenshotObject);
			response.SetProperty(WSLit(it->second->GetString("id")), screenshotObject);
			it++;
		}
		*/

		return response;
	}
	else if (method_name == WSLit("getMap"))
	{
		std::string mapId = WebStringToCharString(args[0].ToString());

		JSObject response;

		KeyValues* pMapKV = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(g_pAnarchyManager->GetMetaverseManager()->GetMap(mapId));

		if (pMapKV)
		{
			JSObject mapObject;
			AddSubKeys(pMapKV, mapObject);
			response.SetProperty(WSLit("map"), mapObject);
		}

		return response;
	}
	else if (method_name == WSLit("getAllMaps"))
	{
		JSObject response;

		std::map<std::string, KeyValues*>& maps = g_pAnarchyManager->GetMetaverseManager()->GetAllMaps();
		std::map<std::string, KeyValues*>::iterator it = maps.begin();
		KeyValues* active;
		KeyValues* map;
		while (it != maps.end())
		{
			map = it->second;
			active = g_pAnarchyManager->GetMetaverseManager()->GetActiveKeyValues(map);

			JSObject mapObject;
			AddSubKeys(active, mapObject);
			response.SetProperty(WSLit(active->GetString("info/id")), mapObject);
			it++;
		}

		return response;
	}
	else if (method_name == WSLit("getSelectedWebTab"))
	{
		//C_WebTab* pWebTab = g_pAnarchyManager->GetWebManager()->GetSelectedWebTab();
		C_AwesomiumBrowserInstance* pEmbeddedInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->GetSelectedAwesomiumBrowserInstance();

		if (pEmbeddedInstance)
		{
			JSObject response;
			response.SetProperty(WSLit("id"), WSLit(pEmbeddedInstance->GetId().c_str()));
			return response;
		}
		
		return JSValue(null);
	}
	else if (method_name == WSLit("getLibretroOptions"))
	{
		// FIXME: TODO: need to add a "type" to this request to know which lvl of "current bind" value to return.
		std::string type = WebStringToCharString(args[0].ToString());

		JSObject response;

		C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->GetSelectedLibretroInstance();//dynamic_cast<C_LibretroInstance*>(g_pAnarchyManager->)
		if (pLibretroInstance)
		{
			if (pLibretroInstance->HasInfo())
			{
				LibretroInstanceInfo_t* pInfo = pLibretroInstance->GetInfo();

				response.SetProperty(WSLit("core"), WSLit(pInfo->core.c_str()));
				response.SetProperty(WSLit("game"), WSLit(pInfo->game.c_str()));
				
				DevMsg("Adding libretro options to response...\n");
				//std::vector<libretro_core_option*>& libretroOptions = pLibretroInstance->GetAllOptions();
				JSArray options;

				unsigned int valuesMax;
				unsigned int j;

				KeyValues* kv;
				KeyValues* activekv;
				KeyValues* defaultkv;
				libretro_core_option* libretroOption;
				unsigned int max = pInfo->options.size();// libretroOptions.size();
				for (unsigned int i = 0; i < max; i++)
				{
					libretroOption = pInfo->options[i];// libretroOptions[i];

					JSObject option;
					option.SetProperty(WSLit("name_internal"), WSLit(libretroOption->name_internal.c_str()));
					option.SetProperty(WSLit("name_display"), WSLit(libretroOption->name_display.c_str()));

					if (type == "core")
						kv = pInfo->coreCoreOptions;
					else if (type == "game")
						kv = pInfo->gameCoreOptions;
					else// if (type == "auto")
					{
						// 1st check gameOptions, then fallback to coreOptions
						kv = pInfo->gameCoreOptions;
						if (!Q_strcmp(kv->GetString(libretroOption->name_internal.c_str(), "default"), "default"))
							kv = pInfo->coreCoreOptions;
					}

					if (type == "game" || type == "core" )
						option.SetProperty(WSLit("current"), WSLit(kv->GetString(libretroOption->name_internal.c_str(), "default")));
					else
					{
						if (Q_strcmp(kv->GetString(libretroOption->name_internal.c_str(), "default"), "default"))
							option.SetProperty(WSLit("current"), WSLit(kv->GetString(libretroOption->name_internal.c_str(), "default")));
						else
							option.SetProperty(WSLit("current"), WSLit(pInfo->options[i]->values[0].c_str()));
					}

					// add the ACTIVE value
					activekv = pInfo->gameCoreOptions;
					if (!Q_strcmp(activekv->GetString(libretroOption->name_internal.c_str(), "default"), "default"))
						activekv = pInfo->coreCoreOptions;

					if (Q_strcmp(activekv->GetString(libretroOption->name_internal.c_str(), "default"), "default"))
						option.SetProperty(WSLit("active"), WSLit(activekv->GetString(libretroOption->name_internal.c_str(), "default")));
					else
						option.SetProperty(WSLit("active"), WSLit(pInfo->options[i]->values[0].c_str()));

					// add the DEFAULT value
					defaultkv = pInfo->coreCoreOptions;

					if (type == "game" && Q_strcmp(defaultkv->GetString(libretroOption->name_internal.c_str(), "default"), "default"))
						option.SetProperty(WSLit("default"), WSLit(defaultkv->GetString(libretroOption->name_internal.c_str(), "default")));
					else
						option.SetProperty(WSLit("default"), WSLit(pInfo->options[i]->values[0].c_str()));

					//option.SetProperty(WSLit("current"), WSLit(pLibretroInstance->GetOptionCurrentValue(libretroOption->name_internal).c_str()));
					//option.SetProperty(WSLit("current"), WSLit(VarArgs("%i", pLibretroInstance->GetOptionCurrentValue(i))));

					DevMsg("Adding %s (%s) w/ current value %s\n", libretroOption->name_display.c_str(), libretroOption->name_internal.c_str(), kv->GetString(libretroOption->name_internal.c_str(), "default"));

					JSArray values;
					valuesMax = libretroOption->values.size();
					for (j = 0; j < valuesMax; j++)
					{
						DevMsg("\tAdding value definition %i: %s\n", j, libretroOption->values[j].c_str());
						values.Push(WSLit(libretroOption->values[j].c_str()));
					}

					option.SetProperty(WSLit("values"), values);
					options.Push(option);
				}
				response.SetProperty(WSLit("options"), options);

				// active retro controllers
				if (pInfo->numports > 0)
				{
					const retro_controller_info* libretroPorts = pInfo->portdata;
					unsigned int numPorts = pInfo->numports;

					JSArray controllerPorts;
					max = numPorts;
					unsigned int numTypes;
					for (unsigned int i = 0; i < max; i++)
					{
						JSObject controllerPort;

						const retro_controller_info* libretroPort = &libretroPorts[i];
						DevMsg("Controller port %u: \n", i);

						JSArray controllerPortTypes;

						JSObject unpluggedControllerType;
						unpluggedControllerType.SetProperty(WSLit("desc"), WSLit("Unplugged"));
						unpluggedControllerType.SetProperty(WSLit("id"), JSValue(0));
						controllerPortTypes.Push(unpluggedControllerType);

						numTypes = libretroPort->num_types;
						for (unsigned int j = 0; j < numTypes; j++)
						{
							const retro_controller_description* libretroControllerType = &libretroPort->types[j];
							DevMsg("\t%s (ID: %u)\n", libretroControllerType->desc, libretroControllerType->id);

							JSObject controllerType;
							controllerType.SetProperty(WSLit("desc"), WSLit(libretroControllerType->desc));
							controllerType.SetProperty(WSLit("id"), JSValue((int)libretroControllerType->id));
							controllerPortTypes.Push(controllerType);
						}
						controllerPort.SetProperty(WSLit("types"), controllerPortTypes);
						controllerPort.SetProperty(WSLit("num_types"), JSValue((int)numTypes + 1));
						controllerPort.SetProperty(WSLit("current"), JSValue(pInfo->currentPortTypes[i]));
						controllerPorts.Push(controllerPort);
					}
					response.SetProperty(WSLit("ports"), controllerPorts);
				}
				else
				{
					// if the core hasn't supplied any port info, give generic info
					JSArray controllerPorts;

					JSObject controllerPort;
					DevMsg("Controller port default\n");

					JSArray controllerPortTypes;

					JSObject unpluggedControllerType;
					unpluggedControllerType.SetProperty(WSLit("desc"), WSLit("Unplugged"));
					unpluggedControllerType.SetProperty(WSLit("id"), JSValue(0));
					controllerPortTypes.Push(unpluggedControllerType);

					JSObject controllerType;
					controllerType.SetProperty(WSLit("desc"), WSLit("Joypad"));
					controllerType.SetProperty(WSLit("id"), JSValue(1));
					controllerPortTypes.Push(controllerType);

					controllerPort.SetProperty(WSLit("types"), controllerPortTypes);
					controllerPort.SetProperty(WSLit("num_types"), JSValue(2));
					controllerPort.SetProperty(WSLit("current"), JSValue(1));
					controllerPorts.Push(controllerPort);

					response.SetProperty(WSLit("ports"), controllerPorts);
				}

				// base controller types
				JSArray baseControllerTypes;
				baseControllerTypes.Push(WSLit("None"));
				baseControllerTypes.Push(WSLit("RetroJoypad"));
				baseControllerTypes.Push(WSLit("RetroMouse"));
				baseControllerTypes.Push(WSLit("RetroKeyboard"));
				baseControllerTypes.Push(WSLit("RetroLightgun"));
				baseControllerTypes.Push(WSLit("RetroAnalog"));
				baseControllerTypes.Push(WSLit("RetroPointer"));
				response.SetProperty(WSLit("baseControllerTypes"), baseControllerTypes);
			}
		}

		return response;
	}
	else if (method_name == WSLit("getDefaultLibretroInputDevices"))
	{
		JSObject response;
		JSObject devices;

		JSObject joypad;
		joypad.SetProperty(WSLit("enumValue"), JSValue(1));
		joypad.SetProperty(WSLit("name"), JSValue(WSLit("RetroJoypad")));
		joypad.SetProperty(WSLit("id"), JSValue(WSLit("RETRO_DEVICE_JOYPAD")));

		JSArray joypadButtons;

		JSObject joypadButtonB;
		joypadButtonB.SetProperty(WSLit("name"), WSLit("B Button"));
		joypadButtonB.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_JOYPAD_B"));
		joypadButtonB.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_JOYPAD_B));
		joypadButtons.Push(joypadButtonB);

		JSObject joypadButtonY;
		joypadButtonY.SetProperty(WSLit("name"), WSLit("Y Button"));
		joypadButtonY.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_JOYPAD_Y"));
		joypadButtonY.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_JOYPAD_Y));
		joypadButtons.Push(joypadButtonY);

		JSObject joypadButtonSelect;
		joypadButtonSelect.SetProperty(WSLit("name"), WSLit("Select Button"));
		joypadButtonSelect.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_JOYPAD_SELECT"));
		joypadButtonSelect.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_JOYPAD_SELECT));
		joypadButtons.Push(joypadButtonSelect);

		JSObject joypadButtonStart;
		joypadButtonStart.SetProperty(WSLit("name"), WSLit("Start Button"));
		joypadButtonStart.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_JOYPAD_START"));
		joypadButtonStart.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_JOYPAD_START));
		joypadButtons.Push(joypadButtonStart);

		JSObject joypadButtonUp;
		joypadButtonUp.SetProperty(WSLit("name"), WSLit("Up Direction"));
		joypadButtonUp.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_JOYPAD_UP"));
		joypadButtonUp.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_JOYPAD_UP));
		joypadButtons.Push(joypadButtonUp);

		JSObject joypadButtonDown;
		joypadButtonDown.SetProperty(WSLit("name"), WSLit("Down Direction"));
		joypadButtonDown.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_JOYPAD_DOWN"));
		joypadButtonDown.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_JOYPAD_DOWN));
		joypadButtons.Push(joypadButtonDown);

		JSObject joypadButtonLeft;
		joypadButtonLeft.SetProperty(WSLit("name"), WSLit("Left Direction"));
		joypadButtonLeft.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_JOYPAD_LEFT"));
		joypadButtonLeft.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_JOYPAD_LEFT));
		joypadButtons.Push(joypadButtonLeft);

		JSObject joypadButtonRight;
		joypadButtonRight.SetProperty(WSLit("name"), WSLit("Right Direction"));
		joypadButtonRight.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_JOYPAD_RIGHT"));
		joypadButtonRight.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_JOYPAD_RIGHT));
		joypadButtons.Push(joypadButtonRight);

		JSObject joypadButtonA;
		joypadButtonA.SetProperty(WSLit("name"), WSLit("A Button"));
		joypadButtonA.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_JOYPAD_A"));
		joypadButtonA.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_JOYPAD_A));
		joypadButtons.Push(joypadButtonA);

		JSObject joypadButtonX;
		joypadButtonX.SetProperty(WSLit("name"), WSLit("X Button"));
		joypadButtonX.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_JOYPAD_X"));
		joypadButtonX.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_JOYPAD_X));
		joypadButtons.Push(joypadButtonX);

		JSObject joypadButtonL;
		joypadButtonL.SetProperty(WSLit("name"), WSLit("L Button"));
		joypadButtonL.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_JOYPAD_L"));
		joypadButtonL.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_JOYPAD_L));
		joypadButtons.Push(joypadButtonL);

		JSObject joypadButtonR;
		joypadButtonR.SetProperty(WSLit("name"), WSLit("R Button"));
		joypadButtonR.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_JOYPAD_R"));
		joypadButtonR.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_JOYPAD_R));
		joypadButtons.Push(joypadButtonR);

		JSObject joypadButtonL2;
		joypadButtonL2.SetProperty(WSLit("name"), WSLit("L Trigger"));
		joypadButtonL2.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_JOYPAD_L2"));
		joypadButtonL2.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_JOYPAD_L2));
		joypadButtons.Push(joypadButtonL2);

		JSObject joypadButtonR2;
		joypadButtonR2.SetProperty(WSLit("name"), WSLit("R Trigger"));
		joypadButtonR2.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_JOYPAD_R2"));
		joypadButtonR2.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_JOYPAD_R2));
		joypadButtons.Push(joypadButtonR2);

		JSObject joypadButtonL3;
		joypadButtonL3.SetProperty(WSLit("name"), WSLit("L Thumbstick Button"));
		joypadButtonL3.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_JOYPAD_L3"));
		joypadButtonL3.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_JOYPAD_L3));
		joypadButtons.Push(joypadButtonL3);

		JSObject joypadButtonR3;
		joypadButtonR3.SetProperty(WSLit("name"), WSLit("R Thumbstick Button"));
		joypadButtonR3.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_JOYPAD_R3"));
		joypadButtonR3.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_JOYPAD_R3));
		joypadButtons.Push(joypadButtonR3);

		joypad.SetProperty(WSLit("buttons"), joypadButtons);
		devices.SetProperty(WSLit("RETRO_DEVICE_JOYPAD"), joypad);

		/*

		// MOUSE
		JSObject mouse;
		mouse.SetProperty(WSLit("enumValue"), JSValue(2));
		mouse.SetProperty(WSLit("name"), JSValue(WSLit("RetroMouse")));
		mouse.SetProperty(WSLit("id"), JSValue(WSLit("RETRO_DEVICE_MOUSE")));

		JSArray mouseButtons;

		JSObject mouseButtonX;
		mouseButtonX.SetProperty(WSLit("name"), WSLit("X-Axis"));
		mouseButtonX.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_MOUSE_X"));
		mouseButtonX.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_MOUSE_X));
		mouseButtons.Push(mouseButtonX);

		JSObject mouseButtonY;
		mouseButtonY.SetProperty(WSLit("name"), WSLit("Y-Axis"));
		mouseButtonY.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_MOUSE_Y"));
		mouseButtonY.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_MOUSE_Y));
		mouseButtons.Push(mouseButtonY);

		JSObject mouseButton1;
		mouseButton1.SetProperty(WSLit("name"), WSLit("Button 1"));
		mouseButton1.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_MOUSE_LEFT"));
		mouseButton1.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_MOUSE_LEFT));
		mouseButtons.Push(mouseButton1);

		JSObject mouseButton2;
		mouseButton2.SetProperty(WSLit("name"), WSLit("Button 2"));
		mouseButton2.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_MOUSE_RIGHT"));
		mouseButton2.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_MOUSE_RIGHT));
		mouseButtons.Push(mouseButton2);

		JSObject mouseButtonWheelUp;
		mouseButtonWheelUp.SetProperty(WSLit("name"), WSLit("Wheel Up"));
		mouseButtonWheelUp.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_MOUSE_WHEELUP"));
		mouseButtonWheelUp.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_MOUSE_WHEELUP));
		mouseButtons.Push(mouseButtonWheelUp);

		JSObject mouseButtonWheelDown;
		mouseButtonWheelDown.SetProperty(WSLit("name"), WSLit("Wheel Down"));
		mouseButtonWheelDown.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_MOUSE_WHEELDOWN"));
		mouseButtonWheelDown.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_MOUSE_WHEELDOWN));
		mouseButtons.Push(mouseButtonWheelDown);

		JSObject mouseButton3;
		mouseButton3.SetProperty(WSLit("name"), WSLit("Button 3"));
		mouseButton3.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_MOUSE_MIDDLE"));
		mouseButton3.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_MOUSE_MIDDLE));
		mouseButtons.Push(mouseButton3);

		JSObject mouseButtonWheelLeft;
		mouseButtonWheelLeft.SetProperty(WSLit("name"), WSLit("Wheel Left"));
		mouseButtonWheelLeft.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELUP"));
		mouseButtonWheelLeft.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELUP));
		mouseButtons.Push(mouseButtonWheelLeft);

		JSObject mouseButtonWheelRight;
		mouseButtonWheelRight.SetProperty(WSLit("name"), WSLit("Wheel Right"));
		mouseButtonWheelRight.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN"));
		mouseButtonWheelRight.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN));
		mouseButtons.Push(mouseButtonWheelRight);

		mouse.SetProperty(WSLit("buttons"), mouseButtons);
		devices.SetProperty(WSLit("RETRO_DEVICE_MOUSE"), mouse);

		// KEYBOARD
		JSObject keyboard;
		keyboard.SetProperty(WSLit("enumValue"), JSValue(3));
		keyboard.SetProperty(WSLit("name"), JSValue(WSLit("RetroKeyboard")));
		keyboard.SetProperty(WSLit("id"), JSValue(WSLit("RETRO_DEVICE_KEYBOARD")));

		JSArray keyboardButtons;

		JSObject keyboardButtonBackspace;
		keyboardButtonBackspace.SetProperty(WSLit("name"), WSLit("Backspace"));
		keyboardButtonBackspace.SetProperty(WSLit("id"), WSLit("RETROK_BACKSPACE"));
		keyboardButtonBackspace.SetProperty(WSLit("enumValue"), JSValue(RETROK_BACKSPACE));
		keyboardButtons.Push(keyboardButtonBackspace);

		JSObject keyboardButtonTab;
		keyboardButtonTab.SetProperty(WSLit("name"), WSLit("Tab"));
		keyboardButtonTab.SetProperty(WSLit("id"), WSLit("RETROK_TAB"));
		keyboardButtonTab.SetProperty(WSLit("enumValue"), JSValue(RETROK_TAB));
		keyboardButtons.Push(keyboardButtonTab);

		JSObject keyboardButtonClear;
		keyboardButtonClear.SetProperty(WSLit("name"), WSLit("Clear"));
		keyboardButtonClear.SetProperty(WSLit("id"), WSLit("RETROK_CLEAR"));
		keyboardButtonClear.SetProperty(WSLit("enumValue"), JSValue(RETROK_CLEAR));
		keyboardButtons.Push(keyboardButtonClear);

		JSObject keyboardButtonReturn;
		keyboardButtonReturn.SetProperty(WSLit("name"), WSLit("Return"));
		keyboardButtonReturn.SetProperty(WSLit("id"), WSLit("RETROK_RETURN"));
		keyboardButtonReturn.SetProperty(WSLit("enumValue"), JSValue(RETROK_RETURN));
		keyboardButtons.Push(keyboardButtonReturn);

		JSObject keyboardButtonPause;
		keyboardButtonPause.SetProperty(WSLit("name"), WSLit("Pause"));
		keyboardButtonPause.SetProperty(WSLit("id"), WSLit("RETROK_PAUSE"));
		keyboardButtonPause.SetProperty(WSLit("enumValue"), JSValue(RETROK_PAUSE));
		keyboardButtons.Push(keyboardButtonPause);

		JSObject keyboardButtonEscape;
		keyboardButtonEscape.SetProperty(WSLit("name"), WSLit("Escape"));
		keyboardButtonEscape.SetProperty(WSLit("id"), WSLit("RETROK_ESCAPE"));
		keyboardButtonEscape.SetProperty(WSLit("enumValue"), JSValue(RETROK_ESCAPE));
		keyboardButtons.Push(keyboardButtonEscape);

		JSObject keyboardButtonSpace;
		keyboardButtonSpace.SetProperty(WSLit("name"), WSLit("Space"));
		keyboardButtonSpace.SetProperty(WSLit("id"), WSLit("RETROK_SPACE"));
		keyboardButtonSpace.SetProperty(WSLit("enumValue"), JSValue(RETROK_SPACE));
		keyboardButtons.Push(keyboardButtonSpace);

		JSObject keyboardButtonExclaimation;
		keyboardButtonExclaimation.SetProperty(WSLit("name"), WSLit("!"));
		keyboardButtonExclaimation.SetProperty(WSLit("id"), WSLit("RETROK_EXCLAIM"));
		keyboardButtonExclaimation.SetProperty(WSLit("enumValue"), JSValue(RETROK_EXCLAIM));
		keyboardButtons.Push(keyboardButtonExclaimation);

		JSObject keyboardButtonQuoteDbl;
		keyboardButtonQuoteDbl.SetProperty(WSLit("name"), WSLit("\""));
		keyboardButtonQuoteDbl.SetProperty(WSLit("id"), WSLit("RETROK_QUOTEDBL"));
		keyboardButtonQuoteDbl.SetProperty(WSLit("enumValue"), JSValue(RETROK_QUOTEDBL));
		keyboardButtons.Push(keyboardButtonQuoteDbl);

		JSObject keyboardButtonHash;
		keyboardButtonHash.SetProperty(WSLit("name"), WSLit("#"));
		keyboardButtonHash.SetProperty(WSLit("id"), WSLit("RETROK_HASH"));
		keyboardButtonHash.SetProperty(WSLit("enumValue"), JSValue(RETROK_HASH));
		keyboardButtons.Push(keyboardButtonHash);

		JSObject keyboardButtonDollar;
		keyboardButtonDollar.SetProperty(WSLit("name"), WSLit("$"));
		keyboardButtonDollar.SetProperty(WSLit("id"), WSLit("RETROK_DOLLAR"));
		keyboardButtonDollar.SetProperty(WSLit("enumValue"), JSValue(RETROK_DOLLAR));
		keyboardButtons.Push(keyboardButtonDollar);

		JSObject keyboardButtonAmpersand;
		keyboardButtonAmpersand.SetProperty(WSLit("name"), WSLit("&"));
		keyboardButtonAmpersand.SetProperty(WSLit("id"), WSLit("RETROK_AMPERSAND"));
		keyboardButtonAmpersand.SetProperty(WSLit("enumValue"), JSValue(RETROK_AMPERSAND));
		keyboardButtons.Push(keyboardButtonAmpersand);

		JSObject keyboardButtonQuote;
		keyboardButtonQuote.SetProperty(WSLit("name"), WSLit("'"));
		keyboardButtonQuote.SetProperty(WSLit("id"), WSLit("RETROK_QUOTE"));
		keyboardButtonQuote.SetProperty(WSLit("enumValue"), JSValue(RETROK_QUOTE));
		keyboardButtons.Push(keyboardButtonQuote);

		JSObject keyboardButtonLeftParen;
		keyboardButtonLeftParen.SetProperty(WSLit("name"), WSLit("("));
		keyboardButtonLeftParen.SetProperty(WSLit("id"), WSLit("RETROK_LEFTPAREN"));
		keyboardButtonLeftParen.SetProperty(WSLit("enumValue"), JSValue(RETROK_LEFTPAREN));
		keyboardButtons.Push(keyboardButtonLeftParen);

		JSObject keyboardButtonRightParen;
		keyboardButtonRightParen.SetProperty(WSLit("name"), WSLit(")"));
		keyboardButtonRightParen.SetProperty(WSLit("id"), WSLit("RETROK_RIGHTPAREN"));
		keyboardButtonRightParen.SetProperty(WSLit("enumValue"), JSValue(RETROK_RIGHTPAREN));
		keyboardButtons.Push(keyboardButtonRightParen);

		JSObject keyboardButtonAsterisk;
		keyboardButtonAsterisk.SetProperty(WSLit("name"), WSLit("*"));
		keyboardButtonAsterisk.SetProperty(WSLit("id"), WSLit("RETROK_ASTERISK"));
		keyboardButtonAsterisk.SetProperty(WSLit("enumValue"), JSValue(RETROK_ASTERISK));
		keyboardButtons.Push(keyboardButtonAsterisk);

		JSObject keyboardButtonPlus;
		keyboardButtonPlus.SetProperty(WSLit("name"), WSLit("+"));
		keyboardButtonPlus.SetProperty(WSLit("id"), WSLit("RETROK_PLUS"));
		keyboardButtonPlus.SetProperty(WSLit("enumValue"), JSValue(RETROK_PLUS));
		keyboardButtons.Push(keyboardButtonPlus);

		JSObject keyboardButtonComma;
		keyboardButtonComma.SetProperty(WSLit("name"), WSLit(","));
		keyboardButtonComma.SetProperty(WSLit("id"), WSLit("RETROK_COMMA"));
		keyboardButtonComma.SetProperty(WSLit("enumValue"), JSValue(RETROK_COMMA));
		keyboardButtons.Push(keyboardButtonComma);

		JSObject keyboardButtonMinus;
		keyboardButtonMinus.SetProperty(WSLit("name"), WSLit("-"));
		keyboardButtonMinus.SetProperty(WSLit("id"), WSLit("RETROK_MINUS"));
		keyboardButtonMinus.SetProperty(WSLit("enumValue"), JSValue(RETROK_MINUS));
		keyboardButtons.Push(keyboardButtonMinus);

		JSObject keyboardButtonPeriod;
		keyboardButtonPeriod.SetProperty(WSLit("name"), WSLit("."));
		keyboardButtonPeriod.SetProperty(WSLit("id"), WSLit("RETROK_PERIOD"));
		keyboardButtonPeriod.SetProperty(WSLit("enumValue"), JSValue(RETROK_PERIOD));
		keyboardButtons.Push(keyboardButtonPeriod);

		JSObject keyboardButtonSlash;
		keyboardButtonSlash.SetProperty(WSLit("name"), WSLit("/"));
		keyboardButtonSlash.SetProperty(WSLit("id"), WSLit("RETROK_SLASH"));
		keyboardButtonSlash.SetProperty(WSLit("enumValue"), JSValue(RETROK_SLASH));
		keyboardButtons.Push(keyboardButtonSlash);

		JSObject keyboardButton0;
		keyboardButton0.SetProperty(WSLit("name"), WSLit("0"));
		keyboardButton0.SetProperty(WSLit("id"), WSLit("RETROK_0"));
		keyboardButton0.SetProperty(WSLit("enumValue"), JSValue(RETROK_0));
		keyboardButtons.Push(keyboardButton0);

		JSObject keyboardButton1;
		keyboardButton1.SetProperty(WSLit("name"), WSLit("1"));
		keyboardButton1.SetProperty(WSLit("id"), WSLit("RETROK_1"));
		keyboardButton1.SetProperty(WSLit("enumValue"), JSValue(RETROK_1));
		keyboardButtons.Push(keyboardButton1);

		JSObject keyboardButton2;
		keyboardButton2.SetProperty(WSLit("name"), WSLit("2"));
		keyboardButton2.SetProperty(WSLit("id"), WSLit("RETROK_2"));
		keyboardButton2.SetProperty(WSLit("enumValue"), JSValue(RETROK_2));
		keyboardButtons.Push(keyboardButton2);

		JSObject keyboardButton3;
		keyboardButton3.SetProperty(WSLit("name"), WSLit("3"));
		keyboardButton3.SetProperty(WSLit("id"), WSLit("RETROK_3"));
		keyboardButton3.SetProperty(WSLit("enumValue"), JSValue(RETROK_3));
		keyboardButtons.Push(keyboardButton3);

		JSObject keyboardButton4;
		keyboardButton4.SetProperty(WSLit("name"), WSLit("4"));
		keyboardButton4.SetProperty(WSLit("id"), WSLit("RETROK_4"));
		keyboardButton4.SetProperty(WSLit("enumValue"), JSValue(RETROK_4));
		keyboardButtons.Push(keyboardButton4);

		JSObject keyboardButton5;
		keyboardButton5.SetProperty(WSLit("name"), WSLit("5"));
		keyboardButton5.SetProperty(WSLit("id"), WSLit("RETROK_5"));
		keyboardButton5.SetProperty(WSLit("enumValue"), JSValue(RETROK_5));
		keyboardButtons.Push(keyboardButton5);

		JSObject keyboardButton6;
		keyboardButton6.SetProperty(WSLit("name"), WSLit("6"));
		keyboardButton6.SetProperty(WSLit("id"), WSLit("RETROK_6"));
		keyboardButton6.SetProperty(WSLit("enumValue"), JSValue(RETROK_6));
		keyboardButtons.Push(keyboardButton6);

		JSObject keyboardButton7;
		keyboardButton7.SetProperty(WSLit("name"), WSLit("7"));
		keyboardButton7.SetProperty(WSLit("id"), WSLit("RETROK_7"));
		keyboardButton7.SetProperty(WSLit("enumValue"), JSValue(RETROK_7));
		keyboardButtons.Push(keyboardButton7);

		JSObject keyboardButton8;
		keyboardButton8.SetProperty(WSLit("name"), WSLit("8"));
		keyboardButton8.SetProperty(WSLit("id"), WSLit("RETROK_8"));
		keyboardButton8.SetProperty(WSLit("enumValue"), JSValue(RETROK_8));
		keyboardButtons.Push(keyboardButton8);

		JSObject keyboardButton9;
		keyboardButton9.SetProperty(WSLit("name"), WSLit("9"));
		keyboardButton9.SetProperty(WSLit("id"), WSLit("RETROK_9"));
		keyboardButton9.SetProperty(WSLit("enumValue"), JSValue(RETROK_9));
		keyboardButtons.Push(keyboardButton9);

		JSObject keyboardButtonColon;
		keyboardButtonColon.SetProperty(WSLit("name"), WSLit(":"));
		keyboardButtonColon.SetProperty(WSLit("id"), WSLit("RETROK_COLON"));
		keyboardButtonColon.SetProperty(WSLit("enumValue"), JSValue(RETROK_COLON));
		keyboardButtons.Push(keyboardButtonColon);

		JSObject keyboardButtonSemicolon;
		keyboardButtonSemicolon.SetProperty(WSLit("name"), WSLit(";"));
		keyboardButtonSemicolon.SetProperty(WSLit("id"), WSLit("RETROK_SEMICOLON"));
		keyboardButtonSemicolon.SetProperty(WSLit("enumValue"), JSValue(RETROK_SEMICOLON));
		keyboardButtons.Push(keyboardButtonSemicolon);

		JSObject keyboardButtonLess;
		keyboardButtonLess.SetProperty(WSLit("name"), WSLit("<"));
		keyboardButtonLess.SetProperty(WSLit("id"), WSLit("RETROK_LESS"));
		keyboardButtonLess.SetProperty(WSLit("enumValue"), JSValue(RETROK_LESS));
		keyboardButtons.Push(keyboardButtonLess);

		JSObject keyboardButtonEquals;
		keyboardButtonEquals.SetProperty(WSLit("name"), WSLit("="));
		keyboardButtonEquals.SetProperty(WSLit("id"), WSLit("RETROK_EQUALS"));
		keyboardButtonEquals.SetProperty(WSLit("enumValue"), JSValue(RETROK_EQUALS));
		keyboardButtons.Push(keyboardButtonEquals);

		JSObject keyboardButtonGreater;
		keyboardButtonGreater.SetProperty(WSLit("name"), WSLit(">"));
		keyboardButtonGreater.SetProperty(WSLit("id"), WSLit("RETROK_GREATER"));
		keyboardButtonGreater.SetProperty(WSLit("enumValue"), JSValue(RETROK_GREATER));
		keyboardButtons.Push(keyboardButtonGreater);

		JSObject keyboardButtonQuestion;
		keyboardButtonQuestion.SetProperty(WSLit("name"), WSLit("?"));
		keyboardButtonQuestion.SetProperty(WSLit("id"), WSLit("RETROK_QUESTION"));
		keyboardButtonQuestion.SetProperty(WSLit("enumValue"), JSValue(RETROK_QUESTION));
		keyboardButtons.Push(keyboardButtonQuestion);

		JSObject keyboardButtonAt;
		keyboardButtonAt.SetProperty(WSLit("name"), WSLit("@"));
		keyboardButtonAt.SetProperty(WSLit("id"), WSLit("RETROK_AT"));
		keyboardButtonAt.SetProperty(WSLit("enumValue"), JSValue(RETROK_AT));
		keyboardButtons.Push(keyboardButtonAt);

		JSObject keyboardButtonLeftBracket;
		keyboardButtonLeftBracket.SetProperty(WSLit("name"), WSLit("["));
		keyboardButtonLeftBracket.SetProperty(WSLit("id"), WSLit("RETROK_LEFTBRACKET"));
		keyboardButtonLeftBracket.SetProperty(WSLit("enumValue"), JSValue(RETROK_LEFTBRACKET));
		keyboardButtons.Push(keyboardButtonLeftBracket);

		JSObject keyboardButtonBackslash;
		keyboardButtonBackslash.SetProperty(WSLit("name"), WSLit("\\"));
		keyboardButtonBackslash.SetProperty(WSLit("id"), WSLit("RETROK_BACKSLASH"));
		keyboardButtonBackslash.SetProperty(WSLit("enumValue"), JSValue(RETROK_BACKSLASH));
		keyboardButtons.Push(keyboardButtonBackslash);

		JSObject keyboardButtonRightBracket;
		keyboardButtonRightBracket.SetProperty(WSLit("name"), WSLit("]"));
		keyboardButtonRightBracket.SetProperty(WSLit("id"), WSLit("RETROK_RIGHTBRACKET"));
		keyboardButtonRightBracket.SetProperty(WSLit("enumValue"), JSValue(RETROK_RIGHTBRACKET));
		keyboardButtons.Push(keyboardButtonRightBracket);

		JSObject keyboardButtonCaret;
		keyboardButtonCaret.SetProperty(WSLit("name"), WSLit("^"));
		keyboardButtonCaret.SetProperty(WSLit("id"), WSLit("RETROK_CARET"));
		keyboardButtonCaret.SetProperty(WSLit("enumValue"), JSValue(RETROK_CARET));
		keyboardButtons.Push(keyboardButtonCaret);

		JSObject keyboardButtonUnderscore;
		keyboardButtonUnderscore.SetProperty(WSLit("name"), WSLit("_"));
		keyboardButtonUnderscore.SetProperty(WSLit("id"), WSLit("RETROK_UNDERSCORE"));
		keyboardButtonUnderscore.SetProperty(WSLit("enumValue"), JSValue(RETROK_UNDERSCORE));
		keyboardButtons.Push(keyboardButtonUnderscore);

		JSObject keyboardButtonBackquote;
		keyboardButtonBackquote.SetProperty(WSLit("name"), WSLit("`"));
		keyboardButtonBackquote.SetProperty(WSLit("id"), WSLit("RETROK_BACKQUOTE"));
		keyboardButtonBackquote.SetProperty(WSLit("enumValue"), JSValue(RETROK_BACKQUOTE));
		keyboardButtons.Push(keyboardButtonBackquote);

		JSObject keyboardButtonA;
		keyboardButtonA.SetProperty(WSLit("name"), WSLit("A"));
		keyboardButtonA.SetProperty(WSLit("id"), WSLit("RETROK_a"));
		keyboardButtonA.SetProperty(WSLit("enumValue"), JSValue(RETROK_a));
		keyboardButtons.Push(keyboardButtonA);

		JSObject keyboardButtonB;
		keyboardButtonB.SetProperty(WSLit("name"), WSLit("B"));
		keyboardButtonB.SetProperty(WSLit("id"), WSLit("RETROK_b"));
		keyboardButtonB.SetProperty(WSLit("enumValue"), JSValue(RETROK_b));
		keyboardButtons.Push(keyboardButtonB);

		JSObject keyboardButtonC;
		keyboardButtonC.SetProperty(WSLit("name"), WSLit("C"));
		keyboardButtonC.SetProperty(WSLit("id"), WSLit("RETROK_c"));
		keyboardButtonC.SetProperty(WSLit("enumValue"), JSValue(RETROK_c));
		keyboardButtons.Push(keyboardButtonC);

		JSObject keyboardButtonD;
		keyboardButtonD.SetProperty(WSLit("name"), WSLit("D"));
		keyboardButtonD.SetProperty(WSLit("id"), WSLit("RETROK_d"));
		keyboardButtonD.SetProperty(WSLit("enumValue"), JSValue(RETROK_d));
		keyboardButtons.Push(keyboardButtonD);

		JSObject keyboardButtonE;
		keyboardButtonE.SetProperty(WSLit("name"), WSLit("E"));
		keyboardButtonE.SetProperty(WSLit("id"), WSLit("RETROK_e"));
		keyboardButtonE.SetProperty(WSLit("enumValue"), JSValue(RETROK_e));
		keyboardButtons.Push(keyboardButtonE);

		JSObject keyboardButtonF;
		keyboardButtonF.SetProperty(WSLit("name"), WSLit("F"));
		keyboardButtonF.SetProperty(WSLit("id"), WSLit("RETROK_f"));
		keyboardButtonF.SetProperty(WSLit("enumValue"), JSValue(RETROK_f));
		keyboardButtons.Push(keyboardButtonF);

		JSObject keyboardButtonG;
		keyboardButtonG.SetProperty(WSLit("name"), WSLit("G"));
		keyboardButtonG.SetProperty(WSLit("id"), WSLit("RETROK_g"));
		keyboardButtonG.SetProperty(WSLit("enumValue"), JSValue(RETROK_g));
		keyboardButtons.Push(keyboardButtonG);

		JSObject keyboardButtonH;
		keyboardButtonH.SetProperty(WSLit("name"), WSLit("H"));
		keyboardButtonH.SetProperty(WSLit("id"), WSLit("RETROK_h"));
		keyboardButtonH.SetProperty(WSLit("enumValue"), JSValue(RETROK_h));
		keyboardButtons.Push(keyboardButtonH);

		JSObject keyboardButtonI;
		keyboardButtonI.SetProperty(WSLit("name"), WSLit("I"));
		keyboardButtonI.SetProperty(WSLit("id"), WSLit("RETROK_i"));
		keyboardButtonI.SetProperty(WSLit("enumValue"), JSValue(RETROK_i));
		keyboardButtons.Push(keyboardButtonI);

		JSObject keyboardButtonJ;
		keyboardButtonJ.SetProperty(WSLit("name"), WSLit("J"));
		keyboardButtonJ.SetProperty(WSLit("id"), WSLit("RETROK_j"));
		keyboardButtonJ.SetProperty(WSLit("enumValue"), JSValue(RETROK_j));
		keyboardButtons.Push(keyboardButtonJ);

		JSObject keyboardButtonK;
		keyboardButtonK.SetProperty(WSLit("name"), WSLit("K"));
		keyboardButtonK.SetProperty(WSLit("id"), WSLit("RETROK_k"));
		keyboardButtonK.SetProperty(WSLit("enumValue"), JSValue(RETROK_k));
		keyboardButtons.Push(keyboardButtonK);

		JSObject keyboardButtonL;
		keyboardButtonL.SetProperty(WSLit("name"), WSLit("L"));
		keyboardButtonL.SetProperty(WSLit("id"), WSLit("RETROK_l"));
		keyboardButtonL.SetProperty(WSLit("enumValue"), JSValue(RETROK_l));
		keyboardButtons.Push(keyboardButtonL);

		JSObject keyboardButtonM;
		keyboardButtonM.SetProperty(WSLit("name"), WSLit("M"));
		keyboardButtonM.SetProperty(WSLit("id"), WSLit("RETROK_m"));
		keyboardButtonM.SetProperty(WSLit("enumValue"), JSValue(RETROK_m));
		keyboardButtons.Push(keyboardButtonM);

		JSObject keyboardButtonN;
		keyboardButtonN.SetProperty(WSLit("name"), WSLit("N"));
		keyboardButtonN.SetProperty(WSLit("id"), WSLit("RETROK_n"));
		keyboardButtonN.SetProperty(WSLit("enumValue"), JSValue(RETROK_n));
		keyboardButtons.Push(keyboardButtonN);

		JSObject keyboardButtonO;
		keyboardButtonO.SetProperty(WSLit("name"), WSLit("O"));
		keyboardButtonO.SetProperty(WSLit("id"), WSLit("RETROK_o"));
		keyboardButtonO.SetProperty(WSLit("enumValue"), JSValue(RETROK_o));
		keyboardButtons.Push(keyboardButtonO);

		JSObject keyboardButtonP;
		keyboardButtonP.SetProperty(WSLit("name"), WSLit("P"));
		keyboardButtonP.SetProperty(WSLit("id"), WSLit("RETROK_p"));
		keyboardButtonP.SetProperty(WSLit("enumValue"), JSValue(RETROK_p));
		keyboardButtons.Push(keyboardButtonP);

		JSObject keyboardButtonQ;
		keyboardButtonQ.SetProperty(WSLit("name"), WSLit("Q"));
		keyboardButtonQ.SetProperty(WSLit("id"), WSLit("RETROK_q"));
		keyboardButtonQ.SetProperty(WSLit("enumValue"), JSValue(RETROK_q));
		keyboardButtons.Push(keyboardButtonQ);

		JSObject keyboardButtonR;
		keyboardButtonR.SetProperty(WSLit("name"), WSLit("R"));
		keyboardButtonR.SetProperty(WSLit("id"), WSLit("RETROK_r"));
		keyboardButtonR.SetProperty(WSLit("enumValue"), JSValue(RETROK_r));
		keyboardButtons.Push(keyboardButtonR);

		JSObject keyboardButtonS;
		keyboardButtonS.SetProperty(WSLit("name"), WSLit("S"));
		keyboardButtonS.SetProperty(WSLit("id"), WSLit("RETROK_s"));
		keyboardButtonS.SetProperty(WSLit("enumValue"), JSValue(RETROK_s));
		keyboardButtons.Push(keyboardButtonS);

		JSObject keyboardButtonT;
		keyboardButtonT.SetProperty(WSLit("name"), WSLit("T"));
		keyboardButtonT.SetProperty(WSLit("id"), WSLit("RETROK_t"));
		keyboardButtonT.SetProperty(WSLit("enumValue"), JSValue(RETROK_t));
		keyboardButtons.Push(keyboardButtonT);

		JSObject keyboardButtonU;
		keyboardButtonU.SetProperty(WSLit("name"), WSLit("U"));
		keyboardButtonU.SetProperty(WSLit("id"), WSLit("RETROK_u"));
		keyboardButtonU.SetProperty(WSLit("enumValue"), JSValue(RETROK_u));
		keyboardButtons.Push(keyboardButtonU);

		JSObject keyboardButtonV;
		keyboardButtonV.SetProperty(WSLit("name"), WSLit("V"));
		keyboardButtonV.SetProperty(WSLit("id"), WSLit("RETROK_v"));
		keyboardButtonV.SetProperty(WSLit("enumValue"), JSValue(RETROK_v));
		keyboardButtons.Push(keyboardButtonV);

		JSObject keyboardButtonW;
		keyboardButtonW.SetProperty(WSLit("name"), WSLit("W"));
		keyboardButtonW.SetProperty(WSLit("id"), WSLit("RETROK_w"));
		keyboardButtonW.SetProperty(WSLit("enumValue"), JSValue(RETROK_w));
		keyboardButtons.Push(keyboardButtonW);

		JSObject keyboardButtonX;
		keyboardButtonX.SetProperty(WSLit("name"), WSLit("X"));
		keyboardButtonX.SetProperty(WSLit("id"), WSLit("RETROK_x"));
		keyboardButtonX.SetProperty(WSLit("enumValue"), JSValue(RETROK_x));
		keyboardButtons.Push(keyboardButtonX);

		JSObject keyboardButtonY;
		keyboardButtonY.SetProperty(WSLit("name"), WSLit("Y"));
		keyboardButtonY.SetProperty(WSLit("id"), WSLit("RETROK_y"));
		keyboardButtonY.SetProperty(WSLit("enumValue"), JSValue(RETROK_y));
		keyboardButtons.Push(keyboardButtonY);

		JSObject keyboardButtonZ;
		keyboardButtonZ.SetProperty(WSLit("name"), WSLit("Z"));
		keyboardButtonZ.SetProperty(WSLit("id"), WSLit("RETROK_z"));
		keyboardButtonZ.SetProperty(WSLit("enumValue"), JSValue(RETROK_z));
		keyboardButtons.Push(keyboardButtonZ);

		JSObject keyboardButtonDelete;
		keyboardButtonDelete.SetProperty(WSLit("name"), WSLit("Delete"));
		keyboardButtonDelete.SetProperty(WSLit("id"), WSLit("RETROK_DELETE"));
		keyboardButtonDelete.SetProperty(WSLit("enumValue"), JSValue(RETROK_DELETE));
		keyboardButtons.Push(keyboardButtonDelete);

		JSObject keyboardButtonKP0;
		keyboardButtonKP0.SetProperty(WSLit("name"), WSLit("Numpad 0"));
		keyboardButtonKP0.SetProperty(WSLit("id"), WSLit("RETROK_KP0"));
		keyboardButtonKP0.SetProperty(WSLit("enumValue"), JSValue(RETROK_KP0));
		keyboardButtons.Push(keyboardButtonKP0);

		JSObject keyboardButtonKP1;
		keyboardButtonKP1.SetProperty(WSLit("name"), WSLit("Numpad 1"));
		keyboardButtonKP1.SetProperty(WSLit("id"), WSLit("RETROK_KP1"));
		keyboardButtonKP1.SetProperty(WSLit("enumValue"), JSValue(RETROK_KP1));
		keyboardButtons.Push(keyboardButtonKP1);

		JSObject keyboardButtonKP2;
		keyboardButtonKP2.SetProperty(WSLit("name"), WSLit("Numpad 2"));
		keyboardButtonKP2.SetProperty(WSLit("id"), WSLit("RETROK_KP2"));
		keyboardButtonKP2.SetProperty(WSLit("enumValue"), JSValue(RETROK_KP2));
		keyboardButtons.Push(keyboardButtonKP2);

		JSObject keyboardButtonKP3;
		keyboardButtonKP3.SetProperty(WSLit("name"), WSLit("Numpad 3"));
		keyboardButtonKP3.SetProperty(WSLit("id"), WSLit("RETROK_KP3"));
		keyboardButtonKP3.SetProperty(WSLit("enumValue"), JSValue(RETROK_KP3));
		keyboardButtons.Push(keyboardButtonKP3);

		JSObject keyboardButtonKP4;
		keyboardButtonKP4.SetProperty(WSLit("name"), WSLit("Numpad 4"));
		keyboardButtonKP4.SetProperty(WSLit("id"), WSLit("RETROK_KP4"));
		keyboardButtonKP4.SetProperty(WSLit("enumValue"), JSValue(RETROK_KP4));
		keyboardButtons.Push(keyboardButtonKP4);

		JSObject keyboardButtonKP5;
		keyboardButtonKP5.SetProperty(WSLit("name"), WSLit("Numpad 5"));
		keyboardButtonKP5.SetProperty(WSLit("id"), WSLit("RETROK_KP5"));
		keyboardButtonKP5.SetProperty(WSLit("enumValue"), JSValue(RETROK_KP5));
		keyboardButtons.Push(keyboardButtonKP5);

		JSObject keyboardButtonKP6;
		keyboardButtonKP6.SetProperty(WSLit("name"), WSLit("Numpad 6"));
		keyboardButtonKP6.SetProperty(WSLit("id"), WSLit("RETROK_KP6"));
		keyboardButtonKP6.SetProperty(WSLit("enumValue"), JSValue(RETROK_KP6));
		keyboardButtons.Push(keyboardButtonKP6);

		JSObject keyboardButtonKP7;
		keyboardButtonKP7.SetProperty(WSLit("name"), WSLit("Numpad 7"));
		keyboardButtonKP7.SetProperty(WSLit("id"), WSLit("RETROK_KP7"));
		keyboardButtonKP7.SetProperty(WSLit("enumValue"), JSValue(RETROK_KP7));
		keyboardButtons.Push(keyboardButtonKP7);

		JSObject keyboardButtonKP8;
		keyboardButtonKP8.SetProperty(WSLit("name"), WSLit("Numpad 8"));
		keyboardButtonKP8.SetProperty(WSLit("id"), WSLit("RETROK_KP8"));
		keyboardButtonKP8.SetProperty(WSLit("enumValue"), JSValue(RETROK_KP8));
		keyboardButtons.Push(keyboardButtonKP8);

		JSObject keyboardButtonKP9;
		keyboardButtonKP9.SetProperty(WSLit("name"), WSLit("Numpad 9"));
		keyboardButtonKP9.SetProperty(WSLit("id"), WSLit("RETROK_KP9"));
		keyboardButtonKP9.SetProperty(WSLit("enumValue"), JSValue(RETROK_KP9));
		keyboardButtons.Push(keyboardButtonKP9);

		JSObject keyboardButtonKPPeriod;
		keyboardButtonKPPeriod.SetProperty(WSLit("name"), WSLit("Numpad ."));
		keyboardButtonKPPeriod.SetProperty(WSLit("id"), WSLit("RETROK_KP_PERIOD"));
		keyboardButtonKPPeriod.SetProperty(WSLit("enumValue"), JSValue(RETROK_KP_PERIOD));
		keyboardButtons.Push(keyboardButtonKPPeriod);

		JSObject keyboardButtonKPDivide;
		keyboardButtonKPDivide.SetProperty(WSLit("name"), WSLit("Numpad /"));
		keyboardButtonKPDivide.SetProperty(WSLit("id"), WSLit("RETROK_KP_DIVIDE"));
		keyboardButtonKPDivide.SetProperty(WSLit("enumValue"), JSValue(RETROK_KP_DIVIDE));
		keyboardButtons.Push(keyboardButtonKPDivide);

		JSObject keyboardButtonKPMultipy;
		keyboardButtonKPMultipy.SetProperty(WSLit("name"), WSLit("Numpad *"));
		keyboardButtonKPMultipy.SetProperty(WSLit("id"), WSLit("RETROK_KP_MULTIPLY"));
		keyboardButtonKPMultipy.SetProperty(WSLit("enumValue"), JSValue(RETROK_KP_MULTIPLY));
		keyboardButtons.Push(keyboardButtonKPMultipy);

		JSObject keyboardButtonKPMinus;
		keyboardButtonKPMinus.SetProperty(WSLit("name"), WSLit("Numpad -"));
		keyboardButtonKPMinus.SetProperty(WSLit("id"), WSLit("RETROK_KP_MINUS"));
		keyboardButtonKPMinus.SetProperty(WSLit("enumValue"), JSValue(RETROK_KP_MINUS));
		keyboardButtons.Push(keyboardButtonKPMinus);

		JSObject keyboardButtonKPPlus;
		keyboardButtonKPPlus.SetProperty(WSLit("name"), WSLit("Numpad +"));
		keyboardButtonKPPlus.SetProperty(WSLit("id"), WSLit("RETROK_KP_PLUS"));
		keyboardButtonKPPlus.SetProperty(WSLit("enumValue"), JSValue(RETROK_KP_PLUS));
		keyboardButtons.Push(keyboardButtonKPPlus);

		JSObject keyboardButtonKPEnter;
		keyboardButtonKPEnter.SetProperty(WSLit("name"), WSLit("Numpad Enter"));
		keyboardButtonKPEnter.SetProperty(WSLit("id"), WSLit("RETROK_KP_EQUALS"));
		keyboardButtonKPEnter.SetProperty(WSLit("enumValue"), JSValue(RETROK_KP_EQUALS));
		keyboardButtons.Push(keyboardButtonKPEnter);

		JSObject keyboardButtonUp;
		keyboardButtonUp.SetProperty(WSLit("name"), WSLit("Up"));
		keyboardButtonUp.SetProperty(WSLit("id"), WSLit("RETROK_UP"));
		keyboardButtonUp.SetProperty(WSLit("enumValue"), JSValue(RETROK_UP));
		keyboardButtons.Push(keyboardButtonUp);

		JSObject keyboardButtonDown;
		keyboardButtonDown.SetProperty(WSLit("name"), WSLit("Down"));
		keyboardButtonDown.SetProperty(WSLit("id"), WSLit("RETROK_DOWN"));
		keyboardButtonDown.SetProperty(WSLit("enumValue"), JSValue(RETROK_DOWN));
		keyboardButtons.Push(keyboardButtonDown);

		JSObject keyboardButtonRight;
		keyboardButtonRight.SetProperty(WSLit("name"), WSLit("Right"));
		keyboardButtonRight.SetProperty(WSLit("id"), WSLit("RETROK_RIGHT"));
		keyboardButtonRight.SetProperty(WSLit("enumValue"), JSValue(RETROK_RIGHT));
		keyboardButtons.Push(keyboardButtonRight);

		JSObject keyboardButtonLeft;
		keyboardButtonLeft.SetProperty(WSLit("name"), WSLit("Left"));
		keyboardButtonLeft.SetProperty(WSLit("id"), WSLit("RETROK_LEFT"));
		keyboardButtonLeft.SetProperty(WSLit("enumValue"), JSValue(RETROK_LEFT));
		keyboardButtons.Push(keyboardButtonLeft);

		JSObject keyboardButtonInsert;
		keyboardButtonInsert.SetProperty(WSLit("name"), WSLit("Insert"));
		keyboardButtonInsert.SetProperty(WSLit("id"), WSLit("RETROK_INSERT"));
		keyboardButtonInsert.SetProperty(WSLit("enumValue"), JSValue(RETROK_INSERT));
		keyboardButtons.Push(keyboardButtonInsert);

		JSObject keyboardButtonHome;
		keyboardButtonHome.SetProperty(WSLit("name"), WSLit("Home"));
		keyboardButtonHome.SetProperty(WSLit("id"), WSLit("RETROK_HOME"));
		keyboardButtonHome.SetProperty(WSLit("enumValue"), JSValue(RETROK_HOME));
		keyboardButtons.Push(keyboardButtonHome);

		JSObject keyboardButtonEnd;
		keyboardButtonEnd.SetProperty(WSLit("name"), WSLit("End"));
		keyboardButtonEnd.SetProperty(WSLit("id"), WSLit("RETROK_END"));
		keyboardButtonEnd.SetProperty(WSLit("enumValue"), JSValue(RETROK_END));
		keyboardButtons.Push(keyboardButtonEnd);

		JSObject keyboardButtonPageUp;
		keyboardButtonPageUp.SetProperty(WSLit("name"), WSLit("PageUp"));
		keyboardButtonPageUp.SetProperty(WSLit("id"), WSLit("RETROK_PAGEUP"));
		keyboardButtonPageUp.SetProperty(WSLit("enumValue"), JSValue(RETROK_PAGEUP));
		keyboardButtons.Push(keyboardButtonPageUp);

		JSObject keyboardButtonPageDown;
		keyboardButtonPageDown.SetProperty(WSLit("name"), WSLit("PageDown"));
		keyboardButtonPageDown.SetProperty(WSLit("id"), WSLit("RETROK_PAGEDOWN"));
		keyboardButtonPageDown.SetProperty(WSLit("enumValue"), JSValue(RETROK_PAGEDOWN));
		keyboardButtons.Push(keyboardButtonPageDown);

		JSObject keyboardButtonF1;
		keyboardButtonF1.SetProperty(WSLit("name"), WSLit("F1"));
		keyboardButtonF1.SetProperty(WSLit("id"), WSLit("RETROK_F1"));
		keyboardButtonF1.SetProperty(WSLit("enumValue"), JSValue(RETROK_F1));
		keyboardButtons.Push(keyboardButtonF1);

		JSObject keyboardButtonF2;
		keyboardButtonF2.SetProperty(WSLit("name"), WSLit("F2"));
		keyboardButtonF2.SetProperty(WSLit("id"), WSLit("RETROK_F2"));
		keyboardButtonF2.SetProperty(WSLit("enumValue"), JSValue(RETROK_F2));
		keyboardButtons.Push(keyboardButtonF2);

		JSObject keyboardButtonF3;
		keyboardButtonF3.SetProperty(WSLit("name"), WSLit("F3"));
		keyboardButtonF3.SetProperty(WSLit("id"), WSLit("RETROK_F3"));
		keyboardButtonF3.SetProperty(WSLit("enumValue"), JSValue(RETROK_F3));
		keyboardButtons.Push(keyboardButtonF3);

		JSObject keyboardButtonF4;
		keyboardButtonF4.SetProperty(WSLit("name"), WSLit("F4"));
		keyboardButtonF4.SetProperty(WSLit("id"), WSLit("RETROK_F4"));
		keyboardButtonF4.SetProperty(WSLit("enumValue"), JSValue(RETROK_F4));
		keyboardButtons.Push(keyboardButtonF4);

		JSObject keyboardButtonF5;
		keyboardButtonF5.SetProperty(WSLit("name"), WSLit("F5"));
		keyboardButtonF5.SetProperty(WSLit("id"), WSLit("RETROK_F5"));
		keyboardButtonF5.SetProperty(WSLit("enumValue"), JSValue(RETROK_F5));
		keyboardButtons.Push(keyboardButtonF5);

		JSObject keyboardButtonF6;
		keyboardButtonF6.SetProperty(WSLit("name"), WSLit("F6"));
		keyboardButtonF6.SetProperty(WSLit("id"), WSLit("RETROK_F6"));
		keyboardButtonF6.SetProperty(WSLit("enumValue"), JSValue(RETROK_F6));
		keyboardButtons.Push(keyboardButtonF6);

		JSObject keyboardButtonF7;
		keyboardButtonF7.SetProperty(WSLit("name"), WSLit("F7"));
		keyboardButtonF7.SetProperty(WSLit("id"), WSLit("RETROK_F7"));
		keyboardButtonF7.SetProperty(WSLit("enumValue"), JSValue(RETROK_F7));
		keyboardButtons.Push(keyboardButtonF7);

		JSObject keyboardButtonF8;
		keyboardButtonF8.SetProperty(WSLit("name"), WSLit("F8"));
		keyboardButtonF8.SetProperty(WSLit("id"), WSLit("RETROK_F8"));
		keyboardButtonF8.SetProperty(WSLit("enumValue"), JSValue(RETROK_F8));
		keyboardButtons.Push(keyboardButtonF8);

		JSObject keyboardButtonF9;
		keyboardButtonF9.SetProperty(WSLit("name"), WSLit("F9"));
		keyboardButtonF9.SetProperty(WSLit("id"), WSLit("RETROK_F9"));
		keyboardButtonF9.SetProperty(WSLit("enumValue"), JSValue(RETROK_F9));
		keyboardButtons.Push(keyboardButtonF9);

		JSObject keyboardButtonF10;
		keyboardButtonF10.SetProperty(WSLit("name"), WSLit("F10"));
		keyboardButtonF10.SetProperty(WSLit("id"), WSLit("RETROK_F10"));
		keyboardButtonF10.SetProperty(WSLit("enumValue"), JSValue(RETROK_F10));
		keyboardButtons.Push(keyboardButtonF10);

		JSObject keyboardButtonF11;
		keyboardButtonF11.SetProperty(WSLit("name"), WSLit("F11"));
		keyboardButtonF11.SetProperty(WSLit("id"), WSLit("RETROK_F11"));
		keyboardButtonF11.SetProperty(WSLit("enumValue"), JSValue(RETROK_F11));
		keyboardButtons.Push(keyboardButtonF11);

		JSObject keyboardButtonF12;
		keyboardButtonF12.SetProperty(WSLit("name"), WSLit("F12"));
		keyboardButtonF12.SetProperty(WSLit("id"), WSLit("RETROK_F12"));
		keyboardButtonF12.SetProperty(WSLit("enumValue"), JSValue(RETROK_F12));
		keyboardButtons.Push(keyboardButtonF12);

		JSObject keyboardButtonF13;
		keyboardButtonF13.SetProperty(WSLit("name"), WSLit("F13"));
		keyboardButtonF13.SetProperty(WSLit("id"), WSLit("RETROK_F13"));
		keyboardButtonF13.SetProperty(WSLit("enumValue"), JSValue(RETROK_F13));
		keyboardButtons.Push(keyboardButtonF13);

		JSObject keyboardButtonF14;
		keyboardButtonF14.SetProperty(WSLit("name"), WSLit("F14"));
		keyboardButtonF14.SetProperty(WSLit("id"), WSLit("RETROK_F14"));
		keyboardButtonF14.SetProperty(WSLit("enumValue"), JSValue(RETROK_F14));
		keyboardButtons.Push(keyboardButtonF14);

		JSObject keyboardButtonF15;
		keyboardButtonF15.SetProperty(WSLit("name"), WSLit("F15"));
		keyboardButtonF15.SetProperty(WSLit("id"), WSLit("RETROK_F15"));
		keyboardButtonF15.SetProperty(WSLit("enumValue"), JSValue(RETROK_F15));
		keyboardButtons.Push(keyboardButtonF15);

		JSObject keyboardButtonNumlock;
		keyboardButtonNumlock.SetProperty(WSLit("name"), WSLit("Numlock"));
		keyboardButtonNumlock.SetProperty(WSLit("id"), WSLit("RETROK_NUMLOCK"));
		keyboardButtonNumlock.SetProperty(WSLit("enumValue"), JSValue(RETROK_NUMLOCK));
		keyboardButtons.Push(keyboardButtonNumlock);

		JSObject keyboardButtonCapslock;
		keyboardButtonCapslock.SetProperty(WSLit("name"), WSLit("Capslock"));
		keyboardButtonCapslock.SetProperty(WSLit("id"), WSLit("RETROK_CAPSLOCK"));
		keyboardButtonCapslock.SetProperty(WSLit("enumValue"), JSValue(RETROK_CAPSLOCK));
		keyboardButtons.Push(keyboardButtonCapslock);

		JSObject keyboardButtonScrollLock;
		keyboardButtonScrollLock.SetProperty(WSLit("name"), WSLit("Scroll Lock"));
		keyboardButtonScrollLock.SetProperty(WSLit("id"), WSLit("RETROK_SCROLLOCK"));
		keyboardButtonScrollLock.SetProperty(WSLit("enumValue"), JSValue(RETROK_SCROLLOCK));
		keyboardButtons.Push(keyboardButtonScrollLock);

		JSObject keyboardButtonRShift;
		keyboardButtonRShift.SetProperty(WSLit("name"), WSLit("Right Shift"));
		keyboardButtonRShift.SetProperty(WSLit("id"), WSLit("RETROK_RSHIFT"));
		keyboardButtonRShift.SetProperty(WSLit("enumValue"), JSValue(RETROK_RSHIFT));
		keyboardButtons.Push(keyboardButtonRShift);

		JSObject keyboardButtonLShift;
		keyboardButtonLShift.SetProperty(WSLit("name"), WSLit("Left Shift"));
		keyboardButtonLShift.SetProperty(WSLit("id"), WSLit("RETROK_LSHIFT"));
		keyboardButtonLShift.SetProperty(WSLit("enumValue"), JSValue(RETROK_LSHIFT));
		keyboardButtons.Push(keyboardButtonLShift);

		JSObject keyboardButtonRCtrl;
		keyboardButtonRCtrl.SetProperty(WSLit("name"), WSLit("Right Ctrl"));
		keyboardButtonRCtrl.SetProperty(WSLit("id"), WSLit("RETROK_RCTRL"));
		keyboardButtonRCtrl.SetProperty(WSLit("enumValue"), JSValue(RETROK_RCTRL));
		keyboardButtons.Push(keyboardButtonRCtrl);

		JSObject keyboardButtonLCtrl;
		keyboardButtonLCtrl.SetProperty(WSLit("name"), WSLit("Left Ctrl"));
		keyboardButtonLCtrl.SetProperty(WSLit("id"), WSLit("RETROK_LCTRL"));
		keyboardButtonLCtrl.SetProperty(WSLit("enumValue"), JSValue(RETROK_LCTRL));
		keyboardButtons.Push(keyboardButtonLCtrl);

		JSObject keyboardButtonRAlt;
		keyboardButtonRAlt.SetProperty(WSLit("name"), WSLit("Right Alt"));
		keyboardButtonRAlt.SetProperty(WSLit("id"), WSLit("RETROK_RALT"));
		keyboardButtonRAlt.SetProperty(WSLit("enumValue"), JSValue(RETROK_RALT));
		keyboardButtons.Push(keyboardButtonRAlt);

		JSObject keyboardButtonLAlt;
		keyboardButtonLAlt.SetProperty(WSLit("name"), WSLit("Left Alt"));
		keyboardButtonLAlt.SetProperty(WSLit("id"), WSLit("RETROK_LALT"));
		keyboardButtonLAlt.SetProperty(WSLit("enumValue"), JSValue(RETROK_LALT));
		keyboardButtons.Push(keyboardButtonLAlt);

		JSObject keyboardButtonRMeta;
		keyboardButtonRMeta.SetProperty(WSLit("name"), WSLit("Right Meta"));
		keyboardButtonRMeta.SetProperty(WSLit("id"), WSLit("RETROK_RMETA"));
		keyboardButtonRMeta.SetProperty(WSLit("enumValue"), JSValue(RETROK_RMETA));
		keyboardButtons.Push(keyboardButtonRMeta);

		JSObject keyboardButtonLMeta;
		keyboardButtonLMeta.SetProperty(WSLit("name"), WSLit("Left Meta"));
		keyboardButtonLMeta.SetProperty(WSLit("id"), WSLit("RETROK_LMETA"));
		keyboardButtonLMeta.SetProperty(WSLit("enumValue"), JSValue(RETROK_LMETA));
		keyboardButtons.Push(keyboardButtonLMeta);

		JSObject keyboardButtonLSuper;
		keyboardButtonLSuper.SetProperty(WSLit("name"), WSLit("Left Super"));
		keyboardButtonLSuper.SetProperty(WSLit("id"), WSLit("RETROK_LSUPER"));
		keyboardButtonLSuper.SetProperty(WSLit("enumValue"), JSValue(RETROK_LSUPER));
		keyboardButtons.Push(keyboardButtonLSuper);

		JSObject keyboardButtonRSuper;
		keyboardButtonRSuper.SetProperty(WSLit("name"), WSLit("Right Super"));
		keyboardButtonRSuper.SetProperty(WSLit("id"), WSLit("RETROK_RSUPER"));
		keyboardButtonRSuper.SetProperty(WSLit("enumValue"), JSValue(RETROK_RSUPER));
		keyboardButtons.Push(keyboardButtonRSuper);

		JSObject keyboardButtonMode;
		keyboardButtonMode.SetProperty(WSLit("name"), WSLit("Mode"));
		keyboardButtonMode.SetProperty(WSLit("id"), WSLit("RETROK_MODE"));
		keyboardButtonMode.SetProperty(WSLit("enumValue"), JSValue(RETROK_MODE));
		keyboardButtons.Push(keyboardButtonMode);

		JSObject keyboardButtonCompose;
		keyboardButtonCompose.SetProperty(WSLit("name"), WSLit("Compose"));
		keyboardButtonCompose.SetProperty(WSLit("id"), WSLit("RETROK_COMPOSE"));
		keyboardButtonCompose.SetProperty(WSLit("enumValue"), JSValue(RETROK_COMPOSE));
		keyboardButtons.Push(keyboardButtonCompose);

		JSObject keyboardButtonHelp;
		keyboardButtonHelp.SetProperty(WSLit("name"), WSLit("Help"));
		keyboardButtonHelp.SetProperty(WSLit("id"), WSLit("RETROK_HELP"));
		keyboardButtonHelp.SetProperty(WSLit("enumValue"), JSValue(RETROK_HELP));
		keyboardButtons.Push(keyboardButtonHelp);

		JSObject keyboardButtonPrint;
		keyboardButtonPrint.SetProperty(WSLit("name"), WSLit("Print"));
		keyboardButtonPrint.SetProperty(WSLit("id"), WSLit("RETROK_PRINT"));
		keyboardButtonPrint.SetProperty(WSLit("enumValue"), JSValue(RETROK_PRINT));
		keyboardButtons.Push(keyboardButtonPrint);

		JSObject keyboardButtonSysReq;
		keyboardButtonSysReq.SetProperty(WSLit("name"), WSLit("Sys Req"));
		keyboardButtonSysReq.SetProperty(WSLit("id"), WSLit("RETROK_SYSREQ"));
		keyboardButtonSysReq.SetProperty(WSLit("enumValue"), JSValue(RETROK_SYSREQ));
		keyboardButtons.Push(keyboardButtonSysReq);

		JSObject keyboardButtonBreak;
		keyboardButtonBreak.SetProperty(WSLit("name"), WSLit("Break"));
		keyboardButtonBreak.SetProperty(WSLit("id"), WSLit("RETROK_BREAK"));
		keyboardButtonBreak.SetProperty(WSLit("enumValue"), JSValue(RETROK_BREAK));
		keyboardButtons.Push(keyboardButtonBreak);

		JSObject keyboardButtonMenu;
		keyboardButtonMenu.SetProperty(WSLit("name"), WSLit("Menu"));
		keyboardButtonMenu.SetProperty(WSLit("id"), WSLit("RETROK_MENU"));
		keyboardButtonMenu.SetProperty(WSLit("enumValue"), JSValue(RETROK_MENU));
		keyboardButtons.Push(keyboardButtonMenu);

		JSObject keyboardButtonPower;
		keyboardButtonPower.SetProperty(WSLit("name"), WSLit("Power"));
		keyboardButtonPower.SetProperty(WSLit("id"), WSLit("RETROK_POWER"));
		keyboardButtonPower.SetProperty(WSLit("enumValue"), JSValue(RETROK_POWER));
		keyboardButtons.Push(keyboardButtonPower);

		JSObject keyboardButtonEuro;
		keyboardButtonEuro.SetProperty(WSLit("name"), WSLit("Euro"));
		keyboardButtonEuro.SetProperty(WSLit("id"), WSLit("RETROK_EURO"));
		keyboardButtonEuro.SetProperty(WSLit("enumValue"), JSValue(RETROK_EURO));
		keyboardButtons.Push(keyboardButtonEuro);

		JSObject keyboardButtonUndo;
		keyboardButtonUndo.SetProperty(WSLit("name"), WSLit("Undo"));
		keyboardButtonUndo.SetProperty(WSLit("id"), WSLit("RETROK_UNDO"));
		keyboardButtonUndo.SetProperty(WSLit("enumValue"), JSValue(RETROK_UNDO));
		keyboardButtons.Push(keyboardButtonUndo);

		keyboard.SetProperty(WSLit("buttons"), keyboardButtons);
		devices.SetProperty(WSLit("RETRO_DEVICE_KEYBOARD"), keyboard);

		// LIGHTGUN
		JSObject lightgun;
		lightgun.SetProperty(WSLit("enumValue"), JSValue(4));
		lightgun.SetProperty(WSLit("name"), JSValue(WSLit("RetroLightgun")));
		lightgun.SetProperty(WSLit("id"), JSValue(WSLit("RETRO_DEVICE_LIGHTGUN")));

		JSArray lightgunButtons;

		JSObject lightgunButtonX;
		lightgunButtonX.SetProperty(WSLit("name"), WSLit("X-Axis"));
		lightgunButtonX.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_LIGHTGUN_X"));
		lightgunButtonX.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_LIGHTGUN_X));
		lightgunButtons.Push(lightgunButtonX);

		JSObject lightgunButtonY;
		lightgunButtonY.SetProperty(WSLit("name"), WSLit("Y-Axis"));
		lightgunButtonY.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_LIGHTGUN_Y"));
		lightgunButtonY.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_LIGHTGUN_Y));
		lightgunButtons.Push(lightgunButtonY);

		JSObject lightgunButtonTrigger;
		lightgunButtonTrigger.SetProperty(WSLit("name"), WSLit("Trigger"));
		lightgunButtonTrigger.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_LIGHTGUN_TRIGGER"));
		lightgunButtonTrigger.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_LIGHTGUN_TRIGGER));
		lightgunButtons.Push(lightgunButtonTrigger);

		JSObject lightgunButtonCursor;
		lightgunButtonCursor.SetProperty(WSLit("name"), WSLit("Cursor"));
		lightgunButtonCursor.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_LIGHTGUN_CURSOR"));
		lightgunButtonCursor.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_LIGHTGUN_CURSOR));
		lightgunButtons.Push(lightgunButtonCursor);

		JSObject lightgunButtonTurbo;
		lightgunButtonTurbo.SetProperty(WSLit("name"), WSLit("Turbo"));
		lightgunButtonTurbo.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_LIGHTGUN_TURBO"));
		lightgunButtonTurbo.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_LIGHTGUN_TURBO));
		lightgunButtons.Push(lightgunButtonTurbo);

		JSObject lightgunButtonPause;
		lightgunButtonPause.SetProperty(WSLit("name"), WSLit("Pause"));
		lightgunButtonPause.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_LIGHTGUN_PAUSE"));
		lightgunButtonPause.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_LIGHTGUN_PAUSE));
		lightgunButtons.Push(lightgunButtonPause);

		JSObject lightgunButtonStart;
		lightgunButtonStart.SetProperty(WSLit("name"), WSLit("Start"));
		lightgunButtonStart.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_LIGHTGUN_START"));
		lightgunButtonStart.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_LIGHTGUN_START));
		lightgunButtons.Push(lightgunButtonStart);

		lightgun.SetProperty(WSLit("buttons"), lightgunButtons);
		devices.SetProperty(WSLit("RETRO_DEVICE_LIGHTGUN"), lightgun);

		// ANALOG
		JSObject analog;
		analog.SetProperty(WSLit("enumValue"), JSValue(5));
		analog.SetProperty(WSLit("name"), JSValue(WSLit("RetroAnalog")));
		analog.SetProperty(WSLit("id"), JSValue(WSLit("RETRO_DEVICE_ANALOG")));

		JSArray analogButtons;

		JSObject analogButtonAxisX;
		analogButtonAxisX.SetProperty(WSLit("name"), WSLit("X-Axis"));
		analogButtonAxisX.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_ANALOG_X"));
		analogButtonAxisX.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_ANALOG_X));
		analogButtons.Push(analogButtonAxisX);

		JSObject analogButtonAxisY;
		analogButtonAxisY.SetProperty(WSLit("name"), WSLit("Y-Axis"));
		analogButtonAxisY.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_ANALOG_Y"));
		analogButtonAxisY.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_ANALOG_Y));
		analogButtons.Push(analogButtonAxisY);

		analog.SetProperty(WSLit("buttons"), analogButtons);
		devices.SetProperty(WSLit("RETRO_DEVICE_ANALOG"), analog);

		// POINTER
		JSObject pointer;
		pointer.SetProperty(WSLit("enumValue"), JSValue(6));
		pointer.SetProperty(WSLit("name"), JSValue(WSLit("RetroPointer")));
		pointer.SetProperty(WSLit("id"), JSValue(WSLit("RETRO_DEVICE_POINTER")));

		JSArray pointerButtons;

		JSObject pointerButtonX;
		pointerButtonX.SetProperty(WSLit("name"), WSLit("X-Axis"));
		pointerButtonX.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_POINTER_X"));
		pointerButtonX.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_POINTER_X));
		pointerButtons.Push(pointerButtonX);

		JSObject pointerButtonY;
		pointerButtonY.SetProperty(WSLit("name"), WSLit("Y-Axis"));
		pointerButtonY.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_POINTER_Y"));
		pointerButtonY.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_POINTER_Y));
		pointerButtons.Push(pointerButtonY);

		JSObject pointerButtonPressed;
		pointerButtonPressed.SetProperty(WSLit("name"), WSLit("Pressed"));
		pointerButtonPressed.SetProperty(WSLit("id"), WSLit("RETRO_DEVICE_ID_POINTER_PRESSED"));
		pointerButtonPressed.SetProperty(WSLit("enumValue"), JSValue(RETRO_DEVICE_ID_POINTER_PRESSED));
		pointerButtons.Push(pointerButtonPressed);

		pointer.SetProperty(WSLit("buttons"), pointerButtons);
		devices.SetProperty(WSLit("RETRO_DEVICE_POINTER"), pointer);

		*/
		
		response.SetProperty(WSLit("devices"), devices);

		JSArray sourceButtons;

		JSObject sourceButtonUnbound;
		sourceButtonUnbound.SetProperty(WSLit("id"), WSLit("unbound"));
		sourceButtonUnbound.SetProperty(WSLit("name"), WSLit("unbound"));
		sourceButtonUnbound.SetProperty(WSLit("keyCodeDown"), JSValue(-1));
		sourceButtons.Push(sourceButtonUnbound);

		JSObject sourceButtonKey0;
		sourceButtonKey0.SetProperty(WSLit("id"), WSLit("KEY_0"));
		sourceButtonKey0.SetProperty(WSLit("name"), WSLit("Keyboard 0"));
		sourceButtonKey0.SetProperty(WSLit("keyCodeDown"), JSValue(49));
		sourceButtons.Push(sourceButtonKey0);

		JSObject sourceButtonKey1;
		sourceButtonKey1.SetProperty(WSLit("id"), WSLit("KEY_1"));
		sourceButtonKey1.SetProperty(WSLit("name"), WSLit("Keyboard 1"));
		sourceButtonKey1.SetProperty(WSLit("keyCodeDown"), JSValue(49));
		sourceButtons.Push(sourceButtonKey1);

		JSObject sourceButtonKey2;
		sourceButtonKey2.SetProperty(WSLit("id"), WSLit("KEY_2"));
		sourceButtonKey2.SetProperty(WSLit("name"), WSLit("Keyboard 2"));
		sourceButtonKey2.SetProperty(WSLit("keyCodeDown"), JSValue(50));
		sourceButtons.Push(sourceButtonKey2);

		JSObject sourceButtonKey3;
		sourceButtonKey3.SetProperty(WSLit("id"), WSLit("KEY_3"));
		sourceButtonKey3.SetProperty(WSLit("name"), WSLit("Keyboard 3"));
		sourceButtonKey3.SetProperty(WSLit("keyCodeDown"), JSValue(51));
		sourceButtons.Push(sourceButtonKey3);

		JSObject sourceButtonKey4;
		sourceButtonKey4.SetProperty(WSLit("id"), WSLit("KEY_4"));
		sourceButtonKey4.SetProperty(WSLit("name"), WSLit("Keyboard 4"));
		sourceButtonKey4.SetProperty(WSLit("keyCodeDown"), JSValue(52));
		sourceButtons.Push(sourceButtonKey4);

		JSObject sourceButtonKey5;
		sourceButtonKey5.SetProperty(WSLit("id"), WSLit("KEY_5"));
		sourceButtonKey5.SetProperty(WSLit("name"), WSLit("Keyboard 5"));
		sourceButtonKey5.SetProperty(WSLit("keyCodeDown"), JSValue(53));
		sourceButtons.Push(sourceButtonKey5);

		JSObject sourceButtonKey6;
		sourceButtonKey6.SetProperty(WSLit("id"), WSLit("KEY_6"));
		sourceButtonKey6.SetProperty(WSLit("name"), WSLit("Keyboard 6"));
		sourceButtonKey6.SetProperty(WSLit("keyCodeDown"), JSValue(54));
		sourceButtons.Push(sourceButtonKey6);

		JSObject sourceButtonKey7;
		sourceButtonKey7.SetProperty(WSLit("id"), WSLit("KEY_7"));
		sourceButtonKey7.SetProperty(WSLit("name"), WSLit("Keyboard 7"));
		sourceButtonKey7.SetProperty(WSLit("keyCodeDown"), JSValue(55));
		sourceButtons.Push(sourceButtonKey7);

		JSObject sourceButtonKey8;
		sourceButtonKey8.SetProperty(WSLit("id"), WSLit("KEY_8"));
		sourceButtonKey8.SetProperty(WSLit("name"), WSLit("Keyboard 8"));
		sourceButtonKey8.SetProperty(WSLit("keyCodeDown"), JSValue(56));
		sourceButtons.Push(sourceButtonKey8);

		JSObject sourceButtonKey9;
		sourceButtonKey9.SetProperty(WSLit("id"), WSLit("KEY_9"));
		sourceButtonKey9.SetProperty(WSLit("name"), WSLit("Keyboard 9"));
		sourceButtonKey9.SetProperty(WSLit("keyCodeDown"), JSValue(57));
		sourceButtons.Push(sourceButtonKey9);

		JSObject sourceButtonKeyA;
		sourceButtonKeyA.SetProperty(WSLit("id"), WSLit("KEY_A"));
		sourceButtonKeyA.SetProperty(WSLit("name"), WSLit("Keyboard A"));
		sourceButtonKeyA.SetProperty(WSLit("keyCodeDown"), JSValue(65));
		sourceButtons.Push(sourceButtonKeyA);

		JSObject sourceButtonKeyB;
		sourceButtonKeyB.SetProperty(WSLit("id"), WSLit("KEY_B"));
		sourceButtonKeyB.SetProperty(WSLit("name"), WSLit("Keyboard B"));
		sourceButtonKeyB.SetProperty(WSLit("keyCodeDown"), JSValue(66));
		sourceButtons.Push(sourceButtonKeyB);

		JSObject sourceButtonKeyC;
		sourceButtonKeyC.SetProperty(WSLit("id"), WSLit("KEY_C"));
		sourceButtonKeyC.SetProperty(WSLit("name"), WSLit("Keyboard C"));
		sourceButtonKeyC.SetProperty(WSLit("keyCodeDown"), JSValue(67));
		sourceButtons.Push(sourceButtonKeyC);

		JSObject sourceButtonKeyD;
		sourceButtonKeyD.SetProperty(WSLit("id"), WSLit("KEY_D"));
		sourceButtonKeyD.SetProperty(WSLit("name"), WSLit("Keyboard D"));
		sourceButtonKeyD.SetProperty(WSLit("keyCodeDown"), JSValue(68));
		sourceButtons.Push(sourceButtonKeyD);

		JSObject sourceButtonKeyE;
		sourceButtonKeyE.SetProperty(WSLit("id"), WSLit("KEY_E"));
		sourceButtonKeyE.SetProperty(WSLit("name"), WSLit("Keyboard E"));
		sourceButtonKeyE.SetProperty(WSLit("keyCodeDown"), JSValue(69));
		sourceButtons.Push(sourceButtonKeyE);

		JSObject sourceButtonKeyF;
		sourceButtonKeyF.SetProperty(WSLit("id"), WSLit("KEY_F"));
		sourceButtonKeyF.SetProperty(WSLit("name"), WSLit("Keyboard F"));
		sourceButtonKeyF.SetProperty(WSLit("keyCodeDown"), JSValue(70));
		sourceButtons.Push(sourceButtonKeyF);

		JSObject sourceButtonKeyG;
		sourceButtonKeyG.SetProperty(WSLit("id"), WSLit("KEY_G"));
		sourceButtonKeyG.SetProperty(WSLit("name"), WSLit("Keyboard G"));
		sourceButtonKeyG.SetProperty(WSLit("keyCodeDown"), JSValue(71));
		sourceButtons.Push(sourceButtonKeyG);

		JSObject sourceButtonKeyH;
		sourceButtonKeyH.SetProperty(WSLit("id"), WSLit("KEY_H"));
		sourceButtonKeyH.SetProperty(WSLit("name"), WSLit("Keyboard H"));
		sourceButtonKeyH.SetProperty(WSLit("keyCodeDown"), JSValue(72));
		sourceButtons.Push(sourceButtonKeyH);

		JSObject sourceButtonKeyI;
		sourceButtonKeyI.SetProperty(WSLit("id"), WSLit("KEY_I"));
		sourceButtonKeyI.SetProperty(WSLit("name"), WSLit("Keyboard I"));
		sourceButtonKeyI.SetProperty(WSLit("keyCodeDown"), JSValue(73));
		sourceButtons.Push(sourceButtonKeyI);

		JSObject sourceButtonKeyJ;
		sourceButtonKeyJ.SetProperty(WSLit("id"), WSLit("KEY_J"));
		sourceButtonKeyJ.SetProperty(WSLit("name"), WSLit("Keyboard J"));
		sourceButtonKeyJ.SetProperty(WSLit("keyCodeDown"), JSValue(74));
		sourceButtons.Push(sourceButtonKeyJ);

		JSObject sourceButtonKeyK;
		sourceButtonKeyK.SetProperty(WSLit("id"), WSLit("KEY_K"));
		sourceButtonKeyK.SetProperty(WSLit("name"), WSLit("Keyboard K"));
		sourceButtonKeyK.SetProperty(WSLit("keyCodeDown"), JSValue(75));
		sourceButtons.Push(sourceButtonKeyK);

		JSObject sourceButtonKeyL;
		sourceButtonKeyL.SetProperty(WSLit("id"), WSLit("KEY_L"));
		sourceButtonKeyL.SetProperty(WSLit("name"), WSLit("Keyboard L"));
		sourceButtonKeyL.SetProperty(WSLit("keyCodeDown"), JSValue(76));
		sourceButtons.Push(sourceButtonKeyL);

		JSObject sourceButtonKeyM;
		sourceButtonKeyM.SetProperty(WSLit("id"), WSLit("KEY_M"));
		sourceButtonKeyM.SetProperty(WSLit("name"), WSLit("Keyboard M"));
		sourceButtonKeyM.SetProperty(WSLit("keyCodeDown"), JSValue(77));
		sourceButtons.Push(sourceButtonKeyM);

		JSObject sourceButtonKeyN;
		sourceButtonKeyN.SetProperty(WSLit("id"), WSLit("KEY_N"));
		sourceButtonKeyN.SetProperty(WSLit("name"), WSLit("Keyboard N"));
		sourceButtonKeyN.SetProperty(WSLit("keyCodeDown"), JSValue(78));
		sourceButtons.Push(sourceButtonKeyN);

		JSObject sourceButtonKeyO;
		sourceButtonKeyO.SetProperty(WSLit("id"), WSLit("KEY_O"));
		sourceButtonKeyO.SetProperty(WSLit("name"), WSLit("Keyboard O"));
		sourceButtonKeyO.SetProperty(WSLit("keyCodeDown"), JSValue(79));
		sourceButtons.Push(sourceButtonKeyO);

		JSObject sourceButtonKeyP;
		sourceButtonKeyP.SetProperty(WSLit("id"), WSLit("KEY_P"));
		sourceButtonKeyP.SetProperty(WSLit("name"), WSLit("Keyboard P"));
		sourceButtonKeyP.SetProperty(WSLit("keyCodeDown"), JSValue(80));
		sourceButtons.Push(sourceButtonKeyP);

		JSObject sourceButtonKeyQ;
		sourceButtonKeyQ.SetProperty(WSLit("id"), WSLit("KEY_Q"));
		sourceButtonKeyQ.SetProperty(WSLit("name"), WSLit("Keyboard Q"));
		sourceButtonKeyQ.SetProperty(WSLit("keyCodeDown"), JSValue(81));
		sourceButtons.Push(sourceButtonKeyQ);

		JSObject sourceButtonKeyR;
		sourceButtonKeyR.SetProperty(WSLit("id"), WSLit("KEY_R"));
		sourceButtonKeyR.SetProperty(WSLit("name"), WSLit("Keyboard R"));
		sourceButtonKeyR.SetProperty(WSLit("keyCodeDown"), JSValue(82));
		sourceButtons.Push(sourceButtonKeyR);

		JSObject sourceButtonKeyS;
		sourceButtonKeyS.SetProperty(WSLit("id"), WSLit("KEY_S"));
		sourceButtonKeyS.SetProperty(WSLit("name"), WSLit("Keyboard S"));
		sourceButtonKeyS.SetProperty(WSLit("keyCodeDown"), JSValue(83));
		sourceButtons.Push(sourceButtonKeyS);

		JSObject sourceButtonKeyT;
		sourceButtonKeyT.SetProperty(WSLit("id"), WSLit("KEY_T"));
		sourceButtonKeyT.SetProperty(WSLit("name"), WSLit("Keyboard T"));
		sourceButtonKeyT.SetProperty(WSLit("keyCodeDown"), JSValue(84));
		sourceButtons.Push(sourceButtonKeyT);

		JSObject sourceButtonKeyU;
		sourceButtonKeyU.SetProperty(WSLit("id"), WSLit("KEY_U"));
		sourceButtonKeyU.SetProperty(WSLit("name"), WSLit("Keyboard U"));
		sourceButtonKeyU.SetProperty(WSLit("keyCodeDown"), JSValue(85));
		sourceButtons.Push(sourceButtonKeyU);

		JSObject sourceButtonKeyV;
		sourceButtonKeyV.SetProperty(WSLit("id"), WSLit("KEY_V"));
		sourceButtonKeyV.SetProperty(WSLit("name"), WSLit("Keyboard V"));
		sourceButtonKeyV.SetProperty(WSLit("keyCodeDown"), JSValue(86));
		sourceButtons.Push(sourceButtonKeyV);

		JSObject sourceButtonKeyW;
		sourceButtonKeyW.SetProperty(WSLit("id"), WSLit("KEY_W"));
		sourceButtonKeyW.SetProperty(WSLit("name"), WSLit("Keyboard W"));
		sourceButtonKeyW.SetProperty(WSLit("keyCodeDown"), JSValue(87));
		sourceButtons.Push(sourceButtonKeyW);

		JSObject sourceButtonKeyX;
		sourceButtonKeyX.SetProperty(WSLit("id"), WSLit("KEY_X"));
		sourceButtonKeyX.SetProperty(WSLit("name"), WSLit("Keyboard X"));
		sourceButtonKeyX.SetProperty(WSLit("keyCodeDown"), JSValue(88));
		sourceButtons.Push(sourceButtonKeyX);

		JSObject sourceButtonKeyY;
		sourceButtonKeyY.SetProperty(WSLit("id"), WSLit("KEY_Y"));
		sourceButtonKeyY.SetProperty(WSLit("name"), WSLit("Keyboard Y"));
		sourceButtonKeyY.SetProperty(WSLit("keyCodeDown"), JSValue(89));
		sourceButtons.Push(sourceButtonKeyY);

		JSObject sourceButtonKeyZ;
		sourceButtonKeyZ.SetProperty(WSLit("id"), WSLit("KEY_Z"));
		sourceButtonKeyZ.SetProperty(WSLit("name"), WSLit("Keyboard Z"));
		sourceButtonKeyZ.SetProperty(WSLit("keyCodeDown"), JSValue(90));
		sourceButtons.Push(sourceButtonKeyZ);

		JSObject sourceButtonKeyPad0;
		sourceButtonKeyPad0.SetProperty(WSLit("id"), WSLit("KEY_PAD_0"));
		sourceButtonKeyPad0.SetProperty(WSLit("name"), WSLit("Numpad 0"));
		sourceButtonKeyPad0.SetProperty(WSLit("keyCodeDown"), JSValue(96));
		sourceButtons.Push(sourceButtonKeyPad0);

		JSObject sourceButtonKeyPad1;
		sourceButtonKeyPad1.SetProperty(WSLit("id"), WSLit("KEY_PAD_1"));
		sourceButtonKeyPad1.SetProperty(WSLit("name"), WSLit("Numpad 1"));
		sourceButtonKeyPad1.SetProperty(WSLit("keyCodeDown"), JSValue(97));
		sourceButtons.Push(sourceButtonKeyPad1);

		JSObject sourceButtonKeyPad2;
		sourceButtonKeyPad2.SetProperty(WSLit("id"), WSLit("KEY_PAD_2"));
		sourceButtonKeyPad2.SetProperty(WSLit("name"), WSLit("Numpad 2"));
		sourceButtonKeyPad2.SetProperty(WSLit("keyCodeDown"), JSValue(98));
		sourceButtons.Push(sourceButtonKeyPad2);

		JSObject sourceButtonKeyPad3;
		sourceButtonKeyPad3.SetProperty(WSLit("id"), WSLit("KEY_PAD_3"));
		sourceButtonKeyPad3.SetProperty(WSLit("name"), WSLit("Numpad 3"));
		sourceButtonKeyPad3.SetProperty(WSLit("keyCodeDown"), JSValue(99));
		sourceButtons.Push(sourceButtonKeyPad3);

		JSObject sourceButtonKeyPad4;
		sourceButtonKeyPad4.SetProperty(WSLit("id"), WSLit("KEY_PAD_4"));
		sourceButtonKeyPad4.SetProperty(WSLit("name"), WSLit("Numpad 4"));
		sourceButtonKeyPad4.SetProperty(WSLit("keyCodeDown"), JSValue(100));
		sourceButtons.Push(sourceButtonKeyPad4);

		JSObject sourceButtonKeyPad5;
		sourceButtonKeyPad5.SetProperty(WSLit("id"), WSLit("KEY_PAD_5"));
		sourceButtonKeyPad5.SetProperty(WSLit("name"), WSLit("Numpad 5"));
		sourceButtonKeyPad5.SetProperty(WSLit("keyCodeDown"), JSValue(101));
		sourceButtons.Push(sourceButtonKeyPad5);

		JSObject sourceButtonKeyPad6;
		sourceButtonKeyPad6.SetProperty(WSLit("id"), WSLit("KEY_PAD_6"));
		sourceButtonKeyPad6.SetProperty(WSLit("name"), WSLit("Numpad 6"));
		sourceButtonKeyPad6.SetProperty(WSLit("keyCodeDown"), JSValue(102));
		sourceButtons.Push(sourceButtonKeyPad6);

		JSObject sourceButtonKeyPad7;
		sourceButtonKeyPad7.SetProperty(WSLit("id"), WSLit("KEY_PAD_7"));
		sourceButtonKeyPad7.SetProperty(WSLit("name"), WSLit("Numpad 7"));
		sourceButtonKeyPad7.SetProperty(WSLit("keyCodeDown"), JSValue(103));
		sourceButtons.Push(sourceButtonKeyPad7);

		JSObject sourceButtonKeyPad8;
		sourceButtonKeyPad8.SetProperty(WSLit("id"), WSLit("KEY_PAD_8"));
		sourceButtonKeyPad8.SetProperty(WSLit("name"), WSLit("Numpad 8"));
		sourceButtonKeyPad8.SetProperty(WSLit("keyCodeDown"), JSValue(104));
		sourceButtons.Push(sourceButtonKeyPad8);

		JSObject sourceButtonKeyPad9;
		sourceButtonKeyPad9.SetProperty(WSLit("id"), WSLit("KEY_PAD_9"));
		sourceButtonKeyPad9.SetProperty(WSLit("name"), WSLit("Numpad 9"));
		sourceButtonKeyPad9.SetProperty(WSLit("keyCodeDown"), JSValue(105));
		sourceButtons.Push(sourceButtonKeyPad9);

		JSObject sourceButtonKeyPadDivide;
		sourceButtonKeyPadDivide.SetProperty(WSLit("id"), WSLit("KEY_PAD_DIVIDE"));
		sourceButtonKeyPadDivide.SetProperty(WSLit("name"), WSLit("Numpad /"));
		sourceButtonKeyPadDivide.SetProperty(WSLit("keyCodeDown"), JSValue(111));
		sourceButtons.Push(sourceButtonKeyPadDivide);

		JSObject sourceButtonKeyPadMultiply;
		sourceButtonKeyPadMultiply.SetProperty(WSLit("id"), WSLit("KEY_PAD_MULTIPLY"));
		sourceButtonKeyPadMultiply.SetProperty(WSLit("name"), WSLit("Numpad *"));
		sourceButtonKeyPadMultiply.SetProperty(WSLit("keyCodeDown"), JSValue(106));
		sourceButtons.Push(sourceButtonKeyPadMultiply);

		JSObject sourceButtonKeyPadMinus;
		sourceButtonKeyPadMinus.SetProperty(WSLit("id"), WSLit("KEY_PAD_MINUS"));
		sourceButtonKeyPadMinus.SetProperty(WSLit("name"), WSLit("Numpad -"));
		sourceButtonKeyPadMinus.SetProperty(WSLit("keyCodeDown"), JSValue(109));
		sourceButtons.Push(sourceButtonKeyPadMinus);

		JSObject sourceButtonKeyPadPlus;
		sourceButtonKeyPadPlus.SetProperty(WSLit("id"), WSLit("KEY_PAD_PLUS"));
		sourceButtonKeyPadPlus.SetProperty(WSLit("name"), WSLit("Numpad +"));
		sourceButtonKeyPadPlus.SetProperty(WSLit("keyCodeDown"), JSValue(107));
		sourceButtons.Push(sourceButtonKeyPadPlus);

		JSObject sourceButtonKeyPadEnter;
		sourceButtonKeyPadEnter.SetProperty(WSLit("id"), WSLit("KEY_PAD_ENTER"));
		sourceButtonKeyPadEnter.SetProperty(WSLit("name"), WSLit("Numpad Enter"));
		sourceButtonKeyPadEnter.SetProperty(WSLit("keyCodeDown"), JSValue(13));
		sourceButtons.Push(sourceButtonKeyPadEnter);

		JSObject sourceButtonKeyPadDecimal;
		sourceButtonKeyPadDecimal.SetProperty(WSLit("id"), WSLit("KEY_PAD_DECIMAL"));
		sourceButtonKeyPadDecimal.SetProperty(WSLit("name"), WSLit("Numpad ."));
		sourceButtonKeyPadDecimal.SetProperty(WSLit("keyCodeDown"), JSValue(110));
		sourceButtons.Push(sourceButtonKeyPadDecimal);

		JSObject sourceButtonKeyLBracket;
		sourceButtonKeyLBracket.SetProperty(WSLit("id"), WSLit("KEY_LBRACKET"));
		sourceButtonKeyLBracket.SetProperty(WSLit("name"), WSLit("Keyboard ["));
		sourceButtonKeyLBracket.SetProperty(WSLit("keyCodeDown"), JSValue(219));
		sourceButtons.Push(sourceButtonKeyLBracket);

		JSObject sourceButtonKeyRBracket;
		sourceButtonKeyRBracket.SetProperty(WSLit("id"), WSLit("KEY_RBRACKET"));
		sourceButtonKeyRBracket.SetProperty(WSLit("name"), WSLit("Keyboard ]"));
		sourceButtonKeyRBracket.SetProperty(WSLit("keyCodeDown"), JSValue(221));
		sourceButtons.Push(sourceButtonKeyRBracket);

		JSObject sourceButtonKeySemicolon;
		sourceButtonKeySemicolon.SetProperty(WSLit("id"), WSLit("KEY_SEMICOLON"));
		sourceButtonKeySemicolon.SetProperty(WSLit("name"), WSLit("Keyboard ;"));
		sourceButtonKeySemicolon.SetProperty(WSLit("keyCodeDown"), JSValue(186));
		sourceButtons.Push(sourceButtonKeySemicolon);

		JSObject sourceButtonKeyApostrophe;
		sourceButtonKeyApostrophe.SetProperty(WSLit("id"), WSLit("KEY_APOSTROPHE"));
		sourceButtonKeyApostrophe.SetProperty(WSLit("name"), WSLit("Keyboard '"));
		sourceButtonKeyApostrophe.SetProperty(WSLit("keyCodeDown"), JSValue(222));
		sourceButtons.Push(sourceButtonKeyApostrophe);

		JSObject sourceButtonKeyBackquote;
		sourceButtonKeyBackquote.SetProperty(WSLit("id"), WSLit("KEY_BACKQUOTE"));
		sourceButtonKeyBackquote.SetProperty(WSLit("name"), WSLit("Keyboard `"));
		sourceButtonKeyBackquote.SetProperty(WSLit("keyCodeDown"), JSValue(192));
		sourceButtons.Push(sourceButtonKeyBackquote);

		JSObject sourceButtonKeyComma;
		sourceButtonKeyComma.SetProperty(WSLit("id"), WSLit("KEY_COMMA"));
		sourceButtonKeyComma.SetProperty(WSLit("name"), WSLit("Keyboard ,"));
		sourceButtonKeyComma.SetProperty(WSLit("keyCodeDown"), JSValue(188));
		sourceButtons.Push(sourceButtonKeyComma);

		JSObject sourceButtonKeyPeroid;
		sourceButtonKeyPeroid.SetProperty(WSLit("id"), WSLit("KEY_PERIOD"));
		sourceButtonKeyPeroid.SetProperty(WSLit("name"), WSLit("Keyboard ."));
		sourceButtonKeyPeroid.SetProperty(WSLit("keyCodeDown"), JSValue(190));
		sourceButtons.Push(sourceButtonKeyPeroid);

		JSObject sourceButtonKeySlash;
		sourceButtonKeySlash.SetProperty(WSLit("id"), WSLit("KEY_SLASH"));
		sourceButtonKeySlash.SetProperty(WSLit("name"), WSLit("Keyboard /"));
		sourceButtonKeySlash.SetProperty(WSLit("keyCodeDown"), JSValue(191));
		sourceButtons.Push(sourceButtonKeySlash);

		JSObject sourceButtonKeyBackslash;
		sourceButtonKeyBackslash.SetProperty(WSLit("id"), WSLit("KEY_BACKSLASH"));
		sourceButtonKeyBackslash.SetProperty(WSLit("name"), WSLit("Keyboard \\"));
		sourceButtonKeyBackslash.SetProperty(WSLit("keyCodeDown"), JSValue(220));
		sourceButtons.Push(sourceButtonKeyBackslash);

		JSObject sourceButtonKeyMinus;
		sourceButtonKeyMinus.SetProperty(WSLit("id"), WSLit("KEY_MINUS"));
		sourceButtonKeyMinus.SetProperty(WSLit("name"), WSLit("Keyboard -"));
		sourceButtonKeyMinus.SetProperty(WSLit("keyCodeDown"), JSValue(189));
		sourceButtons.Push(sourceButtonKeyMinus);

		JSObject sourceButtonKeyEqual;
		sourceButtonKeyEqual.SetProperty(WSLit("id"), WSLit("KEY_EQUAL"));
		sourceButtonKeyEqual.SetProperty(WSLit("name"), WSLit("Keyboard ="));
		sourceButtonKeyEqual.SetProperty(WSLit("keyCodeDown"), JSValue(187));
		sourceButtons.Push(sourceButtonKeyEqual);

		JSObject sourceButtonKeyEnter;
		sourceButtonKeyEnter.SetProperty(WSLit("id"), WSLit("KEY_ENTER"));
		sourceButtonKeyEnter.SetProperty(WSLit("name"), WSLit("Keyboard Enter"));
		sourceButtonKeyEnter.SetProperty(WSLit("keyCodeDown"), JSValue(13));
		sourceButtons.Push(sourceButtonKeyEnter);

		JSObject sourceButtonKeySpace;
		sourceButtonKeySpace.SetProperty(WSLit("id"), WSLit("KEY_SPACE"));
		sourceButtonKeySpace.SetProperty(WSLit("name"), WSLit("Keyboard Space"));
		sourceButtonKeySpace.SetProperty(WSLit("keyCodeDown"), JSValue(32));
		sourceButtons.Push(sourceButtonKeySpace);

		JSObject sourceButtonKeyBackspace;
		sourceButtonKeyBackspace.SetProperty(WSLit("id"), WSLit("KEY_BACKSPACE"));
		sourceButtonKeyBackspace.SetProperty(WSLit("name"), WSLit("Keyboard Backspace"));
		sourceButtonKeyBackspace.SetProperty(WSLit("keyCodeDown"), JSValue(8));
		sourceButtons.Push(sourceButtonKeyBackspace);

		JSObject sourceButtonKeyTab;
		sourceButtonKeyTab.SetProperty(WSLit("id"), WSLit("KEY_TAB"));
		sourceButtonKeyTab.SetProperty(WSLit("name"), WSLit("Keyboard Tab"));
		sourceButtonKeyTab.SetProperty(WSLit("keyCodeDown"), JSValue(9));
		sourceButtons.Push(sourceButtonKeyTab);

		JSObject sourceButtonKeyCapslock;
		sourceButtonKeyCapslock.SetProperty(WSLit("id"), WSLit("KEY_CAPSLOCK"));
		sourceButtonKeyCapslock.SetProperty(WSLit("name"), WSLit("Keyboard Capslock"));
		sourceButtonKeyCapslock.SetProperty(WSLit("keyCodeDown"), JSValue(20));
		sourceButtons.Push(sourceButtonKeyCapslock);

		JSObject sourceButtonKeyNumlock;
		sourceButtonKeyNumlock.SetProperty(WSLit("id"), WSLit("KEY_NUMLOCK"));
		sourceButtonKeyNumlock.SetProperty(WSLit("name"), WSLit("Keyboard Numlock"));
		sourceButtonKeyNumlock.SetProperty(WSLit("keyCodeDown"), JSValue(144));
		sourceButtons.Push(sourceButtonKeyNumlock);

		JSObject sourceButtonKeyEscape;
		sourceButtonKeyEscape.SetProperty(WSLit("id"), WSLit("KEY_ESCAPE"));
		sourceButtonKeyEscape.SetProperty(WSLit("name"), WSLit("Keyboard Escape"));
		sourceButtonKeyEscape.SetProperty(WSLit("keyCodeDown"), JSValue(27));
		sourceButtons.Push(sourceButtonKeyEscape);

		JSObject sourceButtonKeyScrolllock;
		sourceButtonKeyScrolllock.SetProperty(WSLit("id"), WSLit("KEY_SCROLLLOCK"));
		sourceButtonKeyScrolllock.SetProperty(WSLit("name"), WSLit("Keyboard Scroll Lock"));
		sourceButtonKeyScrolllock.SetProperty(WSLit("keyCodeDown"), JSValue(145));
		sourceButtons.Push(sourceButtonKeyScrolllock);

		JSObject sourceButtonKeyInsert;
		sourceButtonKeyInsert.SetProperty(WSLit("id"), WSLit("KEY_INSERT"));
		sourceButtonKeyInsert.SetProperty(WSLit("name"), WSLit("Keyboard Insert"));
		sourceButtonKeyInsert.SetProperty(WSLit("keyCodeDown"), JSValue(45));
		sourceButtons.Push(sourceButtonKeyInsert);

		JSObject sourceButtonKeyDelete;
		sourceButtonKeyDelete.SetProperty(WSLit("id"), WSLit("KEY_DELETE"));
		sourceButtonKeyDelete.SetProperty(WSLit("name"), WSLit("Keyboard Delete"));
		sourceButtonKeyDelete.SetProperty(WSLit("keyCodeDown"), JSValue(46));
		sourceButtons.Push(sourceButtonKeyDelete);

		JSObject sourceButtonKeyHome;
		sourceButtonKeyHome.SetProperty(WSLit("id"), WSLit("KEY_HOME"));
		sourceButtonKeyHome.SetProperty(WSLit("name"), WSLit("Keyboard Home"));
		sourceButtonKeyHome.SetProperty(WSLit("keyCodeDown"), JSValue(36));
		sourceButtons.Push(sourceButtonKeyHome);

		JSObject sourceButtonKeyEnd;
		sourceButtonKeyEnd.SetProperty(WSLit("id"), WSLit("KEY_END"));
		sourceButtonKeyEnd.SetProperty(WSLit("name"), WSLit("Keyboard End"));
		sourceButtonKeyEnd.SetProperty(WSLit("keyCodeDown"), JSValue(35));
		sourceButtons.Push(sourceButtonKeyEnd);

		JSObject sourceButtonKeyPageup;
		sourceButtonKeyPageup.SetProperty(WSLit("id"), WSLit("KEY_PAGEUP"));
		sourceButtonKeyPageup.SetProperty(WSLit("name"), WSLit("Keyboard PageUp"));
		sourceButtonKeyPageup.SetProperty(WSLit("keyCodeDown"), JSValue(33));
		sourceButtons.Push(sourceButtonKeyPageup);

		JSObject sourceButtonKeyPagedown;
		sourceButtonKeyPagedown.SetProperty(WSLit("id"), WSLit("KEY_PAGEDOWN"));
		sourceButtonKeyPagedown.SetProperty(WSLit("name"), WSLit("Keyboard PageDown"));
		sourceButtonKeyPagedown.SetProperty(WSLit("keyCodeDown"), JSValue(34));
		sourceButtons.Push(sourceButtonKeyPagedown);

		JSObject sourceButtonKeyBreak;
		sourceButtonKeyBreak.SetProperty(WSLit("id"), WSLit("KEY_BREAK"));
		sourceButtonKeyBreak.SetProperty(WSLit("name"), WSLit("Keyboard Break"));
		sourceButtonKeyBreak.SetProperty(WSLit("keyCodeDown"), JSValue(19));
		sourceButtons.Push(sourceButtonKeyBreak);

		JSObject sourceButtonKeyLShift;
		sourceButtonKeyLShift.SetProperty(WSLit("id"), WSLit("KEY_LSHIFT"));
		sourceButtonKeyLShift.SetProperty(WSLit("name"), WSLit("Keyboard L Shift"));
		sourceButtonKeyLShift.SetProperty(WSLit("keyCodeDown"), JSValue(16));
		sourceButtons.Push(sourceButtonKeyLShift);

		JSObject sourceButtonKeyRShift;
		sourceButtonKeyRShift.SetProperty(WSLit("id"), WSLit("KEY_RSHIFT"));
		sourceButtonKeyRShift.SetProperty(WSLit("name"), WSLit("Keyboard R Shift"));
		sourceButtonKeyRShift.SetProperty(WSLit("keyCodeDown"), JSValue(16));
		sourceButtons.Push(sourceButtonKeyRShift);

		JSObject sourceButtonKeyLAlt;
		sourceButtonKeyLAlt.SetProperty(WSLit("id"), WSLit("KEY_LALT"));
		sourceButtonKeyLAlt.SetProperty(WSLit("name"), WSLit("Keyboard L Alt"));
		sourceButtonKeyLAlt.SetProperty(WSLit("keyCodeDown"), JSValue(18));
		sourceButtons.Push(sourceButtonKeyLAlt);

		JSObject sourceButtonKeyRAlt;
		sourceButtonKeyRAlt.SetProperty(WSLit("id"), WSLit("KEY_RALT"));
		sourceButtonKeyRAlt.SetProperty(WSLit("name"), WSLit("Keyboard R Alt"));
		sourceButtonKeyRAlt.SetProperty(WSLit("keyCodeDown"), JSValue(18));
		sourceButtons.Push(sourceButtonKeyRAlt);

		JSObject sourceButtonKeyLControl;
		sourceButtonKeyLControl.SetProperty(WSLit("id"), WSLit("KEY_LCONTROL"));
		sourceButtonKeyLControl.SetProperty(WSLit("name"), WSLit("Keyboard L Ctrl"));
		sourceButtonKeyLControl.SetProperty(WSLit("keyCodeDown"), JSValue(17));
		sourceButtons.Push(sourceButtonKeyLControl);

		JSObject sourceButtonKeyRControl;
		sourceButtonKeyRControl.SetProperty(WSLit("id"), WSLit("KEY_RCONTROL"));
		sourceButtonKeyRControl.SetProperty(WSLit("name"), WSLit("Keyboard R Ctrl"));
		sourceButtonKeyRControl.SetProperty(WSLit("keyCodeDown"), JSValue(17));
		sourceButtons.Push(sourceButtonKeyRControl);

		JSObject sourceButtonKeyLWin;
		sourceButtonKeyLWin.SetProperty(WSLit("id"), WSLit("KEY_LWIN"));
		sourceButtonKeyLWin.SetProperty(WSLit("name"), WSLit("Keyboard L Win"));
		sourceButtonKeyLWin.SetProperty(WSLit("keyCodeDown"), JSValue(91));
		sourceButtons.Push(sourceButtonKeyLWin);

		JSObject sourceButtonKeyRWin;
		sourceButtonKeyRWin.SetProperty(WSLit("id"), WSLit("KEY_RWIN"));
		sourceButtonKeyRWin.SetProperty(WSLit("name"), WSLit("Keyboard R Win"));
		sourceButtonKeyRWin.SetProperty(WSLit("keyCodeDown"), JSValue(91));
		sourceButtons.Push(sourceButtonKeyRWin);

		JSObject sourceButtonKeyApp;
		sourceButtonKeyApp.SetProperty(WSLit("id"), WSLit("KEY_APP"));
		sourceButtonKeyApp.SetProperty(WSLit("name"), WSLit("Keyboard App"));
		sourceButtonKeyApp.SetProperty(WSLit("keyCodeDown"), JSValue(93));
		sourceButtons.Push(sourceButtonKeyApp);

		JSObject sourceButtonKeyUp;
		sourceButtonKeyUp.SetProperty(WSLit("id"), WSLit("KEY_UP"));
		sourceButtonKeyUp.SetProperty(WSLit("name"), WSLit("Keyboard Up"));
		sourceButtonKeyUp.SetProperty(WSLit("keyCodeDown"), JSValue(38));
		sourceButtons.Push(sourceButtonKeyUp);

		JSObject sourceButtonKeyLeft;
		sourceButtonKeyLeft.SetProperty(WSLit("id"), WSLit("KEY_LEFT"));
		sourceButtonKeyLeft.SetProperty(WSLit("name"), WSLit("Keyboard Left"));
		sourceButtonKeyLeft.SetProperty(WSLit("keyCodeDown"), JSValue(37));
		sourceButtons.Push(sourceButtonKeyLeft);

		JSObject sourceButtonKeyDown;
		sourceButtonKeyDown.SetProperty(WSLit("id"), WSLit("KEY_DOWN"));
		sourceButtonKeyDown.SetProperty(WSLit("name"), WSLit("Keyboard Down"));
		sourceButtonKeyDown.SetProperty(WSLit("keyCodeDown"), JSValue(40));
		sourceButtons.Push(sourceButtonKeyDown);

		JSObject sourceButtonKeyRight;
		sourceButtonKeyRight.SetProperty(WSLit("id"), WSLit("KEY_RIGHT"));
		sourceButtonKeyRight.SetProperty(WSLit("name"), WSLit("Keyboard Right"));
		sourceButtonKeyRight.SetProperty(WSLit("keyCodeDown"), JSValue(39));
		sourceButtons.Push(sourceButtonKeyRight);

		JSObject sourceButtonKeyF1;
		sourceButtonKeyF1.SetProperty(WSLit("id"), WSLit("KEY_F1"));
		sourceButtonKeyF1.SetProperty(WSLit("name"), WSLit("Keyboard F1"));
		sourceButtonKeyF1.SetProperty(WSLit("keyCodeDown"), JSValue(112));
		sourceButtons.Push(sourceButtonKeyF1);

		JSObject sourceButtonKeyF2;
		sourceButtonKeyF2.SetProperty(WSLit("id"), WSLit("KEY_F2"));
		sourceButtonKeyF2.SetProperty(WSLit("name"), WSLit("Keyboard F2"));
		sourceButtonKeyF2.SetProperty(WSLit("keyCodeDown"), JSValue(113));
		sourceButtons.Push(sourceButtonKeyF2);

		JSObject sourceButtonKeyF3;
		sourceButtonKeyF3.SetProperty(WSLit("id"), WSLit("KEY_F3"));
		sourceButtonKeyF3.SetProperty(WSLit("name"), WSLit("Keyboard F3"));
		sourceButtonKeyF3.SetProperty(WSLit("keyCodeDown"), JSValue(114));
		sourceButtons.Push(sourceButtonKeyF3);

		JSObject sourceButtonKeyF4;
		sourceButtonKeyF4.SetProperty(WSLit("id"), WSLit("KEY_F4"));
		sourceButtonKeyF4.SetProperty(WSLit("name"), WSLit("Keyboard F4"));
		sourceButtonKeyF4.SetProperty(WSLit("keyCodeDown"), JSValue(115));
		sourceButtons.Push(sourceButtonKeyF4);

		JSObject sourceButtonKeyF5;
		sourceButtonKeyF5.SetProperty(WSLit("id"), WSLit("KEY_F5"));
		sourceButtonKeyF5.SetProperty(WSLit("name"), WSLit("Keyboard F5"));
		sourceButtonKeyF5.SetProperty(WSLit("keyCodeDown"), JSValue(116));
		sourceButtons.Push(sourceButtonKeyF5);

		JSObject sourceButtonKeyF6;
		sourceButtonKeyF6.SetProperty(WSLit("id"), WSLit("KEY_F6"));
		sourceButtonKeyF6.SetProperty(WSLit("name"), WSLit("Keyboard F6"));
		sourceButtonKeyF6.SetProperty(WSLit("keyCodeDown"), JSValue(117));
		sourceButtons.Push(sourceButtonKeyF6);

		JSObject sourceButtonKeyF7;
		sourceButtonKeyF7.SetProperty(WSLit("id"), WSLit("KEY_F7"));
		sourceButtonKeyF7.SetProperty(WSLit("name"), WSLit("Keyboard F7"));
		sourceButtonKeyF7.SetProperty(WSLit("keyCodeDown"), JSValue(118));
		sourceButtons.Push(sourceButtonKeyF7);

		JSObject sourceButtonKeyF8;
		sourceButtonKeyF8.SetProperty(WSLit("id"), WSLit("KEY_F8"));
		sourceButtonKeyF8.SetProperty(WSLit("name"), WSLit("Keyboard F8"));
		sourceButtonKeyF8.SetProperty(WSLit("keyCodeDown"), JSValue(119));
		sourceButtons.Push(sourceButtonKeyF8);

		JSObject sourceButtonKeyF9;
		sourceButtonKeyF9.SetProperty(WSLit("id"), WSLit("KEY_F9"));
		sourceButtonKeyF9.SetProperty(WSLit("name"), WSLit("Keyboard F9"));
		sourceButtonKeyF9.SetProperty(WSLit("keyCodeDown"), JSValue(120));
		sourceButtons.Push(sourceButtonKeyF9);

		JSObject sourceButtonKeyF10;
		sourceButtonKeyF10.SetProperty(WSLit("id"), WSLit("KEY_F10"));
		sourceButtonKeyF10.SetProperty(WSLit("name"), WSLit("Keyboard F10"));
		sourceButtonKeyF10.SetProperty(WSLit("keyCodeDown"), JSValue(121));
		sourceButtons.Push(sourceButtonKeyF10);

		JSObject sourceButtonKeyF11;
		sourceButtonKeyF11.SetProperty(WSLit("id"), WSLit("KEY_F11"));
		sourceButtonKeyF11.SetProperty(WSLit("name"), WSLit("Keyboard F11"));
		sourceButtonKeyF11.SetProperty(WSLit("keyCodeDown"), JSValue(122));
		sourceButtons.Push(sourceButtonKeyF11);

		JSObject sourceButtonKeyF12;
		sourceButtonKeyF12.SetProperty(WSLit("id"), WSLit("KEY_F12"));
		sourceButtonKeyF12.SetProperty(WSLit("name"), WSLit("Keyboard F12"));
		sourceButtonKeyF12.SetProperty(WSLit("keyCodeDown"), JSValue(123));
		sourceButtons.Push(sourceButtonKeyF12);

		/*
			KEY_CAPSLOCKTOGGLE,
			KEY_NUMLOCKTOGGLE,
			KEY_SCROLLLOCKTOGGLE,
		*/

		JSObject sourceButtonMouseLeft;
		sourceButtonMouseLeft.SetProperty(WSLit("id"), WSLit("MOUSE_LEFT"));
		sourceButtonMouseLeft.SetProperty(WSLit("name"), WSLit("Mouse Button 1"));
		sourceButtonMouseLeft.SetProperty(WSLit("keyCodeDown"), WSLit("MOUSE1"));
		sourceButtons.Push(sourceButtonMouseLeft);

		JSObject sourceButtonMouseRight;
		sourceButtonMouseRight.SetProperty(WSLit("id"), WSLit("MOUSE_RIGHT"));
		sourceButtonMouseRight.SetProperty(WSLit("name"), WSLit("Mouse Button 2"));
		sourceButtonMouseRight.SetProperty(WSLit("keyCodeDown"), WSLit("MOUSE2"));
		sourceButtons.Push(sourceButtonMouseRight);

		JSObject sourceButtonMouseMiddle;
		sourceButtonMouseMiddle.SetProperty(WSLit("id"), WSLit("MOUSE_MIDDLE"));
		sourceButtonMouseMiddle.SetProperty(WSLit("name"), WSLit("Mouse Button 3"));
		sourceButtonMouseMiddle.SetProperty(WSLit("keyCodeDown"), WSLit("MOUSE3"));
		sourceButtons.Push(sourceButtonMouseMiddle);

		JSObject sourceButtonMouse4;
		sourceButtonMouse4.SetProperty(WSLit("id"), WSLit("MOUSE_4"));
		sourceButtonMouse4.SetProperty(WSLit("name"), WSLit("Mouse Button 4"));
		sourceButtonMouse4.SetProperty(WSLit("keyCodeDown"), WSLit("MOUSE4"));
		sourceButtons.Push(sourceButtonMouse4);

		JSObject sourceButtonMouse5;
		sourceButtonMouse5.SetProperty(WSLit("id"), WSLit("MOUSE_5"));
		sourceButtonMouse5.SetProperty(WSLit("name"), WSLit("Mouse Button 5"));
		sourceButtonMouse5.SetProperty(WSLit("keyCodeDown"), WSLit("MOUSE5"));
		sourceButtons.Push(sourceButtonMouse5);

		JSObject sourceButtonMouseWheelup;
		sourceButtonMouseWheelup.SetProperty(WSLit("id"), WSLit("MOUSE_WHEEL_UP"));
		sourceButtonMouseWheelup.SetProperty(WSLit("name"), WSLit("Mouse Wheel Up"));
		sourceButtonMouseWheelup.SetProperty(WSLit("keyCodeDown"), WSLit("MOUSEWHEELUP"));
		sourceButtons.Push(sourceButtonMouseWheelup);

		JSObject sourceButtonMouseWheeldown;
		sourceButtonMouseWheeldown.SetProperty(WSLit("id"), WSLit("MOUSE_WHEEL_DOWN"));
		sourceButtonMouseWheeldown.SetProperty(WSLit("name"), WSLit("Mouse Wheel Down"));
		sourceButtonMouseWheeldown.SetProperty(WSLit("keyCodeDown"), WSLit("MOUSEWHEELDOWN"));
		sourceButtons.Push(sourceButtonMouseWheeldown);

		JSObject sourceButtonKeyXButtonUp;
		sourceButtonKeyXButtonUp.SetProperty(WSLit("id"), WSLit("KEY_XBUTTON_UP"));
		sourceButtonKeyXButtonUp.SetProperty(WSLit("name"), WSLit("Joy Dpad Up"));
		sourceButtonKeyXButtonUp.SetProperty(WSLit("keyCodeDown"), WSLit("JOYDUP"));
		sourceButtons.Push(sourceButtonKeyXButtonUp);

		JSObject sourceButtonKeyXButtonRight;
		sourceButtonKeyXButtonRight.SetProperty(WSLit("id"), WSLit("KEY_XBUTTON_RIGHT"));
		sourceButtonKeyXButtonRight.SetProperty(WSLit("name"), WSLit("Joy Dpad Right"));
		sourceButtonKeyXButtonRight.SetProperty(WSLit("keyCodeDown"), WSLit("JOYDRIGHT"));
		sourceButtons.Push(sourceButtonKeyXButtonRight);

		JSObject sourceButtonKeyXButtonDown;
		sourceButtonKeyXButtonDown.SetProperty(WSLit("id"), WSLit("KEY_XBUTTON_DOWN"));
		sourceButtonKeyXButtonDown.SetProperty(WSLit("name"), WSLit("Joy Dpad Down"));
		sourceButtonKeyXButtonDown.SetProperty(WSLit("keyCodeDown"), WSLit("JOYDDOWN"));
		sourceButtons.Push(sourceButtonKeyXButtonDown);

		JSObject sourceButtonKeyXButtonLeft;
		sourceButtonKeyXButtonLeft.SetProperty(WSLit("id"), WSLit("KEY_XBUTTON_LEFT"));
		sourceButtonKeyXButtonLeft.SetProperty(WSLit("name"), WSLit("Joy Dpad Left"));
		sourceButtonKeyXButtonLeft.SetProperty(WSLit("keyCodeDown"), WSLit("JOYDLEFT"));
		sourceButtons.Push(sourceButtonKeyXButtonLeft);

		JSObject sourceButtonKeyXButtonA;
		sourceButtonKeyXButtonA.SetProperty(WSLit("id"), WSLit("KEY_XBUTTON_A"));
		sourceButtonKeyXButtonA.SetProperty(WSLit("name"), WSLit("Joy A"));
		sourceButtonKeyXButtonA.SetProperty(WSLit("keyCodeDown"), WSLit("JOYA"));
		sourceButtons.Push(sourceButtonKeyXButtonA);

		JSObject sourceButtonKeyXButtonB;
		sourceButtonKeyXButtonB.SetProperty(WSLit("id"), WSLit("KEY_XBUTTON_B"));
		sourceButtonKeyXButtonB.SetProperty(WSLit("name"), WSLit("Joy B"));
		sourceButtonKeyXButtonB.SetProperty(WSLit("keyCodeDown"), WSLit("JOYB"));
		sourceButtons.Push(sourceButtonKeyXButtonB);

		JSObject sourceButtonKeyXButtonX;
		sourceButtonKeyXButtonX.SetProperty(WSLit("id"), WSLit("KEY_XBUTTON_X"));
		sourceButtonKeyXButtonX.SetProperty(WSLit("name"), WSLit("Joy X"));
		sourceButtonKeyXButtonX.SetProperty(WSLit("keyCodeDown"), WSLit("JOYX"));
		sourceButtons.Push(sourceButtonKeyXButtonX);

		JSObject sourceButtonKeyXButtonY;
		sourceButtonKeyXButtonY.SetProperty(WSLit("id"), WSLit("KEY_XBUTTON_Y"));
		sourceButtonKeyXButtonY.SetProperty(WSLit("name"), WSLit("Joy Y"));
		sourceButtonKeyXButtonY.SetProperty(WSLit("keyCodeDown"), WSLit("JOYY"));
		sourceButtons.Push(sourceButtonKeyXButtonY);

		JSObject sourceButtonKeyXButtonLeftShoulder;
		sourceButtonKeyXButtonLeftShoulder.SetProperty(WSLit("id"), WSLit("KEY_XBUTTON_LEFT_SHOULDER"));
		sourceButtonKeyXButtonLeftShoulder.SetProperty(WSLit("name"), WSLit("Joy LT"));
		sourceButtonKeyXButtonLeftShoulder.SetProperty(WSLit("keyCodeDown"), WSLit("JOYLT"));
		sourceButtons.Push(sourceButtonKeyXButtonLeftShoulder);

		JSObject sourceButtonKeyXButtonRightShoulder;
		sourceButtonKeyXButtonRightShoulder.SetProperty(WSLit("id"), WSLit("KEY_XBUTTON_RIGHT_SHOULDER"));
		sourceButtonKeyXButtonRightShoulder.SetProperty(WSLit("name"), WSLit("Joy RT"));
		sourceButtonKeyXButtonRightShoulder.SetProperty(WSLit("keyCodeDown"), WSLit("JOYRT"));
		sourceButtons.Push(sourceButtonKeyXButtonRightShoulder);

		JSObject sourceButtonKeyXButtonBack;
		sourceButtonKeyXButtonBack.SetProperty(WSLit("id"), WSLit("KEY_XBUTTON_BACK"));
		sourceButtonKeyXButtonBack.SetProperty(WSLit("name"), WSLit("Joy Back"));
		sourceButtonKeyXButtonBack.SetProperty(WSLit("keyCodeDown"), WSLit("JOYBACK"));
		sourceButtons.Push(sourceButtonKeyXButtonBack);

		JSObject sourceButtonKeyXButtonStart;
		sourceButtonKeyXButtonStart.SetProperty(WSLit("id"), WSLit("KEY_XBUTTON_START"));
		sourceButtonKeyXButtonStart.SetProperty(WSLit("name"), WSLit("Joy Start"));
		sourceButtonKeyXButtonStart.SetProperty(WSLit("keyCodeDown"), WSLit("JOYSTART"));
		sourceButtons.Push(sourceButtonKeyXButtonStart);

		JSObject sourceButtonKeyXButtonStick1;
		sourceButtonKeyXButtonStick1.SetProperty(WSLit("id"), WSLit("KEY_XBUTTON_STICK1"));
		sourceButtonKeyXButtonStick1.SetProperty(WSLit("name"), WSLit("Joy L Thumbstick Button"));
		sourceButtonKeyXButtonStick1.SetProperty(WSLit("keyCodeDown"), WSLit("JOYLTHUMB"));
		sourceButtons.Push(sourceButtonKeyXButtonStick1);

		JSObject sourceButtonKeyXButtonStick2;
		sourceButtonKeyXButtonStick2.SetProperty(WSLit("id"), WSLit("KEY_XBUTTON_STICK2"));
		sourceButtonKeyXButtonStick2.SetProperty(WSLit("name"), WSLit("Joy R Thumbstick Button"));
		sourceButtonKeyXButtonStick2.SetProperty(WSLit("keyCodeDown"), WSLit("JOYRTHUMB"));
		sourceButtons.Push(sourceButtonKeyXButtonStick2);

		JSObject sourceButtonKeyXStick1Right;
		sourceButtonKeyXStick1Right.SetProperty(WSLit("id"), WSLit("KEY_XSTICK1_RIGHT"));
		sourceButtonKeyXStick1Right.SetProperty(WSLit("name"), WSLit("Joy L Thumbstick Right"));
		sourceButtonKeyXStick1Right.SetProperty(WSLit("keyCodeDown"), WSLit("JOYLTHUMBRIGHT"));
		sourceButtons.Push(sourceButtonKeyXStick1Right);

		JSObject sourceButtonKeyXStick1Left;
		sourceButtonKeyXStick1Left.SetProperty(WSLit("id"), WSLit("KEY_XSTICK1_LEFT"));
		sourceButtonKeyXStick1Left.SetProperty(WSLit("name"), WSLit("Joy L Thumbstick Left"));
		sourceButtonKeyXStick1Left.SetProperty(WSLit("keyCodeDown"), WSLit("JOYLTHUMBLEFT"));
		sourceButtons.Push(sourceButtonKeyXStick1Left);

		JSObject sourceButtonKeyXStick1Down;
		sourceButtonKeyXStick1Down.SetProperty(WSLit("id"), WSLit("KEY_XSTICK1_DOWN"));
		sourceButtonKeyXStick1Down.SetProperty(WSLit("name"), WSLit("Joy L Thumbstick Down"));
		sourceButtonKeyXStick1Down.SetProperty(WSLit("keyCodeDown"), WSLit("JOYLTHUMBDOWN"));
		sourceButtons.Push(sourceButtonKeyXStick1Down);

		JSObject sourceButtonKeyXStick1Up;
		sourceButtonKeyXStick1Up.SetProperty(WSLit("id"), WSLit("KEY_XSTICK1_UP"));
		sourceButtonKeyXStick1Up.SetProperty(WSLit("name"), WSLit("Joy L Thumbstick Up"));
		sourceButtonKeyXStick1Up.SetProperty(WSLit("keyCodeDown"), WSLit("JOYLTHUMBUP"));
		sourceButtons.Push(sourceButtonKeyXStick1Up);

		JSObject sourceButtonKeyXButtonLTrigger;
		sourceButtonKeyXButtonLTrigger.SetProperty(WSLit("id"), WSLit("KEY_XBUTTON_LTRIGGER"));
		sourceButtonKeyXButtonLTrigger.SetProperty(WSLit("name"), WSLit("Joy L Trigger"));
		sourceButtonKeyXButtonLTrigger.SetProperty(WSLit("keyCodeDown"), WSLit("JOYLT"));
		sourceButtons.Push(sourceButtonKeyXButtonLTrigger);

		JSObject sourceButtonKeyXButtonRTrigger;
		sourceButtonKeyXButtonRTrigger.SetProperty(WSLit("id"), WSLit("KEY_XBUTTON_RTRIGGER"));
		sourceButtonKeyXButtonRTrigger.SetProperty(WSLit("name"), WSLit("Joy R Trigger"));
		sourceButtonKeyXButtonRTrigger.SetProperty(WSLit("keyCodeDown"), WSLit("JOYRT"));
		sourceButtons.Push(sourceButtonKeyXButtonRTrigger);

		JSObject sourceButtonKeyXStick2Right;
		sourceButtonKeyXStick2Right.SetProperty(WSLit("id"), WSLit("KEY_XSTICK2_RIGHT"));
		sourceButtonKeyXStick2Right.SetProperty(WSLit("name"), WSLit("Joy R Thumbstick Right"));
		sourceButtonKeyXStick2Right.SetProperty(WSLit("keyCodeDown"), WSLit("JOYRTHUMBRIGHT"));
		sourceButtons.Push(sourceButtonKeyXStick2Right);

		JSObject sourceButtonKeyXStick2Left;
		sourceButtonKeyXStick2Left.SetProperty(WSLit("id"), WSLit("KEY_XSTICK2_LEFT"));
		sourceButtonKeyXStick2Left.SetProperty(WSLit("name"), WSLit("Joy R Thumbstick Left"));
		sourceButtonKeyXStick2Left.SetProperty(WSLit("keyCodeDown"), WSLit("JOYRTHUMBLEFT"));
		sourceButtons.Push(sourceButtonKeyXStick2Left);

		JSObject sourceButtonKeyXStick2Down;
		sourceButtonKeyXStick2Down.SetProperty(WSLit("id"), WSLit("KEY_XSTICK2_DOWN"));
		sourceButtonKeyXStick2Down.SetProperty(WSLit("name"), WSLit("Joy R Thumbstick Down"));
		sourceButtonKeyXStick2Down.SetProperty(WSLit("keyCodeDown"), WSLit("JOYRTHUMBDOWN"));
		sourceButtons.Push(sourceButtonKeyXStick2Down);

		JSObject sourceButtonKeyXStick2Up;
		sourceButtonKeyXStick2Up.SetProperty(WSLit("id"), WSLit("KEY_XSTICK2_UP"));
		sourceButtonKeyXStick2Up.SetProperty(WSLit("name"), WSLit("Joy R Thumbstick Up"));
		sourceButtonKeyXStick2Up.SetProperty(WSLit("keyCodeDown"), WSLit("JOYRTHUMBUP"));
		sourceButtons.Push(sourceButtonKeyXStick2Up);

		response.SetProperty(WSLit("sourceButtons"), sourceButtons);

		return response;
	}
	else if (method_name == WSLit("importSteamGames"))
	{
		JSArray games = args[0].ToArray();
		unsigned int numGames = games.size();
		KeyValues* kv = new KeyValues("steamgames");
		std::string appid;
		std::string name;
		for (unsigned int i = 0; i < numGames; i += 2)
		{
			name = WebStringToCharString(games.At(i).ToString());
			appid = "id" + std::string(WebStringToCharString(games.At(i + 1).ToString()));
			//DevMsg("Here it's %s and %s\n", appid.c_str(), name.c_str());
			kv->SetString(appid.c_str(), name.c_str());
		}

		//g_pAnarchyManager->GetMetaverseManager()->ImportSteamGame(WebStringToCharString(games.At(i).ToString()), WebStringToCharString(games.At(i+1).ToString()));
		g_pAnarchyManager->GetMetaverseManager()->ImportSteamGames(kv);

		return JSValue(true);
	}
	else if (method_name == WSLit("getLibretroKeybinds"))
	{
		std::string type = WebStringToCharString(args[0].ToString());
		int portNumber = (args.size() > 1) ? args[1].ToInteger() : -1;
		int deviceNumber = (args.size() > 2) ? args[2].ToInteger() : -1;
		int indexNumber = (args.size() > 3) ? args[3].ToInteger() : -1;

		JSObject response;

		// get the active libretro instance
		C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->GetSelectedLibretroInstance();
		if (pLibretroInstance && pLibretroInstance->GetInfo() && pLibretroInstance->GetInfo()->state == 5)
		{
			std::string keyPath;
			std::string shortPath;
			LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();
			unsigned int numPorts = info->numports + 1;
			if (numPorts == 1)
				numPorts = 2;

			// PORT# / DEVICE# / INDEX# / KEY# = VALUE

			JSObject keybinds;
			for (unsigned int i = 0; i < numPorts; i++)
			{
				if (portNumber >= 0 && i != portNumber)
					continue;

				JSObject port;
				unsigned int numDevices = 7;
				for (unsigned int j = 0; j < numDevices; j++)
				{
					if (deviceNumber >= 0 && j != deviceNumber)
						continue;

					JSObject device;
					unsigned int maxIndex = 1;
					for (unsigned int k = 0; k < maxIndex; k++)
					{
						if (indexNumber >= 0 && k != indexNumber)
							continue;

						JSObject index;

						// cycle through ACTIVE keybinds
						KeyValues* kv;
						if (type == "game")
							kv = info->gamekeybinds;
						else if (type == "core")
							kv = info->corekeybinds;
						else if (type == "libretro")
							kv = info->libretrokeybinds;

						keyPath = "port" + std::to_string(i) + "/device" + std::to_string(j) + "/index" + std::to_string(k);//VarArgs("port%u/device%u/index%u", i, j, k)
						kv = kv->FindKey(keyPath.c_str());

						if (kv)
						{
							// cycle through the given KV and find any currently bound buttons
							for (KeyValues *sub = kv->GetFirstSubKey(); sub; sub = sub->GetNextKey())
								index.SetProperty(WSLit(sub->GetName()), WSLit(sub->GetString()));
						}

						shortPath = "index" + std::to_string(k);
						device.SetProperty(WSLit(shortPath.c_str()), index);
					}

					shortPath = "device" + std::to_string(j);
					port.SetProperty(WSLit(shortPath.c_str()), device);
				}

				shortPath = "port" + std::to_string(i);
				keybinds.SetProperty(WSLit(shortPath.c_str()), port);
			}

			response.SetProperty(WSLit("keybinds"), keybinds);
		}

		return response;
	}
	else if (method_name == WSLit("getInstance"))
	{
		std::string instanceId = (args.size() > 0 ) ? WebStringToCharString(args[0].ToString()) : "";
		if (instanceId == "")
			instanceId = g_pAnarchyManager->GetInstanceId();

		JSObject response;

		instance_t* pInstance = g_pAnarchyManager->GetInstanceManager()->FindInstance(instanceId);
		if (pInstance)
		{
			JSObject instanceObject;
			instanceObject.SetProperty(WSLit("id"), WSLit(pInstance->id.c_str()));
			instanceObject.SetProperty(WSLit("mapId"), WSLit(pInstance->mapId.c_str()));
			instanceObject.SetProperty(WSLit("title"), WSLit(pInstance->title.c_str()));
			instanceObject.SetProperty(WSLit("file"), WSLit(pInstance->file.c_str()));
			instanceObject.SetProperty(WSLit("workshopIds"), WSLit(pInstance->workshopIds.c_str()));
			instanceObject.SetProperty(WSLit("mountIds"), WSLit(pInstance->mountIds.c_str()));

			response.SetProperty(WSLit("instance"), instanceObject);
		}

		return response;
	}
	else if (method_name == WSLit("getMapInstances"))
	{
		std::string mapId = WebStringToCharString(args[0].ToString());
		DevMsg("Map id: %s\n", mapId.c_str());
		//const char* mapId = WebStringToCharString(args[0].ToString());

		std::vector<instance_t*> instances;
		g_pAnarchyManager->GetInstanceManager()->FindAllInstances(mapId, instances);
		
		if (instances.size() == 0)
			return JSValue(null);

		//JSObject response;
		JSArray instancesArray;
		unsigned int numInstances = instances.size();
		unsigned int i;
		instance_t* pInstance;
		for (i = 0; i < numInstances; i++)
		{
			pInstance = instances[i];

			JSObject instanceObject;
			instanceObject.SetProperty(WSLit("id"), WSLit(pInstance->id.c_str()));
			instanceObject.SetProperty(WSLit("mapId"), WSLit(pInstance->mapId.c_str()));
			instanceObject.SetProperty(WSLit("title"), WSLit(pInstance->title.c_str()));
			instanceObject.SetProperty(WSLit("file"), WSLit(pInstance->file.c_str()));
			instanceObject.SetProperty(WSLit("workshopIds"), WSLit(pInstance->workshopIds.c_str()));
			instanceObject.SetProperty(WSLit("mountIds"), WSLit(pInstance->mountIds.c_str()));

			instancesArray.Push(instanceObject);
		}
		//response.SetProperty(WSLit("instances"), instancesArray);

		return instancesArray;
	}
	else
		return JSValue(null);
}
#include "cbase.h"

#include "c_awesomiumjshandlers.h"
#include "c_anarchymanager.h"
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
		g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);

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

		std::string modelId = g_pAnarchyManager->GenerateLegacyHash("models/cabinets/two_player_arcade.mdl");
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
	else if (method_name == WSLit("launchItem"))
	{
		/*
		// these items need improvements on how they are launched (they didn't work):
		Launching Item: 
			Executable: C:\Users\Owner\Desktop\launcher - Shortcut.lnk
			Directory: 
			Master Commands: 

			ALSO roms with full file locations as their file instead of just the short filename.
		*/
		DevMsg("LAUNCH THE SHIT!\n");

		// genereate the executable, executableDirectory, and masterCommands values.
		// then give these to the actual launch method.

		bool bUnknownError = false;

		// user resolvable errors to catch
		bool bItemGood = false;
		bool bItemFileGood = false;
		bool bAppGood = false;
		bool bAppExecutableGood = false;
		bool bAppFilepathGood = false;
		bool bAtLeastOneAppFilepathExists = false;
		bool bReadyToActuallyLaunch = false;

		// required fields for ArcadeCreateProcess
		std::string executable;
		std::string executableDirectory;
		std::string masterCommands;

		// other fields used to generate the required fields
		bool bHasApp = false;
		bool bHasAppFilepath = false;
		std::string id = WebStringToCharString(args[0].ToString());
		KeyValues* item = null;	// the KV of the item being used.
		KeyValues* itemActive = null;	// the active node of the item KV.
		std::string file;
		std::string composedFile;
		KeyValues* app = null;
		KeyValues* appActive = null;
		std::string appExecutable;
		std::string appFilepath;
		std::string appCommands;
		
		// attempt to get the item
		item = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(id);
		if (item)
		{
			// if there is an item, attempt to get the active node kv
			itemActive = item->FindKey("current");
			if (!itemActive)
				itemActive = item->FindKey("local", true);

			if (itemActive)
			{
				bItemGood = true;

				// if there is an active node kv for the item, attempt to get file
				file = itemActive->GetString("file");
				if (file != "")
				{
					// does it have an app?
					if (Q_strcmp(itemActive->GetString("app"), ""))
					{
						bHasApp = true;

						app = g_pAnarchyManager->GetMetaverseManager()->GetLibraryApp(itemActive->GetString("app"));
						appActive = app->FindKey("current");
						if (!appActive)
							appActive = app->FindKey("local", true);

						if (appActive)
						{
							bAppGood = true;

							// if there is an app, attempt to get its executable
							appExecutable = appActive->GetString("file");
							if (appExecutable != "")
							{
								bAppExecutableGood = true;

								// just grab the FIRST filepath for now.
								// FIXME: Need to keep searching through filepaths until the item's file is found inside of one.
								// Note: Apps are not required to have a filepath specified.
								std::string testFile;
								std::string testPath;
								KeyValues* filepaths = appActive->FindKey("filepaths", true);
								for (KeyValues *sub = filepaths->GetFirstSubKey(); sub; sub = sub->GetNextKey())
								{
									// true if even 1 filepath exists for the app, even if it is not found on the local PC.
									// (because in that case the local user probably needs to specify a correct location for it.)
									bHasAppFilepath = true;

									testPath = sub->GetString("path");

									// test if this path exists
									// FIXME: always assume it exists for now
									if (true)
									{
										bAtLeastOneAppFilepathExists = true;

										// test if the file exists inside of this filepath
										testFile = testPath + file;

										// FIXME: always assume the file exists in this path for now.
										if (true)
										{
											composedFile = testFile;
											appFilepath = testPath;
											bAppFilepathGood = true;
											bItemFileGood = true;
											break;
										}
									}
								}

								// resolve the composedFile now
								if (!bAppFilepathGood)
									composedFile = file;

								// generate the commands
								// try to apply a command format
								appCommands = appActive->GetString("commandformat");
								if (appCommands != "")
								{
									// if the app has a command syntax, replace item variables with their values.

									// replace $FILE with active->GetString("file")
									// replace $QUOTE with a double quote
									// replace $SHORTFILE with active->GetString("file")'s filename only
									// replace etc.

									size_t found = appCommands.find("$FILE");
									while (found != std::string::npos)
									{
										appCommands.replace(found, 5, composedFile);
										found = appCommands.find("$FILE");
									}

									found = appCommands.find("$QUOTE");
									while (found != std::string::npos)
									{
										appCommands.replace(found, 6, "\"");
										found = appCommands.find("$QUOTE");
									}
								}
								else
								{
									// otherwise, apply the default Windows command syntax for "open with"
									appCommands = "\"" + composedFile + "\"";
								}
							}
						}
					}

					if (!bAppExecutableGood)
						composedFile = file;

					if (!bAppFilepathGood)
					{
						// check if the file exists.
						// (file could also be an HTTP or STEAM protocol address at this point.)

						// are we a web address missing an http?
						if (composedFile.find("www.") == 0)
							composedFile = "http://" + composedFile;

						// is it a steam appid?
						if (!Q_strcmp(VarArgs("%llu", Q_atoui64(composedFile.c_str())), composedFile.c_str()))
						{
							composedFile = "steam://run/" + composedFile;
							bItemFileGood = true;
						}
						else if (composedFile.find("http") == 0 )
							bItemFileGood = true;
						else if (true)	// check if local file exists // FIXME: assume it always exists for now
							bItemFileGood = true;
					}
				}
			}
		}

		// all variables are resolved, now do some logic.
		if (bItemGood)
		{
			// check for a good app first, because that determines if the item file can be resolved.
			if (bHasApp)
			{
				if (bAppGood)
				{
					if (bAppExecutableGood)
					{
						if (bItemFileGood)
						{
							// executable
							executable = appExecutable;

							// executableDirectory
							std::string dir = appExecutable;
							size_t found = dir.find_last_of("/\\");
							if (found != std::string::npos)
								executableDirectory = dir.substr(0, found + 1);

							// masterCommands
							std::string shortAppFile = appExecutable;
							found = shortAppFile.find_last_of("/\\");
							if (found != std::string::npos)
								shortAppFile = shortAppFile.substr(found + 1);

							masterCommands = "\"" + shortAppFile + "\" " + appCommands;
							bReadyToActuallyLaunch = true;
						}
						else
						{
							DevMsg("USER-RESOLVABLE-LAUNCH-ERROR: Show it, bra.\n");
							return;
						}
					}
					else
					{
						DevMsg("USER-RESOLVABLE-LAUNCH-ERROR: Show it, bra.\n");
						return;
					}
				}
				else
				{
					DevMsg("USER-RESOLVABLE-LAUNCH-ERROR: Show it, bra.\n");
					return;
				}
			}
			else
			{
				if (bItemFileGood)
				{
					// doesn't use an app
					executable = composedFile;
					executableDirectory = "";
					masterCommands = "";
					bReadyToActuallyLaunch = true;
				}
				else
				{
					DevMsg("USER-RESOLVABLE-LAUNCH-ERROR: Show it, bra.\n");
					return;
				}
			}
		}
		else
		{
			DevMsg("USER-RESOLVABLE-LAUNCH-ERROR: Show it, bra.\n");
			return;
		}

		if (bReadyToActuallyLaunch)
		{
			// deslect any entity
			if (g_pAnarchyManager->GetSelectedEntity())
				g_pAnarchyManager->DeselectEntity("none");

			// clear the embedded instance (to stop YT videos from playing, for example)
			C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
			C_EmbeddedInstance* pOldEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
			if (pOldEmbeddedInstance && pOldEmbeddedInstance != pHudBrowserInstance)
				pOldEmbeddedInstance->Close();
			g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, null);

			// pause AArcade
			g_pAnarchyManager->Pause();

			// launch the item
			g_pAnarchyManager->ArcadeCreateProcess(executable, executableDirectory, masterCommands);
		}
		else
			DevMsg("ERROR: Could not launch item.\n");
	}
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
	else if (method_name == WSLit("loadLocalAppClose"))	// shouldn't really ever be called!!! (unless user abors the loading process)
		g_pAnarchyManager->GetMetaverseManager()->LoadLocalAppClose();
	/*
	else if (method_name == WSLit("mountNextWorkshopCallback"))
	{
		// FIXME: This should be done outside of the awesomeium-specific classes!!
		C_WorkshopManager* pWorkshopManager = g_pAnarchyManager->GetWorkshopManager();
		C_WebTab* pHudWebTab = g_pAnarchyManager->GetWebManager()->GetHudWebTab();

		bool result = pWorkshopManager->MountNextWorkshop();
		if (result)
			pHudWebTab->AddHudLoadingMessage("progress", "", "Mounting Workshop Subscriptions", "mountworkshops", "0", std::string(VarArgs("%u", pWorkshopManager->GetNumDetails())), "+", "mountNextWorkshopCallback");
		else
		{
			pWorkshopManager->MountWorkshopClose();
			g_pAnarchyManager->OnMountAllWorkshopsComplete();
		}
	}
	*/
	else if (method_name == WSLit("mountNextWorkshopCallback"))
	{
		// FIXME: This should be done outside of the awesomeium-specific classes!!
		C_WorkshopManager* pWorkshopManager = g_pAnarchyManager->GetWorkshopManager();
		//C_WebTab* pHudWebTab = g_pAnarchyManager->GetWebManager()->GetHudWebTab();

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

	//	C_WorkshopManager* pWorkshopManager = g_pAnarchyManager->GetWorkshopManager();
		//C_WebTab* pHudWebTab = g_pAnarchyManager->GetWebManager()->GetHudWebTab();

		//pWorkshopManager->MountNextWorkshop();
	}
	else if (method_name == WSLit("loadMap"))
	{

		std::string mapId = WebStringToCharString(args[0].ToString());
		std::string instanceId = WebStringToCharString(args[1].ToString());
		g_pAnarchyManager->SetNextInstanceId(instanceId);

		KeyValues* map = g_pAnarchyManager->GetMetaverseManager()->GetMap(mapId);
		KeyValues* active = map->FindKey("current");
		if (!active)
			active = map->FindKey("local", true);

		std::string mapName = active->GetString("platforms/-KJvcne3IKMZQTaG7lPo/file");
		mapName = mapName.substr(0, mapName.length() - 4);
		engine->ClientCmd(VarArgs("map \"%s\"\n", mapName.c_str()));
	}
	else if (method_name == WSLit("deactivateInputMode"))
	{
		g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);

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

		ButtonCode_t button;
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
		
		std::vector<std::string> params;
		params.push_back(VarArgs("%i", (g_pAnarchyManager->GetInputManager()->GetFullscreenMode())));
		params.push_back(VarArgs("%i", (g_pAnarchyManager->GetInputManager()->GetWasForceInputMode())));
		params.push_back(VarArgs("%i", engine->IsInGame()));
		params.push_back(VarArgs("%i", (g_pAnarchyManager->GetSelectedEntity() != null)));

		int isItemSelected = 0;
		C_BaseEntity* pEntity = g_pAnarchyManager->GetSelectedEntity();
		if (pEntity)
		{
			C_PropShortcutEntity* pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);
			if (pShortcut && pShortcut->GetItemId() != "")
				isItemSelected = 1;
		}
		params.push_back(VarArgs("%i", isItemSelected));
		params.push_back(VarArgs("%i", (g_pAnarchyManager->GetInputManager()->GetMainMenuMode())));

		C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
		C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
		if (pEmbeddedInstance && pEmbeddedInstance != pHudBrowserInstance)
			params.push_back(pEmbeddedInstance->GetURL());
		else
			params.push_back("");

		pHudBrowserInstance->DispatchJavaScriptMethod("arcadeHud", "onActivateInputMode", params);





		//g_pAnarchyManager->GetWebManager()->DispatchJavaScriptMethod(pWebTab, "arcadeHud", "onActivateInputMode", params);
	}
	else if (method_name == WSLit("autoInspect"))
	{
		// FIXME: THIS SHOULD JUST CALL A SUBROUTINE OF THE METAVERSE MANAGER!!

		std::string itemId = WebStringToCharString(args[0].ToString());
		KeyValues* item = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(itemId);
		if (item)
		{
			KeyValues* active = item->FindKey("current");
			if (!active)
				active = item->FindKey("local", true);

			std::string uri = "file://";
			uri += engine->GetGameDirectory();
			uri += "/resource/ui/html/autoInspectItem.html?id=" + g_pAnarchyManager->encodeURIComponent(itemId) + "&screen=" + g_pAnarchyManager->encodeURIComponent(active->GetString("screen")) + "&marquee=" + g_pAnarchyManager->encodeURIComponent(active->GetString("marquee")) + "&preview=" + g_pAnarchyManager->encodeURIComponent(active->GetString("preview")) + "&reference=" + g_pAnarchyManager->encodeURIComponent(active->GetString("reference")) + "&file=" + g_pAnarchyManager->encodeURIComponent(active->GetString("file"));

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
	else if (method_name == WSLit("cabinetSelected"))
	{
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
				g_pAnarchyManager->GetInstanceManager()->ChangeModel(pEntity, modelFile);
			}
		}

	}
	else if (method_name == WSLit("viewStream"))
	{
		std::string itemId = WebStringToCharString(args[0].ToString());
		KeyValues* item = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(itemId);
		if (item)
		{
			KeyValues* active = item->FindKey("current");
			if (!active)
				active = item->FindKey("local", true);

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
				if (pEmbeddedInstance)
				{
					oldId = pEmbeddedInstance->GetId();

					// close this instance
					DevMsg("Removing embedded Instance ID: %s\n", oldId.c_str());

					pEmbeddedInstance->Blur();
					pEmbeddedInstance->Deselect();
					g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);
					pEmbeddedInstance->Close();
				}

				pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->CreateSteamBrowserInstance();
				pBrowserInstance->Init(oldId, streamUri.c_str(), null);
			}
			else
				pBrowserInstance->SetUrl(streamUri);	// reuse the current focused steam browser if it exists

			pBrowserInstance->SetActiveScraper("", "", "");
			pBrowserInstance->Select();
			pBrowserInstance->Focus();

			g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance((C_EmbeddedInstance*)pBrowserInstance);
		}
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

		DevMsg("Injecting DOM getting code...\n");
		std::string code = "document.location = 'http://www.aarcadeapicall.com.net.org/?doc=";
		code += WebStringToCharString(args[0].ToString());
		code += "AAAPICALL' + encodeURIComponent(document.documentElement.innerHTML);";
	///*
		std::string mainCode = "if (document.readyState === 'complete'){";
		mainCode += code;
		mainCode += "}else{document.addEventListener('DOMContentLoaded', function() {";
		mainCode += code;
		mainCode += "});}";
		//*/

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

		KeyValues* pItem = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(itemId);

		KeyValues* active = pItem->FindKey("current");

		if (!active)
			active = pItem->FindKey("local", true);

		if (active)
		{
			bool bIsEntityInstance = false;

			C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
			C_SteamBrowserInstance* pBrowserInstance = dynamic_cast<C_SteamBrowserInstance*>(pEmbeddedInstance);
			if (!pBrowserInstance)
			{
				std::string oldId;
				if (pEmbeddedInstance)
				{
					oldId = pEmbeddedInstance->GetId();

					// close this instance
					DevMsg("Removing embedded Instance ID: %s\n", oldId.c_str());

					pEmbeddedInstance->Blur();
					pEmbeddedInstance->Deselect();
					g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);
					pEmbeddedInstance->Close();
				}

				pBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->CreateSteamBrowserInstance();
				pBrowserInstance->Init(oldId, query.c_str(), null);
			}
			else
				pBrowserInstance->SetUrl(query);	// reuse the current focused steam browser if it exists

			pBrowserInstance->SetActiveScraper(scraperId, itemId, field);
				
			pBrowserInstance->Select();
			pBrowserInstance->Focus();

			g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance((C_EmbeddedInstance*)pBrowserInstance);
		}
	}
	else if (method_name == WSLit("spawnNearestObject"))
	{
		C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");

		g_pAnarchyManager->GetInstanceManager()->SetNearestSpawnDist(args[0].ToDouble());
		bool bSpawned = g_pAnarchyManager->GetInstanceManager()->SpawnNearestObject();
		if ( bSpawned )
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Spawning Objects", "spawningobjects", "", "", "+", "spawnNextObjectCallback");
	}
	else if (method_name == WSLit("setNearestObjectDist"))
	{
		g_pAnarchyManager->GetInstanceManager()->SetNearestSpawnDist(args[0].ToDouble());
	}
	else if (method_name == WSLit("spawnNextObjectCallback"))
	{
		C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
		//C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();

		bool bSpawned = g_pAnarchyManager->GetInstanceManager()->SpawnNearestObject();
		if (bSpawned)
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Spawning Objects", "spawningobjects", "", "", "+", "spawnNextObjectCallback");
		else
		{
			//pEmbeddedInstance->SetUrl("asset://ui/blank.html");
			pHudBrowserInstance->SetUrl("asset://ui/blank.html");
			//g_pAnarchyManager->GetWebManager()->GetHudWebTab()->SetUrl("asset://ui/blank.html");
			g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
		}
	}
	else if (method_name == WSLit("fileBrowse"))
	{
		std::string browseId = WebStringToCharString(args[0].ToString());
		g_pAnarchyManager->ShowFileBrowseMenu(browseId);
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
				KeyValues* active = item->FindKey("current");
				if (!active)
					active = item->FindKey("local");

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

					int iWidth = 512;
					int iHeight = 512;

					ITexture* pTexture = null;
					//ITexture* pTexture = g_pMaterialSystem->CreateProceduralTexture(textureName.c_str(), TEXTURE_GROUP_MODEL, iWidth, iHeight, IMAGE_FORMAT_BGR888, 1);
					if (!g_pMaterialSystem->IsTextureLoaded(textureName.c_str()))
						pTexture = g_pMaterialSystem->CreateProceduralTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, iWidth, iHeight, IMAGE_FORMAT_BGR888, 1);
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

void AddSubKeys(KeyValues* kv, JSObject& object)
{
	for (KeyValues *sub = kv->GetFirstSubKey(); sub; sub = sub->GetNextKey())
	{
		if (sub->GetFirstSubKey())
		{
			JSObject subObject;
			AddSubKeys(sub, subObject);
			object.SetProperty(WSLit(sub->GetName()), subObject);
		}
		else
			object.SetProperty(WSLit(sub->GetName()), WSLit(sub->GetString()));
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

			active = pType->FindKey("current");
			if (!active)
				active = pType->FindKey("local", true);
			if (active)
			{
				JSObject typeObject;
				AddSubKeys(active, typeObject);
				types.SetProperty(WSLit(active->GetString("info/id")), typeObject);
			}

			pType = g_pAnarchyManager->GetMetaverseManager()->GetNextLibraryType();
		}
		return types;
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

			active = pApp->FindKey("current");
			if (!active)
				active = pApp->FindKey("local", true);
			if (active)
			{
				JSObject appObject;
				AddSubKeys(active, appObject);
				apps.SetProperty(WSLit(active->GetString("info/id")), appObject);
			}

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
			KeyValues* active = pType->FindKey("current");
			if (!active)
				active = pType->FindKey("local", true);
			if (active)
			{
				JSObject type;
				AddSubKeys(active, type);
				return type;
			}
			else
				return JSValue(0);
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
			KeyValues* active = pApp->FindKey("current");
			if (!active)
				active = pApp->FindKey("local", true);
			if (active)
			{
				JSObject app;
				AddSubKeys(active, app);
				return app;
			}
			else
				return JSValue(0);
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("getFirstLibraryItem"))
	{
		KeyValues* pItem = g_pAnarchyManager->GetMetaverseManager()->GetFirstLibraryItem();

		if (pItem)
		{
			KeyValues* active = pItem->FindKey("current");
			if (!active)
				active = pItem->FindKey("local", true);
			if (active)
			{
				JSObject item;
				AddSubKeys(active, item);
				return item;
			}
			else
				return JSValue(0);
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("getNextLibraryItem"))
	{
		KeyValues* pItem = g_pAnarchyManager->GetMetaverseManager()->GetNextLibraryItem();
		if (pItem)
		{
			KeyValues* active = pItem->FindKey("current");
			if (!active)
				active = pItem->FindKey("local", true);
			if (active)
			{
				JSObject item;
				AddSubKeys(active, item);
				return item;
			}
			else
				return JSValue(0);
		}
		else
			return JSValue(0);
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
						KeyValues* active = pItem->FindKey("current");
						if (!active)
							active = pItem->FindKey("local", true);
						if (active)
						{
							JSObject item;
							AddSubKeys(active, item);
							return item;
						}
					}
				}
			}
		}

		return JSValue(0);
	}
	else if (method_name == WSLit("getLibraryItem"))
	{
		std::string id = WebStringToCharString(args[0].ToString());

		KeyValues* pItem = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(id);
		if (pItem)
		{
			KeyValues* active = pItem->FindKey("current");
			if (!active)
				active = pItem->FindKey("local", true);
			if (active)
			{
				JSObject item;
				AddSubKeys(active, item);
				return item;
			}
			else
				return JSValue(0);
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("findFirstLibraryItem"))
	{
		// build the search info
		KeyValues* pSearchInfo = new KeyValues("search");	// this gets deleted by the metaverse manager!!

		unsigned int numArgs = args.size();
		if (numArgs == 0)
			return JSValue(0);
		else if (numArgs > 0)
			pSearchInfo->SetString("title", WebStringToCharString(args[0].ToString()));

		if (numArgs > 1)
			pSearchInfo->SetString("type", WebStringToCharString(args[1].ToString()));

		// start the search
		KeyValues* pItem = g_pAnarchyManager->GetMetaverseManager()->FindFirstLibraryItem(pSearchInfo);

		if (pItem)
		{
			KeyValues* active = pItem->FindKey("current");
			if (!active)
				active = pItem->FindKey("local", true);
			if (active)
			{
				JSObject item;
				AddSubKeys(active, item);
				return item;
			}
			else
				return JSValue(0);
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("findNextLibraryItem"))
	{
		KeyValues* pItem = g_pAnarchyManager->GetMetaverseManager()->FindNextLibraryItem();
		if (pItem)
		{
			KeyValues* active = pItem->FindKey("current");
			if (!active)
				active = pItem->FindKey("local", true);
			if (active)
			{
				JSObject item;
				AddSubKeys(active, item);
				return item;
			}
			else
				return JSValue(0);
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
		else if (numArgs > 0)
			pSearchInfo->SetString("title", WebStringToCharString(args[0].ToString()));
		
		// start the search
		KeyValues* pItem = g_pAnarchyManager->GetMetaverseManager()->FindLibraryItem(pSearchInfo);

		if (pItem)
		{
			KeyValues* active = pItem->FindKey("current");
			if (!active)
				active = pItem->FindKey("local", true);
			if (active)
			{
				JSObject item;
				AddSubKeys(active, item);
				return item;
			}
			else
				return JSValue(0);
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("updateItem"))
	{
		std::string id = WebStringToCharString(args[0].ToString());

		KeyValues* pItem = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(id);
		if (pItem)
		{
			KeyValues* active = pItem->FindKey("current");
			if (!active)
				active = pItem->FindKey("local", true);

			if (active)
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
					g_pAnarchyManager->GetCanvasManager()->RefreshItemTextures(id, "ALL");
					//g_pAnarchyManager->GetCanvasManager()->RefreshItemTextures(id, "screen");
					//g_pAnarchyManager->GetCanvasManager()->RefreshItemTextures(id, "marquee");
				}

				return JSValue(true);
			}
			else
				return JSValue(0);
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("getFirstLibraryModel"))
	{
		KeyValues* pModel = g_pAnarchyManager->GetMetaverseManager()->GetFirstLibraryModel();

		if (pModel)
		{
			KeyValues* active = pModel->FindKey("current");
			if (!active)
				active = pModel->FindKey("local", true);
			if (active)
			{
				JSObject model;
				AddSubKeys(active, model);
				return model;
			}
			else
				return JSValue(0);
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("getNextLibraryModel"))
	{
		KeyValues* pModel = g_pAnarchyManager->GetMetaverseManager()->GetNextLibraryModel();
		if (pModel)
		{
			KeyValues* active = pModel->FindKey("current");
			if (!active)
				active = pModel->FindKey("local", true);
			if (active)
			{
				JSObject model;
				AddSubKeys(active, model);
				return model;
			}
			else
				return JSValue(0);
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
			KeyValues* active = pModel->FindKey("current");
			if (!active)
				active = pModel->FindKey("local", true);
			if (active)
			{
				JSObject model;
				AddSubKeys(active, model);
				return model;
			}
			else
				return JSValue(0);
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("findNextLibraryModel"))
	{
		KeyValues* pModel = g_pAnarchyManager->GetMetaverseManager()->FindNextLibraryModel();
		if (pModel)
		{
			KeyValues* active = pModel->FindKey("current");
			if (!active)
				active = pModel->FindKey("local", true);
			if (active)
			{
				JSObject model;
				AddSubKeys(active, model);
				return model;
			}
			else
				return JSValue(0);
		}
		else
			return JSValue(0);
	}
	else if (method_name == WSLit("detectAllMapScreenshots"))
	{
		JSObject response;

		std::map<std::string, std::string>& screenshots = g_pAnarchyManager->GetMetaverseManager()->DetectAllMapScreenshots();
		std::map<std::string, std::string>::iterator it = screenshots.begin();
		while (it != screenshots.end())
		{
			response.SetProperty(WSLit(it->first.c_str()), WSLit(it->second.c_str()));
			it++;
		}

		return response;
	}
	else if (method_name == WSLit("getAllMapScreenshots"))
	{
		JSObject response;

		std::map<std::string, std::string>& screenshots = g_pAnarchyManager->GetMetaverseManager()->GetAllMapScreenshots();
		std::map<std::string, std::string>::iterator it = screenshots.begin();
		while (it != screenshots.end())
		{
			response.SetProperty(WSLit(it->first.c_str()), WSLit(it->second.c_str()));
			it++;
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
			active = map->FindKey("current");
			if (!active)
				active = map->FindKey("local", true);

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
		JSObject response;

		C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->GetSelectedLibretroInstance();//dynamic_cast<C_LibretroInstance*>(g_pAnarchyManager->)
		if (pLibretroInstance)
		{
			if (pLibretroInstance->HasInfo())
			{
				LibretroInstanceInfo_t* pInfo = pLibretroInstance->GetInfo();
				
				DevMsg("Adding libretro options to response...\n");
				//std::vector<libretro_core_option*>& libretroOptions = pLibretroInstance->GetAllOptions();
				JSArray options;

				unsigned int valuesMax;
				unsigned int j;

				libretro_core_option* libretroOption;
				unsigned int max = pInfo->options.size();// libretroOptions.size();
				for (unsigned int i = 0; i < max; i++)
				{
					libretroOption = pInfo->options[i];// libretroOptions[i];

					JSObject option;
					option.SetProperty(WSLit("name_internal"), WSLit(libretroOption->name_internal));
					option.SetProperty(WSLit("name_display"), WSLit(libretroOption->name_display));
					option.SetProperty(WSLit("current"), WSLit(VarArgs("%i", pLibretroInstance->GetOptionCurrentValue(i))));

					DevMsg("Adding %s (%s) w/ current value %i\n", libretroOption->name_display, libretroOption->name_internal, pLibretroInstance->GetOptionCurrentValue(i));

					JSArray values;
					valuesMax = libretroOption->values.size();
					for (j = 0; j < valuesMax; j++)
					{
						DevMsg("\tAdding value definition %i: %s\n", i, libretroOption->values[j].c_str());
						values.Push(WSLit(libretroOption->values[j].c_str()));
					}

					option.SetProperty(WSLit("values"), values);
					options.Push(option);
				}

				response.SetProperty(WSLit("options"), options);
			}
		}

		return response;
	}
	else if (method_name == WSLit("getMapInstances"))
	{
		std::string mapId = WebStringToCharString(args[0].ToString());

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
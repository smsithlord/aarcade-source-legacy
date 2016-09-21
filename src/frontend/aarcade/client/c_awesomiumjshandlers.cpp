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
		g_pAnarchyManager->GetInputManager()->DeactivateInputMode();

		std::string id = WebStringToCharString(args[0].ToString());
		DevMsg("SPAWN THE SHIT! %s\n", id.c_str());

		/*
		std::string modelFile = "";
		std::string msg = VarArgs("spawnshortcut \"%s\" \"%s\" %.10f %.10f %.10f %.10f %.10f %.10f\n", pNearObject->itemId.c_str(), modelFile.c_str(), pNearObject->origin.x, pNearObject->origin.y, pNearObject->origin.z, pNearObject->angles.x, pNearObject->angles.y, pNearObject->angles.z);
		engine->ServerCmd(msg.c_str(), false);
		*/
		//g_pAnarchyManager->GetInstanceManager()->Spaw>SpawnItem(id);
	}
	else if (method_name == WSLit("launchItem"))
	{
		DevMsg("LAUNCH THE SHIT!\n");
		std::string id = WebStringToCharString(args[0].ToString());

		KeyValues* pItem = g_pAnarchyManager->GetMetaverseManager()->GetLibraryItem(id);
		if (pItem)
		{
			KeyValues* active = pItem->FindKey("current");
			if (!active)
				active = pItem->FindKey("local", true);

			if (active)
			{
				std::string executable = "";
				std::string params = "";
				if (Q_strcmp(active->GetString("app"), ""))
				{
					KeyValues* pApp = g_pAnarchyManager->GetMetaverseManager()->GetLibraryApp(active->GetString("app"));
					KeyValues* activeApp = pApp->FindKey("current");
					if (!activeApp)
						activeApp = pApp->FindKey("local", true);

					if (activeApp)
					{
						executable = activeApp->GetString("file");

						std::string filepath = "";
						KeyValues* filepaths = activeApp->FindKey("filepaths", true);
						for (KeyValues *sub = filepaths->GetFirstSubKey(); sub; sub = sub->GetNextKey())
						{
							filepath = sub->GetString("path");
							break;
						}

						params = activeApp->GetString("commandformat");
						if (params == "")
							params = "\"" + filepath + std::string(active->GetString("file")) + "\"";
						else
						{
							// replace $FILE with active->GetString("file")
							// replace $QUOTE with a double quote
							// replace $SHORTFILE with active->GetString("file")'s filename only
							// replace etc.

							size_t found = params.find("$FILE");
							while (found != std::string::npos)
							{
								params.replace(found, 5, filepath + active->GetString("file"));
								found = params.find("$FILE");
							}

							found = params.find("$QUOTE");
							while (found != std::string::npos)
							{
								params.replace(found, 6, "\"");
								found = params.find("$QUOTE");
							}
						}
					}
				}
				
				if (executable == "")
					executable = active->GetString("file");

				if (!Q_strcmp(VarArgs("%i", Q_atoi(executable.c_str())), executable.c_str()))
				{
					// we are just an integer!! (probably a Steam game)
					executable = "steam://run/" + executable;
				}

				DevMsg("Item to launch: %s %s\n", executable.c_str(), params.c_str());
				//vgui::system()->ShellExecuteA("open", executable.c_str());
				vgui::system()->ShellExecuteEx("open", executable.c_str(), params.c_str());
			}
		}

		g_pAnarchyManager->Pause();
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

		//g_pAnarchyManager->GetWebManager()->DispatchJavaScriptMethod(pBrowserInstance, "arcadeHud", "onActivateInputMode", params);





		//g_pAnarchyManager->GetWebManager()->DispatchJavaScriptMethod(pWebTab, "arcadeHud", "onActivateInputMode", params);
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
	else if (method_name == WSLit("simpleImageReady"))
	{
	//	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
		//C_EmbeddedInstance* pEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();

		C_AwesomiumBrowserInstance* pImagesBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("images");

//		C_WebTab* pWebTab = g_pAnarchyManager->GetWebManager()->GetWebBrowser()->FindWebTab(caller);
		//if (pWebTab->GetId() != "images")
		if (pImagesBrowserInstance->GetWebView() != caller)
			return;// JSValue(false);

		std::string channel = WebStringToCharString(args[0].ToString());
		std::string itemId = WebStringToCharString(args[1].ToString());
		std::string field = WebStringToCharString(args[2].ToString());

		if (channel == "" && itemId == "" && field == "")
		{
			pImagesBrowserInstance->SetNumImagesLoading(0);
			//pWebTab->DecrementNumImagesLoading();
		}
		else if (channel != "" && itemId != "" && field != "")
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
					std::string imageId = g_pAnarchyManager->GenerateLegacyHash(fieldKv->GetString());

					// URLs that are already loaded would have been provided in the blacklist, so the rendered URL should always be fresh at this point.
					std::string textureName = "image_";
					textureName += imageId;

					int iWidth = 512;
					int iHeight = 512;

					//ITexture* pTexture = g_pMaterialSystem->FindTexture(textureName.c_str(), TEXTURE_GROUP_MODEL, true, 1);
					//if ( !pTexture )
					ITexture* pTexture = g_pMaterialSystem->CreateProceduralTexture(textureName.c_str(), TEXTURE_GROUP_MODEL, iWidth, iHeight, IMAGE_FORMAT_BGR888, 1);

					if (!pTexture)
						DevMsg("Failed to create texture!\n");

					// get the regen and assign it
					CCanvasRegen* pRegen = g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen();
					pTexture->SetTextureRegenerator(pRegen);
					
					pRegen->SetEmbeddedInstance(pImagesBrowserInstance);
					pTexture->Download();
					
					pImagesBrowserInstance->OnSimpleImageReady(channel, itemId, field, pTexture);
				}
			}
		}
		else if (itemId != "")
		{
			pImagesBrowserInstance->OnSimpleImageReady(channel, itemId, field, null);
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
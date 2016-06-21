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

				DevMsg("Item to launch: %s %s\n", executable.c_str(), params.c_str());
				//vgui::system()->ShellExecuteA("open", executable.c_str());
				vgui::system()->ShellExecuteEx("open", executable.c_str(), params.c_str());
			}
		}
	}
	else if (method_name == WSLit("loadNextLocalAppCallback"))
	{
		// FIXME: This should be done outside of the awesomeium-specific classes!!
		C_MetaverseManager* pMetaverseManager = g_pAnarchyManager->GetMetaverseManager();
		C_WebTab* pHudWebTab = g_pAnarchyManager->GetWebManager()->GetHudWebTab();

		KeyValues* app = pMetaverseManager->LoadNextLocalApp();
		if (app)
			pHudWebTab->AddHudLoadingMessage("progress", "", "Loading Apps", "locallibraryapps", "", "", "+", "loadNextLocalAppCallback");
		else
		{
			pMetaverseManager->LoadLocalAppClose();
			g_pAnarchyManager->OnLoadAllLocalAppsComplete();
		}
	}
	else if (method_name == WSLit("loadLocalAppClose"))	// shouldn't really ever be called!!! (unless user abors the loading process)
		g_pAnarchyManager->GetMetaverseManager()->LoadLocalAppClose();
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
		std::string id = WebStringToCharString(args[0].ToString());

		C_WebTab* pWebTab;
		if (id == "hud")
			pWebTab = g_pAnarchyManager->GetWebManager()->GetHudWebTab();
		else
			pWebTab = g_pAnarchyManager->GetWebManager()->FindWebTab(id);

		WebView* pWebView = g_pAnarchyManager->GetWebManager()->GetWebBrowser()->FindWebView(pWebTab);
		pWebView->DidCancelPopupMenu();

		return JSValue(true);
	}
	else if (method_name == WSLit("didSelectPopupMenuItem"))
	{
		std::string id = WebStringToCharString(args[0].ToString());
		int itemIndex = args[1].ToInteger();

		C_WebTab* pWebTab;
		if (id == "hud")
			pWebTab = g_pAnarchyManager->GetWebManager()->GetHudWebTab();
		else
			pWebTab = g_pAnarchyManager->GetWebManager()->FindWebTab(id);

		WebView* pWebView = g_pAnarchyManager->GetWebManager()->GetWebBrowser()->FindWebView(pWebTab);
		pWebView->DidSelectPopupMenuItem(itemIndex);

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
	else
		return WSLit("0");
}
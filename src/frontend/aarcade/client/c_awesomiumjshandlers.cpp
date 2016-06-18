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
				DevMsg("Item to launch: %s\n", active->GetString("file"));
				vgui::system()->ShellExecuteA("open", active->GetString("file"));
			}
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
	if (method_name == WSLit("getFirstLibraryItem"))
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
	else
		return WSLit("0");
}
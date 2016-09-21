#include "cbase.h"

//#include "c_simple_image_entity.h"
//#include "c_webViewInput.h"
//#include "aa_globals.h"
#include "c_anarchymanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//ConVar xbmc_enable( "xbmc_enable", "0", FCVAR_ARCHIVE );
ConVar default_width( "default_width", "256", FCVAR_ARCHIVE);
ConVar default_height( "default_height", "256", FCVAR_ARCHIVE);

void TestFunction( const CCommand &args )
{
	// WORKING SEND/RECIEVE FILE CALLS
	#include "inetchannel.h"
//	INetChannel* pINetChannel = static_cast<INetChannel*>(engine->GetNetChannelInfo());
//	pINetChannel->RequestFile("downloads/<hash>.vtf", false);
//	pINetChannel->SendFile("stuff/test.jpg", 0, false);

	// WORKING CURSOR POSITIONS
//	#include "vgui/IInput.h"
//	#include <vgui_controls/Controls.h>
//	int x, y;
//	vgui::input()->GetCursorPos(x, y);

	// NEW TEST

	//webviewinput->Create();
	DevMsg("Planel created.\n");
}
ConCommand test_function( "testfunc", TestFunction, "Usage: executes an arbitrary hard-coded C++ routine" );

void RunEmbeddedLibretro(const CCommand &args)
{
	C_LibretroManager* pLibretroManager = g_pAnarchyManager->GetLibretroManager();
	if ( pLibretroManager )
		pLibretroManager->RunEmbeddedLibretro("V:/Movies/Jay and silent Bob Strike Back (2001).avi");
}
ConCommand run_embedded_libretro("run_embedded_libretro", RunEmbeddedLibretro, "Usage: runs embedded apps");

void RunEmbeddedSteamBrowser(const CCommand &args)
{
	C_SteamBrowserManager* pSteamBrowserManager = g_pAnarchyManager->GetSteamBrowserManager();
	if (pSteamBrowserManager)
		pSteamBrowserManager->RunEmbeddedSteamBrowser();
}
ConCommand run_embedded_steam_browser("run_embedded_steam_browser", RunEmbeddedSteamBrowser, "Usage: runs embedded apps");

void RunEmbeddedAwesomiumBrowser(const CCommand &args)
{
	C_AwesomiumBrowserManager* pAwesomiumBrowserManager = g_pAnarchyManager->GetAwesomiumBrowserManager();
	if (pAwesomiumBrowserManager)
		pAwesomiumBrowserManager->RunEmbeddedAwesomiumBrowser();
}
ConCommand run_embedded_awesomium_browser("run_embedded_awesomium_browser", RunEmbeddedAwesomiumBrowser, "Usage: runs embedded apps");

void SetContinuous(const CCommand &args)
{
	C_BaseEntity* pEntity = g_pAnarchyManager->GetSelectedEntity();
	if (pEntity)
	{
		C_PropShortcutEntity* pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);

		std::vector<C_EmbeddedInstance*> embeddedInstances;
		pShortcut->GetEmbeddedInstances(embeddedInstances);

		C_EmbeddedInstance* pEmbeddedInstance;
		C_EmbeddedInstance* testerInstance;
		unsigned int i;
		unsigned int size = embeddedInstances.size();
		for (i = 0; i < size; i++)
		{
			pEmbeddedInstance = embeddedInstances[i];
			if (pEmbeddedInstance->GetId() == "images")
			{
				testerInstance = g_pAnarchyManager->GetCanvasManager()->FindEmbeddedInstance("auto" + pShortcut->GetItemId());
				if (testerInstance && testerInstance->GetTexture())
				{
					g_pAnarchyManager->DeselectEntity(pEntity, "", false);
					break; // only put the 1st embedded instance on continous play
				}
			}
		}
	}
}
ConCommand setcontinuous("setcontinuous", SetContinuous, "Usage: sets the selected entity as continuous play.");

void RememberWrapper(const CCommand &args)
{
	engine->ClientCmd("setcontinuous\n");
}
ConCommand rememberwrapper("-remember", RememberWrapper, "Usage: wrapper for the remember button to mean setcontinous now.");

void RunAArcade(const CCommand &args)
{
	g_pAnarchyManager->RunAArcade();
}
ConCommand run_aarcade("run_aarcade", RunAArcade, "Usage: runs AArcade");

/*
void TestFunction2( const CCommand &args )
{
	// WORKING SEND/RECIEVE FILE CALLS
//	#include "inetchannel.h"
//	INetChannel* pINetChannel = static_cast<INetChannel*>(engine->GetNetChannelInfo());
//	pINetChannel->RequestFile("downloads/<hash>.vtf", false);
//	pINetChannel->SendFile("stuff/test.jpg", 0, false);

	// WORKING CURSOR POSITIONS
//	#include "vgui/IInput.h"
//	#include <vgui_controls/Controls.h>
//	int x, y;
//	vgui::input()->GetCursorPos(x, y);

	// NEW TEST


	webviewinput->Create();
	DevMsg("Planel created.\n");
}

ConCommand test_function2( "testfunc2", TestFunction2, "Usage: executes an arbitrary hard-coded C++ routine" );
*/

void AnarchyManager(const CCommand &args)
{
	DevMsg("Start the anarchy manager NOW!\n");
	g_pAnarchyManager->AnarchyStartup();
}

ConCommand anarchymanager("anarchymanager", AnarchyManager, "Starts the Anarchy Manager.", FCVAR_HIDDEN);

void ActivateInputMode(const CCommand &args)
{
	//bool fullscreen = (args.ArgC() > 1);
	C_EmbeddedInstance* pSelectedEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
	if (pSelectedEmbeddedInstance)
		g_pAnarchyManager->GetInputManager()->ActivateInputMode(false, false, pSelectedEmbeddedInstance);// fullscreen);
}
ConCommand activateinputmode("+hdview_input_toggle", ActivateInputMode, "Turns ON input mode.", FCVAR_NONE);

void DeactivateInputMode(const CCommand &args)
{
	g_pAnarchyManager->GetInputManager()->DeactivateInputMode();
}
ConCommand deactivateinputmode("-hdview_input_toggle", DeactivateInputMode, "Turns OFF input mode.", FCVAR_NONE);

void AttemptSelectObject(const CCommand &args)
{
	g_pAnarchyManager->AttemptSelectEntity();
}
ConCommand attemptselectobject("focus", AttemptSelectObject, "Attempts to select the object under your crosshair.", FCVAR_NONE);

void Launch( const CCommand &args )
{
	g_pAnarchyManager->GetLibretroManager()->CreateLibretroInstance();

	//g_pFullFileSystem->AddSearchPath(installFolder, "GAME", PATH_ADD_TO_TAIL);

	//std::string fullPath = VarArgs("%s\\", installFolder);

	//unsigned int uNumModels = 0;
	//unsigned int uNumItems = 0;
	//std::string id = VarArgs("%llu", details->m_nPublishedFileId);




	//g_pAnarchyManager->GetInstanceManager()->LoadLegacyInstance();




	/*
	g_pAnarchyManager->GetWebManager()->GetHudWebTab()->AddHudLoadingMessage("progress", "", "Importing Old AArcade Data", "importfolder", "0", "1", "0");
	std::string path = "A:\\SteamLibrary\\steamapps\\common\\Anarchy Arcade\\aarcade\\";
	g_pAnarchyManager->GetMetaverseManager()->LoadFirstLocalItemLegacy(true, path, "", "");
	g_pFullFileSystem->AddSearchPath(path.c_str(), "MOD", PATH_ADD_TO_TAIL);
	g_pFullFileSystem->AddSearchPath(path.c_str(), "GAME", PATH_ADD_TO_TAIL);
	//DevMsg("Loaded %u items from %s\n", uNumItems, path.c_str());
	*/












//	uNumItems = g_pAnarchyManager->GetMetaverseManager()->LoadAllLocalItemsLegacy(uNumModels, path, "", "");
	//			g_pFullFileSystem->AddSearchPath(installFolder, "MOD", PATH_ADD_TO_TAIL);
	//DevMsg("Loaded %u items from %s\n", uNumItems, path.c_str());

	//if( args.ArgC() < 2 )
//		return;
	/*
	C_PropSimpleImageEntity* pProp = NULL;
	pProp = dynamic_cast<C_PropSimpleImageEntity*>( C_BaseEntity::Instance( Q_atoi(args[1]) ) );

	if( !pProp )
	{
		DevMsg("Invalid entindex specified for activate command!\n");
		return;
	}

	pProp->OnUse();
	*/
}

ConCommand launch( "aa_activated", Launch, "Usage: aa_activated entindex" );

void DetectAllMaps(const CCommand &args)
{
	g_pAnarchyManager->GetMetaverseManager()->DetectAllMaps();
	//DevMsg("Detect all maps!\n");
}
ConCommand detectallmaps("detectallmaps", DetectAllMaps, "Usage: aa_activated entindex");

void SpawnObjects(const CCommand &args)
{
	std::string instanceId = g_pAnarchyManager->GetInstanceId();
	if (instanceId != "")
	{
		std::string uri = "asset://ui/spawnItems.html?max=" + std::string(args[1]);
		/*
		C_WebTab* pHudWebTab = g_pAnarchyManager->GetWebManager()->GetHudWebTab();
		g_pAnarchyManager->GetWebManager()->SelectWebTab(pHudWebTab);
		pHudWebTab->SetUrl(uri);
		g_pAnarchyManager->GetInputManager()->ActivateInputMode(true);
		*/

		C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
		g_pAnarchyManager->GetAwesomiumBrowserManager()->SelectAwesomiumBrowserInstance(pHudBrowserInstance);
//		g_pAnarchyManager->GetWebManager()->SelectWebTab(pHudWebTab);
//		pHudWebTab->SetUrl(uri);
		pHudBrowserInstance->SetUrl(uri);
		g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, false, pHudBrowserInstance);
	}

	//g_pAnarchyManager->GetInstanceManager()->SpawnNearestObject();
	//DevMsg("Detect all maps!\n");
}
ConCommand spawnobjects("spawnobjects", SpawnObjects, "Usage: ...");
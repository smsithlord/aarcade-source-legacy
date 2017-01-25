#include "cbase.h"

//#include "c_simple_image_entity.h"
//#include "c_webViewInput.h"
//#include "aa_globals.h"
#include "c_anarchymanager.h"

#include "c_openglmanager.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//ConVar xbmc_enable( "xbmc_enable", "0", FCVAR_ARCHIVE );
ConVar default_width( "default_width", "256", FCVAR_ARCHIVE);
ConVar default_height( "default_height", "256", FCVAR_ARCHIVE);
ConVar broadcast_mode("broadcast_mode", "0", FCVAR_NONE);
ConVar kodi_info("kodi_info", "xbmc:xbmc@192.168.0.100:8080", FCVAR_ARCHIVE, "The username:password@ip:port of the 1st Kodi host.");

void TestFunction( const CCommand &args )
{
	// WORKING SEND/RECIEVE FILE CALLS
	//#include "inetchannel.h"
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
	//DevMsg("Planel created.\n");

	g_pAnarchyManager->TestSQLite();

	/*
	DevMsg("Setting url to overlay test...\n");
	C_AwesomiumBrowserInstance* pHudInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	pHudInstance->SetUrl("asset://ui/cabinetSelect.html");
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, false, null, true);
	*/

	//system("Arcade_Launcher.bat");

	//C_OpenGLManager* pOpenGLManager = new C_OpenGLManager();
	//pOpenGLManager->Init();





	/*
	// Scan user profile.
	// 1. Activate input mode.
	// 2. Navigate to the user's games list on their Steam profile in the in-game Steamworks browser.
	// 3. Notify & instruct the user if their profile is set to private, otherwise have an "IMPORT" button appear.
	// 4. Import all games from their list into a KeyValues file ownedGames.key
	// 5. Load all entries from ownedGames.key as items, but do not automatically save them out until the user modifies them.

	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	if (g_pAnarchyManager->GetSelectedEntity())
		g_pAnarchyManager->DeselectEntity("asset://ui/blank.html");
	else
		pHudBrowserInstance->SetUrl("asset://ui/blank.html");
	
	CSteamID sid = steamapicontext->SteamUser()->GetSteamID();
	std::string profileUrl = "http://www.steamcommunity.com/profiles/" + std::string(VarArgs("%llu", sid.ConvertToUint64())) + "/games/?tab=all";

	C_SteamBrowserInstance* pSteamBrowserInstance = g_pAnarchyManager->GetSteamBrowserManager()->CreateSteamBrowserInstance();
	pSteamBrowserInstance->SetActiveScraper("importSteamGames", "", "");
	pSteamBrowserInstance->Init("", profileUrl, null);
	pSteamBrowserInstance->Focus();
	pSteamBrowserInstance->Select();
	g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(pSteamBrowserInstance);
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, pSteamBrowserInstance);
	*/

	// GOOD STEAM GAMES IMPOOOOOOORT!!
	//g_pAnarchyManager->BeginImportSteamGames();

	/*
	KeyValues* kv = new KeyValues("tester");
	kv->SetString("val", "Will it work? xxxxxx");

	// NOTE: The Source filesystem will try to auto-lowercase file names!!!
	// So the file must be created case-correct outside of the Source filesystem and already exist before the Source filesystem writes to it.
	kv->SaveToFile(g_pFullFileSystem, "tEsTeR.key", "DEFAULT_WRITE_PATH");
	*/


	/*
	CUtlBuffer buf;
	KeyValues* pObjectKV = new KeyValues("originalTester");//pInstanceObjectsKV->FindKey(VarArgs("%s/local", objectId.c_str()), true);
	pObjectKV->SetString("originalTesterKey", "yup");
	pObjectKV->SetString("originalTesterKey2", "yup2");
	pObjectKV->SetString("originalTesterKey3", "yup3");
	pObjectKV->WriteAsBinary(buf);
	pObjectKV->deleteThis();

	int size = buf.Size();
	DevMsg("Buffer size here is: %i\n", size);
	void* mem = malloc(size);
	Q_memcpy(mem, buf.Base(), size);

	CUtlBuffer buf2(0, size, 0);
	buf2.CopyBuffer(mem, size);
	int size2 = buf2.Size();
	DevMsg("Processed buffer size is: %i\n", size2);
	
	KeyValues* pTesterKV = new KeyValues("reduxTester");
	pTesterKV->ReadAsBinary(buf2);
	DevMsg("Annd here the big result is: %s\n", pTesterKV->GetString("originalTesterKey"));
	pTesterKV->deleteThis();
	*/
}
ConCommand test_function( "testfunc", TestFunction, "Usage: executes an arbitrary hard-coded C++ routine" );

void TestFunctionOff(const CCommand &args)
{
	g_pAnarchyManager->TestSQLite2();
	//g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
	//g_pAnarchyManager->GetHUDManager
}
ConCommand test_function_off("testfunc2", TestFunctionOff, "Usage: executes an arbitrary hard-coded C++ routine");

void ImportSteamGames(const CCommand &args)
{
	g_pAnarchyManager->BeginImportSteamGames();
}
ConCommand import_steam_games("import_steam_games", ImportSteamGames, "Usage: imports your steam games from your public profile");

void WheelUp(const CCommand &args)
{
	g_pAnarchyManager->GetInputManager()->OnMouseWheeled(1);
}
ConCommand wheel_up("wheelup", WheelUp, "Usage: mouse wheel up");

void WheelDown(const CCommand &args)
{
	g_pAnarchyManager->GetInputManager()->OnMouseWheeled(-1);
}
ConCommand wheel_down("wheeldown", WheelDown, "Usage: mouse wheel down");

void RunEmbeddedLibretro(const CCommand &args)
{
	C_LibretroManager* pLibretroManager = g_pAnarchyManager->GetLibretroManager();
	if (pLibretroManager)
		pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/Movies/Flash Gordon (1980).avi");
		//pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/Movies/Jay and silent Bob Strike Back (2001).avi");
		//pLibretroManager->RunEmbeddedLibretro("mupen64plus_libretro.dll", "X:\\Emulators\\N64\\Roms\\Super Mario 64 (U) [!].zip");
		//pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/Movies/Jay and silent Bob Strike Back (2001).avi");
		//pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/Movies/Flash Gordon (1980).avi");
		//pLibretroManager->RunEmbeddedLibretro("mednafen_psx_libretro.dll", "X:\\Emulators\\PSP\\roms\\ffn-spac.iso");
		//pLibretroManager->RunEmbeddedLibretro("mednafen_psx_libretro.dll", "X:\\Emulators\\PS\\roms\\Need For Speed 4 - High Stakes [U] [SLUS-00826].cue");
		//pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/Movies/Jay and silent Bob Strike Back (2001).avi");
		//pLibretroManager->RunEmbeddedLibretro("mednafen_psx_libretro.dll", "X:\\Emulators\\PS\\roms\\Need For Speed 4 - High Stakes [U] [SLUS-00826].cue");
		//pLibretroManager->RunEmbeddedLibretro("mupen64plus_libretro.dll", "X:\\Emulators\\N64\\Roms\\Super Mario 64 (U) [!].zip");
		//pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/Movies/Jay and silent Bob Strike Back (2001).avi");
		//pLibretroManager->RunEmbeddedLibretro("mupen64plus_libretro.dll", "X:/Emulators/N64/Roms/GoldenEye 007 (U) [!].zip");
		//pLibretroManager->RunEmbeddedLibretro("mame_libretro.dll", "X:\\Emulators\\Arcade\\roms\\sfrush.zip");
		//pLibretroManager->RunEmbeddedLibretro("mupen64plus_libretro.dll", "X:\\Emulators\\N64\\Roms\\Super Mario 64 (U) [!].zip");
		//pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/Movies/Jay and silent Bob Strike Back (2001).avi");
		//pLibretroManager->RunEmbeddedLibretro("mupen64plus_libretro.dll", "X:\\Emulators\\N64\\Roms\\Super Mario 64 (U) [!].zip");
		
		//pLibretroManager->RunEmbeddedLibretro("mupen64plus_libretro.dll", "X:\\Emulators\\N64\\Roms\\Super Mario 64 (U) [!].zip");
		
		//pLibretroManager->RunEmbeddedLibretro("mame2014_libretro.dll", "X:\\Emulators\\Arcade\\roms\\lethalen.zip");
		//pLibretroManager->RunEmbeddedLibretro("snes9x_libretro.dll", "X:\\Emulators\\SNES\\Roms\\Donkey Kong Country - Competition Cartridge (U).smc");
		//pLibretroManager->RunEmbeddedLibretro("mame_libretro.dll", "X:\\Emulators\\Arcade\\roms\\lethalen.zip");
		//pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/Movies/Flash Gordon (1980).avi");
		//pLibretroManager->RunEmbeddedLibretro("snes9x_libretro.dll", "X:\\Emulators\\SNES\\Roms\\Donkey Kong Country - Competition Cartridge (U).smc");
		//pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/TV/Beavis & Butthead/Beavis and Butthead - Season 7/731 Drinking Butt-ies.mpg");
	
		//pLibretroManager->RunEmbeddedLibretro("mame_libretro.dll", "X:\\Emulators\\Arcade\\roms\\lethalen.zip");
		//pLibretroManager->RunEmbeddedLibretro("snes9x_libretro.dll", "X:\\Emulators\\SNES\\Roms\\Donkey Kong Country - Competition Cartridge (U).smc");
		//pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/Movies/Flash Gordon (1980).avi");
		//pLibretroManager->RunEmbeddedLibretro("mame2014_libretro.dll", "X:\\Emulators\\Arcade\\roms\\lethalen.zip");
		
		//pLibretroManager->RunEmbeddedLibretro("ffmpeg_libretro.dll", "V:/Movies/Flash Gordon (1980).avi");
	
		//pLibretroManager->RunEmbeddedLibretro("V:/Movies/Jay and silent Bob Strike Back (2001).avi");
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

void Continuous(const CCommand &args)
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
					g_pAnarchyManager->DeselectEntity("", false);
					break; // only put the 1st embedded instance on continous play
				}
			}
		}
	}
}
ConCommand continuous("continuous", Continuous, "Usage: sets the selected entity as continuous play.");

void CloseAll(const CCommand &args)
{
	g_pAnarchyManager->GetCanvasManager()->CloseAllInstances();
}
ConCommand closeall("closeall", CloseAll, "Usage: closes all open instaces (execpt for important game system ones)");

/*
void RememberWrapper(const CCommand &args)
{
	engine->ClientCmd("setcontinuous\n");
}
ConCommand rememberwrapper("-remember", RememberWrapper, "Usage: wrapper for the remember button to mean setcontinous now.");
*/

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

void CreateHotlink(const CCommand &args)
{
	// check if a propshortcut is under the player's crosshair
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	//if (!pPlayer)
		//return;

	//if (pPlayer->GetHealth() <= 0)
		//return;

	bool bAutoChooseLibrary = true;

	// fire a trace line
	trace_t tr;
	Vector forward;
	pPlayer->EyeVectors(&forward);
	UTIL_TraceLine(pPlayer->EyePosition(), pPlayer->EyePosition() + forward * MAX_COORD_RANGE, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

	C_BaseEntity *pEntity = (tr.DidHitNonWorldEntity()) ? tr.m_pEnt : null;

	// only allow prop shortcuts
	C_PropShortcutEntity* pShortcut = (pEntity) ? dynamic_cast<C_PropShortcutEntity*>(pEntity) : null;
	if (pShortcut && tr.fraction != 1.0 )
		bAutoChooseLibrary = false;	// TODO: If you want to highlight the object that the context menu applies to, now's the time.

	C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	if (bAutoChooseLibrary)
	{
		//DevMsg("DISPLAY MAIN MENU\n");
		if (g_pAnarchyManager->GetSelectedEntity())
			g_pAnarchyManager->DeselectEntity("asset://ui/libraryBrowser.html");
		else
			pHudBrowserInstance->SetUrl("asset://ui/libraryBrowser.html");

		g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true);
	}
	else
	{
		//DevMsg("DISPLAY BUILD MODE CONTEXT MENU\n");
		if (g_pAnarchyManager->GetInputManager()->GetInputMode())
			g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);

		if (g_pAnarchyManager->GetSelectedEntity())
			g_pAnarchyManager->DeselectEntity("asset://ui/buildModeContext.html");
		else
			pHudBrowserInstance->SetUrl("asset://ui/buildModeContext.html");

		g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, pHudBrowserInstance);
	}

	return;
}
ConCommand createhotlink("createhotlink", CreateHotlink, "Open up the library.", FCVAR_NONE);

void ActivateInputMode(const CCommand &args)
{
	// FIXME: Need to reject commands that are sent before the AArcade system is ready.
	//bool fullscreen = (args.ArgC() > 1);
	
	// if not spawning an object, do regular stuff
	if (!g_pAnarchyManager->GetMetaverseManager()->GetSpawningObjectEntity())
	{
		C_EmbeddedInstance* pSelectedEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
		if (pSelectedEmbeddedInstance)
			g_pAnarchyManager->GetInputManager()->ActivateInputMode(false, false, pSelectedEmbeddedInstance);// fullscreen);
	}
	else
	{
		//g_pAnarchyManager->DeactivateObjectPlacementMode(false);

		// undo changes AND cancel
		C_PropShortcutEntity* pShortcut = g_pAnarchyManager->GetMetaverseManager()->GetSpawningObjectEntity();
		g_pAnarchyManager->DeactivateObjectPlacementMode(false);

		//std::string id = pShortcut->GetObjectId();
		//g_pAnarchyManager->GetInstanceManager()->ResetObjectChanges(pShortcut);

		// "save" cha
		//m_pInstanceManager->ApplyChanges(id, pShortcut);
		DevMsg("CHANGES REVERTED\n");
	}
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

	if (broadcast_mode.GetBool())
		g_pAnarchyManager->xCastSetLiveURL();
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
	g_pAnarchyManager->GetInstanceManager()->SpawnActionPressed();

	/*
	std::string instanceId = g_pAnarchyManager->GetInstanceId();
	if (instanceId != "")
	{
		std::string uri = "asset://ui/spawnItems.html?max=" + std::string(args[1]);

		C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
		g_pAnarchyManager->GetAwesomiumBrowserManager()->SelectAwesomiumBrowserInstance(pHudBrowserInstance);
		pHudBrowserInstance->SetUrl(uri);
		g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, false, pHudBrowserInstance);
	}
	*/
}
ConCommand spawnobjects("spawnobjects", SpawnObjects, "Usage: ...");
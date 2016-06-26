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
	g_pAnarchyManager->AnarchyBegin();
}

ConCommand anarchymanager("anarchymanager", AnarchyManager, "Starts the Anarchy Manager.", FCVAR_HIDDEN);

void ActivateInputMode(const CCommand &args)
{
	//bool fullscreen = (args.ArgC() > 1);
	g_pAnarchyManager->GetInputManager()->ActivateInputMode();// fullscreen);
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
	//g_pAnarchyManager->GetLibretroManager()->CreateLibretroInstance();

	//g_pFullFileSystem->AddSearchPath(installFolder, "GAME", PATH_ADD_TO_TAIL);

	//std::string fullPath = VarArgs("%s\\", installFolder);

	//unsigned int uNumModels = 0;
	//unsigned int uNumItems = 0;
	//std::string id = VarArgs("%llu", details->m_nPublishedFileId);




	g_pAnarchyManager->GetInstanceManager()->LoadLegacyInstance();




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

void SpawnNearestObject(const CCommand &args)
{
	g_pAnarchyManager->GetInstanceManager()->SpawnNearestObject();
	//DevMsg("Detect all maps!\n");
}
ConCommand spawnnearestobject("spawnnearestobject", SpawnNearestObject, "Usage: ...");
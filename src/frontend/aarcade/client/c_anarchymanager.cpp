#include "cbase.h"
#include "aa_globals.h"
#include "c_anarchymanager.h"
//#include "c_systemtime.h"
#include "WinBase.h"
#include <cctype>
#include <algorithm>
#include "c_browseslate.h"
#include "../../sqlite/include/sqlite/sqlite3.h"
//#include "mathlib/mathlib.h"
//#include <math.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_AnarchyManager g_AnarchyManager;
extern C_AnarchyManager* g_pAnarchyManager(&g_AnarchyManager);

C_AnarchyManager::C_AnarchyManager() : CAutoGameSystemPerFrame("C_AnarchyManager")
{
	DevMsg("AnarchyManager: Constructor\n");

	// FIXME: ONLY WORKS FOR ME
	//m_legacyFolder = "A:\\SteamLibrary\\steamapps\\common\\Anarchy Arcade\\aarcade\\";
	m_legacyFolder = "";

	m_state = AASTATE_NONE;
	m_iState = 0;
	m_bSuspendEmbedded = false;
	m_bIncrementState = false;
	m_bPaused = false;
	m_pCanvasManager = null;
	//m_pWebManager = null;
	//m_pLoadingManager = null;
	m_pLibretroManager = null;
	m_pSteamBrowserManager = null;
	m_pInputManager = null;
	m_pSelectedEntity = null;
	m_pMountManager = null;
	m_pInstanceManager = null;
	m_dLastGenerateIdTime = 0;
	m_lastGeneratedChars = "000000000000";
}

C_AnarchyManager::~C_AnarchyManager()
{
	DevMsg("AnarchyManager: Destructor\n");
}

bool C_AnarchyManager::Init()
{
	DevMsg("AnarchyManager: Init\n");

	std::string workshopDir = engine->GetGameDirectory();	// just use the game directory to find workshop content normally.
	if (workshopDir == "d:\\projects\\aarcade-source\\game\\frontend")
		workshopDir = m_legacyFolder;

	workshopDir = workshopDir.substr(0, workshopDir.find_last_of("\\"));
	workshopDir = workshopDir.substr(0, workshopDir.find_last_of("\\"));
	workshopDir = workshopDir.substr(0, workshopDir.find_last_of("\\"));
	workshopDir = workshopDir.substr(0, workshopDir.find_last_of("\\"));
	workshopDir += "\\workshop\\content\\266430\\";
	m_workshopFolder = workshopDir;

	KeyValues* legacyLog = new KeyValues("legacy");
	if (legacyLog->LoadFromFile(g_pFullFileSystem, "legacy_log.key", "MOD"))
		m_legacyFolder = legacyLog->GetString("path");
	legacyLog->deleteThis();

	std::string aarcadeUserFolder = engine->GetGameDirectory();
	size_t found = aarcadeUserFolder.find_last_of("/\\");
	aarcadeUserFolder = aarcadeUserFolder.substr(0, found + 1);
	aarcadeUserFolder += std::string("aarcade_user");
	m_aarcadeUserFolder = aarcadeUserFolder;

	g_pFullFileSystem->CreateDirHierarchy("resource\\ui\\html", "DEFAULT_WRITE_PATH");

	return true;
}

void C_AnarchyManager::PostInit()
{
	DevMsg("AnarchyManager: PostInit\n");

	/*
	// allow for 2 instances of the Source engine to run
	// NOTE: There are conflicts with older Source engine games that use the default VGUI main menu, but other than that they work fine as well.
	bool keepGoing;

	// Disable mutexes
	keepGoing = true;
	while (keepGoing)
	{
		HANDLE myHandle = OpenMutex(MUTEX_ALL_ACCESS, true, "hl2_singleton_mutex");

		if (myHandle)
		{
			if (!ReleaseMutex(myHandle))
				myHandle = OpenMutex(MUTEX_ALL_ACCESS, true, "hl2_singleton_mutex");
			else
			{
				CloseHandle(myHandle);
				keepGoing = false;
			}
		}
		else
			keepGoing = false;
	}

	keepGoing = true;
	while (keepGoing)
	{
		HANDLE myHandle = OpenMutex(MUTEX_ALL_ACCESS, true, "ValvePlatformUIMutex");

		if (myHandle)
		{
			if (!ReleaseMutex(myHandle))
				myHandle = OpenMutex(MUTEX_ALL_ACCESS, true, "ValvePlatformUIMutex");
			else
			{
				CloseHandle(myHandle);
				keepGoing = false;
			}
		}
		else
			keepGoing = false;
	}


	//	keepGoing = true;
	//	while (keepGoing)
	{
		HANDLE myHandle = OpenMutex(MUTEX_ALL_ACCESS, true, "ValvePlatformWaitMutex");

		//		if (!ReleaseMutex(myHandle))
		//			myHandle = OpenMutex(MUTEX_ALL_ACCESS, true, "ValvePlatformWaitMutex");
		//		else
		//		{

		if (myHandle)
		{
			ReleaseMutex(myHandle);
			CloseHandle(myHandle);
		}
		//			keepGoing = false;
		//		}
	}
	*/
}

void C_AnarchyManager::Shutdown()
{
	DevMsg("AnarchyManager: Shutdown\n");

	/*
	if (m_pWebManager)
	{
		delete m_pWebManager;
		m_pWebManager = null;
	}
	*/

//	delete m_pLoadingManager;
	//m_pLoadingManager = null;

	if (m_pLibretroManager)
	{
		delete m_pLibretroManager;
		m_pLibretroManager = null;
	}

	if (m_pSteamBrowserManager)
	{
		delete m_pSteamBrowserManager;
		m_pSteamBrowserManager = null;
	}

	if (m_pMountManager)
	{
		delete m_pMountManager;
		m_pMountManager = null;
	}

	if (m_pWorkshopManager)
	{
		delete m_pWorkshopManager;
		m_pWorkshopManager = null;
	}

	if (m_pMetaverseManager)
	{
		delete m_pMetaverseManager;
		m_pMetaverseManager = null;
	}

	if (m_pInstanceManager)
	{
		delete m_pInstanceManager;
		m_pInstanceManager = null;
	}

	if (m_pAwesomiumBrowserManager)
	{
		delete m_pAwesomiumBrowserManager;
		m_pAwesomiumBrowserManager = null;
	}

	if (m_pInputManager)
	{
		m_pInputManager->DeactivateInputMode(true);
		delete m_pInputManager;
		m_pInputManager = null;
	}

	DevMsg("AnarchyManager: Finished Shutdown\n");

	//g_pFullFileSystem->RemoveAllSearchPaths();	// doesn't make shutdown faster and causes warnings about failing to write cfg/server_blacklist.txt
}

void C_AnarchyManager::LevelInitPreEntity()
{
	DevMsg("AnarchyManager: LevelInitPreEntity\n");
	m_instanceId = m_nextInstanceId;

	C_AwesomiumBrowserInstance* pImagesBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("images");
	pImagesBrowserInstance->ResetImagesSession();

	DevMsg("Finished resetting image session.\n");
}

void C_AnarchyManager::LevelInitPostEntity()
{
	DevMsg("AnarchyManager: LevelInitPostEntity\n");

	if (m_instanceId != "")
		g_pAnarchyManager->GetInstanceManager()->LoadInstance(m_instanceId);

	m_bSuspendEmbedded = false;

	engine->ClientCmd("sv_infinite_aux_power 1; mp_timelimit 0; sv_timeout 300; cl_timeout 300;\n");

//	engine->ClientCmd("r_drawothermodels 1;");
}

void C_AnarchyManager::LevelShutdownPreClearSteamAPIContext()
{
	DevMsg("AnarchyManager: LevelShutdownPreClearSteamAPIContext\n");
}

void C_AnarchyManager::LevelShutdownPreEntity()
{
	//engine->ClientCmd("r_drawothermodels 0;");	// FIXME: obsolete??

	DevMsg("AnarchyManager: LevelShutdownPreEntity\n");
	m_bSuspendEmbedded = true;

	C_BaseEntity* pEntity = this->GetSelectedEntity();
	if (pEntity)
		this->DeselectEntity();

	C_AwesomiumBrowserInstance* pImagesBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("images");
	pImagesBrowserInstance->ResetImagesSession();

///////	// all instances other than HUD and IMAGES should be removed at this time (until cross-map background item play gets re-enabled.)
	m_pCanvasManager->LevelShutdownPreEntity();

	/*
	C_WebTab* pWebTab = m_pWebManager->GetSelectedWebTab();
	if (pWebTab)
		m_pWebManager->DeselectWebTab(pWebTab);

	m_pWebManager->LevelShutdownPreEntity();
	*/
}

void C_AnarchyManager::LevelShutdownPostEntity()
{
	DevMsg("AnarchyManager: LevelShutdownPostEntity\n");

	if (m_pInstanceManager)
		m_pInstanceManager->LevelShutdownPostEntity();

	m_instanceId = "";	// wtf is this??  (its the name of the currently loaded instance, which *should* actually be held inside of the isntance manager.

	// Clear out the simple images
	// all instances other than HUD and IMAGES should be removed at this time (until cross-map background item play gets re-enabled.)
	m_pCanvasManager->LevelShutdownPostEntity();
}

void C_AnarchyManager::OnSave()
{
	DevMsg("AnarchyManager: OnSave\n");
}

void C_AnarchyManager::OnRestore()
{
	DevMsg("AnarchyManager: OnRestore\n");
}
void C_AnarchyManager::SafeRemoveIfDesired()
{
	//DevMsg("AnarchyManager: SafeRemoveIfDesired\n");
}

bool C_AnarchyManager::IsPerFrame()
{
	DevMsg("AnarchyManager: IsPerFrame\n");
	return true;
}

void C_AnarchyManager::PreRender()
{
	//DevMsg("AnarchyManager: PreRender\n");
}

void C_AnarchyManager::IncrementState()
{
	m_bIncrementState = true;
}

void C_AnarchyManager::Update(float frametime)
{
	if (m_bIncrementState)
	{
		m_bIncrementState = false;

		switch (m_state)
		{
			case AASTATE_NONE:
				m_state = AASTATE_INPUTMANAGER;
				break;

			case AASTATE_INPUTMANAGER:
				m_state = AASTATE_CANVASMANAGER;
				break;

			case AASTATE_CANVASMANAGER:
				m_state = AASTATE_LIBRETROMANAGER;
				//m_state = AASTATE_STEAMBROWSERMANAGER;
				break;

			case AASTATE_LIBRETROMANAGER:
			{
				// TEST: AUTO-CREATE AN INSTANCE, LOAD THE FFMPEG CORE, AND PLAY A MOVIE
				/*
				C_LibretroInstance* pLibretroInstance = m_pLibretroManager->CreateLibretroInstance();//>GetSelectedLibretroInstance();// 
				pLibretroInstance->Init();

				// load a core
				pLibretroInstance->LoadCore();

				// load a file
				pLibretroInstance->LoadGame();

				// tell the input manager that the libretro instance is active
				C_InputListenerLibretro* pListener = m_pLibretroManager->GetInputListener();
				m_pInputManager->SetInputCanvasTexture(pLibretroInstance->GetTexture());
				m_pInputManager->ActivateInputMode(true, true, (C_InputListener*)pListener);
				*/

				m_state = AASTATE_STEAMBROWSERMANAGER;
				break;
			}

			case AASTATE_STEAMBROWSERMANAGER:
			{
				// TEST: AUTO-CREATE AN INSTANCE, LOAD A WEBSITE
				/*
				C_SteamBrowserInstance* pSteamBrowserInstance = m_pSteamBrowserManager->CreateSteamBrowserInstance();
				pSteamBrowserInstance->Init("", "http://smarcade.net/dlcv2/view_youtube.php?id=CmRih_VtVAs&autoplay=1", null);//https://www.youtube.com/html5
				*/

				m_state = AASTATE_AWESOMIUMBROWSERMANAGER;
				//m_state = AASTATE_RUN;
				break;
			}

			case AASTATE_AWESOMIUMBROWSERMANAGER:
				m_state = AASTATE_AWESOMIUMBROWSERMANAGERWAIT;
				break;

			case AASTATE_AWESOMIUMBROWSERMANAGERWAIT:
				m_state = AASTATE_AWESOMIUMBROWSERMANAGERHUD;
				break;

			case AASTATE_AWESOMIUMBROWSERMANAGERHUD:
				m_state = AASTATE_AWESOMIUMBROWSERMANAGERHUDWAIT;
				break;

			case AASTATE_AWESOMIUMBROWSERMANAGERHUDWAIT:
				m_state = AASTATE_AWESOMIUMBROWSERMANAGERHUDINIT;
				break;

			case AASTATE_AWESOMIUMBROWSERMANAGERHUDINIT:
				m_state = AASTATE_AWESOMIUMBROWSERMANAGERIMAGES;
				break;

			case AASTATE_AWESOMIUMBROWSERMANAGERIMAGES:
				m_state = AASTATE_AWESOMIUMBROWSERMANAGERIMAGESWAIT;
				break;

			case AASTATE_AWESOMIUMBROWSERMANAGERIMAGESWAIT:
				m_state = AASTATE_AWESOMIUMBROWSERMANAGERIMAGESINIT;
				break;

			case AASTATE_AWESOMIUMBROWSERMANAGERIMAGESINIT:
				m_state = AASTATE_RUN;
				break;
		}
	}

	switch (m_state)
	{
		case AASTATE_RUN:
			if (m_bPaused)	// FIXME: You might want to let the web manager do its core logic, but don't render anything.
				return;

			//DevMsg("Float: %f\n", frametime);	// deltatime
			//DevMsg("Float: %i\n", gpGlobals->framecount);	// numframes total

			if (m_pCanvasManager)
				m_pCanvasManager->Update();

			if (m_pLibretroManager)
				m_pLibretroManager->Update();

			if (m_pSteamBrowserManager)
				m_pSteamBrowserManager->Update();

			if (m_pAwesomiumBrowserManager)
				m_pAwesomiumBrowserManager->Update();

			if (m_pInstanceManager)
				m_pInstanceManager->Update();

			if (m_pMetaverseManager)
				m_pMetaverseManager->Update();
			/*
			if (m_pWebManager)
				m_pWebManager->Update();
			*/

			//DevMsg("AnarchyManager: Update\n");
			break;

		case AASTATE_INPUTMANAGER:
			m_pInputManager = new C_InputManager();	// then wait for state change
			g_pAnarchyManager->IncrementState();
			break;

		case AASTATE_CANVASMANAGER:
			m_pCanvasManager = new C_CanvasManager();	// then wait for state change
			g_pAnarchyManager->IncrementState();
			break;

		case AASTATE_LIBRETROMANAGER:
			m_pLibretroManager = new C_LibretroManager();
			g_pAnarchyManager->IncrementState();
			break;

		case AASTATE_STEAMBROWSERMANAGER:
			m_pSteamBrowserManager = new C_SteamBrowserManager();
			g_pAnarchyManager->IncrementState();
			break;

		case AASTATE_AWESOMIUMBROWSERMANAGER:
			m_pAwesomiumBrowserManager = new C_AwesomiumBrowserManager();
			g_pAnarchyManager->IncrementState();
			break;

		case AASTATE_AWESOMIUMBROWSERMANAGERWAIT:
			if (m_pCanvasManager)
				m_pCanvasManager->Update();

			m_pAwesomiumBrowserManager->Update();
			break;

		case AASTATE_AWESOMIUMBROWSERMANAGERHUD:
			//m_pAwesomiumBrowserManager->CreateAwesomiumBrowserInstance("hud", "asset://ui/startup.html", true);	// defaults to asset://ui/blank.html
			m_pAwesomiumBrowserManager->CreateAwesomiumBrowserInstance("hud", "asset://ui/blank.html", true);
			g_pAnarchyManager->IncrementState();
			break;

		case AASTATE_AWESOMIUMBROWSERMANAGERHUDWAIT:
			if (m_pCanvasManager)
				m_pCanvasManager->Update();

			m_pAwesomiumBrowserManager->Update();
			break;

		case AASTATE_AWESOMIUMBROWSERMANAGERHUDINIT:
			DevMsg("Finished initing HUD.\n");
			g_pAnarchyManager->IncrementState();
			break;

		case AASTATE_AWESOMIUMBROWSERMANAGERIMAGES:
			m_pAwesomiumBrowserManager->CreateAwesomiumBrowserInstance("images", "asset://ui/imageLoader.html", true);	// defaults to asset://ui/blank.html	// does this need to be created here????
			g_pAnarchyManager->IncrementState();
			break;

		case AASTATE_AWESOMIUMBROWSERMANAGERIMAGESWAIT:
			if (m_pCanvasManager)
				m_pCanvasManager->Update();

			m_pAwesomiumBrowserManager->Update();
			break;

		case AASTATE_AWESOMIUMBROWSERMANAGERIMAGESINIT:
			DevMsg("Finished initing IMAGES.\n");
			m_pInstanceManager = new C_InstanceManager();
			m_pMetaverseManager = new C_MetaverseManager();
			m_pMetaverseManager->Init();
			m_pInputManager = new C_InputManager();

			// auto-load aarcade stuff
			g_pAnarchyManager->RunAArcade();

			/*
			C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
			//pAwesomiumBrowserInstance->SetUrl("asset://ui/startup.html");
			

			// Now start loading stuff in...
			//C_WebTab* pHudWebTab = m_pWebManager->GetHudWebTab();
			//C_EmbeddedInstance* pEmbeddedInstance = m_p
			m_pAwesomiumBrowserManager->SelectAwesomiumBrowserInstance(pHudBrowserInstance);
			m_pInputManager->ActivateInputMode(true, true, pHudBrowserInstance);
			//g_pAnarchyManager->GetInputManager()->ActivateInputMode(true);

			unsigned int uCount;
			std::string num;

			// And continue starting up
			uCount = m_pMetaverseManager->LoadAllLocalTypes();
			num = VarArgs("%u", uCount);
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading Types", "locallibrarytypes", "0", num, num);

			//= m_pMetaverseManager->LoadAllLocalTypes();
			//std::string num = VarArgs("%u", uItemCount);
			//	pHudWebTab->AddHudLoadingMessage("progress", "", "Loading Types", "locallibrarytypes", "0", num, num);

			uCount = m_pMetaverseManager->LoadAllLocalModels();
			num = VarArgs("%u", uCount);
			pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading Models", "locallibrarymodels", "0", num, num);

			//uItemCount = m_pMetaverseManager->LoadAllLocalApps();

			// load ALL local apps
			KeyValues* app = m_pMetaverseManager->LoadFirstLocalApp("MOD");
			if (app)
				pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading Apps", "locallibraryapps", "", "", "+", "loadNextLocalAppCallback");
			else
				this->OnLoadAllLocalAppsComplete();
			*/

			this->IncrementState();
			break;

		//case AASTATE_WEBMANAGER:
			/*
			// create a libretro instance
			C_LibretroInstance* pLibretroInstance = m_pLibretroManager->CreateLibretroInstance();//>GetSelectedLibretroInstance();// 
			pLibretroInstance->Init();

			// load a core
			pLibretroInstance->LoadCore();

			// load a file
			pLibretroInstance->LoadGame();

			// tell the input manager that the libretro instance is active
			C_InputListenerLibretro* pListener = m_pLibretroManager->GetInputListener();
			m_pInputManager->SetInputCanvasTexture(pLibretroInstance->GetTexture());
			m_pInputManager->ActivateInputMode(true, true, (C_InputListener*)pListener);

			steamapicontext->SteamHTMLSurface()->Init();
			SteamAPICall_t hAPICall = steamapicontext->SteamHTMLSurface()->CreateBrowser("", "");
			C_SteamworksBrowser* pSteamworksBrowser = new C_SteamworksBrowser();
			pSteamworksBrowser->Init(hAPICall);

			this->SetState(AASTATE_RUN);
			*/
		//	break;
	}
}

#include "vgui/IInput.h"
#include "ienginevgui.h"
bool C_AnarchyManager::HandleUiToggle()
{
	// ignore the start button on the gamepad if we are NOT the focused window
	HWND myHWnd = FindWindow(null, "AArcade: Source");
	//HWND myHWnd = GetActiveWindow();
	//HWND topHWnd = GetTopWindow(null);
	HWND foregroundWindow = GetForegroundWindow();
	//if (myHWnd != topHWnd)
	if (myHWnd != foregroundWindow)
		return true;

	// handle escape if in pause mode (ignore it)
	if (!engine->IsInGame())
	{
		// GOOD MAIN MENU EMBEDDED APP ESCAPE BINDS AS OF 9/13/2016

		if (m_pSteamBrowserManager)
		{
			C_SteamBrowserInstance* pInstance = m_pSteamBrowserManager->GetSelectedSteamBrowserInstance();
			//if (m_pInputManager->GetMainMenuMode() && m_pInputManager->GetInputMode() && m_pInputManager->GetFullscreenMode() && pInstance && pInstance->GetTexture() && pInstance->GetTexture() == m_pInputManager->GetInputCanvasTexture())
			if (pInstance && g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == pInstance)
			{
				m_pSteamBrowserManager->DestroySteamBrowserInstance(pInstance);
				m_pInputManager->SetEmbeddedInstance(null);
				return true;
			}
		}

		if (m_pLibretroManager && !vgui::input()->IsKeyDown(KEY_XBUTTON_START))
		{
			C_LibretroInstance* pInstance = m_pLibretroManager->GetSelectedLibretroInstance();
			//if (m_pInputManager->GetMainMenuMode() && m_pInputManager->GetInputMode() && m_pInputManager->GetFullscreenMode() && pInstance && pInstance->GetTexture() && pInstance->GetTexture() == m_pInputManager->GetInputCanvasTexture())
			if (pInstance && g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == pInstance)
			{
				m_pLibretroManager->DestroyLibretroInstance(pInstance);
				m_pInputManager->SetEmbeddedInstance(null);
				return true;
			}
		}

		if (m_pAwesomiumBrowserManager)
		{
			C_AwesomiumBrowserInstance* pInstance = m_pAwesomiumBrowserManager->GetSelectedAwesomiumBrowserInstance();
			//if (m_pInputManager->GetMainMenuMode() && m_pInputManager->GetInputMode() && m_pInputManager->GetFullscreenMode() && pInstance && pInstance->GetTexture() && pInstance->GetTexture() == m_pInputManager->GetInputCanvasTexture())
			if (pInstance && g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == pInstance )
			{
				//if (pInstance->GetId() == "hud" && )// || pInstance->GetId() == "images" )
				if (engine->IsInGame() || pInstance->GetId() != "hud" )
				{
					m_pAwesomiumBrowserManager->DestroyAwesomiumBrowserInstance(pInstance);
					m_pInputManager->SetEmbeddedInstance(null);
					return true;
				}
				else if (!engine->IsInGame() && pInstance->GetId() == "hud" )
				{
					g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);

						//C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
						//pHudBrowserInstance->SetUrl("asset://ui/welcome.html");
						//g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true);
				}
			}
		}

		return false;
	}

	if (m_bPaused)
	{
		this->Unpause();
		return true;
	}

	// update this code block when joypad input gets restricted to the selected input slate instance!!
	// (currently it will assume that a selected cabinet with a libretro instances ALWAYS wants joypad input!!
	// ignore if its a libretro instance
	C_BaseEntity* pEntity = this->GetSelectedEntity();
	if (pEntity)
	{
		C_PropShortcutEntity* pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);
		if (pShortcut)
		{
			std::vector<C_EmbeddedInstance*> embeddedInstances;
			pShortcut->GetEmbeddedInstances(embeddedInstances);

			C_LibretroInstance* pLibretroInstance;
			unsigned int max = embeddedInstances.size();
			for (unsigned int i = 0; i < max; i++)
			{
				pLibretroInstance = dynamic_cast<C_LibretroInstance*>(m_pInputManager->GetEmbeddedInstance());
				if (pLibretroInstance && vgui::input()->IsKeyDown(KEY_XBUTTON_START))
					return true;
			}
		}
	}

	if (m_pInputManager->GetInputMode())
	{
		if (this->GetMetaverseManager()->GetSpawningObjectEntity())
		{
			g_pAnarchyManager->DeactivateObjectPlacementMode(false);

			// undo changes AND cancel
			C_PropShortcutEntity* pShortcut = g_pAnarchyManager->GetMetaverseManager()->GetSpawningObjectEntity();
			g_pAnarchyManager->DeactivateObjectPlacementMode(false);

			//std::string id = pShortcut->GetObjectId();
			//g_pAnarchyManager->GetInstanceManager()->ResetObjectChanges(pShortcut);

			// "save" cha
			//m_pInstanceManager->ApplyChanges(id, pShortcut);
			DevMsg("CHANGES REVERTED\n");
		}
		else
		{
			// handle escape if in fullscreen input mode (drop out of fullscreen mode)
			if ((!m_pInputManager->GetFullscreenMode() || !this->GetSelectedEntity() || m_pInputManager->GetWasForceInputMode()) || (this->GetSelectedEntity() && m_pInputManager->GetFullscreenMode()))
				m_pInputManager->DeactivateInputMode(true);
			else
				m_pInputManager->SetFullscreenMode(false);
		}

		return true;
	}
	else if (!m_pInputManager->GetInputMode() && engine->IsInGame() )
	{
		// handle escape if not in input mode & map is loaded (display the main menu)
		//engine->IsInGame()
		//engine->IsPaused()
		if (!enginevgui->IsGameUIVisible())
		{
			C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");

			//DevMsg("DISPLAY MAIN MENU\n");
			if (m_pSelectedEntity)
				this->DeselectEntity("asset://ui/welcome.html");
			else
				pHudBrowserInstance->SetUrl("asset://ui/welcome.html");

			m_pInputManager->ActivateInputMode(true, true);
			return false;
		}
	}

	return false;
}

bool C_AnarchyManager::HandleCycleToNextWeapon()
{
	if (this->GetMetaverseManager()->GetSpawningObject())
	{
		g_pAnarchyManager->GetInputManager()->OnMouseWheeled(1);
		return true;
	}

	return false;
}

bool C_AnarchyManager::HandleCycleToPrevWeapon()
{
	if (this->GetMetaverseManager()->GetSpawningObject())
	{
		g_pAnarchyManager->GetInputManager()->OnMouseWheeled(-1);
		return true;
	}

	return false;
}

void C_AnarchyManager::Pause()
{
	m_bPaused = true;
}

void C_AnarchyManager::Unpause()
{
	m_bPaused = false;

	m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud")->SetUrl("asset://ui/blank.html");
//	m_pWebManager->GetHudWebTab()->SetUrl("asset://ui/blank.html");
	m_pInputManager->DeactivateInputMode(true);

	if (cvar->FindVar("broadcast_mode")->GetBool())
	{
		// Write this live URL out to the save file.
		std::string XSPlitLiveFolder = "Z:\\scripts";
		FileHandle_t hFile = g_pFullFileSystem->Open(VarArgs("%s\\game.txt", XSPlitLiveFolder.c_str()), "w+", "");

		if (hFile)
		{
			std::string xml = "";
			xml += "<div class=\"response\">\n";
			xml += "\t<activetitle class=\"activetitle\">Anarchy Arcade</activetitle>\n";
			xml += "</div>";

			g_pFullFileSystem->Write(xml.c_str(), xml.length(), hFile);
			g_pFullFileSystem->Close(hFile);
		}

		// Also update a JS file
		hFile = g_pFullFileSystem->Open(VarArgs("%s\\vote.js", XSPlitLiveFolder.c_str()), "a+", "");

		if (hFile)
		{
			std::string code = "gAnarchyTV.OnAArcadeCommand(\"finishPlaying\");\n";
			g_pFullFileSystem->Write(code.c_str(), code.length(), hFile);
			g_pFullFileSystem->Close(hFile);
		}
	}
}

void C_AnarchyManager::BeginImportSteamGames()
{
	if (!this->IsInitialized())
	{
		DevMsg("Not initialized.  Aborting Steam games import.\n");
		return;
	}

	// Scan user profile.
	// 1. Activate input mode.
	// 2. Navigate to the user's games list on their Steam profile in the in-game Steamworks browser.
	// 3. Notify & instruct the user if their profile is set to private, otherwise have an "IMPORT" button appear.
	// 4. Import all games from their list into a KeyValues file ownedGames.key
	// 5. Load all entries from ownedGames.key as items, but do not automatically save them out until the user modifies them.

	this->GetInputManager()->DeactivateInputMode(true);

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
}

size_t ExecuteProcess(std::string FullPathToExe, std::string Parameters)
{
	size_t iMyCounter = 0, iReturnVal = 0, iPos = 0;
	DWORD dwExitCode = 0;
	std::string sTempStr = "";

	/* - NOTE - You should check here to see if the exe even exists */

	/* Add a space to the beginning of the Parameters */
	if (Parameters.size() != 0)
	{
		if (Parameters[0] != L' ')
		{
			Parameters.insert(0, " ");
		}
	}

	/* The first parameter needs to be the exe itself */
	sTempStr = FullPathToExe;
	iPos = sTempStr.find_last_of("/\\");
	sTempStr.erase(0, iPos + 1);
	Parameters = sTempStr.append(Parameters);

	/* CreateProcessW can modify Parameters thus we allocate needed memory */
	char* pwszParam = new char[Parameters.size() + 1];
	if (pwszParam == 0)
	{
		return 1;
	}
	const char* pchrTemp = Parameters.c_str();
	Q_strcpy(pwszParam, pchrTemp);
	//wcscpy_s(pwszParam, Parameters.size() + 1, pchrTemp);

	/* CreateProcess API initialization */
	//STARTUPINFOW siStartupInfo;
	STARTUPINFO siStartupInfo;
	PROCESS_INFORMATION piProcessInfo;
	memset(&siStartupInfo, 0, sizeof(siStartupInfo));
	memset(&piProcessInfo, 0, sizeof(piProcessInfo));
	siStartupInfo.cb = sizeof(siStartupInfo);

	if (CreateProcess(FullPathToExe.c_str(),
		pwszParam, 0, 0, false,
		CREATE_DEFAULT_ERROR_MODE, 0, 0,
		&siStartupInfo, &piProcessInfo) != false)
	{
		/* Watch the process. */
		//dwExitCode = WaitForSingleObject(piProcessInfo.hProcess, (SecondsToWait * 1000));
	}
	else
	{
		/* CreateProcess failed */
		iReturnVal = GetLastError();
	}


	DevMsg("Done\n");
	/* Free memory */
	delete[]pwszParam;
	pwszParam = 0;

	/* Release handles */
	CloseHandle(piProcessInfo.hProcess);
	CloseHandle(piProcessInfo.hThread);

	return iReturnVal;
}

void C_AnarchyManager::ArcadeCreateProcess(std::string executable, std::string executableDirectory, std::string masterCommands)
{

	//DevMsg("Launching Item: \n\tExecutable: %s\n\tDirectory: %s\n\tMaster Commands: %s\n", executable.c_str(), executableDirectory.c_str(), masterCommands.c_str());
	//size_t result = ExecuteProcess(executable, masterCommands);
	//DevMsg("Finished launching item.\n");

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	// set the size of the structures
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	DevMsg("Launching Item: \n\tExecutable: %s\n\tDirectory: %s\n\tMaster Commands: %s\n", executable.c_str(), executableDirectory.c_str(), masterCommands.c_str());

	bool bIsDirectlyExecutable = false;

	/*
	bool bIsDirectlyExecutable = false;

	std::string executableExtensions = "::exe::";
	std::string fileName = executable;
	std::string fileExtension = "";
	size_t found = fileName.find_last_of("/\\");
	if ( found != std::string::npos )
	{
		fileName = fileName.substr(found + 1);

		found = fileName.find_last_of(".");
		if (found != std::string::npos && found < fileName.length() - 1)
		{
			fileExtension = fileName.substr(found + 1);
			fileExtension = "::" + fileExtension + "::";
			//fileName = fileName.substr(0, found);
			if (executableExtensions.find(fileExtension) != std::string::npos)
			{
				if (g_pFullFileSystem->FileExists(executable.c_str()))
				{
					fileName = fileName + " ";
					bIsDirectlyExecutable = true;
					Q_strcpy(pCommands, fileName.c_str());
					DevMsg("Using %s as the filename\n", fileName.c_str());
				}
			}
		}
	}

	if ( !bIsDirectlyExecutable )
		Q_strcpy(pCommands, masterCommands.c_str());
	*/

	if (!bIsDirectlyExecutable && executableDirectory == "" && masterCommands == "")
	{
		// Check for Kodi files
		std::vector<std::string> kodiFileExtensions;
		kodiFileExtensions.push_back(".avi");
		kodiFileExtensions.push_back(".mpg");
		kodiFileExtensions.push_back(".mp4");
		kodiFileExtensions.push_back(".mpeg");
		kodiFileExtensions.push_back(".vob");
		kodiFileExtensions.push_back(".mkv");

		bool bIsKodiFileExtension = false;
		unsigned int length = executable.length();
		unsigned int max = kodiFileExtensions.size();
		for (unsigned int i = 0; i < max; i++)
		{
			if (executable.find(kodiFileExtensions[i]) == length - kodiFileExtensions[i].length())
			{
				bIsKodiFileExtension = true;
				break;
			}
		}

		if (bIsKodiFileExtension)
		{
			DevMsg("Launch option A\n");
			std::string bufLocationString = executable;

			size_t found = bufLocationString.find("\\");
			while (found != std::string::npos)
			{
				bufLocationString[found] = '/';
				found = bufLocationString.find("\\");
			}

			std::string kodiInfo = cvar->FindVar("kodi_info")->GetString();

			C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
			std::vector<std::string> params;
			params.push_back(kodiInfo);
			params.push_back(executable);
			pHudBrowserInstance->DispatchJavaScriptMethod("kodiListener", "play", params);
		}
		else
		{
			DevMsg("Launch option B\n");
			//g_pVGuiSystem->ShellExecuteA("open", executable.c_str());

			// NOW DO THE ACTUAL LAUNCHING STUFF
			FileHandle_t launch_file = filesystem->Open("Arcade_Launcher.bat", "w", "EXECUTABLE_PATH");

			if (!launch_file)
			{
				Msg("Error creating ArcadeLauncher.bat!\n");
				return;
			}

			bool bCommandIsURL = false;

			bool DoNotPause = false;
			std::string goodExecutable = "\"" + executable + "\"";
			filesystem->FPrintf(launch_file, "%s:\n", goodExecutable.substr(1, 1).c_str());
			filesystem->FPrintf(launch_file, "cd \"%s\"\n", goodExecutable.substr(1, goodExecutable.find_last_of("/\\", goodExecutable.find("\"", 1)) - 1).c_str());
			filesystem->FPrintf(launch_file, "START \"Launching item...\" %s", goodExecutable.c_str());
			//filesystem->FPrintf(launch_file, "START \"Launching item...\" %s", masterCommands.c_str());
			filesystem->Close(launch_file);
			system("Arcade_Launcher.bat");
		}
	}
	else
	{
		DevMsg("Launch option C\n");

		char pCommands[AA_MAX_STRING];
		Q_strcpy(pCommands, masterCommands.c_str());

		// start the program up
		CreateProcess(executable.c_str(),   // the path
			pCommands,        // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			0,//CREATE_DEFAULT_ERROR_MODE,              //0 // No creation flags
			NULL,           // Use parent's environment block
			executableDirectory.c_str(),           // Use parent's starting directory 
			&si,            // Pointer to STARTUPINFO structure
			&pi);           // Pointer to PROCESS_INFORMATION structure
	}
	DevMsg("Finished launching item.\n");
}

void C_AnarchyManager::RunAArcade()
{
	if (!this->IsInitialized())
	{
		C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
		pHudBrowserInstance->SetUrl("asset://ui/startup.html");
	}
	else
	{
		//this->GetInputManager()->DeactivateInputMode(true);

		C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
		pHudBrowserInstance->SetUrl("asset://ui/welcome.html");
		this->GetInputManager()->ActivateInputMode(true, true, pHudBrowserInstance);

		m_bSuspendEmbedded = false;
			//m_bPaused = false;

		//C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
		//pHudBrowserInstance->SetUrl("asset://ui/welcome.html");
		/*
		C_EmbeddedInstance* pSelectedEmbeddedInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
		if (pSelectedEmbeddedInstance)
		{

			g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, pSelectedEmbeddedInstance);
			pHudBrowserInstance->SetUrl("asset://ui/welcome.html");
		}
		else
		{
			C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
			pHudBrowserInstance->SetUrl("asset://ui/welcome.html");

			this->GetInputManager()->ActivateInputMode(true, true, pHudBrowserInstance);
		}
		*/
	}
}

void C_AnarchyManager::HudStateNotify()
{
	std::vector<std::string> params;

	// isFullscreen
	params.push_back(VarArgs("%i", (g_pAnarchyManager->GetInputManager()->GetFullscreenMode())));

	// isHudPinned
	params.push_back(VarArgs("%i", (g_pAnarchyManager->GetInputManager()->GetWasForceInputMode())));

	// isMapLoaded
	params.push_back(VarArgs("%i", engine->IsInGame()));

	// isObjectSelected (any object)
	params.push_back(VarArgs("%i", (g_pAnarchyManager->GetSelectedEntity() != null)));

	// isItemSelected (any item)
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

	// isMainMenu
	params.push_back(VarArgs("%i", (g_pAnarchyManager->GetInputManager()->GetMainMenuMode())));

	// url
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

	// isSelectedObject
	if (pShortcut && pEmbeddedInstance && pEmbeddedInstance->GetOriginalEntIndex() == pShortcut->entindex())
		params.push_back("1");
	else
		params.push_back("0");

	// embeddedInstanceType
	std::string embeddedType = "Unknown";
	if (pEmbeddedInstance)
	{
		C_LibretroInstance* pLibretroInstance = dynamic_cast<C_LibretroInstance*>(pEmbeddedInstance);
		if (pLibretroInstance)
			embeddedType = "Libretro";

		C_SteamBrowserInstance* pSteamBrowserInstance = dynamic_cast<C_SteamBrowserInstance*>(pEmbeddedInstance);
		if (pSteamBrowserInstance)
			embeddedType = "SteamworksBrowser";

		C_AwesomiumBrowserInstance* pAwesomiumBrowserInstance = dynamic_cast<C_AwesomiumBrowserInstance*>(pEmbeddedInstance);
		if (pAwesomiumBrowserInstance)
			embeddedType = "AwesomiumBrowser";
	}

	params.push_back(embeddedType);
	
	pHudBrowserInstance->DispatchJavaScriptMethod("arcadeHud", "onActivateInputMode", params);
}

void C_AnarchyManager::ShowTaskMenu()
{
	if (g_pAnarchyManager->GetSelectedEntity())
		g_pAnarchyManager->TaskRemember();
	//g_pAnarchyManager->DeselectEntity();

	//if (!enginevgui->IsGameUIVisible())
	//{
		C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
		pHudBrowserInstance->SetUrl("asset://ui/taskMenu.html");
		m_pInputManager->ActivateInputMode(true, true);
	//}
}

void C_AnarchyManager::HideTaskMenu()
{
	m_pInputManager->DeactivateInputMode(true);
}

void C_AnarchyManager::TaskRemember()
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

void C_AnarchyManager::SetSlaveScreen(bool bVal)
{
	C_PropShortcutEntity* pShortcut = null;
	C_BaseEntity* pEntity = this->GetSelectedEntity();
	if (pEntity)
		pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);

	if (pShortcut)
	{
		std::string objectId = pShortcut->GetObjectId();
		object_t* pObject = this->GetInstanceManager()->GetInstanceObject(objectId);
		if (pObject)
		{
			//pObject->slave = bVal;
			pShortcut->SetSlave(bVal);
			this->GetInstanceManager()->ApplyChanges(objectId, pShortcut);	// will also update the object
		}
	}
}

void C_AnarchyManager::PostRender()
{
	//DevMsg("AnarchyManager: PostRender\n");
}

// Over 15x faster than: (int)floor(value)
/*
inline int Floor2Int(float a)
{
	int RetVal;
#if defined( __i386__ )
	// Convert to int and back, compare, subtract one if too big
	__m128 a128 = _mm_set_ss(a);
	RetVal = _mm_cvtss_si32(a128);
	__m128 rounded128 = _mm_cvt_si2ss(_mm_setzero_ps(), RetVal);
	RetVal -= _mm_comigt_ss(rounded128, a128);
#else
	RetVal = static_cast<int>(floor(a));
#endif
	return RetVal;
}
*/
#include <chrono>
void C_AnarchyManager::GenerateUniqueId(char* result)
{
	/*
	DevMsg("AnarchyManager: GenerateUniqueId\n");

	// pseudo random pseudo unique ids until the firebase id generator can be ported to C++
	std::string id = "random";

	id += std::to_string(random->RandomInt(0, 10));
	id += std::to_string(random->RandomInt(0, 10));
	id += std::to_string(random->RandomInt(0, 10));
	id += std::to_string(random->RandomInt(0, 10));

	return id;
	*/

	std::string PUSH_CHARS = "-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";

	//double now = vgui::system()->GetCurrentTime();
	//using namespace std::chrono;
	double now = std::chrono::system_clock::now().time_since_epoch().count();//GetCurrentTime();// vgui::system()->GetCurrentTime();
	now = floor(now / 64.0);
	now = floor(now / 64.0);

	//DevMsg("Time now: %lf\n", now);

	bool duplicateTime = (now == m_dLastGenerateIdTime);
	m_dLastGenerateIdTime = now;

	//char* timeStampChars[8];
	std::string timeStampChars = "00000000";
	for (unsigned int i = 8; i > 0; i--)
	{
		timeStampChars.replace(i - 1, 1, 1, PUSH_CHARS.at(fmod(now, 64.0)));
		// NOTE: Can't use << here because javascript will convert to int and lose the upper bits.
		//		if (now >= 64.0)
		now = floor(now / 64.0);
		//	else
		//	now = 0;
	}

	if (now != 0)
	{
		DevMsg("ERROR: We should have converted the entire timestamp. %f\n", now);
	}

	std::string id = timeStampChars;
	//bool bCharsExist = (m_lastGeneratedChars != "");
	if (!duplicateTime)
	{
		for (unsigned int i = 0; i < 12; i++)
		{
			//			if (bCharsExist)
			m_lastGeneratedChars.replace(i, 1, 1, (char)floor(random->RandomFloat() * 64.0L));
			//		else
			//		m_lastGeneratedChars += VarArgs("%c", (char)floor(random->RandomFloat() * 64.0L));
		}
	}
	else
	{
		// If the timestamp hasn't changed since last push, use the same random number, except incremented by 1.
		unsigned int i;
		for (i = 11; i >= 0 && m_lastGeneratedChars.at(i) == 63; i--)
			m_lastGeneratedChars.replace(i, 1, 1, (char)0);

		m_lastGeneratedChars.replace(i, 1, 1, (char)(m_lastGeneratedChars.at(i) + 1));
	}

	for (unsigned int i = 0; i < 12; i++)
	{
		id += PUSH_CHARS.at(m_lastGeneratedChars.at(i));
	}

	if (id.length() != 20)
		DevMsg("ERROR: Lngth should be 20.\n");

	Q_strcpy(result, id.c_str());

	//return VarArgs("%s", id.c_str());	// works on instance menu	// GETS ALL CALLS DURING SAME TICK
	return;// id.c_str();	// works on library browser menu	// GETS CALLS ON DIFFERENT TICKS
}

const char* C_AnarchyManager::GenerateUniqueId()
{
	/*
	DevMsg("AnarchyManager: GenerateUniqueId\n");

	// pseudo random pseudo unique ids until the firebase id generator can be ported to C++
	std::string id = "random";

	id += std::to_string(random->RandomInt(0, 10));
	id += std::to_string(random->RandomInt(0, 10));
	id += std::to_string(random->RandomInt(0, 10));
	id += std::to_string(random->RandomInt(0, 10));

	return id;
	*/

	std::string PUSH_CHARS = "-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";
	
	//double now = vgui::system()->GetCurrentTime();
	//using namespace std::chrono;
	double now = std::chrono::system_clock::now().time_since_epoch().count();//GetCurrentTime();// vgui::system()->GetCurrentTime();
	now = floor(now / 64.0);
	now = floor(now / 64.0);

	//DevMsg("Time now: %lf\n", now);

	bool duplicateTime = (now == m_dLastGenerateIdTime);
	m_dLastGenerateIdTime = now;

	//char* timeStampChars[8];
	std::string timeStampChars = "00000000";
	for (unsigned int i = 8; i > 0; i--)
	{
		timeStampChars.replace(i-1, 1, 1, PUSH_CHARS.at(fmod(now, 64.0)));
		// NOTE: Can't use << here because javascript will convert to int and lose the upper bits.
//		if (now >= 64.0)
			now = floor(now / 64.0);
	//	else
		//	now = 0;
	}

	if (now != 0)
	{
		DevMsg("ERROR: We should have converted the entire timestamp. %f\n", now);
	}

	std::string id = timeStampChars;
	//bool bCharsExist = (m_lastGeneratedChars != "");
	if (!duplicateTime)
	{
		for (unsigned int i = 0; i < 12; i++)
		{
//			if (bCharsExist)
				m_lastGeneratedChars.replace(i, 1, 1, (char)floor(random->RandomFloat() * 64.0L));
	//		else
		//		m_lastGeneratedChars += VarArgs("%c", (char)floor(random->RandomFloat() * 64.0L));
		}
	}
	else
	{
		// If the timestamp hasn't changed since last push, use the same random number, except incremented by 1.
		unsigned int i;
		for (i = 11; i >= 0 && m_lastGeneratedChars.at(i) == 63; i--)
			m_lastGeneratedChars.replace(i, 1, 1, (char)0);

		m_lastGeneratedChars.replace(i, 1, 1, (char)(m_lastGeneratedChars.at(i) + 1));
	}

	for (unsigned int i = 0; i < 12; i++)
	{
		id += PUSH_CHARS.at(m_lastGeneratedChars.at(i));
	}

	if (id.length() != 20)
		DevMsg("ERROR: Lngth should be 20.\n");

	//Q_strcpy(result, id.c_str());

	return VarArgs("%s", id.c_str());	// works on instance menu	// GETS ALL CALLS DURING SAME TICK
	//return id.c_str();	// works on library browser menu	// GETS CALLS ON DIFFERENT TICKS
}

const char* C_AnarchyManager::GenerateUniqueId2()
{
	/*
	DevMsg("AnarchyManager: GenerateUniqueId\n");

	// pseudo random pseudo unique ids until the firebase id generator can be ported to C++
	std::string id = "random";

	id += std::to_string(random->RandomInt(0, 10));
	id += std::to_string(random->RandomInt(0, 10));
	id += std::to_string(random->RandomInt(0, 10));
	id += std::to_string(random->RandomInt(0, 10));

	return id;
	*/

	std::string PUSH_CHARS = "-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";

	//double now = vgui::system()->GetCurrentTime();
	//using namespace std::chrono;
	double now = std::chrono::system_clock::now().time_since_epoch().count();//GetCurrentTime();// vgui::system()->GetCurrentTime();
	now = floor(now / 64.0);
	now = floor(now / 64.0);

	//DevMsg("Time now: %lf\n", now);

	bool duplicateTime = (now == m_dLastGenerateIdTime);
	m_dLastGenerateIdTime = now;

	//char* timeStampChars[8];
	std::string timeStampChars = "00000000";
	for (unsigned int i = 8; i > 0; i--)
	{
		timeStampChars.replace(i - 1, 1, 1, PUSH_CHARS.at(fmod(now, 64.0)));
		// NOTE: Can't use << here because javascript will convert to int and lose the upper bits.
		//		if (now >= 64.0)
		now = floor(now / 64.0);
		//	else
		//	now = 0;
	}

	if (now != 0)
	{
		DevMsg("ERROR: We should have converted the entire timestamp. %f\n", now);
	}

	std::string id = timeStampChars;
	//bool bCharsExist = (m_lastGeneratedChars != "");
	if (!duplicateTime)
	{
		for (unsigned int i = 0; i < 12; i++)
		{
			//			if (bCharsExist)
			m_lastGeneratedChars.replace(i, 1, 1, (char)floor(random->RandomFloat() * 64.0L));
			//		else
			//		m_lastGeneratedChars += VarArgs("%c", (char)floor(random->RandomFloat() * 64.0L));
		}
	}
	else
	{
		// If the timestamp hasn't changed since last push, use the same random number, except incremented by 1.
		unsigned int i;
		for (i = 11; i >= 0 && m_lastGeneratedChars.at(i) == 63; i--)
			m_lastGeneratedChars.replace(i, 1, 1, (char)0);

		m_lastGeneratedChars.replace(i, 1, 1, (char)(m_lastGeneratedChars.at(i) + 1));
	}

	for (unsigned int i = 0; i < 12; i++)
	{
		id += PUSH_CHARS.at(m_lastGeneratedChars.at(i));
	}

	if (id.length() != 20)
		DevMsg("ERROR: Lngth should be 20.\n");

	//Q_strcpy(result, id.c_str());

	//return VarArgs("%s", id.c_str());	// works on instance menu	// GETS ALL CALLS DURING SAME TICK
	return id.c_str();	// works on library browser menu	// GETS CALLS ON DIFFERENT TICKS
}

std::string C_AnarchyManager::ExtractLegacyId(std::string itemFile, KeyValues* item)
{
	std::string alphabet = "0123456789abcdef";

	std::string nameSnip = "";
	bool bPassed = true;

	size_t found = itemFile.find(":");
	if (found != std::string::npos)
		bPassed = false;

	if (bPassed)
	{
		found = itemFile.find_last_of("/\\");
		if (found == std::string::npos)
			bPassed = false;
	}

	if (bPassed)
	{
		nameSnip = itemFile.substr(found + 1);
		found = nameSnip.find_first_of(".");

		if (found == std::string::npos)
			bPassed = false;
		else
		{
			nameSnip = nameSnip.substr(0, found);

			unsigned int nameSnipLength = nameSnip.length();
			if (nameSnipLength != 8)
				bPassed = false;
			else
			{
				unsigned int i;
				for (i = 0; i < nameSnipLength; i++)
				{
					found = alphabet.find(nameSnip[i]);
					if (found == std::string::npos)
					{
						bPassed = false;
						break;
					}
				}
			}
		}
	}
	
	//// generate a legacy ID based on the filelocation if given an item to work with
//	if (!bPassed && item)
	//	nameSnip = this->GenerateLegacyHash(item->GetString("filelocation"));

	if (!bPassed)
		nameSnip = "";

	return nameSnip;
}

const char* C_AnarchyManager::GenerateCRC32Hash(const char* text)
{
	char protectedHash[9];

	CRC32_t protectedCRC = CRC32_ProcessSingleBuffer((void*)text, strlen(text));
	Q_snprintf(protectedHash, 9, "%08x", protectedCRC);

	return VarArgs("%s", protectedHash);
}

const char* C_AnarchyManager::GenerateLegacyHash(const char* text)
{
	char input[AA_MAX_STRING];
	Q_strcpy(input, text);

	// Convert it to lowercase & change all slashes to back-slashes
	V_FixSlashes(input);
	for (int i = 0; input[i] != '\0'; i++)
		input[i] = tolower(input[i]);

	char lower[AA_MAX_STRING];
	unsigned m_crc = 0xffffffff;

	int inputLength = strlen(input);
	for (int i = 0; i < inputLength; i++)
	{
		lower[i] = tolower(input[i]);
	}

	for (int i = 0; i < inputLength; i++)
	{
		unsigned c = lower[i];
		m_crc ^= (c << 24);

		for (int j = 0; j < 8; j++)
		{
			const unsigned FLAG = 0x80000000;
			if ((m_crc & FLAG) == FLAG)
			{
				m_crc = (m_crc << 1) ^ 0x04C11DB7;
			}
			else
			{
				m_crc <<= 1;
			}
		}
	}

	return VarArgs("%08x", m_crc);
}

/*
var PUSH_CHARS = '-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz';

var now = new Date().getTime();
var duplicateTime = (now === this.lastPushTime);
this.lastPushTime = now;

var timeStampChars = new Array(8);
for (var i = 7; i >= 0; i--) {
timeStampChars[i] = PUSH_CHARS.charAt(now % 64);
// NOTE: Can't use << here because javascript will convert to int and lose the upper bits.
now = Math.floor(now / 64);
}
if (now !== 0) throw new Error('We should have converted the entire timestamp.');

var id = timeStampChars.join('');

if (!duplicateTime) {
for (i = 0; i < 12; i++) {
this.lastRandChars[i] = Math.floor(Math.random() * 64);
}
} else {
// If the timestamp hasn't changed since last push, use the same random number, except incremented by 1.
for (i = 11; i >= 0 && this.lastRandChars[i] === 63; i--) {
this.lastRandChars[i] = 0;
}
this.lastRandChars[i]++;
}
for (i = 0; i < 12; i++) {
id += PUSH_CHARS.charAt(this.lastRandChars[i]);
}
if(id.length != 20) throw new Error('Length should be 20.');

return id;
*/

void C_AnarchyManager::Disconnect()
{
	// called when a player clicks the LEAVE button on the main menu.  However, there are other ways they could disconnect w/o clicking that.
	engine->ClientCmd("disconnect;\n");
}

void C_AnarchyManager::AnarchyStartup()
{
	DevMsg("AnarchyManager: AnarchyStartup\n");
	m_bIncrementState = true;
	//m_state = AASTATE_INPUTMANAGER;
	/*
	m_pInstanceManager = new C_InstanceManager();
	m_pMetaverseManager = new C_MetaverseManager();
	m_pInputManager = new C_InputManager();
	m_pWebManager = new C_WebManager();
	m_pWebManager->Init();
	*/
}

/*
void C_AnarchyManager::OnWebManagerReady()
{
	C_WebTab* pHudWebTab = m_pWebManager->GetHudWebTab();
	g_pAnarchyManager->GetWebManager()->SelectWebTab(pHudWebTab);
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true);

	unsigned int uCount;
	std::string num;
	
	// And continue starting up
	uCount = m_pMetaverseManager->LoadAllLocalTypes();
	num = VarArgs("%u", uCount);
	pHudWebTab->AddHudLoadingMessage("progress", "", "Loading Types", "locallibrarytypes", "0", num, num);

	 //= m_pMetaverseManager->LoadAllLocalTypes();
	//std::string num = VarArgs("%u", uItemCount);
//	pHudWebTab->AddHudLoadingMessage("progress", "", "Loading Types", "locallibrarytypes", "0", num, num);

	uCount = m_pMetaverseManager->LoadAllLocalModels();
	num = VarArgs("%u", uCount);
	pHudWebTab->AddHudLoadingMessage("progress", "", "Loading Models", "locallibrarymodels", "0", num, num);
	
	//uItemCount = m_pMetaverseManager->LoadAllLocalApps();

	// load ALL local apps
	KeyValues* app = m_pMetaverseManager->LoadFirstLocalApp("MOD");
	if (app)
		pHudWebTab->AddHudLoadingMessage("progress", "", "Loading Apps", "locallibraryapps", "", "", "+", "loadNextLocalAppCallback");
	else
		this->OnLoadAllLocalAppsComplete();

}
*/

void C_AnarchyManager::OnLoadAllLocalAppsComplete()
{
	/*
	m_pMountManager = new C_MountManager();
	m_pMountManager->Init();
	m_pMountManager->LoadMountsFromKeyValues("mounts.txt");
	*/

	// add legacy search paths
	if (m_legacyFolder != "")
	{
		//std::string path = this->GetLegacyFolder();
		g_pFullFileSystem->AddSearchPath(m_legacyFolder.c_str(), "MOD", PATH_ADD_TO_TAIL);
		g_pFullFileSystem->AddSearchPath(m_legacyFolder.c_str(), "GAME", PATH_ADD_TO_TAIL);

		// and legacy workshop search paths (from workshop folder)
		std::string workshopMapsPath = m_legacyFolder + std::string("workshop\\workshopmaps\\");
		//g_pFullFileSystem->AddSearchPath(workshopMapsPath.c_str(), "MOD", PATH_ADD_TO_TAIL);
		g_pFullFileSystem->AddSearchPath(workshopMapsPath.c_str(), "GAME", PATH_ADD_TO_TAIL);

		std::string workshopFile;
		FileFindHandle_t findHandle;
		const char *pFilename = g_pFullFileSystem->FindFirstEx(VarArgs("%sworkshop\\*", m_legacyFolder.c_str()), "", &findHandle);
		while (pFilename != NULL)
		{
			workshopFile = m_legacyFolder + std::string("workshop\\") + std::string(pFilename);

			if (workshopFile.find(".vpk") == workshopFile.length() - 4) //g_pFullFileSystem->FindIsDirectory(findHandle)
			{
				DevMsg("Adding %s to the search paths.\n", workshopFile.c_str());
				//g_pFullFileSystem->AddSearchPath(workshopFile.c_str(), "MOD", PATH_ADD_TO_TAIL);
				g_pFullFileSystem->AddSearchPath(workshopFile.c_str(), "GAME", PATH_ADD_TO_TAIL);
			}

			pFilename = g_pFullFileSystem->FindNext(findHandle);
		}
	}
	/*
	KeyValues* pLegacyLogKV = new KeyValues("legacy");
	if (pLegacyLogKV->LoadFromFile(g_pFullFileSystem, "legacy_log.key", "DEFAULT_WRITE_PATH"))
	{
		if (pLegacyLogKV->GetInt("exported"))
		{
			std::string path = this->GetLegacyFolder();
			g_pFullFileSystem->AddSearchPath(path.c_str(), "MOD", PATH_ADD_TO_TAIL);
			g_pFullFileSystem->AddSearchPath(path.c_str(), "GAME", PATH_ADD_TO_TAIL);

			// and legacy workshop search paths (from workshop folder)
			std::string workshopMapsPath = path + "workshop\\workshopmaps\\";
			//g_pFullFileSystem->AddSearchPath(workshopMapsPath.c_str(), "MOD", PATH_ADD_TO_TAIL);
			g_pFullFileSystem->AddSearchPath(workshopMapsPath.c_str(), "GAME", PATH_ADD_TO_TAIL);

			std::string workshopFile;
			FileFindHandle_t findHandle;
			const char *pFilename = g_pFullFileSystem->FindFirstEx(VarArgs("%sworkshop\\*", path.c_str()), "", &findHandle);
			while (pFilename != NULL)
			{
				workshopFile = path + "workshop\\" + std::string(pFilename);

				if (workshopFile.find(".vpk") == workshopFile.length() - 4) //g_pFullFileSystem->FindIsDirectory(findHandle)
				{
					DevMsg("Adding %s to the search paths.\n", workshopFile.c_str());
					//g_pFullFileSystem->AddSearchPath(workshopFile.c_str(), "MOD", PATH_ADD_TO_TAIL);
					g_pFullFileSystem->AddSearchPath(workshopFile.c_str(), "GAME", PATH_ADD_TO_TAIL);
					//g_pAnarchyManager->GetMetaverseManager()->LoadFirstLocalItemLegacy(true, workshopFile, "", "");
				}

				pFilename = g_pFullFileSystem->FindNext(findHandle);
			}
		}
	}
	pLegacyLogKV->deleteThis();
	*/

	/* DISABLE MOUNTING OF LEGACY AARCADE CONTENT
	std::string pathDownload = "A:\\SteamLibrary\\steamapps\\common\\Anarchy Arcade\\aarcade\\download\\";	// for resolving cached images
	g_pFullFileSystem->AddSearchPath(path.c_str(), "MOD", PATH_ADD_TO_TAIL);
	g_pFullFileSystem->AddSearchPath(path.c_str(), "GAME", PATH_ADD_TO_TAIL);
	g_pAnarchyManager->GetMetaverseManager()->LoadFirstLocalItemLegacy(true, path, "", "");
	*/

	// INSTEAD, do the following:
	C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
	unsigned int uCount = g_pAnarchyManager->GetMetaverseManager()->LoadAllLocalItems();
	std::string num = VarArgs("%u", uCount);
	pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading Items", "locallibraryitems", "0", num, num);

	// AND THIS IS WHAT USUALLY GETS CALLED AFTER LOADFIRSTLOCALITEMLEGACY IS FINISHED:
	//g_pAnarchyManager->GetMetaverseManager()->LoadFirstLocalItemLegacy(true, "", "", "");



	//g_pAnarchyManager->GetMetaverseManager()->SetPreviousLocaLocalItemLegacyWorkshopIds("dummy");
	//g_pAnarchyManager->GetWorkshopManager()->OnMountWorkshopSucceed();
	this->OnMountAllWorkshopsComplete();	// the first time this is called initializes it all





	//C_AwesomiumBrowserInstance* pHudBrowserInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	//pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Mounting Workshop Subscriptions", "mountworkshops", "", "0", "+", "mountNextWorkshopCallback");
}

bool C_AnarchyManager::AttemptSelectEntity()
{
	if (!g_pAnarchyManager->IsInitialized())
		return false;

	C_PropShortcutEntity* pShortcut = g_pAnarchyManager->GetMetaverseManager()->GetSpawningObjectEntity();
	if (pShortcut)
	{
		// finished positioning & choosing model, ie: changes confirmed
		DevMsg("CHANGES CONFIRMED\n");
		g_pAnarchyManager->DeactivateObjectPlacementMode(true);

		std::string id = pShortcut->GetObjectId();
		m_pInstanceManager->ApplyChanges(id, pShortcut);

		return SelectEntity(pShortcut);
	}
	else
	{
		C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
		if (!pPlayer)
			return false;

		if (pPlayer->GetHealth() <= 0)
			return false;

		// fire a trace line
		trace_t tr;
		Vector forward;
		pPlayer->EyeVectors(&forward);
		UTIL_TraceLine(pPlayer->EyePosition(), pPlayer->EyePosition() + forward * MAX_COORD_RANGE, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

		C_BaseEntity *pEntity = tr.m_pEnt;

		// only allow prop shortcuts
		if (pEntity)
			pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);

		if (pShortcut && tr.fraction != 1.0 && tr.DidHitNonWorldEntity())
		{
			pEntity = tr.m_pEnt;

			if (m_pSelectedEntity && pEntity == m_pSelectedEntity)
			{
				//m_pInputManager->SetFullscreenMode(true);
				C_EmbeddedInstance* pEmbeddedInstance = m_pInputManager->GetEmbeddedInstance();
				m_pInputManager->ActivateInputMode(true, m_pInputManager->GetMainMenuMode(), pEmbeddedInstance);
			}
			else
				return SelectEntity(pEntity);
		}
		else
		{
			if (m_pSelectedEntity)
				return DeselectEntity();
			else
				return false;
		}
	}

	return false;
}

// from http://www.zedwood.com/article/cpp-urlencode-function
#include <iostream>
#include <sstream>
std::string C_AnarchyManager::encodeURIComponent(const std::string &s)
{
	static const char lookup[] = "0123456789abcdef";
	std::stringstream e;
	for (int i = 0, ix = s.length(); i<ix; i++)
	{
		const char& c = s[i];
		if ((48 <= c && c <= 57) ||//0-9
			(65 <= c && c <= 90) ||//abc...xyz
			(97 <= c && c <= 122) || //ABC...XYZ
			(c == '-' || c == '_' || c == '.' || c == '~')
			)
		{
			e << c;
		}
		else
		{
			e << '%';
			e << lookup[(c & 0xF0) >> 4];
			e << lookup[(c & 0x0F)];
		}
	}
	return e.str();
}

bool C_AnarchyManager::SelectEntity(C_BaseEntity* pEntity)
{
//	DevMsg("DISABLED FOR TESTING!\n");
//	return true;
//	/*
	if (m_pSelectedEntity)
		DeselectEntity();

//	m_pWebManager->GetHudWebTab()->SetUrl("asset://ui/blank.html");

	m_pSelectedEntity = pEntity;
	AddGlowEffect(pEntity);

	

		//pMaterials[x]->ColorModulate(255, 0, 0);
	//pMaterials[x]->GetPreviewImage

	std::string itemId;
	std::string tabTitle;
	std::string uri;
	KeyValues* item;
	KeyValues* active;
	//C_PropShortcutEntity* pShortcut;
	//C_WebTab* pWebTab;
	//C_WebTab* pSelectedWebTab;
	C_EmbeddedInstance* pEmbeddedInstance;
	C_EmbeddedInstance* pSelectedEmbeddedInstance;

	C_PropShortcutEntity* pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);
	std::vector<C_EmbeddedInstance*> embeddedInstances;
	pShortcut->GetEmbeddedInstances(embeddedInstances);

	unsigned int i;
	unsigned int size = embeddedInstances.size();
	for (i = 0; i < size; i++)
	{
		pEmbeddedInstance = embeddedInstances[i];
		if (!pEmbeddedInstance)
			continue;

		if (pEmbeddedInstance->GetId() == "hud")
			continue;

		bool bImagesAndHandled = false;
		if (pEmbeddedInstance->GetId() == "images")
		{
			pShortcut = dynamic_cast<C_PropShortcutEntity*>(m_pSelectedEntity);
			if (pShortcut)
			{
				tabTitle = "auto" + pShortcut->GetItemId();
				pEmbeddedInstance = m_pCanvasManager->FindEmbeddedInstance(tabTitle);// this->GetWebManager()->FindWebTab(tabTitle);
				if (!pEmbeddedInstance)
				{
					itemId = pShortcut->GetItemId();
					item = m_pMetaverseManager->GetLibraryItem(itemId);
					if (item)
					{
						active = item->FindKey("current");
						if (!active)
							active = item->FindKey("local", true);

						/*
						std::string uri = "asset://ui/autoInspectItem.html?id=" + encodeURIComponent(itemId) + "&screen=" + encodeURIComponent(active->GetString("screen")) + "&marquee=" + encodeURIComponent(active->GetString("marquee")) + "&preview=" + encodeURIComponent(active->GetString("preview")) + "&reference=" + encodeURIComponent(active->GetString("reference")) + "&file=" + encodeURIComponent(active->GetString("file"));
						WebURL url = WebURL(WSLit(uri.c_str()));
						*/

						//std::string dumbUrl = "http://smarcade.net/dlcv2/view_youtube.php?id=";
						//std::string dumbUrl = active->GetString("file");

						// If this is a video file, play it in libretro instead of the browser
						bool bDoAutoInspect = true;
						if (m_pLibretroManager->GetInstanceCount() == 0)
						{
							std::string coreFile = "";
							std::string file = "";

							// check for compatible files for ffmpeg
							if (coreFile == "")
							{
								std::string exts = "::mpg::mpeg::avi::mp4::mkv::";
								std::string fileExt = active->GetString("file");

								size_t found = fileExt.find_last_of(".");
								if (found != std::string::npos)
									fileExt = fileExt.substr(found + 1);

								found = exts.find("::" + fileExt + "::");
								if (found != std::string::npos )
									coreFile = "ffmpeg_libretro.dll";
							}
							/* DISABLED UNTIL EVERYTHING IS READY TO GO WITH LIBRETRO SUPPORT
							// check for compatible types for libretro cores.
							if (coreFile == "")
							{
								std::map<std::string, std::string> typeToLibretroCore;
								typeToLibretroCore["snes"] = "snes9x_libretro.dll";
								//typeToLibretroCore["nes"] = "bsnes_performance_libretro.dll";
								typeToLibretroCore["nes"] = "fceumm_libretro.dll";
								typeToLibretroCore["arcade"] = "mame_libretro.dll";
								typeToLibretroCore["gameboy advance"] = "vba_next_libretro.dll";
								typeToLibretroCore["gameboy color"] = "vba_next_libretro.dll";
								typeToLibretroCore["gameboy"] = "vba_next_libretro.dll";
								typeToLibretroCore["gba"] = "vba_next_libretro.dll";
								typeToLibretroCore["gbc"] = "vba_next_libretro.dll";
								typeToLibretroCore["gb"] = "vba_next_libretro.dll";

								KeyValues* type = m_pMetaverseManager->GetLibraryType(active->GetString("type"));
								if (type)
								{
									KeyValues* activeType = type->FindKey("current");
									if (!activeType)
										activeType = type->FindKey("local", true);

									std::string typeTitle = activeType->GetString("title");
									if (typeTitle != "")
									{
										// convert to lowercase
										int len = typeTitle.length() + 1;
										char* pTemp = (char*)stackalloc(len);
										Q_strncpy(pTemp, typeTitle.c_str(), len);
										Q_strnlwr(pTemp, len);
										typeTitle = pTemp;
										stackfree(pTemp);

										// loop through the user's types looking for literal title matches (because type ID's could be different for every user)
										auto it = typeToLibretroCore.begin();
										//size_t foundTypeTitle;
										while (it != typeToLibretroCore.end())
										{
											//foundTypeTitle = typeTitle.find(it->first);
											//if (foundTypeTitle != std::string::npos)
											if ( typeTitle == it->first )
											{
												coreFile = it->second;
												break;
											}

											it++;
										}
									}
								}
							}
							*/

							// use the core, if one is found
							if (coreFile != "" )//&& g_pFullFileSystem->FileExists(active->GetString("file")))
							{
								file = active->GetString("file");

								bool bFileIsGood = false;
								// is the path relative to the app?
								size_t found = file.find(":");
								if (found == 1 && g_pFullFileSystem->FileExists(file.c_str()))
								{
									bFileIsGood = true;
								}
								else
								{
									// does it have an app?
									if (Q_strcmp(active->GetString("app"), ""))
									{
										bool bHasApp = true;
										KeyValues* app = g_pAnarchyManager->GetMetaverseManager()->GetLibraryApp(active->GetString("app"));
										KeyValues* appActive = app->FindKey("current");
										if (!appActive)
											appActive = app->FindKey("local", true);

										if (appActive)
										{
											bool bHasAppFilepath = false;
											bool bAtLeastOneAppFilepathExists = false;

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
														file = testFile;
														bFileIsGood = true;
														break;
													}
												}
											}
										}
									}
								}

								if (bFileIsGood)
								{
									C_LibretroInstance* pLibretroInstance = m_pLibretroManager->CreateLibretroInstance();
									pLibretroInstance->Init(tabTitle);
									DevMsg("Setting game to: %s\n", file.c_str());
									pLibretroInstance->SetOriginalGame(file);
									pLibretroInstance->SetOriginalItemId(itemId);
									if (!pLibretroInstance->LoadCore(coreFile))	// FIXME: elegantly revert back to autoInspect if loading the core failed!
										DevMsg("ERROR: Failed to load core: %s\n", coreFile.c_str());
									pEmbeddedInstance = pLibretroInstance;
									bDoAutoInspect = false;
								}
							}
						}

						if ( bDoAutoInspect)
						{
							std::string uri = "file://";
							uri += engine->GetGameDirectory();
							uri += "/resource/ui/html/autoInspectItem.html?id=" + encodeURIComponent(itemId) + "&screen=" + encodeURIComponent(active->GetString("screen")) + "&marquee=" + encodeURIComponent(active->GetString("marquee")) + "&preview=" + encodeURIComponent(active->GetString("preview")) + "&reference=" + encodeURIComponent(active->GetString("reference")) + "&file=" + encodeURIComponent(active->GetString("file"));

							DevMsg("Test URI is: %s\n", uri.c_str());	// FIXME: Might want to make the slashes in the game path go foward.  Also, need to allow HTTP redirection (302).

							C_SteamBrowserInstance* pSteamBrowserInstance = m_pSteamBrowserManager->CreateSteamBrowserInstance();
							pSteamBrowserInstance->Init(tabTitle, uri, null, pShortcut->entindex());
							pSteamBrowserInstance->SetOriginalItemId(itemId);	// FIXME: do we need to do this for original entindex too???
							//pSteamBrowserInstance->SetOriginalEntIndex(pShortcut->entindex());	// probably NOT needed??


							pEmbeddedInstance = pSteamBrowserInstance;
						}
					}
					else
					{
						// the item specified by the shortcut was not found
						// by doing NOTHING, it lets you select the object but not bring up any menus on it
					}
				}
			}
		}
		else
		{
			pSelectedEmbeddedInstance = m_pInputManager->GetEmbeddedInstance();// m_pWebManager->GetSelectedWebTab();
			if (pSelectedEmbeddedInstance)
			{
				pSelectedEmbeddedInstance->Deselect();
				m_pInputManager->SetEmbeddedInstance(null);
				//m_pWebManager->DeselectWebTab(pSelectedEmbeddedInstance);
			}
		}

		if (pEmbeddedInstance)
		{
			pSelectedEmbeddedInstance = m_pInputManager->GetEmbeddedInstance();
			if (pSelectedEmbeddedInstance && pSelectedEmbeddedInstance != pEmbeddedInstance)
			{
				//m_pWebManager->DeselectWebTab(pSelectedEmbeddedInstance);
				pSelectedEmbeddedInstance->Deselect();
				m_pInputManager->SetEmbeddedInstance(null);

				//m_pWebManager->SelectWebTab(pWebTab);
				pEmbeddedInstance->Select();
				m_pInputManager->SetEmbeddedInstance(pEmbeddedInstance);
			}
			else if (!pSelectedEmbeddedInstance)
			{
				//if (pEmbeddedInstance)
				//{
				pEmbeddedInstance->Select();
				m_pInputManager->SetEmbeddedInstance(pEmbeddedInstance);
				//}



			}
		}
		else
		{
			DevMsg("ERROR: No embedded instance!!\n");
			DevMsg("ID on this item is: %s", pShortcut->GetItemId().c_str());
		}
		//C_AwesomiumBrowserInstance* pHudWebTab = this->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");

		//m_pWebManager->SelectWebTab(pWebTab);

		//break;
	}

	g_pAnarchyManager->HudStateNotify();	// because input is not always request when an object is selected?
	return true;
	//*/
}

bool C_AnarchyManager::DeselectEntity(std::string nextUrl, bool bCloseInstance)
{
	C_EmbeddedInstance* pEmbeddedInstance = m_pInputManager->GetEmbeddedInstance();
	C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
	//C_WebTab* pWebTab = m_pWebManager->GetSelectedWebTab();
	if (pEmbeddedInstance && pEmbeddedInstance != pHudBrowserInstance)
	{	
		pEmbeddedInstance->Deselect();
		//pEmbeddedInstance->Blur();
		m_pInputManager->SetEmbeddedInstance(null);
		//m_pWebManager->DeselectWebTab(pWebTab);

		/*
		if (nextUrl != "none")
		{
			if (nextUrl != "")
				pHudBrowserInstance->SetUrl(nextUrl);
			else
				pHudBrowserInstance->SetUrl("asset://ui/blank.html");
		}
		*/

		// ALWAYS close the selected web tab when de-selecting entities. (this has to be accounted for or changed when the continous play button gets re-enabled!)
		if (bCloseInstance && pEmbeddedInstance != pHudBrowserInstance)
			pEmbeddedInstance->Close();	// FIXME: (maybe) This might cause blank.html to be loaded into the HUD layer because the entity that initiated all this is still set as m_pSelectedEntity at this point...
	}

	if (nextUrl != "none")
	{
		if (nextUrl != "")
			pHudBrowserInstance->SetUrl(nextUrl);
		else
			pHudBrowserInstance->SetUrl("asset://ui/blank.html");
	}

	RemoveGlowEffect(m_pSelectedEntity);
	m_pSelectedEntity = null;
	return true;
}

void C_AnarchyManager::AddGlowEffect(C_BaseEntity* pEntity)
{
	engine->ServerCmd(VarArgs("addgloweffect %i", pEntity->entindex()), false);
}

void C_AnarchyManager::RemoveGlowEffect(C_BaseEntity* pEntity)
{
	engine->ServerCmd(VarArgs("removegloweffect %i", pEntity->entindex()), false);
}

void C_AnarchyManager::ShowFileBrowseMenu(std::string browseId)
{
	BrowseSlate->Create(enginevgui->GetPanel(PANEL_ROOT), browseId);
}

void C_AnarchyManager::OnBrowseFileSelected(std::string browseId, std::string response)
{
	// only the HUD web view brings up the file browse menu so far...
	std::vector<std::string> params;
	params.push_back(browseId);
	params.push_back(response);

	C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
	pHudBrowserInstance->DispatchJavaScriptMethod("arcadeHud", "onBrowseFileSelected", params);
}

void C_AnarchyManager::OnWorkshopManagerReady()
{
//	DevMsg("DISABLED FOR TESTING!\n");
//	return;
	///*

	C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
	// mount ALL workshops
	pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Skipping Gen 1 Legacy Workshop Subscriptions", "skiplegacyworkshops", "", "", "0");
	pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading Workshop Models", "workshoplibrarymodels", "", "", "0");
	pHudBrowserInstance->AddHudLoadingMessage("progress", "", "Loading Workshop Items", "workshoplibraryitems", "", "", "0");

	m_pWorkshopManager->MountFirstWorkshop();
	//*/
}

void C_AnarchyManager::ScanForLegacySaveRecursive(std::string path)
{
	// detect any .set files in the legacy folder too
	std::string file;
	KeyValues* kv = new KeyValues("instance");
	FileFindHandle_t findHandle;
	DevMsg("Tester folder: %smaps\\*.set\n", path.c_str());
	//std::string path = "A:\\SteamLibrary\\steamapps\\common\\Anarchy Arcade\\aarcade\\";
	const char *pFilename = g_pFullFileSystem->FindFirstEx(VarArgs("%smaps\\*.set", path.c_str()), "", &findHandle);

	//const char *pFilename = g_pFullFileSystem->FindFirstEx("maps\\*.set", "", &findHandle);
	//const char *pFilename = g_pFullFileSystem->FindFirstEx("*.set", "GAME", &findHandle);
//	const char *pFilename = g_pFullFileSystem->FindFirstEx("*", "GAME", &findHandle);
	std::string goodMapName;
	while (pFilename != NULL)
	{
		file = path + "maps\\" + std::string(pFilename);
		//file = path + std::string(pFilename);

		if (g_pFullFileSystem->FindIsDirectory(findHandle))
		{
			//this->ScanForLegacySaveRecursive(file + "\\");
			pFilename = g_pFullFileSystem->FindNext(findHandle);
			continue;
		}
		//else if (std::string(pFilename).)

		//file = "maps\\" + std::string(pFilename);

		// FIXME: build an ACTUAL generation 3 instance key values here, and save it out!!
		if (kv->LoadFromFile(g_pFullFileSystem, file.c_str()))
		{
			// kv->GetString("map") can be corrupt on certain shitty saves that early access AArcade generated.  so use filename instead.
			goodMapName = pFilename;
			goodMapName = goodMapName.substr(0, goodMapName.find("."));

			//if (kv->FindKey("map") && kv->FindKey("objects", true)->GetFirstSubKey())
			if(kv->FindKey("objects", true)->GetFirstSubKey())
			{
				//	DevMsg("Map ID here is: %s\n", kv->GetString("map"));
				// FIXME: instance_t's should have mapId's, not MapNames.  The "mapName" should be considered the title.  The issue is that maps usually haven't been detected by this point, so assigning a mapID based on the legacy map name is complex.
				// For now, mapId's will be resolved upon map detection if mapID's equal a detected map's filename.

				std::string title = kv->GetString("title");
				if (title == "")
				{
					//title = "Unnamed";
					title = file;
					size_t found = title.find_last_of("/\\");
					if (found != std::string::npos)
						title = title.substr(found + 1);
				}

//				g_pAnarchyManager->GetInstanceManager()->AddInstance(g_pAnarchyManager->GenerateUniqueId(), kv->GetString("map"), title, file, "", "");
				//g_pAnarchyManager->GetInstanceManager()->AddInstance(g_pAnarchyManager->GenerateUniqueId(), goodMapName.c_str(), title, file, "", "");
				g_pAnarchyManager->GetInstanceManager()->AddInstance(g_pAnarchyManager->GenerateUniqueId(), g_pAnarchyManager->GenerateLegacyHash(goodMapName.c_str()), title, file, "", "", "");
				//g_pAnarchyManager->GetInstanceManager()->AddInstance(g_pAnarchyManager->GenerateLegacyHash(kv->GetString("map")), kv->GetString("map"), kv->GetString("map"), file, "", "");
			}
		}

		kv->Clear();
		pFilename = g_pFullFileSystem->FindNext(findHandle);
	}
	g_pFullFileSystem->FindClose(findHandle);
}

void C_AnarchyManager::ShowEngineOptionsMenu()
{
	//// FIXME: If a map is loaded, input mode can be deactivated, but if at main menu that might be weird.
	//if (engine->IsInGame())
	//	g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);

	engine->ClientCmd("gamemenucommand OpenOptionsDialog\n");
}

void C_AnarchyManager::DeactivateObjectPlacementMode(bool confirm)
{
	C_PropShortcutEntity* pShortcut = m_pMetaverseManager->GetSpawningObjectEntity();
	if (pShortcut)
	{
		if (confirm)
		{
			engine->ServerCmd(VarArgs("makenonghost %i;\n", pShortcut->entindex()), false);
			/*
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
			*/
		}
		else
		{
			// CANCEL
			this->GetMetaverseManager()->SetSpawningObjectEntity(null);
			this->GetMetaverseManager()->SetSpawningObject(null);

			//object_t* pObject = this->GetInstanceManager()->GetInstanceObject(pShortcut->GetObjectId());
			//if ( pObject )
			//	this->GetInstanceManager()->ResetObjectChanges(pShortcut);
			//else
			//this->GetInstanceManager()->RemoveEntity(pShortcut);
			this->GetInstanceManager()->ResetObjectChanges(pShortcut);	// this will also delete the object if there's nothing to revert to!!
		}

		m_pMetaverseManager->SetSpawningObjectEntity(null);
	}

	m_pMetaverseManager->SetSpawningObject(null);

	C_AwesomiumBrowserInstance* pHudInstance = m_pAwesomiumBrowserManager->FindAwesomiumBrowserInstance("hud");
	pHudInstance->SetUrl("asset://ui/blank.html");

	m_pInputManager->DeactivateInputMode(true);
}

void C_AnarchyManager::ActivateObjectPlacementMode(C_PropShortcutEntity* pShortcut, const char* mode)
{
	m_pInstanceManager->AdjustObjectScale(pShortcut->GetModelScale());
	m_pMetaverseManager->ResetSpawningAngles();
	m_pMetaverseManager->SetSpawningRotationAxis(1);
	m_pMetaverseManager->SetSpawningObjectEntity(pShortcut);

	std::string objectId = pShortcut->GetObjectId();
	//DevMsg("At placement id is: %s\n", objectId.c_str());
	object_t* theObject = m_pInstanceManager->GetInstanceObject(objectId);
	//DevMsg("Object val: %s\n", theObject->itemId.c_str());
	m_pMetaverseManager->SetSpawningObject(theObject);

	//std::string moreParams = "";
	if (!Q_strcmp(mode, "move"))
	{
		//SetLibraryBrowserContext(std::string category, std::string id, std::string search, std::string filter)
		std::string category = "";
		KeyValues* itemKv = m_pMetaverseManager->GetLibraryItem(pShortcut->GetItemId());
		if (itemKv)
			category = "items";
		else
		{
			// no item found, probably a model
			// FIXME: This is just ASSUMING that it is a model.
			category = "models";
			//moreParams = "&title=" + 
			//KeyValues* modelKv = m_pMetaverseManager->GetLibraryModel(pShortcut->GetItemId());
			//if (modelKv)
			//	category = "models";
		}
		m_pMetaverseManager->SetLibraryBrowserContext(category, std::string("automove"), "", "");
		engine->ServerCmd(VarArgs("makeghost %i;\n", pShortcut->entindex()), false);
		/*	// do this stuff server-side now
		pShortcut->SetSolid(SOLID_NONE);
		pShortcut->SetSize(-Vector(100, 100, 100), Vector(100, 100, 100));
		//SetRenderMode(kRenderTransTexture);
		pShortcut->SetRenderMode(kRenderTransColor);
		pShortcut->SetRenderColorA(160);
		*/
	}

	C_AwesomiumBrowserInstance* pHudInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");
	//pHudInstance->SetUrl("asset://ui/cabinetSelect.html");
	//std::string category = this->GetMetaverseManager()->GetLibraryBrowserContext("category");
	//pHudInstance->SetUrl(VarArgs("asset://ui/buildMode.html?mode=spawn&category=%s", category.c_str()));
	pHudInstance->SetUrl(VarArgs("asset://ui/buildMode.html?mode=%s", mode));
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, false, null, false);

	/*
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
	*/
}

void C_AnarchyManager::OnMountAllWorkshopsComplete()
{
	if (!m_pMountManager)	// it is our first time here
	{
		/*
		bool bObsoleteLegacyTester = true;
		if (bObsoleteLegacyTester)
		{
			std::string path = "A:\\SteamLibrary\\steamapps\\common\\Anarchy Arcade\\aarcade\\";
			this->ScanForLegacySaveRecursive(path);

			std::string workshopPath;
			unsigned int max = m_pWorkshopManager->GetNumDetails();
			for (unsigned int i = 0; i < max; i++)
			{
				SteamUGCDetails_t* pDetails = m_pWorkshopManager->GetDetails(i);
				workshopPath = path + "workshop\\" + std::string(VarArgs("%llu", pDetails->m_nPublishedFileId)) + "\\";
				this->ScanForLegacySaveRecursive(workshopPath);
			}

			// detect any .set files in the legacy folder too
			std::string file;
			KeyValues* kv = new KeyValues("instance");
			FileFindHandle_t findHandle;
			//DevMsg("Tester folder: %smaps\\*.set", path);
			//std::string path = "A:\\SteamLibrary\\steamapps\\common\\Anarchy Arcade\\aarcade\\";
			//const char *pFilename = g_pFullFileSystem->FindFirstEx(VarArgs("%smaps\\*.set", path), "", &findHandle);
			const char *pFilename = g_pFullFileSystem->FindFirstEx("maps\\*.set", "GAME", &findHandle);
			while (pFilename != NULL)
			{
				if (g_pFullFileSystem->FindIsDirectory(findHandle))
				{
					pFilename = g_pFullFileSystem->FindNext(findHandle);
					continue;
				}

				//file = std::string(path) + "maps\\" + std::string(pFilename);
				file = "maps\\" + std::string(pFilename);

				// FIXME: build an ACTUAL generation 3 instance key values here, and save it out!!
				if (kv->LoadFromFile(g_pFullFileSystem, file.c_str()))
				{
					if (kv->FindKey("map") && kv->FindKey("objects", true)->GetFirstSubKey())
					{
						//	DevMsg("Map ID here is: %s\n", kv->GetString("map"));
						// FIXME: instance_t's should have mapId's, not MapNames.  The "mapName" should be considered the title.  The issue is that maps usually haven't been detected by this point, so assigning a mapID based on the legacy map name is complex.
						// For now, mapId's will be resolved upon map detection if mapID's equal a detected map's filename.

						std::string title = kv->GetString("title");
						if (title == "")
						{
							//title = "Unnamed";
							title = file;
							size_t found = title.find_last_of("/\\");
							if (found != std::string::npos)
								title = title.substr(found + 1);
						}

						g_pAnarchyManager->GetInstanceManager()->AddInstance(g_pAnarchyManager->GenerateUniqueId(), kv->GetString("map"), title, file, "", "");
						//g_pAnarchyManager->GetInstanceManager()->AddInstance(g_pAnarchyManager->GenerateLegacyHash(kv->GetString("map")), kv->GetString("map"), kv->GetString("map"), file, "", "");
					}
				}

				kv->Clear();
				pFilename = g_pFullFileSystem->FindNext(findHandle);
			}
			g_pFullFileSystem->FindClose(findHandle);
		}
		*/

		m_pMountManager = new C_MountManager();
		m_pMountManager->Init();
		m_pMountManager->LoadMountsFromKeyValues("mounts.txt");

		m_pWorkshopManager = new C_WorkshopManager();
		m_pWorkshopManager->Init();
	}
	else
		this->GetMetaverseManager()->DetectAllMaps();
		//this->OnDetectAllMapsComplete();
}

void C_AnarchyManager::OnDetectAllMapsComplete()
{
	//DevMsg("DISABLED FOR TESTING!\n");
	//return;
	///*
	m_pLibretroManager = new C_LibretroManager();

	if (m_iState < 1)
	{
		m_iState = 1;

		// iterate through all models and assign the dynamic property to them
		// FIXME: THIS SHOULD BE DONE UPON MODEL IMPORT/LOADING!!
		//m_pMetaverseManager->DetectAllLegacyCabinets();
		m_pMetaverseManager->FlagDynamicModels();

		// this is where steamGames.key could be auto-scanned to make sure all Steam games exist in the library, if wanted.
		if (false && g_pFullFileSystem->FileExists("steamGames.key", "DEFAULT_WRITE_PATH"))
		{
			KeyValues* kv = new KeyValues("steamgames");
			kv->LoadFromFile(g_pFullFileSystem, "steamGames.key", "DEFAULT_WRITE_PATH");
			g_pAnarchyManager->GetMetaverseManager()->ImportSteamGames(kv);
		}

		// restart the sound system so that mounted paths can play sounds
		engine->ClientCmd("snd_restart");

		C_AwesomiumBrowserInstance* pHudBrowserInstance = m_pAwesomiumBrowserManager->GetSelectedAwesomiumBrowserInstance();
		//pHudBrowserInstance->SetUrl("asset://ui/welcome.html");
		pHudBrowserInstance->SetUrl("asset://ui/betasplash.html");
		g_pAnarchyManager->SetInitialized(true);
	}
	else
	{
		DevMsg("Done again!!\n");
	}
	//*/
}

void C_AnarchyManager::Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters)
{
	std::string safeStr = str;
	std::transform(safeStr.begin(), safeStr.end(), safeStr.begin(), ::tolower);

	// Skip delimiters at beginning.
	std::string::size_type lastPos = safeStr.find_first_not_of(delimiters, 0);

	// Find first "non-delimiter".
	std::string::size_type pos = safeStr.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));

		// Skip delimiters.  Note the "not_of"
		lastPos = safeStr.find_first_not_of(delimiters, pos);

		// Find next "non-delimiter"
		pos = safeStr.find_first_of(delimiters, lastPos);
	}
}

/*
void C_AnarchyManager::ReleaseFileBrowseParams()
{
	if (m_pFileParams)
	{
		delete m_pFileParams;
		m_pFileParams = null;
	}
}
*/

void C_AnarchyManager::xCastSetLiveURL()
{
	//if (!m_pXSplitLiveConVar->GetBool())
	//	return;

	//DevMsg("Do anarchybot stuff\n");
	std::string XSPlitLiveFolder = "Z:\\scripts";

	std::string xml = "";
	xml += "<div class=\"response\">\n";

	KeyValues* active = null;
	C_BaseEntity* pEntity = this->GetSelectedEntity();
	if (pEntity)
	{
		C_PropShortcutEntity* pShortcut = dynamic_cast<C_PropShortcutEntity*>(pEntity);
		if (pShortcut)
		{
			KeyValues* pItem = m_pMetaverseManager->GetLibraryItem(pShortcut->GetItemId());
			if (pItem)
			{
				active = pItem->FindKey("current");
				if (!active)
					active = pItem->FindKey("local", true);
			}
		}
	}
	else
	{
		// check for other open instances
		// TODO: This code should also be used for slave screens to show other instances!!

		std::vector<C_EmbeddedInstance*> embeddedInstances;
		m_pLibretroManager->GetAllInstances(embeddedInstances);
		m_pSteamBrowserManager->GetAllInstances(embeddedInstances);
		m_pAwesomiumBrowserManager->GetAllInstances(embeddedInstances);

		C_EmbeddedInstance* pEmbeddedInstance;
		std::string originalItemId;
		unsigned int i;
		unsigned int size = embeddedInstances.size();
		for (i = 0; i < size; i++)
		{
			pEmbeddedInstance = embeddedInstances[i];
			if (!pEmbeddedInstance)
				continue;

			// ignore special instances
			if (pEmbeddedInstance->GetId() == "hud" || pEmbeddedInstance->GetId() == "images")
				continue;

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

					if ( active )
						break;
				}
			}
		}
	}

	if (active)
	{
		xml += "\t<item class=\"item_container\">\n";

		// automatically add all root-level keys & values
		for (KeyValues *sub = active->GetFirstValue(); sub; sub = sub->GetNextValue())
		{
			if (sub->GetFirstSubKey() || !Q_strcmp(sub->GetName(), "") || !Q_strcmp(sub->GetString(), ""))
				continue;

			xml += "\t\t<";
			xml += sub->GetName();
			xml += ">";

			std::string xmlBuf = sub->GetString();
			size_t found = xmlBuf.find("&");
			while (found != std::string::npos)
			{
				xmlBuf.replace(found, 1, "&amp;");
				found = xmlBuf.find("&", found + 5);
			}

			found = xmlBuf.find("<");
			while (found != std::string::npos)
			{
				xmlBuf.replace(found, 1, "&lt;");
				found = xmlBuf.find("<", found + 4);
			}

			found = xmlBuf.find(">");
			while (found != std::string::npos)
			{
				xmlBuf.replace(found, 1, "&gt;");
				found = xmlBuf.find(">", found + 4);
			}

			xml += xmlBuf;

			xml += "</";
			xml += sub->GetName();
			xml += ">\n";
		}

		// do some extra work to get the best possible image results
		std::string bestScreenImage = active->GetString("screen");
		if (bestScreenImage == "" || bestScreenImage.find(":") == 1 || bestScreenImage.find("http") != 0)
			bestScreenImage = active->GetString("marquee");
		if (bestScreenImage == "" || bestScreenImage.find(":") == 1 || bestScreenImage.find("http") != 0)
			bestScreenImage = active->GetString("file");
		if (bestScreenImage == "" || bestScreenImage.find(":") == 1 || bestScreenImage.find("http") != 0)
			bestScreenImage = "noimage.png";

		xml += "\t\t<screen2use>";
		xml += bestScreenImage;
		xml += "</screen2use>\n";

		std::string bestMarqueeImage = active->GetString("marquee");
		if (bestMarqueeImage == "" || bestMarqueeImage.find(":") == 1 || bestMarqueeImage.find("http") != 0)
			bestMarqueeImage = active->GetString("screen");
		if (bestMarqueeImage == "" || bestMarqueeImage.find(":") == 1 || bestMarqueeImage.find("http") != 0)
			bestMarqueeImage = active->GetString("file");
		if (bestMarqueeImage == "" || bestMarqueeImage.find(":") == 1 || bestMarqueeImage.find("http") != 0)
			bestMarqueeImage = "noimage.png";

		xml += "\t\t<marquee2use>";
		xml += bestMarqueeImage;
		xml += "</marquee2use>\n";

		xml += "\t\t<screenurl>";
		xml += active->GetString("screen");
		xml += "</screenurl>\n";

		xml += "\t\t<marqueeurl>";
		xml += active->GetString("marquee");
		xml += "</marqueeurl>\n";

		xml += "\t\t<bestimageurl>";
		if (bestMarqueeImage != "")
			xml += bestMarqueeImage;
		else
			xml += bestScreenImage;
		xml += "</bestimageurl>\n";


		xml += "\t\t<isremember>0</isremember>\n";
		xml += "\t</item>\n";
	}

	xml += "</div>";

	// Write this live URL out to the save file.
	FileHandle_t hFile = g_pFullFileSystem->Open(VarArgs("%s\\liveItems.xml", XSPlitLiveFolder.c_str()), "w", "");

	if (hFile)
	{
		//DevMsg("Writing to liveItems:\n");
		//DevMsg("%s\n", xml.c_str());

		/*
		// null terminate
		int len = xml.length();
		char* stuff = new char[xml.length() + 1];
		Q_strcpy(stuff, xml.c_str());
		stuff[len] = 0;	// null terminator
		*/

		//std::stringstream buffer;
		//buffer << "Text" << std::endl;

		g_pFullFileSystem->FPrintf(hFile, "%s", xml.c_str());
		g_pFullFileSystem->Close(hFile);
	}

	// Also update a JS file to force the page to re-load
	hFile = g_pFullFileSystem->Open(VarArgs("%s\\vote.js", XSPlitLiveFolder.c_str()), "a+", "");

	if (hFile)
	{
		std::string selectedItemTitle = "";
		if (active)
			selectedItemTitle = active->GetString("info/id");

		std::string rememberedItemTitle = "";
		//if (pRememberItemKV)
		//	rememberedItemTitle = pRememberItemKV->GetString("itemfilehash");

		std::string code = "gAnarchyTV.OnAArcadeCommand(\"selection\", \"";
		code += selectedItemTitle;
		code += "\", \"";
		code += rememberedItemTitle;
		code += "\");\n";

		//g_pFullFileSystem->FPrintf(hFile, VarArgs("var currentTick = \"%i\";\n", gpGlobals->tickcount));
		g_pFullFileSystem->Write(code.c_str(), code.length(), hFile);

		g_pFullFileSystem->Close(hFile);
	}

	//DevMsg("Done doing anarchybot stuff!!\n");
}

void C_AnarchyManager::TestSQLite2()
{
	DevMsg("LOAD SQL TEST...\n");
	int rc;
	char *error;

	// Open Database
	DevMsg("Opening MyDb.db ...\n");

	sqlite3 *db;
	rc = sqlite3_open("MyDb.db", &db);
	if (rc)
	{
		DevMsg("Error opening SQLite3 database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return;
		//return 1;
	}
	else
	{
		DevMsg("Opened MyDb.db.\n");
	}

	// Execute SQL
	sqlite3_stmt *stmt = NULL;
	rc = sqlite3_prepare(db, "SELECT * from MyTable", -1, &stmt, NULL);
	//rc = sqlite3_prepare(db, "INSERT INTO MyTable VALUES(NULL, ?)", -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("prepare failed: %s\n", sqlite3_errmsg(db));

	int length;
	while (sqlite3_step(stmt) == SQLITE_ROW)	// THIS IS WHERE THE LOOP CAN BE BROKEN UP AT!!
	{
		length = sqlite3_column_bytes(stmt, 1);

		KeyValues* pTesterKV = new KeyValues("anotherTester");
		CUtlBuffer buf(0, length, 0);	// the length param should NEVER be zero.
		buf.CopyBuffer(sqlite3_column_blob(stmt, 1), length);
		pTesterKV->ReadAsBinary(buf);
		DevMsg("Value here is: %s\n", pTesterKV->GetString("originalTesterKey"));

		/*
		void* buffer = malloc(length);
		memcpy(buffer, sqlite3_column_blob(stmt, 0), length);
		buf.Put(buffer, length);
		pTesterKV->ReadAsBinary(buf);
		DevMsg("Buf value has: %s\n", pTesterKV->GetString("originalTesterKey"));
		pTesterKV->deleteThis();
		*/

	//	buf.Put
	}
	sqlite3_finalize(stmt);	// TODO: error checking?  Maybe not needed, if this is like a close() operation.

	// Close Database
	DevMsg("Closing MyDb.db ...\n");
	sqlite3_close(db);
	DevMsg("Closed MyDb.db\n");
}

void C_AnarchyManager::TestSQLite()
{
	int rc;
	char *error;

	// create the library database
	DevMsg("Create library database...\n");

	sqlite3 *db;
	rc = sqlite3_open("aarcade_user/library.db", &db);
	if (rc)
	{
		DevMsg("Error opening SQLite3 database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return;
	}

	// create the tables
	const char *sqlCreateAppsTable = "CREATE TABLE apps (id INTEGER PRIMARY KEY, value BLOB);";
	rc = sqlite3_exec(db, sqlCreateAppsTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(db));
		sqlite3_free(error);
	}

	const char *sqlCreateItemsTable = "CREATE TABLE items (id INTEGER PRIMARY KEY, value BLOB);";
	rc = sqlite3_exec(db, sqlCreateItemsTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(db));
		sqlite3_free(error);
	}

	const char *sqlCreateMapsTable = "CREATE TABLE maps (id INTEGER PRIMARY KEY, value BLOB);";
	rc = sqlite3_exec(db, sqlCreateMapsTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(db));
		sqlite3_free(error);
	}

	const char *sqlCreateModelsTable = "CREATE TABLE models (id INTEGER PRIMARY KEY, value BLOB);";
	rc = sqlite3_exec(db, sqlCreateModelsTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(db));
		sqlite3_free(error);
	}

	const char *sqlCreatePlatformsTable = "CREATE TABLE platforms (id INTEGER PRIMARY KEY, value BLOB);";
	rc = sqlite3_exec(db, sqlCreatePlatformsTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(db));
		sqlite3_free(error);
	}

	const char *sqlCreateTypesTable = "CREATE TABLE types (id INTEGER PRIMARY KEY, value BLOB);";
	rc = sqlite3_exec(db, sqlCreateTypesTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(db));
		sqlite3_free(error);
	}

	const char *sqlCreateInstancesTable = "CREATE TABLE instances (id INTEGER PRIMARY KEY, value BLOB);";
	rc = sqlite3_exec(db, sqlCreateInstancesTable, NULL, NULL, &error);
	if (rc != SQLITE_OK)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(db));
		sqlite3_free(error);
	}

	// FIXME: TODO: Add the default stuff to the database by default?  or maybe this should be done through a higher-level system.

	// DATABASE IS NOW CREATED WITH THE PROPER TABLES, BUT IS COMPLETELY EMPTY.
	sqlite3_close(db);
	DevMsg("Library created.\n");
	// Execute SQL
	/*
	DevMsg("Inserting a value into MyTable ...\n");
	const char *sqlInsert = "INSERT INTO MyTable VALUES(NULL, 'A Value');";
	rc = sqlite3_exec(db, sqlInsert, NULL, NULL, &error);
	if (rc)
	{
		DevMsg("Error executing SQLite3 statement: %s\n", sqlite3_errmsg(db));
		sqlite3_free(error);
	}
	else
	{
		DevMsg("Inserted a value into MyTable.\n");
	}
	*/
	/*
	DevMsg("Inserting a BINARY value into MyTable ...\n");
	sqlite3_stmt *stmt = NULL;
	rc = sqlite3_prepare(db,
		"INSERT INTO MyTable VALUES(NULL, ?)", -1, &stmt, NULL);
	if (rc != SQLITE_OK)
		DevMsg("prepare failed!\n");

	// SQLITE_STATIC because the statement is finalized
	// before the buffer is freed:

	CUtlBuffer buf;
	KeyValues* pObjectKV = new KeyValues("originalTester");//pInstanceObjectsKV->FindKey(VarArgs("%s/local", objectId.c_str()), true);
	pObjectKV->SetString("originalTesterKey", "yup");
	pObjectKV->WriteAsBinary(buf);
	pObjectKV->deleteThis();

	int size = buf.Size();

	rc = sqlite3_bind_blob(stmt, 1, buf.Base(), size, SQLITE_STATIC);
	if (rc != SQLITE_OK)
		DevMsg("bind failed: %s\n", sqlite3_errmsg(db));
	else
	{
		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE)
			DevMsg("execution failed: %s\n", sqlite3_errmsg(db));
		else
			DevMsg("execution worked!\n");
	}

	sqlite3_finalize(stmt);
	*/
	/*
	// Display MyTable
	DevMsg("Retrieving values in MyTable ...\n");
	const char *sqlSelect = "SELECT * FROM MyTable;";
	char **results = NULL;
	int rows, columns;
	sqlite3_get_table(db, sqlSelect, &results, &rows, &columns, &error);
	if (rc)
	{
		DevMsg("Error executing SQLite3 query: %s\n", sqlite3_errmsg(db));
		sqlite3_free(error);
	}
	else
	{
		// Display Table
		for (int rowCtr = 0; rowCtr <= rows; ++rowCtr)
		{
			for (int colCtr = 0; colCtr < columns; ++colCtr)
			{
				// Determine Cell Position
				int cellPosition = (rowCtr * columns) + colCtr;

				// Display Cell Value
				DevMsg("%s\t", results[cellPosition]);
				//cout.width(12);
				//cout.setf(ios::left);
				//cout << results[cellPosition] << " ";
			}

			// End Line
			//cout << endl;
			DevMsg("\n");

			// Display Separator For Header
			if (0 == rowCtr)
			{
				for (int colCtr = 0; colCtr < columns; ++colCtr)
				{
					//cout.width(12);
					//cout.setf(ios::left);
					DevMsg("~~~~~~~~~~~~");
				}
				DevMsg("\n");
			}
		}
	}
	sqlite3_free_table(results);
	*/

	// Close Database
	//DevMsg("Closing MyDb.db ...\n");
	//sqlite3_close(db);
	//DevMsg("Closed MyDb.db\n");


	/*
		CUtlBuffer buf;
		//buf.Get()
		KeyValues* pObjectKV = new KeyValues("originalTester");//pInstanceObjectsKV->FindKey(VarArgs("%s/local", objectId.c_str()), true);
		pObjectKV->SetString("originalTesterKey", "yup");
		pObjectKV->WriteAsBinary(buf);
		pObjectKV->deleteThis();

		KeyValues* pTesterKV = new KeyValues("reduxTester");
		pTesterKV->ReadAsBinary(buf);
		DevMsg("Annd here the big result is: %s\n", pTesterKV->GetString("originalTesterKey"));
		pTesterKV->deleteThis();
	*/
}

/*
void C_AnarchyManager::LevelInitPreEntity()
{
	DevMsg("AnarchyManager: LevelInitPreEntity\n");
	//m_pWebViewManager = new C_WebViewManager;
//	m_pWebViewManager->Init();
}

void C_AnarchyManager::LevelShutdownPostEntity()
{
	DevMsg("AnarchyManager: LevelShutdownPostEntity\n");
	// FIXME: Deleting the webview manager prevents it from starting up again.
	// Need to only create/delete it ONCE during the lifetime of AArcade.
	//delete m_pWebViewManager;
}
*/
/*
const char* C_AnarchyManager::GenerateHash(const char* text)
{
	char input[AA_MAX_STRING];
	Q_strcpy(input, text);

	// Convert it to lowercase & change all slashes to back-slashes
	V_FixSlashes(input);
	for( int i = 0; input[i] != '\0'; i++ )
		input[i] = tolower(input[i]);

	char lower[256];
	unsigned m_crc = 0xffffffff;

	int inputLength = strlen(input);
	for (int i = 0; i < inputLength; i++)
	{
		lower[i] = tolower(input[i]);
	}

	for (int i = 0; i < inputLength; i++)
	{
		unsigned c = lower[i];
		m_crc ^= (c << 24);

		for (int j = 0; j < 8; j++)
		{
			const unsigned FLAG = 0x80000000;
			if ((m_crc & FLAG) == FLAG)
			{
				m_crc = (m_crc << 1) ^ 0x04C11DB7;
			}
			else
			{
				m_crc <<= 1;
			}
		}
	}

	return VarArgs("%08x", m_crc);
}
*/
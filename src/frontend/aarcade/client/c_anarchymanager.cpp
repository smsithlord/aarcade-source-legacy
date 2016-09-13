#include "cbase.h"
#include "aa_globals.h"
#include "c_anarchymanager.h"
//#include "c_systemtime.h"
#include "WinBase.h"
#include <cctype>
#include <algorithm>
//#include "mathlib/mathlib.h"
//#include <math.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_AnarchyManager g_AnarchyManager;
extern C_AnarchyManager* g_pAnarchyManager(&g_AnarchyManager);

C_AnarchyManager::C_AnarchyManager() : CAutoGameSystemPerFrame("C_AnarchyManager")
{
	DevMsg("AnarchyManager: Constructor\n");
	m_state = AASTATE_NONE;
	m_iState = 0;
	m_bIncrementState = false;
	m_bPaused = false;
	m_pCanvasManager = null;
	m_pWebManager = null;
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
	return true;
}

void C_AnarchyManager::PostInit()
{
	DevMsg("AnarchyManager: PostInit\n");
}

void C_AnarchyManager::Shutdown()
{
	DevMsg("AnarchyManager: Shutdown\n");

	if (m_pWebManager)
	{
		delete m_pWebManager;
		m_pWebManager = null;
	}

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
}

void C_AnarchyManager::LevelInitPostEntity()
{
	DevMsg("AnarchyManager: LevelInitPostEntity\n");

	if (m_instanceId != "")
		g_pAnarchyManager->GetInstanceManager()->LoadLegacyInstance(m_instanceId);
}

void C_AnarchyManager::LevelShutdownPreClearSteamAPIContext()
{
	DevMsg("AnarchyManager: LevelShutdownPreClearSteamAPIContext\n");
}

void C_AnarchyManager::LevelShutdownPreEntity()
{
	DevMsg("AnarchyManager: LevelShutdownPreEntity\n");
	C_BaseEntity* pEntity = this->GetSelectedEntity();
	if (pEntity)
		this->DeselectEntity(pEntity);

	C_WebTab* pWebTab = m_pWebManager->GetSelectedWebTab();
	if (pWebTab)
		m_pWebManager->DeselectWebTab(pWebTab);

	m_pWebManager->LevelShutdownPreEntity();
}

void C_AnarchyManager::LevelShutdownPostEntity()
{
	DevMsg("AnarchyManager: LevelShutdownPostEntity\n");
	m_instanceId = "";
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
			if (m_pLibretroManager)
				m_pLibretroManager->Update();

			if (m_pSteamBrowserManager)
				m_pSteamBrowserManager->Update();

			if (m_pAwesomiumBrowserManager)
				m_pAwesomiumBrowserManager->Update();
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
			m_pAwesomiumBrowserManager->Update();
			break;

		case AASTATE_AWESOMIUMBROWSERMANAGERHUD:
			m_pAwesomiumBrowserManager->CreateAwesomiumBrowserInstance("hud", "asset://ui/welcome.html", true);	// defaults to asset://ui/blank.html
			g_pAnarchyManager->IncrementState();
			break;

		case AASTATE_AWESOMIUMBROWSERMANAGERHUDWAIT:
			m_pAwesomiumBrowserManager->Update();
			break;

		case AASTATE_AWESOMIUMBROWSERMANAGERHUDINIT:
			DevMsg("Finished initing HUD.\n");
			g_pAnarchyManager->IncrementState();
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

#include "ienginevgui.h"
bool C_AnarchyManager::HandleUiToggle()
{
	if (m_pSteamBrowserManager)
	{
		C_SteamBrowserInstance* pInstance = m_pSteamBrowserManager->GetSelectedSteamBrowserInstance();
		//if (m_pInputManager->GetMainMenuMode() && m_pInputManager->GetInputMode() && m_pInputManager->GetFullscreenMode() && pInstance && pInstance->GetTexture() && pInstance->GetTexture() == m_pInputManager->GetInputCanvasTexture())
		if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == pInstance)
		{
			m_pSteamBrowserManager->DestroySteamBrowserInstance(pInstance);
			return true;
		}
	}

	if (m_pLibretroManager)
	{
		C_LibretroInstance* pInstance = m_pLibretroManager->GetSelectedLibretroInstance();
		//if (m_pInputManager->GetMainMenuMode() && m_pInputManager->GetInputMode() && m_pInputManager->GetFullscreenMode() && pInstance && pInstance->GetTexture() && pInstance->GetTexture() == m_pInputManager->GetInputCanvasTexture())
		if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == pInstance)
		{
			m_pLibretroManager->DestroyLibretroInstance(pInstance);
			return true;
		}
	}

	if (m_pAwesomiumBrowserManager)
	{
		C_AwesomiumBrowserInstance* pInstance = m_pAwesomiumBrowserManager->GetSelectedAwesomiumBrowserInstance();
		//if (m_pInputManager->GetMainMenuMode() && m_pInputManager->GetInputMode() && m_pInputManager->GetFullscreenMode() && pInstance && pInstance->GetTexture() && pInstance->GetTexture() == m_pInputManager->GetInputCanvasTexture())
		if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == pInstance)
		{
			m_pAwesomiumBrowserManager->DestroyAwesomiumBrowserInstance(pInstance);
			return true;
		}
	}
	/*
	// handle escape if in pause mode (ignore it)
	if (!engine->IsInGame())
		return false;

	if (m_bPaused)
		return true;

	if (m_pInputManager->GetInputMode())
	{
		// handle escape if in fullscreen input mode (drop out of fullscreen mode)
		if ((!m_pInputManager->GetFullscreenMode() || !this->GetSelectedEntity() || m_pInputManager->GetWasForceInputMode()) || (this->GetSelectedEntity() && m_pInputManager->GetFullscreenMode()))
			m_pInputManager->DeactivateInputMode(true);
		else
			m_pInputManager->SetFullscreenMode(false);

		return true;
	}
	else if (!m_pInputManager->GetInputMode() && engine->IsInGame() )
	{
		// handle escape if not in input mode & map is loaded (display the main menu)
		//engine->IsInGame()
		//engine->IsPaused()
		if (!enginevgui->IsGameUIVisible())
		{
			//DevMsg("DISPLAY MAIN MENU\n");
			if (m_pSelectedEntity)
				this->DeselectEntity(m_pSelectedEntity, "asset://ui/welcome.html");
			else
				m_pWebManager->GetHudWebTab()->SetUrl("asset://ui/welcome.html");

			m_pInputManager->ActivateInputMode(true, true);
			return false;
		}
	}
	*/

	return false;
}

void C_AnarchyManager::Pause()
{
	m_bPaused = true;
}

void C_AnarchyManager::Unpause()
{
	m_bPaused = false;
	m_pWebManager->GetHudWebTab()->SetUrl("asset://ui/blank.html");
	m_pInputManager->DeactivateInputMode(true);
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
std::string C_AnarchyManager::GenerateUniqueId()
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

	return id;
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
	
	// generate a legacy ID based on the filelocation if given an item to work with
	if (!bPassed && item)
		nameSnip = this->GenerateLegacyHash(item->GetString("filelocation"));

	if (!bPassed)
		nameSnip = "";

	return nameSnip;
}

const char* C_AnarchyManager::GenerateLegacyHash(const char* text)
{
	char input[AA_MAX_STRING];
	Q_strcpy(input, text);

	// Convert it to lowercase & change all slashes to back-slashes
	V_FixSlashes(input);
	for (int i = 0; input[i] != '\0'; i++)
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

void C_AnarchyManager::OnLoadAllLocalAppsComplete()
{
	/*
	m_pMountManager = new C_MountManager();
	m_pMountManager->Init();
	m_pMountManager->LoadMountsFromKeyValues("mounts.txt");
	*/

	std::string path = "A:\\SteamLibrary\\steamapps\\common\\Anarchy Arcade\\aarcade\\";
	g_pFullFileSystem->AddSearchPath(path.c_str(), "MOD", PATH_ADD_TO_TAIL);
	g_pFullFileSystem->AddSearchPath(path.c_str(), "GAME", PATH_ADD_TO_TAIL);
	g_pAnarchyManager->GetMetaverseManager()->LoadFirstLocalItemLegacy(true, path, "", "");
}

bool C_AnarchyManager::AttemptSelectEntity()
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

	C_BaseEntity *pEntity;
	if (tr.fraction != 1.0 && tr.DidHitNonWorldEntity())
	{
		pEntity = tr.m_pEnt;

		if (m_pSelectedEntity && pEntity == m_pSelectedEntity)
		{
			//m_pInputManager->SetFullscreenMode(true);
			m_pInputManager->ActivateInputMode(true);
		}
		else
			return SelectEntity(pEntity);
	}
	else
	{
		if (m_pSelectedEntity)
			return DeselectEntity(m_pSelectedEntity);
		else
			return false;
	}
}

// from http://www.zedwood.com/article/cpp-urlencode-function
#include <iostream>
#include <sstream>
std::string encodeURIComponent(const std::string &s)
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
	if (m_pSelectedEntity)
		DeselectEntity(m_pSelectedEntity);

	m_pWebManager->GetHudWebTab()->SetUrl("asset://ui/blank.html");

	m_pSelectedEntity = pEntity;
	AddGlowEffect(pEntity);

	// DETECT DYNAMIC TEXTURES
	const model_t* model = pEntity->GetModel();//modelinfo->FindOrLoadModel(model);

	IMaterial* pMaterials[1024];
	for (int x = 0; x < 1024; x++)
		pMaterials[x] = NULL;

	modelinfo->GetModelMaterials(model, 1024, &pMaterials[0]);

	//pMaterials[x]->ColorModulate(255, 0, 0);
	//pMaterials[x]->GetPreviewImage

	std::string itemId;
	std::string tabTitle;
	std::string uri;
	KeyValues* item;
	KeyValues* active;
	C_PropShortcutEntity* pShortcut;
	C_WebTab* pWebTab;
	C_WebTab* pSelectedWebTab;
	IMaterial* pMaterial;
	for (int x = 0; x < 1024; x++)
	{
		if (pMaterials[x] && pMaterials[x]->HasProxy())
		{
			pMaterial = pMaterials[x];
//			bool found;
	//		IMaterialVar* pMaterialVar = pMaterial->FindVar("simpleimagechannel", &found);
		//	if (found && !Q_strcmp(pMaterialVar->GetStringValue(), 
			// FIXME: Add some kind of material variable that could allow us to be skipped? We'll need something like that when WebImages are added.
			pWebTab = m_pWebManager->FindWebTab(pMaterial);
			if (pWebTab)
			{
				// if this is the "images" web tab, we should create a NEW web tab for it (usually) because we don't actually show the iamges web tab.
				bool bImagesAndHandled = false;
				if (pWebTab->GetId() == "images")
				{
					pShortcut = dynamic_cast<C_PropShortcutEntity*>(m_pSelectedEntity);
					if (pShortcut)
					{
						tabTitle = "auto" + pShortcut->GetItemId();
						pWebTab = this->GetWebManager()->FindWebTab(tabTitle);
						if (!pWebTab)
						{
							itemId = pShortcut->GetItemId();
							item = m_pMetaverseManager->GetLibraryItem(itemId);
							if (item)
							{
								active = item->FindKey("current");
								if (!active)
									active = item->FindKey("local", true);

								std::string uri = "asset://ui/autoInspectItem.html?id=" + encodeURIComponent(itemId) + "&screen=" + encodeURIComponent(active->GetString("screen")) + "&marquee=" + encodeURIComponent(active->GetString("marquee")) + "&preview=" + encodeURIComponent(active->GetString("preview")) + "&reference=" + encodeURIComponent(active->GetString("reference")) + "&file=" + encodeURIComponent(active->GetString("file"));

								WebURL url = WebURL(WSLit(uri.c_str()));
//								DevMsg("Parsed is: %s\n", WebStringToCharString(url.spec()));
								pWebTab = m_pWebManager->CreateWebTab(WebStringToCharString(url.spec()), tabTitle);
							}
						}
					}
				}
				else
				{
					pSelectedWebTab = m_pWebManager->GetSelectedWebTab();
					if (pSelectedWebTab)
						m_pWebManager->DeselectWebTab(pSelectedWebTab);
				}

				pSelectedWebTab = m_pWebManager->GetSelectedWebTab();
				if (pSelectedWebTab && pSelectedWebTab != pWebTab)
				{
					m_pWebManager->DeselectWebTab(pSelectedWebTab);
					m_pWebManager->SelectWebTab(pWebTab);
				}
				else if ( !pSelectedWebTab )
					m_pWebManager->SelectWebTab(pWebTab);

				break;
			}
		}
			/*
			Material: vgui/websurfacealt2
			Material: vgui/websurfacealt
			Material: vgui/websurfacealt5
			Material: vgui/websurfacealt5
			Material: vgui/websurfacealt7
			*/
	}


	return true;
}

bool C_AnarchyManager::DeselectEntity(C_BaseEntity* pEntity, std::string nextUrl)
{
	C_WebTab* pWebTab = m_pWebManager->GetSelectedWebTab();
	if (pWebTab)
	{
		m_pWebManager->DeselectWebTab(pWebTab);

		if (nextUrl != "")
			m_pWebManager->GetHudWebTab()->SetUrl(nextUrl);
		else
			m_pWebManager->GetHudWebTab()->SetUrl("asset://ui/blank.html");
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

void C_AnarchyManager::OnWorkshopManagerReady()
{
	// mount ALL workshops
	m_pWebManager->GetHudWebTab()->AddHudLoadingMessage("progress", "", "Skipping Legacy Workshop Subscriptions", "skiplegacyworkshops", "", "", "0");
	m_pWebManager->GetHudWebTab()->AddHudLoadingMessage("progress", "", "Loading Workshop Models", "workshoplibrarymodels", "", "", "0");
	m_pWebManager->GetHudWebTab()->AddHudLoadingMessage("progress", "", "Loading Workshop Items", "workshoplibraryitems", "", "", "0");

	m_pWorkshopManager->MountFirstWorkshop();
}

void C_AnarchyManager::OnMountAllWorkshopsComplete()
{
	if (!m_pMountManager)	// it is our first time here
	{
		std::string path = "A:\\SteamLibrary\\steamapps\\common\\Anarchy Arcade\\aarcade\\";

		// detect any .set files in the legacy folder too
		std::string file;
		KeyValues* kv = new KeyValues("instance");
		FileFindHandle_t findHandle;
		//DevMsg("Tester folder: %smaps\\*.set", path);
		const char *pFilename = g_pFullFileSystem->FindFirstEx(VarArgs("%smaps\\*.set", path), "", &findHandle);
		while (pFilename != NULL)
		{
			if (g_pFullFileSystem->FindIsDirectory(findHandle))
			{
				pFilename = g_pFullFileSystem->FindNext(findHandle);
				continue;
			}

			file = std::string(path) + "maps\\" + std::string(pFilename);

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
	m_pLibretroManager = new C_LibretroManager();

	if (m_iState < 1)
	{
		m_iState = 1;
		m_pWebManager->GetSelectedWebTab()->SetUrl("asset://ui/welcome.html");
	}
	else
	{
		DevMsg("Done again!!\n");
	}
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
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
	m_pWebManager = null;
	m_pLoadingManager = null;
	m_pLibretroManager = null;
	m_pInputManager = null;
	m_pSelectedEntity = null;
	m_pMountManager = null;
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

	delete m_pWebManager;
	m_pWebManager = null;

	delete m_pLoadingManager;
	m_pLoadingManager = null;

	delete m_pLibretroManager;
	m_pLibretroManager = null;

	delete m_pInputManager;
	m_pInputManager = null;

	delete m_pMountManager;
	m_pMountManager = null;

	delete m_pWorkshopManager;
	m_pWorkshopManager = null;

	delete m_pMetaverseManager;
	m_pMetaverseManager = null;
}

void C_AnarchyManager::LevelInitPreEntity()
{
	DevMsg("AnarchyManager: LevelInitPreEntity\n");
}

void C_AnarchyManager::LevelInitPostEntity()
{
	DevMsg("AnarchyManager: LevelInitPostEntity\n");
}

void C_AnarchyManager::LevelShutdownPreClearSteamAPIContext()
{
	DevMsg("AnarchyManager: LevelShutdownPreClearSteamAPIContext\n");
}

void C_AnarchyManager::LevelShutdownPreEntity()
{
	DevMsg("AnarchyManager: LevelShutdownPreEntity\n");
}

void C_AnarchyManager::LevelShutdownPostEntity()
{
	DevMsg("AnarchyManager: LevelShutdownPostEntity\n");
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

void C_AnarchyManager::Update(float frametime)
{
	//DevMsg("Float: %f\n", frametime);	// deltatime
	//DevMsg("Float: %i\n", gpGlobals->framecount);	// numframes total
	if (m_pLibretroManager)
		m_pLibretroManager->Update();

	if (m_pWebManager)
		m_pWebManager->Update();

	//DevMsg("AnarchyManager: Update\n");
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
	size_t found = itemFile.find_last_of("/\\");
	if (found == std::string::npos)
		bPassed = false;
	else
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

	if (!bPassed)
		nameSnip = this->GenerateLegacyHash(item->GetString("filelocation"));

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
void C_AnarchyManager::AnarchyBegin()
{
	DevMsg("AnarchyManager: AnarchyBegin\n");

	m_pMetaverseManager = new C_MetaverseManager();
	m_pWebManager = new C_WebManager();
	m_pWebManager->Init();
	m_pInputManager = new C_InputManager();
}

void C_AnarchyManager::OnWebManagerReady()
{
	m_pLoadingManager = new C_LoadingManager();
	m_pLoadingManager->SetHeader("Anarchy Arcade - Starting Up");
	m_pLoadingManager->AddMessage("progress", "", "Loading Types", "locallibrarytypes", "", "", "");
	m_pLoadingManager->AddMessage("progress", "", "Loading Models", "locallibrarymodels", "", "", "");
	m_pLoadingManager->AddMessage("progress", "", "Loading Apps", "locallibraryapps", "", "", "");
	m_pLoadingManager->AddMessage("progress", "", "Loading Items", "locallibraryitems", "", "", "");
	m_pLoadingManager->AddMessage("progress", "", "Mounting Source Engine Games", "mounts", "", "", "");
	m_pLoadingManager->AddMessage("progress", "", "Fetching Workshop Subscriptions", "work", "", "", "");
	//m_pLoadingManager->AddMessage("progress", "", "Mounting Workshop Subscriptions", "mountworkshops", "", "", "");
	m_pLoadingManager->AddMessage("progress", "", "Skipping Legacy Workshop Subscriptions", "mountlegacyworkshops", "", "", "");
	m_pLoadingManager->AddMessage("progress", "", "Loading Workshop Models", "workshoplibrarymodels", "", "", "");
	m_pLoadingManager->AddMessage("progress", "", "Loading Workshop Items", "workshoplibraryitems", "", "", "");
	//m_pLoadingManager->AddMessage("progress", "", "Loading Legacy Items", "legacyworkshoplibraryitems", "", "", "");

	//m_pLoadingManager->Init();

	//m_pLibretroManager = new C_LibretroManager();

	//C_WebTab* pWebTab = m_pWebManager->CreateWebTab("asset://ui/welcomemenu.html", "mainmenu", false);
	//m_pWebManager->SelectWebTab(pWebTab);
	//m_pInputManager->ActivateInputMode(true);

	//DevMsg("great fuzzy blockos\n");

	//C_WebTab* pWebTab = pWebManager->CreateWebTab("http://localhost:8001/terminal.html", "metaverse");
//}

//void C_AnarchyManager::OnLoadingManagerReady()
//{
	// Show the loading menu
//	C_WebTab* pWebTab = m_pWebManager->CreateWebTab("asset://ui/loading.html", "metaverse", false);
//	m_pWebManager->SelectWebTab(pWebTab);
//	m_pInputManager->ActivateInputMode(true);

//	m_pLoadingManager->AddMessage("progress", "", "Detecting Workshop Subscriptions", "work", "0", "10", "7");

	// And continue starting up
	unsigned int uItemCount = m_pMetaverseManager->LoadAllLocalTypes();
	std::string num = VarArgs("%u", uItemCount);
	m_pLoadingManager->AddMessage("progress", "", "Loading Types", "locallibrarytypes", "0", num, num);

	uItemCount = m_pMetaverseManager->LoadAllLocalModels();
	num = VarArgs("%u", uItemCount);
	m_pLoadingManager->AddMessage("progress", "", "Loading Models", "locallibrarymodels", "0", num, num);
	
	uItemCount = m_pMetaverseManager->LoadAllLocalApps();
	num = VarArgs("%u", uItemCount);
	m_pLoadingManager->AddMessage("progress", "", "Loading Apps", "locallibraryapps", "0", num, num);

	//uItemCount = m_pMetaverseManager->LoadAllLocalItemsLegacy();
	//num = VarArgs("%u", uItemCount);
	//m_pLoadingManager->AddMessage("progress", "", "Loading Items", "locallibraryitems", "0", num, num);

	m_pMountManager = new C_MountManager();
	m_pMountManager->Init();
	m_pMountManager->LoadMountsFromKeyValues("mounts.txt");

	m_pWorkshopManager = new C_WorkshopManager();
	m_pWorkshopManager->Init();
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

bool C_AnarchyManager::SelectEntity(C_BaseEntity* pEntity)
{
	if (m_pSelectedEntity)
		DeselectEntity(m_pSelectedEntity);

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

	C_WebTab* pWebTab;
	C_WebTab* pSelectedWebTab;
	IMaterial* pMaterial;
	for (int x = 0; x < 1024; x++)
	{
		if (pMaterials[x] && pMaterials[x]->HasProxy())
		{
			pMaterial = pMaterials[x];
			pWebTab = m_pWebManager->FindWebTab(pMaterial);
			if (pWebTab)
			{
				pSelectedWebTab = m_pWebManager->GetSelectedWebTab();
				if (pSelectedWebTab)
					m_pWebManager->DeselectWebTab(pSelectedWebTab);

				// FIXME: Add some kind of material variable that could allow us to be skipped? We'll need something like that when WebImages are added.
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

bool C_AnarchyManager::DeselectEntity(C_BaseEntity* pEntity)
{
	C_WebTab* pWebTab = m_pWebManager->GetSelectedWebTab();
	if (pWebTab)
		m_pWebManager->DeselectWebTab(pWebTab);

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
	//m_pWebManager->FindWebTab("mainmenu")->SetUrl("http://localhost:8001/terminal.html");

	//DevMsg("Workshop manager is done, bra.\n");

	m_pWorkshopManager->MountAllWorkshops();
	//m_pMetaverseManager->LoadAllLocalItems();
}

void C_AnarchyManager::OnMountedAllWorkshop()
{
	m_pLoadingManager->Reset();

	m_pWebManager->GetSelectedWebTab()->SetUrl("asset://ui/welcome.html");
	g_pAnarchyManager->GetWebManager()->SelectWebTab(m_pWebManager->GetHudWebTab());
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true);
}

void C_AnarchyManager::Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters)
{
	std::string safeStr = str;
	std::transform(safeStr.begin(), safeStr.end(), safeStr.begin(), tolower);

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
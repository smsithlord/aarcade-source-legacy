#include "cbase.h"
#include "c_anarchymanager.h"
#include <cctype>
#include <algorithm>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_AnarchyManager g_AnarchyManager;
extern C_AnarchyManager* g_pAnarchyManager(&g_AnarchyManager);

C_AnarchyManager::C_AnarchyManager() : CAutoGameSystemPerFrame("C_AnarchyManager")
{
	DevMsg("AnarchyManager: Constructor\n");
	m_pWebManager = null;
	m_pLibretroManager = null;
	m_pInputManager = null;
	m_pSelectedEntity = null;
	m_pMountManager = null;
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

std::string C_AnarchyManager::GenerateUniqueId()
{
	DevMsg("AnarchyManager: GenerateUniqueId\n");

	// pseudo random pseudo unique ids until the firebase id generator can be ported to C++
	std::string id = "random";

	id += std::to_string(random->RandomInt(0, 10));
	id += std::to_string(random->RandomInt(0, 10));
	id += std::to_string(random->RandomInt(0, 10));
	id += std::to_string(random->RandomInt(0, 10));

	return id;
}

void C_AnarchyManager::AnarchyBegin()
{
	DevMsg("AnarchyManager: AnarchyBegin\n");
	m_pWebManager = new C_WebManager();
	m_pWebManager->Init();
	m_pInputManager = new C_InputManager();
}

void C_AnarchyManager::OnWebManagerReady()
{
	DevMsg("yarbles\n");

	m_pMountManager = new C_MountManager();
	m_pMountManager->Init();
	m_pMountManager->LoadMountsFromKeyValues("mounts.txt");

	m_pLibretroManager = new C_LibretroManager();

	C_WebTab* pWebTab = m_pWebManager->CreateWebTab("asset://ui/welcomemenu.html", "mainmenu", false);
	m_pWebManager->SelectWebTab(pWebTab);
	m_pInputManager->ActivateInputMode(true);

	DevMsg("great fuzzy blockos\n");

	//C_WebTab* pWebTab = pWebManager->CreateWebTab("http://localhost:8001/terminal.html", "metaverse");
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
#include "cbase.h"
#include "c_anarchymanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_AnarchyManager g_AnarchyManager;
extern C_AnarchyManager* g_pAnarchyManager(&g_AnarchyManager);

C_AnarchyManager::C_AnarchyManager() : CAutoGameSystemPerFrame("C_AnarchyManager")
{
	DevMsg("AnarchyManager: Constructor\n");
	m_pWebManager = null;

	// This section gets execute before there is a console
	// Certain things are not safe to do from here.
	// For example, initializing the WebCore (ie. creating a WebViewManager)
	// will cause the entire process to hang.

//	m_pWebViewManager = null;
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
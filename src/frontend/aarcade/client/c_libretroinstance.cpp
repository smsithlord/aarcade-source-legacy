// ;..\..\portaudio\lib\portaudio_x86.lib

#include "cbase.h"
#include "aa_globals.h"

//#include "aa_globals.h"
#include "c_libretroinstance.h"
#include "c_anarchymanager.h"
#include "../../../public/vgui_controls/Controls.h"
#include "vgui/IInput.h"
#include "c_canvasregen.h"
#include "c_embeddedinstance.h"

#include <algorithm>

#include "XUnzip.h"

#include <mutex>

#include <glm/glm.hpp>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_LibretroInstance::C_LibretroInstance()
{
	DevMsg("LibretroInstance: Constructor\n");
	m_pTexture = null;
	m_iLastRenderedFrame = -1;
	m_iLastVisibleFrame = -1;
	m_iOriginalEntIndex = -1;

	m_info = null;

	//m_pOverlayKV = null;
	m_pOverlayKV = new KeyValues("overlay");

	//{
		//std::string id = "PANASONIC.DARK.cfg";
		//if (id.length() < 5)
		//id = id.substr(0, id.length() - 4);
	//}
	//m_fPositionX = 0.25;// 0;
	//m_fPositionY = 0.25;// 0;
	//m_fSizeX = 0.5;// 1;
	//m_fSizeY = 0.5;// 1;
	//m_pOverlayKV->deleteThis();
//	m_pOpenGLManager = null;
}

C_LibretroInstance::~C_LibretroInstance()
{
	DevMsg("LibretroInstance: Destructor\n");
	this->CleanUpTexture();
}

void C_LibretroInstance::ClearOverlay(std::string type, std::string overlayId)
{
	std::string folder = "resource\\ui\\html\\overlays";
	g_pFullFileSystem->CreateDirHierarchy(folder.c_str(), "DEFAULT_WRITE_PATH");
	std::string file = VarArgs("%s\\%s.cfg", folder.c_str(), overlayId.c_str());

	//m_pOverlayKV
	std::string prettyCore = m_info->prettycore;
	std::string prettyGame = m_info->prettygame;
	KeyValues* pDefaultKV = m_pOverlayKV->FindKey("settings/default", true);
	KeyValues* pTargetKV = null;

	std::string testerCore;
	std::string testerGame;
	for (KeyValues *sub = m_pOverlayKV->FindKey("settings", true)->GetFirstSubKey(); sub; sub = sub->GetNextKey())
	{
		if (!Q_stricmp(sub->GetName(), "default"))
			continue;

		testerCore = sub->GetString("core");
		testerGame = sub->GetString("game");

		if (type == "core" && testerCore == prettyCore && testerGame == "")
			pTargetKV = sub;
		else if (testerCore == prettyCore && testerGame == prettyGame && type == "game")
		{
			pTargetKV = sub;
			break;
		}
	}

	if (pTargetKV)
	{
		pTargetKV->Clear();
		pTargetKV->SetString(null, "");

		if (!m_pOverlayKV->SaveToFile(g_pFullFileSystem, file.c_str(), "DEFAULT_WRITE_PATH"))
			DevMsg("ERROR: Could not wite file %s\n", file.c_str());

		this->SetOverlay(overlayId);
	}
}

void C_LibretroInstance::SaveOverlay(std::string type, std::string overlayId, float x, float y, float width, float height)
{
	/*
	if (overlayId == "none")
	{
		x = 0;
		y = 0;
		width = 1;
		height = 1;
	}
	*/

	std::string folder = "resource\\ui\\html\\overlays";
	g_pFullFileSystem->CreateDirHierarchy(folder.c_str(), "DEFAULT_WRITE_PATH");
	std::string file = VarArgs("%s\\%s.cfg", folder.c_str(), overlayId.c_str());

	//m_pOverlayKV
	std::string prettyCore = m_info->prettycore;
	std::string prettyGame = m_info->prettygame;
	std::string preferredOverlayId = g_pAnarchyManager->GetLibretroManager()->DetermineOverlay(prettyCore, prettyGame);
	KeyValues* pDefaultKV = m_pOverlayKV->FindKey("settings/default", true);
	KeyValues* pCoreKV = null;
	KeyValues* pGameKV = null;
	//KeyValues* pTargetKV = null;

	std::string testerCore;
	std::string testerGame;
	for (KeyValues *sub = m_pOverlayKV->FindKey("settings", true)->GetFirstSubKey(); sub; sub = sub->GetNextKey())
	{
		if (!Q_stricmp(sub->GetName(), "default"))
			continue;

		testerCore = sub->GetString("core");
		testerGame = sub->GetString("game");

		if (testerCore == prettyCore && testerGame == "")
			pCoreKV = sub;
		else if (testerCore == prettyCore && testerGame == prettyGame)
			pGameKV = sub;
		/*
		if ((type == "core" || type == "default")  && testerCore == prettyCore && testerGame == "")
			pTargetKV = sub;
		else if (testerCore == prettyCore && testerGame == prettyGame && (type == "game" || type == "default"))
		{
			pTargetKV = sub;
			break;
		}
		*/
	}

	if (type == "game")
	{
		if (!pGameKV)
		{
			pGameKV = m_pOverlayKV->FindKey("settings", true)->CreateNewKey();
			pGameKV->SetName("setting");
			pGameKV->SetString("core", prettyCore.c_str());
			pGameKV->SetString("game", prettyGame.c_str());
		}

		pGameKV->SetFloat("x", x);
		pGameKV->SetFloat("y", y);
		pGameKV->SetFloat("width", width);
		pGameKV->SetFloat("height", height);
	}
	else if (type == "core")
	{
		if (!pCoreKV)
		{
			pCoreKV = m_pOverlayKV->FindKey("settings", true)->CreateNewKey();
			pCoreKV->SetName("setting");
			pCoreKV->SetString("core", prettyCore.c_str());
		}

		pCoreKV->SetFloat("x", x);
		pCoreKV->SetFloat("y", y);
		pCoreKV->SetFloat("width", width);
		pCoreKV->SetFloat("height", height);
	}
	else if (type == "default")
	{
		pDefaultKV->SetFloat("x", x);
		pDefaultKV->SetFloat("y", y);
		pDefaultKV->SetFloat("width", width);
		pDefaultKV->SetFloat("height", height);
	}

	if (preferredOverlayId == overlayId)
	{
		m_pOverlayKV->SetFloat("current/x", x);
		m_pOverlayKV->SetFloat("current/y", y);
		m_pOverlayKV->SetFloat("current/width", width);
		m_pOverlayKV->SetFloat("current/height", height);
	}

	if (!m_pOverlayKV->SaveToFile(g_pFullFileSystem, file.c_str(), "DEFAULT_WRITE_PATH"))
		DevMsg("ERROR: Could not wite file %s\n", file.c_str());

	if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == this)
	{
		vgui::CInputSlate* pInputSlate = g_pAnarchyManager->GetInputManager()->GetInputSlate();
		if (pInputSlate)
			pInputSlate->AdjustOverlay(m_pOverlayKV->GetFloat("current/x", 0), m_pOverlayKV->GetFloat("current/y", 0), m_pOverlayKV->GetFloat("current/width", 1), m_pOverlayKV->GetFloat("current/height", 1), m_overlayId);
	}
}

void C_LibretroInstance::SetOverlay(std::string overlayId)
{
	if (!m_info)
		return;

	std::string goodOverlayId = overlayId;

	std::string prettyCore = m_info->prettycore;
	std::string prettyGame = m_info->prettygame;

	std::string preferredOverlayId = g_pAnarchyManager->GetLibretroManager()->DetermineOverlay(prettyCore, prettyGame);
	//return;
	//if (preferredOverlayId != goodOverlayId)
		//return;
	goodOverlayId = preferredOverlayId;

	if (m_pOverlayKV)
		m_pOverlayKV->Clear();

	//m_overlayId = goodOverlayId;
	if (goodOverlayId != "" && goodOverlayId != "none" && m_pOverlayKV->LoadFromFile(g_pFullFileSystem, VarArgs("resource\\ui\\html\\overlays\\%s.cfg", goodOverlayId.c_str()), "MOD"))
	{
		//m_overlayId = overlayId;
		//m_pOverlayKV->SetString("current/overlayId", overlayId.c_str());

		std::string testerCore;
		std::string testerGame;

		KeyValues* pDefaultOverlayKV = m_pOverlayKV->FindKey("settings/default");
		if (!pDefaultOverlayKV)
		{
			pDefaultOverlayKV = m_pOverlayKV->FindKey("settings/default", true);
			pDefaultOverlayKV->SetFloat("x", 0);
			pDefaultOverlayKV->SetFloat("y", 0);
			pDefaultOverlayKV->SetFloat("width", 1);
			pDefaultOverlayKV->SetFloat("height", 1);
		}

		//KeyValues* pBestOverlayKV = null;
		KeyValues* pCoreOverlayKV = null;
		KeyValues* pGameOverlayKV = null;
		for (KeyValues *sub = m_pOverlayKV->FindKey("settings", true)->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		{
			if (sub == pDefaultOverlayKV)
				continue;

			testerCore = sub->GetString("core");
			testerGame = sub->GetString("game");

			if (testerCore == prettyCore && testerGame == "")
				pCoreOverlayKV = sub;
			else if (testerCore == prettyCore && testerGame == prettyGame)
				pGameOverlayKV = sub;
		}

		KeyValues* pBestOverlayKV = (pGameOverlayKV) ? pGameOverlayKV : pCoreOverlayKV;
		if (!pBestOverlayKV)
			pBestOverlayKV = pDefaultOverlayKV;

		if (preferredOverlayId == goodOverlayId)
		{
			m_pOverlayKV->SetFloat("current/x", pBestOverlayKV->GetFloat("x", 0));
			m_pOverlayKV->SetFloat("current/y", pBestOverlayKV->GetFloat("y", 0));
			m_pOverlayKV->SetFloat("current/width", pBestOverlayKV->GetFloat("width", 1));
			m_pOverlayKV->SetFloat("current/height", pBestOverlayKV->GetFloat("height", 1));
		}
	}
	else
	{
		m_pOverlayKV->SetFloat("current/x", 0);
		m_pOverlayKV->SetFloat("current/y", 0);
		m_pOverlayKV->SetFloat("current/width", 1);
		m_pOverlayKV->SetFloat("current/height", 1);
	}

	if ( g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == this)
	{
		vgui::CInputSlate* pInputSlate = g_pAnarchyManager->GetInputManager()->GetInputSlate();
		if (pInputSlate)
			pInputSlate->AdjustOverlay(m_pOverlayKV->GetFloat("current/x", 0), m_pOverlayKV->GetFloat("current/y", 0), m_pOverlayKV->GetFloat("current/width", 1), m_pOverlayKV->GetFloat("current/height", 1), goodOverlayId);
	}

	m_overlayId = goodOverlayId;
}

void C_LibretroInstance::SelfDestruct()
{
	DevMsg("LibretroInstance: SelfDestruct\n");
	g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen()->NotifyInstanceAboutToDie(this);

	if (m_info)
	{
		m_info->libretroinstance = null;
		m_info->close = true;
	}

	if (m_pOverlayKV)
		m_pOverlayKV->deleteThis();
	/*
	if (m_pLastFrameData)
		free(m_pLastFrameData);

	if (m_pPostData)
		free(m_pPostData);
	*/

	delete this;
}

void C_LibretroInstance::CleanUpTexture()
{
	if (m_pTexture)
	{
		m_pTexture->SetTextureRegenerator(null);
		g_pAnarchyManager->GetCanvasManager()->UnreferenceEmbeddedInstance(this);
		g_pAnarchyManager->GetCanvasManager()->UnreferenceTexture(m_pTexture);
		g_pAnarchyManager->GetCanvasManager()->DoOrDeferTextureCleanup(m_pTexture);
		m_pTexture = null;
	}
}

void C_LibretroInstance::Init(std::string id, std::string title, int iEntIndex)
{
	std::string goodTitle = (title != "") ? title : "Untitled Libretro Tab";
	m_title = goodTitle;
	m_id = id;
	if (m_id == "")
		m_id = g_pAnarchyManager->GenerateUniqueId();

	m_iOriginalEntIndex = iEntIndex;

	// create the texture (each instance has its own texture)
	std::string textureName = "canvas_";
	textureName += m_id;

	int iWidth = (id == "hud") ? AA_HUD_INSTANCE_WIDTH : AA_EMBEDDED_INSTANCE_WIDTH;
	int iHeight = (id == "hud") ? AA_HUD_INSTANCE_HEIGHT : AA_EMBEDDED_INSTANCE_HEIGHT;
	//int iWidth = 1920;
	//int iHeight = 1080;

	//m_pTexture = g_pMaterialSystem->FindTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, false, 1);

	//if (!m_pTexture)

	int multiplyer = g_pAnarchyManager->GetDynamicMultiplyer();
	if (!g_pMaterialSystem->IsTextureLoaded(textureName.c_str()))
		m_pTexture = g_pMaterialSystem->CreateProceduralTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, iWidth * multiplyer, iHeight * multiplyer, IMAGE_FORMAT_BGR888, 1);
	else
	{
		m_pTexture = g_pMaterialSystem->FindTexture(textureName.c_str(), TEXTURE_GROUP_VGUI, false, 1);
		g_pAnarchyManager->GetCanvasManager()->TextureNotDeferred(m_pTexture);
	}

	// get the regen and assign it
	CCanvasRegen* pRegen = g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen();
	//pRegen->SetEmbeddedInstance(this);
	m_pTexture->SetTextureRegenerator(pRegen);

	m_raw = new libretro_raw();
}

void C_LibretroInstance::Update()
{
	if (g_pAnarchyManager->GetSuspendEmbedded())
		return;

	if (m_info->state == 1)
	{
		OnCoreLoaded();
	}
	else if (m_info->state == 5 )// && m_info->audiostream)	// added m_info to try and detect failed video loads!! (FIXME: Should be removed after proper failed video load is added elsewhere.
	{
		unsigned int numPorts = m_info->numports;
		if (numPorts == 0)
		{
			// how the funnuck are we supposd to know how many input ports need to be held in the input back buffer if the core doesn't tell us??  just assume 1 for now.
			//DevMsg("WARNING: zero retro ports are active.\n");
			numPorts = 1;
		}

		for (unsigned int i = 0; i < numPorts; i++)
			g_pAnarchyManager->GetLibretroManager()->ManageInputUpdate(m_info, i, RETRO_DEVICE_JOYPAD);

		/*
		// update input state
		m_info->inputstate["RETRO_DEVICE_ID_JOYPAD_SELECT"] = vgui::input()->IsKeyDown(KEY_XBUTTON_BACK);
		m_info->inputstate["RETRO_DEVICE_ID_JOYPAD_START"] = vgui::input()->IsKeyDown(KEY_XBUTTON_START) || vgui::input()->IsKeyDown(KEY_ENTER);
		m_info->inputstate["RETRO_DEVICE_ID_JOYPAD_UP"] = vgui::input()->IsKeyDown(KEY_XBUTTON_UP);
		m_info->inputstate["RETRO_DEVICE_ID_JOYPAD_DOWN"] = vgui::input()->IsKeyDown(KEY_XBUTTON_DOWN);
		m_info->inputstate["RETRO_DEVICE_ID_JOYPAD_LEFT"] = vgui::input()->IsKeyDown(KEY_XBUTTON_LEFT);
		m_info->inputstate["RETRO_DEVICE_ID_JOYPAD_RIGHT"] = vgui::input()->IsKeyDown(KEY_XBUTTON_RIGHT);
		m_info->inputstate["RETRO_DEVICE_ID_JOYPAD_A"] = vgui::input()->IsKeyDown(KEY_XBUTTON_B);
		m_info->inputstate["RETRO_DEVICE_ID_JOYPAD_B"] = vgui::input()->IsKeyDown(KEY_XBUTTON_A);
		m_info->inputstate["RETRO_DEVICE_ID_JOYPAD_X"] = vgui::input()->IsKeyDown(KEY_XBUTTON_Y);
		m_info->inputstate["RETRO_DEVICE_ID_JOYPAD_Y"] = vgui::input()->IsKeyDown(KEY_XBUTTON_X);
		m_info->inputstate["RETRO_DEVICE_ID_JOYPAD_L"] = vgui::input()->IsKeyDown(KEY_XBUTTON_LEFT_SHOULDER);
		m_info->inputstate["RETRO_DEVICE_ID_JOYPAD_R"] = vgui::input()->IsKeyDown(KEY_XBUTTON_RIGHT_SHOULDER);
		*/

		this->OnProxyBind(null);

		/*
		if (m_raw)
		{
			DevMsg("LibretroInstance: Update\n");
			//m_raw->run();
			DevMsg("after\n");
		}
		*/
	}
	else if (m_info->state == 6)
	{
		// even though the core wants to close, don't do anything at all.  the core will keep waiting until the user closes this C_LibretroInstance like normal.
	}
}

bool C_LibretroInstance::LoadCore(std::string coreFile)
{
	if (coreFile != "")
	{
		// first check for cores in the user folder, then check for cores in the frontend folder.

		bool bReady = false;
		std::string core = g_pAnarchyManager->GetLibretroManager()->GetLibretroPath(RETRO_USER_BASE) + g_pAnarchyManager->GetLibretroManager()->GetLibretroPath(RETRO_CORE_PATH) + std::string("\\") + coreFile;
		if (g_pFullFileSystem->FileExists(core.c_str()))
			bReady = true;
		else
		{
			core = engine->GetGameDirectory() + g_pAnarchyManager->GetLibretroManager()->GetLibretroPath(RETRO_CORE_PATH) + std::string("\\") + coreFile;
			if (g_pFullFileSystem->FileExists(core.c_str()))
				bReady = true;
		}

		if ( bReady )
		{
			//g_pAnarchyManager->AddToastMessage(VarArgs("Libretro Core Opened (%i running)", g_pAnarchyManager->GetLibretroManager()->GetInstanceCount()));
			CreateWorkerThread(core);
			return true;
		}
	}

	g_pAnarchyManager->AddToastMessage("Libretro Core Aborted");
	return false;
	/*

	//	struct retro_system_info info;
	//	s_raw->get_system_info(&info);

	Msg("Successfully loaded libretro module at %s in %i milliseconds.\n", pFilename, pClientArcadeResources->GetSystemTime() - startTime);
	s_bCoreIsLoaded = true;
	*/
}

void C_LibretroInstance::OnGameLoaded()
{
	DevMsg("Game finished loading.\n");
	m_info->state = 5;
}


std::string C_LibretroInstance::GetLibretroCore()
{
	if (m_info)
		return m_info->core;
	else
		return "";
}

std::string C_LibretroInstance::GetLibretroFile()
{
	if (m_info)
		return m_info->game;
	else
		return "";
}

void C_LibretroInstance::SetReset(bool bValue)
{
	if (m_info)
		m_info->reset = bValue;
}

void C_LibretroInstance::SetPause(bool bValue)
{
	if (m_info)
		m_info->paused = bValue;
}

void C_LibretroInstance::SetVolume(float fVolume)
{
	if (m_info)
		m_info->volume = fVolume;
}

bool C_LibretroInstance::GetPause()
{
	if (!m_info)
		return false;
	else
		return m_info->paused;
}

bool C_LibretroInstance::LoadGame()
{
	uint uId = ThreadGetCurrentId();
	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);

	if (!pLibretroInstance)
		return false;

	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();

	std::string filename = info->game;

	// If this core *requires* a full file path, then we can check if the file extension is supported RIGHT NOW.
	// OTHERWISE, we might have to open up a ZIP file before we can check the real file extension.

	std::string fileExtension = filename;
	std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), ::tolower);
	size_t extensionFound = fileExtension.find_last_of(".");
	if (extensionFound != std::string::npos)
		fileExtension = fileExtension.substr(extensionFound + 1);
	else
		fileExtension = "";

	if (fileExtension == "")
	{
		DevMsg("libretro: ABORTED: The file has no file extension.\n");
		//info->close = true;
		return false;
	}

	// Format Example:
	//   valid_extensions: mkv|avi|f4v|f4f|3gp|ogm|flv|mp4|mp3|flac|ogg|m4a
	std::string testerExtensions = info->valid_extensions;
	std::transform(testerExtensions.begin(), testerExtensions.end(), testerExtensions.begin(), ::tolower);

	std::vector<std::string> tokens;
	g_pAnarchyManager->Tokenize(testerExtensions, tokens, "|");

	bool bIsZip = (fileExtension == "zip");
	bool bIsValidExtension = true;

	// If this is NOT a ZIP (or if ZIP is a supported file extension for the core, OR(?) if the core requires fullpath) we can confirm validity RIGHT NOW.
	if (info->need_fullpath || !bIsZip || std::find(tokens.begin(), tokens.end(), "zip") != tokens.end())	//info->need_fullpath
	{
		if (std::find(tokens.begin(), tokens.end(), fileExtension) != tokens.end())
			DevMsg("Found extension %s within %s\n", fileExtension.c_str(), testerExtensions.c_str());
		else
			bIsValidExtension = false;
	}

	// FIXME: Why always pick this pixel format???
	info->videoformat = RETRO_PIXEL_FORMAT_0RGB1555;

	//s_bSupportsNoGame

	void* fileData;
	bool bDataLoaded = false;
	bool bReadyToLoad = false;

	struct retro_game_info game;
	game.path = filename.c_str();

	if (!bIsValidExtension)
	{
		DevMsg("libretro: ABORTED: Invalid file extension %s for this core. Valid extensions for %s are: %s\n", fileExtension.c_str(), info->prettycore.c_str(), testerExtensions.c_str());
		//info->close = true;
	}
	else
	{
		if (info->need_fullpath)
		{
			game.data = NULL;
			game.size = 0;
			game.meta = NULL;

			bReadyToLoad = true;
		}
		else
		{
			DevMsg("libretro: File must be loaded by frontend!\n");

			// for easy char string access
			char pFilename[AA_MAX_STRING];
			Q_strcpy(pFilename, filename.c_str());

			if (bIsZip)
			{
				DevMsg("libretro: ZIP file detected. Attempting to extract the 1st file..\n");

				bool bFailedUnzip = false;
				if (!g_pFullFileSystem->FileExists(filename.c_str()))
				{
					DevMsg("libretro: ABORTED: ZIP file does not exist %s\n", pFilename);
					bFailedUnzip = true;
				}
				else
				{
					HZIP hz = OpenZip(pFilename, 0, ZIP_FILENAME);
					if (!hz)
					{
						DevMsg("libretro: ABORTED: Failed to open ZIP file %s\n", pFilename);
						bFailedUnzip = true;
					}
					else
					{
						int zipIndex = 0;
						ZIPENTRY zipEntry;
						ZRESULT result = GetZipItem(hz, zipIndex, &zipEntry);

						std::string entryTesterExtension;
						size_t entryExtensionFound;
						bool bFoundFile = false;
						while (result == ZR_OK)
						{
							if (zipEntry.attr & FILE_ATTRIBUTE_DIRECTORY)
							{
								zipIndex++;
								result = GetZipItem(hz, zipIndex, &zipEntry);
								continue;
							}

							if (testerExtensions == "")
								bFoundFile = true;
							else
							{
								entryTesterExtension = zipEntry.name;
								std::transform(entryTesterExtension.begin(), entryTesterExtension.end(), entryTesterExtension.begin(), ::tolower);
								entryExtensionFound = entryTesterExtension.find_last_of(".");
								if (entryExtensionFound != std::string::npos)
									entryTesterExtension = entryTesterExtension.substr(entryExtensionFound + 1);
								else
									entryTesterExtension = "";

								if (entryTesterExtension != "" && std::find(tokens.begin(), tokens.end(), entryTesterExtension) != tokens.end())
									bFoundFile = true;
							}

							if (bFoundFile)
								break;
						}

						if (!bFoundFile || result != ZR_OK)
						{
							DevMsg("libretro: ABORTED: Failed to locate a valid file in ZIP.");
							bFailedUnzip = true;
						}
						else
						{
							long fileSize = zipEntry.unc_size;
							fileData = malloc(fileSize);
							bDataLoaded = true;

							result = UnzipItem(hz, zipIndex, fileData, fileSize, ZIP_MEMORY);

							if (result != ZR_OK && result != ZR_MORE)
							{
								DevMsg("libretro: ABORTED: Failed to unzip the file. ERROR CODE %i\n", result);
								bFailedUnzip = true;
							}
							else
							{
								game.data = fileData;
								game.size = fileSize;
								game.meta = NULL;

								bReadyToLoad = true;
							}
						}

						CloseZip(hz);
					}
				}

				// if bFailedUnzip is false, we have failed to unzip.
			}
			else
			{
				game.data = NULL;
				game.size = 0;
				game.meta = NULL;

				FileHandle_t fileHandle = filesystem->Open(pFilename, "rb");
				if (!fileHandle)
				{
					DevMsg("libretro: ABORTED: Failed to open file %s\n", pFilename);
					//info->close = true;
				}
				else
				{
					int bufferSize = filesystem->Size(fileHandle);
					fileData = malloc(bufferSize);
					bDataLoaded = true;

					filesystem->Read(fileData, bufferSize, fileHandle);
					filesystem->Close(fileHandle);

					game.data = fileData;
					game.size = bufferSize;

					bReadyToLoad = true;
				}
			}
		}
	}

	bool bSuccess = false;
	if (bReadyToLoad)
	{
		// load any existing save state
		//if (g_pFullFileSystem->FileExists(VarArgs("%s\\%s\\%s.sav", info->savepath.c_str(), info->prettycore.c_str(), info->prettygame.c_str())))
		//{
		if (info->settings && info->settings->GetBool("statesaves"))
		{
			FileHandle_t fileHandle = filesystem->Open(VarArgs("%s\\%s\\%s.sav", info->savepath.c_str(), info->prettycore.c_str(), info->prettygame.c_str()), "rb", "");
			if (fileHandle)
			{
				info->statesize = filesystem->Size(fileHandle);	// statesize remains ZERO if game was loaded state saves disabled.  this prevents erroneous cleanup.
				info->statedata = malloc(info->statesize);
				filesystem->Read(info->statedata, info->statesize, fileHandle);
				filesystem->Close(fileHandle);
			}
		}
		//}

		info->raw->init();	// could take a while

		if (info->close)	// somebody else could have closed us from a different thread while we were doing that bottleneck above
		{
			DevMsg("libretro: ABORTED: Canceled before loading game.\n");
			//info->close = true;
		}
		else
		{
			if (!info->raw->load_game(&game))
			{
				DevMsg("libretro: ABORTED: Core could not load game.\n");
				//info->close = true;
			}
			else
			{
				DevMsg("libretro: Finished loading game %s\n", filename.c_str());

				if (!info->close)
				{
					info->state = 5;
					bSuccess = true;
				}
				else
				{
					DevMsg("libretro: ABORTED: Canceled while loading game.\n");
					bSuccess = false;
					//info->close = true;
				}
			}
		}
	}

	if (bDataLoaded)
		free(fileData);

	return bSuccess;
}

void C_LibretroInstance::OnCoreLoaded()
{
	DevMsg("Core finished loading!\n");
	m_info->state = 2;

	// automatically load a game right away...
//	m_info->game = "V:/Movies/Flash Gordon (1980).avi";//file
	//m_info->game = "V:/Movies/Jay and silent Bob Strike Back (2001).avi";
	//"V:\\Movies\\Judge Dredd (1995).mp4";
}

//bool C_LibretroManager::BuildInterface(void* pLib, struct libretro_raw * myInterface)
bool C_LibretroInstance::BuildInterface(libretro_raw* raw, void* pLib)
{
	HMODULE hModule = *static_cast<HMODULE*>(pLib);

	//void(*get_system_info)(struct retro_system_info * info);

	// check if this is a libretro core...
	if (!GetProcAddress(hModule, "retro_get_system_info"))
		return false;

	raw->set_environment = (void(*)(retro_environment_t))GetProcAddress(hModule, "retro_set_environment");
	raw->set_video_refresh = (void(*)(retro_video_refresh_t))GetProcAddress(hModule, "retro_set_video_refresh");
	raw->set_audio_sample = (void(*)(retro_audio_sample_t))GetProcAddress(hModule, "retro_set_audio_sample");
	raw->set_audio_sample_batch = (void(*)(retro_audio_sample_batch_t))GetProcAddress(hModule, "retro_set_audio_sample_batch");
	raw->set_input_poll = (void(*)(retro_input_poll_t))GetProcAddress(hModule, "retro_set_input_poll");
	raw->set_input_state = (void(*)(retro_input_state_t))GetProcAddress(hModule, "retro_set_input_state");
	raw->init = (void(*)(void))GetProcAddress(hModule, "retro_init");
	raw->deinit = (void(*)(void))GetProcAddress(hModule, "retro_deinit");
	raw->api_version = (unsigned(*)(void))GetProcAddress(hModule, "retro_api_version");
	raw->get_system_info = (void(*)(struct retro_system_info*))GetProcAddress(hModule, "retro_get_system_info");
	raw->get_system_av_info = (void(*)(struct retro_system_av_info*))GetProcAddress(hModule, "retro_get_system_av_info");
	raw->set_controller_port_device = (void(*)(unsigned, unsigned))GetProcAddress(hModule, "retro_set_controller_port_device");
	raw->reset = (void(*)(void))GetProcAddress(hModule, "retro_reset");
	raw->run = (void(*)(void))GetProcAddress(hModule, "retro_run");
	raw->serialize_size = (size_t(*)(void))GetProcAddress(hModule, "retro_serialize_size");
	raw->serialize = (bool(*)(void* data, size_t size))GetProcAddress(hModule, "retro_serialize");
	raw->unserialize = (bool(*)(const void* data, size_t size))GetProcAddress(hModule, "retro_unserialize");
	raw->cheat_reset = (void(*)(void))GetProcAddress(hModule, "retro_cheat_reset");
	raw->cheat_set = (void(*)(unsigned, bool, const char*))GetProcAddress(hModule, "retro_cheat_set");
	raw->load_game = (bool(*)(const struct retro_game_info *))GetProcAddress(hModule, "retro_load_game");
	raw->load_game_special = (bool(*)(unsigned, const struct retro_game_info*, size_t))GetProcAddress(hModule, "retro_load_game_special");
	raw->unload_game = (void(*)(void))GetProcAddress(hModule, "retro_unload_game");
	raw->get_region = (unsigned(*)(void))GetProcAddress(hModule, "retro_get_region");
	raw->get_memory_data = (void*(*)(unsigned))GetProcAddress(hModule, "retro_get_memory_data");
	raw->get_memory_size = (size_t(*)(unsigned))GetProcAddress(hModule, "retro_get_memory_size");
	return true;
}

//float lastAudioFrame = 0;
/*
typedef float SAMPLE;
static int audiocallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	uint uId = ThreadGetCurrentId();
	//DevMsg("Audio Thread ID: %u\n", uId);

	LibretroInstanceInfo_t* info = (LibretroInstanceInfo_t*)userData;
	//	DevMsg("Pos: %i\n", info->audiobufferpos);
	if (info->audiobufferpos < info->audiobuffersize)
	{
		info->processingaudio = false;
		return paContinue;
	}
	//if (info->processingaudio)
	//		return paContinue;

	//	if (!inputBuffer)
	//	return paContinue;

	//	if (lastAudioFrame != gpGlobals->framecount)
	//{
	//		lastAudioFrame = gpGlobals->framecount;
	//		lastFrameNumber = gpGlobals->framecount;

	//Q_memcpy(outputBuffer, info->audiobuffer, info->audiobufferframes * 2 * sizeof(int16_t));

	//int16_t* copyBuffer = new int16_t[info->audiobufferpos * 2];
	//	Q_memcpy(copyBuffer, info->audiobuffer, info->audiobufferpos * 2 * sizeof(int16_t));

	Q_memcpy(outputBuffer, info->audiobuffer, info->audiobufferpos * 2 * sizeof(int16_t));
	info->audiobufferpos = 0;
	info->processingaudio = false;
	//	}

	return paContinue;
	//Q_memcpy(outputBuffer, inputBuffer, framesPerBuffer * 2 * sizeof(int16_t));
	*/
	/*
	unsigned int i;
	SAMPLE *out = (SAMPLE*)outputBuffer;
	if (!inputBuffer)
	{
	for (i = 0; i < framesPerBuffer; i++)
	{
	*out++ = 0;
	*out++ = 0;
	}
	}
	else
	{
	const SAMPLE *in = (const SAMPLE*)inputBuffer;
	for (int i = 0; i < framesPerBuffer; i++)
	{
	*out++ = *in++;
	*out++ = *in++;
	}
	}

	return paContinue;
	*/
//}

void C_LibretroInstance::CreateAudioStream()
{
	uint uId = ThreadGetCurrentId();
	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);
	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();

	DevMsg("Sample rate is: %i\n", info->samplerate);

	PaStreamParameters outputParameters;
	outputParameters.device = Pa_GetDefaultOutputDevice(); // default output device

	if (outputParameters.device == -1)
	{
		DevMsg("FAILED TO GET PORT AUDIO DEVICE!! PREPARE FOR CRASH!\n");
	}

	outputParameters.channelCount = 2;
	outputParameters.sampleFormat = paInt16;
	//outputParameters.suggestedLatency = 0.032;// 0.064;// Pa_GetDeviceInfo(outputParameters.device)->defaultHighOutputLatency;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultHighOutputLatency;	// FIXME: Add better error handling here!! sometimes this can't be called!!
	//outputParameters.suggestedLatency = 1 / (info->framerate * 1.0);
	outputParameters.hostApiSpecificStreamInfo = NULL;

	PaStream *stream;
	PaError err = Pa_OpenStream(
		&stream,
		NULL,
		&outputParameters,
		info->samplerate,
		paFramesPerBufferUnspecified,
		paNoFlag,//paClipOff,      // we won't output out of range samples so don't bother clipping them
		null, //audiocallback no callback, use blocking API
		info); // no callback, so no callback userData

	info->audiostream = stream;

	//info->audiobuffersize = 1024;
	//info->audiobuffer = new int16_t[info->audiobuffersize];
	//info->audiobufferpos = 0;

	//	info->safebuffersize = info->audiobuffersize;
	//	info->safebuffer = new int16_t[info->safebuffersize];
	//	info->safebufferpos = 0;


	if (err != paNoError)
		DevMsg("Failed to open stream.\n");
	else
		DevMsg("Opened PA stream!\n");

	err = Pa_StartStream(stream);

	if (err != paNoError)
		DevMsg("Failed to start stream.\n");
	else
		DevMsg("Started stream!\n");
}

void C_LibretroInstance::DestroyAudioStream()
{
	uint uId = ThreadGetCurrentId();
	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);
	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();

	PaStream *stream = static_cast<PaStream*>(info->audiostream);

	PaError err = Pa_StopStream(stream);

	if (err != paNoError)
		DevMsg("Failed to stop stream.\n");
	else
		DevMsg("Stopped PA stream!\n");

	err = Pa_CloseStream(stream);
	if (err != paNoError)
		DevMsg("Failed to close stream.\n");
	else
		DevMsg("Closed PA stream!\n");

	info->samplerate = 0;
	//	info->audiobufferpos = 0;
}

unsigned MyThread(void *params)
{
	bool bDidRun = false;

	LibretroInstanceInfo_t* info = (LibretroInstanceInfo_t*)params; // always use a struct!

	CSysModule* pModule;
	bool bDidLoadDll = false;
	if (info->libretroinstance && !info->close)
	{
		bool bDidLoadModule = false;
		//HMODULE	hModule;
		pModule = Sys_LoadModule(info->core.c_str());
		if (!pModule)
		{
			DevMsg("libretro: ERROR - Failed to load %s\n", info->core.c_str());
			info->runninglibretrocores->last_error = "Core Load Failed";
			info->state = 6;
		}
		else
		{
			bDidLoadDll = true;
			HMODULE hModule = reinterpret_cast<HMODULE>(pModule);
			if (!hModule || !C_LibretroInstance::BuildInterface(info->raw, &hModule))
			{
				DevMsg("libretro: ERROR - Failed to build interface!\n");
				info->runninglibretrocores->last_error = "Core Initialization Failed";
				info->state = 6;
			}
			else
			{
				bDidLoadModule = true;

				info->module = pModule;
				info->threadid = ThreadGetCurrentId();
				info->coreloaded = true;

				struct retro_system_info system_info;
				info->raw->get_system_info(&system_info);

				info->library_name = system_info.library_name;
				info->library_version = system_info.library_version;
				info->valid_extensions = system_info.valid_extensions;
				info->need_fullpath = system_info.need_fullpath;
				info->block_extract = system_info.block_extract;

				DevMsg("Loaded libretro core:\n");
				DevMsg("\tlibrary_name: %s\n", info->library_name.c_str());
				DevMsg("\tlibrary_version: %s\n", info->library_version.c_str());
				DevMsg("\tvalid_extensions: %s\n", info->valid_extensions.c_str());
				DevMsg("\tneed_fullpath: %i\n", info->need_fullpath);
				DevMsg("\tblock_extract: %i\n", info->block_extract);

				g_pAnarchyManager->GetLibretroManager()->OnLibretroInstanceCreated(info);	// FIXME: If instance is closed by the time this line is reached, might cause the crash!

				DevMsg("Thread: core loaded.\n");
			}
		}

		int state;
		libretro_raw* raw = info->raw;
		while (!info->close)	//&& glfwWindowShouldClose(window) == 0
		{
			//	glfwPollEvents();
			state = info->state;

			if (info->window && glfwWindowShouldClose(info->window))
				info->close = true;

			if (state == 2)
			{
				CSysModule* myModule = Sys_LoadModule(VarArgs("%s\\bin\\portaudio_x86.dll", engine->GetGameDirectory()));
				if (myModule)
				{
					DevMsg("portaudio_x86.dll loaded successfully.\n");

					PaError err = Pa_Initialize();
					if (err != paNoError)
						DevMsg("Failed to initialize PA.\n");
					else
						DevMsg("Initialized PA successfuly!\n");
				}
				else
					DevMsg("Failed to load portaudio_x86.dll.\n");

				info->state = 3;
			}
			else if (state == 3)
			{
				//raw->set_hw_context_reset(C_LibretroInstance::cbHWContextReset);
				raw->set_environment(C_LibretroInstance::cbEnvironment);
				raw->set_video_refresh(C_LibretroInstance::cbVideoRefresh);
				raw->set_audio_sample(C_LibretroInstance::cbAudioSample);
				raw->set_audio_sample_batch(C_LibretroInstance::cbAudioSampleBatch);
				raw->set_input_poll(C_LibretroInstance::cbInputPoll);
				if (raw->set_input_state)
					raw->set_input_state(C_LibretroInstance::cbInputState);
				info->state = 4;
			}
			else if (state == 4)
			{
				// load a game if we have one
				if (info->game != "")
				{
					DevMsg("Load the game next!!\n");
					//info->state = 5;
					if (C_LibretroInstance::LoadGame())
					{
						if (info->state == 5)
						{
							// setup the memory map prior to the 1st call to run

							/*
							info->memorymap->rtcsize = info->raw->get_memory_size(RETRO_MEMORY_RTC);
							if (info->memorymap->rtcsize > 0)
							{
							info->memorymap->rtcdata = (uint8_t*)info->raw->get_memory_data(RETRO_MEMORY_RTC);

							// data must be altered PRIOR to the 1st run
							// TODO: work
							}
							*/
							if (true)
							{
								info->memorymap->saveramsize = info->raw->get_memory_size(RETRO_MEMORY_SAVE_RAM);
								if (info->memorymap->saveramsize > 0)
								{
									info->memorymap->saveramdata = (uint8_t*)info->raw->get_memory_data(RETRO_MEMORY_SAVE_RAM);

									// data must be altered PRIOR to the 1st run
									if (info->settings && info->settings->GetBool("cartsaves") && g_pFullFileSystem->FileExists(VarArgs("%s\\%s\\%s.srm", info->savepath.c_str(), info->prettycore.c_str(), info->prettygame.c_str())))
									{
										FileHandle_t fileHandle = filesystem->Open(VarArgs("%s\\%s\\%s.srm", info->savepath.c_str(), info->prettycore.c_str(), info->prettygame.c_str()), "rb", "");
										if (fileHandle)
										{
											filesystem->Read((void*)info->memorymap->saveramdata, info->memorymap->saveramsize, fileHandle);
											filesystem->Close(fileHandle);
										}
									}
								}
							}

							/*
							info->memorymap->systemramsize = info->raw->get_memory_size(RETRO_MEMORY_SYSTEM_RAM);
							if (info->memorymap->systemramsize > 0)
							{
							info->memorymap->systemramdata = (uint8_t*)info->raw->get_memory_data(RETRO_MEMORY_SYSTEM_RAM);

							// data must be altered PRIOR to the 1st run
							// TODO: work
							}

							info->memorymap->videoramsize = info->raw->get_memory_size(RETRO_MEMORY_VIDEO_RAM);
							if (info->memorymap->videoramsize > 0)
							{
							info->memorymap->videoramdata = (uint8_t*)info->raw->get_memory_data(RETRO_MEMORY_VIDEO_RAM);

							// data must be altered PRIOR to the 1st run
							// TODO: work
							}
							*/

							//DevMsg("Pt A\n");
							info->raw->run();	// complete the game loading by executing 1 run


							// remember old state size
							if (info->settings && info->settings->GetBool("statesaves"))
							{
								// get current state size
								size_t currentStateSize = raw->serialize_size();

								size_t oldStateSize = info->statesize;

								// handle auto-loaded save states
								if (oldStateSize != 0)
								{
									if (oldStateSize != currentStateSize)
									{
										free(info->statedata);
										info->statedata = malloc(currentStateSize);
										raw->serialize(info->statedata, currentStateSize);	// so it's never empty or garbage
									}
									else if (oldStateSize == currentStateSize)
									{
										// load the state already contained within statedata
										if( raw->unserialize(info->statedata, oldStateSize) )
											info->runninglibretrocores->last_msg = "State Loaded";
									}
								}
								else
								{
									info->statedata = malloc(currentStateSize);
									raw->serialize(info->statedata, currentStateSize);	// so it's never empty or garbage
								}

								// *always* accept the statesize provided by the core... UNLESS we have state saves disabled, then statesize MUST remain zero to avoid erroneous cleanup
								info->statesize = currentStateSize;
							}

							bDidRun = true;

							//DevMsg("Pt B\n");
						}
					}
					else
					{
						info->runninglibretrocores->last_error = "Game Load Failed";
						info->state = 6;
					}
					//	DevMsg("cuatro\n");
				}
			}
			else if (state == 5)
			{
				if (info->reset)
				{
					info->reset = false;
					info->paused = false;
					info->raw->reset();
				}
				else if (!info->paused)
				{
					/*
					if (!info->processingaudio)
					{
					info->processingaudio = true;
					info->raw->run();
					}
					*/
					//DevMsg("Pt C\n");
					info->raw->run();

					if (AA_LIBRETRO_3D)
					{
						info->lastrendered = gpGlobals->curtime;

						if (info->readyfornextframe && !info->copyingframe)
						{
							info->readyfornextframe = false;
							info->readytocopyframe = false;

							if (info->samplerate == 0)
							{
								DevMsg("Get AV info\n");
								struct retro_system_av_info avinfo;
								info->raw->get_system_av_info(&avinfo);

								if (avinfo.timing.sample_rate > 0)
								{
									info->samplerate = int(avinfo.timing.sample_rate);
									info->framerate = int(avinfo.timing.fps);
									C_LibretroInstance::CreateAudioStream();
								}
							}


							//glDisable(GL_DEPTH_TEST); // here for illustrative purposes, depth test is initially DISABLED (key!)
							//glClearColor(0.3f, 0.4f, 0.1f, 1.0f);
							//glClear(GL_COLOR_BUFFER_BIT);
							//glfwMakeContextCurrent(info->window);
							//glfwSwapBuffers(info->window);
							//glfwPollEvents();	// this is what makes the window actually be responsive


							//WORD red_mask = 0xF800;
							//WORD green_mask = 0x7E0;
							//WORD blue_mask = 0x1F;

							//DevMsg("Doin it\n");

							//DevMsg("video refresh\n");

							unsigned int width = info->lastframewidth;
							unsigned int height = info->lastframeheight;
							size_t pitch = info->lastframepitch;
							if (!info->lastframedata)
								info->lastframedata = malloc(pitch*height);
							//void* dest = malloc(pitch*height);
							if (AA_LIBRETRO_3D && info->context_type != RETRO_HW_CONTEXT_NONE)
							{
								//DevMsg("Format is: %i %i x %i\n", info->videoformat, pitch, height);
								//glfwSwapBuffers(info->window);
								glReadPixels(0, 0, pitch / 3, height, GL_RGB, GL_UNSIGNED_BYTE, info->lastframedata);// GL_RGBA8
								//glfwSwapBuffers(info->window);
							}

							//if (info->lastframedata)
								//free(info->lastframedata);

							//info->lastframedata = dest;
							info->readytocopyframe = true;
						}
					}
					/*
					if (info->window)
					{
					//DevMsg("Pt 1\n");
					// background color
					glDisable(GL_DEPTH_TEST); // here for illustrative purposes, depth test is initially DISABLED (key!)
					glClearColor(0.3f, 0.4f, 0.1f, 1.0f);
					glClear(GL_COLOR_BUFFER_BIT);
					//DevMsg("Pt 2\n");
					glfwSwapBuffers(info->window);
					//DevMsg("Pt 3\n");
					glfwPollEvents();
					//DevMsg("Pt 4\n");
					}
					*/

					/*
					bool bShouldRender = false;
					if (lastFrameNumber != gpGlobals->framecount)
					{
					lastFrameNumber = gpGlobals->framecount;

					if (info->framerate == 0)
					bShouldRender = true;
					else
					{
					float dif = 1 / (info->framerate * 1.0);
					if (gpGlobals->curtime - info->lastrendered >= dif * 1.5 || true)	// sense the blocking audio API is being used, we should render every chance we get to be synced to audio.
					bShouldRender = true;
					}

					if (bShouldRender)
					{
					//				info->lastrendered = gpGlobals->curtime;
					//				if (info->readyfornextframe)
					info->raw->run();
					}
					}
					*/
				}
			}
			else if (state == 6) // waiting to die (requested by the libretro core)
			{
				// do nothing
			}
		}
	}

	if (info)
	{
		// save any current state contained in the core to a file.
		if (bDidRun)
		{
			//g_pFullFileSystem->
			//filesystem->WriteFile(char* name, char* path, CUtlBuffer &buf)

			if (info->statesize > 0 && info->settings && info->settings->GetBool("statesaves"))
			{
				info->raw->serialize(info->statedata, info->statesize);

				CUtlBuffer buf;
				buf.Put(info->statedata, info->statesize);
				g_pFullFileSystem->CreateDirHierarchy(VarArgs("%s\\%s", info->savepath.c_str(), info->prettycore.c_str()));
				g_pFullFileSystem->WriteFile(VarArgs("%s\\%s\\%s.sav", info->savepath.c_str(), info->prettycore.c_str(), info->prettygame.c_str()), "", buf);
				buf.Purge();

				info->runninglibretrocores->last_msg = "State Saved";
			}

			if (info->memorymap->saveramsize > 0 && info->settings && info->settings->GetBool("cartsaves"))
			{
				CUtlBuffer buf;
				buf.Put(info->memorymap->saveramdata, info->memorymap->saveramsize);
				g_pFullFileSystem->CreateDirHierarchy(VarArgs("%s\\%s", info->savepath.c_str(), info->prettycore.c_str()));
				g_pFullFileSystem->WriteFile(VarArgs("%s\\%s\\%s.srm", info->savepath.c_str(), info->prettycore.c_str(), info->prettygame.c_str()), "", buf);
				buf.Purge();
			}
		}

		if (info->statesize > 0 && info->statedata)
			free(info->statedata);

		RunningLibretroCores_t* pRunningLibretroCores = info->runninglibretrocores;

		info->statesize = 0;

		// clean up the memory
		if (bDidLoadDll)//info->module)
		{
			Sys_UnloadModule(pModule);
			DevMsg("Unloaded Libretro core.\n");
		}

		if (info->lastframedata)
			free(info->lastframedata);

		if (AA_LIBRETRO_3D && info->framebuffer)
		{
			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
			//glDeleteFramebuffers(1, &info->framebuffer);
		}

		info->libretrokeybinds->deleteThis();
		info->corekeybinds->deleteThis();
		info->gamekeybinds->deleteThis();
		info->inputstate->deleteThis();
		info->coreCoreOptions->deleteThis();
		info->gameCoreOptions->deleteThis();

		delete info->memorymap;
		delete info;

		pRunningLibretroCores->count--;
	}

	return 0;
}

bool C_LibretroInstance::IsSelected()
{
	return (this == g_pAnarchyManager->GetLibretroManager()->GetSelectedLibretroInstance());
}

bool C_LibretroInstance::HasFocus()
{
	return (this == g_pAnarchyManager->GetLibretroManager()->GetFocusedLibretroInstance());
}

bool C_LibretroInstance::Focus()
{
	return g_pAnarchyManager->GetLibretroManager()->FocusLibretroInstance(this);
}

bool C_LibretroInstance::Select()
{
	return g_pAnarchyManager->GetLibretroManager()->SelectLibretroInstance(this);
}

bool C_LibretroInstance::Blur()
{
	if (g_pAnarchyManager->GetLibretroManager()->GetFocusedLibretroInstance())
		g_pAnarchyManager->GetLibretroManager()->FocusLibretroInstance(null);

	return true;
}

bool C_LibretroInstance::Deselect()
{
	return g_pAnarchyManager->GetLibretroManager()->SelectLibretroInstance(null);
}

void C_LibretroInstance::Close()
{
	g_pAnarchyManager->GetLibretroManager()->DestroyLibretroInstance(this);
}

void C_LibretroInstance::GetFullscreenInfo(float& fPositionX, float& fPositionY, float& fSizeX, float& fSizeY, std::string& overlayId)
{
	fPositionX = m_pOverlayKV->GetFloat("current/x", 0);// m_fPositionX;
	fPositionY = m_pOverlayKV->GetFloat("current/y", 0);// m_fPositionY;
	fSizeX = m_pOverlayKV->GetFloat("current/width", 1);// m_fSizeX;
	fSizeY = m_pOverlayKV->GetFloat("current/height", 1);// m_fSizeY;
	//file = m_pOverlayKV->GetString("file", "");// m_file;
	overlayId = m_overlayId;// m_pOverlayKV->GetString("current/overlayId", "");// m_overlayId;
}

bool C_LibretroInstance::CreateWorkerThread(std::string core)
{
	/*
	CSysModule* pModule = Sys_LoadModule(core.c_str());

	if (!pModule)
	{
		Msg("Failed to load %s\n", core.c_str());
		// FIXME FIX ME Probably need to clean up!
		return false;
	}

	HMODULE	hModule = reinterpret_cast<HMODULE>(pModule);
	if (!C_LibretroInstance::BuildInterface(m_raw, &hModule))
	{
		DevMsg("libretro: Failed to build interface!\n");
		// FIXME FIX ME Probably need to clean up!
		return false;
	}
	*/

	std::string corePath = core.substr(0, core.find_last_of("/\\") + 1);

	m_info = new LibretroInstanceInfo_t;
	m_info->runninglibretrocores = g_pAnarchyManager->GetLibretroManager()->GetLibretroRunningCores();
	m_info->state = 0;
	m_info->paused = false;
	m_info->reset = false;
	m_info->close = false;
	m_info->id = "";
	m_info->ready = false;

	float volume = cvar->FindVar("libretro_volume")->GetFloat();
	if (volume > 1.0)
		volume = 1.0;
	m_info->volume = volume;
	m_info->readyfornextframe = true;
	m_info->copyingframe = false;
	m_info->readytocopyframe = false;
	m_info->coreloaded = false;
	m_info->gameloaded = false;
	m_info->raw = m_raw;
	m_info->corepath = corePath;// m_corePath;
	m_info->assetspath = g_pAnarchyManager->GetLibretroManager()->GetLibretroPath(RETRO_ASSETS_PATH);
	m_info->systempath = g_pAnarchyManager->GetLibretroManager()->GetLibretroPath(RETRO_SYSTEM_PATH);
	m_info->savepath = g_pAnarchyManager->GetLibretroManager()->GetLibretroPath(RETRO_SAVE_PATH);
	m_info->module = null;// pModule;
	m_info->threadid = 0;
	m_info->libretroinstance = this;
	m_info->core = core;
	m_info->game = m_originalGame;
	m_info->lastframedata = null;
	m_info->lastframewidth = 0;
	m_info->lastframeheight = 0;
	m_info->lastframepitch = 0;
	m_info->videoformat = RETRO_PIXEL_FORMAT_UNKNOWN;
	m_info->optionshavechanged = false;
	//m_info->numOptions = 0;
//	m_info->audiobuffer = null;
//	m_info->audiobuffersize = 1024;
//	m_info->audiobufferpos = 0;
	m_info->audiostream = null;
	m_info->samplerate = 0;
	m_info->framerate = 30;
	m_info->lastrendered = 0;
//	m_info->audiobuffer = null;
	//m_info->audiobuffersize = 0;
	//m_info->audiobufferpos = 0;
	//m_info->safebuffer = null;
	//m_info->safebuffersize = 0;
	//m_info->safebufferpos = 0;
	m_info->processingaudio = false;
	m_info->window = null;
	m_info->framebuffer = null;
	m_info->portdata = null;
	//m_info->currentPortTypes;
	m_info->numports = 0;

	// hardware acceleration stuff
	m_info->context_type = RETRO_HW_CONTEXT_NONE;
	m_info->depth = false;
	m_info->stencil = false;
	m_info->bottom_left_origin = true;
	m_info->version_major = 0;
	m_info->version_minor = 0;
	m_info->cache_context = true;
	m_info->debug_context = false;

	// system info stuff
	m_info->library_name = "";
	m_info->library_version = "";
	m_info->valid_extensions = "";
	m_info->need_fullpath = true;
	m_info->block_extract = false;

	// state stuff
	m_info->statesize = 0;
	m_info->statedata = null;

	m_info->memorymap = new memory_map_t;
	m_info->memorymap->rtcsize = 0;
	m_info->memorymap->rtcdata = null;
	m_info->memorymap->saveramsize = 0;
	m_info->memorymap->saveramdata = null;
	m_info->memorymap->systemramsize = 0;
	m_info->memorymap->systemramdata = null;
	m_info->memorymap->videoramsize = 0;
	m_info->memorymap->videoramdata = null;

	// LIBRETRO-WIDE KEYBINDS
	KeyValues* kv = new KeyValues("keybinds");
	if (kv->LoadFromFile(g_pFullFileSystem, "libretro\\user\\keybinds.key", "MOD"))
		DevMsg("Loaded libretro keybinds!\n");
	m_info->libretrokeybinds = kv;

	// CORE-SPECIFIC KEYBINDS
	std::string prettyCore = m_info->core;
	size_t found = prettyCore.find_last_of("/\\");
	if ( found != std::string::npos )
		prettyCore = prettyCore.substr(found + 1);

	found = prettyCore.find_last_of(".");
	if (found != std::string::npos)
		prettyCore = prettyCore.substr(0, found);
	prettyCore.erase(std::remove(prettyCore.begin(), prettyCore.end(), '.'), prettyCore.end());

	std::string kvPath = VarArgs("libretro\\user\\%s", prettyCore.c_str());
	//g_pFullFileSystem->CreateDirHierarchy(kvPath.c_str(), "DEFAULT_WRITE_PATH");

	kv = new KeyValues("keybinds");
	kv->LoadFromFile(g_pFullFileSystem, VarArgs("%s\\keybinds.key", kvPath.c_str()), "MOD");
	m_info->corekeybinds = kv;

	// GAME-SPECIFIC KEYBINDS
	std::string prettyGame = m_info->game;
	found = prettyGame.find_last_of("/\\");
	if (found != std::string::npos)
		prettyGame = prettyGame.substr(found + 1);

	found = prettyGame.find_last_of(".");
	if (found != std::string::npos)
		prettyGame = prettyGame.substr(0, found);
	prettyGame.erase(std::remove(prettyGame.begin(), prettyGame.end(), '.'), prettyGame.end());

	m_info->prettygame = prettyGame;
	m_info->prettycore = prettyCore;
	
	kvPath = VarArgs("libretro\\user\\%s\\%s", prettyCore.c_str(), prettyGame.c_str());
	//g_pFullFileSystem->CreateDirHierarchy(kvPath.c_str(), "DEFAULT_WRITE_PATH");

	kv = new KeyValues("keybinds");
	kv->LoadFromFile(g_pFullFileSystem, VarArgs("%s\\keybinds.key", kvPath.c_str()), "MOD");
	m_info->gamekeybinds = kv;

	// CURRENT KEYBINDS
	m_info->inputstate = new KeyValues("keybinds");	// just like the other structs, but holds backbuffer input values instead of source engine key enums.

	// CORE-SPECIFIC OPTIONS
	//kvPath = VarArgs("libretro\\user\\%s", prettyCore.c_str());
	kvPath = "libretro\\user\\" + prettyCore + "\\options.key";

	kv = new KeyValues("options");
	kv->LoadFromFile(g_pFullFileSystem, kvPath.c_str(), "MOD");
	m_info->coreCoreOptions = kv;

	// GAME-SPECIFIC OPTIONS
	kvPath = "libretro\\user\\" + prettyCore + "\\" + prettyGame + "\\options.key";

	kv = new KeyValues("options");
	kv->LoadFromFile(g_pFullFileSystem, kvPath.c_str(), "MOD");
	m_info->gameCoreOptions = kv;

	// we are null settings by default
	m_info->settings = null;

	// but try to find us
	KeyValues* pCoreSettingsKV = g_pAnarchyManager->GetLibretroManager()->GetCoreSettingsKV();
	std::string compareCore = prettyCore + ".dll";
	std::string testerCore;
	for (KeyValues *sub = pCoreSettingsKV->GetFirstSubKey(); sub; sub = sub->GetNextKey())
	{
		testerCore = sub->GetString("file");
		if (testerCore == compareCore)
		{
			m_info->settings = sub;
			break;
		}
	}

	//std::string prettyCore = m_info->prettycore;
	//std::string prettyGame = m_info->prettygame;
	std::string overlayId = g_pAnarchyManager->GetLibretroManager()->DetermineOverlay(prettyCore, prettyGame);
	this->SetOverlay(overlayId);
	g_pAnarchyManager->HudStateNotify();

	m_info->runninglibretrocores->count++;
	g_pAnarchyManager->AddToastMessage(VarArgs("Libretro Opened (%i running)", m_info->runninglibretrocores->count));
	CreateSimpleThread(MyThread, m_info);

	//ThreadId_t pThreadId = 420L;
	//CreateSimpleThread(MyThread, m_info, &pThreadId, 0U);

	/*
	if (!g_CMyAsyncThread.IsAlive())
		g_CMyAsyncThread.Start();

	if (!g_CMyAsyncThread.IsAlive())
		DevMsg("CreateAThreadAndCallTheFunction() failed to start the thread!\n");

	// Thread safety: make some local copies! The real ones could be deleted/changed while we execute.
	char* NewParameter1 = "init";// FUNCTION_THAT_CREATES_A_COPY_OF_THE_MEMORY(Parameter1);
	char* NewParameter2 = "Another test";// FUNCTION_THAT_CREATES_A_COPY_OF_THE_MEMORY(Parameter1);

	g_CMyAsyncThread.CallThreadFunction(NewParameter1, NewParameter2);
	g_CMyAsyncThread.CallWorker(CMyAsyncThread::EXIT);
	*/
	return true;
}

/*
bool CMyAsyncThread::FunctionToBeRunFromInsideTheThread(char* Parameter1, char* Parameter2)
{
	if (!Q_strcmp(Parameter1, "init"))
	{
		//DevMsg("Thread yo: %s %s\n", Parameter1, Parameter2);
		C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->GetSelectedLibretroInstance();
		if (pLibretroInstance)
		{
			libretro_raw* raw = pLibretroInstance->GetRaw();
			raw->set_environment(&this->cbEnvironment);
		}
	}
	return true;
}

void CMyAsyncThread::Update()
{
	DevMsg("once?\n");
	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->GetSelectedLibretroInstance();
	if (pLibretroInstance)
	{
		DevMsg("Yarbles\n");
		libretro_raw* raw = pLibretroInstance->GetRaw();
	//	raw->run();
	}
}
*/
/*
unsigned C_LibretroInstance::Worker(void *params)
{
	DevMsg("whaaaaat\n");
}
*/

/*
int16_t C_LibretroInstance::GetInputState(LibretroInstanceInfo_t* info, unsigned int port, unsigned int device, unsigned int index, unsigned int id)
{
	return g_pAnarchyManager->GetLibretroManager()->GetInputState(info, port, device, index, id);
}
*/

void C_LibretroInstance::cbMessage(enum retro_log_level level, const char * fmt, ...)
{
	va_list args;

	//char* msg = new char[AA_MAX_STRING];
	char msg[AA_MAX_STRING];

	va_start(args, fmt);
	int neededlen = Q_vsnprintf(msg, AA_MAX_STRING, fmt, args);
	va_end(args);

	std::string buf = msg;
	if (buf.at(buf.length() - 1) != '\n')
		buf += "\n";

	DevMsg("libretro: %s", buf.c_str());

	//delete[] msg;
}

///*
HMODULE GetCurrentModule()
{ // NB: XP+ solution!
	HMODULE hModule = NULL;
	GetModuleHandleEx(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPCTSTR)GetCurrentModule,
		&hModule);

	return hModule;
}
//*/

// http://stackoverflow.com/questions/215963/how-do-you-properly-use-widechartomultibyte
// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr)
{
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str)
{
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

const char* GetFormatName(int format)
{
	if (format == 0)
		return "0RGB1555";
	else if (format == 1)
		return "XRGB8888";
	else if (format == 2)
		return "RGB565";
	else
		return "UNKOWN";
}

/*
retro_hw_context_reset_t context_destroy;

void v3d_context_destroy()
{
	DevMsg("Destroy the context!\n");
}

void v3d_context_reset()
{
	DevMsg("Reset the context!\n");
}
*/

static retro_proc_address_t v3d_get_proc_address(const char * sym)
{
	//DevMsg("Getting proc address for %s\n", sym);
	GLFWglproc proc = glfwGetProcAddress(sym);
	//DevMsg("Found proc: %u\n", (bool)(proc));
	return proc;
	//return 0;
}

static uintptr_t v3d_get_current_framebuffer()
{
	//return 0;
	//return 0;	// alcaro says this is related to shaders, and even tho cores don't explicity expect 0 as a response, they handle it well.

	//DevMsg("Getting frame buffer...\n");
	return (uintptr_t)0;
	/*
	uint uId = ThreadGetCurrentId();
	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);

	if (!pLibretroInstance)
		return false;

	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();

	DevMsg("2nd pitt stop: %u\n", info->framebuffer);

//	int width, height;
//	glfwGetFramebufferSize(info->window, &width, &height);
//	DevMsg("Width: %i Height: %i\n", width, height);

	return (uintptr_t)info->framebuffer;//(uintptr_t)GL_FRAMEBUFFER;// (uintptr_t)info->framebuffer;// info->framebuffer;
	//return GL_FRAMEBUFFER;// info->framebuffer;// GL_FRAMEBUFFER;// glfwGetCurrentContext();
	*/
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	DevMsg("OpenGL framebuffer size has changed to: %i x %i\n", width, height);
	glViewport(0, 0, width, height);
}

bool set_rumble_state(unsigned port, enum retro_rumble_effect effect, uint16_t strength)
{
	//DevMsg("libretro: Ignoring %s rumble effect on port %u w/ strength %u\n", (effect == 0) ? "STRONG" : "WEAK", port, strength);
	return true;
}

bool C_LibretroInstance::cbEnvironment(unsigned cmd, void* data)
{
	uint uId = ThreadGetCurrentId();
	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);

	if (!pLibretroInstance)
		return false;

	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();

	//	DevMsg("libretro: Environment called %u.\n", (unsigned int)cmd);

	//1 SET_ROTATION, no known supported core uses that. Cores are expected to deal with failures, anyways.
	//2 GET_OVERSCAN, I have no opinion. Use the default.
	if (cmd == RETRO_ENVIRONMENT_GET_OVERSCAN) //2
	{
		DevMsg("libretro: Asking frontend if overscan should be included or cropped.\n");
		*(bool*)data = false;
		return true;
	}

	if (cmd == RETRO_ENVIRONMENT_GET_CAN_DUPE) //3
	{
		DevMsg("libretro: Asking frontend if CAN_DUPE.\n");
		*(bool*)data = true;
		return true;
	}

	//4 was removed and can safely be ignored.
	//5 was removed and can safely be ignored.
	//6 SET_MESSAGE, ignored because I don't know what to do with that.
	if (cmd == RETRO_ENVIRONMENT_SET_MESSAGE)
	{
		const struct retro_message* msg = (const struct retro_message*)data;
		std::string text = msg->msg;
		DevMsg("libretro: Set Message (%u): %s\n", msg->frames, text.c_str());
		return true;
	}

	//7 SHUTDOWN, ignored because no supported core has any reason to have Off buttons.
	//8 SET_PERFORMANCE_LEVEL, ignored because I don't support a wide range of powers.
	if (cmd == RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY || cmd == RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY || cmd == RETRO_ENVIRONMENT_GET_LIBRETRO_PATH || cmd == RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY || cmd == RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY) // note that libretro path might be wanting a full file location including extension, but ignore that for now and treat it like the others.
	{
		// FIXME: MEMORY LEAK
		// FIXME: there needs to be book keeping so that these temp const char*'s can be cleaned up!!!
		char* buf = new char[AA_MAX_STRING];
		DevMsg("String requested...\n");
		std::string folder;

		if (cmd == RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY || cmd == RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY)
			folder = info->assetspath + "\\" + info->prettycore;// +"\\assets";
		else if (cmd == RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY)
			folder = info->systempath + "\\" + info->prettycore;// +"\\system";
		else if (cmd == RETRO_ENVIRONMENT_GET_LIBRETRO_PATH)
			folder = info->corepath + "\\" + info->core;	// this is a full file location w/ extension
		else if (cmd == RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY)
			folder = info->savepath + "\\" + info->prettycore;// +"\\save";

		if (cmd != RETRO_ENVIRONMENT_GET_LIBRETRO_PATH)
			g_pFullFileSystem->CreateDirHierarchy(folder.c_str());

		Q_strcpy(buf, folder.c_str());// corePath.c_str());

		//V_FixSlashes(buf, '/');

		DevMsg("libretro: Returning string for ");
		if (cmd == RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY)
			DevMsg("system");
		else if (cmd == RETRO_ENVIRONMENT_GET_LIBRETRO_PATH)
			DevMsg("libretro");
		else if (cmd == RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY)
			DevMsg("core assets");
		else if (cmd == RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY)
			DevMsg("content");

		DevMsg(" directory %s\n", buf);

		(*(const char**)data) = buf;

		//delete[] buf;
		return true;
	}

	/*
	if (cmd == RETRO_ENVIRONMENT_SET_VARIABLES)//16
	{
		//HMODULE hModule = GetCurrentModule();	// client.dll
		//HINSTANCE hInstance = GetModuleHandle(0);	// AArcade.exe
		WCHAR  cwBuffer[2048] = { 0 };
		LPWSTR pszBuffer = cwBuffer;
		DWORD  dwMaxChars = _countof(cwBuffer);
		DWORD  dwLength = 0;

		GetModuleFileNameW(hModule, pszBuffer, dwMaxChars);

		std::wstring wString = cwBuffer;
		std::string result = utf8_encode(wString);
		DevMsg("Result: %s\n", result.c_str());

		//long threadId = GetCurrentThreadId();
		//DevMsg("ID is: %l\n", threadId);
		//ThreadSetDebugName()

		//DevMsg("Value is: %s\n", m_info->id.c_str());
//		GetThreadHandle();

		if (ThreadInMainThread())
			DevMsg("MAIN THREAD!\n");
		else
			DevMsg("CHILD THREAD!\n");

		uint uId = ThreadGetCurrentId();
		C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);
		DevMsg("Size is: %i\n", pLibretroInstance->GetInfo()->options.size());
	}
	*/

	if (cmd == RETRO_ENVIRONMENT_SET_PIXEL_FORMAT) //10
	{
		enum retro_pixel_format newfmt = *(enum retro_pixel_format *)data;
		if (newfmt == RETRO_PIXEL_FORMAT_0RGB1555 || newfmt == RETRO_PIXEL_FORMAT_XRGB8888 ||
			newfmt == RETRO_PIXEL_FORMAT_RGB565)
		{
			DevMsg("libretro: Setting video format to %s\n", GetFormatName(newfmt));
			info->videoformat = newfmt;
			return true;
		}
		else
		{
			DevMsg("libretro: Failed at setting video to format %s\n", GetFormatName(newfmt));
			return false;
		}
	}

	if (cmd == RETRO_ENVIRONMENT_SET_HW_RENDER) //14
	{
		if (!AA_LIBRETRO_3D)
		{
			DevMsg("libretro: Denying request for OpenGL context.\n");
			return false;
		}
		else
		{
			DevMsg("libretro: Core requesting HW context: ");

			struct retro_hw_render_callback * render = (struct retro_hw_render_callback*)data;
			render->get_current_framebuffer = v3d_get_current_framebuffer;
			render->get_proc_address = v3d_get_proc_address;

			info->raw->context_reset = (retro_hw_context_reset_t)render->context_reset;
			info->raw->context_destroy = (retro_hw_context_reset_t)render->context_destroy;
			info->context_type = render->context_type;
			info->depth = render->depth;
			info->stencil = render->stencil;
			info->bottom_left_origin = render->bottom_left_origin;
			//info->version_major;
			//info->version_minor;
			info->cache_context = render->cache_context;
			info->debug_context = render->debug_context;

			//GLFW_ORIGIN_UL_BIT;
			//glfwReadImage with the GLFW_ORIGIN_UL_BIT

			unsigned api;
			switch (info->context_type)
			{
				case RETRO_HW_CONTEXT_NONE:
					DevMsg("NONE (UNSUPPORTED)\n");
					api = GLFW_NO_API;
					info->version_major = 0;
					info->version_minor = 0;
					break;

				case RETRO_HW_CONTEXT_OPENGL:
					DevMsg("OpenGL (2.x)\n");
					api = GLFW_OPENGL_API;
					info->version_major = 2;
					info->version_minor = 0;
					break;

				case RETRO_HW_CONTEXT_OPENGLES2:
					DevMsg("OpenGL ES (2.0)\n");
					api = GLFW_OPENGL_ES_API;
					info->version_major = 2;
					info->version_minor = 0;
					break;

				case RETRO_HW_CONTEXT_OPENGL_CORE:
					DevMsg("OpenGL (%u.%u)\n", render->version_major, render->version_minor);
					api = GLFW_OPENGL_API;
					info->version_major = render->version_major;
					info->version_minor = render->version_minor;
					break;

				case RETRO_HW_CONTEXT_OPENGLES3:
					DevMsg("OpenGL ES (3.0)\n");
					api = GLFW_OPENGL_ES_API;
					info->version_major = 3;
					info->version_minor = 0;
					break;

				case RETRO_HW_CONTEXT_OPENGLES_VERSION:
					DevMsg("OpenGL ES (%u.%u)\n", render->version_major, render->version_minor);
					api = GLFW_OPENGL_ES_API;
					info->version_major = render->version_major;
					info->version_minor = render->version_minor;
					break;

				case RETRO_HW_CONTEXT_VULKAN:
					DevMsg("Vulkan (UNSUPPORTED)\n");
					api = GLFW_NO_API;
					info->version_major = 0;
					info->version_minor = 0;
					break;

				default:
					DevMsg("UNKNOWN (UNSUPPORTED)\n");
					api = GLFW_NO_API;
					info->version_major = 0;
					info->version_minor = 0;
					break;
			}

			DevMsg("\tdepth: %i\n", info->depth);
			DevMsg("\tstencil: %i\n", info->stencil);
			DevMsg("\tbottom_left_origin: %i\n", info->bottom_left_origin);
			DevMsg("\tversion_major: %u\n", info->version_major);
			DevMsg("\tversion_minor: %u\n", info->version_minor);
			DevMsg("\tcache_context: %i\n", info->cache_context);
			DevMsg("\tdebug_context: %i\n", info->debug_context);
			
			// init 3d
			if (glfwInit())
			{
				//glfwWindowHint(GLFW_SAMPLES, 4);
				//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
				//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
				glfwWindowHint(GLFW_CLIENT_API, api);
				glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, info->version_major);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, info->version_minor);
				glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
				glfwWindowHint(GLFW_DECORATED, GL_FALSE);
				//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
				//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);// GLFW_OPENGL_ANY_PROFILE);// GLFW_OPENGL_CORE_PROFILE);
				glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);	// invisible window

				//GLFWwindow* window = glfwCreateWindow(640, 480, "", NULL, NULL);
				//HWND myHWnd = FindWindow(null, "AArcade: Source");
				GLFWwindow* window = glfwCreateWindow(1280, 720, "My OPENGL", NULL, NULL);
				//glfwGetWGLContext(window);

				if (window)
				{
				//	render->get_current_framebuffer = v3d_get_current_framebuffer;
					//render->get_proc_address = v3d_get_proc_address;
				//	return true;

					glewExperimental = GL_TRUE; // Needed in core profile
					glfwMakeContextCurrent(window);
					glewInit();

					DevMsg("Window created!\n");
					info->window = window;
					glfwSetFramebufferSizeCallback(info->window, framebuffer_size_callback);	// to detect framebuffer size changes (probably not needed for libretro)
					
					//glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);	// to detect framebuffer size changes (probably not needed for libretro)

					// get version info
					const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
					const GLubyte* version = glGetString(GL_VERSION); // version as a string
					DevMsg("Renderer: %s\n", renderer);
					DevMsg("OpenGL version supported %s\n", version);
					DevMsg("=========================\n");

					//info->framebuffer = new GLuint[1];
					//glGenFramebuffers(1, &info->framebuffer[0]);
					//glBindFramebuffer(GL_FRAMEBUFFER, info->framebuffer[0]);


					/*
					info->framebuffer = new GLuint[1];
					GLuint* tex = new GLuint[1];
					glGenTextures(1, tex);
					glBindTexture(GL_TEXTURE_2D, tex[0]);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB8, GL_UNSIGNED_BYTE, null);	// GL_RGB
					glGenFramebuffers(1, &info->framebuffer[0]);
					glBindFramebuffer(GL_FRAMEBUFFER, info->framebuffer[0]);
					glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex[0], 0);
					if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
						DevMsg("Something went wrong.\n");
					else
					{
						int width, height;
						glfwGetFramebufferSize(window, &width, &height);
						DevMsg("Frame buffer ready: %i x %i\n", width, height);
					}
					*/

					//glfwPollEvents();

					//glDisable(GL_DEPTH_TEST); // here for illustrative purposes, depth test is initially DISABLED (key!)
					//glClearColor(0.3f, 0.4f, 0.1f, 1.0f);
					//glClear(GL_COLOR_BUFFER_BIT);
					//glfwSwapBuffers(info->window);
					//glfwPollEvents();

					//render->context_reset = v3d_context_reset;
					//render->context_destroy = v3d_context_destroy;

					glDisable(GL_DEPTH_TEST);
					//glfwSwapInterval(1);

					// The depth buffer
					/*
					GLuint depthrenderbuffer;
					glGenRenderbuffers(1, &depthrenderbuffer);
					glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
					glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 720);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);
					glEnable(GL_DEPTH_TEST);
					*/

					//glEnable(GL_DOUBLEBUFFER);
					//glEnable(GL_RGB);
					//glPixelStorei(GL_PACK_ALIGNMENT, 3);
					glClearColor(0.3f, 0.4f, 0.1f, 1.0f);
					/*
					if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
						DevMsg("Something went wrong.\n");
					else
					{
						int width, height;
						glfwGetFramebufferSize(window, &width, &height);
						DevMsg("Frame buffer ready: %i x %i\n", width, height);
					}
					*/

					//glViewport(0, 0, 1280, 720); // Render on the whole framebuffer, complete from the lower left corner to the upper right
					// reset the context (TODO: Make sure the window is ready to reset its context!
					info->raw->context_reset();

					DevMsg("done.\n");








					// The texture we're going to render to
					//GLuint renderedTexture;
					//glGenTextures(1, &renderedTexture);
					
					// "Bind" the newly created texture : all future texture functions will modify this texture
					//glBindTexture(GL_TEXTURE_2D, renderedTexture);

					// Give an empty image to OpenGL ( the last "0" )
					//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

					// Poor filtering. Needed !
					//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

					// just make sure to tell glfw you want depth/stencil buffers; if you're using fb0, you can't change that after creating the window
					// Alcaro: glfw deals with context creation, if you're using that then you can ignore anything in the ifdefs

					// The depth buffer
					//GLuint depthrenderbuffer;
					//glGenRenderbuffers(1, &depthrenderbuffer);
					//glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
					//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 1024);
					//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

					// Set "renderedTexture" as our colour attachement #0
					//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

					// Set the list of draw buffers.
					//GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
					//glGenFramebuffers(1, &info->framebuffer);
					//glBindFramebuffer(GL_FRAMEBUFFER, info->framebuffer);
					//glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

					// Always check that our framebuffer is ok
					//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
					//{
					////	DevMsg("Something went wrong.\n");
					//}
					//else
					//	DevMsg("Frame buffer ready.\n");

					// Render to our framebuffer
					//glBindFramebuffer(GL_FRAMEBUFFER, info->framebuffer);
					//glViewport(0, 0, 1024, 1024); // Render on the whole framebuffer, complete from the lower left corner to the upper right

					//glCreateContex()

					//glfwGetWGLContext(window);

					//glfwGetWGLContext

					//DevMsg("done changing frame buffer.\n");
				}
				else
					DevMsg("Failed to create openGL window.\n");

				//if (info->window)
				//{
					//DevMsg("Painting initial frame...\n");
					/*
					// background color
					glDisable(GL_DEPTH_TEST); // here for illustrative purposes, depth test is initially DISABLED (key!)
					glClearColor(0.3f, 0.4f, 0.1f, 1.0f);
					glClear(GL_COLOR_BUFFER_BIT);

					glfwSwapBuffers(info->window);
					glfwPollEvents();
					*/

					//glfwPollEvents();
					//render->context_reset = v3d_context_reset;
					//render->context_destroy = v3d_context_destroy;
					//render->get_current_framebuffer = //(uintptr_t)GL_FRAMEBUFFER;// (uintptr_t)info->framebuffer;// info->framebuffer;
					//render->get_current_framebuffer = v3d_get_current_framebuffer;
					//render->get_proc_address = v3d_get_proc_address;
				//}

				DevMsg("Fin\n");
				return true;
			}

			return false;
		}
		/*
		if (!this->create3d) return false;
		struct retro_hw_render_callback * render = (struct retro_hw_render_callback*)data;
		this->v3d = this->create3d(render);
		if (!this->v3d) return false;
		render->get_current_framebuffer = v3d_get_current_framebuffer;
		render->get_proc_address = v3d_get_proc_address;
		*/
		//return true;

		//return false;
	}

	if (cmd == RETRO_ENVIRONMENT_GET_VARIABLE) //15
	{
		struct retro_variable * variable = (struct retro_variable*)data;

		variable->value = NULL;

		DevMsg("Requesting variable: %s = ", variable->key);

		bool bFoundVal = false;
		std::string val;
		KeyValues* kv;
		unsigned int index;
		unsigned int numOptions = info->options.size();
		for (index = 0; index < numOptions; index++)
		{
			if (std::string(variable->key) == info->options[index]->name_internal)
				break;
		}

		kv = info->gameCoreOptions;
		if (!Q_strcmp(kv->GetString(variable->key, "default"), "default"))
			kv = info->coreCoreOptions;

		if (Q_strcmp(kv->GetString(variable->key, "default"), "default"))
		{
			val = kv->GetString(variable->key);
			bFoundVal = true;
		}
		else if (index < numOptions)
		{
			val = info->options[index]->values[0].c_str();
			bFoundVal = true;
		}
		else
		{
			DevMsg("WARNING: Libretro core requested a variable that it did not tell us about before hand!: %s\n", variable->key);

			// try to reply with a default response (even tho we dont know what a default response is cuz this core never told us shit.)
			if (info->core.find("mame") != std::string::npos)
			{
				val = "disabled";
				bFoundVal = true;
			}
		}

		if (bFoundVal)
		{
			// FIXME: MEMORY LEAK
			// FIXME: there needs to be book keeping so that these temp const char*'s can be cleaned up!!!
			char* buf = new char[AA_MAX_STRING];
			Q_strcpy(buf, val.c_str());
			variable->value = buf;
			DevMsg("%s\n", buf);

			//V_FixSlashes(buf, '/');
			//(*(const char**)data) = buf;
			//delete[] buf;
		}
		else
			variable->value = null;

		info->optionshavechanged = false;
		return true;
	}

	if (cmd == RETRO_ENVIRONMENT_SET_VARIABLES)//16
	{
		DevMsg("libretro: RETRO_ENVIRONMENT_SET_VARIABLES\n");
		const struct retro_variable * variables = (const struct retro_variable*)data;

		// variables are stored as const chars and must be manually dealloc OBSOLTETE: i think they are strings now.
		while (!info->options.empty())
		{
		//	libretro_core_option* pOption = info->options[info->options.size() - 1];
		//	free(pOption->name_display);
			info->options.pop_back();
		}

		const struct retro_variable * variables_count = variables;

		while (variables_count->key) variables_count++;
		unsigned int numvars = variables_count - variables;

		bool bOptionListHasChanged = true;
		bool bOptionsHaveChanged = true;
		DevMsg("Num vars is: %i\n", numvars);
		for (unsigned int i = 0; i<numvars; i++)
		{
			// Initialize to 0 index for this variable's value

//			info->optionscurrentvalues.push_back(0);

			DevMsg("libretro: Setting up environment variable %s with definition %s of %u\n", variables[i].key, variables[i].value, i);

			libretro_core_option* pOption = new libretro_core_option();
			pOption->name_internal = variables[i].key;

			const char * values = Q_strstr(variables[i].value, "; ");

			//if the value does not contain "; ", the core is broken, and broken cores can break shit in whatever way they want, anyways.
			//let's segfault.
			// In other words, values would be null and a crash would occur when we tried to use it.
			//pOption->name_display = VarArgs("%s", variables[i].value);

			/*
			unsigned int namelen = values - variables[i].value;
			values += 2;

			char* name = (char*)malloc(namelen + 1);
			memcpy(name, variables[i].value, namelen);
			name[namelen] = '\0';
			pOption->name_display = name;
			*/

			//buf = VarArgs("%s", variables[i].value);
			pOption->name_display = variables[i].value;
			size_t found = pOption->name_display.find("; ");
			if (found != std::string::npos)
				pOption->name_display = pOption->name_display.substr(0, found);

			unsigned int numvalues = 1;
			const char * valuescount = values;
			while (*valuescount)
			{
				if (*valuescount == '|') numvalues++;
				valuescount++;
			}

			std::string buf;
			const char * nextvalue = values;
			for (unsigned int j = 0; j<numvalues; j++)
			{
				nextvalue = values;
				while (*nextvalue && *nextvalue != '|') nextvalue++;
				unsigned int valuelen = nextvalue - values;

				buf = values;
				if (j == 0)
					buf = buf.substr(2, valuelen - 2);
				else
					buf = buf.substr(0, valuelen);

				pOption->values.push_back(buf);
				values = nextvalue + 1;
			}

			info->options.push_back(pOption);
		}

		return true;
	}

	if (cmd == RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE) //17
	{
		//DevMsg("libretro: Fetching if options have changed (%s).\n", (s_bOptionsHaveChanged)?"true":"false");
		*(bool*)data = info->optionshavechanged;
		return true;
	}

	if (cmd == RETRO_ENVIRONMENT_GET_PERF_INTERFACE) //18
	{
		DevMsg("libretro: UNHANDLED RETRO_ENVIRONMENT_GET_PERF_INTERFACE\n");
		struct retro_perf_callback *cb = (struct retro_perf_callback*)data;

		cb->get_time_usec = null;
		cb->get_cpu_features = null;
		cb->get_perf_counter = null;

		cb->perf_register = null;
		cb->perf_start = null;
		cb->perf_stop = null;
		cb->perf_log = null;
		return false;
	}

	if (cmd == RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE) //23
	{
		struct retro_rumble_interface * iface = (struct retro_rumble_interface*)data;
		DevMsg("libretro: Rumble interface requested.\n");
		iface->set_rumble_state = set_rumble_state;
		return true;
	}

	if (cmd == RETRO_ENVIRONMENT_GET_LOG_INTERFACE) //27
	{
		struct retro_log_callback * logcb = (struct retro_log_callback*)data;
		logcb->log = &C_LibretroInstance::cbMessage;
		return true;
	}


	if (cmd == RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO) //32
	{
		DevMsg("libretro: acknowledging RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO\n");
		return true;
	}

	if (cmd == RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS)
	{
		DevMsg("libretro: RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS\n");

		unsigned controller, typeIndex;
		const struct retro_input_descriptor *controllerData = (const struct retro_input_descriptor*)data;

		//info->currentPortTypes.clear();

		for (controller = 0; controllerData[controller].description; controller++)
		{
			DevMsg("Unhandled Controller info:\n");
			DevMsg("\tPort: %u\n", controllerData[controller].port);
			DevMsg("\tDevice: %u\n", controllerData[controller].device);
			DevMsg("\tIndex: %u\n", controllerData[controller].index);
			DevMsg("\tID: %u\n", controllerData[controller].id);
			DevMsg("\tDescription: %u\n", controllerData[controller].description);

			//info->currentPortTypes.push_back(1);	// set every joystick to use the 1st entry (should always be RetroPad w/ id 1).
			// NOTE: 0 must ALWAYS gets inserted to the front when the current ports are gotten, which would mean Unplugged.
			// NOTE: These are vector indecies, NOT retro device IDs

			//for (typeIndex = 0; typeIndex < portData[port].num_types; typeIndex++)
			//	DevMsg("\t%s (ID: %u)\n", portData[port].types[typeIndex].desc, portData[port].types[typeIndex].id);
		}

		//free((void*)info->portdata);
		//info->portdata = (struct retro_controller_info*)
		//	calloc(port, sizeof(*info->portdata));
		//memcpy((void*)info->portdata, portData,
		//	port * sizeof(*info->portdata));

		//info->numports = port;



		/*
		if (system)
		{
		free(system->ports.data);
		system->ports.data = (struct retro_controller_info*)
		calloc(i, sizeof(*system->ports.data));
		if (!system->ports.data)
		return false;

		memcpy(system->ports.data, info,
		i * sizeof(*system->ports.data));
		system->ports.size = i;
		}
		break;
		*/

		return false;
	}

	if (cmd == RETRO_ENVIRONMENT_SET_CONTROLLER_INFO) //35
	{
		DevMsg("libretro: RETRO_ENVIRONMENT_SET_CONTROLLER_INFO\n");

		unsigned port, typeIndex;
		const struct retro_controller_info *portData = (const struct retro_controller_info*)data;

		info->currentPortTypes.clear();
		for (port = 0; portData[port].types; port++)
		{
			DevMsg("Controller port: %u\n", port + 1);
			info->currentPortTypes.push_back(1);	// set every joystick to use the 1st entry (should always be RetroPad w/ id 1).
			// NOTE: 0 must ALWAYS gets inserted to the front when the current ports are gotten, which would mean Unplugged.
			// NOTE: These are vector indecies, NOT retro device IDs

			for (typeIndex = 0; typeIndex < portData[port].num_types; typeIndex++)
				DevMsg("\t%s (ID: %u)\n", portData[port].types[typeIndex].desc, portData[port].types[typeIndex].id);
		}

		free((void*)info->portdata);
		info->portdata = (struct retro_controller_info*)
			calloc(port, sizeof(*info->portdata));
		memcpy((void*)info->portdata, portData,
			port * sizeof(*info->portdata));

		//info->portdata = portData;
		info->numports = port;
		/*
		if (system)
		{
			free(system->ports.data);
			system->ports.data = (struct retro_controller_info*)
				calloc(i, sizeof(*system->ports.data));
			if (!system->ports.data)
				return false;

			memcpy(system->ports.data, info,
				i * sizeof(*system->ports.data));
			system->ports.size = i;
		}
		break;
		*/

		return true;
	}

	if (cmd == RETRO_ENVIRONMENT_SHUTDOWN)
	{
		//info->close = true;
		info->runninglibretrocores->last_error = "Core Shutdown";
		info->state = 6;
		// should probably show some kind of related items screen when a video ends.
		return true;
	}

	const char * const names[] = {
		"(invalid)",
		"SET_ROTATION",
		"GET_OVERSCAN",
		"GET_CAN_DUPE",
		"(removed)",
		"(removed)",
		"SET_MESSAGE",
		"SHUTDOWN",
		"SET_PERFORMANCE_LEVEL",
		"GET_SYSTEM_DIRECTORY",
		"SET_PIXEL_FORMAT",
		"SET_INPUT_DESCRIPTORS",
		"SET_KEYBOARD_CALLBACK",
		"SET_DISK_CONTROL_INTERFACE",
		"SET_HW_RENDER",
		"GET_VARIABLE",
		"SET_VARIABLES",
		"GET_VARIABLE_UPDATE",
		"SET_SUPPORT_NO_GAME",
		"GET_LIBRETRO_PATH",
		"(removed)",
		"SET_FRAME_TIME_CALLBACK",
		"SET_AUDIO_CALLBACK",
		"GET_RUMBLE_INTERFACE",
		"GET_INPUT_DEVICE_CAPABILITIES",
		"GET_SENSOR_INTERFACE",
		"GET_CAMERA_INTERFACE",
		"GET_LOG_INTERFACE",
		"GET_PERF_INTERFACE",
		"GET_LOCATION_INTERFACE",
		"GET_CONTENT_DIRECTORY",
		"GET_SAVE_DIRECTORY",
		"SET_SYSTEM_AV_INFO",
		"SET_PROC_ADDRESS_CALLBACK",
		"SET_SUBSYSTEM_INFO",
		"SET_CONTROLLER_INFO",
		"SET_MEMORY_MAPS",
		"SET_GEOMETRY",
		"GET_USERNAME",
		"GET_LANGUAGE",
	};

	if ((cmd&~RETRO_ENVIRONMENT_EXPERIMENTAL) < sizeof(names) / sizeof(*names))
		C_LibretroInstance::cbMessage(RETRO_LOG_WARN, "Ignored unsupported environment command #%u %s.", cmd, names[cmd&~RETRO_ENVIRONMENT_EXPERIMENTAL]);
	else
		C_LibretroInstance::cbMessage(RETRO_LOG_WARN, "Ignored unsupported environment command #%u.", cmd);

	return false;
}

void C_LibretroInstance::cbVideoRefresh(const void * data, unsigned width, unsigned height, size_t pitch)
{
//	if (!data || AA_LIBRETRO_3D)
	//	return;
		//glXSwapBuffers();
	//DevMsg("Video Refresh: %u %u %i\n", width, height, pitch);
	uint uId = ThreadGetCurrentId();
	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);

	if (!pLibretroInstance)
		return;

	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();
	if (info->close)
		return;

	info->lastframewidth = width;
	info->lastframeheight = height;
	info->lastframepitch = pitch;

	if (!data || AA_LIBRETRO_3D)
		return;

	//if (AA_LIBRETRO_3D && info->context_type != RETRO_HW_CONTEXT_NONE && data == RETRO_HW_FRAME_BUFFER_VALID)
		//glfwSwapBuffers(info->window);
	
//	if (data == RETRO_HW_FRAME_BUFFER_VALID)
	//	DevMsg("hardware rendered frame given...\n");

	//glfwSwapBuffers(info->window);
	//return;

	info->lastrendered = gpGlobals->curtime;

	if (!info->readyfornextframe || info->copyingframe )
		return;

	info->readyfornextframe = false;
	info->readytocopyframe = false;

	if (info->samplerate == 0)
	{
		DevMsg("Get AV info\n");
		struct retro_system_av_info avinfo;
		info->raw->get_system_av_info(&avinfo);

		if (avinfo.timing.sample_rate > 0)
		{
			info->samplerate = int(avinfo.timing.sample_rate);
			info->framerate = int(avinfo.timing.fps);
			C_LibretroInstance::CreateAudioStream();
		}
	}

	
		//glDisable(GL_DEPTH_TEST); // here for illustrative purposes, depth test is initially DISABLED (key!)
		//glClearColor(0.3f, 0.4f, 0.1f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT);
		//glfwMakeContextCurrent(info->window);
		//glfwSwapBuffers(info->window);
		//glfwPollEvents();	// this is what makes the window actually be responsive


	//WORD red_mask = 0xF800;
	//WORD green_mask = 0x7E0;
	//WORD blue_mask = 0x1F;
	
	//DevMsg("Doin it\n");

	//DevMsg("video refresh\n");

	void* dest = malloc(pitch*height);
	if (AA_LIBRETRO_3D && info->context_type != RETRO_HW_CONTEXT_NONE && data == RETRO_HW_FRAME_BUFFER_VALID)
	{
		//DevMsg("Format is: %i %i x %i\n", info->videoformat, pitch, height);
		glReadPixels(0, 0, pitch / 3, height, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, dest);// GL_RGBA8	//GL_RGB
		//glfwSwapInterval(1);
		//glfwSwapBuffers(info->window);
		//glfwPollEvents();
	}
	else
	{
		Q_memcpy(dest, data, pitch*height);
	}

	if (info->lastframedata)
		free(info->lastframedata);

	info->lastframedata = dest;
	info->lastframewidth = width;
	info->lastframeheight = height;
	info->lastframepitch = pitch;

	info->readytocopyframe = true;
	//pLibretroInstance->m_mutex.unlock();
	//DevMsg("Child Unlock\n");
}

void C_LibretroInstance::cbAudioSample(int16_t left, int16_t right)
{
	uint uId = ThreadGetCurrentId();
	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);
	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();
	DevMsg("cbAudioSample\n");
	/*
	if (info->audiobufferpos < 512)
	{
	double leftRegular = (int)left * 1.0;// pLibretroVolumeScaleConVar->GetFloat();
	if (left > 32767)
	leftRegular = 32767.0;
	if (left < -32768.0)
	leftRegular = -32768.0;

	double rightRegular = (int)right * 1.0;// s_pLibretroVolumeScaleConVar->GetFloat();
	if (right > 32767)
	rightRegular = 32767.0;
	if (right < -32768.0)
	rightRegular = -32768.0;

	info->audiobuffer[info->audiobufferpos++] = (int16_t)leftRegular;
	info->audiobuffer[info->audiobufferpos++] = (int16_t)rightRegular;
	}
	*/
}

size_t C_LibretroInstance::cbAudioSampleBatch(const int16_t * data, size_t frames)
{
	uint uId = ThreadGetCurrentId();
	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);

	if (!pLibretroInstance)
		return 0;

	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();
	/*
	if ( info->processingaudio)
		return 0;
	else
		info->processingaudio = true;
		*/

	if (info->samplerate == 0)
	{
		struct retro_system_av_info avinfo;
		info->raw->get_system_av_info(&avinfo);

		if (avinfo.timing.sample_rate > 0)
		{
			info->samplerate = int(avinfo.timing.sample_rate);
			info->framerate = int(avinfo.timing.fps);
			C_LibretroInstance::CreateAudioStream();
		}
	}

	if (info->samplerate <= 0 || frames <= 0 || !info->audiostream ) // || s_threadAudioParams.frames
		return 0;
	
	//DevMsg("Writing %u frames: %i\n", frames, data);

	//if ( info->volume > 0.0f )

	/*
	//float buffer[SAMPLES_PER_BUFFER];
	//volume in dB 0db = unity gain, no attenuation, full amplitude signal
	//           -20db = 10x attenuation, significantly more quiet
	float volumeLevelDb = -6.f; //cut amplitude in half; same as 0.5 above
	const float VOLUME_REFERENCE = 1.f;
	const float volumeMultiplier = (VOLUME_REFERENCE * pow(10, (volumeLevelDb / 20.f);
	for (int i = 0; i < SAMPLES_PER_BUFFER; ++i)
	{
		data[i] *= volumeMultiplier;
	}
	*/

	//PaError err = Pa_WriteStream(info->audiostream, data, frames);// paFramesPerBufferUnspecified);

	// AMPLIFY THE AUDIO HERE!
	float volume = info->volume;
	int16_t sample[2];
	double leftRegular, rightRegular;
	int16_t left, right;

	for (int i = 0; i < frames * 2; i = i + 2)
	{
		left = data[i];
		right = data[i + 1];

		leftRegular = (double)left * volume;
		if (left > 32767)
			leftRegular = 32767.0;
		if (left < -32768.0)
			leftRegular = -32768.0;

		rightRegular = (double)right * volume;
		if (right > 32767)
			rightRegular = 32767.0;
		if (right < -32768.0)
			rightRegular = -32768.0;

		sample[0] = (int16_t)leftRegular;
		sample[1] = (int16_t)rightRegular;

		PaError err = Pa_WriteStream(info->audiostream, sample, 1);
	}

	return frames;
	/*
	int16_t* buffer = info->audiobuffer;
	unsigned int size = info->audiobuffersize;
	unsigned int pos = info->audiobufferpos;

	unsigned int processedCount = size - pos;
	if (processedCount > frames)
		processedCount = frames;

	if (processedCount > 0)
	{
		Q_memcpy(buffer + (pos * 2), data, sizeof(int16_t) * processedCount * 2);
		info->audiobufferpos += processedCount;
	}

	return processedCount;
	*/
}

void C_LibretroInstance::cbInputPoll(void)
{
	//DevMsg("cbInputPoll\n");
	// TODO: Implement this.  it might be related to timing of analog & mouse input devices because those expect offsets relative to the last time they were polled.
	// This is likely supposed to trigger the polling of input states on the FE for the core to later retrieve with InputState.
	//DevMsg("libretro: Input Poll called.\n");
}

int16_t C_LibretroInstance::cbInputState(unsigned port, unsigned device, unsigned index, unsigned id)
{
	uint uId = ThreadGetCurrentId();
	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);

	if (!pLibretroInstance)
		return (int16_t)0;

	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();
	//DevMsg("Check for: port%u/device%u/index%u/key%u\n", port, device, index, id);

	// only accept input from port0 device1 index0
	if (!info->inputstate || port != 0 || device != 1 || index != 0)
		return (int16_t)0;

	//int intVal = info->inputstate->GetInt(VarArgs("port%u/device%u/index%u/key%u", port, device, index, id), 0);
	//return (int16_t)intVal;

	std::string keyPath = "port" + std::to_string(port) + "/device" + std::to_string(device) + "/index" + std::to_string(index) + "/key" + std::to_string(id);

	return (int16_t)info->inputstate->GetInt(keyPath.c_str());

	//KeyValues* kv = info->inputstate->FindKey();
	//if (!kv)
//		return (int16_t)0;

	//int intVal = kv->GetInt();
	//if ( id > 11 && intVal != 0)
	//	DevMsg("Looking for ID: %u = %i\n", id, intVal);

	//return (int16_t)intVal;
		//pLibretroInstance->GetInputState(info, port, device, index, id);

	/*
	#define RETRO_DEVICE_ID_JOYPAD_B        0
	#define RETRO_DEVICE_ID_JOYPAD_Y        1
	#define RETRO_DEVICE_ID_JOYPAD_SELECT   2
	#define RETRO_DEVICE_ID_JOYPAD_START    3
	#define RETRO_DEVICE_ID_JOYPAD_UP       4
	#define RETRO_DEVICE_ID_JOYPAD_DOWN     5
	#define RETRO_DEVICE_ID_JOYPAD_LEFT     6
	#define RETRO_DEVICE_ID_JOYPAD_RIGHT    7
	#define RETRO_DEVICE_ID_JOYPAD_A        8
	#define RETRO_DEVICE_ID_JOYPAD_X        9
	#define RETRO_DEVICE_ID_JOYPAD_L       10
	#define RETRO_DEVICE_ID_JOYPAD_R       11
	#define RETRO_DEVICE_ID_JOYPAD_L2      12
	#define RETRO_DEVICE_ID_JOYPAD_R2      13
	#define RETRO_DEVICE_ID_JOYPAD_L3      14
	#define RETRO_DEVICE_ID_JOYPAD_R3      15
	*/
	//DevMsg("libretro: Input State called.\n");
	//DevMsg("libretro: Input State called with port %u, device %u, index %u, and ID %u.\n", port, device, index, id);

	// only accept input from player 1
	//if (port != 0 || device != 1 || index != 0)
	//	return (int16_t)0;

	//DevMsg("%i %i %i\n", port, device, index);
	// FIXME: index could allow for additional devices PER port, which would require significant changes to support.  However AArcade will rarely be used by more than 1 local player, so support for index probably isn't needed.  supoorting N input ports is enough.

	//std::string retrokey = g_pAnarchyManager->GetLibretroManager()->RetroKeyboardKeyToString((retro_key)index);
	//std::string retrodevice = g_pAnarchyManager->GetLibretroManager()->RetroDeviceToString(device);

	//int max = 32767;
	//max *= info->inputstate[retrokey];
	//int16_t max = 0x7fff;	//32767


//	if (info->inputstate.find(retrokey) != info->inputstate.end() )
//		return (int16_t)info->inputstate[retrokey];
//	else
//		return (int16_t)0;
}

void C_LibretroInstance::ResizeFrameFromRGB565(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth)
{
//	uint uId = ThreadGetCurrentId();
//	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);
//	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();

	LibretroInstanceInfo_t* info = m_info;
	if (!info->lastframedata)
		return;
	
	info->readyfornextframe = false;

	//	DevMsg("Resizing a %ux%u %iBBP (%i pitch) image to %ux%u %iBBP (%i pitch)\n", sourceWidth, sourceHeight, sourceDepth, sourcePitch, destWidth, destHeight, destDepth, destPitch);

	WORD red_mask = 0xF800;
	WORD green_mask = 0x7E0;
	WORD blue_mask = 0x1F;

	uint16* pRealSrc = (uint16*)pSrc;

	unsigned char* pDstRow = (unsigned char*)pDst;
	for (int dstY = 0; dstY<destHeight; dstY++)
	{

		unsigned int srcY = dstY * sourceHeight / destHeight;
		uint16* pSrcRow = pRealSrc + (srcY * ((int)sourcePitch / 2));

		unsigned char* pDstCur = pDstRow;

		for (int dstX = 0; dstX<destWidth; dstX++)
		{
			int srcX = dstX * sourceWidth / destWidth;

			int red = (pSrcRow[srcX] & red_mask) >> 11;
			int green = (pSrcRow[srcX] & green_mask) >> 5;
			int blue = (pSrcRow[srcX] & blue_mask);

			pDstCur[0] = blue * (255 / 31);
			pDstCur[1] = green * (255 / 63);
			pDstCur[2] = red * (255 / 31);

			pDstCur[3] = 255;

			pDstCur += destDepth;
		}

		pDstRow += destPitch;
	}

	info->readyfornextframe = true;
}

void C_LibretroInstance::ResizeFrameFromRGB1555(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth)
{
//	uint uId = ThreadGetCurrentId();
//	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);
//	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();

	LibretroInstanceInfo_t* info = m_info;
	if (!info->lastframedata)
		return;

	info->readyfornextframe = false;

	//	DevMsg("Resizing a %ux%u %iBBP (%i pitch) image to %ux%u %iBBP (%i pitch)\n", sourceWidth, sourceHeight, sourceDepth, sourcePitch, destWidth, destHeight, destDepth, destPitch);

	WORD red_mask = 0x7C00;
	WORD green_mask = 0x03E0;
	WORD blue_mask = 0x001F;

	uint16* pRealSrc = (uint16*)pSrc;

	unsigned char* pDstRow = (unsigned char*)pDst;
	for (int dstY = 0; dstY<destHeight; dstY++)
	{

		unsigned int srcY = dstY * sourceHeight / destHeight;
		uint16* pSrcRow = pRealSrc + (srcY * ((int)sourcePitch / 2));

		unsigned char* pDstCur = pDstRow;

		for (int dstX = 0; dstX<destWidth; dstX++)
		{
			int srcX = dstX * sourceWidth / destWidth;

			int red = (pSrcRow[srcX] & red_mask) >> 10;
			int green = (pSrcRow[srcX] & green_mask) >> 5;
			int blue = (pSrcRow[srcX] & blue_mask);

			pDstCur[0] = blue * (255 / 31);
			pDstCur[1] = green * (255 / 31);
			pDstCur[2] = red * (255 / 31);

			pDstCur[3] = 255;

			pDstCur += destDepth;
		}

		pDstRow += destPitch;
	}

	info->readyfornextframe = true;
}

void C_LibretroInstance::ResizeFrameFromXRGB8888(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth)
{
	//DevMsg("Thread ID: %u\n", ThreadGetCurrentId);
//	uint uId = ThreadGetCurrentId();
//	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);
//	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();
	//LibretroInstanceInfo_t* info = m_info;

	//if (!m_info->lastframedata)
//	DevMsg("Main Lock\n");
	if (!m_info->lastframedata)
		return;

//	m_mutex.lock();
//	if (!m_info->lastframedata || !m_info->readyfornextframe)
	//	return;


	//m_info->readyfornextframe = false;

	//DevMsg("Resizing a %ux%u %iBBP (%i pitch) image to %ux%u %iBBP (%i pitch)\n", sourceWidth, sourceHeight, sourceDepth, sourcePitch, destWidth, destHeight, destDepth, destPitch);
//	DevMsg("Test: %s\n", pDest);

	unsigned int sourceWidthCopy = sourceWidth;
	unsigned int sourceHeightCopy = sourceHeight;
	size_t sourcePitchCopy = sourcePitch;
	unsigned int sourceDepthCopy = sourceDepth;

	//void* pSrcCopy = malloc(sourcePitchCopy * sourceHeightCopy);
	//Q_memcpy(pSrcCopy, pSrc, sourcePitchCopy * sourceHeightCopy);


	const unsigned char* pRealSrc = (const unsigned char*)pSrc;
	unsigned char* pDstRow = (unsigned char*)pDst;
	for (int dstY = 0; dstY<destHeight; dstY++)
	{
		unsigned int srcY = dstY * sourceHeight / destHeight;
		const unsigned char* pSrcRow = pRealSrc + srcY*(sourcePitch);

		unsigned char* pDstCur = pDstRow;

		for (int dstX = 0; dstX<destWidth; dstX++)
		{
			int srcX = dstX * sourceWidth / destWidth;
			pDstCur[0] = pSrcRow[srcX*sourceDepth + 0];
			pDstCur[1] = pSrcRow[srcX*sourceDepth + 1];
			pDstCur[2] = pSrcRow[srcX*sourceDepth + 2];

			pDstCur[3] = 255;

			pDstCur += destDepth;
		}

		pDstRow += destPitch;
	}

	/*
	const unsigned char* pRealSrc = (const unsigned char*)pSrc;
	unsigned char* pDstRow = (unsigned char*)pDst;
	for (int dstY = 0; dstY<destHeight; dstY++)
	{
		unsigned int srcY = dstY * sourceHeight / destHeight;
		const unsigned char* pSrcRow = pRealSrc + srcY*(sourcePitch);

		unsigned char* pDstCur = pDstRow;

		for (int dstX = 0; dstX<destWidth; dstX++)
		{
			int srcX = dstX * sourceWidth / destWidth;
			pDstCur[0] = pSrcRow[srcX*sourceDepth + 0];
			pDstCur[1] = pSrcRow[srcX*sourceDepth + 1];
			pDstCur[2] = pSrcRow[srcX*sourceDepth + 2];

			pDstCur[3] = 255;

			pDstCur += destDepth;
		}

		pDstRow += destPitch;
	}
	*/

//	free(pSrcCopy);

	m_info->readyfornextframe = true;

//	m_mutex.unlock();
//	DevMsg("Main Unlock\n");
}

void C_LibretroInstance::ResizeFrameFromRGB888(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth)
{
	//DevMsg("Thread ID: %u\n", ThreadGetCurrentId);
	//	uint uId = ThreadGetCurrentId();
	//	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);
	//	LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();
	//LibretroInstanceInfo_t* info = m_info;

	//if (!m_info->lastframedata)
	//	DevMsg("Main Lock\n");
	if (!m_info->lastframedata)
		return;

	//	m_mutex.lock();
	//	if (!m_info->lastframedata || !m_info->readyfornextframe)
	//	return;


	//m_info->readyfornextframe = false;

	//DevMsg("Resizing a %ux%u %iBBP (%i pitch) image to %ux%u %iBBP (%i pitch)\n", sourceWidth, sourceHeight, sourceDepth, sourcePitch, destWidth, destHeight, destDepth, destPitch);
	//	DevMsg("Test: %s\n", pDest);

	unsigned int sourceWidthCopy = sourceWidth;
	unsigned int sourceHeightCopy = sourceHeight;
	size_t sourcePitchCopy = sourcePitch;
	unsigned int sourceDepthCopy = sourceDepth;

	//void* pSrcCopy = malloc(sourcePitchCopy * sourceHeightCopy);
	//Q_memcpy(pSrcCopy, pSrc, sourcePitchCopy * sourceHeightCopy);


	const unsigned char* pRealSrc = (const unsigned char*)pSrc;
	unsigned char* pDstRow = (unsigned char*)pDst;
	for (int dstY = 0; dstY<destHeight; dstY++)
	{
		unsigned int srcY = dstY * sourceHeight / destHeight;
		const unsigned char* pSrcRow = pRealSrc + srcY*(sourcePitch);

		unsigned char* pDstCur = pDstRow;

		for (int dstX = 0; dstX<destWidth; dstX++)
		{
			int srcX = dstX * sourceWidth / destWidth;
			pDstCur[0] = pSrcRow[srcX*sourceDepth + 2];
			pDstCur[1] = pSrcRow[srcX*sourceDepth + 1];
			pDstCur[2] = pSrcRow[srcX*sourceDepth + 0];

			pDstCur[3] = 255;

			pDstCur += destDepth;
		}

		pDstRow += destPitch;
	}

	/*
	const unsigned char* pRealSrc = (const unsigned char*)pSrc;
	unsigned char* pDstRow = (unsigned char*)pDst;
	for (int dstY = 0; dstY<destHeight; dstY++)
	{
	unsigned int srcY = dstY * sourceHeight / destHeight;
	const unsigned char* pSrcRow = pRealSrc + srcY*(sourcePitch);

	unsigned char* pDstCur = pDstRow;

	for (int dstX = 0; dstX<destWidth; dstX++)
	{
	int srcX = dstX * sourceWidth / destWidth;
	pDstCur[0] = pSrcRow[srcX*sourceDepth + 0];
	pDstCur[1] = pSrcRow[srcX*sourceDepth + 1];
	pDstCur[2] = pSrcRow[srcX*sourceDepth + 2];

	pDstCur[3] = 255;

	pDstCur += destDepth;
	}

	pDstRow += destPitch;
	}
	*/

	//	free(pSrcCopy);

	m_info->readyfornextframe = true;

	//	m_mutex.unlock();
	//	DevMsg("Main Unlock\n");
}

void C_LibretroInstance::CopyLastFrame(unsigned char* dest, unsigned int width, unsigned int height, size_t pitch, unsigned int depth)
{
	if ( m_info->copyingframe || !m_info->readytocopyframe || g_pAnarchyManager->GetSuspendEmbedded())
		return;

	m_info->copyingframe = true;
	m_info->readytocopyframe = false;



	//DevMsg("Render: Do it!\n");
	//RETRO_PIXEL_FORMAT_0RGB1555

//	uint uId = ThreadGetCurrentId();
//	C_LibretroInstance* pLibretroInstance = g_pAnarchyManager->GetLibretroManager()->FindLibretroInstance(uId);
	//LibretroInstanceInfo_t* info = pLibretroInstance->GetInfo();
	//LibretroInstanceInfo_t* info = m_info;


	if (m_info->context_type != RETRO_HW_CONTEXT_NONE)
	{
		this->ResizeFrameFromRGB888(m_info->lastframedata, dest, m_info->lastframewidth, m_info->lastframeheight, m_info->lastframepitch, 3, width, height, pitch, depth);
	}
	else
	{
		if (m_info->videoformat == RETRO_PIXEL_FORMAT_RGB565)
			this->ResizeFrameFromRGB565(m_info->lastframedata, dest, m_info->lastframewidth, m_info->lastframeheight, m_info->lastframepitch, 3, width, height, pitch, depth);
		else if (m_info->videoformat == RETRO_PIXEL_FORMAT_XRGB8888)
			this->ResizeFrameFromXRGB8888(m_info->lastframedata, dest, m_info->lastframewidth, m_info->lastframeheight, m_info->lastframepitch, 4, width, height, pitch, depth);
		else
			this->ResizeFrameFromRGB1555(m_info->lastframedata, dest, m_info->lastframewidth, m_info->lastframeheight, m_info->lastframepitch, 3, width, height, pitch, depth);
	}

	m_info->copyingframe = false;
}

void C_LibretroInstance::OnProxyBind(C_BaseEntity* pBaseEntity)
{
	if (g_pAnarchyManager->GetSuspendEmbedded())
		return;

//	if (m_id == "images")
//		return;

	/*
	if ( pBaseEntity )
	DevMsg("WebTab: OnProxyBind: %i\n", pBaseEntity->entindex());
	else
	DevMsg("WebTab: OnProxyBind\n");
	*/

	// visiblity test
	if (m_iLastVisibleFrame < gpGlobals->framecount)
	{
		if (!g_pAnarchyManager->GetCanvasManager()->IsPriorityEmbeddedInstance(this))
			g_pAnarchyManager->GetCanvasManager()->IncrementVisibleCanvasesCurrentFrame();
		else
			g_pAnarchyManager->GetCanvasManager()->IncrementVisiblePriorityCanvasesCurrentFrame();
	}
	m_iLastVisibleFrame = gpGlobals->framecount;

	if (m_iLastRenderedFrame < gpGlobals->framecount)
	{
		/*
		if (!g_pAnarchyManager->GetCanvasManager()->IsPriorityEmbeddedInstance(this))
			g_pAnarchyManager->GetCanvasManager()->IncrementVisibleCanvasesCurrentFrame();
		else
			g_pAnarchyManager->GetCanvasManager()->IncrementVisiblePriorityCanvasesCurrentFrame();
		*/

		if (g_pAnarchyManager->GetCanvasManager()->ShouldRender(this) && m_info->readytocopyframe)
			Render();
	}
}

void C_LibretroInstance::Render()
{
//	if (m_id == "images")
	//	return;
	//DevMsg("Rendering texture: %s\n", m_pTexture->GetName());
	//	DevMsg("Render Web Tab: %s\n", this->GetTexture()->Ge>GetId().c_str());
	//DevMsg("WebTab: Render: %s on %i\n", m_id.c_str(), gpGlobals->framecount);
	g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen()->SetEmbeddedInstance(this);
	m_pTexture->Download();
	g_pAnarchyManager->GetCanvasManager()->GetOrCreateRegen()->SetEmbeddedInstance(null);

	m_iLastRenderedFrame = gpGlobals->framecount;

	if (g_pAnarchyManager->GetCanvasManager()->IsPriorityEmbeddedInstance(this))
		g_pAnarchyManager->GetCanvasManager()->SetLastPriorityRenderedFrame(gpGlobals->framecount);
	else
		g_pAnarchyManager->GetCanvasManager()->SetLastRenderedFrame(gpGlobals->framecount);
}

void C_LibretroInstance::RegenerateTextureBits(ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect)
{
	if (g_pAnarchyManager->GetSuspendEmbedded())
		return;

	if (m_info->state == 5)
		this->CopyLastFrame(pVTFTexture->ImageData(0, 0, 0), pSubRect->width, pSubRect->height, pSubRect->width * 4, 4);
}

C_InputListener* C_LibretroInstance::GetInputListener()
{
	return g_pAnarchyManager->GetLibretroManager()->GetInputListener();
}

bool C_LibretroInstance::SetGame(std::string file)
{
	if (!m_info || m_info->gameloaded || m_info->close)
		return false;

	m_info->game = file;
	return true;
}

C_EmbeddedInstance* C_LibretroInstance::GetParentSelectedEmbeddedInstance()
{
	return g_pAnarchyManager->GetLibretroManager()->GetSelectedLibretroInstance();
}

void C_LibretroInstance::SaveLibretroKeybind(std::string type, unsigned int retroport, unsigned int retrodevice, unsigned int retroindex, unsigned int retrokey, std::string steamkey)
{
	// pretty CORE
	std::string prettyCore = m_info->core;
	size_t found = prettyCore.find_last_of("/\\");
	if (found != std::string::npos)
		prettyCore = prettyCore.substr(found + 1);

	found = prettyCore.find_last_of(".");
	if (found != std::string::npos)
		prettyCore = prettyCore.substr(0, found);
	prettyCore.erase(std::remove(prettyCore.begin(), prettyCore.end(), '.'), prettyCore.end());
	
	// pretty GAME
	std::string prettyGame = m_info->game;
	found = prettyGame.find_last_of("/\\");
	if (found != std::string::npos)
		prettyGame = prettyGame.substr(found + 1);

	found = prettyGame.find_last_of(".");
	if (found != std::string::npos)
		prettyGame = prettyGame.substr(0, found);
	prettyGame.erase(std::remove(prettyGame.begin(), prettyGame.end(), '.'), prettyGame.end());

	//std::replace(prettyGame.begin(), prettyGame.end(), '.', '-');

	// now do keybind stuff
	KeyValues* kv;
	std::string savePath;
	if (type == "libretro")
	{
		kv = m_info->libretrokeybinds;	// LIBRETRO-WIDE KEYBINDS
		savePath = "libretro\\user";
	}
	else if (type == "core")
	{
		kv = m_info->corekeybinds;	// CORE-SPECIFIC KEYBINDS
		savePath = "libretro\\user\\" + prettyCore;
	}
	else if (type == "game")
	{
		kv = m_info->gamekeybinds;	// GAME-SPECIFIC KEYBINDS
		savePath = "libretro\\user\\" + prettyCore + "\\" + prettyGame;
	}

	// add the info to the KV: PORT/INDEX/TYPE/RETORKEY = STEAMKEY
	kv->SetString(VarArgs("port%u/device%u/index%u/key%u", retroport, retrodevice, retroindex, retrokey), steamkey.c_str());

	// save the KV out
	// (load up a fresh version and write ONLY this value to it to avoid saving other shit that we don't really want to save at this time.)
	KeyValues* fresh;
	if (type != "libretro")
	{
		fresh = new KeyValues("keybinds");
		fresh->LoadFromFile(g_pFullFileSystem, VarArgs("%s\\keybinds.key", savePath.c_str()), "DEFAULT_WRITE_PATH");
	}
	else
		fresh = kv;

	if (steamkey != "default")
		fresh->SetString(VarArgs("port%u/device%u/index%u/key%u", retroport, retrodevice, retroindex, retrokey), steamkey.c_str());
	else
		fresh->SetString(VarArgs("port%u/device%u/index%u/key%u", retroport, retrodevice, retroindex, retrokey), "");

	g_pFullFileSystem->CreateDirHierarchy(savePath.c_str(), "DEFAULT_WRITE_PATH");
	fresh->SaveToFile(g_pFullFileSystem, VarArgs("%s\\keybinds.key", savePath.c_str()), "DEFAULT_WRITE_PATH");
	if (type != "libretro")
		fresh->deleteThis();

	//g_pFullFileSystem->CreateDirHierarchy(savePath.c_str(), "DEFAULT_WRITE_PATH");
	//kv->SaveToFile(g_pFullFileSystem, VarArgs("%s\\keybinds.key", savePath.c_str()), "DEFAULT_WRITE_PATH");

	// update the ACTIVE keymap of pointers // OBSOLETE: just check all 3 KV's in hiarchy each poll
	//retro_key retrokeyresolved = g_pAnarchyManager->GetLibretroManager()->StringToRetroKeyEnum(retrokey);
	//vgui::KeyCode steamkeyresolved = g_pAnarchyManager->GetInputManager()->StringToSteamKeyEnum(steamkey);
	//m_info->activekeybinds->SetString(VarArgs("port%u/%s/%s", retroport, retrotype.c_str(), retrokey.c_str()), steamkey.c_str());
}

void C_LibretroInstance::SaveLibretroOption(std::string type, std::string name_internal, std::string value)
{
	// pretty CORE
	std::string prettyCore = m_info->core;
	size_t found = prettyCore.find_last_of("/\\");
	if (found != std::string::npos)
		prettyCore = prettyCore.substr(found + 1);

	found = prettyCore.find_last_of(".");
	if (found != std::string::npos)
		prettyCore = prettyCore.substr(0, found);
	prettyCore.erase(std::remove(prettyCore.begin(), prettyCore.end(), '.'), prettyCore.end());

	// pretty GAME
	std::string prettyGame = m_info->game;
	found = prettyGame.find_last_of("/\\");
	if (found != std::string::npos)
		prettyGame = prettyGame.substr(found + 1);

	found = prettyGame.find_last_of(".");
	if (found != std::string::npos)
		prettyGame = prettyGame.substr(0, found);
	prettyGame.erase(std::remove(prettyGame.begin(), prettyGame.end(), '.'), prettyGame.end());

	// now do keybind stuff
	KeyValues* kv;
	std::string savePath;
	if (type == "core")
	{
		kv = m_info->coreCoreOptions;	// CORE-SPECIFIC OPTIONS
		savePath = "libretro\\user\\" + prettyCore;
	}
	else if (type == "game")
	{
		kv = m_info->gameCoreOptions;	// GAME-SPECIFIC OPTIONS
		savePath = "libretro\\user\\" + prettyCore + "\\" + prettyGame;
	}

	// add the info to the KV: NAME_INTERNAL = VALUE
	kv->SetString(name_internal.c_str(), value.c_str());
	m_info->optionshavechanged = true;

	// save the KV out
	// (load up a fresh version and write ONLY this value to it to avoid saving other shit that we don't really want to save at this time.)
	KeyValues* fresh = new KeyValues("options");
	fresh->LoadFromFile(g_pFullFileSystem, VarArgs("%s\\options.key", savePath.c_str()), "DEFAULT_WRITE_PATH");
	if (value != "default")
		fresh->SetString(name_internal.c_str(), value.c_str());
	else
		fresh->SetString(name_internal.c_str(), "");

	g_pFullFileSystem->CreateDirHierarchy(savePath.c_str(), "DEFAULT_WRITE_PATH");
	fresh->SaveToFile(g_pFullFileSystem, VarArgs("%s\\options.key", savePath.c_str()), "DEFAULT_WRITE_PATH");
	fresh->deleteThis();
}
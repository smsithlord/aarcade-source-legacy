#include "cbase.h"

//#include "aa_globals.h"
#include "c_libretromanager.h"
#include "c_anarchymanager.h"
//#include "Filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_LibretroManager::C_LibretroManager()
{
	DevMsg("LibretroManager: Constructor\n");

	m_pWaitForLibretroConVar = cvar->FindVar("wait_for_libretro");

	m_pRunningLibretroCores = new RunningLibretroCores_t;
	m_pRunningLibretroCores->count = 0;
	m_pRunningLibretroCores->last_error = "";
	m_pRunningLibretroCores->last_msg = "";
	m_iPreviousRunningCoreCount = 0;

	m_corePath = "\\libretro\\cores";

	m_userBase = engine->GetGameDirectory();
	size_t found = m_userBase.find_last_of("/\\");
	if (found != std::string::npos)
		m_userBase = m_userBase.substr(0, found);

	m_userBase += "\\aarcade_user";
	m_assetsPath = m_userBase + "\\libretro\\assets";
	m_systemPath = m_userBase + "\\libretro\\system";
	m_savePath = m_userBase + "\\libretro\\save";
	m_userPath = m_userBase + "\\libretro\\user";

	m_pCoreSettingsKV = new KeyValues("cores");
	m_pCoreSettingsKV->LoadFromFile(g_pFullFileSystem, VarArgs("%s\\coreSettings.txt", m_userPath.c_str()));
	for (KeyValues *sub = m_pCoreSettingsKV->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		sub->SetBool("exists", false);	// assume nothing exists until their DLL is found

	m_pOverlaysKV = new KeyValues("overlays");
	m_pOverlaysKV->LoadFromFile(g_pFullFileSystem, VarArgs("%s\\overlaySettings.txt", m_userPath.c_str()));

	bool bAlreadyExists;
	FileFindHandle_t findHandle;
	KeyValues* pTargetKV;
	const char *pFilename = g_pFullFileSystem->FindFirstEx("libretro\\cores\\*.dll", "MOD", &findHandle);
	while (pFilename != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(findHandle))
		{
			pFilename = g_pFullFileSystem->FindNext(findHandle);
			continue;
		}

		// Now scan for DLL files and populate the core settings with anything missing.
		bAlreadyExists = false;
		for (KeyValues *sub = m_pCoreSettingsKV->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		{
			if (!Q_stricmp(pFilename, sub->GetString("file")))
			{
				bAlreadyExists = true;
				sub->SetBool("exists", true);
				break;
			}
		}

		if (!bAlreadyExists)
		{
			// Add an entry for this to the KV!
			pTargetKV = m_pCoreSettingsKV->CreateNewKey();
			pTargetKV->SetName("core");
			pTargetKV->SetString("file", pFilename);
			pTargetKV->SetBool("exists", true);
			pTargetKV->SetInt("priority", 0);

			if (!Q_stricmp(pFilename, "ffmpeg_libretro.dll"))
			{
				pTargetKV->SetBool("enabled", true);
				pTargetKV->SetString("paths/path/path", "");
				pTargetKV->SetString("paths/path/extensions", "avi, mpg, mp4, mpeg, vob, mkv");
			}
			else
				pTargetKV->SetBool("enabled", false);
		}

		pFilename = g_pFullFileSystem->FindNext(findHandle);
	}
	g_pFullFileSystem->FindClose(findHandle);

	m_pBlacklistedDLLsKV = new KeyValues("blacklist");
	// First, try to load from user folder
	if (!g_pFullFileSystem->FileExists(VarArgs("%s%s\\blacklist.txt", m_userBase.c_str(), m_corePath.c_str())) || !m_pBlacklistedDLLsKV->LoadFromFile(g_pFullFileSystem, VarArgs("%s%s\\blacklist.txt", m_userBase.c_str(), m_corePath.c_str())))
	{
		if (!m_pBlacklistedDLLsKV->LoadFromFile(g_pFullFileSystem, VarArgs("%s%s\\blacklist.txt", engine->GetGameDirectory(), m_corePath.c_str())))	// otherwise, load from frontend folder
			DevMsg("ERROR: Could not load %s%s\\blacklist.txt\n", engine->GetGameDirectory(), m_corePath.c_str());
	}

	m_bSoundEnabled = true;
	m_bGUIGamepadEnabled = cvar->FindVar("libretro_gui_gamepad")->GetBool();
	m_pGUIGamepadStateKV = new KeyValues("gamepad");
	m_pSelectedLibretroInstance = null;
	m_pFocusedLibretroInstance = null;

	m_retroKeyJoypadMap["RETRO_DEVICE_ID_JOYPAD_B"] = RETRO_DEVICE_ID_JOYPAD_B;
	m_retroKeyJoypadMap["RETRO_DEVICE_ID_JOYPAD_Y"] = RETRO_DEVICE_ID_JOYPAD_Y;
	m_retroKeyJoypadMap["RETRO_DEVICE_ID_JOYPAD_SELECT"] = RETRO_DEVICE_ID_JOYPAD_SELECT;
	m_retroKeyJoypadMap["RETRO_DEVICE_ID_JOYPAD_START"] = RETRO_DEVICE_ID_JOYPAD_START;
	m_retroKeyJoypadMap["RETRO_DEVICE_ID_JOYPAD_UP"] = RETRO_DEVICE_ID_JOYPAD_UP;
	m_retroKeyJoypadMap["RETRO_DEVICE_ID_JOYPAD_DOWN"] = RETRO_DEVICE_ID_JOYPAD_DOWN;
	m_retroKeyJoypadMap["RETRO_DEVICE_ID_JOYPAD_LEFT"] = RETRO_DEVICE_ID_JOYPAD_LEFT;
	m_retroKeyJoypadMap["RETRO_DEVICE_ID_JOYPAD_RIGHT"] = RETRO_DEVICE_ID_JOYPAD_RIGHT;
	m_retroKeyJoypadMap["RETRO_DEVICE_ID_JOYPAD_A"] = RETRO_DEVICE_ID_JOYPAD_A;
	m_retroKeyJoypadMap["RETRO_DEVICE_ID_JOYPAD_X"] = RETRO_DEVICE_ID_JOYPAD_X;
	m_retroKeyJoypadMap["RETRO_DEVICE_ID_JOYPAD_L"] = RETRO_DEVICE_ID_JOYPAD_L;
	m_retroKeyJoypadMap["RETRO_DEVICE_ID_JOYPAD_R"] = RETRO_DEVICE_ID_JOYPAD_R;
	m_retroKeyJoypadMap["RETRO_DEVICE_ID_JOYPAD_L2"] = RETRO_DEVICE_ID_JOYPAD_L2;
	m_retroKeyJoypadMap["RETRO_DEVICE_ID_JOYPAD_R2"] = RETRO_DEVICE_ID_JOYPAD_R2;
	m_retroKeyJoypadMap["RETRO_DEVICE_ID_JOYPAD_L3"] = RETRO_DEVICE_ID_JOYPAD_L3;
	m_retroKeyJoypadMap["RETRO_DEVICE_ID_JOYPAD_R3"] = RETRO_DEVICE_ID_JOYPAD_R3;
	m_retroKeyMouseMap["RETRO_DEVICE_ID_MOUSE_X"] = RETRO_DEVICE_ID_MOUSE_X;
	m_retroKeyMouseMap["RETRO_DEVICE_ID_MOUSE_Y"] = RETRO_DEVICE_ID_MOUSE_Y;
	m_retroKeyMouseMap["RETRO_DEVICE_ID_MOUSE_LEFT"] = RETRO_DEVICE_ID_MOUSE_LEFT;
	m_retroKeyMouseMap["RETRO_DEVICE_ID_MOUSE_RIGHT"] = RETRO_DEVICE_ID_MOUSE_RIGHT;
	m_retroKeyMouseMap["RETRO_DEVICE_ID_MOUSE_WHEELUP"] = RETRO_DEVICE_ID_MOUSE_WHEELUP;
	m_retroKeyMouseMap["RETRO_DEVICE_ID_MOUSE_WHEELDOWN"] = RETRO_DEVICE_ID_MOUSE_WHEELDOWN;
	m_retroKeyMouseMap["RETRO_DEVICE_ID_MOUSE_MIDDLE"] = RETRO_DEVICE_ID_MOUSE_MIDDLE;
	m_retroKeyMouseMap["RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELUP"] = RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELUP;
	m_retroKeyMouseMap["RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN"] = RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN;
	m_retroKeyLightgunMap["RETRO_DEVICE_ID_LIGHTGUN_X"] = RETRO_DEVICE_ID_LIGHTGUN_X;
	m_retroKeyLightgunMap["RETRO_DEVICE_ID_LIGHTGUN_Y"] = RETRO_DEVICE_ID_LIGHTGUN_Y;
	m_retroKeyLightgunMap["RETRO_DEVICE_ID_LIGHTGUN_TRIGGER"] = RETRO_DEVICE_ID_LIGHTGUN_TRIGGER;
	m_retroKeyLightgunMap["RETRO_DEVICE_ID_LIGHTGUN_CURSOR"] = RETRO_DEVICE_ID_LIGHTGUN_CURSOR;
	m_retroKeyLightgunMap["RETRO_DEVICE_ID_LIGHTGUN_TURBO"] = RETRO_DEVICE_ID_LIGHTGUN_TURBO;
	m_retroKeyLightgunMap["RETRO_DEVICE_ID_LIGHTGUN_PAUSE"] = RETRO_DEVICE_ID_LIGHTGUN_PAUSE;
	m_retroKeyLightgunMap["RETRO_DEVICE_ID_LIGHTGUN_START"] = RETRO_DEVICE_ID_LIGHTGUN_START;
	m_retroKeyAnalogMap["RETRO_DEVICE_INDEX_ANALOG_LEFT"] = RETRO_DEVICE_INDEX_ANALOG_LEFT;
	m_retroKeyAnalogMap["RETRO_DEVICE_INDEX_ANALOG_RIGHT"] = RETRO_DEVICE_INDEX_ANALOG_RIGHT;
	m_retroKeyAnalogMap["RETRO_DEVICE_ID_ANALOG_X"] = RETRO_DEVICE_ID_ANALOG_X;
	m_retroKeyAnalogMap["RETRO_DEVICE_ID_ANALOG_Y"] = RETRO_DEVICE_ID_ANALOG_Y;
	m_retroKeyPointerMap["RETRO_DEVICE_ID_POINTER_X"] = RETRO_DEVICE_ID_POINTER_X;
	m_retroKeyPointerMap["RETRO_DEVICE_ID_POINTER_Y"] = RETRO_DEVICE_ID_POINTER_Y;
	m_retroKeyPointerMap["RETRO_DEVICE_ID_POINTER_PRESSED"] = RETRO_DEVICE_ID_POINTER_PRESSED;

	m_retroKeyKeyboardMap["RETROK_UNKNOWN"] = RETROK_UNKNOWN;
	m_retroKeyKeyboardMap["RETROK_FIRST"] = RETROK_FIRST;
	m_retroKeyKeyboardMap["RETROK_BACKSPACE"] = RETROK_BACKSPACE;
	m_retroKeyKeyboardMap["RETROK_TAB"] = RETROK_TAB;
	m_retroKeyKeyboardMap["RETROK_CLEAR"] = RETROK_CLEAR;
	m_retroKeyKeyboardMap["RETROK_RETURN"] = RETROK_RETURN;
	m_retroKeyKeyboardMap["RETROK_PAUSE"] = RETROK_PAUSE;
	m_retroKeyKeyboardMap["RETROK_ESCAPE"] = RETROK_ESCAPE;
	m_retroKeyKeyboardMap["RETROK_SPACE"] = RETROK_SPACE;
	m_retroKeyKeyboardMap["RETROK_EXCLAIM"] = RETROK_EXCLAIM;
	m_retroKeyKeyboardMap["RETROK_QUOTEDBL"] = RETROK_QUOTEDBL;
	m_retroKeyKeyboardMap["RETROK_HASH"] = RETROK_HASH;
	m_retroKeyKeyboardMap["RETROK_DOLLAR"] = RETROK_DOLLAR;
	m_retroKeyKeyboardMap["RETROK_AMPERSAND"] = RETROK_AMPERSAND;
	m_retroKeyKeyboardMap["RETROK_QUOTE"] = RETROK_QUOTE;
	m_retroKeyKeyboardMap["RETROK_LEFTPAREN"] = RETROK_LEFTPAREN;
	m_retroKeyKeyboardMap["RETROK_RIGHTPAREN"] = RETROK_RIGHTPAREN;
	m_retroKeyKeyboardMap["RETROK_ASTERISK"] = RETROK_ASTERISK;
	m_retroKeyKeyboardMap["RETROK_PLUS"] = RETROK_PLUS;
	m_retroKeyKeyboardMap["RETROK_COMMA"] = RETROK_COMMA;
	m_retroKeyKeyboardMap["RETROK_MINUS"] = RETROK_MINUS;
	m_retroKeyKeyboardMap["RETROK_PERIOD"] = RETROK_PERIOD;
	m_retroKeyKeyboardMap["RETROK_SLASH"] = RETROK_SLASH;
	m_retroKeyKeyboardMap["RETROK_0"] = RETROK_0;
	m_retroKeyKeyboardMap["RETROK_1"] = RETROK_1;
	m_retroKeyKeyboardMap["RETROK_2"] = RETROK_2;
	m_retroKeyKeyboardMap["RETROK_3"] = RETROK_3;
	m_retroKeyKeyboardMap["RETROK_4"] = RETROK_4;
	m_retroKeyKeyboardMap["RETROK_5"] = RETROK_5;
	m_retroKeyKeyboardMap["RETROK_6"] = RETROK_6;
	m_retroKeyKeyboardMap["RETROK_7"] = RETROK_7;
	m_retroKeyKeyboardMap["RETROK_8"] = RETROK_8;
	m_retroKeyKeyboardMap["RETROK_9"] = RETROK_9;
	m_retroKeyKeyboardMap["RETROK_COLON"] = RETROK_COLON;
	m_retroKeyKeyboardMap["RETROK_SEMICOLON"] = RETROK_SEMICOLON;
	m_retroKeyKeyboardMap["RETROK_LESS"] = RETROK_LESS;
	m_retroKeyKeyboardMap["RETROK_EQUALS"] = RETROK_EQUALS;
	m_retroKeyKeyboardMap["RETROK_GREATER"] = RETROK_GREATER;
	m_retroKeyKeyboardMap["RETROK_QUESTION"] = RETROK_QUESTION;
	m_retroKeyKeyboardMap["RETROK_AT"] = RETROK_AT;
	m_retroKeyKeyboardMap["RETROK_LEFTBRACKET"] = RETROK_LEFTBRACKET;
	m_retroKeyKeyboardMap["RETROK_BACKSLASH"] = RETROK_BACKSLASH;
	m_retroKeyKeyboardMap["RETROK_RIGHTBRACKET"] = RETROK_RIGHTBRACKET;
	m_retroKeyKeyboardMap["RETROK_CARET"] = RETROK_CARET;
	m_retroKeyKeyboardMap["RETROK_UNDERSCORE"] = RETROK_UNDERSCORE;
	m_retroKeyKeyboardMap["RETROK_BACKQUOTE"] = RETROK_BACKQUOTE;
	m_retroKeyKeyboardMap["RETROK_a"] = RETROK_a;
	m_retroKeyKeyboardMap["RETROK_b"] = RETROK_b;
	m_retroKeyKeyboardMap["RETROK_c"] = RETROK_c;
	m_retroKeyKeyboardMap["RETROK_d"] = RETROK_d;
	m_retroKeyKeyboardMap["RETROK_e"] = RETROK_e;
	m_retroKeyKeyboardMap["RETROK_f"] = RETROK_f;
	m_retroKeyKeyboardMap["RETROK_g"] = RETROK_g;
	m_retroKeyKeyboardMap["RETROK_h"] = RETROK_h;
	m_retroKeyKeyboardMap["RETROK_i"] = RETROK_i;
	m_retroKeyKeyboardMap["RETROK_j"] = RETROK_j;
	m_retroKeyKeyboardMap["RETROK_k"] = RETROK_k;
	m_retroKeyKeyboardMap["RETROK_l"] = RETROK_l;
	m_retroKeyKeyboardMap["RETROK_m"] = RETROK_m;
	m_retroKeyKeyboardMap["RETROK_n"] = RETROK_n;
	m_retroKeyKeyboardMap["RETROK_o"] = RETROK_o;
	m_retroKeyKeyboardMap["RETROK_p"] = RETROK_p;
	m_retroKeyKeyboardMap["RETROK_q"] = RETROK_q;
	m_retroKeyKeyboardMap["RETROK_r"] = RETROK_r;
	m_retroKeyKeyboardMap["RETROK_s"] = RETROK_s;
	m_retroKeyKeyboardMap["RETROK_t"] = RETROK_t;
	m_retroKeyKeyboardMap["RETROK_u"] = RETROK_u;
	m_retroKeyKeyboardMap["RETROK_v"] = RETROK_v;
	m_retroKeyKeyboardMap["RETROK_w"] = RETROK_w;
	m_retroKeyKeyboardMap["RETROK_x"] = RETROK_x;
	m_retroKeyKeyboardMap["RETROK_y"] = RETROK_y;
	m_retroKeyKeyboardMap["RETROK_z"] = RETROK_z;
	m_retroKeyKeyboardMap["RETROK_DELETE"] = RETROK_DELETE;
	m_retroKeyKeyboardMap["RETROK_KP0"] = RETROK_KP0;
	m_retroKeyKeyboardMap["RETROK_KP1"] = RETROK_KP1;
	m_retroKeyKeyboardMap["RETROK_KP2"] = RETROK_KP2;
	m_retroKeyKeyboardMap["RETROK_KP3"] = RETROK_KP3;
	m_retroKeyKeyboardMap["RETROK_KP4"] = RETROK_KP4;
	m_retroKeyKeyboardMap["RETROK_KP5"] = RETROK_KP5;
	m_retroKeyKeyboardMap["RETROK_KP6"] = RETROK_KP6;
	m_retroKeyKeyboardMap["RETROK_KP7"] = RETROK_KP7;
	m_retroKeyKeyboardMap["RETROK_KP8"] = RETROK_KP8;
	m_retroKeyKeyboardMap["RETROK_KP9"] = RETROK_KP9;
	m_retroKeyKeyboardMap["RETROK_KP_PERIOD"] = RETROK_KP_PERIOD;
	m_retroKeyKeyboardMap["RETROK_KP_DIVIDE"] = RETROK_KP_DIVIDE;
	m_retroKeyKeyboardMap["RETROK_KP_MULTIPLY"] = RETROK_KP_MULTIPLY;
	m_retroKeyKeyboardMap["RETROK_KP_MINUS"] = RETROK_KP_MINUS;
	m_retroKeyKeyboardMap["RETROK_KP_PLUS"] = RETROK_KP_PLUS;
	m_retroKeyKeyboardMap["RETROK_KP_ENTER"] = RETROK_KP_ENTER;
	m_retroKeyKeyboardMap["RETROK_KP_EQUALS"] = RETROK_KP_EQUALS;
	m_retroKeyKeyboardMap["RETROK_UP"] = RETROK_UP;
	m_retroKeyKeyboardMap["RETROK_DOWN"] = RETROK_DOWN;
	m_retroKeyKeyboardMap["RETROK_RIGHT"] = RETROK_RIGHT;
	m_retroKeyKeyboardMap["RETROK_LEFT"] = RETROK_LEFT;
	m_retroKeyKeyboardMap["RETROK_INSERT"] = RETROK_INSERT;
	m_retroKeyKeyboardMap["RETROK_HOME"] = RETROK_HOME;
	m_retroKeyKeyboardMap["RETROK_END"] = RETROK_END;
	m_retroKeyKeyboardMap["RETROK_PAGEUP"] = RETROK_PAGEUP;
	m_retroKeyKeyboardMap["RETROK_PAGEDOWN"] = RETROK_PAGEDOWN;
	m_retroKeyKeyboardMap["RETROK_F1"] = RETROK_F1;
	m_retroKeyKeyboardMap["RETROK_F2"] = RETROK_F2;
	m_retroKeyKeyboardMap["RETROK_F3"] = RETROK_F3;
	m_retroKeyKeyboardMap["RETROK_F4"] = RETROK_F4;
	m_retroKeyKeyboardMap["RETROK_F5"] = RETROK_F5;
	m_retroKeyKeyboardMap["RETROK_F6"] = RETROK_F6;
	m_retroKeyKeyboardMap["RETROK_F7"] = RETROK_F7;
	m_retroKeyKeyboardMap["RETROK_F8"] = RETROK_F8;
	m_retroKeyKeyboardMap["RETROK_F9"] = RETROK_F9;
	m_retroKeyKeyboardMap["RETROK_F10"] = RETROK_F10;
	m_retroKeyKeyboardMap["RETROK_F11"] = RETROK_F11;
	m_retroKeyKeyboardMap["RETROK_F12"] = RETROK_F12;
	m_retroKeyKeyboardMap["RETROK_F13"] = RETROK_F13;
	m_retroKeyKeyboardMap["RETROK_F14"] = RETROK_F14;
	m_retroKeyKeyboardMap["RETROK_F15"] = RETROK_F15;
	m_retroKeyKeyboardMap["RETROK_NUMLOCK"] = RETROK_NUMLOCK;
	m_retroKeyKeyboardMap["RETROK_CAPSLOCK"] = RETROK_CAPSLOCK;
	m_retroKeyKeyboardMap["RETROK_SCROLLOCK"] = RETROK_SCROLLOCK;
	m_retroKeyKeyboardMap["RETROK_RSHIFT"] = RETROK_RSHIFT;
	m_retroKeyKeyboardMap["RETROK_LSHIFT"] = RETROK_LSHIFT;
	m_retroKeyKeyboardMap["RETROK_RCTRL"] = RETROK_RCTRL;
	m_retroKeyKeyboardMap["RETROK_LCTRL"] = RETROK_LCTRL;
	m_retroKeyKeyboardMap["RETROK_RALT"] = RETROK_RALT;
	m_retroKeyKeyboardMap["RETROK_LALT"] = RETROK_LALT;
	m_retroKeyKeyboardMap["RETROK_RMETA"] = RETROK_RMETA;
	m_retroKeyKeyboardMap["RETROK_LMETA"] = RETROK_LMETA;
	m_retroKeyKeyboardMap["RETROK_LSUPER"] = RETROK_LSUPER;
	m_retroKeyKeyboardMap["RETROK_RSUPER"] = RETROK_RSUPER;
	m_retroKeyKeyboardMap["RETROK_MODE"] = RETROK_MODE;
	m_retroKeyKeyboardMap["RETROK_COMPOSE"] = RETROK_COMPOSE;
	m_retroKeyKeyboardMap["RETROK_HELP"] = RETROK_HELP;
	m_retroKeyKeyboardMap["RETROK_PRINT"] = RETROK_PRINT;
	m_retroKeyKeyboardMap["RETROK_SYSREQ"] = RETROK_SYSREQ;
	m_retroKeyKeyboardMap["RETROK_BREAK"] = RETROK_BREAK;
	m_retroKeyKeyboardMap["RETROK_MENU"] = RETROK_MENU;
	m_retroKeyKeyboardMap["RETROK_POWER"] = RETROK_POWER;
	m_retroKeyKeyboardMap["RETROK_EURO"] = RETROK_EURO;
	m_retroKeyKeyboardMap["RETROK_UNDO"] = RETROK_UNDO;

	m_retroDeviceMap["RETRO_DEVICE_NONE"] = RETRO_DEVICE_NONE;
	m_retroDeviceMap["RETRO_DEVICE_JOYPAD"] = RETRO_DEVICE_JOYPAD;
	m_retroDeviceMap["RETRO_DEVICE_MOUSE"] = RETRO_DEVICE_MOUSE;
	m_retroDeviceMap["RETRO_DEVICE_KEYBOARD"] = RETRO_DEVICE_KEYBOARD;
	m_retroDeviceMap["RETRO_DEVICE_LIGHTGUN"] = RETRO_DEVICE_LIGHTGUN;
	m_retroDeviceMap["RETRO_DEVICE_ANALOG"] = RETRO_DEVICE_ANALOG;
	m_retroDeviceMap["RETRO_DEVICE_POINTER"] = RETRO_DEVICE_POINTER;

	m_pInputListener = new C_InputListenerLibretro();
}

C_LibretroManager::~C_LibretroManager()
{
	DevMsg("LibretroManager: Destructor\n");
	this->CloseAllInstances();

	if (m_pInputListener)
		delete m_pInputListener;

	m_pGUIGamepadStateKV->deleteThis();
	m_pCoreSettingsKV->deleteThis();
	m_pBlacklistedDLLsKV->deleteThis();
	m_pOverlaysKV->deleteThis();

	delete m_pRunningLibretroCores;	// if any CORES are STILL running after this & try to access this, = CTD
}

#include "vgui/IInput.h"
//int16_t C_LibretroManager::GetInputState(LibretroInstanceInfo_t* info, unsigned int retroport, unsigned int retrodevice, unsigned int retroindex, unsigned int retroid)
//inputstate
void C_LibretroManager::ManageInputUpdate(LibretroInstanceInfo_t* info, unsigned int retroport, unsigned int retrodevice)
{
	//std::string idText;
	//vgui::KeyCode steamKeyCode;
	//std::string keyPath;
	//std::string steamKeyText;


	//if (retroport != 0 || retrodevice != 1)
	//	return;

	// loop through all keys of this device
	if (retrodevice == RETRO_DEVICE_JOYPAD)
	{
		auto it = m_retroKeyJoypadMap.begin();
		while (it != m_retroKeyJoypadMap.end())
		{
			//idText = this->RetroKeyToString(it->second);

			unsigned int maxIndex = 1;
			for (unsigned int j = 0; j < maxIndex; j++)
			{
				info->inputstate->SetInt(VarArgs("port%u/device%u/index%u/key%u", retroport, retrodevice, j, it->second), this->GetInputState(info, retroport, retrodevice, j, it->second));
			}

			it++;
		}
	}

	// TODO: add support for the other retro devices
}

int C_LibretroManager::GetInputState(LibretroInstanceInfo_t* info, unsigned int retroport, unsigned int retrodevice, unsigned int retroindex, unsigned int retroid)
{
	std::string keyPath = "port" + std::to_string(retroport) + "/device" + std::to_string(retrodevice) + "/index" + std::to_string(retroindex) + "/key" + std::to_string(retroid);//VarArgs("port%u/device%u/index%u/key%u", retroport, retrodevice, retroindex, retroid);

	if (m_bGUIGamepadEnabled)
	{
		// note: for buttons, returning non-zero should indiate pressed, so its okay that its an int WAY non-zero (max).
		//int min = -0x8000;
		//int max = 0x7fff;

		// grab a [-1, 1] float
		//float floatVal = m_pGUIGamepadStateKV->GetFloat(keyPath.c_str());//) ? 1.0f : 0.0f;

		// scale the float
		//floatVal *= (floatVal > 0.0f) ? max : min;

		// convert to int
		int intVal = m_pGUIGamepadStateKV->GetInt(keyPath.c_str());//(int)floatVal;
		//DevMsg("Value %s: %i\n", keyPath.c_str(), intVal);

		// make sure it's within range
		if (intVal > 0x7fff)
			intVal = 0x7fff;
		else if (intVal < -0x8000)
			intVal = -0x8000;

		return intVal;
	}
	else
	{
		// = this->RetroKeyToString(retroid);
		//std::string deviceText = this->RetroDeviceToString(retrodevice);

		//DevMsg("Found deviceText: %s\n", retrodevice.c_str());

		//return (int16_t)0;
		// get the steam key code to detect for this retro key
		//std::string keyPath = VarArgs("port%u/%s/index%u/%s", retroport, retrodevice.c_str(), retroindex, retroid.c_str());
		//if ( retroid > 11)
		//	DevMsg("Using retroid %u\n", retroid);

		//std::string keyPath = "port" + std::to_string(retroport) + "/device" + std::to_string(retrodevice) + "/index" + std::to_string(retroindex) + "/key" + std::to_string(retroid);//VarArgs("port%u/device%u/index%u/key%u", retroport, retrodevice, retroindex, retroid);
		//DevMsg("Update keypath: %s\n", keyPath.c_str());

		std::string steamKeyText = info->gamekeybinds->GetString(keyPath.c_str(), "default");

		if (steamKeyText == "default")
		{
			steamKeyText = info->corekeybinds->GetString(keyPath.c_str(), "default");

			if (steamKeyText == "default")
			{
				steamKeyText = info->libretrokeybinds->GetString(keyPath.c_str(), "unbound");

				if (retroid > 11 && steamKeyText != "unbound")
					DevMsg("Using high bind from libretro KV: %s\n", keyPath.c_str());
			}
		}

		//	DevMsg("Steam key: %s\n", steamKeyText.c_str());

		if (steamKeyText == "unbound")
			return 0;

		if (retroid > 11)
			DevMsg("Key %u is bound to: %s\n", retroid, steamKeyText.c_str());

		vgui::KeyCode steamKeyCode = g_pAnarchyManager->GetInputManager()->StringToSteamKeyEnum(steamKeyText);

		if (steamKeyCode == KEY_NONE)
			return 0;

		if (IsJoystickAxisCode(steamKeyCode))
			DevMsg("WARNING: GAMEPAD AXIS CODE DETECTED IN LIBRETRO INPUT!!\n");

		// note: for buttons, returning non-zero should indiate pressed, so its okay that its an int WAY non-zero (max).
		int min = -0x8000;
		int max = 0x7fff;

		// grab a [-1, 1] float
		float floatVal = (vgui::input()->IsKeyDown(steamKeyCode)) ? 1.0f : 0.0f;

		// scale the float
		floatVal *= (floatVal > 0.0f) ? max : min;

		// convert to int
		int intVal = (int)floatVal;

		// make sure it's within range
		if (intVal > 0x7fff)
			intVal = 0x7fff;
		else if (intVal < -0x8000)
			intVal = -0x8000;

		return intVal;

		/*

		if ( IsJoystickButtonCode( code ) )
		{
		int offset = ( code - JOYSTICK_FIRST_BUTTON ) % JOYSTICK_MAX_BUTTON_COUNT;
		return (ButtonCode_t)( JOYSTICK_FIRST_BUTTON + offset );
		}

		if ( IsJoystickPOVCode( code ) )
		{
		int offset = ( code - JOYSTICK_FIRST_POV_BUTTON ) % JOYSTICK_POV_BUTTON_COUNT;
		return (ButtonCode_t)( JOYSTICK_FIRST_POV_BUTTON + offset );
		}

		if ( IsJoystickAxisCode( code ) )
		{
		int offset = ( code - JOYSTICK_FIRST_AXIS_BUTTON ) % JOYSTICK_AXIS_BUTTON_COUNT;
		return (ButtonCode_t)( JOYSTICK_FIRST_AXIS_BUTTON + offset );
		}

		void		GetMousePos(int &x, int &y);

		float		m_flAccumulatedMouseXMovement;
		float		m_flAccumulatedMouseYMovement;
		float		m_flPreviousMouseXPosition;
		float		m_flPreviousMouseYPosition;

		virtual		float		Joystick_GetForward( void );
		virtual		float		Joystick_GetSide( void );
		virtual		float		Joystick_GetPitch( void );
		virtual		float		Joystick_GetYaw( void );

		*/

		// loop through all retro keys
		/*
		if (retrodevice == "RETRO_DEVICE_JOYPAD")
		{
		auto it = m_retroKeyJoypadMap.begin();
		while (it != m_retroKeyJoypadMap.end())
		{
		keyPath = VarArgs("port%u/%s/index%u/%s", retroport, retroindex, retrodevice.c_str(), it->first.c_str());
		text = info->gamekeybinds->GetString(keyPath.c_str(), "default");

		if (text == "default")
		{
		text = info->corekeybinds->GetString(it->first.c_str(), "default");

		if (text == "default")
		text = info->libretrokeybinds->GetString(it->first.c_str(), "unbound");
		}

		code = g_pAnarchyManager->GetInputManager()->StringToSteamKeyEnum(text);

		if (!IsJoystickAxisCode(code))
		{
		info->inputstate[it->first] = Q_atof(VarArgs("%f", vgui::input()->IsKeyDown(code)));	// FIXME: FIX ME RTFN cuz retro "floats" range from some huge int to some huge negative int.
		}
		}
		}
		*/
		/*
		// enum (RetroKeyboard only)
		auto it2 = m_retroKeyUnsignedMap.begin();
		while (it2 != m_retroKeyUnsignedMap.end())
		{
		keyPath = VarArgs("port%u/%s/%s", retroport, retrodevice.c_str(), it2->first.c_str());
		text = info->gamekeybinds->GetString(keyPath.c_str(), "default");

		if (text == "default")
		{
		text = info->corekeybinds->GetString(it2->first.c_str(), "default");

		if (text == "default")
		text = info->libretrokeybinds->GetString(it2->first.c_str(), "unbound");
		}

		code = g_pAnarchyManager->GetInputManager()->StringToSteamKeyEnum(text);

		if (!IsJoystickAxisCode(code))
		info->inputstate[it2->first] = Q_atof(VarArgs("%f", vgui::input()->IsKeyDown(code)));
		}
		*/

		/*
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_SELECT] = vgui::input()->IsKeyDown(KEY_XBUTTON_BACK);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_START] = vgui::input()->IsKeyDown(KEY_XBUTTON_START) || vgui::input()->IsKeyDown(KEY_ENTER);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_UP] = vgui::input()->IsKeyDown(KEY_XBUTTON_UP);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_DOWN] = vgui::input()->IsKeyDown(KEY_XBUTTON_DOWN);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_LEFT] = vgui::input()->IsKeyDown(KEY_XBUTTON_LEFT);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_RIGHT] = vgui::input()->IsKeyDown(KEY_XBUTTON_RIGHT);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_A] = vgui::input()->IsKeyDown(KEY_XBUTTON_B);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_B] = vgui::input()->IsKeyDown(KEY_XBUTTON_A);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_X] = vgui::input()->IsKeyDown(KEY_XBUTTON_Y);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_Y] = vgui::input()->IsKeyDown(KEY_XBUTTON_X);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_L] = vgui::input()->IsKeyDown(KEY_XBUTTON_LEFT_SHOULDER);
		m_info->inputstate[RETRO_DEVICE_ID_JOYPAD_R] = vgui::input()->IsKeyDown(KEY_XBUTTON_RIGHT_SHOULDER);
		*/
	}
}

std::string C_LibretroManager::RetroKeyboardKeyToString(retro_key retrokey)
{
	auto it = m_retroKeyKeyboardMap.begin();
	while (it != m_retroKeyKeyboardMap.end())
	{
		if (it->second == retrokey)
			return it->first.c_str();
	}

	return "RETROK_UNKNOWN";
}

std::string C_LibretroManager::RetroKeyToString(unsigned int retrokey)
{
	auto it = m_retroKeyJoypadMap.begin();
	while (it != m_retroKeyJoypadMap.end())
	{
		if (it->second == retrokey)
			return it->first;
	}

	auto it2 = m_retroKeyMouseMap.begin();
	while (it2 != m_retroKeyMouseMap.end())
	{
		if (it2->second == retrokey)
			return it2->first;
	}

	auto it3 = m_retroKeyKeyboardMap.begin();
	while (it3 != m_retroKeyKeyboardMap.end())
	{
		if (it3->second == retrokey)
			return it3->first;
	}

	auto it4 = m_retroKeyLightgunMap.begin();
	while (it4 != m_retroKeyLightgunMap.end())
	{
		if (it4->second == retrokey)
			return it4->first;
	}

	auto it5 = m_retroKeyAnalogMap.begin();
	while (it5 != m_retroKeyAnalogMap.end())
	{
		if (it5->second == retrokey)
			return it5->first;
	}

	auto it6 = m_retroKeyPointerMap.begin();
	while (it6 != m_retroKeyPointerMap.end())
	{
		if (it6->second == retrokey)
			return it6->first;
	}

	//RETRO_DEVICE_MOUSE

	return "unbound";	// there is no unknown key enum for retro keys that aren't part of the keyboard enum
}

std::string C_LibretroManager::RetroDeviceToString(unsigned int number)
{
	auto it = m_retroDeviceMap.begin();
	while (it != m_retroDeviceMap.end())
	{
		if (it->second == number)
			return it->first;
	}

	return "RETRO_DEVICE_NONE";
}

void C_LibretroManager::CloseAllInstances()
{
	g_pAnarchyManager->GetCanvasManager()->SetDisplayInstance(null);

	unsigned int count = 0;
	// iterate over all web tabs and call their destructors
	for (auto it = m_libretroInstances.begin(); it != m_libretroInstances.end(); ++it)
	{
		C_LibretroInstance* pInstance = it->second;

		if (pInstance->GetId() == "hud" || pInstance->GetId() == "images")
		{
			DevMsg("ERROR: Libretro Browser instance detected that is NOT of type Awesomium Browser!!\n");
			continue;
		}

		DevMsg("Removing 1 Libretro instance...\n");

		if (pInstance == m_pSelectedLibretroInstance)
		{
			this->SelectLibretroInstance(null);
			g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
		}

		//if (g_pAnarchyManager->GetInputManager()->GetInputCanvasTexture() == pInstance->GetTexture())
		if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == pInstance)
		{
			g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);
			//g_pAnarchyManager->GetInputManager()->SetInputListener(null);
			//g_pAnarchyManager->GetInputManager()->SetInputCanvasTexture(null);
		}

		//		auto foundSteamBrowserInstance = m_steamBrowserInstances.find(pSteamBrowserInstance->GetId());
		//		if (foundSteamBrowserInstance != m_steamBrowserInstances.end())
		//			m_steamBrowserInstances.erase(foundSteamBrowserInstance);

		pInstance->SelfDestruct();
	}

	m_libretroInstances.clear();
}

void C_LibretroManager::LevelShutdownPreEntity()
{
	this->CloseAllInstances();
}

void C_LibretroManager::Update()
{
	/*
	for (auto it = m_libretroInstances.begin(); it != m_libretroInstances.end(); ++it)
	{
		C_LibretroInstance* pLibretroInstance = it->second;
		pLibretroInstance->Update();

	}
	*/

	for (auto it = m_libretroInstances.begin(); it != m_libretroInstances.end(); ++it)
	{
		if (g_pAnarchyManager->GetCanvasManager()->IsPriorityEmbeddedInstance(it->second))
			it->second->Update();
	}

	bool bLibretroWaitEnabled = m_pWaitForLibretroConVar->GetBool();
	//if (!bLibretroWaitEnabled)
	//{
		bool bHadError = false;
		if (m_pRunningLibretroCores->last_error != "")
		{
			g_pAnarchyManager->AddToastMessage(VarArgs("Libretro Aborted - %s", m_pRunningLibretroCores->last_error.c_str()));
			m_pRunningLibretroCores->last_error = "";
			bHadError = true;
		}

		bool bHadMsg = false;
		if (m_pRunningLibretroCores->last_msg != "")
		{
			g_pAnarchyManager->AddToastMessage(VarArgs("Libretro - %s", m_pRunningLibretroCores->last_msg.c_str()));
			m_pRunningLibretroCores->last_msg = "";
			bHadMsg = true;
		}
	//}

	if (m_iPreviousRunningCoreCount != m_pRunningLibretroCores->count)
	{
		if (m_iPreviousRunningCoreCount > m_pRunningLibretroCores->count && !bLibretroWaitEnabled )
			g_pAnarchyManager->AddToastMessage(VarArgs("Libretro Closed (%i running)", m_pRunningLibretroCores->count));
		//else
			//g_pAnarchyManager->AddToastMessage(VarArgs("Libretro Core Opened (%i running)", m_pRunningLibretroCores->count));	// printed directly prior to thread being spawned, which is where count is incremented.
			
		
		m_iPreviousRunningCoreCount = m_pRunningLibretroCores->count;
	}

//	if (m_pSelectedLibretroInstance)
	//	m_pSelectedLibretroInstance->Update();

	//DevMsg("LibretroManager: Update\n");
	//info->state = state;
//	if (m_pSelectedLibretroInstance)
//		m_pSelectedLibretroInstance->Update();
}

C_LibretroInstance* C_LibretroManager::CreateLibretroInstance()
{
	this->ClearGUIGamepadInputState();

	C_LibretroInstance* pLibretroInstance = new C_LibretroInstance();
	SelectLibretroInstance(pLibretroInstance);
	return pLibretroInstance;
}

void C_LibretroManager::DestroyLibretroInstance(C_LibretroInstance* pInstance)
{
	if (pInstance == m_pSelectedLibretroInstance)
	{
		this->SelectLibretroInstance(null);
		g_pAnarchyManager->GetInputManager()->DeactivateInputMode(true);
	}

	///*
	if (g_pAnarchyManager->GetCanvasManager()->GetDisplayInstance() == pInstance)
		g_pAnarchyManager->GetCanvasManager()->SetDifferentDisplayInstance(pInstance);
	//*/


	//g_pAnarchyManager->GetCanvasManager()->SetDisplayInstance(null);

	//if (g_pAnarchyManager->GetInputManager()->GetInputCanvasTexture() == pInstance->GetTexture())
	if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance() == pInstance)
	{
		g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(null);
		//g_pAnarchyManager->GetInputManager()->SetInputListener(null);
		//g_pAnarchyManager->GetInputManager()->SetInputCanvasTexture(null);
	}

	auto foundLibretroInstance = m_libretroInstances.find(pInstance->GetInfo()->module);
	if (foundLibretroInstance != m_libretroInstances.end())
		m_libretroInstances.erase(foundLibretroInstance);

	int iCurrentCount = m_pRunningLibretroCores->count;
	pInstance->SelfDestruct();

	// Also clear GUI gamepad inputs & deactivate it
	this->ClearGUIGamepadInputState();

	if (iCurrentCount > 0 && m_pWaitForLibretroConVar->GetBool())
	{
		ConVar* pDummyVar = null;
		int iTesterCount = m_pRunningLibretroCores->count;
		while (iTesterCount == iCurrentCount)
		{
			//DevMsg("Waiting on Libretro...\n");
			pDummyVar = cvar->FindVar("wait_for_libretro");
			iTesterCount = m_pRunningLibretroCores->count;
			// hang while the other thread finishes. :S
		}

		if (pDummyVar)	// in case compiler optimization requires the variable to actually be used
			pDummyVar->SetValue(pDummyVar->GetBool());

		bool bHadError = false;
		if (m_pRunningLibretroCores->last_error != "")
		{
			g_pAnarchyManager->AddToastMessage(VarArgs("Libretro Aborted - %s", m_pRunningLibretroCores->last_error.c_str()));
			m_pRunningLibretroCores->last_error = "";
			bHadError = true;
		}

		bool bHadMsg = false;
		if (m_pRunningLibretroCores->last_msg != "")
		{
			g_pAnarchyManager->AddToastMessage(VarArgs("Libretro - %s", m_pRunningLibretroCores->last_msg.c_str()));
			m_pRunningLibretroCores->last_msg = "";
			bHadMsg = true;
		}

		g_pAnarchyManager->AddToastMessage(VarArgs("Libretro Closed (%i running)", iTesterCount));
	}
}

bool C_LibretroManager::FocusLibretroInstance(C_LibretroInstance* pLibretroInstance)
{
	m_pFocusedLibretroInstance = pLibretroInstance;
	return true;
}

bool C_LibretroManager::SelectLibretroInstance(C_LibretroInstance* pLibretroInstance)
{
	m_pSelectedLibretroInstance = pLibretroInstance;
	return true;
}

void C_LibretroManager::OnLibretroInstanceCreated(LibretroInstanceInfo_t* pInfo)//C_LibretroInstance* pLibretroInstance)
{
	// FIXME: If you un-click fast after selecting something, u might trigger a crash here...
	//LibretroInstanceInfo_t* pInfo = (pLibretroInstance) ? pLibretroInstance->GetInfo() : null;
	if (!pInfo || pInfo->close || !pInfo->libretroinstance)
	{
		DevMsg("Extinct libretro instance ignored.\n");
		return;
	}

	CSysModule* pModule = pInfo->module;
	m_libretroInstances[pModule] = pInfo->libretroinstance;

	uint uId = pInfo->threadid;
	m_libretroInstancesModules[uId] = pModule;

	pInfo->state = 1;
}

C_LibretroInstance* C_LibretroManager::FindLibretroInstance(CSysModule* pModule)
{
	auto foundLibretroInstance = m_libretroInstances.find(pModule);
	if (foundLibretroInstance != m_libretroInstances.end())
	{
		return m_libretroInstances[pModule];
	}
	else
		return null;
}

C_LibretroInstance* C_LibretroManager::FindLibretroInstance(uint uId)
{
	auto foundId = m_libretroInstancesModules.find(uId);
	if (foundId != m_libretroInstancesModules.end())
	{
		CSysModule* pModule = m_libretroInstancesModules[uId];

		auto foundWebTab = m_libretroInstances.find(pModule);
		if (foundWebTab != m_libretroInstances.end())
			return m_libretroInstances[pModule];
	}

	return null;
}

C_LibretroInstance* C_LibretroManager::FindLibretroInstanceByEntityIndex(int iEntityIndex)
{
	auto foundLibretroInstance = m_libretroInstances.begin();
	while (foundLibretroInstance != m_libretroInstances.end())
	{
		if (foundLibretroInstance->second->GetOriginalEntIndex() == iEntityIndex)
			return foundLibretroInstance->second;
		else
			foundLibretroInstance++;
	}

	return null;
}

C_LibretroInstance* C_LibretroManager::FindLibretroInstance(std::string id)
{
	//typedef std::map<std::string, std::map<std::string, std::string>>::iterator it_type;
	auto foundLibretroInstance = m_libretroInstances.begin();
	while (foundLibretroInstance != m_libretroInstances.end())
	{
		if (foundLibretroInstance->second->GetId() == id)
			return foundLibretroInstance->second;
		else
			foundLibretroInstance++;
	}

	return null;
}

void C_LibretroManager::SetVolume(float fVolume)
{
	auto foundLibretroInstance = m_libretroInstances.begin();
	while (foundLibretroInstance != m_libretroInstances.end())
	{
		foundLibretroInstance->second->SetVolume(fVolume);
		foundLibretroInstance++;
	}
}

void C_LibretroManager::RunEmbeddedLibretro(std::string core, std::string file)
{
	C_LibretroInstance* pLibretroInstance = this->CreateLibretroInstance();
	pLibretroInstance->Init("", "Manual Libretro Instance", -1);
	pLibretroInstance->SetOriginalGame(file);
//	pLibretroInstance->SetOriginalItemId(itemId);
	if (!pLibretroInstance->LoadCore(core))	// FIXME: elegantly revert back to autoInspect if loading the core failed!
		DevMsg("ERROR: Failed to load core: %s\n", core.c_str());
	//pEmbeddedInstance = pLibretroInstance;
	//bDoAutoInspect = false;



	// TEST: AUTO-CREATE AN INSTANCE, LOAD THE FFMPEG CORE, AND PLAY A MOVIE
//	C_LibretroInstance* pLibretroInstance = this->CreateLibretroInstance();
//	pLibretroInstance->Init();

	// load a core
	//pLibretroInstance->LoadCore(core);

	// tell the input manager that the libretro instance is active
	//C_InputListenerLibretro* pListener = this->GetInputListener();
	//g_pAnarchyManager->GetInputManager()->SetInputCanvasTexture(pLibretroInstance->GetTexture());
	//pLibretroInstance->Select();
	//g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, pLibretroInstance);

	pLibretroInstance->Focus();
	//g_pAnarchyManager->GetInputManager()->SetEmbeddedInstance(pSteamBrowserInstance);
	g_pAnarchyManager->GetInputManager()->ActivateInputMode(true, true, pLibretroInstance);

	// load a file
	pLibretroInstance->SetGame(file);
}

unsigned int C_LibretroManager::StringToRetroDevice(std::string device)
{
	auto it = m_retroDeviceMap.find(device);
	if (it != m_retroDeviceMap.end())
		return it->second;
}

unsigned int C_LibretroManager::StringToRetroKey(std::string text)
{
	auto it = m_retroKeyJoypadMap.find(text);
	if (it != m_retroKeyJoypadMap.end())
		return it->second;

	auto it2 = m_retroKeyMouseMap.find(text);
	if (it2 != m_retroKeyMouseMap.end())
		return it2->second;

	auto it3 = m_retroKeyKeyboardMap.find(text);
	if (it3 != m_retroKeyKeyboardMap.end())
		return it3->second;

	auto it4 = m_retroKeyLightgunMap.find(text);
	if (it4 != m_retroKeyLightgunMap.end())
		return it4->second;

	auto it5 = m_retroKeyAnalogMap.find(text);
	if (it5 != m_retroKeyAnalogMap.end())
		return it5->second;

	auto it6 = m_retroKeyPointerMap.find(text);
	if (it6 != m_retroKeyPointerMap.end())
		return it6->second;

	return 0;	// FIXME: What if 0 is a valid enum for a retro key?
}

retro_key C_LibretroManager::StringToRetroKeyboardKey(std::string text)
{
	auto it = m_retroKeyKeyboardMap.find(text);
	if (it != m_retroKeyKeyboardMap.end())
		return it->second;

	return RETROK_UNKNOWN;
}

int C_LibretroManager::GetInstanceCount()
{
	return m_pRunningLibretroCores->count;
	/*
	unsigned int count = 0;

	auto it = m_libretroInstances.begin();
	while (it != m_libretroInstances.end())
	{
		count++;
		it++;
	}

	return count;
	*/
}

void C_LibretroManager::SetGUIGamepadInputState(unsigned int retroport, unsigned int retrodevice, unsigned int retroindex, unsigned int retroid, int iValue)
{
	std::string keyPath = "port" + std::to_string(retroport) + "/device" + std::to_string(retrodevice) + "/index" + std::to_string(retroindex) + "/key" + std::to_string(retroid);
	m_pGUIGamepadStateKV->SetInt(keyPath.c_str(), iValue);
}

void C_LibretroManager::ClearGUIGamepadInputState()
{
	m_pGUIGamepadStateKV->Clear();	// TODO: Make sure this doesn't rekt the key itself, because we'll be adding stuff back into it later.
}

KeyValues* C_LibretroManager::FindOrCreateCoreSettings(std::string coreFile)
{
	for (KeyValues *sub = m_pCoreSettingsKV->GetFirstSubKey(); sub; sub = sub->GetNextKey())
	{
		if (std::string(sub->GetString("file")) == coreFile)
			return sub;
	}
	
	return null;
}

void C_LibretroManager::SaveCoreSettings()
{
	g_pFullFileSystem->CreateDirHierarchy("libretro\\user", "DEFAULT_WRITE_PATH");
	std::string saveFile = "libretro\\user\\coreSettings.txt";
	m_pCoreSettingsKV->SaveToFile(g_pFullFileSystem, "libretro\\user\\coreSettings.txt", "DEFAULT_WRITE_PATH");
}

void C_LibretroManager::SaveOverlaysKV(std::string type, std::string overlayId, std::string prettyCore, std::string prettyGame)
{
	KeyValues* pOverlayEntryKV = null;
	std::string testerCore;
	std::string testerGame;
	for (KeyValues *sub = m_pOverlaysKV->GetFirstSubKey(); sub; sub = sub->GetNextKey())
	{
		testerCore = sub->GetString("core");
		testerGame = sub->GetString("game");

		if (testerCore != prettyCore)
			continue;

		if ((type == "core" && testerGame == "") || (type == "game" && testerGame == prettyGame))
		{
			pOverlayEntryKV = sub;
			break;
		}
	}

	if (!pOverlayEntryKV && overlayId != "")
	{
		pOverlayEntryKV = m_pOverlaysKV->CreateNewKey();
		pOverlayEntryKV->SetName("overlay");
		pOverlayEntryKV->SetString("core", prettyCore.c_str());
		if (type == "game")
			pOverlayEntryKV->SetString("game", prettyGame.c_str());
	}

	if (overlayId != "")
		pOverlayEntryKV->SetString("overlayId", overlayId.c_str());
	else if (pOverlayEntryKV)
	{
		pOverlayEntryKV->Clear();
		pOverlayEntryKV->SetString(null, "");
	}

	g_pFullFileSystem->CreateDirHierarchy(m_userPath.c_str(), "DEFAULT_WRITE_PATH");
	m_pOverlaysKV->SaveToFile(g_pFullFileSystem, VarArgs("%s\\overlaySettings.txt", m_userPath.c_str()), "DEFAULT_WRITE_PATH");
}

std::string C_LibretroManager::DetermineOverlay(std::string prettyCore, std::string prettyGame)
{
	KeyValues* pOverlayCoreKV = null;
	KeyValues* pOverlayGameKV = null;
	std::string testerCore;
	std::string testerGame;
	for (KeyValues *sub = m_pOverlaysKV->GetFirstSubKey(); sub; sub = sub->GetNextKey())
	{
		testerCore = sub->GetString("core");
		testerGame = sub->GetString("game");
		if (testerCore == prettyCore)
		{
			if (testerGame == prettyGame)
				pOverlayGameKV = sub;
			else if (testerGame == "")
				pOverlayCoreKV = sub;
		}
	}

	std::string overlayId;

	if (pOverlayGameKV)
		overlayId = pOverlayGameKV->GetString("overlayId");
	else if (pOverlayCoreKV)
		overlayId = pOverlayCoreKV->GetString("overlayId");
	else
		overlayId = "";

	return overlayId;
}

void C_LibretroManager::SetGUIGamepadEnabled(bool bValue)
{
	m_bGUIGamepadEnabled = bValue;
	cvar->FindVar("libretro_gui_gamepad")->SetValue(bValue);
}

std::string C_LibretroManager::GetLibretroPath(retro_path_names retro_path_name)
{
	switch (retro_path_name)
	{
		case RETRO_CORE_PATH:
			return m_corePath;
		case RETRO_USER_BASE:
			return m_userBase;
		case RETRO_ASSETS_PATH:
			return m_assetsPath;
		case RETRO_SYSTEM_PATH:
			return m_systemPath;
		case RETRO_SAVE_PATH:
			return m_savePath;
		case RETRO_USER_PATH:
			return m_userPath;

		default:
			return "";
	}
}

void C_LibretroManager::DetectAllOverlaysPNGs()
{
	std::vector<std::string> files;
	std::string id;
	FileFindHandle_t pFileFindHandle;
	const char *pFile = g_pFullFileSystem->FindFirstEx("resource\\ui\\html\\overlays\\*.png", "MOD", &pFileFindHandle);
	while (pFile != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(pFileFindHandle))
		{
			pFile = g_pFullFileSystem->FindNext(pFileFindHandle);
			continue;
		}

		id = pFile;
		if (id.length() < 5)
		{
			pFile = g_pFullFileSystem->FindNext(pFileFindHandle);
			continue;
		}

		id = id.substr(0, id.length() - 4);

		if (!g_pFullFileSystem->FileExists(VarArgs("resource\\ui\\html\\overlays\\%s.cfg", id.c_str()), "MOD"))
			files.push_back(id);

		pFile = g_pFullFileSystem->FindNext(pFileFindHandle);
	}
	g_pFullFileSystem->FindClose(pFileFindHandle);

	KeyValues* pOverlayKV;
	for (unsigned int i = 0; i < files.size(); i++)
	{
		id = files[i];
		pOverlayKV = new KeyValues("overlay");
		pOverlayKV->SetFloat("settings/default/x", 0);
		pOverlayKV->SetFloat("settings/default/y", 0);
		pOverlayKV->SetFloat("settings/default/width", 1);
		pOverlayKV->SetFloat("settings/default/height", 1);
		if (!pOverlayKV->SaveToFile(g_pFullFileSystem, VarArgs("resource\\ui\\html\\overlays\\%s.cfg", id.c_str()), "DEFAULT_WRITE_PATH"))
			DevMsg("ERROR: Could not write file.\n");
		pOverlayKV->deleteThis();
		pOverlayKV = null;
	}
}

void C_LibretroManager::DetectAllOverlays(std::vector<std::string>& overlayFiles)

{
	std::string id;
	FileFindHandle_t pFileFindHandle;
	const char *pFile = g_pFullFileSystem->FindFirstEx("resource\\ui\\html\\overlays\\*.cfg", "MOD", &pFileFindHandle);
	while (pFile != NULL)
	{
		if (g_pFullFileSystem->FindIsDirectory(pFileFindHandle))
		{
			pFile = g_pFullFileSystem->FindNext(pFileFindHandle);
			continue;
		}

		id = pFile;
		if (id.length() < 5)
		{
			pFile = g_pFullFileSystem->FindNext(pFileFindHandle);
			continue;
		}

		id = id.substr(0, id.length() - 4);
		overlayFiles.push_back(id);

		pFile = g_pFullFileSystem->FindNext(pFileFindHandle);
	}
	g_pFullFileSystem->FindClose(pFileFindHandle);
}

void C_LibretroManager::GetAllInstances(std::vector<C_EmbeddedInstance*>& embeddedInstances)
{
	auto it = m_libretroInstances.begin();
	while (it != m_libretroInstances.end())
	{
		if (it->second->HasInfo() && it->second->GetInfo()->state == 5)
			embeddedInstances.push_back(it->second);

		it++;
	}
}
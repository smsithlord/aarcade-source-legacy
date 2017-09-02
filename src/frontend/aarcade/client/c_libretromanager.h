#ifndef C_LIBRETRO_MANAGER_H
#define C_LIBRETRO_MANAGER_H

#include "c_libretroinstance.h"
#include "c_inputlistenerlibretro.h"
//#include "c_libretrosurfaceregen.h"
#include "libretro.h"
#include <map>

enum retro_path_names
{
	RETRO_CORE_PATH = 0,
	RETRO_USER_BASE,
	RETRO_ASSETS_PATH,
	RETRO_SYSTEM_PATH,
	RETRO_SAVE_PATH,
	RETRO_USER_PATH,

	RETRO_LAST_PATH = INT_MAX
};

class C_LibretroManager
{
public:
	C_LibretroManager();
	~C_LibretroManager();

	void Update();
	void CloseAllInstances();
	void LevelShutdownPreEntity();

	C_LibretroInstance* CreateLibretroInstance();
	void DestroyLibretroInstance(C_LibretroInstance* pInstance);

	bool FocusLibretroInstance(C_LibretroInstance* pLibretroInstance);
	bool SelectLibretroInstance(C_LibretroInstance* pLibretroInstance);
	void OnLibretroInstanceCreated(LibretroInstanceInfo_t* pInfo);//C_LibretroInstance* pLibretroInstance);
	C_LibretroInstance* FindLibretroInstance(CSysModule* pModule);
	C_LibretroInstance* FindLibretroInstance(uint uId);
	C_LibretroInstance* FindLibretroInstance(std::string id);
	C_LibretroInstance* FindLibretroInstanceByEntityIndex(int iEntityIndex);

	void SetVolume(float fVolume);

	void RunEmbeddedLibretro(std::string core, std::string file);
	void ManageInputUpdate(LibretroInstanceInfo_t* info, unsigned int retroport, unsigned int retrodevice);
	int GetInputState(LibretroInstanceInfo_t* info, unsigned int retroport, unsigned int retrodevice, unsigned int retroindex, unsigned int retroid);
	std::string RetroDeviceToString(unsigned int number);
	//unsigned int StringToRetroKey(std::string text);
	std::string RetroKeyboardKeyToString(retro_key retrokey);
	std::string RetroKeyToString(unsigned int retrokey);

	unsigned int StringToRetroDevice(std::string);
	unsigned int StringToRetroKey(std::string text);
	retro_key StringToRetroKeyboardKey(std::string text);

	int GetInstanceCount();

	void DetectAllOverlaysPNGs();
	void DetectAllOverlays(std::vector<std::string>& overlayFiles);

	void GetAllInstances(std::vector<C_EmbeddedInstance*>& embeddedInstances);

	void SetGUIGamepadInputState(unsigned int retroport, unsigned int retrodevice, unsigned int retroindex, unsigned int retroid, int iValue);
	void ClearGUIGamepadInputState();

	KeyValues* FindOrCreateCoreSettings(std::string coreFile);
	void SaveCoreSettings();
	std::string GetLibretroPath(retro_path_names retro_path_name);

	std::string DetermineOverlay(std::string prettyCore, std::string prettyPath);
	void SaveOverlaysKV(std::string type, std::string overlayId, std::string prettyCore, std::string prettyPath);

	// accessors
	KeyValues* GetOverlaysKV() { return m_pOverlaysKV; }
	KeyValues* GetCoreSettingsKV() { return m_pCoreSettingsKV; }
	KeyValues* GetBlacklistedDLLsKV() { return m_pBlacklistedDLLsKV; }
	bool GetGUIGamepadEnabled() { return m_bGUIGamepadEnabled; }
	C_LibretroInstance* GetFocusedLibretroInstance() { return m_pFocusedLibretroInstance; }
	C_LibretroInstance* GetSelectedLibretroInstance() { return m_pSelectedLibretroInstance; }
	C_InputListenerLibretro* GetInputListener() { return m_pInputListener; }
	RunningLibretroCores_t* GetLibretroRunningCores() { return m_pRunningLibretroCores; }

	// mutators	
	void SetGUIGamepadEnabled(bool bValue);// { m_bGUIGamepadEnabled = value; }

private:
	ConVar* m_pWaitForLibretroConVar;
	RunningLibretroCores_t* m_pRunningLibretroCores;
	int m_iPreviousRunningCoreCount;
	std::string m_corePath;
	std::string m_userBase;
	std::string m_assetsPath;
	std::string m_systemPath;
	std::string m_savePath;
	std::string m_userPath;
	bool m_bSoundEnabled;
	bool m_bGUIGamepadEnabled;
	KeyValues* m_pBlacklistedDLLsKV;
	KeyValues* m_pCoreSettingsKV;
	KeyValues* m_pGUIGamepadStateKV;
	KeyValues* m_pOverlaysKV;
	C_InputListenerLibretro* m_pInputListener;
	C_LibretroInstance* m_pSelectedLibretroInstance;
	C_LibretroInstance* m_pFocusedLibretroInstance;
	std::map<CSysModule*, C_LibretroInstance*> m_libretroInstances;
	std::map<uint, CSysModule*> m_libretroInstancesModules;
	std::map<std::string, unsigned int> m_retroDeviceMap;
	std::map<std::string, unsigned int> m_retroKeyUnknowndMap;
	std::map<std::string, unsigned int> m_retroKeyJoypadMap;
	std::map<std::string, unsigned int> m_retroKeyMouseMap;
	std::map<std::string, unsigned int> m_retroKeyLightgunMap;
	std::map<std::string, unsigned int> m_retroKeyAnalogMap;
	std::map<std::string, unsigned int> m_retroKeyPointerMap;
	std::map<std::string, retro_key> m_retroKeyKeyboardMap;
};

#endif
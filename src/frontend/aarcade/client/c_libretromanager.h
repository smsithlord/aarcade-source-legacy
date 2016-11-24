#ifndef C_LIBRETRO_MANAGER_H
#define C_LIBRETRO_MANAGER_H

#include "c_libretroinstance.h"
#include "c_inputlistenerlibretro.h"
//#include "c_libretrosurfaceregen.h"
#include "libretro.h"
#include <map>

class C_LibretroManager
{
public:
	C_LibretroManager();
	~C_LibretroManager();

	void Update();
	void CloseAllInstances();

	C_LibretroInstance* CreateLibretroInstance();
	void DestroyLibretroInstance(C_LibretroInstance* pInstance);

	bool FocusLibretroInstance(C_LibretroInstance* pLibretroInstance);
	bool SelectLibretroInstance(C_LibretroInstance* pLibretroInstance);
	void OnLibretroInstanceCreated(LibretroInstanceInfo_t* pInfo);//C_LibretroInstance* pLibretroInstance);
	C_LibretroInstance* FindLibretroInstance(CSysModule* pModule);
	C_LibretroInstance* FindLibretroInstance(uint uId);
	C_LibretroInstance* FindLibretroInstance(std::string id);

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

	unsigned int GetInstanceCount();

	void GetAllInstances(std::vector<C_EmbeddedInstance*>& embeddedInstances);

	// accessors
	C_LibretroInstance* GetFocusedLibretroInstance() { return m_pFocusedLibretroInstance; }
	C_LibretroInstance* GetSelectedLibretroInstance() { return m_pSelectedLibretroInstance; }
	C_InputListenerLibretro* GetInputListener() { return m_pInputListener; }

	// mutators	

private:
	bool m_bSoundEnabled;
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
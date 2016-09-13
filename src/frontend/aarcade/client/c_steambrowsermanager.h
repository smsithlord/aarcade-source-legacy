#ifndef C_STEAMBROWSER_MANAGER_H
#define C_STEAMBROWSER_MANAGER_H

#include "c_steambrowserinstance.h"
#include "c_inputlistenersteambrowser.h"
//#include "c_libretrosurfaceregen.h"
#include <map>

class C_SteamBrowserManager
{
public:
	C_SteamBrowserManager();
	~C_SteamBrowserManager();

	void Update();

	C_SteamBrowserInstance* CreateSteamBrowserInstance();
	bool SelectSteamBrowserInstance(C_SteamBrowserInstance* pSteamBrowserInstance);
	void OnSteamBrowserInstanceCreated(C_SteamBrowserInstance* pSteamBrowserInstance);
	//C_SteamBrowserInstance* FindSteamBrowserInstance(CSysModule* pModule);
	C_SteamBrowserInstance* FindSteamBrowserInstance(std::string id);

	void RunEmbeddedSteamBrowser();
	void DestroySteamBrowserInstance(C_SteamBrowserInstance* pInstance);

	// accessors
	C_SteamBrowserInstance* GetSelectedSteamBrowserInstance() { return m_pSelectedSteamBrowserInstance; }
	C_InputListenerSteamBrowser* GetInputListener() { return m_pInputListener; }

	// mutators	

private:
	bool m_bSoundEnabled;
	C_InputListenerSteamBrowser* m_pInputListener;
	C_SteamBrowserInstance* m_pSelectedSteamBrowserInstance;
	std::map<std::string, C_SteamBrowserInstance*> m_steamBrowserInstances;
	//std::map<uint, CSysModule*> m_steamBrowserInstancesModules;

};

#endif
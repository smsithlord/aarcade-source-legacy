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

	void CloseAllInstances();

	C_SteamBrowserInstance* CreateSteamBrowserInstance();
	bool FocusSteamBrowserInstance(C_SteamBrowserInstance* pSteamBrowserInstance);
	bool SelectSteamBrowserInstance(C_SteamBrowserInstance* pSteamBrowserInstance);
	void AddFreshSteamBrowserInstance(C_SteamBrowserInstance* pSteamBrowserInstance);
	void OnSteamBrowserInstanceCreated(C_SteamBrowserInstance* pSteamBrowserInstance);
	C_SteamBrowserInstance* FindSteamBrowserInstance(unsigned int unHandle);
	C_SteamBrowserInstance* FindSteamBrowserInstance(std::string id);
	C_SteamBrowserInstance* GetPendingSteamBrowserInstance();

	void RunEmbeddedSteamBrowser();
	void DestroySteamBrowserInstance(C_SteamBrowserInstance* pInstance);

	// accessors
	C_SteamBrowserInstance* GetFocusedSteamBrowserInstance() { return m_pFocusedSteamBrowserInstance; }
	C_SteamBrowserInstance* GetSelectedSteamBrowserInstance() { return m_pSelectedSteamBrowserInstance; }
	C_InputListenerSteamBrowser* GetInputListener() { return m_pInputListener; }

	// mutators	

private:
	bool m_bSoundEnabled;
	C_InputListenerSteamBrowser* m_pInputListener;
	C_SteamBrowserInstance* m_pSelectedSteamBrowserInstance;
	C_SteamBrowserInstance* m_pFocusedSteamBrowserInstance;
	std::map<std::string, C_SteamBrowserInstance*> m_steamBrowserInstances;
	//std::map<unsigned int, std::string> m_steamBrowserInstanceIds;

};

#endif
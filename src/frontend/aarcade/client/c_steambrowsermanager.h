#ifndef C_STEAMBROWSER_MANAGER_H
#define C_STEAMBROWSER_MANAGER_H

#include "c_steambrowserinstance.h"
#include "c_steambrowserlistener.h"
#include "c_inputlistenersteambrowser.h"
//#include "c_libretrosurfaceregen.h"
//#include <map>
#include <vector>

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
	bool IsAlreadyInInstances(C_SteamBrowserInstance* pSteamBrowserInstance);
	C_SteamBrowserInstance* FindSteamBrowserInstanceByEntityIndex(int iEntityIndex);
	C_SteamBrowserInstance* FindSteamBrowserInstance(std::string id);
	C_SteamBrowserInstance* FindSteamBrowserInstance(unsigned int unHandle);
	C_SteamBrowserInstance* GetPendingSteamBrowserInstance();

	void RunEmbeddedSteamBrowser();
	void DestroySteamBrowserInstance(C_SteamBrowserInstance* pInstance);

	void GetAllInstances(std::vector<C_EmbeddedInstance*>& embeddedInstances);
	unsigned int GetInstanceCount();

	//void AddDefunctInstance(C_SteamBrowserInstance* pInstance);
	//bool DestroyDefunctInstance(C_SteamBrowserInstance* pInstance);
	//C_SteamBrowserInstance* FindDefunctInstance(unsigned int unHandle);

	// accessors
	C_SteamBrowserInstance* GetFocusedSteamBrowserInstance() { return m_pFocusedSteamBrowserInstance; }
	C_SteamBrowserInstance* GetSelectedSteamBrowserInstance() { return m_pSelectedSteamBrowserInstance; }
	C_InputListenerSteamBrowser* GetInputListener() { return m_pInputListener; }
	C_SteamBrowserListener* GetBrowserListener() { return m_pBrowserListener; }

	// mutators	

private:
	C_SteamBrowserListener* m_pBrowserListener;
	bool m_bSoundEnabled;
	C_InputListenerSteamBrowser* m_pInputListener;
	C_SteamBrowserInstance* m_pSelectedSteamBrowserInstance;
	C_SteamBrowserInstance* m_pFocusedSteamBrowserInstance;
	std::vector<C_SteamBrowserInstance*> m_steamBrowserInstances;
	//std::map<std::string, C_SteamBrowserInstance*> m_steamBrowserInstances;
	//std::map<std::string, C_SteamBrowserInstance*> m_defunctSteamBrowserInstances;
	//std::map<unsigned int, std::string> m_steamBrowserInstanceIds;

};

#endif
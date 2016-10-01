#ifndef C_ANARCHY_MANAGER_H
#define C_ANARCHY_MANAGER_H

#include <KeyValues.h>
#include "c_canvasmanager.h"
//#include "c_webmanager.h"
//#include "c_loadingmanager.h"
#include "c_libretromanager.h"
#include "c_steambrowsermanager.h"
#include "c_awesomiumbrowsermanager.h"
#include "c_inputmanager.h"
#include "c_mountmanager.h"
#include "c_workshopmanager.h"
#include "c_metaversemanager.h"
#include "c_instancemanager.h"
#include <vector>
#include "vgui/ISystem.h"
#include "vgui_controls/Controls.h"

enum aaState
{
	AASTATE_NONE = 0,
	AASTATE_INPUTMANAGER = 1,
	AASTATE_CANVASMANAGER = 2,
	AASTATE_LIBRETROMANAGER = 3,
	AASTATE_STEAMBROWSERMANAGER = 4,
	AASTATE_AWESOMIUMBROWSERMANAGER = 5,
	AASTATE_AWESOMIUMBROWSERMANAGERWAIT = 6,
	AASTATE_AWESOMIUMBROWSERMANAGERHUD = 7,
	AASTATE_AWESOMIUMBROWSERMANAGERHUDWAIT = 8,
	AASTATE_AWESOMIUMBROWSERMANAGERHUDINIT = 9,
	AASTATE_AWESOMIUMBROWSERMANAGERIMAGES = 10,
	AASTATE_AWESOMIUMBROWSERMANAGERIMAGESWAIT = 11,
	AASTATE_AWESOMIUMBROWSERMANAGERIMAGESINIT = 12,
	AASTATE_LIBRARYMANAGER = 13,
	AASTATE_MOUNTMANAGER = 14,
	AASTATE_WORKSHOPMANAGER = 15,
	AASTATE_INSTANCEMANAGER = 16,
	AASTATE_RUN = 17
};

class C_AnarchyManager : public CAutoGameSystemPerFrame
{
public:
	C_AnarchyManager();
	~C_AnarchyManager();

	virtual bool Init();
	virtual void PostInit();
	virtual void Shutdown();

	// Level init, shutdown
	virtual void LevelInitPreEntity();
	virtual void LevelInitPostEntity();
	virtual void LevelShutdownPreClearSteamAPIContext();
	virtual void LevelShutdownPreEntity();
	virtual void LevelShutdownPostEntity();

	virtual void OnSave();
	virtual void OnRestore();
	virtual void SafeRemoveIfDesired();

	virtual bool IsPerFrame();

	// Called before rendering
	virtual void PreRender();

	// Gets called each frame
	virtual void Update(float frametime);

	// Called after rendering
	virtual void PostRender();

	bool HandleUiToggle();
	void Pause();
	void Unpause();
	bool IsPaused() { return m_bPaused; }
	
	//void Run();
	void RunAArcade();	// initializes AArcade's loading of libraries and stuff.

	// TRY TO KEEP THESE IN CHRONOLOGICAL ORDER, AT LEAST FOR THE STARTUP SEQUENCE!
	void AnarchyStartup();
	void OnWebManagerReady();
	void OnLoadAllLocalAppsComplete();
	void OnWorkshopManagerReady();
	void OnMountAllWorkshopsComplete();
	void OnDetectAllMapsComplete();

	//void OnLoadingManagerReady();
	bool AttemptSelectEntity();
	bool SelectEntity(C_BaseEntity* pEntity);
	bool DeselectEntity(C_BaseEntity* pEntity, std::string nextUrl = "", bool bCloseInstance = true);
	void AddGlowEffect(C_BaseEntity* pEntity);
	void RemoveGlowEffect(C_BaseEntity* pEntity);

	void ShowFileBrowseMenu();// const char* keyFieldName, KeyValues* itemKV);
	//void ReleaseFileBrowseParams();

	void ScanForLegacySaveRecursive(std::string path);

	// helpers
	std::string GenerateUniqueId();
	std::string ExtractLegacyId(std::string itemFile, KeyValues* item = null);
	const char* GenerateLegacyHash(const char* text);
	void Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters);
	std::string C_AnarchyManager::encodeURIComponent(const std::string &s);

	void SetNextInstanceId(std::string instanceId) { m_nextInstanceId = instanceId; }

	void TestSQLite();

	// accessors
	bool IsInitialized() { return m_bInitialized; }
	aaState GetState() { return m_state; }
	std::string GetInstanceId() { return m_instanceId; }
	std::string GetNextInstanceId() { return m_nextInstanceId; }
	C_InputManager* GetInputManager() { return m_pInputManager; }
	//C_WebManager* GetWebManager() { return m_pWebManager; }
	//C_LoadingManager* GetLoadingManager() { return m_pLoadingManager; }
	C_CanvasManager* GetCanvasManager() { return m_pCanvasManager; }
	C_SteamBrowserManager* GetSteamBrowserManager() { return m_pSteamBrowserManager; }
	C_AwesomiumBrowserManager* GetAwesomiumBrowserManager() { return m_pAwesomiumBrowserManager; }
	//C_AwesomiumBrowserManager* GetAwesomiumBrowserManager() { return m_pAwesomiumBrowserManager; }
	C_LibretroManager* GetLibretroManager() { return m_pLibretroManager; }
	C_MountManager* GetMountManager() { return m_pMountManager; }
	C_WorkshopManager* GetWorkshopManager() { return m_pWorkshopManager; }
	C_MetaverseManager* GetMetaverseManager() { return m_pMetaverseManager; }
	C_InstanceManager* GetInstanceManager() { return m_pInstanceManager; }
	C_BaseEntity* GetSelectedEntity() { return m_pSelectedEntity; }
	//ThreadedFileBrowseParams_t* GetFileBrowseParams() { return m_pFileParams; }

	// mutators
	void SetInitialized(bool bValue) { m_bInitialized = bValue; }
	void SetState(aaState state) { m_state = state; }
	void IncrementState();
	
private:
	//ThreadedFileBrowseParams_t* m_pFileParams;
	//ThreadedFolderBrowseParams_t* m_pFolderParams;

	bool m_bInitialized;
	bool m_bIncrementState;
	aaState m_state;
	bool m_bPaused;
	std::string m_instanceId;
	std::string m_nextInstanceId;
	int m_iState;
	double m_dLastGenerateIdTime;
	std::string m_lastGeneratedChars;

	C_CanvasManager* m_pCanvasManager;
	//C_WebManager* m_pWebManager;
	//C_LoadingManager* m_pLoadingManager;
	C_LibretroManager* m_pLibretroManager;
	C_SteamBrowserManager* m_pSteamBrowserManager;
	C_AwesomiumBrowserManager* m_pAwesomiumBrowserManager;
	C_InputManager* m_pInputManager;
	C_MountManager* m_pMountManager;
	C_WorkshopManager* m_pWorkshopManager;
	C_MetaverseManager* m_pMetaverseManager;
	C_InstanceManager* m_pInstanceManager;
	C_BaseEntity* m_pSelectedEntity;
};

extern C_AnarchyManager* g_pAnarchyManager;

#endif
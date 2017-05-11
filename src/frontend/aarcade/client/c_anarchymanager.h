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
#include "c_windowmanager.h"
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

enum launchErrorType_t {
	NONE = 0,
	UNKNOWN_ERROR = 1,
	ITEM_NOT_FOUND = 2,
	ITEM_FILE_NOT_FOUND = 3,
	ITEM_FILE_PATH_RESTRICTED = 4,
	APP_NOT_FOUND = 5,
	APP_FILE_NOT_FOUND = 6,
	APP_PATH_NOT_FOUND = 7
};

struct nextLoadInfo_t {
	std::string instanceId;
	std::string position;
	std::string rotation;
};

class C_AnarchyManager : public CAutoGameSystemPerFrame
{
public:
	C_AnarchyManager();
	~C_AnarchyManager();

	virtual bool Init();
	virtual void PostInit();
	virtual void Shutdown();
	bool IsShuttingDown() { return m_bIsShuttingDown; }

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

	//void CalculateDynamicMultiplyer();
	void CheckPicMip();
	int GetDynamicMultiplyer();
	//void SetDynamicMultiplyer(int val) { m_iDynamicMultiplyer = val; }

	void SpecialReady(C_AwesomiumBrowserInstance* pInstance);

	bool HandleUiToggle();
	bool HandleCycleToNextWeapon();
	bool HandleCycleToPrevWeapon();
	void Pause();
	void Unpause();
	bool IsPaused() { return m_bPaused; }

	C_SteamBrowserInstance* AutoInspect(KeyValues* pItemKV, std::string tabId = "", C_PropShortcutEntity* pShortcut = null);

	launchErrorType_t LaunchItem(std::string id);
	bool AlphabetSafe(std::string text, std::string in_alphabet = "");
	bool PrefixSafe(std::string text);
	bool DirectorySafe(std::string text);
	bool ExecutableSafe(std::string text);

	void AddSubKeysToKeys(KeyValues* kv, KeyValues* targetKV);	// TODO: Make the sibling to this weirdly named function a method of the anarchy manager too.

	bool WeaponsEnabled();

	uint64 GetTimeNumber();
	std::string GetTimeString();

	void BeginImportSteamGames();

	void ArcadeCreateProcess(std::string executable, std::string executableDirectory, std::string masterCommands);
	
	//void Run();
	void RunAArcade();	// initializes AArcade's loading of libraries and stuff.

	void HudStateNotify();
	void SetSlaveScreen(bool bVal);

	bool CompareLoadedFromKeyValuesFileId(const char* testId, const char* baseId);

	void Feedback(std::string type);

	void TaskClear();
	void TaskRemember();
	void ShowTaskMenu();
	void HideTaskMenu();
	void ObsoleteLegacyCommandReceived();

	void StartHoldingPrimaryFire();
	void StopHoldingPrimaryFire();

	object_t* GetLastNearestObjectToPlayerLook() { return m_pLastNearestObjectToPlayerLook; }

	void ShowScreenshotMenu();
	void HideScreenshotMenu();
	bool TakeScreenshot(bool bCreateBig = false, std::string id = "");
	void TeleportToScreenshot(std::string id, bool bDeactivateInputMode = true);

	// TRY TO KEEP THESE IN CHRONOLOGICAL ORDER, AT LEAST FOR THE STARTUP SEQUENCE!
	void Disconnect();
	void AnarchyStartup();
	void OnWebManagerReady();
	void OnLoadAllLocalAppsComplete();
	void OnWorkshopManagerReady();
	void OnMountAllWorkshopsComplete();
	void OnDetectAllMapsComplete();

	bool OnSteamBrowserCallback(unsigned int unHandle);

	//void OnLoadingManagerReady();
	bool AttemptSelectEntity(C_BaseEntity* pTargetEntity = null);
	bool SelectEntity(C_BaseEntity* pEntity);
	bool DeselectEntity(std::string nextUrl = "", bool bCloseInstance = true);
	void AddGlowEffect(C_BaseEntity* pEntity);
	void RemoveGlowEffect(C_BaseEntity* pEntity);

	void AddHoverGlowEffect(C_BaseEntity* pEntity);
	void RemoveLastHoverGlowEffect();

	void ShowFileBrowseMenu(std::string browseId);// const char* keyFieldName, KeyValues* itemKV);
	void OnBrowseFileSelected(std::string browseId, std::string response);
	//void ReleaseFileBrowseParams();

	void ScanForLegacySaveRecursive(std::string path);

	void ActivateObjectPlacementMode(C_PropShortcutEntity* pShortcut, const char* mode = "spawn");
	void DeactivateObjectPlacementMode(bool confirm);

	void ShowEngineOptionsMenu();

	// helpers
	void GenerateUniqueId(char* result);
	const char* GenerateUniqueId();
	const char* GenerateUniqueId2();
	std::string ExtractLegacyId(std::string itemFile, KeyValues* item = null);
	const char* GenerateLegacyHash(const char* text);
	const char* GenerateCRC32Hash(const char* text);
	void Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters);
	std::string C_AnarchyManager::encodeURIComponent(const std::string &s);

	//void SetNextInstanceId(std::string instanceId) { m_nextInstanceId = instanceId; }

	void xCastSetLiveURL();
	void TestSQLite();
	void TestSQLite2();

	void SetNextLoadInfo(std::string instanceId = "", std::string position = "", std::string rotation = "");

	// accessors
	bool GetSuspendEmbedded() { return m_bSuspendEmbedded; }
	bool IsInitialized() { return m_bInitialized; }
	aaState GetState() { return m_state; }
	std::string GetInstanceId() { return m_instanceId; }
	//std::string GetNextInstanceId() { return m_nextInstanceId; }
	nextLoadInfo_t* GetNextLoadInfo() { return m_pNextLoadInfo; }
	C_InputManager* GetInputManager() { return m_pInputManager; }
	//C_WebManager* GetWebManager() { return m_pWebManager; }
	//C_LoadingManager* GetLoadingManager() { return m_pLoadingManager; }
	C_CanvasManager* GetCanvasManager() { return m_pCanvasManager; }
	C_SteamBrowserManager* GetSteamBrowserManager() { return m_pSteamBrowserManager; }
	C_WindowManager* GetWindowManager() { return m_pWindowManager; }
	C_AwesomiumBrowserManager* GetAwesomiumBrowserManager() { return m_pAwesomiumBrowserManager; }
	//C_AwesomiumBrowserManager* GetAwesomiumBrowserManager() { return m_pAwesomiumBrowserManager; }
	C_LibretroManager* GetLibretroManager() { return m_pLibretroManager; }
	C_MountManager* GetMountManager() { return m_pMountManager; }
	C_WorkshopManager* GetWorkshopManager() { return m_pWorkshopManager; }
	C_MetaverseManager* GetMetaverseManager() { return m_pMetaverseManager; }
	C_InstanceManager* GetInstanceManager() { return m_pInstanceManager; }
	C_BaseEntity* GetSelectedEntity() { return m_pSelectedEntity; }
	//ThreadedFileBrowseParams_t* GetFileBrowseParams() { return m_pFileParams; }
	std::string GetLegacyFolder() { return m_legacyFolder; }
	std::string GetWorkshopFolder() { return m_workshopFolder; }
	std::string GetAArcadeUserFolder() { return m_aarcadeUserFolder; }
	std::string GetOldEngineNoFocusSleep() { return m_oldEngineNoFocusSleep; }
	C_BaseEntity* GetLastHoverGlowEntity() { return m_pHoverGlowEntity; }
	std::string GetTabMenuFile() { return m_tabMenuFile; }

	// mutators
	void SetLastNearestObjectToPlayerLook(object_t* pObject) { m_pLastNearestObjectToPlayerLook = pObject; }
	void SetInitialized(bool bValue) { m_bInitialized = bValue; }
	void SetState(aaState state) { m_state = state; }
	void IncrementState();
	void SetLegacyFolder(std::string val) { m_legacyFolder = val; }
	void SetWorkshopFolder(std::string val) { m_workshopFolder = val; }
	void SetAArcadeUserFolder(std::string val) { m_aarcadeUserFolder = val; }
	void SetOldEngineNoFocusSleep(std::string val) { m_oldEngineNoFocusSleep = val; }
	void SetTabMenuFile(std::string url) { m_tabMenuFile = url; }
	
private:
	ConVar* m_pWeaponsEnabledConVar;
	std::string m_tabMenuFile;
	int m_iLastDynamicMultiplyer;
	ConVar* m_pPicMipConVar;
	object_t* m_pLastNearestObjectToPlayerLook;
	C_BaseEntity* m_pHoverGlowEntity;
	bool m_bIsShuttingDown;
	bool m_bIsHoldingPrimaryFire;
	std::map<std::string, bool> m_specialInstances;
	//ThreadedFileBrowseParams_t* m_pFileParams;
	//ThreadedFolderBrowseParams_t* m_pFolderParams;

	nextLoadInfo_t* m_pNextLoadInfo;

	bool m_bSuspendEmbedded;
	bool m_bInitialized;
	bool m_bIncrementState;
	aaState m_state;
	bool m_bPaused;
	std::string m_instanceId;
	//std::string m_nextInstanceId;
	int m_iState;
	double m_dLastGenerateIdTime;
	std::string m_lastGeneratedChars;

	C_CanvasManager* m_pCanvasManager;
	//C_WebManager* m_pWebManager;
	//C_LoadingManager* m_pLoadingManager;
	C_LibretroManager* m_pLibretroManager;
	C_SteamBrowserManager* m_pSteamBrowserManager;
	C_WindowManager* m_pWindowManager;
	C_AwesomiumBrowserManager* m_pAwesomiumBrowserManager;
	C_InputManager* m_pInputManager;
	C_MountManager* m_pMountManager;
	C_WorkshopManager* m_pWorkshopManager;
	C_MetaverseManager* m_pMetaverseManager;
	C_InstanceManager* m_pInstanceManager;
	C_BaseEntity* m_pSelectedEntity;

	std::string m_aarcadeUserFolder;
	std::string m_legacyFolder;
	std::string m_workshopFolder;
	std::string m_oldEngineNoFocusSleep;
};

extern C_AnarchyManager* g_pAnarchyManager;

#endif
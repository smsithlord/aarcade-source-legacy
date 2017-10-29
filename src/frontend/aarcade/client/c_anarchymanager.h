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
#include "c_backpackmanager.h"
#include "c_instancemanager.h"
#include "c_windowmanager.h"
#include <vector>
#include "vgui/ISystem.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Controls.h"
#include "hlvr/proxydll.h"

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
	AASTATE_AWESOMIUMBROWSERMANAGERNETWORK = 10,
	AASTATE_AWESOMIUMBROWSERMANAGERNETWORKWAIT = 11,
	AASTATE_AWESOMIUMBROWSERMANAGERNETWORKINIT = 12,
	AASTATE_AWESOMIUMBROWSERMANAGERIMAGES = 13,
	AASTATE_AWESOMIUMBROWSERMANAGERIMAGESWAIT = 14,
	AASTATE_AWESOMIUMBROWSERMANAGERIMAGESINIT = 15,
	AASTATE_LIBRARYMANAGER = 16,
	AASTATE_MOUNTMANAGER = 17,
	AASTATE_WORKSHOPMANAGER = 18,
	AASTATE_INSTANCEMANAGER = 19,
	AASTATE_RUN = 20
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

enum panoshotState {
	PANO_NONE,
	PANO_ORIENT_SHOT_0,
	PANO_TAKE_SHOT_0,
	PANO_ORIENT_SHOT_1,
	PANO_TAKE_SHOT_1,
	PANO_ORIENT_SHOT_2,
	PANO_TAKE_SHOT_2,
	PANO_ORIENT_SHOT_3,
	PANO_TAKE_SHOT_3,
	PANO_ORIENT_SHOT_4,
	PANO_TAKE_SHOT_4,
	PANO_ORIENT_SHOT_5,
	PANO_TAKE_SHOT_5,
	PANO_ORIENT_SHOT_6,
	PANO_TAKE_SHOT_6,
	PANO_COMPLETE
};

struct panoStuff_t {
	std::string weapons;
	std::string hud;
	std::string titles;
	std::string toast;
	std::string developer;
};

struct nextLoadInfo_t {
	std::string instanceId;
	std::string position;
	std::string rotation;
};

struct aampConnection_t {
	bool connected;
	std::string address;
	std::string universe;
	std::string instance;
	std::string user;
	std::string session;
	std::string lobby;
	std::string lobbyPassword;
	bool isPublic;
};

class C_AnarchyManager : public CAutoGameSystemPerFrame
{
public:
	C_AnarchyManager();
	~C_AnarchyManager();

	virtual bool Init();
	virtual void PostInit();
	void OnStartup();
	void OnStartupCallback(bool bForceDefaultAdd = false);
	void OnAddNextDefaultLibraryCallback();
	void OnDefaultLibraryReadyCallback();
	void OnDefaultLibraryReady();
	void OnUpdateLibraryVersionCallback();
	void OnReadyToLoadUserLibrary();
	void OnRebuildSoundCacheCallback();
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
	bool UseBuildGhosts();

	bool CheckVideoCardAbilities();

	void ThrowEarlyError(const char* msg);

	void DetectAllModelsThreaded();
	void ManageImportScans();
	void ProcessAllModels();
	void ProcessNextModel();
	void AddNextModel();

	void ManageWindow();

	bool CheckStartWithWindows();
	bool SetStartWithWindows(bool bValue);
	void InitHoverLabel(vgui::Label* pHoverLabel) { m_pHoverLabel = pHoverLabel; }
	void ManageHoverLabel();
	void UpdateHoverLabel();
	std::string GetHoverTitle() { return m_hoverTitle; }
	//void SetHoverLabel(int iEntityIndex, std::string title);

	std::string ExtractRelativeAssetPath(std::string fullPath);

	void PopToast();
	void AddToastMessage(std::string text);
	void SetNextToastExpiration(float fValue) { m_fNextToastExpiration = fValue; }
	float GetNextToastExpiration() { return m_fNextToastExpiration; }
	void AddToastLabel(vgui::Label* pLabel);
	void RemoveToastLabel(vgui::Label* pLabel);
	void SetToastText(std::string text);
	void UpdateToastText();
	std::string GetToastText() { return m_toastText; }

	//void CalculateDynamicMultiplyer();
	void CheckPicMip();
	int GetDynamicMultiplyer();
	//void SetDynamicMultiplyer(int val) { m_iDynamicMultiplyer = val; }

	void SpecialReady(C_AwesomiumBrowserInstance* pInstance);

	bool HandleUiToggle();
	bool HandleCycleToNextWeapon();
	bool HandleCycleToPrevWeapon();
	void DoPause();
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
	bool LoadMapCommand(std::string mapId, std::string instanceId, std::string position, std::string rotation, std::string screenshotId);

	uint64 GetTimeNumber();
	std::string GetTimeString();

	void BeginImportSteamGames();
	void Acquire(std::string query);

	void ArcadeCreateProcess(std::string executable, std::string executableDirectory, std::string masterCommands);
	
	//void Run();
	void RunAArcade();	// initializes AArcade's loading of libraries and stuff.

	void HudStateNotify();
	void SetSlaveScreen(std::string objectId, bool bVal);

	bool CompareLoadedFromKeyValuesFileId(const char* testId, const char* baseId);

	void Feedback(std::string type);

	void HardPause();
	void WakeUp();
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

	void ScanForLegacySaveRecursive(std::string path, std::string searchPath, std::string workshopIds, std::string mountIds, C_Backpack* pBackpack);

	void ActivateObjectPlacementMode(C_PropShortcutEntity* pShortcut, const char* mode = "spawn");
	void DeactivateObjectPlacementMode(bool confirm);

	void ShowHubSaveMenuClient(C_PropShortcutEntity* pInfoShortcut);
	void ShowNodeManagerMenu();
	void ShowEngineOptionsMenu();

	std::string GetSteamGamesCode(std::string requestId);

	void Panoshot();
	void ManagePanoshot();

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

	bool DetermineLibretroCompatible(KeyValues* pItemKV, std::string& gameFile, std::string& coreFile);
	bool DetermineStreamCompatible(KeyValues* pItemKV);
	bool DeterminePreviewCompatible(KeyValues* pItemKV);

	void WriteBroadcastGame(std::string gameTitle);
	//void xCastSetGameName();
	void xCastSetLiveURL();
	void TestSQLite();
	void TestSQLite2();

	void SetNextLoadInfo(std::string instanceId = "", std::string position = "", std::string rotation = "");
	std::string GetHomeURL();
	void ClearConnectedUniverse();

	void WaitForOverviewExtract();
	void ManageExtractOverview();

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
	C_BackpackManager* GetBackpackManager() { return m_pBackpackManager; }
	C_InstanceManager* GetInstanceManager() { return m_pInstanceManager; }
	C_BaseEntity* GetSelectedEntity() { return m_pSelectedEntity; }
	//ThreadedFileBrowseParams_t* GetFileBrowseParams() { return m_pFileParams; }
	std::string GetLegacyFolder() { return m_legacyFolder; }
	std::string GetWorkshopFolder() { return m_workshopFolder; }
	std::string GetAArcadeUserFolder() { return m_aarcadeUserFolder; }
	std::string GetAArcadeToolsFolder() { return m_aarcadeToolsFolder; }
	std::string GetOldEngineNoFocusSleep() { return m_oldEngineNoFocusSleep; }
	C_BaseEntity* GetLastHoverGlowEntity() { return m_pHoverGlowEntity; }
	std::string GetTabMenuFile() { return m_tabMenuFile; }
	//bool GetIgnoreNextFire() { return m_bIgnoreNextFire; }
	aampConnection_t* GetConnectedUniverse() { return m_pConnectedUniverse; }
	panoshotState GetPanoshotState() { return m_panoshotState; }

	// mutators
	void SetLastNearestObjectToPlayerLook(object_t* pObject) { m_pLastNearestObjectToPlayerLook = pObject; }
	void SetInitialized(bool bValue) { m_bInitialized = bValue; }
	void SetState(aaState state) { m_state = state; }
	void IncrementState();
	void SetLegacyFolder(std::string val) { m_legacyFolder = val; }
	void SetWorkshopFolder(std::string val) { m_workshopFolder = val; }
	void SetAArcadeUserFolder(std::string val) { m_aarcadeUserFolder = val; }
	void SetAArcadeToolsFolder(std::string val) { m_aarcadeToolsFolder = val; }
	void SetOldEngineNoFocusSleep(std::string val) { m_oldEngineNoFocusSleep = val; }
	void SetTabMenuFile(std::string url) { m_tabMenuFile = url; }
	//void SetIgnoreNextFire(bool bValue) { m_bIgnoreNextFire = bValue; }
	void SetConnectedUniverse(bool bConnected, std::string address, std::string universeId, std::string instanceId, std::string sessionId, std::string lobbyId, bool bPublic, std::string lobbyPassword);
	
protected:
	void ScanForLegacySave(std::string path, std::string searchPath, std::string workshopIds, std::string mountIds, C_Backpack* pBackpack);

private:
	panoStuff_t* m_pPanoStuff;
	float m_fNextExtractOverviewCompleteManage;
	float m_fNextPanoCompleteManage;
	std::vector<std::string> m_existingMapScreenshotsForPano;
	panoshotState m_panoshotState;
	float m_fNextWindowManage;
	bool m_bAutoRes;
	ConVar* m_pBuildGhostConVar;
	aampConnection_t* m_pConnectedUniverse;
	unsigned int m_uProcessBatchSize;
	unsigned int m_uProcessCurrentCycle;
	unsigned int m_uValidProcessedModelCount;
	unsigned int m_uLastProcessedModelIndex;
	unsigned int m_uPreviousImportCount;
	importInfo_t* m_pImportInfo;
	ConVar* m_pHoverTitlesConVar;
	ConVar* m_pToastMsgsConVar;
	std::string m_hoverTitle;
	int m_iHoverEntityIndex;
	float m_fHoverTitleExpiration;
	vgui::Label* m_pHoverLabel;
	float m_fNextToastExpiration;
	KeyValues* m_pToastMessagesKV;
	std::vector<vgui::Label*> m_toastLabels;
	std::string m_toastText;
	//bool m_bIgnoreNextFire;
	bool m_bIsDisconnecting;
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
	C_BackpackManager* m_pBackpackManager;
	C_InstanceManager* m_pInstanceManager;
	C_BaseEntity* m_pSelectedEntity;

	std::string m_aarcadeUserFolder;
	std::string m_aarcadeToolsFolder;
	std::string m_legacyFolder;
	std::string m_workshopFolder;
	std::string m_oldEngineNoFocusSleep;
};

extern C_AnarchyManager* g_pAnarchyManager;

#endif
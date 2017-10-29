#ifndef C_METAVERSE_MANAGER_H
#define C_METAVERSE_MANAGER_H

#include "c_webtab.h"
#include "filesystem.h"
#include "c_instancemanager.h"
#include "c_anarchymanager.h"
//#include "c_prop_shortcut.h"
#include "../../sqlite/include/sqlite/sqlite3.h"

//#include <string>
//#include <vector>

struct addDefaultLibraryContext_t {
	sqlite3** pDb;
	FileFindHandle_t handle;
	const char *pFilename;
	KeyValues* kv;
	unsigned int state;
	unsigned int numApps;
	unsigned int numCabinets;
	unsigned int numMaps;
	unsigned int numModels;
	unsigned int numTypes;
};

struct user_t {
	std::string userId;
	std::string sessionId;

	std::string followingId;	// todo: implement this on the Firebase-side stuff.
	// todo: implement some generic fields that can be primarily used for Twitch channel ID, Twitch user Id, Steam ID, maybe Twitter ID, etc.
	//std::string instanceId;
	std::string displayName;
	std::string itemId;
	std::string objectId;
	std::string say;
	Vector bodyOrigin;//std::string bodyOrigin;
	QAngle bodyAngles;//std::string bodyAngles;
	Vector headOrigin;//std::string headOrigin;
	QAngle headAngles;//std::string headAngles;
	std::string mouseX;
	std::string mouseY;
	std::string webUrl;
	std::string avatarUrl;
	C_DynamicProp* entity;
	bool needsEntity;
};

struct user_update_t {
	int updateMask;
	std::string userId;
	std::string sessionId;
	std::string displayName;
	std::string itemId;
	std::string objectId;
	std::string say;
	std::string bodyOrigin;
	std::string bodyAngles;
	std::string headOrigin;
	std::string headAngles;
	std::string mouseX;
	std::string mouseY;
	std::string webUrl;
	std::string avatarUrl;
};


class C_MetaverseManager
{
public:
	C_MetaverseManager();
	~C_MetaverseManager();

	addDefaultLibraryContext_t* GetAddDefaultLibraryContext();
	void SetAddDefaultLibraryToDbIterativeContext(addDefaultLibraryContext_t* pContext);
	bool DeleteAddDefaultLibraryContext(addDefaultLibraryContext_t* pContext);
	void AddDefaultLibraryToDb(unsigned int& numApps, unsigned int& numCabinets, unsigned int& numModels, unsigned int& numTypes, sqlite3** pDb = null);
	void AddDefaultLibraryToDbIterative(addDefaultLibraryContext_t* pContext);
	
	void AddDefaultTables(sqlite3** pDb = null);
	bool CreateDb(std::string libraryFile, sqlite3** pDb);
	bool IsEmptyDb(sqlite3** pDb = null);
	void Init();
	
	int ExtractLibraryVersion(sqlite3** pDb = null);

	bool ConvertLibraryVersion(unsigned int uOld, unsigned int uTarget);
	bool CreateLibraryBackup();
	unsigned int GetLibraryDbSize();

	void Update();
	void PerformLocalPlayerUpdate();

	//void ImportSteamGame(std::string name, std::string appid);
	void ImportSteamGames(KeyValues* kv);
	void ImportNextSteamGame();
	void StartImportSteamGames();

	void ModelFileChanged(std::string id);
	void AddModel(KeyValues* pModel);
	void AddItem(KeyValues* pItem);
	void AddType(KeyValues* pType);
	void SaveItem(KeyValues* pItem, sqlite3* pDb = null);
	void SaveApp(KeyValues* pApp, sqlite3* pDb = null);
	void SaveModel(KeyValues* pItem, sqlite3* pDb = null);
	void SaveType(KeyValues* pType, sqlite3* pDb = null);
	//void SaveMap(KeyValues* pMap, sqlite3* pDb = null);
	void SaveSQL(sqlite3** pDb, const char* tableName, const char* id, KeyValues* kv);
	void DeleteSQL(sqlite3** pDb, const char* tableName, const char* id);
	//KeyValues* CreateItem(KeyValues* pInfo);
	bool CreateItem(int iLegacy, std::string itemId, KeyValues* pItemKV, std::string title, std::string description, std::string file, std::string type, std::string app, std::string reference, std::string preview, std::string download, std::string stream, std::string screen, std::string marquee, std::string model);
	bool DeleteApp(std::string appId);

	void SmartMergItemKVs(KeyValues* pItemA, KeyValues* pItemB, bool bPreferFirst = true);// , bool bFullMerg, bool bRefreshArtworkIfNeeded = true);

	bool LoadSQLKevValues(const char* tableName, const char* id, KeyValues* kv, sqlite3* pDb = null);

	void UpdateScrapersJS();

	std::vector<std::string>* GetDefaultFields();

	void SaveInstanceTitle(instance_t* pInstance);
	void DeleteInstance(instance_t* pInstance);

	// local legacy
	KeyValues* LoadLocalItemLegacy(bool& bIsModel, bool& bWasAlreadyLoaded, std::string file, std::string filePath = "", std::string workshopIds = "", std::string mountIds = "", C_Backpack* pBackpack = null, std::string searchPath = "", bool bShouldAddToActiveLibrary = true);
	//unsigned int LoadAllLocalItemsLegacy(unsigned int& uNumModels, std::string filePath = "", std::string workshopIds = "", std::string mountIds = "");	// probably obsolete!!!

	void LoadFirstLocalItemLegacy(bool bFastMode = true, std::string filePath = "", std::string workshopIds = "", std::string mountIds = "", C_Backpack* pBackpack = null);
	void LoadNextLocalItemLegacy();
	void LoadLocalItemLegacyClose();
	void ResolveLoadLocalItemLegacyBuffer();

	void AddApp(KeyValues* pAppKV);

	// local
	std::string ResolveLegacyType(std::string legacyType);
	KeyValues* LoadLocalType(std::string file, std::string filePath = "");

	unsigned int LoadAllLocalTypes(sqlite3* pDb = null, std::map<std::string, KeyValues*>* pResponseMap = null);
	unsigned int LoadAllLocalApps(sqlite3* pDb = null, std::map<std::string, KeyValues*>* pResponseMap = null);
	unsigned int LoadAllLocalModels(unsigned int& numDynamic, sqlite3* pDb = null, std::map<std::string, KeyValues*>* pResponseMap = null);
	unsigned int LoadAllLocalItems(sqlite3* pDb = null, std::map<std::string, KeyValues*>* pResponseMap = null);
	unsigned int LoadAllLocalInstances(sqlite3* pDb = null, std::map<std::string, KeyValues*>* pResponseMap = null);

	std::string ResolveLegacyModel(std::string legacyModel);
	KeyValues* LoadLocalModel(std::string file, std::string filePath = "");

	KeyValues* LoadLocalItem(std::string file, std::string filePath = "");

	std::string ResolveLegacyApp(std::string legacyApp);
	KeyValues* LoadLocalApp(std::string file, std::string filePath = "", std::string searchPath = "");
	KeyValues* LoadFirstLocalApp(std::string filePath = "");
	KeyValues* LoadNextLocalApp();
	void LoadLocalAppClose();

	std::string GetFirstLibraryEntry(KeyValues*& response, const char* category);//const char* 
	KeyValues* GetNextLibraryEntry(const char* queryId, const char* category);

	KeyValues* GetFirstLibraryItem();	// LEGACY OBSOLETE
	KeyValues* GetNextLibraryItem();
	KeyValues* GetLibraryItem(std::string id);

	KeyValues* GetFirstLibraryModel();
	KeyValues* GetNextLibraryModel();
	KeyValues* FindFirstLibraryModel(KeyValues* pSearchInfo);
	KeyValues* FindNextLibraryModel();
	KeyValues* FindLibraryModel(KeyValues* pSearchInfo, std::map<std::string, KeyValues*>::iterator& it);
	KeyValues* FindLibraryModel(KeyValues* pSearchInfo);

	KeyValues* GetLibraryModel(std::string id);
	//KeyValues* FindLibraryModel(KeyValues* pSearchInfo, bool bExactOnly);

	KeyValues* GetFirstLibraryApp();
	KeyValues* GetNextLibraryApp();
	KeyValues* FindFirstLibraryApp(KeyValues* pSearchInfo);
	KeyValues* FindNextLibraryApp();
	KeyValues* FindLibraryApp(KeyValues* pSearchInfo, std::map<std::string, KeyValues*>::iterator& it);
	KeyValues* FindLibraryApp(KeyValues* pSearchInfo);
	KeyValues* FindLibraryType(KeyValues* pSearchInfo, std::map<std::string, KeyValues*>::iterator& it);
	KeyValues* FindLibraryType(KeyValues* pSearchInfo);


	std::string FindFirstLibraryEntry(KeyValues*& response, const char* category, KeyValues* pSearchInfo);
	KeyValues* FindNextLibraryEntry(const char* queryId, const char* category);	// TODO: get rid of category and store that info in the search context (just like pSearchInfo is.)
	
	KeyValues* FindLibraryEntry(const char* category, KeyValues* pSearchInfo, std::map<std::string, KeyValues*>::iterator& it);

	KeyValues* FindFirstLibraryItem(KeyValues* pSearchInfo);
	KeyValues* FindNextLibraryItem();
	KeyValues* FindLibraryItem(KeyValues* pSearchInfo, std::map<std::string, KeyValues*>::iterator& it);
	KeyValues* FindLibraryItem(KeyValues* pSearchInfo);

	KeyValues* GetFirstLibraryType();
	KeyValues* GetNextLibraryType();
	KeyValues* GetLibraryType(std::string id);
	std::string GetSpecialTypeId(std::string typeTitle);
	std::string GetSpecialModelId(std::string modelType);

//	KeyValues* GetFirstLibraryApp();
//	KeyValues* GetNextLibraryApp();
	KeyValues* GetLibraryApp(std::string id);
	KeyValues* GetLibraryMap(std::string id);

	KeyValues* GetScreenshot(std::string id);
	void AddScreenshot(KeyValues* pScreenshotKV);
	void DeleteScreenshot(std::string id);
	KeyValues* FindMostRecentScreenshot(std::string mapId = "", instance_t* pInstance = null);

	std::map<std::string, KeyValues*>& DetectAllMapScreenshots();
	void GetAllMapScreenshots(std::vector<KeyValues*>& responseVector, std::string mapId = "");// { return m_mapScreenshots; }
	//std::map<std::string, std::string>& GetAllMapScreenshots() { return m_mapScreenshots; }


	KeyValues* FindMap(const char* mapFile);
	KeyValues* GetMap(std::string mapId);
	std::map<std::string, KeyValues*>& GetAllMaps() { return m_maps; }
	std::map<std::string, KeyValues*>& GetAllModels() { return m_models; }
	std::map<std::string, KeyValues*>& GetAllTypes() { return m_types; }
	std::map<std::string, KeyValues*>& GetAllApps() { return m_apps; }
	void DetectAllLegacyCabinets();
	void DetectAllMaps();
	void DetectAllModels();	// OBSOLETE!!
	unsigned int ProcessModels(importInfo_t* pImportInfo);
	KeyValues* CreateModelFromFileTarget(std::string modelFile);
	bool ProcessModel(std::string modelFile);
	unsigned int DetectAllModelsRecursive(const char* folder);//(std::string pathname, FileFindHandle_t hFile, const char* pFilename);
	KeyValues* DetectFirstMap(bool& bAlreadyExists);
	KeyValues* DetectNextMap(bool& bAlreadyExists);
	void OnDetectAllMapsCompleted();
	void FlagDynamicModels();

	KeyValues* GetActiveKeyValues(KeyValues* entry);

	//void DetectMapClose();

	void SetLibraryBrowserContext(std::string category, std::string id, std::string search, std::string filter);
	std::string GetLibraryBrowserContext(std::string name);

	void ScaleObject(C_PropShortcutEntity* pEntity, int iDelta);
	void SetObjectScale(C_PropShortcutEntity* pEntity, float scale);

	int CycleSpawningRotationAxis(int direction);
	void ResetSpawningAngles();

	// if this function returns non-null, the CALLER is responsible for cleaning up the KV!
	KeyValues* DetectRequiredWorkshopForMapFile(std::string mapFile);
	KeyValues* DetectRequiredWorkshopForModelFile(std::string modelFile);

	// if this function returns non-null, the CALLER is responsible for cleaning up the KVs!
	void GetObjectInfo(object_t* pObject, KeyValues* &pObjectInfo, KeyValues* &pItemInfo, KeyValues* &pModelInfo);

	void HostSession();
	void HostSessionNow();
	void ReallyHostNow();
	void Disconnected() { m_bHasDisconnected = true; }
	bool GetHasDisconnected() { return m_bHasDisconnected; }
	void RestartNetwork(bool bCleanupAvatars = true);
	void ForgetAvatarDeathList();

	void InstanceUserAddedReceived(std::string userId, std::string sessionId, std::string displayName);
	void InstanceUserRemoved(std::string userId);

	void SyncToUser(std::string objectId, std::string oldObjectId);

	void UserSessionUpdated(int iUpdateMask, std::string userId, std::string sessionId, std::string displayName, std::string itemId, std::string objectId, std::string say, std::string bodyOrigin, std::string bodyAngles, std::string headOrigin, std::string headAngles, std::string mouseX, std::string mouseY, std::string webUrl, std::string avatarUrl);
	user_update_t* FindPendingUserUpdate(std::string userId);
	bool ProcessUserSessionUpdate(user_update_t* pUserUpdate);

	void BanSessionUser(std::string userId);
	void UnbanSessionUser(std::string userId);
	void SyncPano();
	void PanoSyncComplete(std::string cachedPanoName, std::string panoId);
	//void OverviewSyncComplete();

	void ExtractOverviewTGA();
	void OverviewExtracted();

	void ObjectUpdateReceived(bool bIsLocalUserUpdate, bool bIsFreshObject, std::string id, std::string item, std::string model, bool bSlave, bool bChild, std::string parentObject, float fScale, std::string in_origin, std::string in_angles);
	void SendChangeInstanceNotification(std::string instanceId, std::string map);
	void SendObjectUpdate(C_PropShortcutEntity* pShortcut);
	void SendObjectRemoved(object_t* pObject);
	void SendEntryUpdate(std::string mode, std::string entryId);
	void AvatarObjectCreated(int iEntIndex, std::string userId);

	void SendLocalChatMsg(std::string chatText);

	void InstanceUserClicked(user_t* pUser);

	user_t* GetInstanceUser(std::string userId);
	user_t* FindInstanceUser(C_DynamicProp* pProp);
	void GetAllInstanceUsers(std::vector<user_t*>& users);
	unsigned int GetNumInstanceUsers();
	void RemoveInstanceUser(std::string userId);
	void RemoveInstanceUser(user_t* pUser);
	void RemoveAllInstanceUsers();

	// accessors
	std::string GetFollowingId() { return m_followingUserId; }
	// FIXME: All of these spawning stuff should be in INSTANCE MANAGER not here!!
	int GetSpawningRotationAxis() { return m_iSpawningRotationAxis; }
	C_PropShortcutEntity* GetSpawningObjectEntity() { return m_pSpawningObjectEntity; }
	object_t* GetSpawningObject() { return m_pSpawningObject; }

	KeyValues* GetPreviousSearchInfo() { return m_pPreviousSearchInfo; }
	KeyValues* GetPreviousModelSearchInfo() { return m_pPreviousModelSearchInfo; }
	KeyValues* GetPreviousAppSearchInfo() { return m_pPreviousAppSearchInfo; }
	std::string GetPreviousLocaLocalItemLegacyWorkshopIds() { return m_previousLocaLocalItemLegacyWorkshopIds; }
	std::string GetLoadingScreenshotId() { return m_loadingScreenshotId; }
	user_t* GetLocalUser() { return m_pLocalUser; }

	// these should only be used for grabbing iterators!!
	std::map<std::string, KeyValues*> GetMapsMap() { return m_maps; }
	std::map<std::string, KeyValues*> GetAppsMap() { return m_apps; }
	std::map<std::string, KeyValues*> GetModelsMap() { return m_models; }
	std::map<std::string, KeyValues*> GetItemsMap() { return m_items; }
	std::map<std::string, KeyValues*> GetTypesMap() { return m_types; }

	// mutators
	void SetSpawningRotationAxis(int value) { m_iSpawningRotationAxis = value; }
	void SetSpawningObjectEntity(C_PropShortcutEntity* pShortcut) { m_pSpawningObjectEntity = pShortcut; }
	void SetSpawningObject(object_t* pObject) { m_pSpawningObject = pObject; }
	void SetPreviousLocaLocalItemLegacyWorkshopIds(std::string value) { m_previousLocaLocalItemLegacyWorkshopIds = value; }	// SHOULDN'T EVER BE USED EXCEPT FOR IN CASE OF HACKMERGENCY
	void SetLoadingScreenshotId(std::string val) { m_loadingScreenshotId = val; }

	void UserInfoReceived(HTTPRequestCompleted_t *pResult, bool bIOFailure);
	CCallResult<C_MetaverseManager, HTTPRequestCompleted_t> m_UserInfoCallback;
	
private:
	bool m_bHasDisconnected;
	user_t* m_pLocalUser;
	bool m_bHostSessionNow;
	std::map<std::string, user_update_t*> m_pendingUserUpdates;
	std::vector<C_DynamicProp*> m_avatarDeathList;
	std::string m_followingUserId;
	std::map<std::string, user_t*> m_users;
	std::string m_say;
	float m_fPresenceLastSynced;
	//unsigned int m_uProcessBatchSize;
	//unsigned int m_uProcessCurrentCycle;
	unsigned int m_uNumSteamGamesToImport;
	unsigned int m_uNumSteamGamesToImported;
	KeyValues* m_pImportSteamGamesKV;
	KeyValues* m_pImportSteamGamesSubKV;
	std::string m_loadingScreenshotId;
	std::vector<std::string> m_defaultFields;

	sqlite3* m_db;
	object_t* m_pSpawningObject;
	C_PropShortcutEntity* m_pSpawningObjectEntity;
	QAngle m_spawningAngles;
	int m_iSpawningRotationAxis;
	C_WebTab* m_pWebTab;
	std::map<std::string, KeyValues*> m_mapScreenshots;
	//std::map<std::string, std::string> m_mapScreenshots;

	std::map<addDefaultLibraryContext_t*, bool> m_addDefaultLibraryContexts;

	std::string m_libraryBrowserContextCategory;
	std::string m_libraryBrowserContextId;
	std::string m_libraryBrowserContextFilter;
	std::string m_libraryBrowserContextSearch;

	std::map<std::string, KeyValues*> m_maps;
	std::map<std::string, KeyValues*> m_apps;
	std::map<std::string, KeyValues*> m_models;
	std::map<std::string, KeyValues*> m_items;
	std::map<std::string, KeyValues*> m_types;
	std::map<std::string, KeyValues*>::iterator m_previousGetTypeIterator;
	std::map<std::string, KeyValues*>::iterator m_previousGetAppIterator;

	std::map<std::string, KeyValues*>::iterator m_previousGetItemIterator;	// are these used yet?????
	std::map<std::string, KeyValues*>::iterator m_previousFindItemIterator;
	std::map<std::string, KeyValues*>::iterator m_previousGetModelIterator;
//	std::map<std::string, KeyValues*>::iterator m_previousGetAppIterator;
	std::map<std::string, KeyValues*>::iterator m_previousFindModelIterator;
	std::map<std::string, KeyValues*>::iterator m_previousFindAppIterator;
	KeyValues* m_pPreviousSearchInfo;
	KeyValues* m_pPreviousModelSearchInfo;
	KeyValues* m_pPreviousAppSearchInfo;

	FileFindHandle_t m_previousLoadLocalAppFileHandle;
	std::string m_previousLoadLocalAppFilePath;

	FileFindHandle_t m_previousLoadLocalItemLegacyFileHandle;
	FileFindHandle_t m_previousLoadLocalItemLegacyFolderHandle;
	bool m_previousLoadLocalItemLegacyFastMode;
	std::string m_previousLoadLocalItemLegacyFile;
	std::string m_previousLoadLocalItemLegacyFilePath;
	std::string m_previousLoadLocalItemLegacyFolderPath;
	std::string m_previousLocaLocalItemLegacyWorkshopIds;
	std::string m_previousLoadLocalItemLegacyMountIds;
	C_Backpack* m_pPreviousLoadLocalItemLegacyBackpack;
	std::vector<KeyValues*> m_previousLoadLocalItemsLegacyBuffer;

	//std::string m_previousDetectLocalMapFilePath;
	FileFindHandle_t m_previousDetectLocalMapFileHandle;
};

#endif
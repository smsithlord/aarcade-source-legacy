#ifndef C_METAVERSE_MANAGER_H
#define C_METAVERSE_MANAGER_H

#include "c_webtab.h"
#include "filesystem.h"
#include "c_instancemanager.h"
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

	void Update();

	//void ImportSteamGame(std::string name, std::string appid);
	void ImportSteamGames(KeyValues* kv);

	void ModelFileChanged(std::string id);
	void AddModel(KeyValues* pModel);
	void AddItem(KeyValues* pItem);
	void AddType(KeyValues* pType);
	void SaveItem(KeyValues* pItem, sqlite3* pDb = null);
	void SaveModel(KeyValues* pItem, sqlite3* pDb = null);
	void SaveType(KeyValues* pType, sqlite3* pDb = null);
	//void SaveMap(KeyValues* pMap, sqlite3* pDb = null);
	void SaveSQL(sqlite3** pDb, const char* tableName, const char* id, KeyValues* kv);
	void DeleteSQL(sqlite3** pDb, const char* tableName, const char* id);
	//KeyValues* CreateItem(KeyValues* pInfo);
	bool CreateItem(int iLegacy, std::string itemId, KeyValues* pItemKV, std::string title, std::string description, std::string file, std::string type, std::string app, std::string reference, std::string preview, std::string download, std::string stream, std::string screen, std::string marquee, std::string model);

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

	const char* GetFirstLibraryEntry(KeyValues*& response, const char* category);
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

	KeyValues* GetFirstLibraryApp();
	KeyValues* GetNextLibraryApp();
	KeyValues* GetLibraryApp(std::string id);

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
	KeyValues* DetectFirstMap(bool& bAlreadyExists);
	KeyValues* DetectNextMap(bool& bAlreadyExists);
	void OnDetectAllMapsCompleted();
	void FlagDynamicModels();

	KeyValues* GetActiveKeyValues(KeyValues* entry);

	//KeyValues* FindLibraryType(std::string term);

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

	// accessors
	// FIXME: All of these spawning stuff should be in INSTANCE MANAGER not here!!
	int GetSpawningRotationAxis() { return m_iSpawningRotationAxis; }
	C_PropShortcutEntity* GetSpawningObjectEntity() { return m_pSpawningObjectEntity; }
	object_t* GetSpawningObject() { return m_pSpawningObject; }

	KeyValues* GetPreviousSearchInfo() { return m_pPreviousSearchInfo; }
	KeyValues* GetPreviousModelSearchInfo() { return m_pPreviousModelSearchInfo; }
	std::string GetPreviousLocaLocalItemLegacyWorkshopIds() { return m_previousLocaLocalItemLegacyWorkshopIds; }
	std::string GetLoadingScreenshotId() { return m_loadingScreenshotId; }

	// mutators
	void SetSpawningRotationAxis(int value) { m_iSpawningRotationAxis = value; }
	void SetSpawningObjectEntity(C_PropShortcutEntity* pShortcut) { m_pSpawningObjectEntity = pShortcut; }
	void SetSpawningObject(object_t* pObject) { m_pSpawningObject = pObject; }
	void SetPreviousLocaLocalItemLegacyWorkshopIds(std::string value) { m_previousLocaLocalItemLegacyWorkshopIds = value; }	// SHOULDN'T EVER BE USED EXCEPT FOR IN CASE OF HACKMERGENCY
	void SetLoadingScreenshotId(std::string val) { m_loadingScreenshotId = val; }
	
private:
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
	std::map<std::string, KeyValues*>::iterator m_previousFindModelIterator;
	KeyValues* m_pPreviousSearchInfo;
	KeyValues* m_pPreviousModelSearchInfo;

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
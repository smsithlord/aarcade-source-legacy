#ifndef C_METAVERSE_MANAGER_H
#define C_METAVERSE_MANAGER_H

#include "c_webtab.h"
#include "filesystem.h"
#include "c_instancemanager.h"
//#include <string>
//#include <vector>

class C_MetaverseManager
{
public:
	C_MetaverseManager();
	~C_MetaverseManager();
	
	void Update();

	//void OnWebTabCreated(C_WebTab* pWebTab);
	void OnMountAllWorkshopsCompleted();

	void AddItem(KeyValues* pItem);
	void SaveItem(KeyValues* pItem);
	KeyValues* CreateItem(KeyValues* pInfo);

	void SmartMergItemKVs(KeyValues* pItemA, KeyValues* pItemB, bool bPreferFirst = true);// , bool bFullMerg, bool bRefreshArtworkIfNeeded = true);

	// local legacy
	KeyValues* LoadLocalItemLegacy(bool& bIsModel, std::string file, std::string filePath = "", std::string workshopIds = "", std::string mountIds = "");
	unsigned int LoadAllLocalItemsLegacy(unsigned int& uNumModels, std::string filePath = "", std::string workshopIds = "", std::string mountIds = "");	// probably obsolete!!!

	void LoadFirstLocalItemLegacy(bool bFastMode = true, std::string filePath = "", std::string workshopIds = "", std::string mountIds = "");
	void LoadNextLocalItemLegacy();
	void LoadLocalItemLegacyClose();
	void ResolveLoadLocalItemLegacyBuffer();

	// local
	std::string ResolveLegacyType(std::string legacyType);
	KeyValues* LoadLocalType(std::string file, std::string filePath = "");
	unsigned int LoadAllLocalTypes(std::string filePath = "");

	std::string ResolveLegacyModel(std::string legacyModel);
	KeyValues* LoadLocalModel(std::string file, std::string filePath = "");
	unsigned int LoadAllLocalModels(std::string filePath = "");

	KeyValues* LoadLocalItem(std::string file, std::string filePath = "");
	unsigned int LoadAllLocalItems(std::string filePath = "");

	std::string ResolveLegacyApp(std::string legacyApp);
	KeyValues* LoadLocalApp(std::string file, std::string filePath = "", std::string searchPath = "");
	unsigned int LoadAllLocalApps(std::string filePath = "");
	KeyValues* LoadFirstLocalApp(std::string filePath = "");
	KeyValues* LoadNextLocalApp();
	void LoadLocalAppClose();

	KeyValues* GetFirstLibraryItem();
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

	KeyValues* FindFirstLibraryItem(KeyValues* pSearchInfo);
	KeyValues* FindNextLibraryItem();
	KeyValues* FindLibraryItem(KeyValues* pSearchInfo, std::map<std::string, KeyValues*>::iterator& it);
	KeyValues* FindLibraryItem(KeyValues* pSearchInfo);

	KeyValues* GetFirstLibraryType();
	KeyValues* GetNextLibraryType();
	KeyValues* GetLibraryType(std::string id);

	KeyValues* GetFirstLibraryApp();
	KeyValues* GetNextLibraryApp();
	KeyValues* GetLibraryApp(std::string id);

	std::map<std::string, std::string>& DetectAllMapScreenshots();
	std::map<std::string, std::string>& GetAllMapScreenshots() { return m_mapScreenshots; }


	KeyValues* GetMap(std::string mapId);
	std::map<std::string, KeyValues*>& GetAllMaps() { return m_maps; }
	void DetectAllLegacyCabinets();
	void DetectAllMaps();
	KeyValues* DetectFirstMap(bool& bAlreadyExists);
	KeyValues* DetectNextMap(bool& bAlreadyExists);
	void OnDetectAllMapsCompleted();
	void FlagDynamicModels();

	//void DetectMapClose();

	// accessors
	// FIXME: All of these spawning stuff should be in INSTANCE MANAGER not here!!
	C_PropShortcutEntity* GetSpawningObjectEntity() { return m_pSpawningObjectEntity; }
	object_t* GetSpawningObject() { return m_pSpawningObject; }

	KeyValues* GetPreviousSearchInfo() { return m_pPreviousSearchInfo; }
	KeyValues* GetPreviousModelSearchInfo() { return m_pPreviousModelSearchInfo; }
	std::string GetPreviousLocaLocalItemLegacyWorkshopIds() { return m_previousLocaLocalItemLegacyWorkshopIds; }

	// mutators
	void SetSpawningObjectEntity(C_PropShortcutEntity* pShortcut) { m_pSpawningObjectEntity = pShortcut; }
	void SetSpawningObject(object_t* pObject) { m_pSpawningObject = pObject; }
	void SetPreviousLocaLocalItemLegacyWorkshopIds(std::string value) { m_previousLocaLocalItemLegacyWorkshopIds = value; }	// SHOULDN'T EVER BE USED EXCEPT FOR IN CASE OF HACKMERGENCY
	
private:
	object_t* m_pSpawningObject;
	C_PropShortcutEntity* m_pSpawningObjectEntity;
	C_WebTab* m_pWebTab;
	std::map<std::string, std::string> m_mapScreenshots;

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
	std::vector<KeyValues*> m_previousLoadLocalItemsLegacyBuffer;

	//std::string m_previousDetectLocalMapFilePath;
	FileFindHandle_t m_previousDetectLocalMapFileHandle;
};

#endif
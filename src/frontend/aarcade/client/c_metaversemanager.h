#ifndef C_METAVERSE_MANAGER_H
#define C_METAVERSE_MANAGER_H

#include "c_webtab.h"
#include "filesystem.h"
//#include <string>
//#include <vector>

class C_MetaverseManager
{
public:
	C_MetaverseManager();
	~C_MetaverseManager();
	
	void OnWebTabCreated(C_WebTab* pWebTab);
	void OnMountAllWorkshopsCompleted();

	// local legacy
	KeyValues* LoadLocalItemLegacy(bool& bIsModel, std::string file, std::string filePath = "", std::string workshopIds = "", std::string mountIds = "");
	unsigned int LoadAllLocalItemsLegacy(unsigned int& uNumModels, std::string filePath = "", std::string workshopIds = "", std::string mountIds = "");

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

	std::string ResolveLegacyApp(std::string legacyApp);
	KeyValues* LoadLocalApp(std::string file, std::string filePath = "", std::string searchPath = "");
	unsigned int LoadAllLocalApps(std::string filePath = "");
	KeyValues* LoadFirstLocalApp(std::string filePath = "");
	KeyValues* LoadNextLocalApp();
	void LoadLocalAppClose();

	KeyValues* GetFirstLibraryItem();
	KeyValues* GetNextLibraryItem();
	KeyValues* GetLibraryItem(std::string id);

	KeyValues* FindFirstLibraryItem(KeyValues* pSearchInfo);
	KeyValues* FindNextLibraryItem();
	KeyValues* FindLibraryItem(KeyValues* pSearchInfo, std::map<std::string, KeyValues*>::iterator& it);
	KeyValues* FindLibraryItem(KeyValues* pSearchInfo);

	KeyValues* GetFirstLibraryType();
	KeyValues* GetNextLibraryType();
	KeyValues* GetLibraryType(std::string id);

	KeyValues* GetLibraryApp(std::string id);

	// accessors
	KeyValues* GetPreviousSearchInfo() { return m_pPreviousSearchInfo; }
	std::string GetPreviousLocaLocalItemLegacyWorkshopIds() { return m_previousLocaLocalItemLegacyWorkshopIds; }

	// mutators
	
private:
	C_WebTab* m_pWebTab;
	std::map<std::string, KeyValues*> m_apps;
	std::map<std::string, KeyValues*> m_models;
	std::map<std::string, KeyValues*> m_items;
	std::map<std::string, KeyValues*> m_types;
	std::map<std::string, KeyValues*>::iterator m_previousGetTypeIterator;
	std::map<std::string, KeyValues*>::iterator m_previousGetItemIterator;	// are these used yet?????
	std::map<std::string, KeyValues*>::iterator m_previousFindItemIterator;
	KeyValues* m_pPreviousSearchInfo;

	FileFindHandle_t m_previousLoadLocalAppFileHandle;
	std::string m_previousLoadLocalAppFilePath;

	FileFindHandle_t m_previousLoadLocalItemLegacyFileHandle;
	FileFindHandle_t m_previousLoadLocalItemLegacyFolderHandle;
	std::string m_previousLoadLocalItemLegacyFile;
	std::string m_previousLoadLocalItemLegacyFilePath;
	std::string m_previousLoadLocalItemLegacyFolderPath;
	std::string m_previousLocaLocalItemLegacyWorkshopIds;
	std::string m_previousLoadLocalItemLegacyMountIds;
	std::vector<KeyValues*> m_previousLoadLocalItemsLegacyBuffer;
};

#endif
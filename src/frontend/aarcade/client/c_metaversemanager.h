#ifndef C_METAVERSE_MANAGER_H
#define C_METAVERSE_MANAGER_H

#include "c_webtab.h"
//#include <string>
//#include <vector>

class C_MetaverseManager
{
public:
	C_MetaverseManager();
	~C_MetaverseManager();
	
	void OnWebTabCreated(C_WebTab* pWebTab);

	// local
	KeyValues* LoadLocalItemLegacy(bool& bIsModel, std::string file, std::string filePath = "", std::string workshopIds = "", std::string mountIds = "");
	unsigned int LoadAllLocalItemsLegacy(unsigned int& uNumModels, std::string filePath = "", std::string workshopIds = "", std::string mountIds = "");

	KeyValues* LoadLocalType(std::string file, std::string filePath = "");
	unsigned int LoadAllLocalTypes(std::string filePath = "");
	std::string ResolveLegacyType(std::string legacyType);

	KeyValues* LoadLocalModel(std::string file, std::string filePath = "");
	unsigned int LoadAllLocalModels(std::string filePath = "");
	std::string ResolveLegacyModel(std::string legacyModel);

	KeyValues* LoadLocalApp(std::string file, std::string filePath = "");
	unsigned int LoadAllLocalApps(std::string filePath = "");
	std::string ResolveLegacyApp(std::string legacyApp);

	KeyValues* GetFirstLibraryType();
	KeyValues* GetNextLibraryType();
	KeyValues* GetLibraryType(std::string id);
	//KeyValues* GetLibraryType(std::string id);

	KeyValues* GetFirstLibraryItem();
	KeyValues* GetNextLibraryItem();
	KeyValues* GetLibraryItem(std::string id);

	KeyValues* FindFirstLibraryItem(KeyValues* pSearchInfo);
	KeyValues* FindNextLibraryItem();
	KeyValues* FindLibraryItem(KeyValues* pSearchInfo, std::map<std::string, KeyValues*>::iterator& it);
	KeyValues* FindLibraryItem(KeyValues* pSearchInfo);

	KeyValues* GetLibraryApp(std::string id);

	// accessors
	KeyValues* GetPreviousSearchInfo() { return m_pPreviousSearchInfo; }

	// mutators
	
private:
	C_WebTab* m_pWebTab;
	std::map<std::string, KeyValues*> m_apps;
	std::map<std::string, KeyValues*> m_models;
	std::map<std::string, KeyValues*> m_items;
	std::map<std::string, KeyValues*> m_types;
	std::map<std::string, KeyValues*>::iterator m_previousGetTypeIterator;
	std::map<std::string, KeyValues*>::iterator m_previousGetItemIterator;
	std::map<std::string, KeyValues*>::iterator m_previousFindItemIterator;
	KeyValues* m_pPreviousSearchInfo;
};

#endif
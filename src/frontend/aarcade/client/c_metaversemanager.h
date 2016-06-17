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
	KeyValues* LoadLocalItem(std::string file, std::string filePath = "");
	unsigned int LoadAllLocalItems(std::string filePath = "");

	// accessors

	// mutators
	
private:
	C_WebTab* m_pWebTab;
	std::map<std::string, KeyValues*> m_items;
};

#endif
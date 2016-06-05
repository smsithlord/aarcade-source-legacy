#ifndef C_MOUNT_MANAGER_H
#define C_MOUNT_MANAGER_H

#include "c_mount.h"
#include <vector>

class C_MountManager
{
public:
	C_MountManager();
	~C_MountManager();

	void Init();
	bool LoadMountsFromKeyValues(std::string filename);
//	void Update();

	// accessors
	std::vector<std::string>* GetLibraryPaths() { return &m_libraryPaths; }

	// mutators
	
private:
	std::vector<std::string> m_libraryPaths;
	//std::map<std::string, C_Mount*> m_webTabs;
};

#endif
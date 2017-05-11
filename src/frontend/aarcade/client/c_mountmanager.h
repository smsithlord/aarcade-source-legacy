#ifndef C_MOUNT_MANAGER_H
#define C_MOUNT_MANAGER_H

#include "c_mount.h"
#include <vector>
#include <map>

class C_MountManager
{
public:
	C_MountManager();
	~C_MountManager();

	void Init();
	bool LoadMountsFromKeyValues(std::string filename);
//	void Update();

	C_Mount* GetMount(std::string id);
	void GetAllMounts(std::vector<C_Mount*>& responseVector);

	// accessors
	std::vector<std::string>* GetLibraryPaths() { return &m_libraryPaths; }
	C_Mount* FindOwningMount(std::string file);

	// mutators
	
private:
	std::vector<std::string> m_libraryPaths;
	std::map<std::string, C_Mount*> m_mounts;
	//std::map<std::string, C_Mount*> m_webTabs;
};

#endif
#ifndef C_BACKPACK_H
#define C_BACKPACK_H

#include <map>
#include <string>
#include <vector>
#include "filesystem.h"
#include "../../sqlite/include/sqlite/sqlite3.h"

class C_Backpack
{
public:
	C_Backpack();
	~C_Backpack();

	void Init(std::string id, std::string title, std::string base);
	void Activate();
	void Prepare();
	void Release();
	void CreateDb();
	void CloseDb();
	void LegacyAuditDb();
	void MergDb();
	bool IsDbOpen();

	void vpkDetect();
	void GetAllVPKs(std::vector<std::string>& allVPKs);
	void GetAllFiles(std::vector<std::string>& allFiles);
	//void GetBackpackInfo(KeyValues* pBackpackKV);

	// accessors
	bool IsPrepared() { return m_bPrepared; }
	std::string GetId() { return m_id; }
	std::string GetTitle() { return m_title; }
	std::string GetBackpackFolder() { return m_backpackFolder; }
	
	// mutators
	void SetPrepared(bool bValue) { m_bPrepared = bValue; }

protected:
	void GetAllFilesRecursive(std::vector<std::string>& allFiles, std::string path, const char* pFilename, FileFindHandle_t& fileFindHandle);
	
private:
	bool m_bPrepared;
	bool m_bActive;
	std::string m_searchPath;
	std::string m_id;
	std::string m_title;
	std::string m_backpackFolder;
	std::vector<std::string> m_vpks;

	// FIXME: These backpacks have **SO** much in common with the active library, that their functions should be merged, so improvements can benefit both. Death to (most) redundancy!
	sqlite3* m_pDb;
	std::map<std::string, KeyValues*> m_types;
	std::map<std::string, KeyValues*> m_items;
	std::map<std::string, KeyValues*> m_models;
	std::map<std::string, KeyValues*> m_apps;
	std::map<std::string, KeyValues*> m_instances;
};

#endif
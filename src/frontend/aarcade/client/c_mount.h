#ifndef C_MOUNT_H
#define C_MOUNT_H

#include <string>
#include <vector>

class C_Mount
{
public:
	C_Mount();
	~C_Mount();

	void Init(std::string id, std::string title, std::string base, std::vector<std::string> paths);
	bool Activate();

	bool DoesOwn(std::string file);
//	void Update();

	// accessors
	bool GetActive() { return m_bActive; }
	std::string GetId() { return m_id; }
	std::string GetTitle() { return m_title; }
	std::string GetBase() { return m_base; }
	std::vector<std::string> GetPaths() { return m_paths; }
	std::vector<std::string> GetMountedPaths() { return m_mountedPaths; }

	// mutators
	
private:
	bool m_bActive;
	std::string m_id;
	std::string m_title;
	std::string m_base;
	std::vector<std::string> m_paths;
	std::vector<std::string> m_mountedPaths;
};

#endif
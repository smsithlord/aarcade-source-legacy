#ifndef C_MOUNT_H
#define C_MOUNT_H

#include <string>
#include <vector>

class C_Mount
{
public:
	C_Mount();
	~C_Mount();

	void Init(std::string id, std::string base, std::vector<std::string> paths);
	bool Activate();
//	void Update();

	// accessors

	// mutators
	
private:
	bool m_bActive;
	std::string m_id;
	std::string m_base;
	std::vector<std::string> m_paths;
	std::vector<std::string> m_mountedPaths;
};

#endif
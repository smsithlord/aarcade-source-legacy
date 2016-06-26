#ifndef C_INSTANCE_MANAGER_H
#define C_INSTANCE_MANAGER_H

#include "KeyValues.h"
#include <map>
#include <vector>
#include <string>

struct object_t
{
	std::string itemId;
	std::string modelId;
	Vector origin;
	Vector angles;
	bool spawned;
};

class C_InstanceManager
{
public:
	C_InstanceManager();
	~C_InstanceManager();

	void LoadLegacyInstance();

	void AddObject(std::string objectId, std::string itemId, std::string modelId, Vector origin, QAngle angles);
	bool SpawnNearestObject();
	//void SpawnItem(std::string id);

	// accessors

	// mutators
	
private:
	std::map<std::string, object_t*> m_objects;
	std::vector<object_t*> m_unspawnedObjects;
};

#endif
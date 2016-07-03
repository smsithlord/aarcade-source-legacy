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
	std::string objectId;
	Vector origin;
	Vector angles;
	bool spawned;
};

struct instance_t
{
	std::string id;
	std::string mapId;
	std::string title;
	std::string file;
	std::string workshopIds;
	std::string mountIds;
};

class C_InstanceManager
{
public:
	C_InstanceManager();
	~C_InstanceManager();

	void LoadLegacyInstance(std::string instanceId);

	void AddObject(std::string objectId, std::string itemId, std::string modelId, Vector origin, QAngle angles);
	bool SpawnNearestObject();
	void SetNearestSpawnDist(double maxDist) { m_fNearestSpawnDist = (float)m_fNearestSpawnDist = maxDist; }

	void AddInstance(std::string instanceId, std::string mapId, std::string title, std::string file, std::string workshopIds, std::string mountIds);
	instance_t* GetInstance(std::string id);
	instance_t* FindInstance(std::string mapId);
	void FindAllInstances(std::string mapId, std::vector<instance_t*> &instances);
	void LegacyMapIdFix(std::string legacyMapName, std::string mapId);
	//void SpawnItem(std::string id);

	// accessors

	// mutators
	
private:
	float m_fNearestSpawnDist;
	std::map<std::string, object_t*> m_objects;
	std::vector<object_t*> m_unspawnedObjects;

	std::map<std::string, instance_t*> m_instances;
};

#endif
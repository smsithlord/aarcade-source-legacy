#ifndef C_INSTANCE_MANAGER_H
#define C_INSTANCE_MANAGER_H

#include "KeyValues.h"
#include <map>
#include <vector>
#include <string>
#include "c_prop_shortcut.h"
#include "c_backpack.h"

struct object_t
{
	std::string objectId;
	unsigned int created;
	std::string owner;
	unsigned int removed;
	std::string remover;
	unsigned int modified;
	std::string modifier;
	std::string itemId;
	std::string modelId;
	Vector origin;
	QAngle angles;
	bool spawned;
	bool child;
	float scale;
	bool slave;
	int entityIndex;
	//C_BaseEntity* entity;
};

struct instance_t
{
	std::string id;
	std::string mapId;
	std::string title;
	std::string style;
	std::string file;
	std::string workshopIds;
	std::string mountIds;
	//std::string backpackId;
	int legacy;
};

struct transform_t
{
	float offX;
	float offY;
	float offZ;
	float rotP;
	float rotY;
	float rotR;
	float scale;
};

class C_InstanceManager
{
public:
	C_InstanceManager();
	~C_InstanceManager();

	void LevelShutdownPostEntity();

	void Update();

	void LoadInstance(std::string instanceId, std::string position = "", std::string rotation = "");
	void LoadLegacyInstance(std::string instanceId, KeyValues* instanceKV);

	void ApplyChanges(C_PropShortcutEntity* pShortcut);
	void ResetObjectChanges(C_PropShortcutEntity* pShortcut);

	transform_t* GetTransform() { return m_pTransform; }
	void ResetTransform();
	void AdjustObjectOffset(float x, float y, float z);
	void AdjustObjectRot(float p, float y, float r);
	void AdjustObjectScale(float scale);

	void SetObjectEntity(std::string objectId, C_BaseEntity* pEntity);
	C_BaseEntity* GetObjectEntity(std::string objectId);

	object_t* GetNearestObjectToPlayerLook(object_t* pStartingObject = null);

	std::string CreateBlankInstance(int iLegacy = 0, KeyValues* pInstanceKV = null, std::string instanceId = "", std::string mapId = "", std::string title = "", std::string file = "", std::string workshopIds = "", std::string mountIds = "", std::string style = "", C_Backpack* pBackpack = null);
	void CreateObject(KeyValues* pObjectKV, std::string objectId, std::string itemId, std::string modelId, std::string position, std::string rotation, float scale, int slave, int child);

	void SpawnObject(object_t* object, bool bShouldGhost = false);
	object_t* AddObject(std::string objectId, std::string itemId, std::string modelId, Vector origin, QAngle angles, float scale, bool slave, unsigned int created = 0, std::string owner = "", unsigned int removed = 0, std::string remover = "", unsigned int modified = 0, std::string modifier = "", bool isChild = false);
	object_t* GetInstanceObject(std::string objectId);
	unsigned int GetInstanceObjectCount();
	void RemoveEntity(C_PropShortcutEntity* pShortcutEntity);
	bool SpawnNearestObject();
	//void SetNearestSpawnDist(double maxDist) { m_fNearestSpawnDist = (float)m_fNearestSpawnDist = maxDist; }
	int SetNearestSpawnDist(double maxDist);	// returns how many unspawned objects are within that dist

	void AddInstance(int iLegacy, std::string instanceId, std::string mapId, std::string title, std::string file = "", std::string workshopIds = "", std::string mountIds = "", std::string style = "");
	instance_t* GetInstance(std::string id);
	instance_t* FindInstance(std::string instanceId);
	instance_t* GetCurrentInstance();
	void FindAllInstances(std::string mapId, std::vector<instance_t*> &instances);
	void LegacyMapIdFix(std::string legacyMapName, std::string mapId);

	bool ConsumeLegacyInstance(std::string instanceId, std::string filename, std::string path, std::string searchPath, std::string workshopIds, std::string mountIds, C_Backpack* pBackpack);

	void SpawnActionPressed();
	void ChangeModel(C_BaseEntity* pEntity, std::string modelId, std::string in_modelFile);
	//void SpawnItem(std::string id);

	// accessors
	int GetUnspawnedWithinRangeEstimate() { return m_iUnspawnedWithinRangeEstimate; }

	// mutators
	
private:
	int m_iUnspawnedWithinRangeEstimate;
	transform_t* m_pTransform;
	KeyValues* m_pInstanceKV;
	unsigned int m_uNextFlashedObject;
	float m_fLastSpawnActionPressed;
	float m_fNearestSpawnDist;
	std::map<std::string, object_t*> m_objects;
	std::vector<object_t*> m_unspawnedObjects;

	std::map<std::string, instance_t*> m_instances;
};

#endif
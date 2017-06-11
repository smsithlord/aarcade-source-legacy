#ifndef C_BACKPACK_MANAGER_H
#define C_BACKPACK_MANAGER_H

#include "c_backpack.h"
#include <map>

//#include "steam/steam_api.h"
//#include <vector>

/*
class C_WorkshopQuery;

struct SteamWorkshopKeyValueTag_t
{
	std::string key;
	std::string value;
};

struct SteamWorkshopDetails_t
{
	PublishedFileId_t publishedFileId;
	EWorkshopFileType type;
	std::string title;
	std::string description;
	uint64 owner;
	uint32 created;
	uint32 updated;
	uint32 subscribed;
	ERemoteStoragePublishedFileVisibility visibility;
	bool banned;
	//bool accepted;
	bool tagsTruncated;
	std::string tags;
	UGCHandle_t file;
	UGCHandle_t preview;
	std::string filename;
	int32 fileSize;
	int32 previewSize;
	std::string url;
	uint32 votesUp;
	uint32 votesDown;
	float score;
	uint32 numChildren;
	std::string previewURL;
	uint32 numSubscriptions;
	uint32 numFavorites;
	//uint32 numAdditionalPreviews;
	std::vector<std::string> additionalPreviewURLs;
	std::vector<SteamWorkshopKeyValueTag_t*> keyValueTags;
	//uint32 numFollowers;
	//uint32 numUniqueSubscriptions;
	//uint32 numUniqueFavorites;
	//uint32 numUniqueFollowers;
	//uint32 numUniqueWebsiteViews;
	//uint32 reportScore;
	//uint32 numSecondsPlayed;
	//uint32 numPlaytimeSessions;
	//uint32 numComments;
};
*/

class C_BackpackManager
{
public:
	C_BackpackManager();
	~C_BackpackManager();

	void Init();
	void Update();
	void LoadBackpack(std::string backpackFolder);

	std::string DetectRequiredBackpackForModelFile(std::string modelFile);
	C_Backpack* CreateBackpack(std::string backpackFolder);
	C_Backpack* FindBackpackByFolder(std::string backpackFolder);
	std::string ExtractBackpackFoldeNameFromPath(std::string backpackFolder);

	C_Backpack* FindBackpackWithInstanceId(std::string id);

	void ActivateAllBackpacks();
	void DetectBackpack(std::string backpackFolder);
	void DetectAllBackpacks();

	C_Backpack* GetBackpack(std::string backpackId);
	void GetAllBackpacks(std::vector<C_Backpack*>& response);

	// accessors
	bool IsInitialized() { return m_bInitialized; }
	std::string GetCustomFolder() { return m_customFolder; }

	// mutators
	
private:
	std::string m_customFolder;
	bool m_bInitialized;
	std::map<std::string, C_Backpack*> m_backpacks;
	//std::map<PublishedFileId_t, SteamWorkshopDetails_t*>::iterator m_previousMountWorkshopIterator;
};

#endif
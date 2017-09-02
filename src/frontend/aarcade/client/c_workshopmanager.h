#ifndef C_WORKSHOP_MANAGER_H
#define C_WORKSHOP_MANAGER_H

#include "steam/steam_api.h"
#include <map>
#include <vector>

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
	// Changed for Steamworks update
	//uint64 numSubscriptions;
	//uint64 numFavorites;
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

class C_WorkshopManager
{
public:
	C_WorkshopManager();
	~C_WorkshopManager();

	void Init();
	void OnQueryComplete(C_WorkshopQuery* pQuery);

	void MountWorkshop(PublishedFileId_t id, bool& bIsLegacy, unsigned int& uNumItems, unsigned int& uNumModels, SteamWorkshopDetails_t* pDetails = null);
	void OnMountWorkshopFail();
	void OnMountWorkshopSucceed();

	void GetAllWorkshopSubscriptions(std::vector<SteamWorkshopDetails_t*>& details);
	SteamWorkshopDetails_t* GetWorkshopSubscription(PublishedFileId_t id);
	void AddWorkshopDetails(SteamWorkshopDetails_t* pDetails);

	void MountFirstWorkshop();
	void MountNextWorkshop();

	bool IsEnabled() { return m_bWorkshopEnabled; }

	void OnFinishedMountingAllWorkshop();

	unsigned int GetNumDetails();
	SteamWorkshopDetails_t* GetDetails(unsigned int index);

	// accessors

	// mutators
	
private:
	bool m_bWorkshopEnabled;
	std::map<PublishedFileId_t, SteamWorkshopDetails_t*> m_details;
	std::map<PublishedFileId_t, SteamWorkshopDetails_t*>::iterator m_previousMountWorkshopIterator;
	bool m_bMountWorkshopIsLegacy;
	unsigned int m_uMountWorkshopuNumItems;
	unsigned int m_uMountWorkshopNumModels;
};

class C_WorkshopQuery
{
public:
	C_WorkshopQuery();
	~C_WorkshopQuery();

	void Init(SteamAPICall_t hAPICall);
	CCallResult<C_WorkshopQuery, SteamUGCQueryCompleted_t> callback;

	void OnUGCQueried(SteamUGCQueryCompleted_t *pResult, bool bIOFailure);

private:
	uint32 m_uPageNum;
};

#endif
#ifndef C_WORKSHOP_MANAGER_H
#define C_WORKSHOP_MANAGER_H

#include "steam/steam_api.h"
#include <map>

class C_WorkshopQuery;

class C_WorkshopManager
{
public:
	C_WorkshopManager();
	~C_WorkshopManager();

	void Init();
	void OnQueryComplete(C_WorkshopQuery* pQuery);

	void MountWorkshop(PublishedFileId_t id, bool& bIsLegacy, unsigned int& uNumItems, unsigned int& uNumModels, SteamUGCDetails_t* pDetails = null);
	void OnMountWorkshopFail();
	void OnMountWorkshopSucceed();

	void AddWorkshopDetails(SteamUGCDetails_t* pDetails);

	//void MountAllWorkshops();
	void MountFirstWorkshop();
	void MountNextWorkshop();
	void MountWorkshopClose();

	unsigned int GetNumDetails();
	SteamUGCDetails_t* GetDetails(unsigned int index);

	// accessors

	// mutators
	
private:
	std::map<PublishedFileId_t, SteamUGCDetails_t*> m_details;
	std::map<PublishedFileId_t, SteamUGCDetails_t*>::iterator m_previousMountWorkshopIterator;
	bool m_bMountWorkshopIsLegacy;
	//unsigned int m_uMountWorkshopNumLegacyItems;
	unsigned int m_uMountWorkshopuNumItems;
	unsigned int m_uMountWorkshopNumModels;
//	unsigned int m_uMountWorkshopuNumItemsTotal;
//	unsigned int m_uMountWorkshopNumModelsTotal;
};

class C_WorkshopQuery
{
public:
	C_WorkshopQuery();
	~C_WorkshopQuery(){};

	void Init(SteamAPICall_t hAPICall);
	CCallResult<C_WorkshopQuery, SteamUGCQueryCompleted_t> callback;

//	CCallResult<C_WorkshopQuery, SteamUGCQueryCompleted_t> m_callbackUGCQueried;
	void OnUGCQueried(SteamUGCQueryCompleted_t *pResult, bool bIOFailure);

private:
	uint32 m_uPageNum;
	//uint32 m_uTotalCount;
};

#endif
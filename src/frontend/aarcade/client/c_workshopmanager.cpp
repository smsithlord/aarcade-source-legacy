#include "cbase.h"
#include "c_workshopmanager.h"

#include "aa_globals.h"
#include "c_anarchymanager.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_WorkshopManager::C_WorkshopManager()
{
	DevMsg("WorkshopManager: Constructor\n");
}

C_WorkshopManager::~C_WorkshopManager()
{
	DevMsg("WorkshopManager: Destructor\n");
}

void C_WorkshopManager::Init()
{
	DevMsg("WorkshopManager: Init\n");

	// Start the 
	AccountID_t aid = steamapicontext->SteamUser()->GetSteamID().GetAccountID();
	UGCQueryHandle_t hUGCQuery = steamapicontext->SteamUGC()->CreateQueryUserUGCRequest(aid, k_EUserUGCList_Subscribed, k_EUGCMatchingUGCType_Items, k_EUserUGCListSortOrder_SubscriptionDateDesc, engine->GetAppID(), engine->GetAppID(), 1);

	SteamAPICall_t hAPICall = steamapicontext->SteamUGC()->SendQueryUGCRequest(hUGCQuery);

	C_WorkshopQuery* pWorkshopQuery = new C_WorkshopQuery();
	pWorkshopQuery->Init(hAPICall);
}

void C_WorkshopManager::OnQueryComplete(C_WorkshopQuery* pQuery)
{
	delete pQuery;

	g_pAnarchyManager->OnWorkshopManagerReady();
}

C_WorkshopQuery::C_WorkshopQuery()
{
	m_uPageNum = 1;
}

void C_WorkshopQuery::Init(SteamAPICall_t hAPICall)
{
	callback.Set(hAPICall, this, &C_WorkshopQuery::OnUGCQueried);
}

void C_WorkshopQuery::OnUGCQueried(SteamUGCQueryCompleted_t* pResult, bool bIOFailure)
{
	DevMsg("WorkshopManager: OnUGCQueried\n");
	if (pResult->m_eResult == k_EResultOK)
	{
		DevMsg("Batch Size: %u Total Found: %u\n", pResult->m_unNumResultsReturned, pResult->m_unTotalMatchingResults);
		//m_UGCQueryHandle = pResult->m_handle;
	//	m_iUGCQueryNumReturned = pResult->m_unNumResultsReturned;
		//m_uTotalCount = pResult->m_unTotalMatchingResults;

//		this->SteamUGCQueryInit();


		
//		m_details

		unsigned int previousNum = (m_uPageNum - 1) * 50;

		unsigned int i;
		for (i = 0; i < pResult->m_unNumResultsReturned; i++)
		{
			SteamUGCDetails_t* pDetails = new SteamUGCDetails_t;
			steamapicontext->SteamUGC()->GetQueryUGCResult(pResult->m_handle, i, pDetails);

			if (pDetails->m_eResult == k_EResultOK)
			{
				//m_details[pDetails->m_nPublishedFileId] = pDetails;
				g_pAnarchyManager->GetWorkshopManager()->AddWorkshopDetails(pDetails);
				DevMsg("Added %s\n", pDetails->m_rgchTitle);// %llu\n", pDetails->m_nPublishedFileId);

				///*
				std::string num = VarArgs("%u", i + previousNum + 1);
				std::string max = VarArgs("%u", pResult->m_unTotalMatchingResults);
				g_pAnarchyManager->GetWebManager()->GetHudWebTab()->AddHudLoadingMessage("progress", "", "Fetching Workshop Subscriptions", "workfetch", "0", max, num);
//				g_pAnarchyManager->GetWebManager()->GetHudWebTab()->AddHudLoadingMessage("progress", "", "Fetching Workshop Subscriptions", "workfetch", "", "", "");
				//*/
				

				/*
				unsigned int iStateFlags = steamapicontext->SteamUGC()->GetItemState(pDetails->m_nPublishedFileId);
				if (iStateFlags & k_EItemStateSubscribed && iStateFlags & k_EItemStateInstalled && !(iStateFlags & k_EItemStateNeedsUpdate))
				{
					//DevMsg("Item is ready to do shit.\n");

					uint32 unTimeStamp;
					uint64 unSizeOnDisk;
					char installFolder[AA_MAX_STRING];
					unsigned int unFolderSize = AA_MAX_STRING;

					if (!steamapicontext->SteamUGC()->GetItemInstallInfo(pDetails->m_nPublishedFileId, &unSizeOnDisk, installFolder, sizeof(installFolder), &unTimeStamp))
					{
						Msg("Workshop Manager: Error - Failed to get install information about the workshop item about to be consumed.\n");
						continue;
					}

					DevMsg("Workshop Manager: The following workshop item is about to be consumed:\n");
					DevMsg("\tID: %llu\n", pDetails->m_nPublishedFileId);
					DevMsg("\tSize On Disk: %u\n", unSizeOnDisk);
					DevMsg("\tFolder: %s\n", installFolder);
					DevMsg("\tLegacy Item: %i\n", (iStateFlags & k_EItemStateLegacyItem));
				}
				*/
			}
		}

		if (previousNum + pResult->m_unNumResultsReturned < pResult->m_unTotalMatchingResults)
		{
			m_uPageNum++;

			AccountID_t aid = steamapicontext->SteamUser()->GetSteamID().GetAccountID();
			UGCQueryHandle_t hUGCQuery = steamapicontext->SteamUGC()->CreateQueryUserUGCRequest(aid, k_EUserUGCList_Subscribed, k_EUGCMatchingUGCType_Items, k_EUserUGCListSortOrder_SubscriptionDateDesc, engine->GetAppID(), engine->GetAppID(), m_uPageNum);

			SteamAPICall_t hAPICall = steamapicontext->SteamUGC()->SendQueryUGCRequest(hUGCQuery);
			callback.Set(hAPICall, this, &C_WorkshopQuery::OnUGCQueried);
		}
		else
			g_pAnarchyManager->GetWorkshopManager()->OnQueryComplete(this);
			//DevMsg("Finsihed querying workshop content!\n");
	}
}

void C_WorkshopManager::MountWorkshop(PublishedFileId_t id, bool& bIsLegacy, unsigned int& uNumItems, unsigned int& uNumModels, SteamUGCDetails_t* pDetails)
{
	SteamUGCDetails_t* details = null;
	if (pDetails)
		details = pDetails;
	else
	{
		auto foundMount = m_details.find(id);
		if (foundMount != m_details.end())
			details = m_details[id];
	}
	
	if (!details)
	{
		this->MountNextWorkshop();
		return;
	}

	unsigned int iStateFlags = steamapicontext->SteamUGC()->GetItemState(details->m_nPublishedFileId);
	if (iStateFlags & k_EItemStateSubscribed && iStateFlags & k_EItemStateInstalled && !(iStateFlags & k_EItemStateNeedsUpdate))
	{
		//DevMsg("Item is ready to do shit.\n");

		uint32 unTimeStamp;
		uint64 unSizeOnDisk;
		char installFolder[AA_MAX_STRING];
		unsigned int unFolderSize = AA_MAX_STRING;

		if (!steamapicontext->SteamUGC()->GetItemInstallInfo(details->m_nPublishedFileId, &unSizeOnDisk, installFolder, sizeof(installFolder), &unTimeStamp))
		{
			Msg("Workshop Manager: Error - Failed to get install information about the workshop item about to be consumed.\n");
			this->MountNextWorkshop();
			//this->OnMountWorkshopFail();
			return;
		}

//		DevMsg("Workshop Manager: The following workshop item is about to be consumed:\n");
//		DevMsg("\tID: %llu\n", details->m_nPublishedFileId);
//		DevMsg("\tSize On Disk: %u\n", unSizeOnDisk);
//		DevMsg("\tFolder: %s\n", installFolder);
//		DevMsg("\tLegacy Item: %i\n", (iStateFlags & k_EItemStateLegacyItem));

		if ((iStateFlags & k_EItemStateLegacyItem))
		{
			bIsLegacy = true;
			g_pAnarchyManager->GetWebManager()->GetHudWebTab()->AddHudLoadingMessage("progress", "", "Skipping Legacy Workshop Subscriptions", "skiplegacyworkshops", "", "", "+");
			g_pAnarchyManager->GetWebManager()->GetHudWebTab()->AddHudLoadingMessage("progress", "", "Mounting Workshop Subscriptions", "mountworkshops", "", std::string(VarArgs("%u", m_details.size())), "+");
			// mount legacy
			//this->MountNextWorkshop();
			//return;
		}	
		else
		{
			bIsLegacy = false;
			// mount non-legacy

			/* NOTE: The [id].txt file contains a list of every file in the archive, but this info can be derrived from the folder path anyways.
			std::string file = VarArgs("%s/resource/workshop/%llu.txt", installFolder, details->m_nPublishedFileId);
			if (!g_pFullFileSystem->FileExists(file.c_str(), ""))
			{
				DevMsg("WorkshopManager: Error - Workshop info file does not exist for installed workshop item with ID %llu\n", details->m_nPublishedFileId);
				return false;
			}
			else
			{
				KeyValues* pWorkshopInfoKv = new KeyValues("workshop");
				if (!pWorkshopInfoKv->LoadFromFile(g_pFullFileSystem, file.c_str(), ""))
				{
					DevMsg("WorkshopManager: Failed to load file.\n");
					pWorkshopInfoKv->deleteThis();
					return false;
				}
				else
				{
					DevMsg("\tWorkshop Thing: %s\n", pWorkshopInfoKv->GetString("title"));
					pWorkshopInfoKv->deleteThis();
					return true;
				}
			}
			*/

			//g_pFullFileSystem->AddSearchPath(installFolder, "MOD", PATH_ADD_TO_TAIL);
			g_pFullFileSystem->AddSearchPath(installFolder, "GAME", PATH_ADD_TO_TAIL);

			std::string fullPath = VarArgs("%s\\", installFolder);
			
			// FIXME: Actually check that this IS a legacy workshop item!! (generation < 3)
			std::string id = VarArgs("%llu", details->m_nPublishedFileId);

			// detect any .set files
			std::string file;
			KeyValues* kv = new KeyValues("instance");
			FileFindHandle_t findHandle;
			VarArgs("Tester folder: %s\\maps\\*.set", installFolder);
			const char *pFilename = g_pFullFileSystem->FindFirst(VarArgs("%s\\maps\\*.set", installFolder), &findHandle);
			while (pFilename != NULL)
			{
				if (g_pFullFileSystem->FindIsDirectory(findHandle))
				{
					pFilename = g_pFullFileSystem->FindNext(findHandle);
					continue;
				}

				file = std::string(installFolder) + "\\maps\\" + std::string(pFilename);

				// FIXME: build an ACTUAL generation 3 instance key values here, and save it out!!
				if (kv->LoadFromFile(g_pFullFileSystem, file.c_str()))
				{
					if (kv->FindKey("map") && kv->FindKey("objects", true)->GetFirstSubKey())
					{
						DevMsg("Map ID here is: %s\n", kv->GetString("map"));
						// FIXME: instance_t's should have mapId's, not MapNames.  The "mapName" should be considered the title.  The issue is that maps usually haven't been detected by this point, so assigning a mapID based on the legacy map name is complex.
						// For now, mapId's will be resolved upon map detection if mapID's equal a detected map's filename.

						std::string title = kv->GetString("title");
						if (title == "")
						{
							// attempt to load a .txt file from the legacy workshop addon that has the title
							FileFindHandle_t infoFindHandle;
							const char *pInfoFilename = g_pFullFileSystem->FindFirst(VarArgs("%s\\resource\\workshop\\*.txt", installFolder), &infoFindHandle);
							if (pInfoFilename)
							{
								KeyValues* infoKv = new KeyValues("info");
								if (infoKv->LoadFromFile(g_pFullFileSystem, VarArgs("%s\\resource\\workshop\\%s", installFolder, pInfoFilename)))
									title = infoKv->GetString("title");
								infoKv->deleteThis();
							}
							g_pFullFileSystem->FindClose(infoFindHandle);
						}

						if (title == "")
							title = "Unnamed";

						g_pAnarchyManager->GetInstanceManager()->AddInstance(g_pAnarchyManager->GenerateUniqueId(), kv->GetString("map"), title, file, VarArgs("%llu", details->m_nPublishedFileId), "");
					}
				}

				pFilename = g_pFullFileSystem->FindNext(findHandle);
			}
			g_pFullFileSystem->FindClose(findHandle);
			kv->Clear();

			// now start loading the items from it
			g_pAnarchyManager->GetMetaverseManager()->LoadFirstLocalItemLegacy(true, fullPath, id, "");
			return;
		}
	}

	this->MountNextWorkshop();
	return;
}

void C_WorkshopManager::OnMountWorkshopFail()
{
	// before 
	//this->OnMountWorkshopSucceed();
	//this->MountWorkshopClose();
	g_pAnarchyManager->GetMetaverseManager()->OnMountAllWorkshopsCompleted();
}

void C_WorkshopManager::OnMountWorkshopSucceed()
{
	g_pAnarchyManager->GetMetaverseManager()->ResolveLoadLocalItemLegacyBuffer();

	if (g_pAnarchyManager->GetMetaverseManager()->GetPreviousLocaLocalItemLegacyWorkshopIds() != "")
	{
		// DON'T CALL THIS UNTIL AFTER MAPS ARE LOADED TOO!! (only items have been loaded so far)
		// wellll, maybe its OK to to call it now.  maps should be loaded LAST, after everything else is already in.
		g_pAnarchyManager->GetWebManager()->GetHudWebTab()->AddHudLoadingMessage("progress", "", "Mounting Workshop Subscriptions", "mountworkshops", "", std::string(VarArgs("%u", m_details.size())), "+", "mountNextWorkshopCallback");
	}
	else
	{
		//g_pAnarchyManager->GetWebManager()->GetHudWebTab()->AddHudLoadingMessage("progress", "", "Importing Old AArcade Data", "importfolder", "0", "1", "1");
//		g_pAnarchyManager->GetMetaverseManager()->ResolveLoadLocalItemLegacyBuffer();

		g_pAnarchyManager->OnMountAllWorkshopsComplete();

//		g_pAnarchyManager->GetMetaverseManager()->DetectAllMaps();
//		g_pAnarchyManager->GetMetaverseManager()->DetectAllMapScreenshots();
//		g_pAnarchyManager->OnMountAllWorkshopsComplete();
	}
}

void C_WorkshopManager::AddWorkshopDetails(SteamUGCDetails_t* pDetails)
{
	m_details[pDetails->m_nPublishedFileId] = pDetails;
}

/*
void C_WorkshopManager::MountAllWorkshops()
{
	unsigned int count = 0;
	unsigned int legacyCount = 0;
	bool bIsLegacy;
	unsigned int uNumItemsTotal = 0;
	unsigned int uNumLegacyItemsTotal = 0;
	unsigned int uNumModelsTotal = 0;
	unsigned int uNumLegacyModelsTotal = 0;
	for (std::map<PublishedFileId_t, SteamUGCDetails_t*>::iterator it = m_details.begin(); it != m_details.end(); ++it)
	{
		unsigned int uNumItems, uNumModels;
		if (this->MountWorkshop(it->first, bIsLegacy, uNumItems, uNumModels, it->second))
		{
			if (bIsLegacy)
			{
				legacyCount++;
				uNumLegacyItemsTotal += uNumItems;
				uNumLegacyModelsTotal += uNumModels;
			}
			else
			{
				count++;
				uNumItemsTotal += uNumItems;
				uNumModelsTotal += uNumModels;
			}
		}
		else
			DevMsg("WorkshopManager: Failed to mount workshop addon!\n");
	}

	std::string max = VarArgs("%u", count);
	std::string min = "0";
	std::string current = VarArgs("%u", count);
	//g_pAnarchyManager->GetLoadingManager()->AddMessage("progress", "", "Mounting Workshop Subscriptions", "mountworkshops", min, current, max);

	max = VarArgs("%u", uNumModelsTotal);
	min = "0";
	current = VarArgs("%u", uNumModelsTotal);
	g_pAnarchyManager->GetWebManager()->GetHudWebTab()->AddHudLoadingMessage("progress", "", "Loading Workshop Models", "workshoplibrarymodels", min, current, max);

	max = VarArgs("%u", uNumItemsTotal);
	min = "0";
	current = VarArgs("%u", uNumItemsTotal);
	g_pAnarchyManager->GetWebManager()->GetHudWebTab()->AddHudLoadingMessage("progress", "", "Loading Workshop Items", "workshoplibraryitems", min, current, max);

	max = VarArgs("%u", legacyCount);
	min = "0";
	current = VarArgs("%u", legacyCount);
	g_pAnarchyManager->GetWebManager()->GetHudWebTab()->AddHudLoadingMessage("progress", "", "Skipping Legacy Workshop Subscriptions", "mountlegacyworkshops", min, current, max);

	g_pAnarchyManager->OnMountAllWorkshopsComplete();
}
*/

void C_WorkshopManager::MountFirstWorkshop()
{
	// start the search
	m_bMountWorkshopIsLegacy = false;
	m_uMountWorkshopuNumItems = 0;
	m_uMountWorkshopNumModels = 0;

	m_previousMountWorkshopIterator = m_details.begin();
	if (m_previousMountWorkshopIterator != m_details.end())
		this->MountWorkshop(m_previousMountWorkshopIterator->first, m_bMountWorkshopIsLegacy, m_uMountWorkshopuNumItems, m_uMountWorkshopNumModels, m_previousMountWorkshopIterator->second);
	else
	{
		this->OnMountWorkshopFail();
		return;
	}

	return;
}

void C_WorkshopManager::MountNextWorkshop()
{
//	m_uMountWorkshopuNumItems
//		m_uMountWorkshopuNumModels

	// continue the search
	m_previousMountWorkshopIterator++;
	if (m_previousMountWorkshopIterator != m_details.end())
		this->MountWorkshop(m_previousMountWorkshopIterator->first, m_bMountWorkshopIsLegacy, m_uMountWorkshopuNumItems, m_uMountWorkshopNumModels, m_previousMountWorkshopIterator->second);
	else
		this->OnMountWorkshopFail();

	/*
	// continue the search
	unsigned int count = 0;
	unsigned int legacyCount = 0;
	bool bIsLegacy;
	unsigned int uNumItemsTotal = 0;
	unsigned int uNumLegacyItemsTotal = 0;
	unsigned int uNumModelsTotal = 0;
	unsigned int uNumLegacyModelsTotal = 0;

	m_previousMountWorkshopIterator++;
	if (m_previousMountWorkshopIterator != m_details.end())
	{
		unsigned int uNumItems, uNumModels;
		if (this->MountWorkshop(m_previousMountWorkshopIterator->first, bIsLegacy, uNumItems, uNumModels, m_previousMountWorkshopIterator->second))
		{
			if (bIsLegacy)
			{
				legacyCount++;
				uNumLegacyItemsTotal += uNumItems;
				uNumLegacyModelsTotal += uNumModels;
			}
			else
			{
				count++;
				uNumItemsTotal += uNumItems;
				uNumModelsTotal += uNumModels;
			}
		}
		else
		{
			DevMsg("WorkshopManager: Failed to mount workshop addon!\n");
			return false;
		}
	}
	else
		return false;

	return true;
	*/
}

void C_WorkshopManager::MountWorkshopClose()
{
	//g_pAnarchyManager->OnMountedAllWorkshop();
}

unsigned int C_WorkshopManager::GetNumDetails()
{
	return m_details.size();
}
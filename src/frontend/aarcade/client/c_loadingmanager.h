#ifndef C_LOADING_MANAGER_H
#define C_LOADING_MANAGER_H

//#include "aa_globals.h"
#include <string>
#include <vector>
#include "c_webtab.h"
//#include "c_webmanager.h"

struct LoadingMessage_t
{
	std::string name;
	std::vector<std::string> arguments;
	//std::string text;
};

class C_LoadingManager
{
public:
	C_LoadingManager();
	~C_LoadingManager();

	//void Init();
	void Reset();

	void SetHudHeader(std::string text);
	void AddHudLoadingMessage(std::string type, std::string text, std::string title = "", std::string id = "", std::string min = "", std::string max = "", std::string current = "");
	void CreateWebTab();
	void OnWebTabReady();
	void DispatchMethods();
	void ClearMethods();

	// accessors

	// mutators
	
private:
	C_WebTab* m_pWebTab;
	std::vector<LoadingMessage_t*> m_loadingMessages;
};

#endif
#ifndef C_ANARCHY_MANAGER_H
#define C_ANARCHY_MANAGER_H

#include <KeyValues.h>
#include "c_webmanager.h"

class C_AnarchyManager : public CAutoGameSystemPerFrame
{
public:
	C_AnarchyManager();
	~C_AnarchyManager();

	virtual bool Init();
	virtual void PostInit();
	virtual void Shutdown();

	// Level init, shutdown
	virtual void LevelInitPreEntity();
	virtual void LevelInitPostEntity();
	virtual void LevelShutdownPreClearSteamAPIContext();
	virtual void LevelShutdownPreEntity();
	virtual void LevelShutdownPostEntity();

	virtual void OnSave();
	virtual void OnRestore();
	virtual void SafeRemoveIfDesired();

	virtual bool	IsPerFrame();

	// Called before rendering
	virtual void PreRender();

	// Gets called each frame
	virtual void Update(float frametime);

	// Called after rendering
	virtual void PostRender();

//	C_WebViewManager* GetWebViewManager() { return m_pWebViewManager; };

	std::string GenerateUniqueId();

	void AnarchyBegin();
	C_WebManager* GetWebManager() { return m_pWebManager; }

	//const char* GenerateHash(const char* text);
	
private:
	C_WebManager* m_pWebManager;
};

//C_AnarchyManager* g_pAnarchyManager = null;
extern C_AnarchyManager* g_pAnarchyManager;

#endif
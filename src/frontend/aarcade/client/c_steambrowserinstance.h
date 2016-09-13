#ifndef C_STEAM_BROWSER_INSTANCE_H
#define C_STEAM_BROWSER_INSTANCE_H

//#include "..\..\public\steam\isteamhtmlsurface.h"
#include "../../public/steam/steam_api.h"
#include "c_inputlistenersteambrowser.h"
#include "c_embeddedinstance.h"
#include <string>
#include "vgui_controls/Controls.h"
//#include <vector>

class C_SteamBrowserInstance : public C_EmbeddedInstance
{
public:
	C_SteamBrowserInstance();
	~C_SteamBrowserInstance();

	void SelfDestruct();

	void Init(std::string id = "", std::string url = "", const char* pchPostData = null);
	CCallResult<C_SteamBrowserInstance, HTML_BrowserReady_t> m_CreateBrowserInstance;
	void OnBrowserInstanceCreated(HTML_BrowserReady_t *pResult, bool bIOFailure);

	std::string GetId() { return m_id; }
	void Update();

	void ResizeFrameFromRGB565(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth);
	void ResizeFrameFromRGB1555(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth);
	void ResizeFrameFromXRGB8888(const void* pSrc, void* pDst, unsigned int sourceWidth, unsigned int sourceHeight, size_t sourcePitch, unsigned int sourceDepth, unsigned int destWidth, unsigned int destHeight, size_t destPitch, unsigned int destDepth);
	void CopyLastFrame(unsigned char* dest, unsigned int width, unsigned int height, size_t pitch, unsigned int depth);

	void OnProxyBind(C_BaseEntity* pBaseEntity);
	void Render();
	void RegenerateTextureBits(ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect);

	bool OnStartRequest(const char *url, const char *target, const char *pchPostData, bool bIsRedirect);
	void OnFinishRequest(const char *url, const char *pageTitle, const CUtlMap < CUtlString, CUtlString > &headers);
	void OnNeedsPaint(const void* data, unsigned int width, unsigned int height, unsigned int depth);
	//void BrowserInstanceNeedsPaint(HTML_NeedsPaint_t *pCallback);

	void OnMouseMove(float x, float y);
	void OnMousePressed(vgui::MouseCode code);
	void OnMouseReleased(vgui::MouseCode code);

	// accessors
	ITexture* GetTexture() { return m_pTexture; }
	int GetLastRenderedFrame() { return m_iLastRenderedFrame; }
	C_InputListener* GetInputListener();

	// mutators	

private:
	STEAM_CALLBACK(C_SteamBrowserInstance, BrowserInstanceStartRequest, HTML_StartRequest_t, m_StartRequest);
	STEAM_CALLBACK(C_SteamBrowserInstance, BrowserInstanceFinishedRequest, HTML_FinishedRequest_t, m_FinishedRequest);
	STEAM_CALLBACK(C_SteamBrowserInstance, BrowserInstanceNeedsPaint, HTML_NeedsPaint_t, m_NeedsPaint);
	void* m_pLastFrameData;
	bool m_bReadyForNextFrame;
	bool m_bCopyingFrame;
	bool m_bReadyToCopyFrame;
	ITexture* m_pTexture;
	int m_iLastRenderedFrame;
	HHTMLBrowser m_unBrowserHandle;
	std::string m_id;
	std::string m_initialURL;
	void* m_pPostData;
	bool m_bIsDirty;
};

#endif
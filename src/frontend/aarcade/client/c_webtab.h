#ifndef C_WEB_TAB_H
#define C_WEB_TAB_H

#include <Awesomium/WebCore.h>
#include <Awesomium/STLHelpers.h>
#include "vgui/MouseCode.h"
#include "c_awesomiumjshandlers.h"
//#include "vgui_controls/Controls.h"
#include <vector>
//#include "c_webmanager.h"

/*
struct JavaScriptMethodCall_t
{
	std::string objectName;
	std::string methodName;
	std::vector<std::string> methodArguments;
};
*/

class C_WebTab
{
public:
	C_WebTab(std::string url, std::string id, bool bAlpha = false);
	~C_WebTab();

	// accessors
	std::string GetId() { return m_id; }
	std::string GetInitialUrl() { return m_initialUrl; }
	int GetLastRenderedFrame() { return m_iLastRenderFrame; }
	ITexture* GetTexture() { return m_pTexture; }
	int GetState() { return m_iState; }

	// mutators
	void SetState(int state) { m_iState = state; }
	//void SetReadyForNextSimpleImage(bool state) { m_iNumImagesLoading = state; }
	void DecrementNumImagesLoading() { m_iNumImagesLoading--; };
	void SetNumImagesLoading(int num) { m_iNumImagesLoading = num; }

	void OnProxyBind(C_BaseEntity* pBaseEntity = null);
	void Render();
	void RegenerateTextureBits(ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect);
	void GetMousePos(float &fMouseX, float &fMouseY);
	void MouseMove(float fMouseX, float fMouseY);
	void MouseWheel(int delta);
	void MousePress(vgui::MouseCode code);
	void MouseRelease(vgui::MouseCode code);
	void KeyCodePress(vgui::MouseCode code, bool bShiftState, bool bCtrlState, bool bAltState);
	void KeyCodeRelease(vgui::MouseCode code, bool bShiftState, bool bCtrlState, bool bAltState);

	void SetUrl(std::string);

	bool RequestLoadSimpleImage(std::string channel, std::string itemId);	// images web-views only!
	void OnSimpleImageReady(std::string channel, std::string itemId, std::string field, ITexture* pTexture);	// images web-views only!

	void DispatchJavaScriptMethod(std::string objectName, std::string objectMethod, std::vector<std::string> methodArguments);
	void DispatchJavaScriptMethodCalls();
	//void DispatchJavaScriptEventMessages(std::string objectName, std::string objectMethod, std::vector<JSEventMessage_t*> eventArgs);
	//void DispatchJavaScriptMethod(std::string objectName, std::string objectMethod, std::vector<std::string> methodArguments);
	//void DispatchJavaScriptMethodBatch(C_WebTab* pWebTab, std::vector<MethodBatch_t*> batch);

	// hud webtab only
	void SetHudTitle(std::string title);
	void AddHudLoadingMessage(std::string type, std::string text, std::string title = "", std::string id = "", std::string min = "", std::string max = "", std::string current = "", std::string callbackMethod = "");
	std::vector<JavaScriptMethodCall_t*> GetJavaScriptMethodCalls() { return m_javaScriptMethodCalls; }

private:
//	bool m_bReadyForNextSimpleImage;
	int m_iNumImagesLoading;
	int m_iMaxImagesLoading;
	std::vector<JavaScriptMethodCall_t*> m_javaScriptMethodCalls;
	float m_fMouseX;
	float m_fMouseY;
	int m_iState;
	std::string m_id;
	std::string m_initialUrl;
	ITexture* m_pTexture;
	int m_iLastRenderFrame;
};

#endif
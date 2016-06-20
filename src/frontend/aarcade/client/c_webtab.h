#ifndef C_WEB_TAB_H
#define C_WEB_TAB_H

#include <Awesomium/WebCore.h>
#include <Awesomium/STLHelpers.h>
#include "vgui/MouseCode.h"
//#include "vgui_controls/Controls.h"
#include <vector>
//#include "c_webmanager.h"

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

	void OnProxyBind(C_BaseEntity* pBaseEntity = null);
	void Render();
	void RegenerateTextureBits(ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect);
	void GetMousePos(float &fMouseX, float &fMouseY);
	void MouseMove(float fMouseX, float fMouseY);
	void MousePress(vgui::MouseCode code);
	void MouseRelease(vgui::MouseCode code);
	void KeyCodePress(vgui::MouseCode code, bool bShiftState, bool bCtrlState, bool bAltState);
	void KeyCodeRelease(vgui::MouseCode code, bool bShiftState, bool bCtrlState, bool bAltState);

	void SetUrl(std::string);

	//void DispatchJavaScriptEventMessages(std::string objectName, std::string objectMethod, std::vector<JSEventMessage_t*> eventArgs);
	void DispatchJavaScriptMethod(std::string objectName, std::string objectMethod, std::vector<std::string> methodArguments);
	//void DispatchJavaScriptMethodBatch(C_WebTab* pWebTab, std::vector<MethodBatch_t*> batch);

private:
	float m_fMouseX;
	float m_fMouseY;
	int m_iState;
	std::string m_id;
	std::string m_initialUrl;
	ITexture* m_pTexture;
	int m_iLastRenderFrame;
};

#endif
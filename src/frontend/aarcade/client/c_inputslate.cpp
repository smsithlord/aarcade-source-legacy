#include "cbase.h"
#include <vgui_controls/Panel.h>

#include "c_inputslate.h"
#include "c_anarchymanager.h"

#include "vgui/IVGUI.h"
#include "vgui/IInput.h"

#include "vgui_controls/ImagePanel.h"
#include <vgui/ISurface.h>
#include "materialsystem/IMaterialVar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

long CInputSlate::m_fPreviousTime = 0;

//ITexture* CInputSlate::s_pOriginalTexture = null;
//IMaterial* CInputSlate::s_pMaterial = null;
CInputSlate::CInputSlate(vgui::VPANEL parent) : Frame(null, "InputSlate")
{

	DevMsg("Input slate created\n");
	SetParent( parent );

	// automatically pin the input slate open if it was just opened a moment ago (double-tap to stick functionality)
	long currentTime = system()->GetTimeMillis();
	if (m_fPreviousTime > 0 && currentTime - m_fPreviousTime < 200)
		g_pAnarchyManager->GetInputManager()->ForceInputMode();

	m_bCursorAlphaZero = false;
	m_bCursorHidden = false;

//	m_pWebViewManager = C_AnarchyManager::GetSelf()->GetWebViewManager();

	m_pOriginalTexture = null;
	m_pMaterial = null;

	m_bOverlay = g_pAnarchyManager->GetInputManager()->GetOverlayMode();

	if (!m_bOverlay)
	{
		SetKeyBoardInputEnabled(true);
		SetMouseInputEnabled(true);
	}
	else
	{
		SetKeyBoardInputEnabled(false);
		SetMouseInputEnabled(false);
	}

	m_pCanvasTexture = null;
	m_bFullscreen = g_pAnarchyManager->GetInputManager()->GetFullscreenMode();
	if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance())
		m_pCanvasTexture = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance()->GetTexture();//g_pAnarchyManager->GetInputManager()->GetInputCanvasTexture();

	SetProportional( false );
	SetTitleBarVisible( false );
	SetMinimizeButtonVisible( false );
	SetMaximizeButtonVisible( false );
	SetCloseButtonVisible( false );
	SetSizeable( false );
	SetMoveable( false );

	//SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/sourcescheme.res", "SourceScheme"));
	//LoadControlSettings("resource/ui/inputslate.res");
	SetWide(ScreenWidth());
	SetTall(ScreenHeight());
	SetPaintBackgroundEnabled(false);

	// Hide all children of this panel (including the invisible title bar that steals mouse move input
	for(int i=0; i<GetChildCount(); i++)
	{
		Panel* pPanel = GetChild(i);
		pPanel->SetVisible(false);
	}

	//if (m_pCanvasTexture)
	if (g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance())
	{
		m_pMaterial = g_pMaterialSystem->FindMaterial("vgui/activecanvas", TEXTURE_GROUP_VGUI);
		DevMsg("Found 1: %i\n", m_pMaterial->IsErrorMaterial());

		bool found;
		IMaterialVar* pMaterialVar = m_pMaterial->FindVar("$basetexture", &found, false);
		if (!pMaterialVar || !pMaterialVar->IsDefined() || !pMaterialVar->IsTexture())
			DevMsg("ERROR: Material not found!!\n");

		m_pOriginalTexture = pMaterialVar->GetTextureValue();

		if (m_pCanvasTexture )
			pMaterialVar->SetTextureValue(m_pCanvasTexture);

		if (g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud") != g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance())
		{
			ImagePanel* pImagePanel = new ImagePanel(this, "active_canvas_panel");
			pImagePanel->DisableMouseInputForThisPanel(true);	// prevents the mouse lag
			pImagePanel->SetShouldScaleImage(true);
			pImagePanel->SetSize(GetWide(), GetTall());
			//pImagePanel->SetAutoResize() //pin
			
			pImagePanel->SetImage("activecanvas");
			DevMsg("INPUT SLATE: active_canvas_panel - activecanvas\n");
		}
		
		ITexture* pTexture = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud")->GetTexture();
		ImagePanel* pHudImagePanel = new ImagePanel(this, "hud_canvas_panel");
		pHudImagePanel->DisableMouseInputForThisPanel(true);	// prevents the mouse lag
		pHudImagePanel->SetShouldScaleImage(true);
		pHudImagePanel->SetSize(GetWide(), GetTall());
		//pImagePanel->SetAutoResize() //pin
		pHudImagePanel->SetImage("hudcanvas");
		DevMsg("INPUT SLATE: hud_canvas_panel - hudcanvas\n");

		/*
		ITexture* pTexture = g_pAnarchyManager->GetInputManager()->GetInputCanvasTexture();
		if (!pTexture)
		DevMsg("Texture is not ready yet!!\n");

		ImagePanel* pImagePanel = new ImagePanel(this, "canvas_panel");
		pImagePanel->DisableMouseInputForThisPanel(true);	// prevents the mouse lag
		pImagePanel->SetShouldScaleImage(true);
		pImagePanel->SetSize(GetWide(), GetTall());
		pImagePanel->SetImage("activecanvas");
		*/
	}

	//if (g_pAnarchyManager->GetWebManager()->GetSelectedWebTab() && g_pAnarchyManager->GetInputManager()->GetFullscreenMode())
	//if (g_pAnarchyManager->GetInputManager()->GetFullscreenMode())
	/*
	if (g_pAnarchyManager->GetWebManager()->GetSelectedWebTab() )//&& g_pAnarchyManager->GetWebManager()->GetSelectedWebTab() != g_pAnarchyManager->GetWebManager()->GetHudWebTab())//&& m_bFullscreen)
	{
		m_pMaterial = g_pMaterialSystem->FindMaterial("vgui/selectedwebtab", TEXTURE_GROUP_VGUI);
		ITexture* pTexture = g_pAnarchyManager->GetWebManager()->GetSelectedWebTab()->GetTexture();
		if (!pTexture)
			DevMsg("Texture is not ready yet!!\n");

		bool found;
		IMaterialVar* pMaterialVar = m_pMaterial->FindVar("$basetexture", &found, false);
		if (!pMaterialVar)
			DevMsg("ERROR: Material not found!!\n");

		m_pOriginalTexture = pMaterialVar->GetTextureValue();

		if (pTexture)
			pMaterialVar->SetTextureValue(pTexture);

		if (g_pAnarchyManager->GetWebManager()->GetSelectedWebTab() != g_pAnarchyManager->GetWebManager()->GetHudWebTab())
		{
			ImagePanel* pImagePanel = new ImagePanel(this, "selected_webtab_panel");
			pImagePanel->DisableMouseInputForThisPanel(true);	// prevents the mouse lag
			pImagePanel->SetShouldScaleImage(true);
			pImagePanel->SetSize(GetWide(), GetTall());
			//pImagePanel->SetAutoResize() //pin
			pImagePanel->SetImage("selectedwebtab");
		}

		ImagePanel* pImagePanel = new ImagePanel(this, "hud_webtab_panel");
		pImagePanel->DisableMouseInputForThisPanel(true);	// prevents the mouse lag
		pImagePanel->SetShouldScaleImage(true);
		pImagePanel->SetSize(GetWide(), GetTall());
		//pImagePanel->SetAutoResize() //pin
		pImagePanel->SetImage("hudwebtab");
	}
//	else
*/
	if ( !m_bFullscreen )
	{
		m_bCursorHidden = true;

		if (!m_bOverlay)
			ShowCursor(false);
	}

	// for opacity
	ivgui()->AddTickSignal(this->GetVPanel());

	SetVisible(true);
	Activate();

	//ShowCursor(true);
}

/*
void CInputSlate::OnTick()
{
	if( GetAlpha() > 0 )
		SetAlpha(0);	// This won't actually change anything unless the panel has finished being created
	else
		ivgui()->RemoveTickSignal(this->GetVPanel());
}
*/

void CInputSlate::SelfDestruct()
{
	bool found;
	IMaterialVar* pMaterialVar = m_pMaterial->FindVar("$basetexture", &found, false);
	if (!found || !pMaterialVar || !pMaterialVar->IsDefined() || !pMaterialVar->IsTexture())
		DevMsg("ERROR: Material not found 222222222222222222222222222222222222222!!\n");
	else if ( m_pOriginalTexture )
		pMaterialVar->SetTextureValue(m_pOriginalTexture);
}

vgui::Panel* CInputSlate::GetPanel()
{
	return this;
}

void CInputSlate::OnTick()
{
	if (g_pAnarchyManager->IsPaused())
		return;

	bool bFullscreen = g_pAnarchyManager->GetInputManager()->GetFullscreenMode();
	if (!bFullscreen && !m_bCursorAlphaZero)
	{
		if (GetAlpha() > 0)
			SetAlpha(0);
		else
		{
			DevMsg("hiding cursor\n");
			m_bCursorAlphaZero = true;

			if (!m_bCursorHidden)
			{
				m_bCursorHidden = true;
				if( !m_bOverlay )
					ShowCursor(false);
			}
		}
	}
	else if (bFullscreen && m_bCursorAlphaZero)
	{
		DevMsg("showing cursor\n");
		SetAlpha(255);
		m_bCursorAlphaZero = false;
		if (m_bCursorHidden)
		{
			m_bCursorHidden = false;
			if (!m_bOverlay)
				ShowCursor(true);
		}
	}
	//else
		//ivgui()->RemoveTickSignal(this->GetVPanel());
}

void CInputSlate::OnMouseWheeled(int delta)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	//DevMsg("Disbaled for testing!\n");
	//return;
	g_pAnarchyManager->GetInputManager()->OnMouseWheeled(delta);
}

void CInputSlate::OnCursorMoved(int x, int y)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	/*
	int pX, pY;
	int pWidth, pHeight;

	m_pImagePanel->GetPos(pX, pY);
	m_pImagePanel->GetSize(pWidth, pHeight);

	mouseX = x - pX;
	mouseY = y - pY;

	mouseX = (mouseX / (pWidth + 0.0))*m_iWidth;
	mouseY = (mouseY / (pHeight + 0.0))*m_iHeight;

	if (mouseX < 0 || mouseX > m_iWidth || mouseY < 0 || mouseY > m_iHeight)
		m_bMouseIsOnScreen = false;
	*/

	g_pAnarchyManager->GetInputManager()->MouseMove(x / (GetWide() * 1.0), y / (GetTall() * 1.0));
}

void CInputSlate::OnMouseDoublePressed(MouseCode code)
{
	if (g_pAnarchyManager->IsPaused())
		return;

//	g_pAnarchyManager->GetInputManager()->MousePress(code);
	g_pAnarchyManager->GetInputManager()->MousePress(code);
}

void CInputSlate::OnMousePressed(MouseCode code)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	/*
	if (g_pAnarchyManager->IsPaused())
	{
		g_pAnarchyManager->Unpause();
		return;
	}
	*/

	g_pAnarchyManager->GetInputManager()->MousePress(code);
}

void CInputSlate::OnMouseReleased(MouseCode code)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	g_pAnarchyManager->GetInputManager()->MouseRelease(code);
}

void CInputSlate::OnKeyCodePressed(KeyCode code)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	g_pAnarchyManager->GetInputManager()->KeyCodePressed(code, (input()->IsKeyDown(KEY_LSHIFT) || input()->IsKeyDown(KEY_RSHIFT)), (input()->IsKeyDown(KEY_LCONTROL) || input()->IsKeyDown(KEY_RCONTROL)), (input()->IsKeyDown(KEY_LALT) || input()->IsKeyDown(KEY_RALT)));
}

void CInputSlate::SetFullscreenMode(bool bFullscreenMode)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	DevMsg("Fullscreen transition canceled!\n");
	return;

	if (m_bFullscreen != bFullscreenMode)
	{
		if (bFullscreenMode)
		{
			// change us from non-fullscreen to fullscreen
		}
		else
		{
			// change us from fullscreen to non-fullscreen
		}

		m_bFullscreen = bFullscreenMode;
	}

}

// wtf does this function even exist for. fuck it and remove it asap.  i created it when adding overlay mode overwhere that fullscreen mode was at in the code.....
void CInputSlate::SetOverlayMode(bool bOverlayMode)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	DevMsg("Fullscreen transition canceled!\n");
	return;

	if (m_bOverlay != bOverlayMode)
	{
		if (bOverlayMode)
		{
			// change us from non-fullscreen to fullscreen
		}
		else
		{
			// change us from fullscreen to non-fullscreen
		}

		m_bOverlay = bOverlayMode;
	}

}

void CInputSlate::OnKeyCodeReleased(KeyCode code)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	//if (code == KEY_ESCAPE && g_pAnarchyManager->IsPaused())
	//{
		//g_pAnarchyManager->Unpause();
		//return;
	//}

	g_pAnarchyManager->GetInputManager()->KeyCodeReleased(code, (input()->IsKeyDown(KEY_LSHIFT) || input()->IsKeyDown(KEY_RSHIFT)), (input()->IsKeyDown(KEY_LCONTROL) || input()->IsKeyDown(KEY_RCONTROL)), (input()->IsKeyDown(KEY_LALT) || input()->IsKeyDown(KEY_RALT)));
	/*
	using namespace Awesomium;

	if( !web_view )
		return;

	WebView* pWebView = static_cast<WebView*>(web_view);

	WebKeyboardEvent pWebKeyboardEvent;
	pWebKeyboardEvent.type = WebKeyboardEvent::kTypeKeyUp;

	char* buf = new char[20];
    pWebKeyboardEvent.virtual_key_code = 0x42;
    GetKeyIdentifierFromVirtualKeyCode(pWebKeyboardEvent.virtual_key_code, &buf);
    strcpy(pWebKeyboardEvent.key_identifier, buf);
    delete[] buf;

	pWebKeyboardEvent.modifiers = 0;

	pWebKeyboardEvent.native_key_code = pWebKeyboardEvent.virtual_key_code;

	pWebView->InjectKeyboardEvent(pWebKeyboardEvent);
	*/
}

/*
void CHDViewInput::OnTick()
{
	// turn us invisible if the button isn't held down
	if( this->IsVisible() )
	{
		if( !m_pVisibleConVar->GetBool() )
		{
			C_ArcadeResources* pClientArcadeResources = C_ArcadeResources::GetSelf();
			CAwesomiumRegen* pRegen = pClientArcadeResources->GetHDViewRegen();
			pRegen->SetMouse(m_iWidth/2, m_iHeight/2);

			this->SetVisible(false);
			ShowCursor(true);
			this->SetAlpha(0);
		}
	}
	else
	{
		if( m_pVisibleConVar->GetBool() )
		{
			this->SetVisible(true);
			ShowCursor(false);
			this->SetAlpha(0);
		}
	}

}

*/
void CInputSlate::OnCommand(const char* pcCommand)
{
	if( !Q_stricmp(pcCommand, "Close") )
	{
//		CAwesomium_Input* pAwesomium_Input = static_cast<CAwesomium_Input*>(m_inputPanel);
		//pAwesomium_Input->ForceClose();
		//delete pAwesomium_Input;
		//ShowCursor(true);

		/*
		if (m_bFullscreen)
			ShowCursor(true);
			*/

		BaseClass::OnCommand(pcCommand);
	}
}

CInputSlate::~CInputSlate()
{
	DevMsg("Close input slate\n");
	m_fPreviousTime = system()->GetTimeMillis();
	/*
	if (m_pOriginalTexture)
	{
		bool found;
		IMaterialVar* pMaterialVar = m_pMaterial->FindVar("$basetexture", &found, false);
		if( pMaterialVar )
			pMaterialVar->SetTextureValue(m_pOriginalTexture);
	}
	*/

	this->SelfDestruct();

	if (m_bCursorHidden)
	{
		m_bCursorHidden = false;
		if (!m_bOverlay)
			ShowCursor(true);
		DevMsg("showing cursor\n");
	}

	DevMsg("done closing input slate.\n");
}

class CInputSlateInterface : public IInputSlate
{
private:
	CInputSlate *InputSlate;
public:
	CInputSlateInterface()
	{
		InputSlate = NULL;
	}

	void Create(vgui::VPANEL parent)
	{
		InputSlate = new CInputSlate(parent);
	}

	vgui::Panel* GetPanel()
	{
		return InputSlate->GetPanel();
	}

	void SetFullscreenMode(bool bFullscreenMode)
	{
		InputSlate->SetFullscreenMode(bFullscreenMode);
	}

	void SetOverlayMode(bool bOverlayMode)
	{
		InputSlate->SetOverlayMode(bOverlayMode);
	}

	void Destroy()
	{
		if (InputSlate)
		{
			InputSlate->SetParent((vgui::Panel *)NULL);
			//InputSlate->SelfDestruct();
			delete InputSlate;
		}
	}
};
static CInputSlateInterface g_InputSlate;
IInputSlate* InputSlate = (IInputSlate*)&g_InputSlate;
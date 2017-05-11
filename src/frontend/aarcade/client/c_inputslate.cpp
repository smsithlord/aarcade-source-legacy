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

//long CInputSlate::m_fPreviousTime = 0;

//ITexture* CInputSlate::s_pOriginalTexture = null;
//IMaterial* CInputSlate::s_pMaterial = null;
CInputSlate::CInputSlate(vgui::VPANEL parent) : Frame(null, "InputSlate")
{
	m_pMaterialVar = null;
	DevMsg("Input Slate Created\n");

	// give the input manager a pointer to ourselves
	//g_pAnarchyManager->GetInputManager()->SetInputSlate(this);

	// vgui constructor stuff
	SetParent( parent );
	SetProportional(false);
	SetTitleBarVisible(false);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);
	SetSizeable(false);
	SetMoveable(false);
	SetWide(ScreenWidth());
	SetTall(ScreenHeight());
	SetPaintBackgroundEnabled(false);

	// Hide all children of this panel (including the invisible title bar that steals mouse move input
	for (int i = 0; i<GetChildCount(); i++)
	{
		Panel* pPanel = GetChild(i);
		pPanel->SetVisible(false);
	}

	//vgui::HCursor
	//input()->SetCursorOveride();
	// regular constructor stuff
	m_pOriginalTexture = null;
	m_bReadyForUpdates = false;
	m_bCursorAlphaZero = false;
	m_bCursorHidden = false;
	m_bInputCapture = g_pAnarchyManager->GetInputManager()->GetInputCapture();
	m_bFullscreen = g_pAnarchyManager->GetInputManager()->GetFullscreenMode();

	// the instance that wants input, under the UI layer.
	m_pCanvasInstance = g_pAnarchyManager->GetInputManager()->GetEmbeddedInstance();
	m_pCanvasTexture = (m_pCanvasInstance && m_pCanvasInstance->GetTexture()) ? m_pCanvasInstance->GetTexture() : null;

	// our own material & texture
	m_pMaterial = g_pMaterialSystem->FindMaterial("vgui/activecanvas", TEXTURE_GROUP_VGUI);

	if (!m_pMaterial)
		DevMsg("WARNING: Input Slate's material cannot be found among loaded materials: vgui/activecanvas in TEXTURE_GROUP_VGUI\n");
	else if (m_pMaterial->IsErrorMaterial())
		DevMsg("WARNING: Input Slate's material is the error material instead of: vgui/activecanvas in TEXTURE_GROUP_VGUI\n");

	// find our texture var, and make sure it's valid, otherwise set our texture var to null.
	if (m_pMaterial)
	{
		bool found;
		m_pMaterialVar = m_pMaterial->FindVar("$basetexture", &found, false);

		if (!found || !m_pMaterialVar || !m_pMaterialVar->IsDefined() )//|| !m_pMaterialVar->IsTexture())
		{
			DevMsg("WARNING: FindVar fail:\n\tFound: %i\n\tHasMaterialVar: %i\n\tIsDefined: %i\n\tIsTexture: %i\n", found, (m_pMaterialVar != null), m_pMaterialVar->IsDefined(), m_pMaterialVar->IsTexture());
			m_pMaterialVar = null;
		}

		if (m_pMaterialVar->IsTexture())
			m_pOriginalTexture = (m_pMaterialVar) ? m_pMaterialVar->GetTextureValue() : null;
		else
			DevMsg("WARNING: The active canvas has NO texture loaded onto it, so it does not have an original texture.\n");

		if (m_pOriginalTexture && m_pOriginalTexture->IsError())
		{
			DevMsg("WARNING: The original texture was not loaded and was the error texture.  It was set to null instead.\n");
			m_pOriginalTexture = null;
		}
	}
	else
		m_pMaterialVar = null;

	if (!m_pMaterialVar)
		DevMsg("WARNING: Could not find material var $basetexture of vgui/activecanvas in TEXTURE_GROUP_VGUI\n");

	if (m_pOriginalTexture)
		DevMsg("Input Slate Original Texture: %s\n", m_pOriginalTexture->GetName());

	// should we capture keyboard & mouse input?
	if (m_bInputCapture)
	{
		DevMsg("Capturing keyboard & mouse input.\n");
		SetKeyBoardInputEnabled(true);
		SetMouseInputEnabled(true);
	}
	else
	{
		DevMsg("NOT capturing keyboard & mouse input.\n");
		SetKeyBoardInputEnabled(false);
		SetMouseInputEnabled(false);
	}

	// DISABLED FOR NOW - automatically pin the input slate open if it was just opened a moment ago (double-tap to stick functionality)
	/*
	long currentTime = system()->GetTimeMillis();
	if (m_fPreviousTime > 0 && currentTime - m_fPreviousTime < 200)
	g_pAnarchyManager->GetInputManager()->ForceInputMode();
	*/
	
	// swap the canvas texture in (if we have one)
	if (m_pMaterialVar && m_pCanvasTexture)
		m_pMaterialVar->SetTextureValue(m_pCanvasTexture);

	C_AwesomiumBrowserInstance* pHudInstance = g_pAnarchyManager->GetAwesomiumBrowserManager()->FindAwesomiumBrowserInstance("hud");


	if (!m_pCanvasInstance)
	{
		DevMsg("WARNING: Input Slate has no canvas instance to display.\n");
	}
	else
	{
		// if an underlying instance will be shown under the HUD layer, create it's texture reference on our VGUI panel.
		if (pHudInstance != m_pCanvasInstance)
		{
			DevMsg("Input Slate is adding texture to VGUI panel: %s\n", m_pCanvasTexture->GetName());

			// NOTE: This might also be what changes the vgui/activecanvas texture from the error texture to an actual texture too!!
			ImagePanel* pImagePanel = new ImagePanel(this, "active_canvas_panel");
			pImagePanel->DisableMouseInputForThisPanel(true);	// prevents the mouse lag
			pImagePanel->SetShouldScaleImage(true);
			pImagePanel->SetSize(GetWide(), GetTall());
			pImagePanel->SetImage("activecanvas");
		}

		// create the texture reference on our VGUI panel for the HUD layer.
		// NOTE: This is what creates the texture for sure, because hudcanvas doesn't even exist in the files anywhere.
		ITexture* pHudTexture = pHudInstance->GetTexture();
		ImagePanel* pHudImagePanel = new ImagePanel(this, "hud_canvas_panel");
		pHudImagePanel->DisableMouseInputForThisPanel(true);	// prevents the mouse lag
		pHudImagePanel->SetShouldScaleImage(true);
		pHudImagePanel->SetSize(GetWide(), GetTall());
		pHudImagePanel->SetImage("hudcanvas");
	}

	// handle cursor visibility
	if (!m_bFullscreen)
		this->HideInputCursor();
	//else
	//	this->ShowCursor();

	ivgui()->AddTickSignal(this->GetVPanel());	// because it's not until the 1st tick signal that it's able to set alpha to 0!
	// for opacity (we need to be 0 opacity, but visible)
	//ivgui()->AddTickSignal(this->GetVPanel());	// OBSOLETE!! Now handled in the Update() method!

	// finished initialization
	SetVisible(true);
	Activate();
}

void CInputSlate::SelfDestruct()
{
	DevMsg("Begin input slate destructing...\n");
	//delete this;

	if (!g_pAnarchyManager->IsShuttingDown() && m_pMaterialVar && m_pMaterialVar->IsDefined() && m_pMaterialVar->IsTexture())
	{
		if (m_pOriginalTexture)
		{
			DevMsg("Swapping original texture back in aborted. Setting to null instead.\n");
			//m_pMaterialVar->SetTextureValue(m_pOriginalTexture);
			m_pMaterialVar->SetTextureValue(null);
		}
		else if (m_pCanvasTexture)
		{
			//DevMsg("WARNING: Input Slate is closing but has no original texture to swap back into its material var!\n");
			DevMsg("No original texture to swap back in.\n");
			m_pMaterialVar->SetTextureValue(null);
		}
	}

	DevMsg("Input slate destructed.\n");
	delete this;

	/*
	bool found;
	IMaterialVar* pMaterialVar = m_pMaterial->FindVar("$basetexture", &found, false);
	if (!found || !pMaterialVar || !pMaterialVar->IsDefined() || !pMaterialVar->IsTexture())
		DevMsg("WARNING: Input slate is self-destructing but doesn't have a material to clean up.\n");
	else if ( m_pOriginalTexture )
		pMaterialVar->SetTextureValue(m_pOriginalTexture);
	*/
}

vgui::Panel* CInputSlate::GetPanel()
{
	return this;
}

ITexture* CInputSlate::GetCanvasTexture()
{
	return m_pCanvasTexture;
}

void CInputSlate::HideInputCursor()
{
	m_bCursorHidden = true;

	if (m_bInputCapture)
		ShowCursor(false);
}

void CInputSlate::ShowInputCursor()
{
	m_bCursorHidden = false;

	if (m_bInputCapture)
		ShowCursor(true);
}

void CInputSlate::OnTick()
{
	//ivgui()->RemoveTickSignal(this->GetVPanel());

	//m_bReadyForUpdates = true;
	//this->Update();

	if (g_pAnarchyManager->IsPaused())
		return;

	bool bFullscreen = g_pAnarchyManager->GetInputManager()->GetFullscreenMode();
	if (!bFullscreen && !m_bCursorAlphaZero)
	{
		if (GetAlpha() > 0)
			SetAlpha(0);
		else
		{
			//DevMsg("hiding cursor\n");
			m_bCursorAlphaZero = true;

			if (!m_bCursorHidden)
				this->HideInputCursor();
		}
	}
	else if (bFullscreen && m_bCursorAlphaZero)
	{
		//DevMsg("showing cursor\n");
		SetAlpha(255);
		m_bCursorAlphaZero = false;
		if (m_bCursorHidden)
			this->ShowInputCursor();
	}

	// update input mode
	// TODO: This logic should be triggered when ever needed by the input manager.
	bool bInputCaptureMode = g_pAnarchyManager->GetInputManager()->GetInputCapture();
	if (bInputCaptureMode != this->IsMouseInputEnabled())
	{
		if (this->IsMouseInputEnabled())
		{
			// deactivate our input
			this->SetKeyBoardInputEnabled(false);
			this->SetMouseInputEnabled(false);
		}
		else
		{
			// activate our input
			this->SetKeyBoardInputEnabled(true);
			this->SetMouseInputEnabled(true);
		}
	}
}

void CInputSlate::Update()
{
	//if (!m_bReadyForUpdates || g_pAnarchyManager->IsPaused())
	
}

void CInputSlate::OnMouseWheeled(int delta)
{
	if (g_pAnarchyManager->IsPaused())
		return;

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

	g_pAnarchyManager->GetInputManager()->MousePress(code);
}

void CInputSlate::OnMousePressed(MouseCode code)
{
	if (g_pAnarchyManager->IsPaused())
		return;

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

	g_pAnarchyManager->GetInputManager()->KeyCodePressed(code,
		(input()->IsKeyDown(KEY_LSHIFT) || input()->IsKeyDown(KEY_RSHIFT)),
		(input()->IsKeyDown(KEY_LCONTROL) || input()->IsKeyDown(KEY_RCONTROL)),
		(input()->IsKeyDown(KEY_LALT) || input()->IsKeyDown(KEY_RALT)),
		(input()->IsKeyDown(KEY_LWIN) || input()->IsKeyDown(KEY_RWIN)),
		false);
}

void CInputSlate::SetFullscreenMode(bool bFullscreenMode)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	DevMsg("Fullscreen transition canceled!\n");
	return;
	/*
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
	*/
}

void CInputSlate::OnKeyCodeReleased(KeyCode code)
{
	if (g_pAnarchyManager->IsPaused())
		return;

	g_pAnarchyManager->GetInputManager()->KeyCodeReleased(code,
		(input()->IsKeyDown(KEY_LSHIFT) || input()->IsKeyDown(KEY_RSHIFT)),
		(input()->IsKeyDown(KEY_LCONTROL) || input()->IsKeyDown(KEY_RCONTROL)),
		(input()->IsKeyDown(KEY_LALT) || input()->IsKeyDown(KEY_RALT)),
		(input()->IsKeyDown(KEY_LWIN) || input()->IsKeyDown(KEY_RWIN)),
		false);
}

void CInputSlate::OnCommand(const char* pcCommand)
{
	if( !Q_stricmp(pcCommand, "Close") )
		BaseClass::OnCommand(pcCommand);
}

CInputSlate::~CInputSlate()
{
	DevMsg("Close input slate\n");
	//m_fPreviousTime = system()->GetTimeMillis();

	//this->SelfDestruct();

	if (m_bCursorHidden)
		this->ShowInputCursor();
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
		//g_pAnarchyManager->GetInputManager()->SetInputSlate(this);
	}

	vgui::Panel* GetPanel()
	{
		return InputSlate->GetPanel();
	}

	void SetFullscreenMode(bool bFullscreenMode)
	{
		InputSlate->SetFullscreenMode(bFullscreenMode);
	}
	
	void Destroy()
	{
		if (InputSlate)
		{
			InputSlate->SetParent((vgui::Panel *)NULL);
			InputSlate->SelfDestruct();
			//delete InputSlate;
			InputSlate = null;
			//g_pAnarchyManager->GetInputManager()->SetInputSlate(null);
		}
	}

	void Update()
	{
		if (InputSlate)
			InputSlate->Update();
	}

	ITexture* GetCanvasTexture()
	{
		if (InputSlate)
			return InputSlate->GetCanvasTexture();

		return null;
	}
};
static CInputSlateInterface g_InputSlate;
IInputSlate* InputSlate = (IInputSlate*)&g_InputSlate;
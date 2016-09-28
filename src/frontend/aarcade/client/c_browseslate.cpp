#include "cbase.h"
#include <vgui_controls/Panel.h>

#include "c_browseslate.h"
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

unsigned ThreadedFileBrowse(void *params)
{
	//C_ArcadeResources* pClientArcadeResources = C_ArcadeResources::GetSelf();

	HWND myHWnd = FindWindow(null, "Anarchy Arcade");

	ThreadedFileBrowseParams_t* vars = (ThreadedFileBrowseParams_t*)params; // always use a struct!

	OPENFILENAME ofn;
	char szFile[MAX_PATH];
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = myHWnd;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0Images\0*.BMP;*.ICO;*.GIF;*.JPG;*.JPEG;*.JPE;*.JP2;*.PCX;*.PIC;*.PNG;*.PIX;*.RAW;*.TGA;*.TIF;*.VTF;*.TBN\0Videos\0*.MPG;*.MPEG;*.AVI;*.MKV;*.MP4;*.MPG;*.MPEG;*.FLV;*.BIN;*.ISO;*.VOB\0Executables\0*.EXE\0Steam Shortcuts\0*.URL\0Custom Shortcuts\0*.LNK\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
	//	ofn.Flags = 0;
	//	ofn.Flags = NULL;
	GetOpenFileName(&ofn);

	std::string buf = ofn.lpstrFile;

	vars->response = VarArgs("%s", buf.c_str());
	vars->ready = true;
	//	if( vars->listener )
	//		vars->listener->BrowseCallback(buf.c_str(), vars->keyFieldName.c_str(), vars->itemKV);
	//	else if( vars->browser )
	//		vars->browser->BrowseCallback(buf.c_str(), vars->keyFieldName.c_str(), vars->itemKV);

	//	delete vars;
	return 0;
}

CBrowseSlate::CBrowseSlate(vgui::VPANEL parent) : Frame(null, "BrowseSlate")
{
	SetParent( parent );
	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);
	DoModal();

	// for monitoring browse dialog state
	ivgui()->AddTickSignal(this->GetVPanel());

	m_pFileParams = new ThreadedFileBrowseParams_t;
	m_pFileParams->ready = false;
	m_pFileParams->response = "";
	CreateSimpleThread(ThreadedFileBrowse, m_pFileParams);

	// automatically pin the input slate open if it was just opened a moment ago
	/*
	long currentTime = system()->GetTimeMillis();
	if (m_fPreviousTime > 0 && currentTime - m_fPreviousTime < 200)
		g_pAnarchyManager->GetInputManager()->ForceInputMode();

	m_bCursorAlphaZero = false;
	m_bCursorHidden = false;

//	m_pWebViewManager = C_AnarchyManager::GetSelf()->GetWebViewManager();

	m_pOriginalTexture = null;
	m_pMaterial = null;

	SetKeyBoardInputEnabled( true );
	SetMouseInputEnabled( true );

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

	if ( !m_bFullscreen )
	{
		m_bCursorHidden = true;
		ShowCursor(false);
	}

	// for opacity
	ivgui()->AddTickSignal(this->GetVPanel());

	SetVisible(true);
	Activate();

	//ShowCursor(true);
	*/
}

void CBrowseSlate::OnTick()
{
	if (m_pFileParams)
	{
		if (m_pFileParams->ready)
		{
			DevMsg("Params are ready: %s\n", m_pFileParams->response.c_str());

			this->OnCommand("close");
		}
	}
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

/*
vgui::Panel* CBrowseSlate::GetPanel()
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

	DevMsg("Disbaled for testing!\n");
	return;
//	g_pAnarchyManager->GetWebManager()->OnMouseWheel(delta);
}

void CInputSlate::OnCursorMoved(int x, int y)
{
	if (g_pAnarchyManager->IsPaused())
		return;

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

void CInputSlate::OnKeyCodeReleased(KeyCode code)
{
	if (code == KEY_ESCAPE && g_pAnarchyManager->IsPaused())
	{
		g_pAnarchyManager->Unpause();
		return;
	}

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
}
*/

void CBrowseSlate::OnCommand(const char* pcCommand)
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

		if (m_pFileParams)
		{
			delete m_pFileParams;
			m_pFileParams = null;
		}

		BaseClass::OnCommand(pcCommand);
	}
}

CBrowseSlate::~CBrowseSlate()
{
	//m_fPreviousTime = system()->GetTimeMillis();
	/*
	if (m_pOriginalTexture)
	{
		bool found;
		IMaterialVar* pMaterialVar = m_pMaterial->FindVar("$basetexture", &found, false);
		if( pMaterialVar )
			pMaterialVar->SetTextureValue(m_pOriginalTexture);
	}
	*/

	/*
	if (m_bCursorHidden)
	{
		m_bCursorHidden = false;
		ShowCursor(true);
		DevMsg("showing cursor\n");
	}
	*/
}

class CBrowseSlateInterface : public IBrowseSlate
{
private:
	CBrowseSlate *BrowseSlate;
public:
	CBrowseSlateInterface()
	{
		BrowseSlate = NULL;
	}

	void Create(vgui::VPANEL parent)
	{
		BrowseSlate = new CBrowseSlate(parent);
	}
	/*
	vgui::Panel* GetPanel()
	{
		return InputSlate->GetPanel();
	}

	void SetFullscreenMode(bool bFullscreenMode)
	{
		InputSlate->SetFullscreenMode(bFullscreenMode);
	}
	*/

	void Destroy()
	{
		if (BrowseSlate)
		{
			BrowseSlate->SetParent((vgui::Panel *)NULL);
			//BrowseSlate->SelfDestruct();
			delete BrowseSlate;
		}
	}
};
static CBrowseSlateInterface g_BrowseSlate;
IBrowseSlate* BrowseSlate = (IBrowseSlate*)&g_BrowseSlate;
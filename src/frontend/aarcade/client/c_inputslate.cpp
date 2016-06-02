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

CInputSlate::CInputSlate(vgui::VPANEL parent) : Frame(null, "InputSlate")
{
//	SetParent( parent );

//	m_pWebViewManager = C_AnarchyManager::GetSelf()->GetWebViewManager();

	SetKeyBoardInputEnabled( true );
	SetMouseInputEnabled( true );

	SetProportional( false );
	SetTitleBarVisible( false );
	SetMinimizeButtonVisible( false );
	SetMaximizeButtonVisible( false );
	SetCloseButtonVisible( false );
	SetSizeable( false );
	SetMoveable( false );

	SetWide(ScreenWidth());
	SetTall(ScreenHeight());

	// Hide all children of this panel
	/*
	for(int i=0; i<GetChildCount(); i++)
	{
		Panel* pPanel = GetChild(i);
		pPanel->SetVisible(false);
	}
	*/

	//ivgui()->AddTickSignal(this->GetVPanel(),1);	// disabled for in-panel view 6/29/13

	if (g_pAnarchyManager->GetWebManager()->GetSelectedWebTab())
	{
		IMaterial* pMaterial = g_pMaterialSystem->FindMaterial("vgui/selectedwebtab", TEXTURE_GROUP_VGUI);
		ITexture* pTexture = g_pAnarchyManager->GetWebManager()->GetSelectedWebTab()->GetTexture();
		if (!pTexture)
			DevMsg("No texture found yet!\n");
		bool found;
		IMaterialVar* pMaterialVar = pMaterial->FindVar("$basetexture", &found, false);
		pMaterialVar->SetTextureValue(pTexture);

		ImagePanel* pImagePanel = new ImagePanel(this, "selected_webtab_panel");
		pImagePanel->DisableMouseInputForThisPanel(true);	// prevents the mouse lag
		pImagePanel->SetShouldScaleImage(true);
		pImagePanel->SetSize(GetWide(), GetTall());
		pImagePanel->SetImage("selectedwebtab");
	}

	//ShowCursor(false);
	SetVisible(true);
}

void CInputSlate::OnTick()
{
	/*
	if( GetAlpha() > 0 )
	{
		SetAlpha(0);	// This won't actually change anything unless the panel has finished being created
	}
	else
	{
		ivgui()->RemoveTickSignal(this->GetVPanel());
	}
	*/
}

void CInputSlate::OnCursorMoved(int x, int y)
{
	g_pAnarchyManager->GetInputManager()->MouseMove(x / (GetWide() * 1.0), y / (GetTall() * 1.0));
}

/*
void CHDViewInput::OnMouseDoublePressed(MouseCode code)
{
//	ConVar* pConVar = cvar->FindVar("hdview_input_lock");
//	pConVar->SetValue(1);

	DevMsg("Double pressed!\n");
}

		//JSArray args;
		//JSValue window2 = pWebView->ExecuteJavascriptWithResult(WSLit("function BgAlt(){document.body.style.backgroundColor = \"#00FFFF\";}"), WSLit(""));
		//window.ToObject().Invoke(WSLit("BgAlt"), args);

		*/

void CInputSlate::OnMousePressed(MouseCode code)
{
	g_pAnarchyManager->GetInputManager()->MousePress(code);
}

void CInputSlate::OnMouseReleased(MouseCode code)
{
	g_pAnarchyManager->GetInputManager()->MouseRelease(code);
}

void CInputSlate::OnKeyCodePressed(KeyCode code)
{
	bool shift = (input()->IsKeyDown(KEY_LSHIFT) || input()->IsKeyDown(KEY_RSHIFT));

	/*
	if( code == KEY_Q && shift )
	{
		OnCommand("close");
	}
	else
		m_pWebViewManager->RelayOnKeyDown(code);
	*/

	DevMsg("Key pressed: %i %i\n", code, shift);
}
/*
	using namespace Awesomium;

	if( !web_view )
		return;

	WebView* pWebView = static_cast<WebView*>(web_view);

	WebKeyboardEvent pWebKeyboardEvent;
	pWebKeyboardEvent.type = WebKeyboardEvent::kTypeKeyDown;

	pWebKeyboardEvent.modifiers = 0;

	bool shift = (input()->IsKeyDown(KEY_LSHIFT) || input()->IsKeyDown(KEY_RSHIFT));
	bool ctrl = (input()->IsKeyDown(KEY_LCONTROL) || input()->IsKeyDown(KEY_RCONTROL));
	bool alt = (input()->IsKeyDown(KEY_LALT) || input()->IsKeyDown(KEY_RALT));

	if ( shift )
		pWebKeyboardEvent.modifiers |= MODIFIER_SHIFT;

	if ( ctrl )
		pWebKeyboardEvent.modifiers |= MODIFIER_CONTROL;

	if ( alt )
		pWebKeyboardEvent.modifiers |= MODIFIER_ALT;

	int virtualKeyCode = KeyCodes::AK_UNKNOWN;
	std::string actualCharOutput = "";

	switch( code )
	{
		case KEY_0:
			virtualKeyCode = 0x30;
			if( shift )
				actualCharOutput = ')';
			else
				actualCharOutput = '0';
			break;

		case KEY_1:
			virtualKeyCode = 0x31;
			if( shift )
				actualCharOutput = '!';
			else
				actualCharOutput = '1';
			break;

		case KEY_2:
			virtualKeyCode = 0x32;
			if( shift )
				actualCharOutput = '@';
			else
				actualCharOutput = '2';
			break;

		case KEY_3:
			virtualKeyCode = 0x33;
			if( shift )
				actualCharOutput = '#';
			else
				actualCharOutput = '3';
			break;

		case KEY_4:
			virtualKeyCode = 0x34;
			if( shift )
				actualCharOutput = '$';
			else
				actualCharOutput = '4';
			break;

		case KEY_5:
			virtualKeyCode = 0x35;
			if( shift )
				actualCharOutput = '%';
			else
				actualCharOutput = '5';
			break;

		case KEY_6:
			virtualKeyCode = 0x36;
			if( shift )
				actualCharOutput = '^';
			else
				actualCharOutput = '6';
			break;

		case KEY_7:
			virtualKeyCode = 0x37;
			if( shift )
				actualCharOutput = '&';
			else
				actualCharOutput = '7';
			break;

		case KEY_8:
			virtualKeyCode = 0x38;
			if( shift )
				actualCharOutput = '*';
			else
				actualCharOutput = '8';
			break;

		case KEY_9:
			virtualKeyCode = 0x39;
			if( shift )
				actualCharOutput = '(';
			else
				actualCharOutput = '9';
			break;

		case KEY_A:
			virtualKeyCode = 0x41;
			if( shift )
				actualCharOutput = 'A';
			else
				actualCharOutput = 'a';
			break;

		case KEY_B:
			virtualKeyCode = 0x42;
			if( shift )
				actualCharOutput = 'B';
			else
				actualCharOutput = 'b';
			break;

		case KEY_C:
			virtualKeyCode = 0x43;
			if( shift )
				actualCharOutput = 'C';
			else
				actualCharOutput = 'c';
			break;

		case KEY_D:
			virtualKeyCode = 0x44;
			if( shift )
				actualCharOutput = 'D';
			else
				actualCharOutput = 'd';
			break;

		case KEY_E:
			virtualKeyCode = 0x45;
			if( shift )
				actualCharOutput = 'E';
			else
				actualCharOutput = 'e';
			break;

		case KEY_F:
			virtualKeyCode = 0x46;
			if( shift )
				actualCharOutput = 'F';
			else
				actualCharOutput = 'f';
			break;

		case KEY_G:
			virtualKeyCode = 0x47;
			if( shift )
				actualCharOutput = 'G';
			else
				actualCharOutput = 'g';
			break;

		case KEY_H:
			virtualKeyCode = 0x48;
			if( shift )
				actualCharOutput = 'H';
			else
				actualCharOutput = 'h';
			break;

		case KEY_I:
			virtualKeyCode = 0x49;
			if( shift )
				actualCharOutput = 'I';
			else
				actualCharOutput = 'i';
			break;

		case KEY_J:
			virtualKeyCode = 0x4A;
			if( shift )
				actualCharOutput = 'J';
			else
				actualCharOutput = 'j';
			break;

		case KEY_K:
			virtualKeyCode = 0x4B;
			if( shift )
				actualCharOutput = 'K';
			else
				actualCharOutput = 'k';
			break;

		case KEY_L:
			virtualKeyCode = 0x4C;
			if( shift )
				actualCharOutput = 'L';
			else
				actualCharOutput = 'l';
			break;

		case KEY_M:
			virtualKeyCode = 0x4D;
			if( shift )
				actualCharOutput = 'M';
			else
				actualCharOutput = 'm';
			break;

		case KEY_N:
			virtualKeyCode = 0x4E;
			if( shift )
				actualCharOutput = 'N';
			else
				actualCharOutput = 'n';
			break;

		case KEY_O:
			virtualKeyCode = 0x4F;
			if( shift )
				actualCharOutput = 'O';
			else
				actualCharOutput = 'o';
			break;

		case KEY_P:
			virtualKeyCode = 0x50;
			if( shift )
				actualCharOutput = 'P';
			else
				actualCharOutput = 'p';
			break;

		case KEY_Q:
			virtualKeyCode = 0x51;
			if( shift )
				actualCharOutput = 'Q';
			else
				actualCharOutput = 'q';
			break;

		case KEY_R:
			virtualKeyCode = 0x52;
			if( shift )
				actualCharOutput = 'R';
			else
				actualCharOutput = 'r';
			break;

		case KEY_S:
			virtualKeyCode = 0x53;
			if( shift )
				actualCharOutput = 'S';
			else
				actualCharOutput = 's';
			break;

		case KEY_T:
			virtualKeyCode = 0x54;
			if( shift )
				actualCharOutput = 'T';
			else
				actualCharOutput = 't';
			break;

		case KEY_U:
			virtualKeyCode = 0x55;
			if( shift )
				actualCharOutput = 'U';
			else
				actualCharOutput = 'u';
			break;

		case KEY_V:
			virtualKeyCode = 0x56;
			if( shift )
				actualCharOutput = 'V';
			else
				actualCharOutput = 'v';
			break;

		case KEY_W:
			virtualKeyCode = 0x57;
			if( shift )
				actualCharOutput = 'W';
			else
				actualCharOutput = 'w';
			break;

		case KEY_X:
			virtualKeyCode = 0x58;
			if( shift )
				actualCharOutput = 'X';
			else
				actualCharOutput = 'x';
			break;

		case KEY_Y:
			virtualKeyCode = 0x59;
			if( shift )
				actualCharOutput = 'Y';
			else
				actualCharOutput = 'y';
			break;

		case KEY_Z:
			virtualKeyCode = 0x5A;
			if( shift )
				actualCharOutput = 'Z';
			else
				actualCharOutput = 'z';
			break;

		case KEY_PAD_0:
			virtualKeyCode = 0x60;
			if( !shift )
				actualCharOutput = '0';
			break;

		case KEY_PAD_1:
			virtualKeyCode = 0x61;
			actualCharOutput = '1';
			break;

		case KEY_PAD_2:
			virtualKeyCode = 0x62;
			actualCharOutput = '2';
			break;

		case KEY_PAD_3:
			virtualKeyCode = 0x63;
			actualCharOutput = '3';
			break;

		case KEY_PAD_4:
			virtualKeyCode = 0x64;
			actualCharOutput = '4';
			break;

		case KEY_PAD_5:
			virtualKeyCode = 0x65;
			actualCharOutput = '5';
			break;

		case KEY_PAD_6:
			virtualKeyCode = 0x66;
			actualCharOutput = '6';
			break;

		case KEY_PAD_7:
			virtualKeyCode = 0x67;
			actualCharOutput = '7';
			break;

		case KEY_PAD_8:
			virtualKeyCode = 0x68;
			actualCharOutput = '8';
			break;

		case KEY_PAD_9:
			virtualKeyCode = 0x69;
			actualCharOutput = '9';
			break;

		case KEY_PAD_DIVIDE:
			virtualKeyCode = 0x6F;
			actualCharOutput = '/';
			break;

		case KEY_PAD_MULTIPLY:
			virtualKeyCode = 0x6A;
			actualCharOutput = '*';
			break;

		case KEY_PAD_MINUS:
			virtualKeyCode = 0x6D;
			actualCharOutput = '-';
			break;

		case KEY_PAD_PLUS:
			virtualKeyCode = 0x6B;
			actualCharOutput = '+';
			break;

		case KEY_PAD_ENTER:
			virtualKeyCode = 0x0D;
			actualCharOutput = '\r';
			break;

		case KEY_PAD_DECIMAL:
			virtualKeyCode = 0x6E;
			actualCharOutput = '.';
			break;

		case KEY_LBRACKET:
			virtualKeyCode = 0xDB;
			if( shift )
				actualCharOutput = '{';
			else
				actualCharOutput = '[';
			break;

		case KEY_RBRACKET:
			virtualKeyCode = 0xDD;
			if( shift )
				actualCharOutput = '}';
			else
				actualCharOutput = ']';
			break;

		case KEY_SEMICOLON:
			virtualKeyCode = 0xBA;
			if( shift )
				actualCharOutput = ':';
			else
				actualCharOutput = ';';
			break;

		case KEY_APOSTROPHE:
			virtualKeyCode = 0xDE;
			if( shift )
				actualCharOutput = '"';
			else
				actualCharOutput = '\'';
			break;

		case KEY_BACKQUOTE:
			virtualKeyCode = 0xC0;
			if( shift )
				actualCharOutput = '~';
			else
				actualCharOutput = '`';
			break;

		case KEY_COMMA:
			virtualKeyCode = 0xBC;
			if( shift )
				actualCharOutput = '<';
			else
				actualCharOutput = ',';
			break;

		case KEY_PERIOD:
			virtualKeyCode = 0xBE;
			if( shift )
				actualCharOutput = '>';
			else
				actualCharOutput = '.';
			break;

		case KEY_SLASH:
			virtualKeyCode = 0xBF;
			if( shift )
				actualCharOutput = '?';
			else
				actualCharOutput = '/';
			break;

		case KEY_BACKSLASH:
			virtualKeyCode = 0xDC;
			if( shift )
				actualCharOutput = '|';
			else
				actualCharOutput = '\\';
			break;

		case KEY_MINUS:
			virtualKeyCode = 0xBD;
			if( shift )
				actualCharOutput = '_';
			else
				actualCharOutput = '-';
			break;

		case KEY_EQUAL:
			virtualKeyCode = 0xBB;
			if( shift )
				actualCharOutput = '+';
			else
				actualCharOutput = '=';
			break;

		case KEY_ENTER:
			virtualKeyCode = 0x0D;
			actualCharOutput = '\r';
			break;

		case KEY_SPACE:
			virtualKeyCode = 0x20;
			actualCharOutput = ' ';
			break;

		case KEY_BACKSPACE:
			virtualKeyCode = 0x08;
			break;

		case KEY_TAB:
			virtualKeyCode = 0x09;
			break;

		case KEY_CAPSLOCK:
			virtualKeyCode = 0x14;
			break;

		case KEY_NUMLOCK:
			virtualKeyCode = 0x90;
			break;

		case KEY_ESCAPE:
			virtualKeyCode = 0x1B;
			break;

		case KEY_SCROLLLOCK:
			virtualKeyCode = 0x91;
			break;

		case KEY_INSERT:
			virtualKeyCode = 0x2D;
			break;

		case KEY_DELETE:
			virtualKeyCode = 0x2E;
			break;

		case KEY_HOME:
			virtualKeyCode = 0x24;
			break;

		case KEY_END:
			virtualKeyCode = 0x23;
			break;

		case KEY_PAGEUP:
			virtualKeyCode = 0x21;
			break;

		case KEY_PAGEDOWN:
			virtualKeyCode = 0x22;
			break;

		case KEY_LSHIFT:
			virtualKeyCode = 0xA0;
			break;

		case KEY_RSHIFT:
			virtualKeyCode = 0xA1;
			break;

		case KEY_LALT:
			virtualKeyCode = 0x12;
			break;

		case KEY_RALT:
			virtualKeyCode = 0x12;
			break;

		case KEY_LCONTROL:
			virtualKeyCode = 0xA2;
			break;

		case KEY_RCONTROL:
			virtualKeyCode = 0xA3;
			break;

		case KEY_LWIN:
			virtualKeyCode = 0x5B;
			break;

		case KEY_RWIN:
			virtualKeyCode = 0x5C;
			break;

		case KEY_APP:
			virtualKeyCode = 0x5D;
			break;

		case KEY_UP:
			virtualKeyCode = 0x26;
			break;

		case KEY_LEFT:
			virtualKeyCode = 0x25;
			break;

		case KEY_DOWN:
			virtualKeyCode = 0x28;
			break;

		case KEY_RIGHT:
			virtualKeyCode = 0x27;
			break;
	}

	char outputChar = actualCharOutput[0];

	char* buf = new char[20];
    pWebKeyboardEvent.virtual_key_code = virtualKeyCode;
    GetKeyIdentifierFromVirtualKeyCode(pWebKeyboardEvent.virtual_key_code, &buf);
    strcpy(pWebKeyboardEvent.key_identifier, buf);
    delete[] buf;

	pWebKeyboardEvent.native_key_code = pWebKeyboardEvent.virtual_key_code;

	bool hasChar = false;

	// If this key generates text output...
	if( actualCharOutput != "" )
		hasChar = true;

	if( hasChar )
	{
		pWebKeyboardEvent.text[0] = outputChar;
		pWebKeyboardEvent.unmodified_text[0] = outputChar;
	}
	else
	{
		pWebKeyboardEvent.text[0] = null;
		pWebKeyboardEvent.unmodified_text[0] = null;
	}

	pWebView->InjectKeyboardEvent(pWebKeyboardEvent);

	// If this key has text output, we gotta send a char msg too
	if( hasChar )
	{
		pWebKeyboardEvent.type = WebKeyboardEvent::kTypeChar;

		pWebKeyboardEvent.virtual_key_code = virtualKeyCode;
		pWebKeyboardEvent.native_key_code = virtualKeyCode;

		pWebView->InjectKeyboardEvent(pWebKeyboardEvent);
	}
}

void CHDViewInput::OnKeyCodeReleased(KeyCode code)
{
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
		BaseClass::OnCommand(pcCommand);
	}
}

CInputSlate::~CInputSlate()
{
	// do nothing
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

	void Destroy()
	{
		if (InputSlate)
		{
			InputSlate->SetParent((vgui::Panel *)NULL);
			delete InputSlate;
		}
	}
};
static CInputSlateInterface g_InputSlate;
IInputSlate* InputSlate = (IInputSlate*)&g_InputSlate;
#include "cbase.h"
#include <vgui_controls/Panel.h>

#include "c_toast.h"
#include "c_anarchymanager.h"

/*
#include "vgui_controls/MenuButton.h"
#include "vgui_controls/Menu.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/CheckButton.h"
#include "vgui_controls/Slider.h"
#include "vgui_controls/ListPanel.h"
*/

#include "vgui_controls/Label.h"
#include "vgui/IPanel.h"
#include "vgui/IVGui.h"
#include "vgui/ISurface.h"
#include "vgui/IInput.h"
#include "vgui/IVGUI.h"
#include "vgui/IInput.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

CToastSlate::CToastSlate(vgui::VPANEL parent) : Frame(null, "ToastSlate")
{
	DevMsg("Toast Slate Created\n");

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
	SetPaintBackgroundEnabled(true);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	// Hide all children of this panel (including the invisible title bar that steals mouse move input
	for (int i = 0; i<GetChildCount(); i++)
	{
		Panel* pPanel = GetChild(i);
		pPanel->SetVisible(false);
	}

	vgui::IScheme* pScheme = vgui::scheme()->GetIScheme(GetScheme());
	vgui::HFont hFont = pScheme->GetFont("Trebuchet24");

	// Toast
	m_pLabel = new Label(this, "Toast", g_pAnarchyManager->GetToastText().c_str());
	m_pLabel->SetFont(hFont);
	m_pLabel->DisableMouseInputForThisPanel(true);
	m_pLabel->SetContentAlignment(Label::a_west);
	m_pLabel->SetTextInset(20, 0);
	m_pLabel->SetPos(0, ScreenHeight() / 5);
	m_pLabel->SetPaintBackgroundEnabled(true);

	int contentWidth;
	int contentHeight;
	m_pLabel->GetContentSize(contentWidth, contentHeight);
	m_pLabel->SetSize(contentWidth + 20, contentHeight + 20);
	m_pLabel->SetBgColor(Color(0, 0, 0, 220));
	g_pAnarchyManager->AddToastLabel(m_pLabel);

	// Hover Title
	m_pHoverLabel = new Label(this, "HoverTitle", "");
	m_pHoverLabel->SetFont(hFont);
	m_pHoverLabel->DisableMouseInputForThisPanel(true);
	m_pHoverLabel->SetContentAlignment(Label::a_center);
	m_pHoverLabel->SetPaintBackgroundEnabled(true);

	int hoverContentWidth;
	int hoverContentHeight;
	m_pHoverLabel->GetContentSize(hoverContentWidth, hoverContentHeight);
	//m_pHoverLabel->SetSize(ScreenWidth(), contentHeight + 20);
	//m_pHoverLabel->SetPos(0, ScreenHeight() - ((contentHeight + 20) / 2.0));
	m_pHoverLabel->SetSize(1, 1);
	m_pHoverLabel->SetPos(ScreenWidth() / 2, ScreenHeight() / 2);
	m_pHoverLabel->SetBgColor(Color(0, 0, 0, 220));
	g_pAnarchyManager->InitHoverLabel(m_pHoverLabel);

	SetVisible(true);
	//Activate();
}

void CToastSlate::PaintBackground()
{
	if (g_pAnarchyManager->GetToastText() != "")
		m_pLabel->SetBgColor(Color(0, 0, 0, 220));
	else
		m_pLabel->SetBgColor(Color(0, 0, 0, 0));

	if (g_pAnarchyManager->GetHoverTitle() != "")
		m_pHoverLabel->SetBgColor(Color(0, 0, 0, 220));
	else
		m_pHoverLabel->SetBgColor(Color(0, 0, 0, 0));
}

vgui::Panel* CToastSlate::GetPanel()
{
	return this;
}

void CToastSlate::Update()
{
}

void CToastSlate::OnMouseWheeled(int delta)
{
}

void CToastSlate::OnCursorMoved(int x, int y)
{
}

void CToastSlate::OnMouseDoublePressed(MouseCode code)
{
}

void CToastSlate::OnMousePressed(MouseCode code)
{
}

void CToastSlate::OnMouseReleased(MouseCode code)
{
}

void CToastSlate::OnKeyCodePressed(KeyCode code)
{
}

void CToastSlate::OnKeyCodeReleased(KeyCode code)
{
}

void CToastSlate::OnCommand(const char* pcCommand)
{
	if( !Q_stricmp(pcCommand, "Close") )
		BaseClass::OnCommand(pcCommand);
}

CToastSlate::~CToastSlate()
{
	DevMsg("Close toast slate\n");
	g_pAnarchyManager->RemoveToastLabel(m_pLabel);
}

class CToastSlateInterface : public IToastSlate
{
private:
	CToastSlate *ToastSlate;
public:
	CToastSlateInterface()
	{
		ToastSlate = NULL;
	}

	void Create(vgui::VPANEL parent)
	{
		ToastSlate = new CToastSlate(parent);
	}

	vgui::Panel* GetPanel()
	{
		return ToastSlate->GetPanel();
	}
	
	void Destroy()
	{
		if (ToastSlate)
		{
			ToastSlate->SetParent((vgui::Panel *)NULL);
			//ToastSlate->SelfDestruct();
			delete ToastSlate;
			ToastSlate = null;
		}
	}

	void Update()
	{
		if (ToastSlate)
			ToastSlate->Update();
	}
};
static CToastSlateInterface g_ToastSlate;
IToastSlate* ToastSlate = (IToastSlate*)&g_ToastSlate;
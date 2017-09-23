#include "cbase.h"
#include <vgui_controls/Panel.h>

#include "c_earlyerror.h"

#include "vgui/IVGUI.h"
#include "vgui/IInput.h"

#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/HTML.h"
#include <vgui/ISurface.h>
#include "materialsystem/IMaterialVar.h"
#include <string>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;
CEarlyError::CEarlyError(vgui::VPANEL parent, const char* msg) : Frame(null, "EarlyError")
{
	std::string errorText = msg;

	// vgui constructor stuff
	SetParent( parent );
	SetProportional(false);
	SetTitleBarVisible(true);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(true);
	SetSizeable(false);
	SetMoveable(true);
	//SetWide(ScreenWidth());
	//SetTall(ScreenHeight());
	//SetPaintBackgroundEnabled(false);

	LoadControlSettings("resource/UI/EarlyError.res");
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/SourceSchemeX.res", "SourceSchemeX"));
	vgui::IScheme* pScheme = vgui::scheme()->GetIScheme(GetScheme());
	HFont hFont = pScheme->GetFont("DefaultBig");
	DoModal();

	this->SetTitle("ERROR", true);

	// finished initialization
//	std::string errorText;

	int iWidth = ScreenWidth();
	int iHeight = ScreenHeight();

	Label* pLabel = new Label(this, "ErrorMsg", errorText.c_str());
	pLabel->SetFont(hFont);
	
	int iLabelWidth;
	int iLabelHeight;
	pLabel->GetContentSize(iLabelWidth, iLabelHeight);

	Panel* pCaptionPanel = this->FindChildByName("frame_caption");
	int iCaptionWidth;
	int iCaptionHeight;
	pCaptionPanel->GetSize(iCaptionWidth, iCaptionHeight);

	int iLabelPadding = 30;
	int iFrameWidth = iLabelWidth + (iLabelPadding * 2);
	int iFrameHeight = iLabelHeight + iCaptionHeight + (iLabelPadding * 2);
	this->SetSize(iFrameWidth, iFrameHeight);
	this->SetPos((iWidth / 2) - (iFrameWidth / 2), (iHeight / 2) - (iFrameHeight / 2));
	pLabel->SetPos(iLabelPadding, iCaptionHeight + iLabelPadding);
	pLabel->SetSize(iLabelWidth, iLabelHeight);

	SetVisible(true);
	Activate();
}

vgui::Panel* CEarlyError::GetPanel()
{
	return this;
}
/*
void CEarlyError::Paint()
{
	Label* pLabel = dynamic_cast<Label*>(this->FindChildByName("ErrorMsg"));
	pLabel->SetFgColor(Color(0, 0, 0, 255));
}*/

/*
void CInputSlate::OnCursorMoved(int x, int y)
{

}

void CInputSlate::OnMouseDoublePressed(MouseCode code)
{
}

void CInputSlate::OnMousePressed(MouseCode code)
{
}

void CInputSlate::OnMouseReleased(MouseCode code)
{
}

void CInputSlate::OnKeyCodePressed(KeyCode code)
{
}

void CInputSlate::OnKeyCodeReleased(KeyCode code)
{
}
*/
void CEarlyError::OnCommand(const char* pcCommand)
{
	if( !Q_stricmp(pcCommand, "Close") )
		BaseClass::OnCommand(pcCommand);
}

CEarlyError::~CEarlyError()
{
}

class CEarlyErrorInterface : public IEarlyError
{
private:
	CEarlyError *EarlyError;
public:
	CEarlyErrorInterface()
	{
		EarlyError = NULL;
	}

	void Create(vgui::VPANEL parent, const char* msg)
	{
		EarlyError = new CEarlyError(parent, msg);
	}

	vgui::Panel* GetPanel()
	{
		return EarlyError->GetPanel();
	}

	void Destroy()
	{
		if (EarlyError)
			delete EarlyError;
	}

	void Update()
	{
		if (EarlyError)
			EarlyError->Update();
	}
};

static CEarlyErrorInterface g_EarlyError;
IEarlyError* EarlyError = (IEarlyError*)&g_EarlyError;
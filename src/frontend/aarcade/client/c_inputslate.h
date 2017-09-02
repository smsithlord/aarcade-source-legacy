#ifndef INPUT_SLATE_H
#define INPUT_SLATE_H

class IInputSlate
{
public:
	virtual void		Create(vgui::VPANEL parent) = 0;
	virtual void		Destroy(void) = 0;
	virtual vgui::Panel*		GetPanel() = 0;
	virtual void Update() = 0;
	virtual ITexture* GetCanvasTexture() { return null; }
};

extern IInputSlate* InputSlate;

//#include "c_webViewManager.h"
#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include "vgui_controls/Label.h"
#include "aarcade/client/C_EmbeddedInstance.h"
//#include <vgui_controls/TextEntry.h>
//#include <vgui_controls/Button.h>
//#include <vgui_controls/ComboBox.h>

//#include <string>

//#include "UIPanel.h"

//#include "Awesomium_Panel.h"

namespace vgui
{
	class CInputSlate : public Frame
	{
		DECLARE_CLASS_SIMPLE(CInputSlate, Frame);

	public:
		CInputSlate(vgui::VPANEL parent);
		virtual ~CInputSlate();

		//static ITexture* s_pOriginalTexture;
	//	static IMaterial* s_pMaterial;

		void HideInputCursor();
		void ShowInputCursor();

		void OnTick();
		void Update();
		void SelfDestruct();
		void PaintBackground();

		void OnCursorMoved(int x, int y);
		void OnMouseWheeled(int delta);
		void OnMousePressed(MouseCode code);
		void OnMouseReleased(MouseCode code);
		void OnMouseDoublePressed(MouseCode code);

//		void ForceClose();

		void OnKeyCodePressed(KeyCode code);
		void OnKeyCodeReleased(KeyCode code);

		void SetFullscreenMode(bool bFullscreenMode);
	//	void SetOverlayMode(bool bOverlayMode);

		vgui::Panel* GetPanel();

		ITexture* GetCanvasTexture();

		void AdjustOverlay(float fX, float fY, float fWidth, float fHeight, std::string overlayId);

	private:
		Label* m_pLabel;
		bool m_bMainMenu;
		bool m_bFullscreen;
		bool m_bInputCapture;
		ImagePanel* m_pImagePanel;
		ImagePanel* m_pHudImagePanel;
	//	bool m_bOverlay;
	//	static long m_fPreviousTime;

	protected:
		void OnCommand(const char* pcCommand);
		//virtual void PaintBackground();

	private:
		bool m_bReadyForUpdates;
		IMaterialVar* m_pMaterialVar;
		C_EmbeddedInstance* m_pCanvasInstance;
		ITexture* m_pCanvasTexture;
		bool m_bCursorAlphaZero;	// different than actually hiding the cursor
		bool m_bCursorHidden;
		ITexture* m_pOriginalTexture;
		IMaterial* m_pMaterial;
//		CWebViewInput* m_pWebViewInput;
	};
} // namespace vgui

#endif // INPUT_SLATE_H
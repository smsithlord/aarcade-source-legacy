class IInputSlate
{
	public:
		virtual void		Create(vgui::VPANEL parent) = 0;
		virtual void		Destroy( void ) = 0;
};
 
extern IInputSlate* InputSlate;


#ifndef INPUT_SLATE_H
#define INPUT_SLATE_H

//#include "c_webViewManager.h"
#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
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

		void OnTick();

		void OnCursorMoved(int x, int y);
		void OnMousePressed(MouseCode code);
		void OnMouseReleased(MouseCode code);
//		void OnMouseDoublePressed(MouseCode code);

//		void ForceClose();

		void OnKeyCodePressed(KeyCode code);
//		void OnKeyCodeReleased(KeyCode code);

	protected:
		void OnCommand(const char* pcCommand);

//	private:
//		CWebViewInput* m_pWebViewInput;
	};
} // namespace vgui

#endif // INPUT_SLATE_H
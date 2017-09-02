#ifndef TOAST_SLATE_H
#define TOAST_SLATE_H

class IToastSlate
{
public:
	virtual void		Create(vgui::VPANEL parent) = 0;
	virtual void		Destroy(void) = 0;
	virtual vgui::Panel*		GetPanel() = 0;
	virtual void Update() = 0;
};

extern IToastSlate* ToastSlate;

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>

namespace vgui
{
	class CToastSlate : public Frame
	{
		DECLARE_CLASS_SIMPLE(CToastSlate, Frame);

	public:
		CToastSlate(vgui::VPANEL parent);
		virtual ~CToastSlate();

		//void OnTick();
		void Update();
		void SelfDestruct();
		void PaintBackground();

		void OnCursorMoved(int x, int y);
		void OnMouseWheeled(int delta);
		void OnMousePressed(MouseCode code);
		void OnMouseReleased(MouseCode code);
		void OnMouseDoublePressed(MouseCode code);

		void OnKeyCodePressed(KeyCode code);
		void OnKeyCodeReleased(KeyCode code);

		vgui::Panel* GetPanel();

	private:
		Label* m_pLabel;
		Label* m_pHoverLabel;

	protected:
		void OnCommand(const char* pcCommand);

	private:
	};
} // namespace vgui

#endif // TOAST_SLATE_H
#ifndef EARLY_ERROR_H
#define EARLY_ERROR_H

class IEarlyError
{
public:
	virtual void		Create(vgui::VPANEL parent, const char* msg) = 0;
	virtual void		Destroy(void) = 0;
	virtual vgui::Panel*		GetPanel() = 0;
	virtual void Update() = 0;
};

extern IEarlyError* EarlyError;

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>

namespace vgui
{
	class CEarlyError : public Frame
	{
		DECLARE_CLASS_SIMPLE(CEarlyError, Frame);

	public:
		CEarlyError(vgui::VPANEL parent, const char* msg);
		virtual ~CEarlyError();

		//void OnTick() {};
		void Update() {};
		//void SelfDestruct();
		//void PaintBackground();
	//	void Paint();

		//void OnCursorMoved(int x, int y);
		//void OnMouseWheeled(int delta) {};
		//void OnMousePressed(MouseCode code);
		//void OnMouseReleased(MouseCode code);
		//void OnMouseDoublePressed(MouseCode code);

		//void OnKeyCodePressed(KeyCode code);
		//void OnKeyCodeReleased(KeyCode code);

		vgui::Panel* GetPanel();

	private:

	protected:
		void OnCommand(const char* pcCommand);

	private:

	};
} // namespace vgui

#endif // EARLY_ERROR_H
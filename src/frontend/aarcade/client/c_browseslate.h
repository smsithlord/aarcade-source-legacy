#ifndef BROWSE_SLATE_H
#define BROWSE_SLATE_H

//#include "c_webViewManager.h"
#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <string>
//#include <vgui_controls/TextEntry.h>
//#include <vgui_controls/Button.h>
//#include <vgui_controls/ComboBox.h>

//#include <string>

//#include "UIPanel.h"

//#include "Awesomium_Panel.h"

/*
struct ThreadedFolderBrowseParams_t
{
	//CChangeItem* listener;
	//std::string keyFieldName;
	//KeyValues* itemKV;
	std::string response;
	bool ready;
};
*/

struct ThreadedFileBrowseParams_t
{
	//CChangeItem* listener;
	//std::string keyFieldName;
	//KeyValues* itemKV;
	//CLibraryBrowser* browser;
	std::string response;
	bool ready;
};

class IBrowseSlate
{
public:
	virtual void		Create(vgui::VPANEL parent, std::string browseId) = 0;
	virtual void		Destroy(void) = 0;
	//	virtual vgui::Panel*		GetPanel() = 0;
};

extern IBrowseSlate* BrowseSlate;

namespace vgui
{
	class CBrowseSlate : public Frame
	{
		DECLARE_CLASS_SIMPLE(CBrowseSlate, Frame);

	public:
		CBrowseSlate(vgui::VPANEL parent, std::string browseId);
		virtual ~CBrowseSlate();

		//static ITexture* s_pOriginalTexture;
	//	static IMaterial* s_pMaterial;

		void OnTick();

	//	void SelfDestruct();
		//void PaintBackground();

		//void OnCursorMoved(int x, int y);
		//void OnMouseWheeled(int delta);
		//void OnMousePressed(MouseCode code);
		//void OnMouseReleased(MouseCode code);
		//void OnMouseDoublePressed(MouseCode code);

//		void ForceClose();

	//	void OnKeyCodePressed(KeyCode code);
	//	void OnKeyCodeReleased(KeyCode code);

	//	void SetFullscreenMode(bool bFullscreenMode);

	//	vgui::Panel* GetPanel();

	private:
		bool m_bIsInvisible;
		std::string m_browseId;
		ThreadedFileBrowseParams_t* m_pFileParams;
//		bool m_bMainMenu;
//		bool m_bFullscreen;
//		static long m_fPreviousTime;

	protected:
		void OnCommand(const char* pcCommand);
		//virtual void PaintBackground();

	private:
//		ITexture* m_pCanvasTexture;
//		bool m_bCursorAlphaZero;	// different than actually hiding the cursor
//		bool m_bCursorHidden;
//		ITexture* m_pOriginalTexture;
//		IMaterial* m_pMaterial;
//		CWebViewInput* m_pWebViewInput;
	};
} // namespace vgui

#endif // BROWSE_SLATE_H
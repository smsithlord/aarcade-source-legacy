#ifndef C_UI_TOGGLE_HANDLER_H
#define C_UI_TOGGLE_HANDLER_H

// This class exists because including Anarchy Arcade can cause namespace conflicts in some CPP files!!

class C_UIToggleHandler
{
public:
	C_UIToggleHandler();
	~C_UIToggleHandler();

	bool HandleUiToggle();
};

extern C_UIToggleHandler* g_pUIToggleHandler;

#endif
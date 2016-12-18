#include "cbase.h"

//#include "aa_globals.h"
#include "c_embeddedinstance.h"
//#include "c_anarchymanager.h"
//#include "inputsystem/iinputsystem.h"


#include "vgui/VGUI.h"
//#include "vgui/IInput.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

std::map<ButtonCode_t, const char*> C_EmbeddedInstance::s_buttonMap;
std::map<ButtonCode_t, const char*> C_EmbeddedInstance::s_shiftedButtonMap;

C_EmbeddedInstance::C_EmbeddedInstance()
{
	DevMsg("EmbeddedInstance: Constructor\n");

	if (s_buttonMap.find(KEY_0) == s_buttonMap.end())
	{
		s_buttonMap[KEY_0] = "0";
		s_shiftedButtonMap[KEY_0] = ")";

		s_buttonMap[KEY_1] = "1";
		s_shiftedButtonMap[KEY_1] = "!";

		s_buttonMap[KEY_2] = "2";
		s_shiftedButtonMap[KEY_2] = "@";

		s_buttonMap[KEY_3] = "3";
		s_shiftedButtonMap[KEY_3] = "#";

		s_buttonMap[KEY_4] = "4";
		s_shiftedButtonMap[KEY_4] = "$";

		s_buttonMap[KEY_5] = "5";
		s_shiftedButtonMap[KEY_5] = "%";

		s_buttonMap[KEY_6] = "6";
		s_shiftedButtonMap[KEY_6] = "^";

		s_buttonMap[KEY_7] = "7";
		s_shiftedButtonMap[KEY_7] = "&";

		s_buttonMap[KEY_8] = "8";
		s_shiftedButtonMap[KEY_8] = "*";

		s_buttonMap[KEY_9] = "9";
		s_shiftedButtonMap[KEY_9] = "(";

		s_buttonMap[KEY_A] = "a";
		s_shiftedButtonMap[KEY_A] = "A";

		s_buttonMap[KEY_B] = "b";
		s_shiftedButtonMap[KEY_B] = "B";

		s_buttonMap[KEY_C] = "c";
		s_shiftedButtonMap[KEY_C] = "C";

		s_buttonMap[KEY_D] = "d";
		s_shiftedButtonMap[KEY_D] = "D";

		s_buttonMap[KEY_E] = "e";
		s_shiftedButtonMap[KEY_E] = "E";

		s_buttonMap[KEY_F] = "f";
		s_shiftedButtonMap[KEY_F] = "F";

		s_buttonMap[KEY_G] = "g";
		s_shiftedButtonMap[KEY_G] = "G";

		s_buttonMap[KEY_H] = "h";
		s_shiftedButtonMap[KEY_H] = "H";

		s_buttonMap[KEY_I] = "i";
		s_shiftedButtonMap[KEY_I] = "I";

		s_buttonMap[KEY_J] = "j";
		s_shiftedButtonMap[KEY_J] = "J";

		s_buttonMap[KEY_K] = "k";
		s_shiftedButtonMap[KEY_K] = "K";

		s_buttonMap[KEY_L] = "l";
		s_shiftedButtonMap[KEY_L] = "L";

		s_buttonMap[KEY_M] = "m";
		s_shiftedButtonMap[KEY_M] = "M";

		s_buttonMap[KEY_N] = "n";
		s_shiftedButtonMap[KEY_N] = "N";

		s_buttonMap[KEY_O] = "o";
		s_shiftedButtonMap[KEY_O] = "O";

		s_buttonMap[KEY_P] = "p";
		s_shiftedButtonMap[KEY_P] = "P";

		s_buttonMap[KEY_Q] = "q";
		s_shiftedButtonMap[KEY_Q] = "Q";

		s_buttonMap[KEY_R] = "r";
		s_shiftedButtonMap[KEY_R] = "R";

		s_buttonMap[KEY_S] = "s";
		s_shiftedButtonMap[KEY_S] = "S";

		s_buttonMap[KEY_T] = "t";
		s_shiftedButtonMap[KEY_T] = "T";

		s_buttonMap[KEY_U] = "u";
		s_shiftedButtonMap[KEY_U] = "U";

		s_buttonMap[KEY_V] = "v";
		s_shiftedButtonMap[KEY_V] = "V";

		s_buttonMap[KEY_W] = "w";
		s_shiftedButtonMap[KEY_W] = "W";

		s_buttonMap[KEY_X] = "x";
		s_shiftedButtonMap[KEY_X] = "X";

		s_buttonMap[KEY_Y] = "y";
		s_shiftedButtonMap[KEY_Y] = "Y";

		s_buttonMap[KEY_Z] = "z";
		s_shiftedButtonMap[KEY_Z] = "Z";

		s_buttonMap[KEY_PAD_0] = "0";
		s_shiftedButtonMap[KEY_PAD_0] = "0";

		s_buttonMap[KEY_PAD_1] = "1";
		s_shiftedButtonMap[KEY_PAD_1] = "1";

		s_buttonMap[KEY_PAD_2] = "2";
		s_shiftedButtonMap[KEY_PAD_2] = "2";

		s_buttonMap[KEY_PAD_3] = "3";
		s_shiftedButtonMap[KEY_PAD_3] = "3";

		s_buttonMap[KEY_PAD_4] = "4";
		s_shiftedButtonMap[KEY_PAD_4] = "4";

		s_buttonMap[KEY_PAD_5] = "5";
		s_shiftedButtonMap[KEY_PAD_5] = "5";

		s_buttonMap[KEY_PAD_6] = "6";
		s_shiftedButtonMap[KEY_PAD_6] = "6";

		s_buttonMap[KEY_PAD_7] = "7";
		s_shiftedButtonMap[KEY_PAD_7] = "7";

		s_buttonMap[KEY_PAD_8] = "8";
		s_shiftedButtonMap[KEY_PAD_8] = "8";

		s_buttonMap[KEY_PAD_9] = "9";
		s_shiftedButtonMap[KEY_PAD_9] = "9";

		s_buttonMap[KEY_PAD_DIVIDE] = "/";
		s_shiftedButtonMap[KEY_PAD_DIVIDE] = "/";

		s_buttonMap[KEY_PAD_MULTIPLY] = "*";
		s_shiftedButtonMap[KEY_PAD_MULTIPLY] = "*";

		s_buttonMap[KEY_PAD_MINUS] = "-";
		s_shiftedButtonMap[KEY_PAD_MINUS] = "-";

		s_buttonMap[KEY_PAD_PLUS] = "+";
		s_shiftedButtonMap[KEY_PAD_PLUS] = "+";

		s_buttonMap[KEY_PAD_ENTER] = "\r";
		s_shiftedButtonMap[KEY_PAD_ENTER] = "\r";

		s_buttonMap[KEY_PAD_DECIMAL] = ".";
		s_shiftedButtonMap[KEY_PAD_DECIMAL] = ".";

		s_buttonMap[KEY_LBRACKET] = "[";
		s_shiftedButtonMap[KEY_LBRACKET] = "{";

		s_buttonMap[KEY_RBRACKET] = "]";
		s_shiftedButtonMap[KEY_RBRACKET] = "}";

		s_buttonMap[KEY_SEMICOLON] = ";";
		s_shiftedButtonMap[KEY_SEMICOLON] = ":";

		s_buttonMap[KEY_APOSTROPHE] = "'";
		s_shiftedButtonMap[KEY_APOSTROPHE] = "\"";

		s_buttonMap[KEY_BACKQUOTE] = "`";
		s_shiftedButtonMap[KEY_BACKQUOTE] = "~";

		s_buttonMap[KEY_COMMA] = ",";
		s_shiftedButtonMap[KEY_COMMA] = "<";

		s_buttonMap[KEY_PERIOD] = ".";
		s_shiftedButtonMap[KEY_PERIOD] = ">";

		s_buttonMap[KEY_SLASH] = "/";
		s_shiftedButtonMap[KEY_SLASH] = "?";

		s_buttonMap[KEY_BACKSLASH] = "\\";
		s_shiftedButtonMap[KEY_BACKSLASH] = "|";

		s_buttonMap[KEY_MINUS] = "-";
		s_shiftedButtonMap[KEY_MINUS] = "_";

		s_buttonMap[KEY_EQUAL] = "=";
		s_shiftedButtonMap[KEY_EQUAL] = "+";

		s_buttonMap[KEY_ENTER] = "\r";
		s_shiftedButtonMap[KEY_ENTER] = "\r";

		s_buttonMap[KEY_SPACE] = " ";
		s_shiftedButtonMap[KEY_SPACE] = " ";

		s_buttonMap[KEY_BACKSPACE] = "";
		s_shiftedButtonMap[KEY_BACKSPACE] = "";

		s_buttonMap[KEY_TAB] = "\t";
		s_shiftedButtonMap[KEY_TAB] = "\t";

		s_buttonMap[KEY_CAPSLOCK] = "";
		s_shiftedButtonMap[KEY_CAPSLOCK] = "";

		s_buttonMap[KEY_NUMLOCK] = "";
		s_shiftedButtonMap[KEY_NUMLOCK] = "";

		s_buttonMap[KEY_ESCAPE] = "";
		s_shiftedButtonMap[KEY_ESCAPE] = "";

		s_buttonMap[KEY_SCROLLLOCK] = "";
		s_shiftedButtonMap[KEY_SCROLLLOCK] = "";

		s_buttonMap[KEY_INSERT] = "";
		s_shiftedButtonMap[KEY_INSERT] = "";

		s_buttonMap[KEY_DELETE] = "";
		s_shiftedButtonMap[KEY_DELETE] = "";

		s_buttonMap[KEY_HOME] = "";
		s_shiftedButtonMap[KEY_HOME] = "";

		s_buttonMap[KEY_END] = "";
		s_shiftedButtonMap[KEY_END] = "";

		s_buttonMap[KEY_PAGEUP] = "";
		s_shiftedButtonMap[KEY_PAGEUP] = "";

		s_buttonMap[KEY_PAGEDOWN] = "";
		s_shiftedButtonMap[KEY_PAGEDOWN] = "";

		s_buttonMap[KEY_BREAK] = "";
		s_shiftedButtonMap[KEY_BREAK] = "";

		s_buttonMap[KEY_LSHIFT] = "";
		s_shiftedButtonMap[KEY_LSHIFT] = "";

		s_buttonMap[KEY_RSHIFT] = "";
		s_shiftedButtonMap[KEY_RSHIFT] = "";

		s_buttonMap[KEY_LALT] = "";
		s_shiftedButtonMap[KEY_LALT] = "";

		s_buttonMap[KEY_RALT] = "";
		s_shiftedButtonMap[KEY_RALT] = "";

		s_buttonMap[KEY_LCONTROL] = "";
		s_shiftedButtonMap[KEY_LCONTROL] = "";

		s_buttonMap[KEY_RCONTROL] = "";
		s_shiftedButtonMap[KEY_RCONTROL] = "";

		s_buttonMap[KEY_LWIN] = "";
		s_shiftedButtonMap[KEY_LWIN] = "";

		s_buttonMap[KEY_RWIN] = "";
		s_shiftedButtonMap[KEY_RWIN] = "";

		s_buttonMap[KEY_APP] = "";
		s_shiftedButtonMap[KEY_APP] = "";

		s_buttonMap[KEY_UP] = "";
		s_shiftedButtonMap[KEY_UP] = "";

		s_buttonMap[KEY_LEFT] = "";
		s_shiftedButtonMap[KEY_LEFT] = "";

		s_buttonMap[KEY_DOWN] = "";
		s_shiftedButtonMap[KEY_DOWN] = "";

		s_buttonMap[KEY_RIGHT] = "";
		s_shiftedButtonMap[KEY_RIGHT] = "";

		s_buttonMap[KEY_F1] = "";
		s_shiftedButtonMap[KEY_F1] = "";

		s_buttonMap[KEY_F2] = "";
		s_shiftedButtonMap[KEY_F2] = "";

		s_buttonMap[KEY_F3] = "";
		s_shiftedButtonMap[KEY_F3] = "";

		s_buttonMap[KEY_F4] = "";
		s_shiftedButtonMap[KEY_F4] = "";

		s_buttonMap[KEY_F5] = "";
		s_shiftedButtonMap[KEY_F5] = "";

		s_buttonMap[KEY_F6] = "";
		s_shiftedButtonMap[KEY_F6] = "";

		s_buttonMap[KEY_F7] = "";
		s_shiftedButtonMap[KEY_F7] = "";

		s_buttonMap[KEY_F8] = "";
		s_shiftedButtonMap[KEY_F8] = "";

		s_buttonMap[KEY_F9] = "";
		s_shiftedButtonMap[KEY_F9] = "";

		s_buttonMap[KEY_CAPSLOCKTOGGLE] = "";
		s_shiftedButtonMap[KEY_CAPSLOCKTOGGLE] = "";

		s_buttonMap[KEY_NUMLOCKTOGGLE] = "";
		s_shiftedButtonMap[KEY_NUMLOCKTOGGLE] = "";

		s_buttonMap[KEY_SCROLLLOCKTOGGLE] = "";
		s_shiftedButtonMap[KEY_SCROLLLOCKTOGGLE] = "";
	}

//	m_iLastRenderedFrame = 0;
}

C_EmbeddedInstance::~C_EmbeddedInstance()
{
	DevMsg("EmbeddedInstance: Destructor\n");
}

std::string C_EmbeddedInstance::GetId()
{
	DevMsg("ERROR: Base method called!\n");
	return "";
}

std::string C_EmbeddedInstance::GetOutput(vgui::KeyCode code, bool bShift, bool bCtrl, bool bAlt, bool bWin, bool bAutorepeat)
{
	std::string output = "";

	// only generate output w/ modifiers that don't kill text output
	if (!bCtrl && !bAlt && !bWin)
	{
		auto it = (bShift) ? s_shiftedButtonMap.find(code) : s_buttonMap.find(code);
		auto endIt = (bShift) ? s_shiftedButtonMap.end() : s_buttonMap.end();
		if (it != endIt)
			output = it->second;
	}

	return output.c_str();
}
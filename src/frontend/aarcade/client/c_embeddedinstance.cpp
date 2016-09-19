#include "cbase.h"

//#include "aa_globals.h"
#include "c_embeddedinstance.h"
//#include "c_anarchymanager.h"
//#include "inputsystem/iinputsystem.h"


#include "vgui/VGUI.h"
//#include "vgui/IInput.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_EmbeddedInstance::C_EmbeddedInstance()
{
	DevMsg("EmbeddedInstance: Constructor\n");
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

std::string C_EmbeddedInstance::GetCharTyped(vgui::KeyCode code, bool bShift, bool bCtrl, bool bAlt)
{
	bool shift = bShift;
	bool ctrl = bCtrl;
	bool alt = bAlt;
	
	std::string actualCharOutput = "";

	switch (code)
	{
	case KEY_0:
		if (shift)
			actualCharOutput = ')';
		else
			actualCharOutput = '0';
		break;

	case KEY_1:
		if (shift)
			actualCharOutput = '!';
		else
			actualCharOutput = '1';
		break;

	case KEY_2:
		if (shift)
			actualCharOutput = '@';
		else
			actualCharOutput = '2';
		break;

	case KEY_3:
		if (shift)
			actualCharOutput = '#';
		else
			actualCharOutput = '3';
		break;

	case KEY_4:
		if (shift)
			actualCharOutput = '$';
		else
			actualCharOutput = '4';
		break;

	case KEY_5:
		if (shift)
			actualCharOutput = '%';
		else
			actualCharOutput = '5';
		break;

	case KEY_6:
		if (shift)
			actualCharOutput = '^';
		else
			actualCharOutput = '6';
		break;

	case KEY_7:
		if (shift)
			actualCharOutput = '&';
		else
			actualCharOutput = '7';
		break;

	case KEY_8:
		if (shift)
			actualCharOutput = '*';
		else
			actualCharOutput = '8';
		break;

	case KEY_9:
		if (shift)
			actualCharOutput = '(';
		else
			actualCharOutput = '9';
		break;

	case KEY_A:
		if (shift)
			actualCharOutput = 'A';
		else
			actualCharOutput = 'a';
		break;

	case KEY_B:
		if (shift)
			actualCharOutput = 'B';
		else
			actualCharOutput = 'b';
		break;

	case KEY_C:
		if (shift)
			actualCharOutput = 'C';
		else
			actualCharOutput = 'c';
		break;

	case KEY_D:
		if (shift)
			actualCharOutput = 'D';
		else
			actualCharOutput = 'd';
		break;

	case KEY_E:
		if (shift)
			actualCharOutput = 'E';
		else
			actualCharOutput = 'e';
		break;

	case KEY_F:
		if (shift)
			actualCharOutput = 'F';
		else
			actualCharOutput = 'f';
		break;

	case KEY_G:
		if (shift)
			actualCharOutput = 'G';
		else
			actualCharOutput = 'g';
		break;

	case KEY_H:
		if (shift)
			actualCharOutput = 'H';
		else
			actualCharOutput = 'h';
		break;

	case KEY_I:
		if (shift)
			actualCharOutput = 'I';
		else
			actualCharOutput = 'i';
		break;

	case KEY_J:
		if (shift)
			actualCharOutput = 'J';
		else
			actualCharOutput = 'j';
		break;

	case KEY_K:
		if (shift)
			actualCharOutput = 'K';
		else
			actualCharOutput = 'k';
		break;

	case KEY_L:
		if (shift)
			actualCharOutput = 'L';
		else
			actualCharOutput = 'l';
		break;

	case KEY_M:
		if (shift)
			actualCharOutput = 'M';
		else
			actualCharOutput = 'm';
		break;

	case KEY_N:
		if (shift)
			actualCharOutput = 'N';
		else
			actualCharOutput = 'n';
		break;

	case KEY_O:
		if (shift)
			actualCharOutput = 'O';
		else
			actualCharOutput = 'o';
		break;

	case KEY_P:
		if (shift)
			actualCharOutput = 'P';
		else
			actualCharOutput = 'p';
		break;

	case KEY_Q:
		if (shift)
			actualCharOutput = 'Q';
		else
			actualCharOutput = 'q';
		break;

	case KEY_R:
		if (shift)
			actualCharOutput = 'R';
		else
			actualCharOutput = 'r';
		break;

	case KEY_S:
		if (shift)
			actualCharOutput = 'S';
		else
			actualCharOutput = 's';
		break;

	case KEY_T:
		if (shift)
			actualCharOutput = 'T';
		else
			actualCharOutput = 't';
		break;

	case KEY_U:
		if (shift)
			actualCharOutput = 'U';
		else
			actualCharOutput = 'u';
		break;

	case KEY_V:
		if (shift)
			actualCharOutput = 'V';
		else
			actualCharOutput = 'v';
		break;

	case KEY_W:
		if (shift)
			actualCharOutput = 'W';
		else
			actualCharOutput = 'w';
		break;

	case KEY_X:
		if (shift)
			actualCharOutput = 'X';
		else
			actualCharOutput = 'x';
		break;

	case KEY_Y:
		if (shift)
			actualCharOutput = 'Y';
		else
			actualCharOutput = 'y';
		break;

	case KEY_Z:
		if (shift)
			actualCharOutput = 'Z';
		else
			actualCharOutput = 'z';
		break;

	case KEY_PAD_0:
		actualCharOutput = '0';
		break;

	case KEY_PAD_1:
		actualCharOutput = '1';
		break;

	case KEY_PAD_2:
		actualCharOutput = '2';
		break;

	case KEY_PAD_3:
		actualCharOutput = '3';
		break;

	case KEY_PAD_4:
		actualCharOutput = '4';
		break;

	case KEY_PAD_5:
		actualCharOutput = '5';
		break;

	case KEY_PAD_6:
		actualCharOutput = '6';
		break;

	case KEY_PAD_7:
		actualCharOutput = '7';
		break;

	case KEY_PAD_8:
		actualCharOutput = '8';
		break;

	case KEY_PAD_9:
		actualCharOutput = '9';
		break;

	case KEY_PAD_DIVIDE:
		actualCharOutput = '/';
		break;

	case KEY_PAD_MULTIPLY:
		actualCharOutput = '*';
		break;

	case KEY_PAD_MINUS:
		actualCharOutput = '-';
		break;

	case KEY_PAD_PLUS:
		actualCharOutput = '+';
		break;

	case KEY_PAD_ENTER:
		actualCharOutput = '\r';
		break;

	case KEY_PAD_DECIMAL:
		actualCharOutput = '.';
		break;

	case KEY_LBRACKET:
		if (shift)
			actualCharOutput = '{';
		else
			actualCharOutput = '[';
		break;

	case KEY_RBRACKET:
		if (shift)
			actualCharOutput = '}';
		else
			actualCharOutput = ']';
		break;

	case KEY_SEMICOLON:
		if (shift)
			actualCharOutput = ':';
		else
			actualCharOutput = ';';
		break;

	case KEY_APOSTROPHE:
		if (shift)
			actualCharOutput = '"';
		else
			actualCharOutput = '\'';
		break;

	case KEY_BACKQUOTE:
		if (shift)
			actualCharOutput = '~';
		else
			actualCharOutput = '`';
		break;

	case KEY_COMMA:
		if (shift)
			actualCharOutput = '<';
		else
			actualCharOutput = ',';
		break;

	case KEY_PERIOD:
		if (shift)
			actualCharOutput = '>';
		else
			actualCharOutput = '.';
		break;

	case KEY_SLASH:
		if (shift)
			actualCharOutput = '?';
		else
			actualCharOutput = '/';
		break;

	case KEY_BACKSLASH:
		if (shift)
			actualCharOutput = '|';
		else
			actualCharOutput = '\\';
		break;

	case KEY_MINUS:
		if (shift)
			actualCharOutput = '_';
		else
			actualCharOutput = '-';
		break;

	case KEY_EQUAL:
		if (shift)
			actualCharOutput = '+';
		else
			actualCharOutput = '=';
		break;

	case KEY_ENTER:
		actualCharOutput = '\r';
		break;

	case KEY_SPACE:
		actualCharOutput = ' ';
		break;

	case KEY_BACKSPACE:
		break;

	case KEY_TAB:
		break;

	case KEY_CAPSLOCK:
		break;

	case KEY_NUMLOCK:
		break;

		//		case KEY_ESCAPE:
		//			virtualKeyCode = 0x1B;
		//			DevMsg("ESCAPE PRESSED!\n");
		//			break;

	case KEY_SCROLLLOCK:
		break;

	case KEY_INSERT:
		break;

	case KEY_DELETE:
		break;

	case KEY_HOME:
		break;

	case KEY_END:
		break;

	case KEY_PAGEUP:
		break;

	case KEY_PAGEDOWN:
		break;

	case KEY_LSHIFT:
		break;

	case KEY_RSHIFT:
		break;

	case KEY_LALT:
		break;

	case KEY_RALT:
		break;

	case KEY_LCONTROL:
		break;

	case KEY_RCONTROL:
		break;

	case KEY_LWIN:
		break;

	case KEY_RWIN:
		break;

	case KEY_APP:
		break;

	case KEY_UP:
		break;

	case KEY_LEFT:
		break;

	case KEY_DOWN:
		break;

	case KEY_RIGHT:
		break;
	}

	if (actualCharOutput.length() > 0)
	{
		char outputChar = actualCharOutput.at(0);
		actualCharOutput = outputChar;
	}

	bool hasChar = false;
	
	// If this key generates text output...
	if (actualCharOutput != "")
		hasChar = true;

	if (hasChar)
		return actualCharOutput;
	else
		return "";
}
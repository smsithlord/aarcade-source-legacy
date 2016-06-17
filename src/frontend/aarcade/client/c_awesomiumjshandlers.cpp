#include "cbase.h"

#include "c_awesomiumjshandlers.h"
#include "c_anarchymanager.h"
//#include "aa_globals.h"
//#include "Filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/*
namespace Awesomium
{
	const char* WebStringToCharString(WebString web_string)
	{
		int len = web_string.ToUTF8(null, 0);
		char* buf = new char[len + 1];
		web_string.ToUTF8(buf, len);
		buf[len] = 0;	// null terminator

		std::string title = buf;
		delete[] buf;

		return VarArgs("%s", title.c_str());
	}
}
*/


void JSHandler::OnMethodCall(WebView* caller, unsigned int remote_object_id, const WebString& method_name, const JSArray& args)
{
	if (method_name == WSLit("notifyPending"))
	{
		DevMsg("Msg recieved!!\n");
	}
}

JSValue JSHandler::OnMethodCallWithReturnValue(WebView* caller, unsigned int remote_object_id, const WebString &method_name, const JSArray &args)
{
	return WSLit("0");
}
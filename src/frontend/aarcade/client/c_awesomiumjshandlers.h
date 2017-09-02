#ifndef C_AWESOMIUM_JS_HANDLERS_H
#define C_AWESOMIUM_JS_HANDLERS_H

#include <Awesomium/WebCore.h>
#include <Awesomium/STLHelpers.h>
#include <string>
#include <vector>
//#include <map>
//#include "vgui_controls/Controls.h"

using namespace Awesomium;

struct JavaScriptMethodCall_t
{
	std::string objectName;
	std::string methodName;
	std::vector<std::string> methodArguments;
};

class JSHandler : public JSMethodHandler
{
public:
	JSHandler();
	virtual ~JSHandler() {};
	virtual void OnMethodCall(WebView* caller, unsigned int remote_object_id, const WebString& method_name, const JSArray& args);
	virtual JSValue OnMethodCallWithReturnValue(WebView* caller, unsigned int remote_object_id, const WebString &method_name, const JSArray &args);
//private:
//	std::map<std::string, KeyValues*>::iterator* m_pPreviousLibraryItemIt;
};

#endif
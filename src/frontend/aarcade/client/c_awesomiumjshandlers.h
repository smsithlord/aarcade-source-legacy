#ifndef C_AWESOMIUM_JS_HANDLERS_H
#define C_AWESOMIUM_JS_HANDLERS_H

#include <Awesomium/WebCore.h>
#include <Awesomium/STLHelpers.h>
//#include "vgui_controls/Controls.h"

using namespace Awesomium;

class JSHandler : public JSMethodHandler
{
public:
	JSHandler() {};
	virtual ~JSHandler() {};
	virtual void OnMethodCall(WebView* caller, unsigned int remote_object_id, const WebString& method_name, const JSArray& args);
	virtual JSValue OnMethodCallWithReturnValue(WebView* caller, unsigned int remote_object_id, const WebString &method_name, const JSArray &args);
};

#endif
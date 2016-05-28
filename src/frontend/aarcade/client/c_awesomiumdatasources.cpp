#include "cbase.h"

#include "c_awesomiumdatasources.h"
#include "c_anarchymanager.h"
//#include "aa_globals.h"
//#include "Filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace Awesomium;

//namespace Awesomium
//{
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
//}

NewWindowDataSource::NewWindowDataSource()
{
	DevMsg("NewWindowDataSource: Constructor\n");
}

NewWindowDataSource::~NewWindowDataSource()
{
	DevMsg("NewWindowDataSource: Destructor\n");
}

void NewWindowDataSource::OnRequest(int request_id, const ResourceRequest& request, const WebString& path)
{
	DevMsg("NewWindowDataSource: OnRequest: %s\n", WebStringToCharString(path));

	std::string html = "<html><body></body></html>";
	SendResponse(request_id, strlen(html.c_str()), (unsigned char*)html.c_str(), WSLit("text/html"));
}
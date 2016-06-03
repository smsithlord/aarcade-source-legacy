#include "cbase.h"

#include "c_awesomiumdatasources.h"
#include "c_anarchymanager.h"
#include <regex>
#include <algorithm>
#include "Filesystem.h"
//#include "aa_globals.h"
//#include "Filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace Awesomium;

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

UiDataSource::UiDataSource()
{
	DevMsg("UiDataSource: Constructor\n");
}

UiDataSource::~UiDataSource()
{
	DevMsg("UiDataSource: Destructor\n");
}

void UiDataSource::OnRequest(int request_id, const ResourceRequest& request, const WebString& path)
{
	DevMsg("UiDataSource: OnRequest: %s\n", WebStringToCharString(path));

	std::string requestPath = WebStringToCharString(path);	// everything after asset://ui/
	std::string requestUrl = WebStringToCharString(request.url().spec());	// the entire Url

	enum datatype_t {
		RESOURCE_UNKNOWN = 0,
		RESOURCE_BINARY = 1,
		RESOURCE_TEXT = 2
	};

	// determine the resource type
	datatype_t datatype = RESOURCE_UNKNOWN;

	std::string fileExtension = requestPath;
	size_t foundLastDot = fileExtension.find_last_of(".");
	fileExtension = fileExtension.substr(foundLastDot + 1);
	std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), tolower);

	std::string binaryExtensions = "jpg, jpeg, tbn, png, gif, tbn, tga";
	
	std::vector<std::string> tokens;
	g_pAnarchyManager->Tokenize(binaryExtensions, tokens, ", ");
	std::vector<std::string>::iterator foundToken = std::find(tokens.begin(), tokens.end(), fileExtension);
	if (foundToken != tokens.end())
	{
		DevMsg("Found binary extension!\n");
		datatype = RESOURCE_BINARY;
	}
	else
	{
		DevMsg("Found text extension!\n");
		datatype = RESOURCE_TEXT;
	}

	if (datatype == RESOURCE_TEXT)
	{
		if (requestPath.find(".html") == requestPath.length() - 5)
		{
			CUtlBuffer buf;
			if (filesystem->ReadFile(requestPath.c_str(), "UI", buf))
			{
				char* data = new char[buf.Size() + 1];
				//buf.GetString(data);
				buf.GetStringManualCharCount(data, buf.Size());
				data[buf.Size()] = 0; // null terminator

				std::string generatedPage = data;

				delete[] data;

				SendResponse(request_id, strlen(generatedPage.c_str()), (unsigned char*)generatedPage.c_str(), WSLit("text/html"));
			}
		}
		else if (requestPath.find(".js") == requestPath.length() - 3)
		{
			CUtlBuffer buf;
			if (filesystem->ReadFile(requestPath.c_str(), "UI", buf))
			{
				char* data = new char[buf.Size() + 1];
				//buf.GetString(data);
				buf.GetStringManualCharCount(data, buf.Size());
				data[buf.Size()] = 0; // null terminator

				std::string loadedContent = data;

				delete[] data;

				SendResponse(request_id, strlen(loadedContent.c_str()), (unsigned char*)loadedContent.c_str(), WSLit("application/javascript"));
			}
		}
		else if (requestPath.find(".css") == requestPath.length() - 4)
		{
			FileHandle_t fileHandle = filesystem->Open(requestPath.c_str(), "r", "UI");

			if (fileHandle)
			{
				int bufferSize = filesystem->Size(fileHandle);
				unsigned char* responseBuffer = new unsigned char[bufferSize + 1];

				filesystem->Read((void*)responseBuffer, bufferSize, fileHandle);
				responseBuffer[bufferSize] = 0; // null terminator

				filesystem->Close(fileHandle);

				SendResponse(request_id, bufferSize, responseBuffer, WSLit("text/css"));

				delete[] responseBuffer;
			}
		}
		else
		{
			/*	This method probably only works for non-binary files.
			FileHandle_t fileHandle = filesystem->Open(localFile.c_str(), "rb", "AAPROTECTED");

			if( fileHandle )
			{
			int bufferSize = filesystem->Size(fileHandle);
			unsigned char* responseBuffer = new unsigned char[bufferSize + 1];

			filesystem->Read((void*)responseBuffer, bufferSize, fileHandle);
			responseBuffer[bufferSize] = 0; // null terminator

			filesystem->Close(fileHandle);

			SendResponse(request_id, bufferSize, responseBuffer, WSLit("image"));

			delete[] responseBuffer;
			}
			*/

			FileHandle_t fileHandle = filesystem->Open(requestPath.c_str(), "rb", "UI");

			if (fileHandle)
			{
				int bufferSize = filesystem->Size(fileHandle);
				unsigned char* responseBuffer = new unsigned char[bufferSize + 1];

				filesystem->Read((void*)responseBuffer, bufferSize, fileHandle);
				responseBuffer[bufferSize] = 0; // null terminator

				filesystem->Close(fileHandle);

				SendResponse(request_id, bufferSize + 1, responseBuffer, WSLit("image"));

				delete[] responseBuffer;
			}
		}
	}
}
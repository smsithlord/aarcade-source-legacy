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
/*
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
*/
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
//	DevMsg("UiDataSource: OnRequest: %s\n", WebStringToCharString(path));

	std::string requestPath = WebStringToCharString(path);	// everything after asset://ui/
	std::string requestUrl = WebStringToCharString(request.url().spec());	// the entire Url

	DevMsg("UiDataSource: OnRequest: %s from %s\n", requestPath.c_str(), requestUrl.c_str());
	if (requestPath == "undefined")
	{
		SendResponse(request_id, 0, null, WSLit("text/html"));
		return;
	}

	size_t found = requestPath.find_first_of("#?");
	std::string shortPath = requestPath.substr(0, found);

	enum datatype_t {
		RESOURCE_UNKNOWN = 0,
		RESOURCE_BINARY = 1,
		RESOURCE_TEXT = 2
	};

	// determine the resource type
	datatype_t datatype = RESOURCE_UNKNOWN;

	std::string fileExtension = shortPath;
	size_t foundLastDot = fileExtension.find_last_of(".");
	fileExtension = fileExtension.substr(foundLastDot + 1);
	std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), tolower);

	bool bIsFontTFF = false;
	bool bIsFontWOFF = false;
	bool bIsImage = false;
	std::string binaryExtensions = "jpg, jpeg, tbn, png, gif, tbn, tga";
	
	std::vector<std::string> tokens;
	g_pAnarchyManager->Tokenize(binaryExtensions, tokens, ", ");
	std::vector<std::string>::iterator foundToken = std::find(tokens.begin(), tokens.end(), fileExtension);
	if (fileExtension == "ttf")
	{
		bIsFontTFF = true;
		datatype = RESOURCE_BINARY;
	}
	else if (fileExtension == "woff")
	{
		bIsFontWOFF = true;
		datatype = RESOURCE_BINARY;
	}
	else if (foundToken != tokens.end())
	{
		bIsImage = true;
		datatype = RESOURCE_BINARY;
	}
	else
		datatype = RESOURCE_TEXT;

	if (datatype == RESOURCE_TEXT)
	{
		if (shortPath.find(".html") == shortPath.length() - 5)
		{
			CUtlBuffer buf;
			if (filesystem->ReadFile(shortPath.c_str(), "UI", buf))
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
		else if (shortPath.find(".js") == shortPath.length() - 3)
		{
			CUtlBuffer buf;
			if (filesystem->ReadFile(shortPath.c_str(), "UI", buf))
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
		else if (shortPath.find(".css") == shortPath.length() - 4)
		{
			FileHandle_t fileHandle = filesystem->Open(shortPath.c_str(), "r", "UI");

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
//		else
	//	{
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
		/*
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
			*/
		//}
	}
	else if (datatype == RESOURCE_BINARY)
	{
		FileHandle_t fileHandle = filesystem->Open(shortPath.c_str(), "rb", "UI");

		if (fileHandle)
		{
			int bufferSize = filesystem->Size(fileHandle);
			unsigned char* responseBuffer = new unsigned char[bufferSize + 1];

			filesystem->Read((void*)responseBuffer, bufferSize, fileHandle);
			responseBuffer[bufferSize] = 0; // null terminator

			filesystem->Close(fileHandle);

			if ( bIsImage )
				SendResponse(request_id, bufferSize + 1, responseBuffer, WSLit("image"));
			else if (bIsFontTFF)
				SendResponse(request_id, bufferSize + 1, responseBuffer, WSLit("application/x-font-ttf"));
			else if (bIsFontWOFF)
				SendResponse(request_id, bufferSize + 1, responseBuffer, WSLit("application/x-font-woff"));

			delete[] responseBuffer;
		}
	}
}

ScreenshotDataSource::ScreenshotDataSource()
{
	DevMsg("ScreenshotDataSource: Constructor\n");
}

ScreenshotDataSource::~ScreenshotDataSource()
{
	DevMsg("ScreenshotDataSource: Destructor\n");
}

void ScreenshotDataSource::OnRequest(int request_id, const ResourceRequest& request, const WebString& path)
{
	DevMsg("ScreenshotDataSource: OnRequest: %s\n", WebStringToCharString(path));

	std::string requestPath = WebStringToCharString(path);	// everything after asset://ui/
	std::string requestUrl = WebStringToCharString(request.url().spec());	// the entire Url

	size_t found = requestPath.find_first_of("#?");
	std::string shortPath = "screenshots\\" + requestPath.substr(0, found);

	enum datatype_t {
		RESOURCE_UNKNOWN = 0,
		RESOURCE_BINARY = 1,
		RESOURCE_TEXT = 2
	};

	// determine the resource type
	datatype_t datatype = RESOURCE_UNKNOWN;

	std::string fileExtension = shortPath;
	size_t foundLastDot = fileExtension.find_last_of(".");
	fileExtension = fileExtension.substr(foundLastDot + 1);
	std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), tolower);

	std::string binaryExtensions = "jpg jpeg tbn png gif tbn tga";

	std::vector<std::string> tokens;
	g_pAnarchyManager->Tokenize(binaryExtensions, tokens, " ");
	std::vector<std::string>::iterator foundToken = std::find(tokens.begin(), tokens.end(), fileExtension);
	if (foundToken != tokens.end())
		datatype = RESOURCE_BINARY;
	else
		datatype = RESOURCE_TEXT;

	if (datatype == RESOURCE_TEXT)
	{
		if (shortPath.find(".html") == shortPath.length() - 5)
		{
			CUtlBuffer buf;
			if (filesystem->ReadFile(shortPath.c_str(), "MOD", buf))
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
		else if (shortPath.find(".js") == shortPath.length() - 3)
		{
			CUtlBuffer buf;
			if (filesystem->ReadFile(shortPath.c_str(), "MOD", buf))
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
		else if (shortPath.find(".css") == shortPath.length() - 4)
		{
			FileHandle_t fileHandle = filesystem->Open(shortPath.c_str(), "r", "MOD");

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
		//		else
		//	{
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
		/*
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
		*/
		//}
	}
	else if (datatype == RESOURCE_BINARY)
	{
		FileHandle_t fileHandle = filesystem->Open(shortPath.c_str(), "rb", "MOD");

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
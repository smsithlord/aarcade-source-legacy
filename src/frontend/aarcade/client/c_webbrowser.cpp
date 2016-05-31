#include "cbase.h"

#include "c_webbrowser.h"
#include "c_anarchymanager.h"
#include <Awesomium/BitmapSurface.h>
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

	class MasterWebViewListenerLoad : public WebViewListener::Load
	{
		void OnBeginLoadingFrame(WebView* caller, int64 frame_id, bool is_main_frame, const WebURL& url, bool is_error_page) {};
		void OnFailLoadingFrame(WebView* caller, int64 frame_id, bool is_main_frame, const WebURL& url, int error_code, const WebString& error_desc) {};
		void OnFinishLoadingFrame(WebView* caller, int64 frame_id, bool is_main_frame, const WebURL& url) {};
		void OnDocumentReady(WebView* caller, const WebURL& url);

	public:
		void Init(C_WebViewManager* pWebViewManager) { m_pWebViewManager = pWebViewManager; }

	private:
		C_WebViewManager* m_pWebViewManager;
	};

	void MasterWebViewListenerLoad::OnDocumentReady(WebView* caller, const WebURL& url)
	{
		DevMsg("OnDocumentReady\n");
		DevMsg("JAR JAR2: %s\n", WebStringToCharString(url.spec()));
		if (!m_pWebViewManager->GetAwesomiumMasterWebView())
		{
			m_pWebViewManager->SetAwesomiumMasterWebView(caller);
			DevMsg("Now spawn the dynamic view.\n");
			caller->ExecuteJavascript(WSLit("window.open('webui://dynamicImage.html', '', 'width=200,height=100');"), WSLit(""));
		}
		else if (caller == m_pWebViewManager->GetAwesomiumMasterWebView())
		{
			DevMsg("ERROR: MASTER WEB VIEW (AGAIN)\n");
		}
		else if (caller == m_pWebViewManager->GetAwesomiumDynamicImageWebView())
		{
			DevMsg("DYNAMIC IMAGE WEB VIEW\n");
		}
		else
		{
			DevMsg("OTHER WEB VIEW\n");
		}
	}

	class DynamicImageWebViewListenerView : public WebViewListener::View
	{
		void OnChangeTitle(WebView* caller, const WebString &title) {};
		void OnChangeAddressBar(WebView* caller, const WebURL &url) {};
		void OnChangeTooltip(WebView* caller, const WebString &tooltip) {};
		void OnChangeTargetURL(WebView* caller, const WebURL &url) {};
		void OnChangeCursor(WebView* caller, Cursor cursor) {};
		void OnChangeFocus(WebView* caller, FocusedElementType focused_type) {};
		void OnAddConsoleMessage(WebView* caller, const WebString &message, int line_number, const WebString &source);
		void OnShowCreatedWebView(WebView* caller, WebView* new_view, const WebURL &opener_url, const WebURL &target_url, const Rect &initial_pos, bool is_popup);

	public:
		void Init(C_WebViewManager* pWebViewManager) { m_pWebViewManager = pWebViewManager; }

	private:
		C_WebViewManager* m_pWebViewManager;
	};

	void DynamicImageWebViewListenerView::OnAddConsoleMessage(WebView* caller, const WebString &message, int line_number, const WebString &source)
	{
		//		C_ArcadeResources* pClientArcadeResources = C_ArcadeResources::GetSelf();
		//	if (pClientArcadeResources->GetWebLogConVar()->GetBool())
		DevMsg("DYNAMIC IMAGE WEB VIEW OUTPUT: %i\t%s\n", line_number, WebStringToCharString(message));
	}

	void DynamicImageWebViewListenerView::OnShowCreatedWebView(WebView *caller, WebView *new_view, const WebURL &opener_url, const WebURL &target_url, const Rect &initial_pos, bool is_popup)
	{
		//new_view->Destroy();
	}

	class MasterWebViewListenerView : public WebViewListener::View
	{
		void OnChangeTitle(WebView* caller, const WebString &title) {};
		void OnChangeAddressBar(WebView* caller, const WebURL &url) {};
		void OnChangeTooltip(WebView* caller, const WebString &tooltip) {};
		void OnChangeTargetURL(WebView* caller, const WebURL &url) {};
		void OnChangeCursor(WebView* caller, Cursor cursor) {};
		void OnChangeFocus(WebView* caller, FocusedElementType focused_type) {};
		void OnAddConsoleMessage(WebView* caller, const WebString &message, int line_number, const WebString &source);
		void OnShowCreatedWebView(WebView* caller, WebView* new_view, const WebURL &opener_url, const WebURL &target_url, const Rect &initial_pos, bool is_popup);

	public:
		void Init(C_WebViewManager* pWebViewManager) { m_pWebViewManager = pWebViewManager; }

	private:
		C_WebViewManager* m_pWebViewManager;
	};

	void MasterWebViewListenerView::OnAddConsoleMessage(WebView* caller, const WebString &message, int line_number, const WebString &source)
	{
//		C_ArcadeResources* pClientArcadeResources = C_ArcadeResources::GetSelf();
	//	if (pClientArcadeResources->GetWebLogConVar()->GetBool())
		DevMsg("WEB VIEW OUTPUT: %i\t%s\n", line_number, WebStringToCharString(message));
	}

	void MasterWebViewListenerView::OnShowCreatedWebView(WebView *caller, WebView *new_view, const WebURL &opener_url, const WebURL &target_url, const Rect &initial_pos, bool is_popup)
	{
		if (!m_pWebViewManager->GetAwesomiumDynamicImageWebView())
		{
			DevMsg("DYNAMIC IMAGE VIEW CREATED!!\n");
			//DevMsg("JAR JAR: %s\n", WebStringToCharString(target_url.spec()));
			//m_pWebViewManager->SetAwesomiumDynamicImageWebView(new_view);

			//MasterWebViewListenerLoad* pMasterWebViewListenerLoad = new MasterWebViewListenerLoad;
			//pMasterWebViewListenerLoad->Init(this);

			DynamicImageWebViewListenerView* pDynamicImageWebViewListenerView = new DynamicImageWebViewListenerView;
			pDynamicImageWebViewListenerView->Init(m_pWebViewManager);
			new_view->set_view_listener(pDynamicImageWebViewListenerView);

			m_pWebViewManager->GetDynamicImageWebView()->Activate(new_view);

//			DynamicImageWebViewListenerView* pDynamicImageWebViewListenerView = new DynamicImageWebViewListenerView;
	//		pDynamicImageWebViewListenerView->Init(m_pWebViewManager);
		//	new_view->set_view_listener(pDynamicImageWebViewListenerView);
		}
		else
			new_view->Destroy();
	}

	class DataSourceUI : public DataSource
	{
	public:
		DataSourceUI();
		virtual ~DataSourceUI();
		virtual void OnRequest(int request_id, const ResourceRequest& request, const WebString& path);
	};

	DataSourceUI::DataSourceUI()
	{

	}

	DataSourceUI::~DataSourceUI()
	{

	}

	void DataSourceUI::OnRequest(int request_id, const ResourceRequest& request, const WebString& path)
	{
		// Convert the path to a string
		int len = path.ToUTF8(null, 0);
		char* buf = new char[len + 1];
		path.ToUTF8(buf, len);
		buf[len] = 0;	// null terminator

		std::string requestPath = buf;
		delete[] buf;

		size_t foundAbruptEnd = requestPath.find_first_of("?#");
		if (foundAbruptEnd != std::string::npos)
			requestPath = requestPath.substr(0, foundAbruptEnd);

		//		std::string localFile = engine->GetGameDirectory();
		std::string localFile = "resource\\uihtml\\";
		localFile += requestPath;

		size_t foundQuestion = localFile.find("?");
		if (foundQuestion != std::string::npos)
			localFile = localFile.substr(0, foundQuestion);

		size_t foundPound = localFile.find("#");
		if (foundPound != std::string::npos)
			localFile = localFile.substr(0, foundPound);

		// If the local file exists within the resource folder or its an image, send it.
		size_t foundEnd = localFile.find_last_of(".");
		std::string ending = localFile.substr(foundEnd + 1);

		// Determine if it ends with an image extension
		std::string imageEndings = ".jpg.JPG.jpeg.JPEG.tbn.TBN.png.PNG.gif.GIF.tbn.TBN.tga.TGA.";
		size_t endsWithImage = imageEndings.find(VarArgs(".%s.", ending.c_str()));
		//		DevMsg("Local file request: %s\n", localFile.c_str());

		if (filesystem->FileExists(localFile.c_str(), "AAPROTECTED") || endsWithImage != std::string::npos)
		{
			if (localFile.find(".html") == localFile.length() - 5)
			{
				CUtlBuffer buf;
				if (filesystem->ReadFile(localFile.c_str(), "AAPROTECTED", buf))
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
			else if (localFile.find(".js") == localFile.length() - 3)
			{
				CUtlBuffer buf;
				if (filesystem->ReadFile(localFile.c_str(), "AAPROTECTED", buf))
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
			else if (localFile.find(".css") == localFile.length() - 4)
			{
				FileHandle_t fileHandle = filesystem->Open(localFile.c_str(), "r", "AAPROTECTED");

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
				FileHandle_t fileHandle = filesystem->Open(localFile.c_str(), "rb", "AAPROTECTED");

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
		else
		{
			DevMsg("Could not find local file for asset: %s\n", requestPath.c_str());
			SendResponse(request_id, 0, null, WSLit("image"));
		}
	}
}
*/

C_WebBrowser::C_WebBrowser()
{
	m_iState = 0;	// initialized
	m_pWebCore = null;
	m_pWebSession = null;
	m_pMasterWebView = null;
	m_pMasterLoadListener = null;
}

void C_WebBrowser::Init()
{
	DevMsg("WebBrowser: Init\n");
	m_iState = 1;	// initializing

	using namespace Awesomium;

	WebConfig config;

	config.log_level = kLogLevel_Normal;
	config.child_process_path = WSLit("./AArcadeWebview.exe");

	m_pWebCore = WebCore::Initialize(config);

	// Create the master web view
	Awesomium::WebPreferences prefs;
	prefs.enable_plugins = true;
	prefs.enable_smooth_scrolling = true;
	prefs.user_stylesheet = WSLit("body{ background-color: #000000; }");

	m_pWebSession = m_pWebCore->CreateWebSession(WSLit(VarArgs("%s\\cache", engine->GetGameDirectory())), prefs);

	NewWindowDataSource* pNewWindowDataSource = new NewWindowDataSource();
	m_pWebSession->AddDataSource(WSLit("newwindow"), pNewWindowDataSource);
	
	// MASTER
	m_pMasterLoadListener = new MasterLoadListener;
	m_pMasterViewListener = new MasterViewListener;

	// REGULAR
	m_pLoadListener = new LoadListener;
	m_pViewListener = new ViewListener;

	m_pMasterWebView = m_pWebCore->CreateWebView(1280, 720, m_pWebSession);
	m_pMasterWebView->set_load_listener(m_pMasterLoadListener);
	m_pMasterWebView->set_view_listener(m_pMasterViewListener);

	m_pMasterWebView->LoadURL(WebURL(WSLit("asset://newwindow/master")));
}

void C_WebBrowser::PrepareWebView(Awesomium::WebView* pWebView)
{
	pWebView->Resize(1280, 720);
	pWebView->set_load_listener(m_pLoadListener);
	pWebView->set_view_listener(m_pViewListener);
}

void C_WebBrowser::OnMasterWebViewDocumentReady()
{
	m_iState = 2;	// initialized

	DevMsg("END OF THE WORLD!\n");
//	C_WebManager* pWebManager = g_pAnarchyManager->GetWebManager();
//	C_WebTab* pWebTab = pWebManager->CreateWebTab("http://www.smsithlord.com/");
}

void C_WebBrowser::CreateWebView(C_WebTab* pWebTab)
{
	DevMsg("WebBrowser: CreateWebView\n");

	std::string id = pWebTab->GetId();
	m_pMasterWebView->ExecuteJavascript(WSLit(VarArgs("window.open('asset://newwindow/%s', '', 'width=200,height=100');", id.c_str())), WSLit(""));
}

WebView* C_WebBrowser::FindWebView(C_WebTab* pWebTab)
{
	auto foundWebView = m_webViews.find(pWebTab);
	if (foundWebView != m_webViews.end())
		return m_webViews[pWebTab];
	else
		return null;
}

void C_WebBrowser::OnCreateWebViewDocumentReady(WebView* pWebView, std::string id)
{
	// The master webview has created a new webview on demand.
	DevMsg("WebBrowser: OnCreateWebViewDocumentReady: %s\n", id.c_str());

	// TODO: Add global JS API object to the web view.

	C_WebTab* pWebTab = g_pAnarchyManager->GetWebManager()->FindWebTab(id);
	if (pWebTab)
	{
		pWebTab->SetState(2);
		m_webViews[pWebTab] = pWebView;
		pWebView->LoadURL(WebURL(WSLit(pWebTab->GetInitialUrl().c_str())));
	}
}

void C_WebBrowser::Update()
{
	if (m_pWebCore)
		m_pWebCore->Update();
}

void C_WebBrowser::RegenerateTextureBits(C_WebTab* pWebTab, ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect)
{
	//DevMsg("WebBrowser: RegenerateTextureBits\n");

	WebView* pWebView = FindWebView(pWebTab);

	if (!pWebView)
		return;

	BitmapSurface* surface = static_cast<BitmapSurface*>(pWebView->surface());
	//BitmapSurface* surface = (BitmapSurface*)pWebView->surface();
	if (surface != 0)
		surface->CopyTo(pVTFTexture->ImageData(0, 0, 0), pSubRect->width * 4, 4, false, false);
		/*
		// BLACK SCREEN deal with square texture size!!
		if (m_iStrangeVideo == 2)
		{
			int sourceWidth = 1280;
			int sourceHeight = 720;
			int sourceStride = sourceWidth * 4;
			unsigned char* pSourceFrame = new unsigned char[sourceStride*sourceHeight];

			surface->CopyTo(pSourceFrame, sourceStride, 4, false, false);

			pClientArcadeResources->ResizeFrame(pSourceFrame, pVTFTexture->ImageData(0, 0, 0));

			delete[] pSourceFrame;
		}
		else
		{
			// do it regular
			surface->CopyTo(pVTFTexture->ImageData(0, 0, 0), pSubRect->width * 4, 4, false, false);
		}
		*/
}
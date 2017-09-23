arcadeHud.addScraper({
	"id": "vimeo",
	"api_version": 0.1,
	"title": "Vimeo",
	"summary": "Videos & Music",
	"description": "Vimeo is a site with streaming videos, much like YouTube.",
	"homepage": "http://www.vimeo.com/",
	"search": "http://vimeo.com/search?q=$TERM",
	"can_acquire": false,
	"allow_keywords": true,
	"fields":
	{
		"all": 100,
		"reference": 100,
		"file": 100,
		"preview": 100,
		"screen": 100,
		"stream": 100,
		"title": 100,
		"description": 100,
		"type": 100
	},
	"run": function(url, field, doc)
	{
		var response = {};

		var urlElem = doc.querySelector("meta[property='og:url']");
		var metaURL = urlElem.getAttribute("content");
		if( metaURL.indexOf("https://") === 0 )
			metaURL = "http://" + metaURL.substring(8);

		// reference
		response.reference = metaURL;

		// file
		response.file = metaURL;

		// stream
		response.stream = metaURL;

		// preview
		var previewBase = "http://player.vimeo.com/video/";

		var videoId = metaURL;
		videoId = videoId.substring(videoId.indexOf("vimeo.com/") + 10);

		var found = videoId.indexOf("/");
		if( found === -1 )
			found = videoId.indexOf("?");
		if( found === -1 )
			found = videoId.indexOf("&");
		if( found !== -1 )
			videoId = videoId.substring(0, found-1);

		response.preview = previewBase + videoId;

		// title
		var titleElem = doc.querySelector("meta[property='og:title']");
		response.title = titleElem.getAttribute("content");

		// description
		var descriptionElem = doc.querySelector("meta[property='og:description']");
		response.description = descriptionElem.getAttribute("content");
		
		// screen
		var screenElem = doc.querySelector("meta[property='og:image']");
		var screenURL = screenElem.getAttribute("content");
		if( screenURL.indexOf("https://") === 0 )
			screenURL = "http://" + screenURL.substring(8);
		response.screen = screenURL;

		// type
		response.type = "videos";

		return response;
	},
	"test": function(url, doc, callback)
	{		
		var testerLocation = document.createElement("a");
		testerLocation.href = url;

		//callback({"validForScrape": true, "redirect": false});
		var validForScrape = false;
		var redirect = false;

		if( testerLocation.hostname.indexOf("vimeo") >= 0 )
		{
			var metaURL = doc.querySelector("meta[property='og:video:url']");//.getAttribute("content");
			if( !!metaURL )
			{
				metaURL = metaURL.getAttribute("content");

				if( metaURL !== "" )
					validForScrape = true;
			}
		}

		callback({"validForScrape": validForScrape, "redirect": redirect});
	},
	"testDelay": 0,
	"runDelay": 0
});
arcadeHud.addScraper({
	"id": "youtube",
	"api_version": 0.1,
	"title": "YouTube",
	"homepage": "http://www.youtube.com/",
	"search": "http://www.youtube.com/results?search_query=$TERM",
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

		var videoId = doc.querySelector("meta[itemprop='videoId']").getAttribute("content");
		//var goodUri = "http://www.youtube.com/watch?v=" + videoId;
		var goodUri = doc.querySelector("link[itemprop='url']").getAttribute("href");

		// reference
		var goodRef = goodUri;
		var ytid = arcadeHud.extractYouTubeId(goodRef);
		var ytplaylist = arcadeHud.extractYouTubePlaylistId(goodRef);
		if( !!ytid )
		{
			if( !!ytplaylist )
				goodRef = "http://www.youtube.com/watch?v=" + ytid + "&index=1&list=" + ytplaylist;
			else
				goodRef = "http://www.youtube.com/watch?v=" + ytid;
		}
		response.reference = goodRef;

		// file
		response.file = goodUri;

		// preview
		response.preview = goodUri;
		
		// screen
		var screenElem = doc.querySelector("link[itemprop='thumbnailUrl']");
		response.screen = screenElem.getAttribute("href");

		// stream
		response.stream = goodUri;

		// title
		var titleElem = doc.querySelector("meta[itemprop='name']");
		response.title = titleElem.getAttribute("content");

		// description
		var descriptionElem = doc.querySelector("meta[itemprop='description']");
		response.description = descriptionElem.getAttribute("content");

		// type
		response.type = "videos";

		return response;
	},
	"test": function(url, doc, callback)
	{
		var testerLocation = document.createElement("a");
		testerLocation.href = url;

		var validForScrape = false;
		var redirect = false;

		if( testerLocation.hostname.indexOf("youtube") >= 0 )
		{
			var pageElem = doc.querySelector("#page");
			var pageType = pageElem.className;
			if( pageType.indexOf(" search ") >= 0 )
			{
				// perform search results logic
			}
			else if( pageType.indexOf(" watch ") >= 0 )
			{
				console.log("what meow");
				//var container = doc.querySelector(".metaScrapeContainer");
				//container.style.display = "block";


				/*
				backstab
				volitile
				lacerate
				lacerate
				root with knife throw
				try to backstab
				good shiv
				try to backstab
				lacerate
				lacerate
				volitile
				*/

				// hud-notify that this page can be scraped
				validForScrape = true;
			}
		}
		//<div id="page" class=" search branded-page-v2-secondary-column-wide no-flex">
		//console.log("You Tube is examining the page...");
		callback({"validForScrape": validForScrape, "redirect": redirect});
	},
	"testDelay": 2000,
	"runDelay": 0
});
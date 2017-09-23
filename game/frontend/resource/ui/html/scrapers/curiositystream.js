arcadeHud.addScraper({
	"id": "curiositystream",
	"api_version": 0.1,
	"title": "CuriosityStream",
	"summary": "TV & Documentaries",
	"description": "CuriosityStream is a subscription site with lots of short science documentaries & TV shows.",
	"homepage": "http://app.curiositystream.com/",
	"search": "http://app.curiositystream.com/search/$TERM",
	"can_acquire": false,
	"allow_keywords": false,
	"fields":
	{
		"all": 100,
		"reference": 100,
		"file": 100,
		"stream": 100,
		"title": 100,
		"description": 100,
		"screen": 100,
		"marquee": 100,
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

		// marquee
		response.marquee = screenURL;

		// type
		response.type = "videos";

		return response;
	},
	"test": function(url, doc, callback)
	{
		var validForScrape = false;
		var redirect = false;

		if( url.indexOf("curiositystream.com/video/") >= 0 )
			validForScrape = true;

		callback({"validForScrape": validForScrape, "redirect": redirect});
	},
	"testDelay": 100,
	"runDelay": 0
});
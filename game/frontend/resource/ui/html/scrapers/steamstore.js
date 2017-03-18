arcadeHud.addScraper({
	"id": "steamstore",
	"title": "Steam Store",
	"homepage": "http://store.steampowered.com/",
	"search": "http://store.steampowered.com/search/?term=$TERM",
	"fields":
	{
		"all": 100,
		"reference": 100,
		"description": 100,
		"title": 100,
		"screen": 100,
		"marquee": 100,
		"preview": 100,
		"file": 50
	},
	"run": function(url, field, doc)
	{
		var response = {};

		// reference
		var referenceElem = doc.querySelector("link[rel='canonical']");
		response.reference = referenceElem.getAttribute("href");
		/*
		var referenceValue = url;
		if( referenceValue.indexOf("https") === 0 )
			referenceValue = "http" + referenceValue.substring(5);
		referenceValue = referenceValue.substring(0, referenceValue.indexOf("?"));
		response.reference = referenceValue;
		*/				

		// description
		var descriptionElem = doc.querySelector("meta[name='Description']");
		response.description = descriptionElem.getAttribute("content");

		// title
		var titleElem = doc.querySelector("span[itemprop='name']");
		response.title = titleElem.innerHTML;

		var appId = response.reference.substring(0, response.reference.length-1);
		appId = appId.substring(appId.lastIndexOf("/") + 1);

		// screen
		var screenElem = doc.querySelector(".highlight_strip_screenshot");
		var screenId = screenElem.getAttribute("id");
		var screenId = screenId.substring(17);//thumb_screenshot_
		response.screen = "http://cdn.akamai.steamstatic.com/steam/apps/" + appId + "/" + screenId;

		// marquee
		response.marquee = "http://cdn.akamai.steamstatic.com/steam/apps/" + appId + "/header.jpg";

		// preview
		var previewElem = doc.querySelector(".highlight_strip_movie");
		if( !!previewElem )
			response.preview = "http://store.steampowered.com/video/" + appId;

		// file
		response.file = appId;

		return response;
	},
	"test": function(url, doc, callback)
	{
		var validForScrape = false;
		var redirect = false;

		// title
		//var titleElem = doc.querySelector("span[itemprop='name']");
		//if( titleElem )
		//	validForScrape = true;
		//response.title = titleElem.innerHTML;

		var screenElem = doc.querySelector(".highlight_strip_screenshot");
		if( !!screenElem )
			validForScrape = true;

		callback({"validForScrape": validForScrape, "redirect": redirect});
	},
	"testDelay": 0,
	"runDelay": 0
});
arcadeHud.addScraper({
	"id": "steamstore",
	"api_version": 0.1,
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
		"file": 50,
		"type": 100
	},
	"run": function(url, field, doc)
	{
		var response = {};

		// reference
		var referenceElem = doc.querySelector("link[rel='canonical']");
		var referenceURL = referenceElem.getAttribute("href");

		var regex = new RegExp("(http|https):\/\/(?:www\.)?store\.steampowered\.com\/app\/[0-9]+\/");
		var results = regex.exec(referenceURL);
		referenceURL = results[0];

		response.reference = referenceURL;

		//http://store.steampowered.com/app/480490/Prey/?snr=1_4_4__100

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

		var appId = referenceURL.substring(0, referenceURL.length-1);
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

		// type
		response.type = "PC";

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
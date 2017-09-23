arcadeHud.addScraper({
	"id": "oculusstore",
	"api_version": 0.1,
	"title": "Oculus Store",
	"summary": "VR Games",
	"description": "The Oculus Store is home to Oculus Rift VR games.",
	"homepage": "https://www.oculus.com/experiences/rift/",
	"search": "http://www.google.com/search?q=$TERM+" + encodeURIComponent("site:https://www.oculus.com/experiences/rift"),
	"can_acquire": true,
	"allow_keywords": false,
	"fields":
	{
		"all": 100,
		"reference": 100,
		"description": 100,
		"title": 100,
		"screen": 100,
		"marquee": 100,
		"preview": 100,
		"file": 1,
		"type": 99
	},
	"run": function(url, field, doc)
	{
		var response = {};

		// reference
		var goodUrl = doc.querySelector("meta[property='og:url']").getAttribute("content");
		if( goodUrl.indexOf("https://") === 0 )
			goodUrl = "http://" + goodUrl.substring(8);
		response.reference = goodUrl;

		// description
		var descriptionElem = doc.querySelector("meta[property='og:description']");
		response.description = descriptionElem.getAttribute("content");

		// title
		var titleElem = doc.querySelector("meta[property='og:title']");
		var titleVal = titleElem.getAttribute("content");
		response.title = titleVal;

		// file
		response.file = titleVal;

		// screen
		var screenElem = doc.querySelector("meta[property='og:image:url']");
		var goodScreenUrl = screenElem.getAttribute("content");
		var lastIndex = goodScreenUrl.lastIndexOf("?oh=");
		if( lastIndex >= 0 )
			goodScreenUrl = goodScreenUrl.substring(0, lastIndex);

		if( goodScreenUrl.indexOf("https://") === 0 )
			goodScreenUrl = "http://" + goodScreenUrl.substring(8);

		response.screen = goodScreenUrl;

		// marquee
		response.marquee = goodScreenUrl;

		// preview
		var previewElem = doc.querySelector("video");
		var previewUrl = "";
		if( previewElem )
		{
			previewUrl = previewElem.src;
			lastIndex = previewUrl.lastIndexOf("?oh=");
			if( lastIndex >= 0 )
				previewUrl = previewUrl.substring(0, lastIndex);

			if( previewUrl.indexOf("https://") === 0 )
				previewUrl = "http://" + previewUrl.substring(8);
		}

		if( previewUrl !== "" )
			response.preview = previewUrl;

		// type
		response.type = "pc";
		
		return response;
	},
	"test": function(url, doc, callback)
	{
		var validForScrape = false;
		var redirect = false;

		if( url.indexOf("oculus.com/experiences/") >= 0 )
			validForScrape = true;

		callback({"validForScrape": validForScrape, "redirect": redirect});
	},
	"testDelay": 1000,
	"runDelay": 0
});
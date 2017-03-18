arcadeHud.addScraper({
	"id": "origin",
	"title": "Origin Store",
	"homepage": "http://www.origin.com/",
	"search": "http://www.origin.com/search?searchString=$TERM",
	"fields":
	{
		"all": 100,
		"reference": 100,
		"description": 100,
		"title": 100,
		"screen": 100,
		"marquee": 100,
		"preview": 100,
		"type": 99
	},
	"run": function(url, field, doc)
	{
		// helper function for extracting YT ID's from YT URLs
		function extractYouTubeId(trailerURL)
		{
			if( typeof trailerURL === "undefined" )
				return trailerURL;

			var youtubeid;
			if( trailerURL.indexOf("youtube") != -1 && trailerURL.indexOf("v=") != -1 )
			{
				youtubeid = trailerURL.substr(trailerURL.indexOf("v=")+2);

				var found = youtubeid.indexOf("&");
				if( found > -1 )
				{
					youtubeid = youtubeid.substr(0, found);
				}
			}
			else
			{
				var bases = ["youtu.be/", "/embed/"]
				var found = trailerURL.indexOf("youtu.be/");
				if( found >= 0 )
					youtubeid = trailerURL.substr(found+9);
				else
				{
					found = trailerURL.indexOf("/embed/");
					if( found >= 0 )
						youtubeid = trailerURL.substr(found+7);
				}

				if( !!youtubeid )
				{
					found = youtubeid.indexOf("?");
					if( found > 0 )
						youtubeid = youtubeid.substr(0, found);
					else
					{
						found = youtubeid.indexOf("&");
						if( found > 0 )
							youtubeid = youtubeid.substr(0, found);
					}
				}
			}

		  return youtubeid;
		}

		var response = {};

		// reference
		response.reference = url;

		// description
		var descriptionElem = doc.querySelector("meta[property='og:description']");
		response.description = descriptionElem.getAttribute("content");

		// title
		var titleElem = doc.querySelector("meta[property='og:title']");
		response.title = titleElem.getAttribute("content");

		// screen
		var screenElem = doc.querySelector(".origin-store-blurimage-image");
		response.screen = screenElem.getAttribute("ng-src");
		
		/*
		var screenElem = doc.querySelector("img .origin-carouselimageitem-thumb");
		response.screen = screenElem.getAttribute("ng-src");
		*/

		// marquee
		var marqueeElem = doc.querySelector("meta[property='og:image']");
		response.marquee = marqueeElem.getAttribute("content");

		// preview
		var previewElem = doc.querySelector(".origin-store-carousel-media-videoitem-videothumb");
		//var extractedId = extractYouTubeId(previewElem.getAttribute("ng-src"));
		var extractedId = previewElem.getAttribute("ng-src");
		extractedId = extractedId.substring(0, extractedId.length - 6);
		extractedId = extractedId.substring(extractedId.lastIndexOf("/") + 1);
		if( extractedId != "")
			response.preview = "http://www.youtube.com/watch?v=" + extractedId;

		// type
		response.type = "pc";
		
		return response;
	},
	"test": function(url, doc, callback)
	{
		var validForScrape = false;
		var redirect = false;
		
		var previewElem = doc.querySelector(".origin-store-carousel-media-videoitem-videothumb");
		console.log(previewElem);
		if( !!previewElem )
			validForScrape = true;

		callback({"validForScrape": validForScrape, "redirect": redirect});
	},
	"testDelay": 4000,
	"runDelay": 0
});
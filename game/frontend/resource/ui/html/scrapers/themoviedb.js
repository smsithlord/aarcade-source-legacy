arcadeHud.addScraper({
	"id": "themoviedb",
	"title": "TheMovieDb",
	"homepage": "http://www.themoviedb.org/",
	"search": "http://www.themoviedb.org/search?query=$TERM",
	"fields":
	{
		"all": 100,
		"marquee": 100,
		"screen": 100,
		"preview": 100,
		"description": 100,
		"reference": 100,
		"title": 100,
		"type": 80
	},
	"test": function(url, doc, callback)
	{
		var validForScrape = false;
		var redirect = "";
		//<meta property="og:type" content="movie"/>
		var elem = doc.querySelector("meta[property='og:type']");
		if( elem && elem.getAttribute("content") === "movie" )
			validForScrape = true;

		if( !validForScrape && url.indexOf("themoviedb.org/search?query=") >= 0 )
		{
			// check how many search results there are
			var items = doc.querySelectorAll(".item");
			console.log("Num results: " + items.length);

			if( items.length === 1 )
			{
				var anchor = items[0].querySelector("a");
				if(!!anchor)
					redirect = "http://www.themoviedb.org" + anchor.getAttribute("href");
			}
		}

		callback({"validForScrape": validForScrape, "redirect": redirect});
	},
	"run": function(uri, field, doc)
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

		// marqueeBase: http://image.tmdb.org/t/p/original/
		var response = {};

		// reference
		var goodUri = uri.substring(0, uri.indexOf("-")).replace("https", "http");
		response.reference = goodUri;
		
		// title
		var goodTitle = doc.querySelector("meta[property='og:title']").getAttribute("content");

		// type
		var goodType = doc.querySelector("meta[property='og:type']").getAttribute("content");
		if( goodType === "movie" )
		{
			// fix the type to be an AArcade type
			goodType += "s";

			// need to find a date to append if this movie's title
			var releaseDate = doc.querySelector(".release_date").innerHTML;
			if( releaseDate )
				goodTitle += " " + releaseDate;
		}

		response.type = goodType;
		response.title = goodTitle;

		// preview
		var yt = doc.querySelector(".original_content iframe");
		if(!!yt)
		{
			yt = yt.getAttribute("data-src");
			yt = extractYouTubeId(yt);
		}

		if(!!!yt)
			yt = "";
		else
			yt = "http://www.youtube.com/watch?v=" + yt;

		response.preview = yt;

		// description
		response.description = doc.querySelector("meta[name='description']").getAttribute("content");

		// screen
		var screen = doc.querySelector(".original_content div .backdrop");

		if(!!screen)
		{
			screen = screen.getAttribute("data-src");
			screen = screen.substring(screen.lastIndexOf("/") + 1);
		}

		if(!!!screen)
			screen = "";
		else
			screen = "http://image.tmdb.org/t/p/original/" + screen;

		response.screen = screen;

		// marquee
		var marquee = doc.querySelector(".original_content div .poster");
		
		if(!!marquee)
		{
			marquee = marquee.getAttribute("data-src");
			marquee = marquee.substring(marquee.lastIndexOf("/") + 1);
		}

		if(!!!marquee)
			marquee = "";
		else
			marquee = "http://image.tmdb.org/t/p/original/" + marquee;

		response.marquee = marquee;
		return response;
	}
});
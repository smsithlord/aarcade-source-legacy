arcadeHud.addScraper({
	"id": "netflix",
	"api_version": 0.1,
	"title": "Netflix",
	"summary": "Movies & TV",
	"description": "Netflix is a subscription site with lots of movies, TV shows, & documentaries.<br /><br />This scraper is *not* useful for finding movie posters or trailers.  Note that you'll have to manually give your new shortcut a title after you spawn it.",
	"homepage": "http://www.netflix.com/",
	"search": "http://www.netflix.com/search/$TERM",
	"can_acquire": true,
	"allow_keywords": false,
	"fields":
	{
		"all": 100,
		"description": 100,
		"file": 100,
		"stream": 100,
		"title": 1,
		"type": 80
	},
	"test": function(url, doc, callback)
	{
		var validForScrape = false;
		var redirect = "";

		/*
		var elem = doc.querySelector(".content");
		if(!!elem)
			validForScrape = true;
		console.log(elem);
		*/

		if( url.indexOf("netflix.com/watch/") >= 0 )
			validForScrape = true;

		callback({"validForScrape": validForScrape, "redirect": redirect});
	},
	"run": function(uri, field, doc)
	{
		// netflix is a special type of scraper that just looks at the URL instead of looking at the DOM.
		// marqueeBase: http://image.tmdb.org/t/p/original/
		var response = {};

		// stream
		response.stream = uri.substring(0, uri.indexOf("?")).replace("https", "http");
		response.file = response.stream;

		var found = response.stream.lastIndexOf("/");
		if( found > 0 )
		{
			var movieId = response.stream.substring(found+1);
			response.title = "Untitled Netflix Movie " + movieId;
		}
		else
			response.title = "Untitled Netflix Movie";

		// title
		//var elem = doc.querySelector(".content");
		//response.title = elem.querySelector("h2").innerHTML;

		// type
		response.type = "movies";

		// description
		//var ps = elem.querySelectorAll("p");
		//ps = ps[ps.length-1];
		//response.description = ps.innerHTML;

		return response;
	},
	"testDelay": 4000,	// added in December to try and fix the search results page glitch when scraping.
	"runDelay": 0
});
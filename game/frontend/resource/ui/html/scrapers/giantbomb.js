arcadeHud.addScraper({
	"id": "giantbomb",
	"api_version": 0.1,
	"title": "Giant Bomb",
	"summary": "Retro & Modern Games",
	"description": "Giant Bomb is a massive database of games from every era. Great for finding boxart, posters, screenshots, etc.",
	"homepage": "http://www.giantbomb.com/",
	"search": "http://www.giantbomb.com/search/?q=$TERM",
	"can_acquire": false,
	"allow_keywords": false,
	"fields":
	{
		"all": 100,
		"reference": 100,
		"description": 100,
		"title": 100,
		"screen": 100,
		"marquee": 100
	},
	"test": function(url, doc, callback)
	{
		var validForScrape = false;
		var redirect = "";

		var elem = doc.querySelector(".wiki-details table tbody tr td div span a");
		if(!!elem)
			validForScrape = true;

		callback({"validForScrape": validForScrape, "redirect": redirect});
	},
	"run": function(uri, field, doc)
	{
		var response = {};

		// description
		var descriptionElem = doc.querySelector("meta[name='description']");
		response.description = descriptionElem.innerHTML;

		var detailsTable = doc.querySelector(".wiki-details table tbody");
		var rows = detailsTable.querySelectorAll("tr");

		var i;
		var foundRow;
		var max = rows.length;
		
		// title & reference
		foundRow = null;
		for( i = 0; i < max; i++ )
		{
			if( rows[i].querySelector("th").innerHTML === "Name" )
			{
				foundRow = rows[i];
				break;
			}
		}

		if( foundRow )
		{
			var titleElem = foundRow.querySelector("td a");
			response.title = titleElem.innerHTML;
			response.reference = "http://www.giantbomb.com" + titleElem.getAttribute("href");
		}

		// CAN'T RESOLVE TYPE FROM HERE BECAUSE IT LISTS ALL TYPES

		// marquee
		var marqueeElem = doc.querySelector(".wiki-boxart img");
		if( !!marqueeElem )
			response.marquee = marqueeElem.getAttribute("src").replace("scale_small", "original");

		// screen
		var screenElem = doc.querySelector("div[class='display-view'] figure a");
		if( !!screenElem )
			response.screen = screenElem.getAttribute("href");
		else
		{
			var screenElems = doc.querySelectorAll("div[class='gallery-box-pod'] figure a");
			console.log(screenElems.length);
			var numScreenElems = screenElems.length;
			for( i = 1; i < numScreenElems; i++ )
			{
				if( Math.random() > 0.5 )
				{
					screenElem = screenElems[i];
					break;
				}
			}

			if( !!!screenElem && numScreenElems > 1 )
				screenElem = screenElems[2];

			if( !!screenElem )
				response.screen = screenElem.getAttribute("href");

			console.log(response.screen);
		}
		
		return response;
	}
});
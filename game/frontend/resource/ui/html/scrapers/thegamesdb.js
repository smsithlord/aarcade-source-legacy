arcadeHud.addScraper({
	"id": "thegamesdb",
	"title": "TheGamesDB",
	"homepage": "http://www.thegamesdb.net/",
	"search": "http://thegamesdb.net/search/?string=$TERM",
	"fields":
	{
		"all": 100,
		"reference": 100,
		"description": 100,
		"title": 100,
		"screen": 100,
		"marquee": 100,
		"type": 80
	},
	"test": function(url, doc, callback)
	{
		var validForScrape = false;
		var redirect = "";

		var elem = doc.querySelector("#gameTitle");
		if(!!elem)
			validForScrape = true;

		callback({"validForScrape": validForScrape, "redirect": redirect});
	},
	"run": function(uri, field, doc)
	{
		var response = {};

		// reference
		var referenceElem = doc.querySelector("link[rel='canonical']");
		response.reference = referenceElem.getAttribute("href");

		// title
		var titleElem = doc.querySelector("#gameTitle > h1");
		response.title = titleElem.innerHTML;

		// marquee
		var marqueeElem = doc.querySelector("#gameCovers > img");
		if( !!marqueeElem )
			response.marquee = marqueeElem.getAttribute("src");

		if( !!response.marquee && response.marquee.indexOf("placeholders") >= 0 )
			delete response["marquee"];

		response.marquee = response.marquee.replace("_gameviewcache/", "");

		if( !!response.marquee && response.marquee.indexOf("http") !== 0 )
			response.marquee = "http://thegamesdb.net" + response.marquee;

		// screen
		var screenElem = doc.querySelector("#screens img");
		if( !!!screenElem )
			screenElem = doc.querySelector("#fanartScreens img")
		if( !!screenElem )
			response.screen = screenElem.getAttribute("src");

		if( !!response.screen && response.screen.indexOf("placeholders") >= 0 )
			delete response["screen"];

		if( !!response.screen && response.screen.indexOf("http") !== 0 )
			response.screen = "http://thegamesdb.net" + response.screen;

		// description
		var descriptionElem = doc.querySelector("#gameInfo > p");
		if( !!descriptionElem )
			response.description = descriptionElem.innerText;

		// type
		var typeElem = doc.querySelector("#gameinfo > h2 > a");
		if( typeElem )
		{
			var goodType = typeElem.innerHTML;

			if( goodType === "Sega Genesis" )
				goodType = "genesis";
			else if( goodType === "Nintendo GameCube")
				goodType = "gamecube";
			else if( goodType === "Nintendo 64" )
				goodType = "n64";
			else if( goodType === "Super Nintendo (SNES)" )
				goodType = "snes";
			else if( goodType === "Nintendo Entertainment System (NES)" )
				goodType = "nes";
			else if( goodType === "Nintendo 3DS" )
				goodType = "3ds";
			else if( goodType === "Nintendo DS" )
				goodType = "ds";
			else if( goodType === "Nintendo Wii U" )
				goodType = "wiiu";
			else if( goodType === "Sonly Playstation" )
				goodType = "ps";
			else if( goodType === "Arcade" )
				goodType = "arcade";
			else if( goodType === "Atari 2600" )
				goodType = "atari2600";
			else if( goodType === "Neo Geo" )
				goodType = "arcade";
			else if( goodType === "Nintendo Game Boy" )
				goodType = "gameboy";
			else if( goodType === "Nintendo Game Boy Advance" )
				goodType = "gba";
			else if( goodType === "Nintendo Game Boy Color" )
				goodType = "gbc";
			else if( goodType === "Nintendo Virtual Boy" )
				goodType = "virtualboy";
			else if( goodType === "Nintendo Wii" )
				goodType = "wii";
			else if( goodType === "PC" )
				goodType = "pc";
			else if( goodType === "Sega 32X" )
				goodType = "32x";
			else if( goodType === "Sega Master System" )
				goodType = "mastersystem";
			else if( goodType === "Sega Saturn" )
				goodType = "saturn";
			else if( goodType === "Sony Playstation 2" )
				goodType = "ps2";
			else if( goodType === "Sony Playstation 3" )
				goodType = "ps3";
			else if( goodType === "Sony Playstation 4" )
				goodType = "ps4";
			else if( goodType === "Sony Playstation Vita" )
				goodType = "vita";
			else if( goodType === "Sony PSP" )
				goodType = "psp";

			response.type = goodType;
		}

		return response;
	}
});
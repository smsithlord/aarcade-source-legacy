arcadeHud.addScraper({
	"id": "amazon",
	"api_version": 0.1,
	"title": "Amazon",
	"summary": "Movies, Games, etc.",
	"description": "They sell movies, games, and everything else.",
	"homepage": "http://www.amazon.com/",
	"search": "http://www.amazon.com/s?url=search-alias%3Daps&field-keywords=$TERM",
	"can_acquire": true,
	"allow_keywords": false,
	"fields":
	{
	},
	"run": function(url, field, doc)
	{
		return {};
	},
	"test": function(url, doc, callback)
	{
		callback({"validForScrape": false, "redirect": false});
	},
	"testDelay": 4000,
	"runDelay": 0
});
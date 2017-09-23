arcadeHud.addScraper({
	"id": "google",
	"api_version": 0.1,
	"title": "Google",
	"summary": "Everything",
	"description": "Google will let you find anything you need.",
	"homepage": "http://www.google.com/",
	"search": "http://www.google.com/search?q=$TERM",
	"can_acquire": true,
	"allow_keywords": true,
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
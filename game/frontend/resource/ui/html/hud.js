function ArcadeHud()
{
	//this.selectedItem;
	this.pageTitle = "Untitled";
	this.platformId = "-KJvcne3IKMZQTaG7lPo";
	this.url = "";
	this.selectedWebTab = null;
	this.libretroHudButtonElem;
	this.pinHudButtonElem;
	this.returnHudButtonElem;
	//this.closeContentButtonElem;
	//this.hudHeaderContainerElem;
	this.clickThruElem;
	this.cursorElem;
	this.cursorPreviewImageElem;
	this.cursorImageElem;
	this.helpElem;
	//this.metaScrapeElem;
	//this.hudMetaScrapeContainer;
	this.startupLoadingMessagesContainer;
	this.hudLoadingMessagesContainer;
	this.hudLoadingMessages = {};
	this.DOMReady = false;
	this.addressElem;
	this.DOMParser;
	this.lastMousePassThru = false;
	this.fileBrowseHandles = {};
	this.metaScrapeHandles = {};
	this.activeScraper = null;
	this.activeScraperItemId = "";
	this.activeScraperField = "";
	this.scrapers = {
		"importSteamGames":
		{
			"id": "importSteamGames",
			"title": "Import Steam Games",
			"search": "http://www.google.com/search?q=$TERM",	// should never be used. this is an abnormal scraper only called when importing games from a user's Steam profile.
			"fields": {
				"all": 100
			},
			"run": function(url, field, doc)
			{
				var rawGames = doc;
				var numRawGames = rawGames.length;
				var steamItems = [];
				var i, steamItem;
				for( i = 0; i < numRawGames; i++ )
				{
					steamItems.push(rawGames[i].name_escaped);
					steamItems.push(rawGames[i].appid + "");
				}

				console.log("There are " + (steamItems.length / 2) + " Steam items to return to AArcade.");
				return steamItems;
			},
			"test": function(url, doc, callback)
			{
				var validForScrape = false;
				var redirect = false;
				
				if( Array.isArray(doc) )
					validForScrape = true;

				/*
				var rawGames = doc;
				var numRawGames = rawGames.length;
				var steamItems = [];
				var i, steamItem;
				for( i = 0; i < numRawGames; i++ )
				{
					steamItem = {
						"name": rawGames[i].name_escaped,
						"appid": rawGames[i].appid
					};

					steamItems.push(steamitem);
				}

				aaapi.system.importSteamGames(steamItems);
				*/
				callback({"validForScrape": validForScrape, "redirect": redirect});
			},
			"testDelay": 2000,
			"runDelay": 0
		}/*,
		"currenturi": 	// SHOULD USE GOOGLE AS ITS "HOMEPAGE"
		{
			"id": "currenturi",
			"title": "Use Current Address",
			"fields":
			{
				"marquee": 100,
				"screen": 100,
				"preview": 100,
				"file": 100,
				"reference": 100,
				"download": 100,
				"stream": 100
			},
			"run": function(url, doc)
			{
				// marqueeBase: http://image.tmdb.org/t/p/original/
				var response = {};
				response.marquee = url;
				response.screen = url;
				response.preview = url;
				response.file = url;
				response.reference = url;
				response.download = url;
				response.stream = url;
				return response;
			}
		}*/
	};

	this.onDOMReady().then(function()
	{
		console.log("body ready");
		this.DOMParser = new DOMParser();

		/*
		 * DOMParser HTML extension
		 * 2012-09-04
		 * 
		 * By Eli Grey, http://eligrey.com
		 * Public domain.
		 * NO WARRANTY EXPRESSED OR IMPLIED. USE AT YOUR OWN RISK.
		 */

		/*! @source https://gist.github.com/1129031 */
		/*global document, DOMParser*/
		this.DOMParser.parseFromString = function(markup, type) {
			"use strict";
				if (/^\s*text\/html\s*(?:;|$)/i.test(type)) {
					var
					  doc = document.implementation.createHTMLDocument("")
					;
			      		if (markup.toLowerCase().indexOf('<!doctype') > -1) {
		        			doc.documentElement.innerHTML = markup;
		      			}
		      			else {
		        			doc.body.innerHTML = markup;
		      			}
					return doc;
				} else {
					return nativeParse.apply(this, arguments);
				}
			};

		this.DOMReady = true;

		// inject the browser menu
		//var bodyElem = document.body;

		// top-level container for the browser menu
		//var browserMenuElem = document.createElement("div");
		//browserMenuElem.className = "hudHeaderContainer";

		var browserMenuElem = document.querySelector(".hudHeaderContainer");
		if( browserMenuElem )
		{
			// navigation container
			var navigationElem = document.createElement("div");
			navigationElem.className = "hudHeaderNavigationContainer";
			navigationElem.style.cssText = "text-align: center; font-size: 20px; font-family: Arial;";

			// address form
			var addressFormElem = document.createElement("form");
			addressFormElem.style.cssText = "display: inline-block;";

			addressFormElem.addEventListener("submit", function(e)
			{
				e.preventDefault();
				arcadeHud.navigateToURI(arcadeHud.addressElem.value);
				//return false;
			}, true);

			// address label
			var addressLabelElem = document.createElement("div");//createTextNode("Internet Address:");
			addressLabelElem.style.cssText = "display: inline-block; border: 2px solid rgba(50, 0, 0, 0.7); background-color: rgba(100, 0, 0, 0.7); border-radius: 2px; padding: 1px; padding-left: 5px; padding-right: 5px; text-shadow: 2px 2px 2px #000; font-weight: bold;";
			addressLabelElem.innerText = "Internet Address";
			addressFormElem.appendChild(addressLabelElem);
			//navigationElem.appendChild(addressLabelElem);

			// address
			this.addressElem = document.createElement("input");
			this.addressElem.style.cssText= "width: 600px; font-size: 20px; font-family: Arial;";
			this.addressElem.addEventListener("focus", function()
			{
				arcadeHud.addressElem.select();
				//arcadeHud.addressElem.setSelectionRange(0, arcadeHud.addressElem.value.length);
			}, true);
			
			addressFormElem.appendChild(this.addressElem);
			navigationElem.appendChild(addressFormElem);
			//navigationElem.appendChild(this.addressElem);
			//navigationElem.innerHTML = "helloo world";

			// back label
			var backLabelElem = document.createElement("div");//createTextNode("Internet Address:");
			backLabelElem.style.cssText = "display: inline-block; border: 2px solid rgba(50, 0, 0, 0.7); background-color: rgba(100, 0, 0, 0.7); border-radius: 2px; padding: 1px; padding-left: 5px; padding-right: 5px; text-shadow: 2px 2px 2px #000; font-weight: bold;";
			backLabelElem.innerHTML = "&#x25c0;";
			navigationElem.appendChild(backLabelElem);

			// home label
			var homeLabelElem = document.createElement("div");//createTextNode("Internet Address:");
			homeLabelElem.style.cssText = "display: inline-block; border: 2px solid rgba(50, 0, 0, 0.7); background-color: rgba(100, 0, 0, 0.7); border-radius: 2px; padding: 1px; padding-left: 5px; padding-right: 5px; text-shadow: 2px 2px 2px #000; font-weight: bold;";
			homeLabelElem.innerHTML = "&#10070;";
			navigationElem.appendChild(homeLabelElem);

			// fav label
			var favLabelElem = document.createElement("div");//createTextNode("Internet Address:");
			favLabelElem.style.cssText = "display: inline-block; border: 2px solid rgba(50, 0, 0, 0.7); background-color: rgba(100, 0, 0, 0.7); border-radius: 2px; padding: 1px; padding-left: 5px; padding-right: 5px; text-shadow: 2px 2px 2px #000; font-weight: bold;";
			favLabelElem.innerHTML = "&#9733;";
			navigationElem.appendChild(favLabelElem);

			// browser tabs container table
			var topTabsTableElem = document.createElement("div");
			topTabsTableElem.className = "hudHeaderContainerTable";

			// browser tabs container row
			var topTabsRowElem = document.createElement("tr");
			topTabsRowElem.className = "hudHeaderContainerRow";

			// blank browser tab container cell
			var blankBrowserTabsCell = document.createElement("div");
			blankBrowserTabsCell.className = "hudHeaderContainerCell";

			// aarcade browser tab container cell
			var aarcadeBrowserTabsCell = document.createElement("div");
			aarcadeBrowserTabsCell.className = "hudHeaderContainerCell";

			// aarcade browser tab
			var aarcadeBrowserTabElem = document.createElement("div");
			aarcadeBrowserTabElem.className = "hudHeaderButtonContainer";

			// aarcade tab label
			var aarcadeTabLabelElem = document.createElement("div");
			aarcadeTabLabelElem.className = "hudHeaderButton hudHeaderButtonOn helpNote";
			aarcadeTabLabelElem.setAttribute("help", "Expand the address bar.");
			aarcadeTabLabelElem.innerHTML = "AArcade";
			aarcadeTabLabelElem.addEventListener("click", function()
			{
				this.expandAddressMenu();
			}.bind(this), true);

			// other browser tab container cell
			var otherBrowserTabsCell = document.createElement("div");
			otherBrowserTabsCell.className = "hudHeaderContainerCell";

			// pin browser tab container cell
			//var pinBrowserTabsCell = document.createElement("div");
			//pinBrowserTabsCell.className = "hudHeaderContainerCell";

			// unpin browser tab
			var unpinBrowserTabElem = document.createElement("div");
			unpinBrowserTabElem.className = "hudHeaderButtonContainer";
			unpinBrowserTabElem.style.right = "20%";
			unpinBrowserTabElem.id = "returnHudButton";

			// unpin tab label
			var unpinTabLabelElem = document.createElement("div");
			unpinTabLabelElem.className = "hudHeaderButton hudHeaderButtonOn helpNote";
			unpinTabLabelElem.setAttribute("help", "Unpin the HUD.");
			unpinTabLabelElem.innerHTML = "&nbsp;<img src=\"asset://ui/pinicon.png\" />&nbsp;";
			unpinTabLabelElem.addEventListener("click", function()
			{
				aaapi.system.deactivateInputMode();
			}.bind(this), true);

			// libretro browser tab
			var libretroBrowserTabElem = document.createElement("div");
			libretroBrowserTabElem.className = "hudHeaderButtonContainer";
			//libretroBrowserTabElem.style.right = "20%";
			libretroBrowserTabElem.id = "libretroHudButton";

			// libretro tab label
			//console.log("shiiit");
			var libretroTabLabelElem = document.createElement("div");
			libretroTabLabelElem.className = "hudHeaderButton hudHeaderButtonOn helpNote";
			libretroTabLabelElem.setAttribute("help", "Libretro Menu");
			libretroTabLabelElem.innerHTML = "&nbsp;<img src=\"asset://ui/runicon.png\" />&nbsp;";
			libretroTabLabelElem.addEventListener("click", function()
			{
				window.location = "asset://ui/libretro.html";
			}.bind(this), true);

			// pin browser tab
			var pinBrowserTabElem = document.createElement("div");
			pinBrowserTabElem.className = "hudHeaderButtonContainer";
			pinBrowserTabElem.style.right = "20%";
			pinBrowserTabElem.id = "pinHudButton";

			// pin tab label
			var pinTabLabelElem = document.createElement("div");
			pinTabLabelElem.className = "hudHeaderButton helpNote";
			pinTabLabelElem.setAttribute("help", "Pin the HUD to stay open.");
			pinTabLabelElem.innerHTML = "&nbsp;<img src=\"asset://ui/pinicon.png\" />&nbsp;";
			pinTabLabelElem.addEventListener("click", function()
			{
				aaapi.system.forceInputMode();
				this.pinHudButtonElem.style.display = 'none';
				this.returnHudButtonElem.style.display = 'inline-block';
			}.bind(this), true);

			// compose
			aarcadeBrowserTabsCell.appendChild(aarcadeBrowserTabElem);
			aarcadeBrowserTabElem.appendChild(aarcadeTabLabelElem);

			unpinBrowserTabElem.appendChild(unpinTabLabelElem);
			pinBrowserTabElem.appendChild(pinTabLabelElem);
			libretroBrowserTabElem.appendChild(libretroTabLabelElem);
			otherBrowserTabsCell.appendChild(unpinBrowserTabElem);
			otherBrowserTabsCell.appendChild(pinBrowserTabElem);
			otherBrowserTabsCell.appendChild(libretroBrowserTabElem);

			topTabsRowElem.appendChild(blankBrowserTabsCell);
			topTabsRowElem.appendChild(aarcadeBrowserTabsCell);
			topTabsRowElem.appendChild(otherBrowserTabsCell);

			topTabsTableElem.appendChild(topTabsRowElem);

			browserMenuElem.appendChild(navigationElem);
			browserMenuElem.appendChild(topTabsTableElem);
		}

		////bodyElem.appendChild(browserMenuElem);
		//bodyElem.insertBefore(browserMenuElem, bodyElem.firstChild);

		//var startupLoadingContainer = 
		this.startupLoadingMessagesContainer = document.body.querySelector("#startupLoadingMessagesContainer");	// usually undefined

		this.pinHudButtonElem = document.body.querySelector("#pinHudButton");
		this.libretroHudButtonElem = document.body.querySelector("#libretroHudButton");
		this.returnHudButtonElem = document.body.querySelector("#returnHudButton");
		this.addressTabElem = document.body.querySelector("#addressTab");
		this.hudHeaderContainerElem = document.body.querySelector(".hudHeaderContainer");
		//this.hudMetaScrapeContainerElem = document.body.querySelector("#hudMetaScrapeContainer");
//		this.closeContentButtonElem = document.body.querySelector(".hudContentHeaderCell:nth-of-type(3) .hudContentHeaderButton");
		
		//aaapi.system.requestActivateInputMode();

		this.helpElem = document.createElement("div");
		this.helpElem.className = "helpContainer";

		this.hudLoadingMessagesContainer = document.createElement("div");
		this.hudLoadingMessagesContainer.className = "hudLoadingMessagesContainer";
		this.helpElem.appendChild(this.hudLoadingMessagesContainer);
		document.body.appendChild(this.helpElem);

		aaapi.system.requestActivateInputMode();	// gets called needlessly when an object is de-selected, but fuck it.

		/*
		this.metaScrapeElem = document.createElement("div");
		this.metaScrapeElem.className = "metaScrapeContainer";
		this.metaScrapeElem.addEventListener("click", function()
		{
			arcadeHud.metaScrape(this.scraperId, this.field, function(scrapedData)
			{
				console.log("Scraped data is: ");
				console.log(scrapedData);
				var usedFields = [];
				var args = [];
				var x, field;
				for( x in scrapedData)
				{
					field = scrapedData[x];
					//if( field === "" || (this.field !== "all" && this.field !== x))
					if( this.field !== "all" && this.field !== x)
						continue;

					if( x === "type" )
					{
						var allTypes = aaapi.library.getAllLibraryTypes();
						var y;
						for( y in allTypes )
						{
							console.log(allTypes[y].title);
							if( allTypes[y].title === field )
							{
								field = allTypes[y].info.id;
								break;
							}
						}
					}
					
					var inputs = document.querySelectorAll("input, select");
					var i;
					for( i = 0; i < inputs.length; i++ )
					{
						if( inputs[i].field === x )
						{
							//inputs[i].focus();
							inputs[i].value = field;
							break;
						}
					}

					args.push(x);
					args.push(field);
					usedFields.push(x);
				}

				var success = aaapi.library.updateItem(this.itemId, args);

				if( success )
				{
					console.log("Item updated!");

					var container = document.querySelector(".metaScrapeContainer");
					container.style.display = "none";

					aaapi.system.autoInspect(this.itemId);
					aaapi.system.deactivateInputMode();
				}
				else
					console.log("Item update rejected!");
			}.bind(this));
		}.bind(this.metaScrapeElem), true);
		this.hudMetaScrapeContainer = document.createElement("div");
		this.hudMetaScrapeContainer.className = "hudScrapeButton";//hudMetaScrapeContainer";
		//this.hudMetaScrapeContainer.style = "";
		this.hudMetaScrapeContainer.innerHTML = "<img src='scrapeicon.png' style='vertical-align: middle;' /><div class='buttonText'> Meta Scrape</div>";
		this.metaScrapeElem.appendChild(this.hudMetaScrapeContainer);
		document.body.appendChild(this.metaScrapeElem);
		*/

/*
		document.body.addEventListener("dblclick", function(e)
		{
			console.log("dblclicked: " + (e.target === document.body));
		}, true);
*/

		document.body.addEventListener("mousedown", function(e)
		{
			//console.log("mouse down: " + (e.target === document.body));

			this.lastMousePassThru = (e.target === document.body);
			if(this.lastMousePassThru)
			{
				var buttonCode = 0;
				if( e.button === 1 )
					buttonCode = 1;
				else if( e.button === 2 )
					buttonCode = 2;
				aaapi.system.hudMouseDown(buttonCode, (this.lastMousePassThru));
			}
		}.bind(this), true);

		document.body.addEventListener("mouseup", function(e)
		{
			//console.log("mouse up: " + (e.target === document.body) + " vs " + this.lastMousePassThru);

			if(this.lastMousePassThru)
			{
				var buttonCode = 0;
				if( e.button === 1 )
					buttonCode = 1;
				else if( e.button === 2 )
					buttonCode = 2;
				aaapi.system.hudMouseUp(buttonCode, (this.lastMousePassThru));
			}
		}.bind(this), true);

		document.body.addEventListener("selectstart", function(e)
		{
			e.preventDefault();
			return false;
		}, true);

/*
		document.body.addEventListener("click", function(e)
		{
			//console.log("clicked: " + (e.target === document.body));
			if( (e.target === document.body) )
				aaapi.system.hud
		}, true);
*/

		this.cursorElem = document.createElement("div");
		this.cursorElem.id = "cursor";

		this.cursorImageElem = document.createElement("img");
		this.cursorImageElem.className = "cursorImage";
		this.cursorImageElem.src = "cursors/hippie_default.png";

		//if( document.body.offsetWidth > 1280 )
			//this.cursorImageElem.style.width = this.cursorImageElem.offsetWidth * 2.0 + "px";

		this.cursorElem.style.left = (document.body.offsetWidth / 2) + "px";
		this.cursorElem.style.top = (document.body.offsetHeight / 2) + "px";

		this.cursorPreviewImageElem = document.createElement("img");
		this.cursorPreviewImageElem.style.cssText = "border: 3px solid #000; display: none; margin-top: 35px; max-width: 40%; max-height: 40%; vertical-align: top;";
		this.cursorPreviewImageElem.backupUri = "";
		this.cursorPreviewImageElem.addEventListener("load", function(e)
		{
			var elem = e.srcElement;
			elem.style.display = "inline-block";
		}, true);
		this.cursorPreviewImageElem.addEventListener("error", function(e)
		{
			var elem = e.srcElement;
			if( elem.backupUri !== "" && elem.backupUri !== elem.src )
				elem.src = elem.backupUri;
		}, true);

		this.cursorElem.appendChild(this.cursorImageElem);
		this.cursorElem.appendChild(this.cursorPreviewImageElem);
		document.body.appendChild(this.cursorElem);

		document.addEventListener("mousemove", function(e)
		{
			this.cursorElem.style.display = "block";
			this.cursorElem.style.left = e.clientX + "px";
			this.cursorElem.style.top = e.clientY + "px";
		}.bind(this), true);

		// prep all of the help notes
		var elems = document.querySelectorAll(".helpNote");
		var i, elem;
		var numElems = elems.length;
		for( i = 0; i < numElems; i++ )
		{
			elem = elems[i];
			this.assignHelp(elem);
		}

		// dispatch messages we may have gotten before the DOM was ready
		this.dispatchHudLoadingMessages();
	}.bind(this));
}

ArcadeHud.prototype.navigateToURI = function(uri)
{
	if( uri !== "" )
		aaapi.system.navigateToURI(uri);
};

ArcadeHud.prototype.play = function()
{
	var item = aaapi.library.getSelectedLibraryItem();	// FIXME: This is probably overkill if all we want is the ID!
	if( item )
		window.location='asset://ui/launchItem.html?id=' + encodeURIComponent(item.info.id);
};

ArcadeHud.prototype.edit = function()
{
	var item = aaapi.library.getSelectedLibraryItem();	// FIXME: This is probably overkill if all we want is the ID!
	if( item )
		window.location='asset://ui/editItem.html?id=' + encodeURIComponent(item.info.id);
};

ArcadeHud.prototype.objectEdit = function()
{
	//var item = aaapi.library.getSelectedLibraryItem();	// FIXME: This is probably overkill if all we want is the ID!
	//if( item )
	//	window.location='asset://ui/editItem.html?id=' + encodeURIComponent(item.info.id);
	window.location = 'asset://ui/editObject.html';
};

ArcadeHud.prototype.expandAddressMenu = function()
{
	var elem = document.body.querySelector(".hudHeaderContainer");

	if( elem.style.top != "0px" )
		elem.style.top = "0px";
	else
		elem.style.top = "-65px";
};

ArcadeHud.prototype.getURL = function()
{
	return this.url;
};

//ArcadeHud.prototype.onURLChanged = function(url)
ArcadeHud.prototype.onURLChanged = function(url, scraperId, itemId, field)
{
	this.url = url;

	if( !!this.addressElem && this.addressElem.value !== this.url )
	{
		//aaapi.system.requestActivateInputMode();	// FIXME: this gets called needlessly when an object gets DEselected
		
		this.addressElem.value = this.url;

		//console.log("URL changed to " + url + " and we are " + document.location.href);
		if( scraperId !== "" )
			this.onBrowserFinishedRequest(url, scraperId, itemId, field);
	}
};

ArcadeHud.prototype.onActivateInputMode = function(isFullscreen, isHudPinned, isMapLoaded, isObjectSelected, isItemSelected, isMainMenu, url, isSelectedObject, embeddedInstanceType)
{
	isFullscreen = parseInt(isFullscreen);
	isHudPinned = parseInt(isHudPinned);
	isMapLoaded = parseInt(isMapLoaded);
	isObjectSelected = parseInt(isObjectSelected);
	isItemSelected = parseInt(isItemSelected);
	isMainMenu = parseInt(isMainMenu);
	isSelectedObject = parseInt(isSelectedObject);	// it is THE selected object

	// handle all aaOnlyIfMapLoaded elems
	var elems = document.querySelectorAll(".aaOnlyIfMapLoaded");
	var num = elems.length;
	var i;
	for( i = 0; i < num; i++ )
		elems[i].style.display = (isMapLoaded) ? "block" : "none";

//this.addressTabElem
//console.log("Is object selected: " + isObjectSelected);
///*
	if( isItemSelected )
	{
		var elems = document.body.querySelectorAll(".hudSideContainerButton");
		var i;
		for( i = 0; i < elems.length; i++ )
		{
			if( elems[i].id === "metaSearchButton" )
				continue;
			
			elems[i].style.display = "inline-block";
		}
	}
	//*/
/*
	if( isMainMenu )
		this.hudHeaderContainerElem.style.display = "none";
	else
		this.hudHeaderContainerElem.style.display = "block";
	*/

	if( isMapLoaded )
	{
		if( isFullscreen )
		{
			//if( !!this.pinHudButtonElem )
			//	this.pinHudButtonElem.style.display = "none";

			//if( !!this.returnHudButtonElem )
			//	this.returnHudButtonElem.style.display = "none";
		}
		else
		{
			if( isHudPinned )
			{
				//if( !!this.pinHudButtonElem )
				//	this.pinHudButtonElem.style.display = "none";

				//if( !!this.returnHudButtonElem )
				//	this.returnHudButtonElem.style.display = "inline-block";
			}
			else
			{
				//if( !!this.returnHudButtonElem )
				//	this.returnHudButtonElem.style.display = "none";
				
				if( isMapLoaded )
				{
				//	if( !!this.pinHudButtonElem )
				//		this.pinHudButtonElem.style.display = "inline-block";
				}
				else
				{
				//	if( !!this.pinHudButtonElem )
				//		this.pinHudButtonElem.style.display = "none";
				}
			}
		}

		//if( !!this.closeContentButtonElem )
		//	this.closeContentButtonElem.style.display = "block";
	}
	else
	{
		//if( !!this.pinHudButtonElem )
		//	this.pinHudButtonElem.style.display = "none";

		//if( !!this.returnHudButtonElem )
		//	this.returnHudButtonElem.style.display = "none";

		////if( !!this.closeContentButtonElem )
		////	this.closeContentButtonElem.style.display = "none";
	}

	if( isFullscreen )
		this.cursorImageElem.style.display = "none";
	else
		this.cursorImageElem.style.display = "block";

	var elem = document.querySelector("#objectMenu");
	console.log("Is selected object: " + isSelectedObject + " and " + (isSelectedObject === 1));
	if( isSelectedObject === 1 )
		elem.style.display = "block";
	//else
	//	elem.style.display = "none";
		


/*
	var selectedWebTab = aaapi.system.getSelectedWebTab();
	if( selectedWebTab && (!this.selectedWebTab || selectedWebTab.id !== this.selectedWebTab.id) )
	{
		if( !selectedWebTab || selectedWebTab.id === "hud" )
			document.body.style.backgroundColor = "rgba(0, 0, 0, 0.9)";
		else
			document.body.style.backgroundColor = "transparent";

		if( isHudPinned )
			this.pinHudButtonElem.style.display = "none";
		else
			this.pinHudButtonElem.style.display = "table-cell";

		this.selectedWebTab = selectedWebTab;
	}
	*/

//	document.body.style.backgroundColor = "transparent";
	if( !!this.libretroHudButtonElem )
	{
		if( embeddedInstanceType === "Libretro" )
			this.libretroHudButtonElem.style.display = "inline-block";
		else
			this.libretroHudButtonElem.style.display = "none";
	}

	this.onURLChanged(url);
};

ArcadeHud.prototype.setHudTitle = function(text)
{
	/*
	this.title = text;
	this.titleNode.nodeValue = this.title;
	*/
};

ArcadeHud.prototype.addHudLoadingMessage = function(type, text, title, id, min, max, current, callbackMethod)
{
	var message = {
		"type": type,
		"text": text,
		"title": title,
		"id": id,
		"min": min,
		"max": max,
		"current": current,
		"callbackMethod": callbackMethod	// always in the system sub-object
	};

	if( !!!this.hudLoadingMessages[id] )
	{
		this.hudLoadingMessages[id] = {
			"message": message
		};
	}
	else
	{
		if( !!!this.hudLoadingMessages[id].message )
			this.hudLoadingMessages[id].message = {};

		var x;
		for( x in message )
		{
			this.hudLoadingMessages[id].message[x] = message[x];
		}
	}

	if( this.DOMReady )
		this.dispatchHudLoadingMessages();
};

ArcadeHud.prototype.dispatchHudLoadingMessages = function()
{
	var startupIds = [
		"locallibrarytypes",
		"locallibrarymodels",
		"locallibraryapps",
		"locallibraryitems",
		"mounts",
		"workfetch",
		"detectmaps",
		"workshoplibrarymodels",
		"workshoplibraryitems",
		"skiplegacyworkshops",
		"mountworkshops",
		"oldlibrarymodels",
		"oldlibraryitems"
		];

	var bStartupHandled = false;

	var isNewMsg = false;
	var empty = true;
	var x, message, messageObject, className, progressText, percent;
	for( x in this.hudLoadingMessages )
	{
		messageObject = this.hudLoadingMessages[x];
		//console.log(messageObject.message);

		if( !!messageObject.message && !!this.startupLoadingMessagesContainer && startupIds.indexOf(messageObject.message.id) >= 0 )
			bStartupHandled = true;

		if( !!!messageObject.container )
		{
			isNewMsg = true;

			messageObject.container = document.createElement("div");

			if( bStartupHandled )
				this.startupLoadingMessagesContainer.appendChild(messageObject.container);
			else
				this.hudLoadingMessagesContainer.appendChild(messageObject.container);
		}
		else
		{
			if( !!!messageObject.message )
				continue;

			// empty out the container
			var firstChild = messageObject.container.firstChild;
			while( firstChild )
			{
				messageObject.container.removeChild(firstChild);
				firstChild = messageObject.container.firstChild;
			}
		}

		empty = false;

		if( messageObject.message.type === "progress" )
		{
			var goodCurrent;
			if( messageObject.message.current[0] === "+" )
			{
				var delta = (messageObject.message.current.length === 1) ? 1 : parseInt(messageObject.message.current.substring(1));

				if( !!!messageObject.container.previousCurrent )
					messageObject.container.previousCurrent = delta;
				else
					messageObject.container.previousCurrent += delta;

				goodCurrent = messageObject.container.previousCurrent.toString();
			}
			else
				goodCurrent = messageObject.message.current;

			progressText = "";
			if( goodCurrent !== "" )
			{
				if( messageObject.message.max !== "" )
					progressText += " (" + goodCurrent + "/" + messageObject.message.max + ")";
				else
					progressText = " (" + goodCurrent + ")";
			}

			className = "hudLoadingMessageContainer progress";

			percent = 0;
			if( messageObject.message.max === "0" || messageObject.message.max === "" )
				percent = 100;
			else if( progressText !== "" )
			{
				if( messageObject.message.max === goodCurrent )
					percent = 100;

				percent = Math.floor((parseInt(goodCurrent) / parseInt(messageObject.message.max)) * 100);

				/*
				if( percent === 100 )
				{
					setTimeout(function()
					{
						this.container.parentNode.removeChild(this.container);
					}.bind(messageObject), 500);
				}
				*/
			}

			messageObject.container.style.backgroundImage = "-webkit-linear-gradient(left, rgba(100, 100, 100, 1.0), rgba(100, 100, 100, 1.0) " + percent + "%, rgba(50, 50, 50, 0.7) " + percent + "%, rgba(50, 50, 50, 0.7) 100%)";
			messageObject.container.innerText = messageObject.message.title + progressText;
		}
		else
		{
			className = "hudLoadingMessageContainer";
			messageObject.container.innerText = messageObject.message.text;
		}

		if(  messageObject.message.callbackMethod !== "" )
		{
			if( !!aaapi.callbacks[messageObject.message.callbackMethod] )
			{
				messageObject.container.callbackMethod = messageObject.message.callbackMethod;
				setTimeout(function()
				{
					aaapi.callbacks[this.callbackMethod]();
					//this.parentNode.removeChild(this);
				}.bind(messageObject.container), 1);
			}
		}

		messageObject.container.className = className;
		delete messageObject.message;
	}

	if( !empty )
	{
		if( !bStartupHandled )
		{
			if( isNewMsg )
				this.hudLoadingMessagesContainer.scrollTop = this.hudLoadingMessagesContainer.scrollHeight;

			this.helpElem.style.display = "block";
		}
		else
		{
			if( isNewMsg )
				this.startupLoadingMessagesContainer.scrollTop = this.startupLoadingMessagesContainer.scrollHeight;
		}
	}
};

ArcadeHud.prototype.showCursorPreviewImage = function(uri, backupUri)
{
	var goodUri;
	var goodBackupUri;

	if( uri !== "" )
	{
		goodUri = uri;
		goodBackupUri = backupUri;
	}
	else if( goodBackupUri !== "" )
	{
		goodUri = backupUri;
		goodBackupUri = "";
	}

	if( this.cursorPreviewImageElem.src === goodUri )
		this.cursorPreviewImageElem.style.display = "inline-block";
	else
	{
		this.cursorPreviewImageElem.style.display = "none";

		this.cursorPreviewImageElem.backupUri = goodBackupUri;
		this.cursorPreviewImageElem.src = goodUri;
	}
};

ArcadeHud.prototype.hideCursorPreviewImage = function(uri)
{
	this.cursorPreviewImageElem.style.display = "none";
};

ArcadeHud.prototype.showScraperPopupMenu = function(popupId, x, y, width, height, itemHeight, fontSize, selectedItem, rightAligned, callback)
{
	console.log("Popup is: " + popupId);
	var popup = {
		"popupId": popupId,
		"x": parseInt(x),
		"y": parseInt(y),
		"width": parseInt(width),
		"height": parseInt(height),
		"itemHeight": parseInt(itemHeight),
		"fontSize": parseFloat(fontSize),
		"selectedItem": parseInt(selectedItem),
		"rightAligned": (rightAligned !== "0"),
		"callback": callback,
		"items": new Array()
	};

	var argIndex = 10;
	var numArguments = arguments.length;
	var popupItem;
	while( argIndex <= arguments.length - 8)
	{
		popupItem = {
			"type": arguments[argIndex],
			"label": arguments[argIndex+1],
			"tooltip": arguments[argIndex+2],
			"action": arguments[argIndex+3],	// a string for generic popups.
			"right_to_left": (arguments[argIndex+4] !== "0"),
			"has_directional_override": (arguments[argIndex+5] !== "0"),
			"enabled": (arguments[argIndex+6] !== "0"),
			"checked": (arguments[argIndex+7] !== "0")
		};

		popup.items.push(popupItem);
		argIndex += 8;
	}

	var blackout = document.createElement("div");
	blackout.style.cssText = "background-color: transparent; position: fixed; top: 0; left: 0; bottom: 0; right: 0;";
	blackout.popup = popup;

	blackout.addEventListener("mousedown", function(e)
	{
		var elem = e.srcElement;
		// make sure the blackout elem is what got clicked
		if( !!!elem.popup )
			return;

		elem.parentNode.removeChild(elem);
	}, true);

	var container = document.createElement("div");
	container.className = "popupMenuContainer";
	container.style.left = popup.x - 2 + "px";
	container.style.top = popup.y + popup.height + "px";
	container.style.width = popup.width -2 + "px";
	//container.style.height = popup.height + "px";

	var popupMenuItems = document.createElement("div");
	popupMenuItems.className = "popupMenuItems";

	var i, option, optionTextNode;
	var numItems = popup.items.length;
	for( i = 0; i < numItems; i++ )
	{
		option = document.createElement("div");
		option.className = "popupMenuItem";
		option.style.fontSize = Math.round(popup.fontSize) + "px";
//		if( popup.selectedItem === i )
//			option.className += " selectedPopupMenuItem";

		option.blackout = blackout;		
		option.popupItem = popup.items[i];
		option.addEventListener("click", function(e)
		{
			var elem = e.srcElement;
			if( !!!elem.blackout )
				return;

			//aaapi.system.didSelectPopupMenuItem(elem.blackout.popup.popupId, elem.blackout.popup.items.indexOf(elem.popupItem));

			//elem.blackout.popup.callback(elem.blackout.popup.action);
			elem.blackout.popup.callback(elem.popupItem.action);
			elem.blackout.parentNode.removeChild(elem.blackout);
		}, true);

		optionTextNode = document.createTextNode(popup.items[i].label);
		option.appendChild(optionTextNode);
		popupMenuItems.appendChild(option);
	}

	var optionSearchContainer = document.createElement("div");
	optionSearchContainer.className = "popupMenuItemSearch";

	var optionSearchForm = document.createElement("form");
	optionSearchForm.addEventListener("submit", function(e)
	{
		console.log("FIRST ONE");
		//console.log(e.srcElement.firstChild.popupMenuItems.childNodes);
		var elem = e.srcElement.firstChild;

		var i;
		var count = 0;
		var firstIndex = -1;
		var numNodes = elem.popupMenuItems.childNodes.length;
		for( i = 0; i < numNodes && count < 2; i++ )
		{
			if( elem.popupMenuItems.childNodes[i].style.display !== "none" )
			{
				if( count === 0 )
					firstIndex = i;

				count++;
			}
		}

		if( count > 0 )//count === 1 )
		{
			var optionElem = elem.popupMenuItems.childNodes[firstIndex];
			//aaapi.system.didSelectPopupMenuItem(optionElem.blackout.popup.popupId, optionElem.blackout.popup.items.indexOf(optionElem.popupItem));
			optionElem.blackout.parentNode.removeChild(optionElem.blackout);
		}

		e.preventDefault();
		return false;
	}, true);

	var optionSearch = document.createElement("input");
	optionSearch.popupMenuItems = popupMenuItems;
	optionSearch.type = "text";
	optionSearch.placeholder = "Search...";
	optionSearch.changeTimeout = null;

	optionSearch.addEventListener("input", function(e)
	{
		var elem = e.srcElement;
		if( elem.changeTimeout )
			clearTimeout(elem.changeTimeout);

		elem.changeTimeout = setTimeout(function()
		{
			// step through all options and set display mode accordingly
			var searchText = this.value.toLowerCase();

			var i, popupMenuItem;
			var numPopupMenuItems = this.popupMenuItems.childNodes.length;
			for( i = 0; i < numPopupMenuItems; i++ )
			{
				popupMenuItem = this.popupMenuItems.childNodes[i];
				if( popupMenuItem.popupItem.label.toLowerCase().indexOf(searchText) >= 0 )
					popupMenuItem.style.display = "block";
				else
					popupMenuItem.style.display = "none";
			}

			//console.log(this.value);
			changeTimeout = null;
		}.bind(elem), 300);
	}, true);

	optionSearchForm.appendChild(optionSearch);
	optionSearchContainer.appendChild(optionSearchForm);

	container.appendChild(popupMenuItems);
	container.appendChild(optionSearchContainer);

	blackout.appendChild(container);
	document.body.insertBefore(blackout, this.cursorElem);

	optionSearch.focus();
};

ArcadeHud.prototype.showPopupMenu = function(popupId, x, y, width, height, itemHeight, fontSize, selectedItem, rightAligned)
{
	console.log("Popup is: " + popupId);
	console.log(JSON.stringify(arguments));
	var popup = {
		"popupId": popupId,
		"x": parseInt(x),
		"y": parseInt(y),
		"width": parseInt(width),
		"height": parseInt(height),
		"itemHeight": parseInt(itemHeight),
		"fontSize": parseFloat(fontSize),
		"selectedItem": parseInt(selectedItem),
		"rightAligned": (rightAligned !== "0"),
		"items": new Array()
	};

	var argIndex = 9;
	var numArguments = arguments.length;
	var popupItem;
	while( argIndex <= arguments.length - 8)
	{
		popupItem = {
			"type": arguments[argIndex],
			"label": arguments[argIndex+1],
			"tooltip": arguments[argIndex+2],
			"action": parseInt(arguments[argIndex+3]),
			"right_to_left": (arguments[argIndex+4] !== "0"),
			"has_directional_override": (arguments[argIndex+5] !== "0"),
			"enabled": (arguments[argIndex+6] !== "0"),
			"checked": (arguments[argIndex+7] !== "0")
		};

		popup.items.push(popupItem);
		argIndex += 8;
	}

	var blackout = document.createElement("div");
	blackout.style.cssText = "background-color: transparent; position: fixed; top: 0; left: 0; bottom: 0; right: 0;";
	blackout.popup = popup;

	blackout.addEventListener("mousedown", function(e)
	{
		var elem = e.srcElement;
		// make sure the blackout elem is what got clicked
		if( !!!elem.popup )
			return;

		aaapi.system.didCancelPopupMenu(elem.popup.popupId);
		elem.parentNode.removeChild(elem);
	}, true);

	var container = document.createElement("div");
	container.className = "popupMenuContainer";
	container.style.left = popup.x - 2 + "px";
	container.style.top = popup.y + popup.height + "px";
	container.style.width = popup.width -2 + "px";
	//container.style.height = popup.height + "px";

	var popupMenuItems = document.createElement("div");
	popupMenuItems.className = "popupMenuItems";

	var i, option, optionTextNode;
	var numItems = popup.items.length;
	for( i = 0; i < numItems; i++ )
	{
		option = document.createElement("div");
		option.className = "popupMenuItem";
		option.style.fontSize = Math.round(popup.fontSize) + "px";
//		if( popup.selectedItem === i )
//			option.className += " selectedPopupMenuItem";

		option.blackout = blackout;		
		option.popupItem = popup.items[i];
		option.addEventListener("click", function(e)
		{
			var elem = e.srcElement;
			if( !!!elem.blackout )
				return;

			aaapi.system.didSelectPopupMenuItem(elem.blackout.popup.popupId, elem.blackout.popup.items.indexOf(elem.popupItem));
			elem.blackout.parentNode.removeChild(elem.blackout);
		}, true);

		optionTextNode = document.createTextNode(popup.items[i].label);
		option.appendChild(optionTextNode);
		popupMenuItems.appendChild(option);
	}

	var optionSearchContainer = document.createElement("div");
	optionSearchContainer.className = "popupMenuItemSearch";

	var optionSearchForm = document.createElement("form");
	optionSearchForm.addEventListener("submit", function(e)
	{
		console.log("SECOND ONE");
		//console.log(e.srcElement.firstChild.popupMenuItems.childNodes);
		var elem = e.srcElement.firstChild;

		var i;
		var count = 0;
		var firstIndex = -1;
		var numNodes = elem.popupMenuItems.childNodes.length;
		for( i = 0; i < numNodes && count < 2; i++ )
		{
			if( elem.popupMenuItems.childNodes[i].style.display !== "none" )
			{
				if( count === 0 )
					firstIndex = i;

				count++;
			}
		}

		if( count > 0 )// === 1 )
		{
			var optionElem = elem.popupMenuItems.childNodes[firstIndex];
			aaapi.system.didSelectPopupMenuItem(optionElem.blackout.popup.popupId, optionElem.blackout.popup.items.indexOf(optionElem.popupItem));
			optionElem.blackout.parentNode.removeChild(optionElem.blackout);
		}

		e.preventDefault();
		return false;
	}, true);

	var optionSearch = document.createElement("input");
	optionSearch.popupMenuItems = popupMenuItems;
	optionSearch.type = "text";
	optionSearch.placeholder = "Search...";
	optionSearch.changeTimeout = null;

	optionSearch.addEventListener("input", function(e)
	{
		var elem = e.srcElement;
		if( elem.changeTimeout )
			clearTimeout(elem.changeTimeout);

		elem.changeTimeout = setTimeout(function()
		{
			// step through all options and set display mode accordingly
			var searchText = this.value.toLowerCase();

			var i, popupMenuItem;
			var numPopupMenuItems = this.popupMenuItems.childNodes.length;
			for( i = 0; i < numPopupMenuItems; i++ )
			{
				popupMenuItem = this.popupMenuItems.childNodes[i];
				if( popupMenuItem.popupItem.label.toLowerCase().indexOf(searchText) >= 0 )
					popupMenuItem.style.display = "block";
				else
					popupMenuItem.style.display = "none";
			}

			//console.log(this.value);
			changeTimeout = null;
		}.bind(elem), 300);
	}, true);

	optionSearchForm.appendChild(optionSearch);
	optionSearchContainer.appendChild(optionSearchForm);

	container.appendChild(popupMenuItems);
	container.appendChild(optionSearchContainer);

//container.style.top = popup.y + popup.height + "px";
	blackout.appendChild(container);
	document.body.insertBefore(blackout, this.cursorElem);

	var offsetHeight = container.offsetHeight;
	console.log(parseInt(container.style.top));
	if( parseInt(container.style.top) + offsetHeight > window.innerHeight )
		container.style.top = parseInt(container.style.top) - popup.height - offsetHeight + "px";

	optionSearch.focus();

	//document.body.appendChild(container);
/*
	for( var i = 0; i < arguments.length; i++ )
	{
		console.log("arg: " + arguments[i]);
	}
*/
};

ArcadeHud.prototype.assignHelp = function(elem)
{
	elem.addEventListener("mouseover", function(e)
	{
		arcadeHud.addHelpMessage(this.getAttribute("help"));
	}.bind(elem), true);

	elem.addEventListener("mouseout", function(e)
	{
		arcadeHud.removeHelp();
	}.bind(elem), true);
};

ArcadeHud.prototype.removeHelp = function()
{
	// empty out messages
	var firstChild = this.helpElem.firstChild;
	while( firstChild )
	{
		if( firstChild.className.search(/\bhudLoadingMessagesContainer\b/) >= 0 )
			break;

		this.helpElem.removeChild(firstChild);
		firstChild = this.helpElem.firstChild;
	}

	// if there are no loading messages, hide the message slate
	if( !firstChild || !!!firstChild.firstChild )
		this.helpElem.style.display = "none";
};

ArcadeHud.prototype.addHelpMessage = function(text)
{
	// empty out messages
	var firstChild = this.helpElem.firstChild;
	while( firstChild )
	{
		if( firstChild.className.search(/\bhudLoadingMessagesContainer\b/) >= 0 )
			break;

		this.helpElem.removeChild(firstChild);
		firstChild = this.helpElem.firstChild;
	}

	var helpText = document.createElement("div");
	helpText.className = "helpMessage";
	var helpTextNode = document.createTextNode(text);
	helpText.appendChild(helpTextNode);
	this.helpElem.insertBefore(helpText, this.helpElem.firstChild);
	this.helpElem.style.display = "block";
};

//ArcadeHud.prototype.metaSearch = function(scraperId, callback)
//aaapi.system.metaSearch(id, elem.field, query);

ArcadeHud.prototype.metaSearchEasy = function()
{
	//console.log("Scooby do!");
	//console.log(this.scraper);

	var item = aaapi.library.getSelectedLibraryItem();	// FIXME: This is probably overkill if all we want is the ID!
	if( item )
		window.location='asset://ui/metaSearch.html?id=' + encodeURIComponent(item.info.id);
};

ArcadeHud.prototype.metaSearch = function(itemId, field, scraperId, term)
{
	var scraper = this.scrapers[scraperId];
	if( !!scraper && !!scraper.search && (term === "" || scraper.search.indexOf("$TERM") >= 0) )
	{
		var query = (term !== "") ? scraper.search.replace("$TERM", term) : scraper.homepage;

		// set this as the active scraper so it can handle page loaded events with scraper logic
		//console.log("set active scraper");
		//this.activeScraper = scraper;

		//this.metaSearchHandles[id] = {"scraper": scraper, "callback": callback};
		aaapi.system.metaSearch(scraperId, itemId, field, query);
	}
	else
		console.log("ERROR: Invalid or unsupported scraper ID received.");
};

ArcadeHud.prototype.metaScrapeCurrent = function()
{
	this.metaScrape(this.activeScraper.id, this.activeScraperField, function(scrapedData)
	{
		if( this.activeScraper.id === "importSteamGames" )
		{
			console.log("Time to send " + (scrapedData.length / 2) + " Steam items to AArcade...");

			var success = aaapi.system.importSteamGames(scrapedData);

			if( success )
			{
				console.log("Steam games imported!");

				//aaapi.system.autoInspect(this.activeScraperItemId);
				aaapi.system.deactivateInputMode(true);
			}
			else
				console.log("Import rejected!");
		}
		else
		{
			console.log("Scraped data is: ");
			console.log(JSON.stringify(scrapedData));
			var usedFields = [];
			var args = [];
			var x, field;
			for( x in scrapedData)
			{
				field = scrapedData[x];
				//if( field === "" || (this.activeScraperField !== "all" && this.activeScraperField !== x))
				console.log(this.activeScraperField);
				if( this.activeScraperField !== "all" && this.activeScraperField !== x)
					continue;

				if( x === "type" )
				{
					var allTypes = aaapi.library.getAllLibraryTypes();
					var y;
					for( y in allTypes )
					{
						//console.log(allTypes[y].title);
						if( allTypes[y].title === field )
						{
							field = allTypes[y].info.id;
							break;
						}
					}
				}
				
				/*
				var inputs = document.querySelectorAll("input, select");
				var i;
				for( i = 0; i < inputs.length; i++ )
				{
					if( inputs[i].field === x )
					{
						//inputs[i].focus();
						inputs[i].value = field;
						break;
					}
				}
				*/

				args.push(x);
				args.push(field);
				usedFields.push(x);
			}

			if( this.activeScraperItemId !== "" )
			{
				var success = aaapi.library.updateItem(this.activeScraperItemId, args);				

				if( success )
				{
					console.log("Item updated!");

					aaapi.system.autoInspect(this.activeScraperItemId);
					aaapi.system.deactivateInputMode();
				}
				else
					console.log("Item update rejected!");
			}
			else
			{
				// first, check if an item that matches this one already exists...
	 			var item = aaapi.library.findLibraryItem("file", scrapedData.file);
	 			if( !item && !!scrapedData.refrence && scrapedData.refrence !== "" )
	 				item = aaapi.library.findLibraryItem("refrence", scrapedData.refrence);

	 			if( item )
	 			{
	 				//console.log("ybi A");
	 				aaapi.system.setLibraryBrowserContext("items", item.info.id, "", "");
	 				aaapi.system.spawnItem(item.info.id);
	 			}
	 			else
	 			{
	 				//// make sure the item has all fields
	 				//var vitalFields = ["title", "type", "app", "file", "reference", "download", "stream", "preview", "screen", "marquee", "description"];
	 				//for( var w = 0; w < vitalFields.length; w++ )
	 				//	if( !!!item[vitalFields[w]] )
	 				//		item[vitalFields[w]] = "";
	 					
					var createdItemId = aaapi.library.saveItem(this.activeScraperItemId, args);	// the response is actually the item ID or FALSE
					if( createdItemId )
					{
						aaapi.system.setLibraryBrowserContext("items", createdItemId, "", "");
						aaapi.system.spawnItem(createdItemId);
					}
				}
			}
		}
	}.bind(this));
};

ArcadeHud.prototype.metaScrape = function(scraperId, field, callback)
{
	var id = "meta" + Math.round(Math.random() * 10.0).toString() + Math.round(Math.random() * 10.0).toString() + Math.round(Math.random() * 10.0).toString() + Math.round(Math.random() * 10.0).toString();
	//var id = "run";
	var scraper = this.scrapers[scraperId];
	if( !!scraper )
	{
		var dummy = new Object();
		dummy.scraper = scraper;
		dummy.callback = callback;
		dummy.field = field;
		//this.metaScrapeHandles[id] = {"scraper": scraper, "callback": callback};

		var scraperObject = {"scraper": scraper, "callback": function(callId, url, doc)
		{
			// clear the a active scraper
			//console.log("clear active scraper");
			//arcadeHud.activeScraper = null;

			var results = this.scraper.run(url, this.field, doc);

			if( Array.isArray(results) )
			{
				//aaapi.system.importSteamGames(steamItems);	// do this on what ever is calling run.
			}
			else
			{
				// strip everything out of the response except what was asked for.
				if( this.field !== "all" )
				{
					var shitList = [];

					var x;
					for( x in results )
					{
						if( dummy.field !== x )
							shitList.push(x);
					}

					var i;
					var max = shitList.length;
					for( i = 0; i < max; i++ )
						delete results[shitList[i]];
				}

				// eliminate duplicates intellegently
				if( !!results.file && results.file !== "" )
				{
					// if there is a file, do not use duplicates on anything else
					if( results.file === results.reference )
						results.reference = "";
						//delete results["reference"];

					if( results.file === results.preview )
						results.preview = "";
	//					delete results["preview"];

					//if( results.file === results.stream )
					//	results.stream = "";
						//delete results["stream"];

					if( results.file === results.download )
						results.download = "";
						//delete results["download"];

				//	if( results.file === results.screen )
				//		results.screen = "";
						//delete results["screen"];

				//	if( results.file === results.marquee )
				//		results.marquee = "";
						//delete results["marquee"];
				}

				if( !!results.stream && results.stream !== "" )
				{
					// if there is a stream, do not use duplicates on anything
					if( results.stream === results.preview )
						results.preview = "";
						//delete results["preview"];

					if( results.stream === results.download )
						results.download = "";
						//delete results["download"];
				}

				if( !!results.preview && results.preview !== "" )
				{
					// if there is a preview, do not use duplicates on anything
					if( results.preview === results.download )
						results.download = "";
						//delete results["download"];

					if( results.file === results.screen )
						results.screen = "";
						//delete results["screen"];

					if( results.file === results.marquee )
						results.marquee = "";
						//delete results["marquee"];
				}

				if( !!results.download && results.download !== "" )
				{
					// if there is a download, do not use duplicates on anything
					if( results.download === results.screen )
						results.screen = "";
						//delete results["screen"];

					if( results.download === results.marquee )
						results.marquee = "";
						//delete results["marquee"];
				}

				if( !!results.screen && results.screen !== "" )
				{
					// if there is a screen, do not use duplicates on anything
					if( results.screen === results.marquee )
						results.marquee = "";
						//delete results["marquee"];
				}
			}

			this.callback(results);
		}.bind(dummy)};

		if( scraper.id === "netflix" )
		{
			var content = "<html><title>" + this.pageTitle + "</title></html>";
			var doc = arcadeHud.DOMParser.parseFromString(content, "text/html");
			scraperObject.callback.call(dummy, id, this.url, doc);
		}
		else
		{
			this.metaScrapeHandles[id] = scraperObject;
			aaapi.system.getDOM(id, scraperId);
		}
	}
	else
		console.log("ERROR: Invalid scraper ID received.");
};

ArcadeHud.prototype.browseForFile = function(callback)
{
	var browseId = "browse" + Math.round(Math.random() * 10.0).toString() + Math.round(Math.random() * 10.0).toString() + Math.round(Math.random() * 10.0).toString() + Math.round(Math.random() * 10.0).toString();
	this.fileBrowseHandles[browseId] = {"callback": callback};
	aaapi.system.fileBrowse(browseId);
};

ArcadeHud.prototype.onBrowseFileSelected = function(browseId, response)
{
	var browseInfo = this.fileBrowseHandles[browseId];
	var callback = browseInfo.callback;

	delete this.fileBrowseHandles[browseId];

	callback(response);
};

ArcadeHud.prototype.onBrowserFinishedRequest = function(url, scraperId, itemId, field)
{
	console.log("Finished a request " + scraperId + ": " + url);
	var scraper = this.scrapers[scraperId];
	if( !!scraper )
	{
		console.log(typeof scraper.test);
		if(typeof scraper.test === "function")
		{
			console.log("yarrrrrrr");
			//scraper.test();
			var id = "meta" + Math.round(Math.random() * 10.0).toString() + Math.round(Math.random() * 10.0).toString() + Math.round(Math.random() * 10.0).toString() + Math.round(Math.random() * 10.0).toString();
			//var id = "test";

			var dummy = new Object();
			dummy.scraper = scraper;
			dummy.itemId = itemId;
			dummy.field = field;
//			dummy.callback = callback;

			if( this.metaScrapeHandles[id] )
				console.log("WARNING: handle already exists for this scraper!  Should probably abort, but not aborting right now.");

			this.metaScrapeHandles[id] = {"scraper": scraper, "callback": function(callId, url, doc)
			{
				console.log("run test logic");
				this.scraper.test(url, doc, function(response)
				{
					var container = document.querySelector("#hudSideScrapeContainer");
					if(container)
					{
						if( response.validForScrape )
						{
							console.log("Display the 'scrape field' prompt for " + this.scraper.title + "'s " + this.field + " for item " + this.itemId);
							container.style.display = "block";
							container.scraperId = this.scraper.id;
							container.itemId = this.itemId;
							container.field = this.field;
							return;
						}
						else
							container.style.display = "none";
					}

					if( !response.validForScrape && !!response.redirect && response.redirect !== "")
					{
						//var dummy3 = {"scraper": this.scraper, ""};
						aaapi.system.metaSearch(this.scraper.id, this.itemId, this.field, response.redirect);
					}
				}.bind(this));
			}.bind(dummy)};

			var delay = scraper.testDelay;
			if( !!!delay )
				delay = 0;

			var dummy2 = {"id": id, "scraper": scraper};
			setTimeout(function()
			{
				//console.log("Get that DOM: " + this.id);

				// don't get the DOM of Netflix, just check the URL.
				// TODO: Add a standard way for scrapers to do this.
				if( this.scraper.id === "netflix" )
				{
					this.scraper.test(url, null, function(response)
					{
						var container = document.querySelector("#hudSideScrapeContainer");
						if(container)
						{
							if( response.validForScrape )
							{
								//console.log("Display the 'scrape field' prompt for " + this.scraper.title + "'s " + this.field + " for item " + this.itemId);
								container.style.display = "block";
								container.scraperId = this.scraper.id;
								container.itemId = this.itemId;
								container.field = this.field;
								return;
							}
							else
								container.style.display = "none";
						}

						if( !response.validForScrape && !!response.redirect && response.redirect !== "")
						{
							//var dummy3 = {"scraper": this.scraper, ""};
							aaapi.system.metaSearch(this.scraper.id, this.itemId, this.field, response.redirect);
						}
					}.bind(this));
				}
				else
					aaapi.system.getDOM(this.id, this.scraper.id);
			}.bind(dummy2), delay);
		}

		this.activeScraper = scraper;
		this.activeScraperItemId = itemId;
		this.activeScraperField = field;
	}
	//console.log("yar yar yar");
	//console.log(this.activeScraper);
	//if( this.activeScraper )
	//{
//		console.log("twat teh funnuck");
	//}
	//console.log("Main frame just finished loading " + url);
	//console.log("Should any scrapers analyze the newly loaded page?");
};

ArcadeHud.prototype.getParameterByName = function(name, url)
{
	// http://stackoverflow.com/questions/901115/how-can-i-get-query-string-values-in-javascript
    if (!url) url = window.location.href;
    name = name.replace(/[\[\]]/g, "\\$&");
    var regex = new RegExp("[?&]" + name + "(=([^&#]*)|&|#|$)"),
        results = regex.exec(url);
    if (!results) return null;
    if (!results[2]) return '';
    return decodeURIComponent(results[2].replace(/\+/g, " "));
};

ArcadeHud.prototype.loadItemMarqueeImage = function(imageElem, item)
{
	imageElem.src = item.marquee;
	imageElem.style.display = "block";
};

ArcadeHud.prototype.viewStream = function()
{
	var item = aaapi.library.getSelectedLibraryItem();	// FIXME: This is probably overkill if all we want is the ID!
	if( item )
		aaapi.system.viewStream(item.info.id);
};

// Originally from https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/encodeURIComponent
ArcadeHud.prototype.encodeRFC5987ValueChars = function(str){
    return encodeURIComponent(str).
        // Note that although RFC3986 reserves "!", RFC5987 does not,
        // so we do not need to escape it
        replace(/['()]/g, escape). // i.e., %27 %28 %29
        replace(/\*/g, '%2A').
            // The following are not required for percent-encoding per RFC5987, 
            // so we can allow for a little better readability over the wire: |`^
            replace(/%(?:7C|60|5E)/g, unescape);
};

ArcadeHud.prototype.onTitleChanged = function(title)
{
	//console.log("Page title changed to: " + title);
	this.pageTitle = title;
};

ArcadeHud.prototype.onDOMGot = function(url, response)
{
/*
	var totalSource = response;
	var partialSource;
	while( totalSource.length > 0 )
	{
		partialSource = totalSource.substring(0, 1000);
		totalSource = totalSource.substring(1000);
		console.log(partialSource);
	}
*/
	
	var index = response.indexOf("AAAPICALL");
	var callId = response.substring(0, index);
	if( !!this.metaScrapeHandles[callId] )
	{
		var scraper = this.metaScrapeHandles[callId].scraper;
		var callback = this.metaScrapeHandles[callId].callback;

		var doc;
		var content = response.substring(index + 9);
		if( scraper.id === "importSteamGames")
		{
			doc = JSON.parse(decodeURIComponent(content));
		}
		else
		{
//			var crap = decodeURIComponent(content);
			content = "<html>" + decodeURIComponent(content) + "</html>";
			doc = arcadeHud.DOMParser.parseFromString(content, "text/html");
/*
					if( false || content.indexOf("id=\"page\"") >= 0 )
					{
						var elemm = document.createElement("textarea");
						elemm.value = crap;
						elemm.style.cssText = "position: absolute; width: 80%; height: 80%; top: 20px; left: 20px;";
						document.body.appendChild(elemm);
						elemm.focus();
						elemm.select();
					}*/
		}

		//var scraper = this.metaScrapeHandles[callId].scraper;
		//var callback = this.metaScrapeHandles[callId].callback;
		delete this.metaScrapeHandles[callId];

		// FIXME: WHEN DOES THE BROWSER INSTANCE CLEAR ITS ACTIVE SCRAPER????? It doesn't, but it needs to.
		//var results = scraper[callId](url, doc);
		//callback(results);
		callback(callId, url, doc);
	}
	else
		console.log("ERROR: DOM received with no matching scrape handle.");
};

ArcadeHud.prototype.onDOMReady = function()
{
	// Async
	return {
			"then": function(DOMReadyCallback)
			{
				if( document.readyState === "interactive" || document.readyState === "complete" )
					DOMReadyCallback();
				else
				{
					function readyWatch(DOMEvent)
					{
						DOMReadyCallback();
					}

					document.addEventListener("DOMContentLoaded", readyWatch.bind(this), true);
				}
			}.bind(this)
		};
};

ArcadeHud.prototype.loadHeadScript = function(scriptName)
{
	// load all scrapers before the body loads
	var xhrObj = new XMLHttpRequest();
	xhrObj.open('GET', scriptName, false);
	xhrObj.send('');

	var se = document.createElement('script');
	se.type = "text/javascript";
	se.text = xhrObj.responseText;

	document.getElementsByTagName('head')[0].appendChild(se);
};

ArcadeHud.prototype.addScraper = function(scraper)
{
	this.scrapers[scraper.id] = scraper;
};

ArcadeHud.prototype.init = function()
{
	this.loadHeadScript("scrapers.js");
};

var arcadeHud = new ArcadeHud();
arcadeHud.init();
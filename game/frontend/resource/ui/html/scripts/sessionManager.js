function SessionManager()
{
	this.options = {
		"server": "",
		"universe": "",
		"instance": ""
	};

	this.metaverse;

	this.server = "";
	this.universe = "";
	this.instance = "";

	this.modalMessageElem;
	this.playerCursorElem;
	this.playerMarkerElems = {};
	this.playerMarkerWidth;
	this.playerCursorElem;
	this.overviewElem;
	this.overviewWidth;
	this.viewerElem;
	this.viewerWidth;
	this.viewerHeight;

	this.overviewInfo;
	this.overviewInfos = {};
	this.overviewInfos["ge_bunker_classic.bsp"] = {"file": "overviews/ge_bunker_classic.png", "scale": 3.50, "pos_x": -1487, "pos_y": 2017};
	this.overviewInfos["sm_apartmentsuite.bsp"] = {"file": "overviews/sm_apartmentsuite.png", "scale": 1.00, "pos_x": -405, "pos_y": 955};
	this.overviewInfos["meta_hood.bsp"] = {"scale": 8.00, "pos_x": -4193, "pos_y": 7291, "shift_x": 30, "shift_y": 0};
	this.overviewInfos["sm_apartment.bsp"] = {"scale": 0.80, "pos_x": -244, "pos_y": 555, "shift_x": -250, "shift_y": 0};
	this.overviewInfos["sm_acreage.bsp"] = {"scale": 1.30, "pos_x": -612, "pos_y": 791, "shift_x": -40, "shift_y": 0};
	this.overviewInfos["sm_expo.bsp"] = {"scale": 3.30, "pos_x": -1167, "pos_y": 2103, "shift_x": -165, "shift_y": 0};
	this.overviewInfos["sm_orchard.bsp"] = {"scale": 2.00, "pos_x": -533, "pos_y": 832, "shift_x": -225, "shift_y": 10};
	this.overviewInfos["sm_gallery.bsp"] = {"scale": 2.00, "pos_x": -816, "pos_y": 898, "shift_x": -110, "shift_y": 0};
	this.overviewInfos["ge_casino.bsp"] = {"scale": 4.30, "pos_x": -1206, "pos_y": 2354, "shift_x": 0, "shift_y": 0};
	this.overviewInfos["cyberpunk_3.bsp"] = {"scale": 6.00, "pos_x": 3511, "pos_y": 3345, "shift_x": -20, "shift_y": 0};
	this.overviewInfos["fof_robertlee.bsp"] = {"scale": 6.00, "pos_x": 2080, "pos_y": 3361, "shift_x": -235, "shift_y": -125};
}

SessionManager.prototype.getParameterByName = function(name, url)
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

SessionManager.prototype.loadItemBestImage = function(imageElem, item, callback, overrideUrl)
{
	var dummy = {
		"imageElem": imageElem,
		"item": item,
		"potentials":
		{
			"marquee": true,
			"screen": true,
			"preview": true,
			"file": true
		},
		"re": /(.bmp|.ico|.gif|.jpg|.jpeg|.jpe|.jp2|.pcx|.pic|.png|.pix|.raw|.tga|.tif|.vtf|.tbn)$/i
	};

	function getNextPotential()
	{
		var i;
		var keys = Object.keys(this.potentials);
		var potential;
		for( i = 0; i < keys.length; i++ )
		{
			potential = keys[i];//this.potentials[keys[i]];
			//console.log(potential);

			if( !!this.item[potential] && this.item[potential].match(this.re) )
				return potential;
			else
				delete this.potentials[potential];
		}
	}

	function tryPotential()
	{
		var potential = getNextPotential.call(this);

		//console.log("Potential is: " + potential);
		if( !!potential )
			this.imageElem.src = item[potential];
			//tryPotential.call(this);		
	}

	imageElem.addEventListener("error", function()
	{
		// remove the failed potential
		var potential = getNextPotential.call(this);
		if( !!potential )
			delete this.potentials[potential];

		tryPotential.call(this);
	}.bind(dummy));

	imageElem.addEventListener("load", function()
	{
		if( !!callback )
			callback.call(this.imageElem);
		else
			this.imageElem.style.display = "block";
	}.bind(dummy));

	if( !!overrideUrl )
		dummy.imageElem.src = overrideUrl;
	else	
		tryPotential.call(dummy);
};

SessionManager.prototype.connect = function(options, callback)
{
	// temp test stuff...
	//this.overviewInfo = this.overviewInfos["ge_bunker_classic"];//sm_apartmentsuite"];
	//this.playerMarkerElem = document.querySelector(".playerMarker");
	//this.playerMarkerWidth = this.playerMarkerElem.offsetWidth;
	this.playerCursorElem = document.querySelector("#playerCursor");
	this.overviewElem = document.querySelector(".overview");
	this.overviewWidth = this.overviewElem.offsetWidth;
	this.viewerElem = document.querySelector("#viewer");
	this.viewerWidth = this.viewerElem.offsetWidth;
	this.viewerHeight = this.viewerElem.offsetHeight;
	//this.overviewImageElem = document.querySelector("#overviewImage");
	// end temp test stuff




	window.onInstanceUserAdded = this.onInstanceUserAdded.bind(this);
	window.onInstanceChanged = this.onInstanceChanged.bind(this);
	window.onUserSessionUpdated = this.onUserSessionUpdated.bind(this);
	window.onEntryChanged = this.onEntryChanged.bind(this);
	this.metaverse = new Metaverse();

	// set any custom options
	for( x in this.options )
	{
		if( options.hasOwnProperty(x) )
			this.options[x] = options[x];
	}

	var paramServer = this.getParameterByName("server");
	var paramUniverse = this.getParameterByName("universe");
	var paramInstance = this.getParameterByName("instance");

	this.server = (this.options.server !== "") ? this.options.server : "";
	if( this.server === "" )
		this.server = (!!paramServer) ? codeURIComponent(paramServer) : "https://metaverse.firebaseio.com/"

	this.universe = this.options.universe;
	if( this.universe === "" && !!paramUniverse)
		this.universe = paramUniverse;

	this.instance = this.options.instance;
	if( this.instance === "" && !!paramInstance)
		this.instance = paramInstance;

	this.metaverse.connect(this.server, function()
	{
		console.log("Initial connection...");
		if( this.universe !== "" )
		{
			this.metaverse.joinUniverse(this.universe, function()
			{
				if( this.instance !== "" )
				{
					this.metaverse.connectInstance(this.instance, function(connectInfo)
					{
						var mapFile = connectInfo.map.current.platforms["-KJvcne3IKMZQTaG7lPo"]
.file;

						if( !!this.overviewInfos[mapFile] )
							this.overviewInfo = this.overviewInfos[mapFile];
						else
							this.overviewInfo = this.overviewInfos["sm_apartmentsuite.bsp"];
						
						var imageElem = document.createElement("img");
						imageElem.className = "overviewImage";
						imageElem.style.width = this.overviewWidth + "px";
						imageElem.src = this.overviewInfo.file;
						imageElem.addEventListener("load", function(e)
						{
							setTimeout(function()
							{
								callback();
							}.bind(this), 500);
						}.bind(this));

						this.overviewElem.appendChild(imageElem);
						this.overviewImageElem = imageElem;
					}.bind(this));
				}
				else
					callback();
			}.bind(this));
		}
		else
			callback();
	}.bind(this));
};

SessionManager.prototype.adjustOrigin = function(position)
{
	var max = this.overviewWidth;
	var mid = max / 2;

	var x = (position.x - this.overviewInfo.pos_x) / this.overviewInfo.scale;
	var y = -(position.y - this.overviewInfo.pos_y) / this.overviewInfo.scale;

	x -= 12 / this.overviewInfo.scale;
	y -= 12 / this.overviewInfo.scale;

	x *= (this.overviewWidth / 1024.0);
	y *= (this.overviewWidth / 1024.0);

	return {"x": x, "y": y, "z": position.z};
};

SessionManager.prototype.modalMessage = function(content)
{
	if( this.modalMessageElem )
		this.modalMessageElem.parentNode.removeChild(this.modalMessageElem);

	var modalMessageElem = document.createElement("div");
	modalMessageElem.style.cssText = "position: fixed; top: 0; bottom: 0; left: 0; right: 0; background-color: rgba(0, 0, 0.7);";
	modalMessageElem.innerHTML = content;
	modalMessageElem.addEventListener("click", function(e)
	{
		this.modalMessageElem.parentNode.removeChild(this.modalMessageElem);
		this.modalMessageElem = undefined;
	}.bind(this), true);
	document.body.appendChild(modalMessageElem);

	this.modalMessageElem = modalMessageElem;
};

SessionManager.prototype.shortcutMouseOut = function(e)//shortcutContainerElem)
{
	var sessionManager = window.sessionManager;
	//var shortcutContainerElem = e.target.parentNode;
	var shortcutContainerElem = (!!e && !!e.target) ? e.target.parentNode : this;

	var selectedEntriesElems = document.querySelectorAll(".selectedEntryContainer[objectId='" + shortcutContainerElem.getAttribute("objectId") + "']");
	if( selectedEntriesElems.length > 0 )//shortcutContainerElem.classList.contains("shortcutContainerElem"))
		return;

	var shortcutElem = shortcutContainerElem.querySelector(".shortcut");
	var objectSlateElem = shortcutContainerElem.querySelector(".objectSlate");
	//if( objectSlateElem.style.width === "100%" )
	//	return;

	shortcutElem.style.webkitTransitionDelay = "0.2s";
	shortcutElem.style.transitionDelay = "0.2s";

	objectSlateElem.style.webkitTransitionDelay = "0s";
	objectSlateElem.style.transitionDelay = "0s";

	//sessionManager.shortcutMouseOut.call(sessionManager, shortcutContainerElem);

	/*
	//var shortcutContainerElem = e.target.parentNode;
	var shortcutElem = shortcutContainerElem.querySelector(".shortcut");
	var objectSlateElem = shortcutContainerElem.querySelector(".objectSlate");
*/
	objectSlateElem.style.width = "0";
	objectSlateElem.style.opacity = 0;
	//objectSlateElem.style.transform = "scale(0.1)";

	shortcutElem.style.transform = "scale(1.0)";
	shortcutElem.style.zIndex = shortcutElem.oldZ;
	shortcutContainerElem.style.zIndex = shortcutElem.oldZ;

	objectSlateElem.offsetWidth;

	shortcutElem.style.webkitTransitionDelay = "0s";
	shortcutElem.style.transitionDelay = "0s";

	objectSlateElem.style.webkitTransitionDelay = "0.2s";
	objectSlateElem.style.transitionDelay = "0.2s";
};

SessionManager.prototype.switchToTab = function(targetContentElem)
{
	var oldSelectedTab = this.parentNode.querySelector(".activeTab");
	oldSelectedTab.classList.remove("activeTab");

	var itemContentElem = this.parentNode.parentNode.querySelector(".itemInfoContainer");
	if( !!itemContentElem )
		itemContentElem = this.parentNode.parentNode.querySelector(".itemInfoContainer").parentNode;

	var modelContentElem = this.parentNode.parentNode.querySelector(".modelInfoContainer");
	if( !!modelContentElem )
		modelContentElem = this.parentNode.parentNode.querySelector(".modelInfoContainer").parentNode;

	var viewerContentElem = this.parentNode.parentNode.querySelector("#viewer");
	if( !!viewerContentElem )
		viewerContentElem = this.parentNode.parentNode.querySelector("#viewer").parentNode;


	var propsContentElem = this.parentNode.parentNode.querySelector(".propsContainer");
	if( !!propsContentElem )
		propsContentElem = this.parentNode.parentNode.querySelector(".propsContainer").parentNode;

	var shortcutsContentElem = this.parentNode.parentNode.querySelector(".shortcutsContainer");
	if( !!shortcutsContentElem )
		shortcutsContentElem = this.parentNode.parentNode.querySelector(".shortcutsContainer").parentNode;

	if( targetContentElem === itemContentElem || targetContentElem === modelContentElem || targetContentElem === viewerContentElem )
	{
		if( targetContentElem === itemContentElem )
			itemContentElem.style.display = "block";
		else
			itemContentElem.style.display = "none";

		if( targetContentElem === modelContentElem )
			modelContentElem.style.display = "block";
		else
			modelContentElem.style.display = "none";

		if( targetContentElem === viewerContentElem )
			viewerContentElem.style.display = "block";
		else
			viewerContentElem.style.display = "none";
	}
	else
	{
		if( targetContentElem === propsContentElem )
			propsContentElem.style.display = "block";
		else
			propsContentElem.style.display = "none";

		if( targetContentElem === shortcutsContentElem )
			shortcutsContentElem.style.display = "block";
		else
			shortcutsContentElem.style.display = "none";
	}

	this.classList.add("activeTab");
};

SessionManager.prototype.entryClick = function(e)
{
	var sessionManager = window.sessionManager;

	//this.parentNode.querySelector to keep it within the panel... if you want that.
	var oldSelectedElem = document.querySelector(".selectedEntryContainer");
	if( !!oldSelectedElem )
	{
		oldSelectedElem.classList.remove("selectedEntryContainer");
		
		var oldShortcutContainerElem = this.sessionManager.overviewElem.querySelector("div[objectId='" + oldSelectedElem.getAttribute("objectId") + "']");
		sessionManager.shortcutMouseOut.call(oldShortcutContainerElem);
	}

	this.classList.add("selectedEntryContainer");

	var shortcutContainerElem = this.sessionManager.overviewElem.querySelector("div[objectId='" + this.getAttribute("objectId") + "']");
	sessionManager.shortcutMouseOver.call(shortcutContainerElem);

	// fill the item panel (if needed)
	var bestItemImageElem = document.querySelector(".bestItemImage");
	bestItemImageElem.style.display = "none";

	var panelElem = bestItemImageElem.parentNode.parentNode.parentNode.parentNode;
	var noEntryElem = bestItemImageElem.parentNode.parentNode.parentNode.querySelector(".noEntry");

	var itemId = this.getAttribute("itemId");
	var item = (!!sessionManager.metaverse.library.items[itemId]) ? sessionManager.metaverse.library.items[itemId].current : undefined;
	if( item )
	{
		var url = "autoInspectItem.html?title=" + encodeURIComponent(item.title) + "&preview=" + encodeURIComponent(item.preview) + "&file=" + encodeURIComponent(item.file) + "&screen=" + encodeURIComponent(item.screen) + "&marquee=" + encodeURIComponent(item.marquee);
		sessionManager.viewerElem.src = url;
		window.alwaysBringToFront.call(sessionManager.viewerElem.parentNode.parentNode);

		sessionManager.switchToTab.call(document.querySelector("#viewScreenTab"), document.querySelector("#viewer").parentNode);

		noEntryElem.style.display = "none";

		var loadingIndicatorElem = panelElem.querySelector(".loadingIndicator");
		loadingIndicatorElem.parentNode.style.display = "block";

		sessionManager.loadItemBestImage(bestItemImageElem, item, function()
		{
			var panelElem = this.parentNode.parentNode.parentNode.parentNode;
		
			var loadingIndicatorElem = panelElem.querySelector(".loadingIndicator");
			loadingIndicatorElem.parentNode.style.display = "none";

			this.style.display = "block";
			//alwaysBringToFront.call(panelElem);
		});

		// fill the model panel (if needed)
		var bestModelImageElem = document.querySelector(".bestModelImage");
		bestModelImageElem.style.display = "none";

		var panelElem = bestModelImageElem.parentNode.parentNode.parentNode.parentNode;
		var noEntryElem = bestModelImageElem.parentNode.parentNode.parentNode.querySelector(".noEntry");

		var modelId = this.getAttribute("modelId");
		var model = (!!sessionManager.metaverse.library.models[modelId]) ? sessionManager.metaverse.library.models[modelId].current : undefined;
		if( model )
		{
			noEntryElem.style.display = "none";

			var overrideUrl = model.platforms["-KJvcne3IKMZQTaG7lPo"].file.toLowerCase();
			var found = overrideUrl.lastIndexOf("/");
			if( found < 0 )
				found = overrideUrl.lastIndexOf("\\");
			if( found >= 0 )
				overrideUrl = overrideUrl.substring(found+1);
			found = overrideUrl.indexOf(".mdl");
			if( found >= 0 )
				overrideUrl = overrideUrl.substring(0, found);
			overrideUrl = "cabinets/" + overrideUrl + ".png";
		
			var loadingIndicatorElem = panelElem.querySelector(".loadingIndicator");
			loadingIndicatorElem.parentNode.style.display = "block";

			sessionManager.loadItemBestImage(bestModelImageElem, model, function()
			{
				var panelElem = this.parentNode.parentNode.parentNode.parentNode;
		
				var loadingIndicatorElem = panelElem.querySelector(".loadingIndicator");
				loadingIndicatorElem.parentNode.style.display = "none";

				this.style.display = "block";
				//alwaysBringToFront.call(panelElem);
			}, overrideUrl);
		}
	}
	else
	{
		// Remove the item panel stuff
		var bestItemImageElem = document.querySelector(".bestItemImage");
		bestItemImageElem.style.display = "none";
		var panelElem = bestItemImageElem.parentNode.parentNode.parentNode.parentNode;
		var noEntryElem = bestItemImageElem.parentNode.parentNode.parentNode.querySelector(".noEntry");
		noEntryElem.style.display = "block";
		var loadingIndicatorElem = panelElem.querySelector(".loadingIndicator");
		loadingIndicatorElem.parentNode.display = "none";

		// Remove the model panel stuff
		var bestModelImageElem = document.querySelector(".bestModelImage");
		bestModelImageElem.style.display = "none";
		var panelElem = bestModelImageElem.parentNode.parentNode.parentNode.parentNode;
		var noEntryElem = bestModelImageElem.parentNode.parentNode.parentNode.querySelector(".noEntry");
		noEntryElem.style.display = "block";
		var loadingIndicatorElem = panelElem.querySelector(".loadingIndicator");
		loadingIndicatorElem.parentNode.display = "none";

		// Clear the view screen
		var url = "nopreview.html";
		if( sessionManager.viewerElem.src.indexOf(url) < 0 )
			sessionManager.viewerElem.src = url;
	}

	if( !!e )
	{
		e.preventDefault();
		return true;
	}
};

SessionManager.prototype.addBoxEntry = function(options, entriesContainerElem)
{
	var entryContainerElem = document.createElement("div");
	entryContainerElem.className = "entryContainer";
	entryContainerElem.addEventListener("selectstart", function(e)
	{
		e.preventDefault();
		return true;
	}, true);

	entryContainerElem.sessionManager = this;

	entryContainerElem.addEventListener("click", function(e)
	{
		var sessionManager = this.sessionManager;

		if( !this.classList.contains("selectedEntryContainer") )
			sessionManager.entryClick.call(this, e);
		else
		{
			// Remove the selected entry fx
			this.classList.remove("selectedEntryContainer");

			// Remove the selected shortcut fx
			var oldShortcutContainerElem = sessionManager.overviewElem.querySelector(".shortcutContainer[objectId='" + this.getAttribute("objectId") + "']");
			if( !!oldShortcutContainerElem )
				sessionManager.shortcutMouseOut.call(oldShortcutContainerElem);

			if( this.getAttribute("itemId") )
			{
				// Remove the item panel stuff
				var bestItemImageElem = document.querySelector(".bestItemImage");
				bestItemImageElem.style.display = "none";
				var panelElem = bestItemImageElem.parentNode.parentNode.parentNode.parentNode;
				var noEntryElem = bestItemImageElem.parentNode.parentNode.parentNode.querySelector(".noEntry");
				noEntryElem.style.display = "block";

				// Remove the model panel stuff
				var bestModelImageElem = document.querySelector(".bestModelImage");
				bestModelImageElem.style.display = "none";
				var panelElem = bestModelImageElem.parentNode.parentNode.parentNode.parentNode;
				var noEntryElem = bestModelImageElem.parentNode.parentNode.parentNode.querySelector(".noEntry");
				noEntryElem.style.display = "block";

				// Clear the view screen
				var url = "nopreview.html";
				sessionManager.viewerElem.src = url;
			}

			//if( oldSelectedEntryContainerElem.getAttribute("objectId") !== shortcutContainerElem.getAttribute("objectId") )
			//	oldSelectedEntryContainerElem = undefined;
		}
	}.bind(entryContainerElem), true);

	var emptyListElem = entriesContainerElem.parentNode.querySelector(".noEntries");
	emptyListElem.style.display = "none";

	if( !!options.objectId )
		entryContainerElem.setAttribute("objectId", options.objectId);

	if( !!options.itemId )
		entryContainerElem.setAttribute("itemId", options.itemId);

	if( !!options.modelId)
		entryContainerElem.setAttribute("modelId", options.modelId);

	var entryElem = document.createElement("div");
	entryElem.className = "entry";
	entryElem.innerHTML = options.text;

	entryContainerElem.appendChild(entryElem);
	entriesContainerElem.appendChild(entryContainerElem);
};

SessionManager.prototype.shortcutMouseOver = function(e)
{
	var sessionManager = window.sessionManager;

	var shortcutContainerElem = (!!e && !!e.target) ? e.target.parentNode : this;
	var objectSlateElem = shortcutContainerElem.querySelector(".objectSlate");

	if( shortcutContainerElem.style.width === "100%" )
		return;

	var shortcutElem = shortcutContainerElem.querySelector(".shortcut");
	shortcutElem.style.transform = "scale(3.0)";
	shortcutElem.oldZ = shortcutElem.style.zIndex;
	shortcutContainerElem.style.zIndex = 50;

	//var shortcutContainerElem = e.target.parentNode;
	//var shortcutElem = shortcutContainerElem.querySelector(".shortcut");
	//if( shortcutElem.classList.contains("prop") )
	//	return;

	var objectSlateElem = shortcutContainerElem.querySelector(".objectSlate");

	if( objectSlateElem.style.width !== "100%" )
	{
		objectSlateElem.style.width = "100%";
		//objectSlateElem.style.transform = "scale(1.0)";
		//objectSlateElem.style.height = "50px";
		//shortcutElem.style.zIndex = 100;

		var itemId = shortcutContainerElem.getAttribute("itemId");
		var modelId = shortcutContainerElem.getAttribute("modelId");

		var shortcutTitle = "Untitled Shortcut";
		var item = (!!sessionManager.metaverse.library.items[itemId]) ? sessionManager.metaverse.library.items[itemId].current : undefined;
		if( !!item )
			shortcutTitle = item.title;
		else
		{
			var model = (!!sessionManager.metaverse.library.models[modelId]) ? sessionManager.metaverse.library.models[modelId].current : undefined;
			if( !!model )
				shortcutTitle = model.title;
		}

		var shortcutTitleElem = shortcutContainerElem.querySelector(".shortcutTitle");
		shortcutTitleElem.innerHTML = shortcutTitle;

		shortcutContainerElem.style.zIndex = 100;
		objectSlateElem.style.opacity = 1.0;
	}
	//else
	//{
	//	sessionManager.shortcutMouseOut.call(shortcutContainerElem);
	//}
};

var g_testerSwitch = false;
SessionManager.prototype.onEntryChanged = function(mode, val, fields, isNewEntry)
{
	if( mode === "Item" )
	{
		if( isNewEntry )
		{
			// Check if it's used in the entries list somewhere...
			var entriesContainerElem = document.querySelector(".shortcutsContainer");
			var usedEntryContainerElems = entriesContainerElem.querySelectorAll(".entryContainer[itemId='" + val.info.id + "']");

			var usedEntryContainerElem;
			var usedEntryElem;
			for( var i = 0; i < usedEntryContainerElems.length; i++ )
			{
				usedEntryContainerElem = usedEntryContainerElems[i];
				usedEntryElem = usedEntryContainerElem.querySelector(".entry");

				usedEntryElem.innerHTML = val.title;
			}
		}
	}
	else if( mode === "Model" )
	{
		if( isNewEntry )
		{
			// Check if it's used in the entries list somewhere...
			var entriesContainerElem = document.querySelector(".propsContainer");
			var usedEntryContainerElems = entriesContainerElem.querySelectorAll(".entryContainer[modelId='" + val.info.id + "']");

			var usedEntryContainerElem;
			var usedEntryElem;
			for( var i = 0; i < usedEntryContainerElems.length; i++ )
			{
				usedEntryContainerElem = usedEntryContainerElems[i];
				usedEntryElem = usedEntryContainerElem.querySelector(".entry");

				usedEntryElem.innerHTML = val.title;
			}
		}
	}
	else if( mode === "Object" )
	{
		// check if we need to add this to the Shortcuts or Props boxes.
		if( isNewEntry )
		{
			// add to the shortcuts boxes
			var entriesContainerElem;
			if( val.item !== "" )
			{
				entriesContainerElem = document.querySelector(".shortcutsContainer");
				this.addBoxEntry({"text": val.info.id, "objectId": val.info.id, "itemId": val.item, "modelId": val.model}, entriesContainerElem);
			}
			else if( val.model !== "" )
			{
				entriesContainerElem = document.querySelector(".propsContainer");
				this.addBoxEntry({"text": val.info.id, "objectId": val.info.id, "itemId": val.item, "modelId": val.model}, entriesContainerElem);
			}
		}

		if( !!fields.origin )
		{
			var shortcutContainerElem;
			if( isNewEntry )
			{
				var containerOuter = document.createElement("div");
				containerOuter.style.position = "absolute";

				shortcutContainerElem = document.createElement("div");
				shortcutContainerElem.className = "shortcutContainer";
				shortcutContainerElem.setAttribute("objectId", val.info.id);
				shortcutContainerElem.setAttribute("itemId", val.item);
				shortcutContainerElem.setAttribute("modelId", val.model);
				shortcutContainerElem.style.position = "relative";

				//var containerInner = document.createElement("div");
				//containerInner.style.position = "absolute";

				var shortcutElem = document.createElement("div");

				var dynamicWidth = this.overviewWidth * 0.007;
				var dynamicHeight = this.overviewWidth * 0.007;
				var dynamicBorderWidth = this.overviewWidth * 0.001;
				shortcutElem.style.width = parseInt(Math.ceil(dynamicWidth)) + "px";
				shortcutElem.style.height = parseInt(Math.ceil(dynamicHeight)) + "px";
				shortcutElem.style.borderWidth = parseInt(Math.ceil(dynamicBorderWidth)) + "px";

				if( val.item !== "" )
					shortcutElem.className = "shortcut";
				else
				{
					shortcutElem.className = "shortcut prop";
					//shortcutContainerElem.style.pointerEvents = "none";
				}

				shortcutElem.addEventListener("mouseover", this.shortcutMouseOver.bind(this));

				shortcutElem.addEventListener("mouseout", this.shortcutMouseOut.bind(this));

				shortcutElem.addEventListener("click", function(e)
				{
					var sessionManager = window.sessionManager;
					var shortcutContainerElem = e.target.parentNode;
					
					// only if we are not ALREADY the selected entry
					var oldSelectedEntryContainerElem = document.querySelector(".selectedEntryContainer");//[objectId='" + oldSelectedElem.getAttribute("objectId") + "']");
					if( !!oldSelectedEntryContainerElem )
					{
						// Remove the old selected entry fx
						oldSelectedEntryContainerElem.classList.remove("selectedEntryContainer");

						// Remove the old selected shortcut fx
						var oldShortcutContainerElem = sessionManager.overviewElem.querySelector(".shortcutContainer[objectId='" + oldSelectedEntryContainerElem.getAttribute("objectId") + "']");
						if( !!oldShortcutContainerElem )
							sessionManager.shortcutMouseOut.call(oldShortcutContainerElem);

						// Remove the item panel stuff
						var bestItemImageElem = document.querySelector(".bestItemImage");
						bestItemImageElem.style.display = "none";
						var panelElem = bestItemImageElem.parentNode.parentNode.parentNode.parentNode;
						var noEntryElem = bestItemImageElem.parentNode.parentNode.parentNode.querySelector(".noEntry");
						noEntryElem.style.display = "block";

						// Remove the model panel stuff
						var bestModelImageElem = document.querySelector(".bestModelImage");
						bestModelImageElem.style.display = "none";
						var panelElem = bestModelImageElem.parentNode.parentNode.parentNode.parentNode;
						var noEntryElem = bestModelImageElem.parentNode.parentNode.parentNode.querySelector(".noEntry");
						noEntryElem.style.display = "block";

						// Clear the view screen
						var url = "nopreview.html";
						sessionManager.viewerElem.src = url;

						if( oldSelectedEntryContainerElem.getAttribute("objectId") !== shortcutContainerElem.getAttribute("objectId") )
							oldSelectedEntryContainerElem = undefined;
					}

					if( !!!oldSelectedEntryContainerElem )
					{
						// Switch to the right tab
						if( shortcutContainerElem.getAttribute("itemId") )
							sessionManager.switchToTab.call(document.querySelector("#shortcutsTab"), document.querySelector(".shortcutsContainer").parentNode);
						else
							sessionManager.switchToTab.call(document.querySelector("#propsTab"), document.querySelector(".propsContainer").parentNode);

						// Select the new entry
						var entryContainerElem = document.querySelector(".entryContainer[objectId='" + shortcutContainerElem.getAttribute("objectId") + "']");
						sessionManager.entryClick.call(entryContainerElem);

						// now auto-scroll the list to this element
						var entryTop = entryContainerElem.offsetTop;
						entryContainerElem.parentNode.scrollTop = entryTop;
					}


					//sessionManager.shortcutMouseOut.call(oldShortcutContainerElem);

					//this.classList.add("selectedEntryContainer");

					//var shortcutContainerElem = this.sessionManager.overviewElem.querySelector("div[objectId='" + this.getAttribute("objectId") + "']");
					//sessionManager.shortcutMouseOver.call(shortcutContainerElem);
				}.bind(this), true);

				// SLATE
				var objectSlateContainerElem = document.createElement("div");
				objectSlateContainerElem.className = "objectSlateContainer";

				var objectSlateElem = document.createElement("div");
				objectSlateElem.className = "objectSlate";
				objectSlateElem.style.top = -parseInt(Math.ceil(dynamicHeight/2)) + "px";

				//var shortcutTitleContainerElem = document.createElement("div");
				//shortcutTitleContainerElem.className = "shortcutTitleContainer";

				var shortcutTitleElem = document.createElement("div");
				if( val.item !== "" || val.model !== "" )
					shortcutTitleElem.className = "shortcutTitle";

				if( val.item === "" )
					shortcutTitleElem.classList.add("propTitle");

				var dynamicShortcutTitleFontSize = parseInt(Math.ceil(this.overviewWidth * 0.01));
				shortcutTitleElem.style.fontSize = dynamicShortcutTitleFontSize + "px";
				shortcutTitleElem.style.padding = parseInt(Math.ceil(this.overviewWidth * 0.003));
				shortcutTitleElem.style.paddingLeft = parseInt(Math.ceil(this.overviewWidth * 0.025));
				shortcutTitleElem.style.paddingRight = parseInt(Math.ceil(this.overviewWidth * 0.01));
				shortcutTitleElem.innerHTML = "";

				objectSlateElem.appendChild(shortcutTitleElem);
				objectSlateContainerElem.appendChild(objectSlateElem);


				
				shortcutContainerElem.appendChild(objectSlateContainerElem);
				shortcutContainerElem.appendChild(shortcutElem);
				containerOuter.appendChild(shortcutContainerElem);
/*
				shortcutContainerElem.appendChild(objectSlateContainerElem);
				shortcutContainerElem.appendChild(shortcutElem);
				*/
			}
			else
			{
				shortcutContainerElem = this.overviewElem.querySelector(".shortcutContainer[objectId='" + val.info.id + "']");//.createElement("div");
				//shortcutContainerElem = shortcutElem.parentNode;
			}

			var splitOrigin = fields.origin.trim().split(" ");
			var origin = {
				"x": parseFloat(splitOrigin[0]),
				"y": parseFloat(splitOrigin[1]),
				"z": parseFloat(splitOrigin[2])
			};
			
			var position = this.adjustOrigin(origin);
			var newTop = parseInt(Math.round(position.y)) + "px";
			var newLeft = parseInt(Math.round(position.x)) + "px";
			shortcutContainerElem.style.top = newTop;
			shortcutContainerElem.style.left = newLeft;

			if( isNewEntry )
				this.overviewElem.insertBefore(shortcutContainerElem.parentNode, this.overviewElem.firstChild);
		}
	}
};

SessionManager.prototype.onInstanceChanged = function(instanceId)
{
	console.log("onUserAdded: " + instanceId);
};

SessionManager.prototype.onInstanceUserAdded = function(user, sessionId)
{
	// create a miniUserSlate for them
	//var miniUserSlate = document.createElement("div");
	//playerName
	//console.log("onUserAdded: " + JSON.stringify(arguments));
	var userId = user.id;

	if( !!!this.playerMarkerElems[userId] )
	{
		// create a player marker for them on the minimap
		var playerMarkerContainerElem = document.createElement("div");
		playerMarkerContainerElem.style.cssText = "position: absolute;";

		var playerMarkerElem = document.createElement("div");
		playerMarkerElem.className = "playerMarker";
		playerMarkerContainerElem.appendChild(playerMarkerElem);

		var userLookConeContainerContainerElem = document.createElement("div");
		userLookConeContainerContainerElem.style.cssText = "position: absolute;";

		var userLookConeContainerElem = document.createElement("div");
		userLookConeContainerElem.className = "userLookConeContainer";

		var userLookConeElem = document.createElement("div");
		userLookConeElem.className = "userLookCone";
		userLookConeContainerElem.appendChild(userLookConeElem);
		userLookConeContainerContainerElem.appendChild(userLookConeContainerElem);

		var userOriginDotContainerElem = document.createElement("div");
		userOriginDotContainerElem.style.cssText = "position: absolute;";

		var userOriginDotElem = document.createElement("div");
		userOriginDotElem.className = "userOriginDot";
		userOriginDotContainerElem.appendChild(userOriginDotElem);

		var userNameContainerElem = document.createElement("div");
		userNameContainerElem.style.cssText = "position: absolute;";

		var userNameElem = document.createElement("div");
		userNameElem.className = "userName";
		userNameElem.innerHTML = "SM Sith Lord";
		userNameContainerElem.appendChild(userNameElem);

		//playerMarkerElem.appendChild(playerMarkerContainerElem);
		playerMarkerElem.appendChild(userLookConeContainerContainerElem);
		playerMarkerElem.appendChild(userOriginDotContainerElem);
		playerMarkerElem.appendChild(userNameContainerElem);

		this.overviewElem.insertBefore(playerMarkerContainerElem, this.overviewElem.firstChild);
		this.playerMarkerElems[userId] = playerMarkerElem;

		/*
			/<div style="position: absolute;">
				/<div class="playerMarker">
					/<div style="position: absolute;">
						/<div class="userLookConeContainer">
							/<div class="userLookCone"></div>
						/</div>
					/</div>
					/<div style="position: absolute;">
						/<div class="userOriginDot"></div>
					/</div>
					<div style="position: absolute;">
						<div class="playerName" style="">
							SM Sith Lord
						</div>
					</div>
				</div>
			</div>
		*/
	}
};

SessionManager.prototype.onUserSessionUpdated = function(userId, userSession, oldUserSession)
{
	/*
	var usersElem = document.querySelector("#users");
	var userElem = usersElem.querySelector("option[value='" + userId + "']");
	if( !!!userElem )
	{
		var userElem = document.createElement("option");
		userElem.value = userId;
		userElem.text = userSession.displayName;
		usersElem.appendChild(userElem);
	}
	*/

	var splitOrigin = userSession.transform.body.origin.trim().split(" ");
	var origin = {
		"x": parseFloat(splitOrigin[0]),
		"y": parseFloat(splitOrigin[1]),
		"z": parseFloat(splitOrigin[2])
	};

	var position = this.adjustOrigin(origin);
	var newTop = parseInt(Math.round(position.y)) + "px";
	var newLeft = parseInt(Math.round(position.x)) + "px";
	var playerMarkerElem = this.playerMarkerElems[userId];
	
	if( !!playerMarkerElem )
	{
		if( playerMarkerElem.style.top !== newTop ||  playerMarkerElem.style.left !== newLeft )
		{
			var payload = {
				"top": parseInt(playerMarkerElem.style.top) + 7 + "px",
				"left": parseInt(playerMarkerElem.style.left) + 7 + "px",
				"playerMarkerElem": playerMarkerElem
			};

			setTimeout(function()
			{
				var containerElem = document.createElement("div");
				containerElem.style.position = "absolute";

				var breadCrumbContainerElem = document.createElement("div");
				breadCrumbContainerElem.className = "breadCrumbContainer";
				breadCrumbContainerElem.style.top = payload.top;
				breadCrumbContainerElem.style.left = payload.left;

				var breadCrumbElem = document.createElement("div");
				breadCrumbElem.className = "breadCrumb";

				containerElem.appendChild(breadCrumbContainerElem);
				breadCrumbContainerElem.appendChild(breadCrumbElem);
				this.playerMarkerElem.parentNode.appendChild(containerElem);

				breadCrumbElem.offsetWidth;
				breadCrumbElem.style.opacity = 0;
				breadCrumbElem.style.transform = "scale(0.01)";

				setTimeout(function()
				{
					this.parentNode.removeChild(this);
				}.bind(containerElem), 30000);
			}.bind(payload), 300);

			playerMarkerElem.style.top = newTop;
			playerMarkerElem.style.left = newLeft;
		}

		// angles
		var splitAngles = userSession.transform.body.angles.trim().split(" ");
		var angles = {
			"x": parseInt(Math.round(parseFloat(splitAngles[0]))),
			"y": -parseInt(Math.round(parseFloat(splitAngles[1]))),
			"z": parseInt(Math.round(parseFloat(splitAngles[2])))
		};

		var viewConeContainerElem = playerMarkerElem.querySelector(".userLookConeContainer");
		viewConeContainerElem.style.transform = "rotate(" + angles.y + "deg)";
	}

	// web url
	/* DISABLED until a player following system is added to the web client!!
	if( !!!oldUserSession || oldUserSession.web.url !== userSession.web.url )
	{
		if( userSession.web.url.toLowerCase().indexOf("http://") === 0 || userSession.web.url.toLowerCase().indexOf("https://") === 0 )
		{
			var url = userSession.web.url;
			var searchString = "smarcade.net/dlcv2/view_youtube.php";
			var foundIndex = url.indexOf(searchString);
			if( foundIndex >= 0 )
				url = "http://www.anarchyarcade.com/youtube_player.php" + url.substring(foundIndex + searchString.length);

			//console.log(g_viewerWidth + " , " + g_viewerHeight);
			if( url.indexOf("youtube_player.php") >= 0 )
			{
				url += "&width=" + this.viewerWidth + "&height=" + this.viewerHeight;
				//url = "&width=1920&height=1080";
			}
			this.viewerElem.src = url;
		}
		else
			this.viewerElem.src = "nopreview.html";//userSession.web.url;
	}
	*/

	// g_playerCursorElem

	/* DISABLED until a player following system is added to the web client!!
	if( !!!oldUserSession || oldUserSession.mouse.x !== userSession.mouse.x )
	{
		var mousePosition = {
			"x": this.viewerWidth * userSession.mouse.x,
			"y":this.viewerHeight * userSession.mouse.y,
		};

		this.playerCursorElem.style.top = parseInt(Math.round(mousePosition.y)) + "px";
		this.playerCursorElem.style.left = parseInt(Math.round(mousePosition.x)) + "px";
	}
	*/
};
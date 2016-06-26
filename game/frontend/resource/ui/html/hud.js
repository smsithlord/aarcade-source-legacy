function ArcadeHud()
{
	this.platformId = "-KJvcne3IKMZQTaG7lPo";
	this.selectedWebTab = null;
	this.pinHudButtonElem;
	this.returnHudButtonElem;
	this.closeContentButtonElem;
	this.clickThruElem;
	this.cursorElem;
	this.cursorPreviewImageElem;
	this.cursorImageElem;
	this.helpElem;
	this.startupLoadingMessagesContainer;
	this.hudLoadingMessagesContainer;
	this.hudLoadingMessages = {};
	this.DOMReady = false;
	this.onDOMReady().then(function()
	{
		this.DOMReady = true;

		this.startupLoadingMessagesContainer = document.body.querySelector("#startupLoadingMessagesContainer");	// usually undefined

		this.pinHudButtonElem = document.body.querySelector("#pinHudButton");
		this.returnHudButtonElem = document.body.querySelector("#returnHudButton");
		this.closeContentButtonElem = document.body.querySelector(".hudContentHeaderCell:nth-of-type(3) .hudContentHeaderButton");
		
		aaapi.system.requestActivateInputMode();

		this.helpElem = document.createElement("div");
		this.helpElem.className = "helpContainer";

		this.hudLoadingMessagesContainer = document.createElement("div");
		this.hudLoadingMessagesContainer.className = "hudLoadingMessagesContainer";
		this.helpElem.appendChild(this.hudLoadingMessagesContainer);
		document.body.appendChild(this.helpElem);

/*
		document.body.addEventListener("dblclick", function(e)
		{
			console.log("dblclicked: " + (e.target === document.body));
		}, true);
*/

		document.body.addEventListener("mousedown", function(e)
		{
//			console.log("mouse down: " + (e.target === document.body));

//			if(e.target === document.body)
//			{
				var buttonCode = 0;
				if( e.button === 1 )
					buttonCode = 1;
				else if( e.button === 2 )
					buttonCode = 2;
				aaapi.system.hudMouseDown(buttonCode, (e.target === document.body));
//			}
		}, true);

		document.body.addEventListener("mouseup", function(e)
		{
//			console.log("mouse up: " + (e.target === document.body));

//			if(e.target === document.body)
//			{
				var buttonCode = 0;
				if( e.button === 1 )
					buttonCode = 1;
				else if( e.button === 2 )
					buttonCode = 2;
				aaapi.system.hudMouseUp(buttonCode, (e.target === document.body));
//			}
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

ArcadeHud.prototype.expandAddressMenu = function()
{
	var elem = document.body.querySelector(".hudHeaderContainer");

	if( elem.style.top != "0px" )
		elem.style.top = "0px";
	else
		elem.style.top = "-65px";
};

ArcadeHud.prototype.onActivateInputMode = function(isFullscreen, isHudPinned, isMapLoaded)
{
	isFullscreen = parseInt(isFullscreen);
	isHudPinned = parseInt(isHudPinned);
	isMapLoaded = parseInt(isMapLoaded);

	if( isMapLoaded )
	{
		if( isHudPinned )
		{
			if( !!this.pinHudButtonElem )
				this.pinHudButtonElem.style.display = "none";

			if( !!this.returnHudButtonElem )
				this.returnHudButtonElem.style.display = "inline-block";
		}
		else
		{
			if( !!this.returnHudButtonElem )
				this.returnHudButtonElem.style.display = "none";
			
			if( isMapLoaded )
			{
				if( !!this.pinHudButtonElem )
					this.pinHudButtonElem.style.display = "inline-block";
			}
			else
			{
				if( !!this.pinHudButtonElem )
					this.pinHudButtonElem.style.display = "none";
			}
		}

		//if( !!this.closeContentButtonElem )
		//	this.closeContentButtonElem.style.display = "block";
	}
	else
	{
		if( !!this.pinHudButtonElem )
			this.pinHudButtonElem.style.display = "none";

		if( !!this.returnHudButtonElem )
			this.returnHudButtonElem.style.display = "none";

		//if( !!this.closeContentButtonElem )
		//	this.closeContentButtonElem.style.display = "none";
	}

	if( isFullscreen )
		this.cursorImageElem.style.display = "none";
	else
		this.cursorImageElem.style.display = "block";


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
		"mounts",
		"workfetch",
		"detectmaps",
		"workshoplibrarymodels",
		"workshoplibraryitems",
		"skiplegacyworkshops",
		"mountworkshops"
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

ArcadeHud.prototype.showPopupMenu = function(popupId, x, y, width, height, itemHeight, fontSize, selectedItem, rightAligned)
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
		"items": new Array()
	};

	var argIndex = 9;
	var numArguments = arguments.length;
	var popupItem;
	while( argIndex < arguments.length - 8)
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

		if( count === 1 )
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

	blackout.appendChild(container);
	document.body.insertBefore(blackout, this.cursorElem);

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

var arcadeHud = new ArcadeHud();
function ArcadeHud()
{
	this.cursorElem;
	this.helpElem = null;
	this.DOMReady().then(function()
	{
		this.cursorElem = document.createElement("div");
		this.cursorElem.id = "cursor";

		this.cursorImageElem = document.createElement("img");
		this.cursorImageElem.className = "cursorImage";
		this.cursorImageElem.src = "cursors/hippie_default.png";

		this.cursorElem.style.left = (document.body.offsetWidth / 2) + "px";
		this.cursorElem.style.top = (document.body.offsetHeight / 2) + "px";

		this.cursorElem.appendChild(this.cursorImageElem);
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
	}.bind(this));
}

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

	blackout.addEventListener("click", function(e)
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

	blackout.appendChild(container, this.cursorElem);
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
	this.helpElem.parentNode.removeChild(this.helpElem);
	this.helpElem = null;
};

ArcadeHud.prototype.addHelpMessage = function(text)
{
	var needsAppend;
	if( !this.helpElem )
	{
		this.helpElem = document.createElement("div");
		this.helpElem.className = "helpContainer";
		needsAppend = true;
	}
	else
	{
		// empty out the container
		var firstChild = this.helpElem.firstChild;
		while( firstChild )
		{
			this.helpElem.removeChild(firstChild);
			firstChild = this.helpElem.firstChild;
		}

		needsAppend = false;
	}

	var helpText = document.createElement("div");
	var helpTextNode = document.createTextNode(text);
	helpText.appendChild(helpTextNode);
	this.helpElem.appendChild(helpText);

	if( needsAppend )
		document.body.appendChild(this.helpElem);
};

ArcadeHud.prototype.DOMReady = function()
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
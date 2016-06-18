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
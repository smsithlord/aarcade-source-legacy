function Metaverse(eventHandler)
{
	this.quickJoinAddress = "https://metaverse.firebaseio.com/";
	this.eventHandler = eventHandler;
	this.error;
	this.verbose = false;
	this.users = {};
	this.panos = {};
	this.library = {
		"items": {},
		"models": {},
		"types": {},
		"apps": {},
		"databases": {},
		"platforms": {},
		"maps": {},
		"instances": {},
		"objects": {}
	};

	this.localUser = 
	{
		"id": "annon",
		"username": "annon",
		//"displayName": "Annon",
		"lastUpdate": {}
	};

	this.root = null;
	this.universe = null;
	this.universeTitles = {};
	this.local = {};
	this.rootRef = null;
	this.universeRef = null;
	this.usersRef = null;
	this.libraryRef = null;
	this.localUserRef = null;
	this.connectedRef = null;
	this.instanceObjectsRef = null;
	this.instanceUsersRef = null;
	this.instancePanosRef = null;
	this.status = "Offline";
	//this.universeInfo = {};

	this.isConnectedToInstance = false;

	//this.freshInstanceObjects = {};

	this.pendingInstanceEntries = {};
	//this.pendingInstanceItems = {};
	//this.pendingInstanceObjects = {};
	this.fetching = {};

	this.lastPushTime = 0;
	this.lastRandChars = [];

	this.instanceInfo;

	this.listeners =
	{
		"connect": {},
		"disconnect": {},
		"status": {},
		"reset": {}
	};

	this.objectListeners = {};

	this.defaultSession = {
		"instance": "",
		"displayName": "Human Player",
		"item":
		{
			"id": ""
		},
		"object":
		{
			"id": ""
		},
		"say":
		{
			"text": ""
		},
		"transform":
		{
			"body":
			{
				"origin": "",
				"angles": ""
			},
			"head":
			{
				"origin": "",
				"angles": ""
			}
		},
		"mouse":
		{
			"x": 0,
			"y": 0
		},
		"web":
		{
			"url": ""
		},
		"avatar":
		{
			"url": ""
		}
	};

	this.defaultInfo = {
		"created":
		{
			"default": Firebase.ServerValue.TIMESTAMP,
			"types": "timestamp",
			"format": "/^[-+]?\\d+$/i",
			"formatDescription": "Created must be a timestamp."
		},
		"id":
		{
			"default": "",
			"types": "autokey",
			"format": "/.+$/i",
			"formatDescription": "ID must be an auto-generated key."
		},
		"owner":
		{
			"default": "",
			"types": "autokey",
			"format": "/.+$/i",
			"formatDescription": "Owner must be an auto-generated key."
		},
		"removed":
		{
			"default": "0",
			"types": "timestamp",
			"format": "/^[-+]?\\d+$/i",
			"formatDescription": "Removed must be a timestamp."
		},
		"remover":
		{
			"default": "",
			"types": "autokey",
			"format": "/.*$/i",
			"formatDescription": "Remover must be an auto-generated key."
		}		
	};

	this.defaultUniverse = {
		"info":
		{
			"title":
			{
				"default": "",
				"types": "string",
				"format": "/^.{2,1024}$/i",
				"formatDescription": "Title must be between 2 and 1024 characters."
			}
		}
	};

	this.defaultUser = {
		"username":
		{
			"default": "",
			"types": "string",
			"format": "/^.{3,1024}$/i",
			"formatDescription": "Username must be between 3 and 1024 characters."
		},/*
		"displayName":
		{
			"default": "",
			"types": "string",
			"format": "/^.{3,1024}$/i",
			"formatDescription": "Display name must be between 3 and 1024 characters."
		},*/
		"passcode":
		{
			"default": "",
			"types": "string",
			"format": "/^.{5,1024}$/i",
			"formatDescription": "Passcode must be between 5 and 1024 characters."
		}
	};

	this.defaultDatabase = {
		"info": true,
		"title":
		{
			"label": "Title",
			"default": "",
			"types": "string",
			"format": "/^.{2,1024}$/i",
			"formatDescription": "Title must be between 2 and 1024 characters."
		},
		"singleQueryUri":
		{
			"label": "Single Query Handler",
			"default": "",
			"types": "uri",
			"format": "/(((http|https):\\/\\/|(www\\.|www\\d\\.))([^\\-][a-zA-Z0-9\\-]+)?(\\.\\w+)(\\/\\w+){0,}(\\.\\w+){0,}(\\?\\w+\\=\\w+){0,}(\\&\\w+\\=\\w+)?)/i",
			"formatDescription": "Single Query must be a valid URI to a PHP handler."
		}/*,
		"types":
		{
			"label": "Types",
			"id":
			{
				"label": "ID",
				"default": "",
				"types": "autokey",
				"format": "/.+$/i",
				"formatDescription": "Types must be an auto-generated key."
			}
		}*/,
		"typeAliases":
		{
			"label": "Type Aliases",
			"id":
			{
				"label": "ID",
				"default": "",
				"types": "autokey",
				"format": "/.+$/i",
				"formatDescription": "ID must be an auto-generated key."
			},
			"aliases":
			{
				"label": "Aliases",
				"default": "",
				"types": "string",
				"format": "/.*$/i",//"/^.{2,1024}$/i",
				"formatDescription": "Aliases must be between 0 and 1024 characters."
			}
		}
	};

	this.defaultPlatform = {
		"info": true,
		"title":
		{
			"label": "Title",
			"default": "",
			"types": "string",
			"format": "/^.{2,1024}$/i",
			"formatDescription": "Title must be between 2 and 1024 characters."
		},
		"reference":
		{
			"label": "Reference",
			"default": "",
			"types": "uri",
			"format": "/((((http|https):\\/\\/|(www\\.|www\\d\\.))([^\\-][a-zA-Z0-9\\-]+)?(\\.\\w+)(\\/\\w+){0,}(\\.\\w+){0,}(\\?\\w+\\=\\w+){0,}(\\&\\w+\\=\\w+)?)|(^$))/i",
			"formatDescription": "Reference must be a valid URI."
		},
		"download":
		{
			"label": "Download",
			"default": "",
			"types": "uri",
			"format": "/((((http|https):\\/\\/|(www\\.|www\\d\\.))([^\\-][a-zA-Z0-9\\-]+)?(\\.\\w+)(\\/\\w+){0,}(\\.\\w+){0,}(\\?\\w+\\=\\w+){0,}(\\&\\w+\\=\\w+)?)|(^$))/i",
			"formatDescription": "Download must be a valid URI."
		},
		"modelCustomFields":
		{
			"label": "Model Custom Fields",
			"default": "",
			"types": "string",
			"format": "/.*$/i",
			"formatDescription": "Model Custom Fields must be comma separated fieldIDs longer than 2 characters each and under 1024 characters total."
		},
		"mapCustomFields":
		{
			"label": "Map Custom Fields",
			"default": "",
			"types": "string",
			"format": "/.*$/i",
			"formatDescription": "Map Custom Fields must be comma separated fieldIDs longer than 2 characters each and under 1024 characters total."
		},
		"instanceCustomFields":
		{
			"label": "Instance Custom Fields",
			"default": "",
			"types": "string",
			"format": "/.*$/i",
			"formatDescription": "Instance Custom Fields must be comma separated fieldIDs longer than 2 characters each and under 1024 characters total."
		}
	};

	this.defaultType = {
		"info": true,
		"fileformat":
		{
			"label": "File Format",
			"default": "/.+$/i",
			"types": "regex",
			"format": "/.*$/i",
			"formatDescription": "File Format must be a regular expression."
		},
		"titleformat":
		{
			"label": "Title Format",
			"default": "/(?=[^\\/]*$).+$/i",
			"types": "regex",
			"format": "/.*$/i",
			"formatDescription": "Title Format must be a regular expression."
		},
		"title":
		{
			"label": "Title",
			"default": "",
			"types": "string",
			"format": "/^.*$/i",
			//"format": "/^.{2,1024}$/i",
			"formatDescription": "Title must be between 2 and 1024 characters."
		},
		"priority":
		{
			"label": "Priority",
			"default": 1,
			"types": "integer",
			"format": "/.*$/i",
			"formatDescription": "Priority must be an integer between 0 and 1024."
		}
	};

	this.defaultApp = {
		"info": true,
		"title":
		{
			"label": "Title",
			"default": "",
			"types": "string",
			//"format": "/^.{2,1024}$/i",
			"format": "/^.*$/i",
			"formatDescription": "Title must be between 2 and 1024 characters."
		},
		"file":
		{
			"label": "Executable",
			"default": "",
			"types": "string",
			//"format": "/^.{2,1024}$/i",
			"format": "/^.*$/i",
			"formatDescription": "Executable must be a valid local file."
		},
		"commandformat":
		{
			"label": "Command Format",
			"default": "",
			"types": "string",
			"format": "/^.*$/i",
			"formatDescription": "Command Format must be a valid metaverse command format."
		},
		"type":
		{
			"label": "Type",
			"default": "",
			"types": "autokey",
			"format": "/^.*$/i",
			"formatDescription": "Type must be an auto-generated key."
		},
		"download":
		{
			"label": "Download",
			"default": "",
			"types": "uri",
			"format": "/((((http|https):\\/\\/|(www\\.|www\\d\\.))([^\\-][a-zA-Z0-9\\-]+)?(\\.\\w+)(\\/\\w+){0,}(\\.\\w+){0,}(\\?\\w+\\=\\w+){0,}(\\&\\w+\\=\\w+)?)|(^$))/i",
			"formatDescription": "Download must be a valid URI."
		},
		"reference":
		{
			"label": "Reference",
			"default": "",
			"types": "uri",
			"format": "/((((http|https):\\/\\/|(www\\.|www\\d\\.))([^\\-][a-zA-Z0-9\\-]+)?(\\.\\w+)(\\/\\w+){0,}(\\.\\w+){0,}(\\?\\w+\\=\\w+){0,}(\\&\\w+\\=\\w+)?)|(^$))/i",
			"formatDescription": "Reference must be a valid URI."
		},
		"screen":
		{
			"label": "Screen",
			"default": "",
			"types": "uri|string",
			"format": "/^.{0,1024}$/i",
			"formatDescription": "Screen must be between 0 and 1024 characters."
		},
		"description":
		{
			"label": "Description",
			"default": "",
			"types": "string",
			"format": "/^.*$/i",
			//"format": "/^.{0,1024}$/i",
			"formatDescription": "Description must be between 0 and 1024 characters."
		},
		"filepaths":
		{
			"label": "File Paths",
			"id":
			{
				"label": "ID",
				"default": "",
				"types": "autokey",
				"format": "/.+$/i",
				"formatDescription": "ID must be an auto-generated key."
			},
			"path":
			{
				"label": "Path",
				"default": "",
				"types": "string",
				"format": "/^.*$/i",
				//"format": "/^.{2,1024}$/i",
				"formatDescription": "Path must be a valid directory."
			},
			"extensions":
			{
				"label": "Extensions",
				"default": "",
				"types": "string",
				"format": "/^.*$/i",
				"formatDescription": "Extensions must be comma separated strings under 1024 characters total."
			}
		}
	};

	this.defaultItem = {
		"info": true,
		"title":
		{
			"label": "Title",
			"default": "",
			"types": "string",
			"format": "/^.*$/i",
			//"format": "/^.{2,1024}$/i",
			"formatDescription": "Title must be between 2 and 1024 characters."
		},
		"description":
		{
			"label": "Description",
			"default": "",
			"types": "string",
			"format": "/^.*/i",
			/*"format": "/^.*$/i",*/
			/*"format": "/^.{0,1024}$/i",*/
			"formatDescription": "Description must be between 0 and 1024 characters."
		},
		"file":
		{
			"label": "File",
			"default": "",
			"types": "uri|string",
			"format": "/^.*$/i",
			//"format": "/^.{1,1024}$/i",
			"formatDescription": "File must be between 3 and 1024 characters."
		},
		"type":
		{
			"label": "Type",
			"default": "",
			"types": "autokey",
			"format": "/^.*$/i",
			"formatDescription": "Type must be an auto-generated key."
		},
		"app":
		{
			"label": "App",
			"default": "",
			"types": "autokey",
			"format": "/^.*$/i",
			"formatDescription": "App must be an auto-generated key."
		},
		"reference":
		{
			"label": "Reference",
			"default": "",
			"types": "uri",
			"format": "/((((http|https):\\/\\/|(www\\.|www\\d\\.))([^\\-][a-zA-Z0-9\\-]+)?(\\.\\w+)(\\/\\w+){0,}(\\.\\w+){0,}(\\?\\w+\\=\\w+){0,}(\\&\\w+\\=\\w+)?)|(^$))/i",
			"formatDescription": "Reference must be a valid URI."
		},
		"preview":
		{
			"label": "Preview",
			"default": "",
			"types": "uri|string",
			"format": "/^.{0,1024}$/i",
			"formatDescription": "Preview must be between 0 and 1024 characters."
		},
		"download":
		{
			"label": "Download",
			"default": "",
			"types": "uri",
			"format": "/((((http|https):\\/\\/|(www\\.|www\\d\\.))([^\\-][a-zA-Z0-9\\-]+)?(\\.\\w+)(\\/\\w+){0,}(\\.\\w+){0,}(\\?\\w+\\=\\w+){0,}(\\&\\w+\\=\\w+)?)|(^$))/i",
			"formatDescription": "Download must be a valid URI."
		},
		"stream":
		{
			"label": "Stream",
			"default": "",
			"types": "uri",
			"format": "/((((http|https):\\/\\/|(www\\.|www\\d\\.))([^\\-][a-zA-Z0-9\\-]+)?(\\.\\w+)(\\/\\w+){0,}(\\.\\w+){0,}(\\?\\w+\\=\\w+){0,}(\\&\\w+\\=\\w+)?)|(^$))/i",
			"formatDescription": "Stream must be a valid URI."
		},
		"screen":
		{
			"label": "Screen",
			"default": "",
			"types": "uri|string",
			"format": "/^.{0,1024}$/i",
			"formatDescription": "Screen must be between 0 and 1024 characters."
		},
		"marquee":
		{
			"label": "Marquee",
			"default": "",
			"types": "uri|string",
			"format": "/^.{0,1024}$/i",
			"formatDescription": "Description must be between 0 and 1024 characters."
		}/*,
		"model":
		{
			"label": "Model",
			"default": "",
			"types": "autokey",
			"format": "/.+$/i",
			"formatDescription": "Model must be an auto-generated key."
		}*/
	};

	this.defaultMap = {
		"info": true,
		"title":
		{
			"label": "Title",
			"default": "",
			"types": "string",
			"format": "/^.{2,1024}$/i",
			"formatDescription": "Title must be between 2 and 1024 characters."
		},
		"keywords":
		{
			"label": "Keywords",
			"default": "",
			"types": "string",
			"format": "/^.*$/i",
			"formatDescription": "Keywords must be comma separated strings under 1024 characters total."
		},
		"platforms":
		{
			"label": "Platforms",
			"id":
			{
				"label": "ID",
				"default": "",
				"types": "autokey",
				"format": "/.+$/i",
				"formatDescription": "ID must be an auto-generated key."
			},
			"file":
			{
				"label": "File",
				"default": "",
				"types": "string",
				"format": "/^.{2,1024}$/i",
				"formatDescription": "File must be between 0 and 1024 characters."
			},
			"download":
			{
				"label": "Download",
				"default": "",
				"types": "uri",
				"format": "/((((http|https):\\/\\/|(www\\.|www\\d\\.))([^\\-][a-zA-Z0-9\\-]+)?(\\.\\w+)(\\/\\w+){0,}(\\.\\w+){0,}(\\?\\w+\\=\\w+){0,}(\\&\\w+\\=\\w+)?)|(^$))/i",
				"formatDescription": "Download must be a valid URI."
			}
		}
	};

	this.defaultInstance = {
		"info": true,
		"title":
		{
			"label": "Title",
			"default": "",
			"types": "string",
			"format": "/^.{2,1024}$/i",
			"formatDescription": "Title must be between 2 and 1024 characters."
		},
		"password":
		{
			"label": "Password",
			"default": "",
			"types": "string",
			"format": "/^.*$/i",
			"formatDescription": "Password field."
		},
		"map":
		{
			"label": "Map",
			"default": "",
			"types": "autokey",
			"format": "/.+$/i",
			"formatDescription": "Map must be an auto-generated key."
		},
		"platforms":
		{
			"label": "Platforms",
			"id":
			{
				"label": "ID",
				"default": "",
				"types": "autokey",
				"format": "/.+$/i",
				"formatDescription": "ID must be an auto-generated key."
			}
		}
	};

	this.defaultObject = {
		"info": true,
		"instance":
		{
			"label": "Instance",
			"default": "",
			"types": "autokey",
			"format": "/.+$/i",
			"formatDescription": "Instance must be an auto-generated key."
		},
		"item":
		{
			"label": "Item",
			"default": "",
			"types": "autokey",
			"format": "/^.*$/i",
			"formatDescription": "Item must be an auto-generated key."
		},
		"model":
		{
			"label": "Model",
			"default": "",
			"types": "autokey",
			"format": "/^.*$/i",
			"formatDescription": "Model must be an auto-generated key."
		},
		"slave":
		{
			"label": "Slave",
			"default": 0,
			"types": "integer",
			"format": "/(0|1)$/i",
			"formatDescription": "Slave must be 0 or 1."
		},
		"child":
		{
			"label": "Slave",
			"default": 0,
			"types": "integer",
			"format": "/(0|1)$/i",
			"formatDescription": "Slave must be 0 or 1."
		},
		"parentObject":
		{
			"label": "Parent Object",
			"default": "",
			"types": "autokey",
			"format": "/^.*$/i",
			"formatDescription": "ParentObject must be an auto-generated key."
		},
		"scale":
		{
			"label": "Scale",
			"default": 0.0,
			"types": "number",
			"format": "/^-?\d*\.?\d*$/i",
			"formatDescription": "Scale must be a number."
		},
		"origin":
		{
			"label": "Origin",
			"default": "",
			"types": "string",
			"format": "/^.{2,1024}$/i",
			"formatDescription": "Origin must be a space separated string of vector XYZ component numbers."
		},
		"angles":
		{
			"label": "Angles",
			"default": "",
			"types": "string",
			"format": "/^.{2,1024}$/i",
			"formatDescription": "Angles must be a space separated string of vector PYR component numbers."
		}
	};

	this.defaultModel = {
		"info": true,
		"title":
		{
			"label": "Title",
			"default": "",
			"types": "string",
			"format": "/^.{2,1024}$/i",
			"formatDescription": "Title must be between 2 and 1024 characters."
		},
		"keywords":
		{
			"label": "Keywords",
			"default": "",
			"types": "string",
			"format": "/^.*$/i",
			"formatDescription": "Keywords must be comma separated strings longer than 2 characters each and under 1024 characters total."
		},
		"dynamic":
		{
			"label": "Dynamic",
			"default": 0,
			"types": "integer",
			"format": "/(0|1)$/i",
			"formatDescription": "Dynamic must be 0 or 1."
		},
		"screen":
		{
			"label": "Thumbnail",
			"default": "",
			"types": "uri|string",
			"format": "/^.{0,1024}$/i",
			"formatDescription": "Thumbnail must be between 0 and 1024 characters."
		},
		"preview":
		{
			"label": "Preview",
			"default": "",
			"types": "uri|string",
			"format": "/^.{0,1024}$/i",
			"formatDescription": "Preview must be between 0 and 1024 characters."
		},
		"platforms":
		{
			"label": "Platforms",
			"id":
			{
				"label": "ID",
				"default": "",
				"types": "autokey",
				"format": "/.+$/i",
				"formatDescription": "ID must be an auto-generated key."
			},
			"file":
			{
				"label": "File",
				"default": "",
				"types": "string",
				"format": "/^.{2,1024}$/i",
				"formatDescription": "File must be between 0 and 1024 characters."
			}/*,
			"download":
			{
				"label": "Download",
				"default": "",
				"types": "uri",
				"format": "/((((http|https):\\/\\/|(www\\.|www\\d\\.))([^\\-][a-zA-Z0-9\\-]+)?(\\.\\w+)(\\/\\w+){0,}(\\.\\w+){0,}(\\?\\w+\\=\\w+){0,}(\\&\\w+\\=\\w+)?)|(^$))/i",
				"formatDescription": "Download must be a valid URI."
			}*/
		}
	};

	// NOTE: The regex's get encoded as strings to be compatible with JSON.
	this.defaultPlatforms = [
		{
			"title": "AArcade: Source",
			"reference": "http://www.anarchyarcade.com/",
			"download": "http://store.steampowered.com/app/266430/",
			"modelCustomFields": "workshopIds, mountIds, download",
			"mapCustomFields": "workshopIds, mountIds",
			"instanceCustomFields": "workshopIds, mountIds"
		}
	];

	function customHelper(object)
	{
		var pushId = this.generatePushId();

		var dummy = {};
		dummy[pushId] = object;

		return JSON.stringify(dummy);
	}

	this.defaultDatabases = [
		{
			"title": "YouTube.com",
			"singleQueryUri": "http://metaverse.anarchyarcade.com/tubeinfo.php",
			"typeAliases":
			{
				"YouTube":
				{
					"aliases": ""
				}
			}
		}
	];
	this.defaultDatabases = [];	// disabled

	this.defaultTypes = [
		{
			"title": "YouTube",
			"fileformat": "/(http|https):\\/\\/(?:www\\.)?youtu(?:be\\.com\\/watch\\?v=|\\.be\\/)([\\w\\-]+)(&(amp;)?[\\w\\?=]*)?/i",
			"titleformat": "/(?=[^\\/]*$).+$/i",
			"priority": 2
		},
		{
			"title": "Image",
			"fileformat": "/(.jpg|.jpeg|.gif|.png|.tga)$/i",
			"titleformat": "/(?=[^\\/]*$).+$/i",
			"priority": 2
		},
		{
			"title": "DS",
			"fileformat": "/(.nds)$/i",
			"titleformat": "/.+$/i",
			"priority": 1
		},
		{
			"title": "Wii",
			"fileformat": "/(.iso)$/i",
			"titleformat": "/.+$/i",
			"priority": 1
		},
		{
			"title": "PS",
			"fileformat": "/(.iso|.bin|.img|.ccd|.mds|.pbp|.ecm)$/i",
			"titleformat": "/.+$/i",
			"priority": 1
		},
		{
			"title": "NES",
			"fileformat": "/(.nes|.zip)$/i",
			"titleformat": "/.+$/i",
			"priority": 1
		},
		{
			"title": "Genesis",
			"fileformat": "/(.zip|.gen|.smc)$/i",
			"titleformat": "/.+$/i",
			"priority": 1
		},
		{
			"title": "Arcade",
			"fileformat": "/(.zip)$/i",
			"titleformat": "/.+$/i",
			"priority": 1
		},
		{
			"title": "PSP",
			"fileformat": "/(.iso)$/i",
			"titleformat": "/.+$/i",
			"priority": 1
		},
		{
			"title": "N64",
			"fileformat": "/(.n64|.zip)$/i",
			"titleformat": "/.+$/i",
			"priority": 1
		},
		{
			"title": "SNES",
			"fileformat": "/(.smc|.zip)$/i",
			"titleformat": "/.+$/i",
			"priority": 1
		},
		{
			"title": "GB",
			"fileformat": "/(.zip)$/i",
			"titleformat": "/.+$/i",
			"priority": 1
		},
		{
			"title": "GBA",
			"fileformat": "/(.zip)$/i",
			"titleformat": "/.+$/i",
			"priority": 1
		},
		{
			"title": "Pinball",
			"fileformat": "/(.vpt)$/i",
			"titleformat": "/.+$/i",
			"priority": 1
		},
		{
			"title": "Website",
			"fileformat": "/((http|https):\\/\\/|(www\\.|www\\d\\.))([^\\-][a-zA-Z0-9\\-]+)?(\\.\\w+)(\\/\\w+){0,}(\\.\\w+){0,}(\\?\\w+\\=\\w+){0,}(\\&\\w+\\=\\w+)?/i",
			"titleformat": "/(?=[^\\/]*$).+$/i",
			"priority": 1
		},
		{
			"title": "Default",
			"fileformat": "/.+$/i",
			"titleformat": "/.+$/i",
			"priority": 0
		}
	];
	this.defaultTypes = [];	// disabled

//(https?:\/\/|www\d?\.)
//TomyLobo: if you have a user-submitted url you want to check
//TomyLobo: this should be anchored
//TomyLobo: ^$

	this.defaultApps = [
		{
			"title": "Steam",
			"file": "",
			"commandformat": "",
			"download": "http://store.steampowered.com/about/",
			"reference": "https://en.wikipedia.org/wiki/Steam_(software)",
			"filepaths": {}
		},
		{
			"title": "Windows",
			"file": "",
			"commandformat": "",
			"download": "http://www.microsoft.com/Windows10",
			"reference": "https://en.wikipedia.org/wiki/Microsoft_Windows",
			"filepaths": {}
		}
	];
	this.defaultApps = [];	// disabled

//	var defaultOtherType = this.defaultTypes[this.defaultTypes.length-1];
//	var key = this.generatePushId();
//	this.defaultApps[0].filepaths[key] = {
//		"id": key,
//		"path": "",
//		"type": defaultOtherType,
//							}
//	};
}

Metaverse.prototype.getMenus = function(options)
{
	var item;
	if( !!options && !!options.itemId )
		item = this.library.items[options.itemId].current;

	var type;
	if( !!options && !!options.typeId )
		type = this.library.types[options.typeId].current;

	var database;
	if( !!options && !!options.databaseId )
		database = this.library.databases[options.databaseId].current;

	var platform;
	if( !!options && !!options.platformId )
		platform = this.library.platforms[options.platformId].current;

	var app;
	if( !!options && !!options.appId )
		app = this.library.apps[options.appId].current;

	var map;
	if( !!options && !!options.mapId )
		map = this.library.maps[options.mapId].current;

	var instance;
	if( !!options && !!options.instanceId )
	{
		instance = this.library.instances[options.instanceId].current;
	}

	var objects;
	if( !!options && !!options.objectId )
	{
		object = this.library.objects[options.objectId].current;
	}

	var model;
	if( !!options && !!options.modelId )
		model = this.library.models[options.modelId].current;

	var menus = {
		"metaverseMenu":
		{
			"menuId": "metaverseMenu",
			"menuHeader": "Metaverse",
			"quickJoin":
			{
				"type": "button",
				"value": "Quick Join",
				"action": "quickJoin"
			},
			"firebaseConnect":
			{
				"type": "button",
				"value": "Connect to Firebase",
				"action": "firebaseConnect"
			},
			"firebaseHost":
			{
				"type": "button",
				"value": "Host Firebase",
				"action": "firebaseHost"
			},
			"localLoad":
			{
				"type": "button",
				"value": "Load Local Session",
				"action": "localLoad"
			},
			"localNew":
			{
				"type": "button",
				"value": "New Local Session",
				"action": "localNew"
			}
		},
		"firebaseConnectMenu":
		{
			"menuId": "firebaseConnectMenu",
			"menuHeader": "Firebase",
			"address":
			{
				"label": "Firebase Address: ",
				"type": "text",
				"value": this.quickJoinAddress,
				"focus": true
			},
			"connect":
			{
				"type": "submit",
				"value": "Connect",
				"action": "connectToFirebase"
			},
			"cancel":
			{
				"type": "button",
				"value": "Cancel",
				"action": "cancelFirebaseConnect"
			}
		},
		"universeMenu":
		{
			"menuId": "universeMenu",
			"menuHeader": "Universe",
			"universeSelect":
			{
				"type": "select",
				"generateOptions": "universeTitles",
				"focus": true,
				"action": "universeSelectChange"
			},
			"joinUniverse":
			{
				"type": "submit",
				"value": "Join Universe",
				"action": "joinUniverse"
			},
			"newUniverse":
			{
				"type": "button",
				"value": "New Universe",
				"action": "newUniverse"
			},
			"cancel":
			{
				"type": "button",
				"value": "Cancel",
				"action": "cancelUniverse"
			}
		},
		"newUniverseMenu":
		{
			"menuId": "newUniverseMenu",
			"menuHeader": "Universe / New",
			"title":
			{
				"label": "Universe Title: ",
				"type": "text",
				"value": "",
				"placeholder": "string",
				"focus": true
			},
			"createUniverse":
			{
				"type": "submit",
				"value": "Create Universe",
				"action": "createUniverse"
			},
			"cancel":
			{
				"type": "button",
				"value": "Cancel",
				"action": "cancelNewUniverse"
			}
		},
		"signUpMenu":
		{
			"menuId": "signUpMenu",
			"menuHeader": "Dashboard / Sign-Up",
			"username":
			{
				"label": "Username: ",
				"type": "text",
				"value": this.defaultUser.username.default,
				"placeholder": this.defaultUser.username.types,
				"focus": true
			},
			"passcode":
			{
				"label": "<u>Public</u> Passcode: ",
				"type": "password",
				"value": this.defaultUser.passcode.default,
				"placeholder": this.defaultUser.passcode.types
			},
			/*"displayName":
			{
				"label": "Display Name: ",
				"type": "text",
				"value": this.defaultUser.displayName.default,
				"placeholder": this.defaultUser.displayName.types
			},*/
			"signUp":
			{
				"type": "submit",
				"value": "Sign-Up",
				"action": "doSignUp"
			},
			"cancel":
			{
				"type": "button",
				"value": "Cancel",
				"action": "cancelSignUp"
			}
		},
		"logInMenu":
		{
			"menuId": "logInMenu",
			"menuHeader": "Dashboard / Log-In",
			"username":
			{
				"label": "Username: ",
				"type": "text",
				"value": this.defaultUser.username.default,
				"placeholder": this.defaultUser.username.types,
				"focus": true
			},
			"passcode":
			{
				"label": "<u>Public</u> Passcode: ",
				"type": "password",
				"value": this.defaultUser.passcode.default,
				"placeholder": this.defaultUser.passcode.types
			},
			"logIn":
			{
				"type": "submit",
				"value": "Log-In",
				"action": "doLogIn"
			},
			"cancel":
			{
				"type": "button",
				"value": "Cancel",
				"action": "cancelLogIn"
			}
		},
		"dashboardMenu":
		{
			"menuId": "dashboardMenu",
			"menuHeader": "Dashboard",
			"library":
			{
				"type": "button",
				"value": "Library",
				"action": "showLibrary"
			},
			"logIn":
			{
				"type": "button",
				"value": "Log-In",
				"action": "logIn"
			},
			"signUp":
			{
				"type": "button",
				"value": "Sign-Up",
				"action": "signUp"
			},
			"account":
			{
				"type": "button",
				"value": "Account",
				"action": "showAccount"
			},
			"disconnect":
			{
				"type": "button",
				"value": "Disconnect",
				"action": "disconnectMetaverse"
			}
		},
		"libraryMenu":
		{
			"menuId": "libraryMenu",
			"menuHeader": "Dashboard / Library",
			"items":
			{
				"type": "button",
				"value": "Items",
				"action": "showLibraryItems"
			},
			"models":
			{
				"type": "button",
				"value": "Models",
				"action": "showLibraryModels"
			},
			"maps":
			{
				"type": "button",
				"value": "Maps",
				"action": "showLibraryMaps"
			},
			"instances":
			{
				"type": "button",
				"value": "Instances",
				"action": "showLibraryInstances"
			},
			"objects":
			{
				"type": "button",
				"value": "Objects",
				"action": "showLibraryObjects"
			},
			"types":
			{
				"type": "button",
				"value": "Types",
				"action": "showLibraryTypes"
			},
			"apps":
			{
				"type": "button",
				"value": "Apps",
				"action": "showLibraryApps"
			},
			"databases":
			{
				"type": "button",
				"value": "Databases",
				"action": "showLibraryDatabases"
			},
			"platforms":
			{
				"type": "button",
				"value": "Platforms",
				"action": "showLibraryPlatforms"
			},
			"cancel":
			{
				"type": "button",
				"value": "Cancel",
				"action": "cancelLibrary"
			}
		},
		"accountMenu":
		{
			"menuId": "accountMenu",
			"menuHeader": "Dashboard / Account",
			"metaverse":
			{
				"label": "Metaverse: ",
				"type": "text",
				"value": this.root,
				"placeholder": "uri",
				"locked": true
			},
			"universe":
			{
				"label": "Universe: ",
				"type": "text",
				"value": this.universe,
				"placeholder": "string",
				"locked": true
			},
			"username":
			{
				"label": "Username: ",
				"type": "text",
				"value": this.localUser.username,
				"placeholder": "string",
				"locked": true
			},
			/*"displayName":
			{
				"label": "Display Name: ",
				"type": "text",
				"value": this.localUser.displayName,
				"placeholder": "string",
				"focus": true
			},*/
			"oldPasscode":
			{
				"label": "Old <u>Public</u> Passcode: ",
				"type": "password",
				"value": "",
				"placeholder": "string"
			},
			"newPasscode":
			{
				"label": "New <u>Public</u> Passcode: ",
				"type": "password",
				"value": "",
				"placeholder": "string"
			},
			"newPasscodeAgain":
			{
				"label": "New Passcode Again: ",
				"type": "password",
				"value": "",
				"placeholder": "string"
			},
			"changePasscode":
			{
				"type": "button",
				"value": "Change Passcode",
				"action": "changePasscode"
			},
			"save":
			{
				"type": "submit",
				"value": "Save",
				"action": "saveAccount"
			},
			"cancel":
			{
				"type": "button",
				"value": "Cancel",
				"action": "cancelAccount"
			}
		},
		"libraryItems":
		{
			"menuId": "libraryItems",
			"menuHeader": "Dashboard / Library / Items",
			"itemSelect":
			{
				"type": "select",
				"generateOptions": "libraryItems",
				"focus": true,
				"action": "libraryItemsSelectChange"
			},
			"selectItem":
			{
				"type": "submit",
				"value": "Select Item",
				"action": "selectItem"
			},
			"editItem":
			{
				"type": "button",
				"value": "Edit Item",
				"action": "showEditItem"
			},
			"newItem":
			{
				"type": "button",
				"value": "Create New Item",
				"action": "showCreateItem"
			},
			"cancel":
			{
				"type": "button",
				"value": "Cancel",
				"action": "cancelLibraryItems"
			}
		},
		"libraryMaps":
		{
			"menuId": "libraryMaps",
			"menuHeader": "Dashboard / Library / Maps",
			"mapSelect":
			{
				"type": "select",
				"generateOptions": "libraryMaps",
				"focus": true,
				"action": "libraryMapSelectChange"
			},
			"editMap":
			{
				"type": "submit",
				"value": "Edit Map",
				"action": "showEditMap"
			},
			"newMap":
			{
				"type": "button",
				"value": "Create New Map",
				"action": "showCreateMap"
			},
			"cancel":
			{
				"type": "button",
				"value": "Cancel",
				"action": "cancelLibraryMaps"
			}
		},
		"libraryInstances":
		{
			"menuId": "libraryInstances",
			"menuHeader": "Dashboard / Library / Instances",
			"instanceSelect":
			{
				"type": "select",
				"generateOptions": "libraryInstances",
				"focus": true,
				"action": "libraryInstanceSelectChange"
			},
			"editInstance":
			{
				"type": "submit",
				"value": "Edit Instance",
				"action": "showEditInstance"
			},
			"newInstance":
			{
				"type": "button",
				"value": "Create New Instance",
				"action": "showCreateInstance"
			},
			"cancel":
			{
				"type": "button",
				"value": "Cancel",
				"action": "cancelLibraryInstances"
			}
		},
		"libraryObjects":
		{
			"menuId": "libraryObjects",
			"menuHeader": "Dashboard / Library / Objects",
			"objectSelect":
			{
				"type": "select",
				"generateOptions": "libraryObjects",
				"focus": true,
				"action": "libraryObjectSelectChange"
			},
			"editObject":
			{
				"type": "submit",
				"value": "Edit Object",
				"action": "showEditObject"
			},
			"newObject":
			{
				"type": "button",
				"value": "Create New Object",
				"action": "showCreateObject"
			},
			"cancel":
			{
				"type": "button",
				"value": "Cancel",
				"action": "cancelLibraryObjects"
			}
		},
		"libraryModels":
		{
			"menuId": "libraryItems",
			"menuHeader": "Dashboard / Library / Models",
			"modelSelect":
			{
				"type": "select",
				"generateOptions": "libraryModels",
				"focus": true,
				"action": "libraryModelSelectChange"
			},
			"editModel":
			{
				"type": "submit",
				"value": "Edit Model",
				"action": "showEditModel"
			},
			"newModel":
			{
				"type": "button",
				"value": "Create New Model",
				"action": "showCreateModel"
			},
			"cancel":
			{
				"type": "button",
				"value": "Cancel",
				"action": "cancelLibraryModels"
			}
		},
		"libraryItemsCreate":
		{
			"menuId": "libraryItemsCreate",
			"menuHeader": "Dashboard / Library / Items / New",
			"shortcut":
			{
				"label": "Shortcut: ",
				"type": "text",
				"value": "",
				"placeholder": "uri|string",
				"focus": true
			},
			"create":
			{
				"type": "submit",
				"value": "Create Item",
				"action": "generateNewItem"
			},
			"cancel":
			{
				"type": "button",
				"value": "Cancel",
				"action": "cancelLibraryItemCreate"
			}
		},
		"libraryMapsCreate":
		{
			"menuId": "libraryMapsCreate",
			"menuHeader": "Dashboard / Library / Maps / New",
		},
		"libraryInstancesCreate":
		{
			"menuId": "libraryInstancesCreate",
			"menuHeader": "Dashboard / Library / Instances / New",
		},
		"libraryObjectsCreate":
		{
			"menuId": "libraryObjectsCreate",
			"menuHeader": "Dashboard / Library / Objects / New",
		},
		"libraryModelsCreate":
		{
			"menuId": "libraryModelsCreate",
			"menuHeader": "Dashboard / Library / Models / New",
		},
		"libraryItemsEdit":
		{
			"menuId": "libraryItemsEdit",
			"menuHeader": "Dashboard / Library / Items / Edit"
		},
		"libraryTypes":
		{
			"menuId": "libraryTypes",
			"menuHeader": "Dashboard / Library / Types",
			"typeSelect":
			{
				"type": "select",
				"generateOptions": "libraryTypes",
				"focus": true,
				"action": "libraryTypesSelectChange"
			},
			"editType":
			{
				"type": "submit",
				"value": "Edit Type",
				"action": "showEditType"
			},
			"newType":
			{
				"type": "button",
				"value": "Create New Type",
				"action": "showCreateType"
			},
			"cancel":
			{
				"type": "button",
				"value": "Cancel",
				"action": "cancelLibraryTypes"
			}
		},
		"libraryDatabases":
		{
			"menuId": "libraryDatabases",
			"menuHeader": "Dashboard / Library / Databases",
			"databaseSelect":
			{
				"type": "select",
				"generateOptions": "libraryDatabases",
				"focus": true,
				"action": "libraryDatabasesSelectChange"
			},
			"editDatabase":
			{
				"type": "submit",
				"value": "Edit Database",
				"action": "showEditDatabase"
			},
			"newDatabase":
			{
				"type": "button",
				"value": "Create New Database",
				"action": "showCreateDatabase"
			},
			"cancel":
			{
				"type": "button",
				"value": "Cancel",
				"action": "cancelLibraryDatabases"
			}
		},
		"libraryApps":
		{
			"menuId": "libraryApps",
			"menuHeader": "Dashboard / Library / Apps",
			"appSelect":
			{
				"type": "select",
				"generateOptions": "libraryApps",
				"focus": true,
				"action": "libraryAppsSelectChange"
			},
			"editApp":
			{
				"type": "submit",
				"value": "Edit App",
				"action": "showEditApp"
			},
			"newApp":
			{
				"type": "button",
				"value": "Create New App",
				"action": "showCreateApp"
			},
			"cancel":
			{
				"type": "button",
				"value": "Cancel",
				"action": "cancelLibraryApps"
			}
		},
		"libraryTypesCreate":
		{
			"menuId": "libraryTypesCreate",
			"menuHeader": "Dashboard / Library / Types / New",
		},
		"libraryAppsCreate":
		{
			"menuId": "libraryAppsCreate",
			"menuHeader": "Dashboard / Library / Apps / New",
		},
		"libraryTypesEdit":
		{
			"menuId": "libraryTypesEdit",
			"menuHeader": "Dashboard / Library / Types / Edit",
		},
		"libraryMapsEdit":
		{
			"menuId": "libraryMapsEdit",
			"menuHeader": "Dashboard / Library / Maps / Edit",
		},
		"libraryInstancesEdit":
		{
			"menuId": "libraryInstancesEdit",
			"menuHeader": "Dashboard / Library / Instances / Edit",
		},
		"libraryObjectsEdit":
		{
			"menuId": "libraryObjectsEdit",
			"menuHeader": "Dashboard / Library / Objects / Edit",
		},
		"libraryModelsEdit":
		{
			"menuId": "libraryModelsEdit",
			"menuHeader": "Dashboard / Library / Models / Edit",
		},
		"libraryAppsEdit":
		{
			"menuId": "libraryAppsEdit",
			"menuHeader": "Dashboard / Library / Apps / Edit",
		},
		"libraryPlatforms":
		{
			"menuId": "libraryPlatforms",
			"menuHeader": "Dashboard / Library / Platforms",
			"platformSelect":
			{
				"type": "select",
				"generateOptions": "libraryPlatforms",
				"focus": true,
				"action": "libraryPlatformsSelectChange"
			},
			"editPlatform":
			{
				"type": "submit",
				"value": "Edit Platform",
				"action": "showEditPlatform"
			},
			"newPlatform":
			{
				"type": "button",
				"value": "Create New Platform",
				"action": "showCreatePlatform"
			},
			"cancel":
			{
				"type": "button",
				"value": "Cancel",
				"action": "cancelLibraryPlatforms"
			}
		},
		"libraryDatabasesCreate":
		{
			"menuId": "libraryDatabasesCreate",
			"menuHeader": "Dashboard / Library / Databases / New",
		},
		"libraryDatabasesEdit":
		{
			"menuId": "libraryDatabasesEdit",
			"menuHeader": "Dashboard / Library / Databases / Edit",	
		},
		"libraryPlatformsCreate":
		{
			"menuId": "libraryPlatformsCreate",
			"menuHeader": "Dashboard / Library / Platforms / New",
		},
		"libraryPlatformsEdit":
		{
			"menuId": "libraryPlatformsEdit",
			"menuHeader": "Dashboard / Library / Platforms / Edit",	
		}
	};

	var x;
	var menuId;

	// MENU
	menuId = "libraryItemsEdit";
	if( !!item )
	{
		menus[menuId]["id"] = {
			"label": "ID: ",
			"type": "text",
			"value": item.info.id,
			"placeholder": "autokey",
			"locked": true
		};
	}
	for( x in this.defaultItem )
	{
		if( x === "info" )
			continue;

		menus[menuId][x] = {
			"label": this.defaultItem[x].label + ": ",
			"type": "text",
			"value": (!!item) ? item[x] : "",
			"placeholder": this.defaultItem[x].types
		};
	}
	menus[menuId]["save"] = {
		"type": "submit",
		"value": "Save",
		"action": "saveLibraryItemEdit"
	};
	menus[menuId]["cancel"] = {
		"type": "button",
		"value": "Cancel",
		"action": "cancelLibraryItemEdit"
	};

	// MENU
	menuId = "libraryMapsCreate";
	for( x in this.defaultMap )
	{
		if( x === "info" )
			continue;

		if( !this.defaultMap[x].hasOwnProperty("default") )
		{
			menus[menuId][x] = {
				"label": this.defaultMap[x].label
			};

			var y;
			for( y in this.defaultMap[x] )
			{
				if( y === "label" )
					continue;
				
				menus[menuId][x][y] = {
					"label": this.defaultMap[x][y].label + ": ",
					"type": "text",
					"value": "",
					"placeholder": this.defaultMap[x][y].types
				};
			}
		}
		else
		{
			menus[menuId][x] = {
				"label": this.defaultMap[x].label + ": ",
				"type": "text",
				"value": (!!item) ? item[x] : "",
				"placeholder": this.defaultMap[x].types
			};
		}
	}
	menus[menuId]["save"] = {
		"type": "submit",
		"value": "Save",
		"action": "createLibraryMap"
	};
	menus[menuId]["cancel"] = {
		"type": "button",
		"value": "Cancel",
		"action": "cancelLibraryMapCreate"
	};

	// MENU
	menuId = "libraryInstancesCreate";
	for( x in this.defaultInstance )
	{
		if( x === "info" )
			continue;

		if( !this.defaultInstance[x].hasOwnProperty("default") )
		{
			menus[menuId][x] = {
				"label": this.defaultInstance[x].label
			};

			var y;
			for( y in this.defaultInstance[x] )
			{
				if( y === "label" )
					continue;
				
				menus[menuId][x][y] = {
					"label": this.defaultInstance[x][y].label + ": ",
					"type": "text",
					"value": "",
					"placeholder": this.defaultInstance[x][y].types
				};
			}
		}
		else
		{
			menus[menuId][x] = {
				"label": this.defaultInstance[x].label + ": ",
				"type": "text",
				"value": (!!item) ? item[x] : "",
				"placeholder": this.defaultInstance[x].types
			};
		}
	}
	menus[menuId]["save"] = {
		"type": "submit",
		"value": "Save",
		"action": "createLibraryInstance"
	};
	menus[menuId]["cancel"] = {
		"type": "button",
		"value": "Cancel",
		"action": "cancelLibraryInstanceCreate"
	};

	// MENU
	menuId = "libraryObjectsCreate";
	for( x in this.defaultObject )
	{
		if( x === "info" )
			continue;

		if( !this.defaultObject[x].hasOwnProperty("default") )
		{
			menus[menuId][x] = {
				"label": this.defaultObject[x].label
			};

			var y;
			for( y in this.defaultObject[x] )
			{
				if( y === "label" )
					continue;
				
				menus[menuId][x][y] = {
					"label": this.defaultObject[x][y].label + ": ",
					"type": "text",
					"value": "",
					"placeholder": this.defaultObject[x][y].types
				};
			}
		}
		else
		{
			menus[menuId][x] = {
				"label": this.defaultObject[x].label + ": ",
				"type": "text",
				"value": (!!item) ? item[x] : "",
				"placeholder": this.defaultObject[x].types
			};
		}
	}
	menus[menuId]["save"] = {
		"type": "submit",
		"value": "Save",
		"action": "createLibraryEntry"
	};
	menus[menuId]["cancel"] = {
		"type": "button",
		"value": "Cancel",
		"action": "cancelLibraryObjectCreate"
	};

	// MENU
	menuId = "libraryModelsCreate";
	for( x in this.defaultModel )
	{
		if( x === "info" )
			continue;

		if( !this.defaultModel[x].hasOwnProperty("default") )
		{
			menus[menuId][x] = {
				"label": this.defaultModel[x].label
			};

			var y;
			for( y in this.defaultModel[x] )
			{
				if( y === "label" )
					continue;
				
				menus[menuId][x][y] = {
					"label": this.defaultModel[x][y].label + ": ",
					"type": "text",
					"value": "",
					"placeholder": this.defaultModel[x][y].types
				};
			}
		}
		else
		{
			menus[menuId][x] = {
				"label": this.defaultModel[x].label + ": ",
				"type": "text",
				"value": (!!item) ? item[x] : "",
				"placeholder": this.defaultModel[x].types
			};
		}
	}
	menus[menuId]["save"] = {
		"type": "submit",
		"value": "Save",
		"action": "createLibraryModel"
	};
	menus[menuId]["cancel"] = {
		"type": "button",
		"value": "Cancel",
		"action": "cancelLibraryModelCreate"
	};

	// MENU
	menuId = "libraryTypesCreate";
	for( x in this.defaultType )
	{
		if( x === "info" )
			continue;

		menus[menuId][x] = {
			"label": this.defaultType[x].label + ": ",
			"type": "text",
			"value": (!!item) ? item[x] : "",
			"placeholder": this.defaultType[x].types
		};
	}
	menus[menuId]["save"] = {
		"type": "submit",
		"value": "Save",
		"action": "createLibraryType"
	};
	menus[menuId]["cancel"] = {
		"type": "button",
		"value": "Cancel",
		"action": "cancelLibraryTypeCreate"
	};

/*
			"path":
			{
				"label": "Path",
				"default": "",
				"types": "string",
				"format": "/^.{2,1024}$/i",
				"formatDescription": "Executable must be a valid local file."
			},
			"type":
			{
				"label": "Type",
				"default": "",
				"types": "autokey",
				"format": "/.+$/i",
				"formatDescription": "Type must be an auto-generated key."
			}
*/

	// MENU
	menuId = "libraryAppsCreate";
	for( x in this.defaultApp )
	{
		if( x === "info" )
			continue;

		if( !this.defaultApp[x].hasOwnProperty("default") )
		{
			menus[menuId][x] = {
				"label": this.defaultApp[x].label
			};

			var y;
			for( y in this.defaultApp[x] )
			{
				if( y === "label" )
					continue;
				
				menus[menuId][x][y] = {
					"label": this.defaultApp[x][y].label + ": ",
					"type": "text",
					"value": "",
					"placeholder": this.defaultApp[x][y].types
				};
			}
		}
		else
		{
			menus[menuId][x] = {
				"label": this.defaultApp[x].label + ": ",
				"type": "text",
				"value": (!!item) ? item[x] : "",
				"placeholder": this.defaultApp[x].types
			};
		}
	}
	menus[menuId]["save"] = {
		"type": "submit",
		"value": "Save",
		"action": "createLibraryApp"
	};
	menus[menuId]["cancel"] = {
		"type": "button",
		"value": "Cancel",
		"action": "cancelLibraryAppCreate"
	};

	// MENU
	menuId = "libraryTypesEdit";
	if( !!type )
	{
		menus[menuId]["id"] = {
			"label": "ID: ",
			"type": "text",
			"value": type.info.id,
			"placeholder": "autokey",
			"locked": true
		};
	}
	for( x in this.defaultType )
	{
		if( x === "info" )
			continue;

		menus[menuId][x] = {
			"label": this.defaultType[x].label + ": ",
			"type": "text",
			"value": (!!type) ? type[x] : "",
			"placeholder": this.defaultType[x].types
		};
	}
	menus[menuId]["save"] = {
		"type": "submit",
		"value": "Save",
		"action": "updateLibraryType"
	};
	menus[menuId]["cancel"] = {
		"type": "button",
		"value": "Cancel",
		"action": "cancelLibraryTypeEdit"
	};

	// MENU
	menuId = "libraryMapsEdit";
	if( !!map )
	{
		menus[menuId]["id"] = {
			"label": "ID: ",
			"type": "text",
			"value": map.info.id,
			"placeholder": "autokey",
			"locked": true
		};
	}
	for( x in this.defaultMap )
	{
		if( x === "info" )
			continue;

		if( !!map && typeof map[x] === "object" )
		{
			menus[menuId][x] = {
				"label": this.defaultMap[x].label,
				"type": "child",
				"value": (!!map) ? map[x] : "",
				"placeholder": this.defaultMap[x]
			};
		}
		else
		{
			menus[menuId][x] = {
				"label": this.defaultMap[x].label + ": ",
				"type": "text",
				"value": (!!map) ? map[x] : "",
				"placeholder": this.defaultMap[x].types
			};
		}
	}
	menus[menuId]["save"] = {
		"type": "submit",
		"value": "Save",
		"action": "updateLibraryMap"
	};
	menus[menuId]["cancel"] = {
		"type": "button",
		"value": "Cancel",
		"action": "cancelLibraryMapEdit"
	};

	// MENU
	menuId = "libraryInstancesEdit";
	if( !!instance )
	{
		menus[menuId]["id"] = {
			"label": "ID: ",
			"type": "text",
			"value": instance.info.id,
			"placeholder": "autokey",
			"locked": true
		};
	}

	for( x in this.defaultInstance )
	{
		if( x === "info" )
			continue;

		if( !!instance && typeof instance[x] === "object" )
		{
			menus[menuId][x] = {
				"label": this.defaultInstance[x].label,
				"type": "child",
				"value": (!!instance) ? instance[x] : "",
				"placeholder": this.defaultInstance[x]
			};
		}
		else
		{
			menus[menuId][x] = {
				"label": this.defaultInstance[x].label + ": ",
				"type": "text",
				"value": (!!instance) ? instance[x] : "",
				"placeholder": this.defaultInstance[x].types
			};
		}
	}
	menus[menuId]["save"] = {
		"type": "submit",
		"value": "Save",
		"action": "updateLibraryInstance"
	};
	menus[menuId]["cancel"] = {
		"type": "button",
		"value": "Cancel",
		"action": "cancelLibraryInstanceEdit"
	};

	// MENU
	menuId = "libraryObjectsEdit";
	if( !!object )
	{
		menus[menuId]["id"] = {
			"label": "ID: ",
			"type": "text",
			"value": object.info.id,
			"placeholder": "autokey",
			"locked": true
		};
	}

	for( x in this.defaultObject )
	{
		if( x === "info" )
			continue;

		if( !!object && typeof object[x] === "object" )
		{
			menus[menuId][x] = {
				"label": this.defaultObject[x].label,
				"type": "child",
				"value": (!!object) ? object[x] : "",
				"placeholder": this.defaultObject[x]
			};
		}
		else
		{
			menus[menuId][x] = {
				"label": this.defaultObject[x].label + ": ",
				"type": "text",
				"value": (!!object) ? object[x] : "",
				"placeholder": this.defaultObject[x].types
			};
		}
	}
	menus[menuId]["save"] = {
		"type": "submit",
		"value": "Save",
		"action": "updateLibraryEntry"
	};
	menus[menuId]["cancel"] = {
		"type": "button",
		"value": "Cancel",
		"action": "cancelLibraryObjectEdit"
	};

	// MENU
	menuId = "libraryModelsEdit";
	if( !!model )
	{
		menus[menuId]["id"] = {
			"label": "ID: ",
			"type": "text",
			"value": model.info.id,
			"placeholder": "autokey",
			"locked": true
		};
	}
	for( x in this.defaultModel )
	{
		if( x === "info" )
			continue;

		if( !!model && typeof model[x] === "object" )
		{
			//console.log(model[x]);
			menus[menuId][x] = {
				"label": this.defaultModel[x].label,
				"type": "child",
				"value": (!!model) ? model[x] : "",
				"placeholder": this.defaultModel[x]
			};
		}
		else
		{
			menus[menuId][x] = {
				"label": this.defaultModel[x].label + ": ",
				"type": "text",
				"value": (!!model) ? model[x] : "",
				"placeholder": this.defaultModel[x].types
			};
		}
	}
	menus[menuId]["save"] = {
		"type": "submit",
		"value": "Save",
		"action": "updateLibraryModel"
	};
	menus[menuId]["cancel"] = {
		"type": "button",
		"value": "Cancel",
		"action": "cancelLibraryModelEdit"
	};

	// MENU
	menuId = "libraryAppsEdit";
	if( !!app )
	{
		menus[menuId]["id"] = {
			"label": "ID: ",
			"type": "text",
			"value": app.info.id,
			"placeholder": "autokey",
			"locked": true
		};
	}
	for( x in this.defaultApp )
	{
		if( x === "info" )
			continue;

		if( !!app && typeof app[x] === "object" )
		{
			//console.log(app[x]);
			menus[menuId][x] = {
				"label": this.defaultApp[x].label + ": ",
				"type": "child",
				"value": (!!app) ? app[x] : "",
				"placeholder": this.defaultApp[x]
			};
		}
		else
		{
			menus[menuId][x] = {
				"label": this.defaultApp[x].label + ": ",
				"type": "text",
				"value": (!!app) ? app[x] : "",
				"placeholder": this.defaultApp[x].types
			};
		}
	}
	menus[menuId]["save"] = {
		"type": "submit",
		"value": "Save",
		"action": "updateLibraryApp"
	};
	menus[menuId]["cancel"] = {
		"type": "button",
		"value": "Cancel",
		"action": "cancelLibraryAppEdit"
	};

	// MENU
	menuId = "libraryDatabasesCreate";
	for( x in this.defaultDatabase )
	{
		if( x === "info" )
			continue;

		if( !this.defaultDatabase[x].hasOwnProperty("default") )
		{
			menus[menuId][x] = {
				"label": this.defaultDatabase[x].label
			};

			var y;
			for( y in this.defaultDatabase[x] )
			{
				if( y === "label" )
					continue;
				
				menus[menuId][x][y] = {
					"label": this.defaultDatabase[x][y].label + ": ",
					"type": "text",
					"value": "",
					"placeholder": this.defaultDatabase[x][y].types
				};
			}
		}
		else
		{
			menus[menuId][x] = {
				"label": this.defaultDatabase[x].label + ": ",
				"type": "text",
				"value": (!!item) ? item[x] : "",
				"placeholder": this.defaultDatabase[x].types
			};
		}
	}
	menus[menuId]["save"] = {
		"type": "submit",
		"value": "Save",
		"action": "createLibraryDatabase"
	};
	menus[menuId]["cancel"] = {
		"type": "button",
		"value": "Cancel",
		"action": "cancelLibraryDatabaseCreate"
	};

	// MENU
	menuId = "libraryPlatformsCreate";
	if( !!platform )
	{
		menus[menuId]["id"] = {
			"label": "ID: ",
			"type": "text",
			"value": platform.info.id,
			"placeholder": "autokey",
			"locked": true
		};
	}
	for( x in this.defaultPlatform )
	{
		if( x === "info" )
			continue;

		menus[menuId][x] = {
			"label": this.defaultPlatform[x].label + ": ",
			"type": "text",
			"value": (!!platform) ? platform[x] : "",
			"placeholder": this.defaultPlatform[x].types
		};
	}
	menus[menuId]["save"] = {
		"type": "submit",
		"value": "Save",
		"action": "createLibraryPlatform"
	};
	menus[menuId]["cancel"] = {
		"type": "button",
		"value": "Cancel",
		"action": "cancelLibraryPlatformCreate"
	};

	// MENU
	menuId = "libraryPlatformsEdit";
	if( !!platform )
	{
		menus[menuId]["id"] = {
			"label": "ID: ",
			"type": "text",
			"value": platform.info.id,
			"placeholder": "autokey",
			"locked": true
		};
	}
	for( x in this.defaultPlatform )
	{
		if( x === "info" )
			continue;

		menus[menuId][x] = {
			"label": this.defaultPlatform[x].label + ": ",
			"type": "text",
			"value": (!!platform) ? platform[x] : "",
			"placeholder": this.defaultPlatform[x].types
		};
	}
	menus[menuId]["save"] = {
		"type": "submit",
		"value": "Save",
		"action": "updateLibraryPlatform"
	};
	menus[menuId]["cancel"] = {
		"type": "button",
		"value": "Cancel",
		"action": "cancelLibraryPlatformEdit"
	};

	// MENU
	menuId = "libraryDatabasesEdit";
	if( !!database )
	{
		menus[menuId]["id"] = {
			"label": "ID: ",
			"type": "text",
			"value": database.info.id,
			"placeholder": "autokey",
			"locked": true
		};
	}
	for( x in this.defaultDatabase )
	{
		if( x === "info" )
			continue;

		if( !!database && typeof database[x] === "object" )
		{
			//console.log(model[x]);
			menus[menuId][x] = {
				"label": this.defaultDatabase[x].label,
				"type": "child",
				"value": (!!database) ? database[x] : "",
				"placeholder": this.defaultDatabase[x]
			};
		}
		else
		{
			menus[menuId][x] = {
				"label": this.defaultDatabase[x].label + ": ",
				"type": "text",
				"value": (!!database) ? database[x] : "",
				"placeholder": this.defaultDatabase[x].types
			};
		}
/*
		menus[menuId][x] = {
			"label": this.defaultDatabase[x].label + ": ",
			"type": "text",
			"value": (!!database) ? database[x] : "",
			"placeholder": this.defaultDatabase[x].types
		};
		*/
	}
	menus[menuId]["save"] = {
		"type": "submit",
		"value": "Save",
		"action": "updateLibraryDatabase"
	};
	menus[menuId]["cancel"] = {
		"type": "button",
		"value": "Cancel",
		"action": "cancelLibraryDatabaseEdit"
	};

	return menus;
};

Metaverse.prototype.tokenize = function(string)
{
	return string.split(/[,\s]+/g);
};

Metaverse.prototype.showMenu = function(menuId, options)
{
	//console.log(this.getMenus(options)[menuId]);
	//console.log(options);
	//this.eventHandler("showMenu", this.getMenus(options)[menuId]);
	this.eventHandler("showMenu", this.getMenus(options)[menuId]);
};

Metaverse.prototype.menuAction = function(actionName, actionData)
{
	/*
	if( actionName === "quickJoin" )
	{
		this.eventHandler("freezeInputs");
		this.connect(this.quickJoinAddress, function(error)
		{
			if( !!error )
			{
				this.error = error;
				this.eventHandler("error", this.error);
				this.eventHandler("unfreezeInputs");
				return;
			}
			else
				this.showMenu("universeMenu");
		}.bind(this));
	}
	else if( actionName === "cancelUniverse" )
	{
		this.reset();
		this.showMenu("metaverseMenu");
	}
	else if( actionName === "firebaseConnect" )
	{
		this.showMenu("firebaseConnectMenu");
	}
	else if( actionName === "cancelFirebaseConnect" )
	{
		this.reset();
		this.showMenu("metaverseMenu");
	}
	else if( actionName === "connectToFirebase" )
	{
		this.eventHandler("freezeInputs");

		var address = actionData["address"].value;
		this.connect(address, function(error)
		{
			if( !!error )
			{
				this.error = error;
				this.eventHandler("showError", this.error);
				this.eventHandler("unfreezeInputs");
				return;
			}
			else
			{
				this.showMenu("universeMenu");
			}
		}.bind(this));
	}
	else if( actionName === "joinUniverse" )
	{
		this.eventHandler("freezeInputs");

		var universeId = actionData["universeSelect"].options[actionData["universeSelect"].selectedIndex].value;
		this.joinUniverse(universeId, function(error)
		{
			if( !!error )
			{
				this.error = error;
				this.eventHandler("error", this.error);
				this.eventHandler("unfreezeInputs");
				return;
			}

			this.showMenu("dashboardMenu");
		}.bind(this));
	}
	else if( actionName === "newUniverse" )
	{
		this.showMenu("newUniverseMenu");
	}
	else if( actionName === "createUniverse" )
	{
		this.eventHandler("freezeInputs");

		var title = actionData["title"].value;
		this.createUniverse(title, "", function(universeId)
		{
			if( !!!universeId )
			{
				this.eventHandler("error", this.error);
				this.eventHandler("unfreezeInputs");
				return;
			}
			
			this.joinUniverse(universeId, function(error)
			{
				if( !!error )
				{
					this.error = error;
					this.eventHandler("error", this.error);
					this.showMenu("universeMenu");
					return;
				}

				this.showMenu("signUpMenu");
			}.bind(this));
		}.bind(this));
	}
	else if( actionName === "cancelNewUniverse" )
	{
		this.showMenu("universeMenu");
	}
	else if( actionName === "disconnectMetaverse" )
	{
		this.reset();
		this.showMenu("metaverseMenu");
	}
	else if( actionName === "signUp" )
	{
		this.showMenu("signUpMenu");
	}
	else if( actionName === "cancelSignUp" )
	{
		this.showMenu("dashboardMenu");
	}
	else if( actionName === "doSignUp" )
	{
		this.eventHandler("freezeInputs");

		var userData = {};
		var x;
		for( x in actionData )
		{
			if( actionData[x].type !== "button" && actionData[x].type !== "submit" )
				userData[x] = actionData[x].value;
		}

		this.createUser(userData, function(responseData)
		{
			if( !!!responseData )
			{
				this.eventHandler("error", this.error);
				this.eventHandler("unfreezeInputs");
				return;
			}

			this.logIn(responseData.username, actionData["passcode"].value, function(error)
			{
				if( !!error )
				{
					this.error = error;
					this.eventHandler("error", this.error);
					this.showMenu("logInMenu");
					return;
				}

				this.showMenu("dashboardMenu");
			}.bind(this));
		}.bind(this));
	}
	else if( actionName === "logIn" )
	{
		this.showMenu("logInMenu");
	}
	else if( actionName === "doLogIn" )
	{
		this.eventHandler("freezeInputs");

		var username = actionData["username"].value;
		var passcode = actionData["passcode"].value;
		this.logIn(username, passcode, function(error)
		{
			if( !!error )
			{
				this.error = error;
				this.eventHandler("error", this.error);
				this.eventHandler("unfreezeInputs");
				return;
			}
			
			this.showMenu("dashboardMenu");
		}.bind(this));
	}
	else if( actionName === "cancelLogIn" )
	{
		this.showMenu("dashboardMenu");
	}
	else if( actionName === "showLibrary" )
	{
		this.showMenu("libraryMenu");
	}
	else if( actionName === "cancelLibrary" )
	{
		this.showMenu("dashboardMenu");
	}
	else if( actionName === "showAccount" )
	{
		this.showMenu("accountMenu");
	}
	else if( actionName === "cancelAccount" )
	{
		this.showMenu("dashboardMenu");
	}
	else if( actionName === "changePasscode" )
	{
		actionData["changePasscode"].style.display = "none";
		actionData["oldPasscode"].parentNode.style.display = "block";
		actionData["newPasscode"].parentNode.style.display = "block";
		actionData["newPasscodeAgain"].parentNode.style.display = "block";
	}
	else if( actionName === "saveAccount" )
	{
		this.eventHandler("freezeInputs");

		var data = {};
		var x;
		for( x in actionData )
		{
			if( actionData[x].type !== "button" && actionData[x].type !== "submit" && x !== "metaverse" && x !== "universe" && x !== "oldPasscode" && x !== "newPasscode" && x !== "newPasscodeAgain" )
				data[x] = actionData[x].value;
		}

		if( actionData["oldPasscode"].parentNode.style.display !== "none" )
		{
			if( this.encodePasscode(actionData["oldPasscode"].value) !== this.localUser.passcode )
			{
				this.error = new Error("Incorrect public passcode.");
				this.eventHandler("error", this.error);
				this.eventHandler("unfreezeInputs");
				return;
			}

			if( actionData["newPasscode"].value !== actionData["newPasscodeAgain"].value )
			{
				this.error = new Error("New public passcodes do not match.");
				this.eventHandler("error", this.error);
				this.eventHandler("unfreezeInputs");
				return;
			}

			if( actionData["newPasscode"].value === "" || actionData["newPasscode"].value.length <= 5 )
			{
				this.error = new Error("Public passcodes must be at least 6 characters long.");
				this.eventHandler("error", this.error);
				this.eventHandler("unfreezeInputs");
				return;
			}

			data.passcode = this.encodePasscode(actionData["newPasscode"].value);
		}

		if( !this.validateData(data, this.defaultUser) )
		{
			this.eventHandler("unfreezeInputs");
			return;
		}

		this.updateLocalUser(data, function(error)
		{
			if( !!error )
			{
				this.error = error;
				this.eventHandler("error", this.error);
				this.eventHandler("unfreezeInputs");
				return;
			}

			this.showMenu("dashboardMenu");
		}.bind(this));
	}
	else if( actionName === "cancelLibraryItems" )
	{
		this.showMenu("libraryMenu");
	}
	else if( actionName === "cancelLibraryMaps" )
	{
		this.showMenu("libraryMenu");
	}
	else if( actionName === "cancelLibraryInstances" )
	{
		this.showMenu("libraryMenu");
	}
	else if( actionName === "cancelLibraryObjects" )
	{
		this.showMenu("libraryMenu");
	}
	else if( actionName === "cancelLibraryModels" )
	{
		this.showMenu("libraryMenu");
	}
	else if( actionName === "showLibraryItems" )
	{
		this.showMenu("libraryItems");
	}
	else if( actionName === "showLibraryModels" )
	{
		this.showMenu("libraryModels");
	}
	else if( actionName === "showLibraryMaps" )
	{
		this.showMenu("libraryMaps");
	}
	else if( actionName === "showLibraryInstances" )
	{
		this.showMenu("libraryInstances");
	}
	else if( actionName === "showLibraryObjects" )
	{
		this.showMenu("libraryObjects");
	}
	else if( actionName === "selectItem" )
	{
		console.log("Yarrr");
	}
	else if( actionName === "showEditItem" )
	{
		this.showMenu("libraryItemsEdit", {"itemId": actionData["itemSelect"].options[actionData["itemSelect"].selectedIndex].value});
	}
	else if( actionName === "cancelLibraryItemEdit" )
	{
		this.showMenu("libraryItems");
	}
	else if( actionName === "showCreateItem" )
	{
		this.showMenu("libraryItemsCreate");
	}
	else if( actionName === "showCreateMap" )
	{
		this.showMenu("libraryMapsCreate");
	}
	else if( actionName === "showCreateInstance" )
	{
		this.showMenu("libraryInstancesCreate");
	}
	else if( actionName === "showCreateObject" )
	{
		this.showMenu("libraryObjectsCreate");
	}
	else if( actionName === "showCreateModel" )
	{
		this.showMenu("libraryModelsCreate");
	}
	else if( actionName === "cancelLibraryItemCreate" )
	{
		this.showMenu("libraryItems");
	}
	else if( actionName === "cancelLibraryMapCreate" )
	{
		this.showMenu("libraryMaps");
	}
	else if( actionName === "cancelLibraryInstanceCreate" )
	{
		this.showMenu("libraryInstances");
	}
	else if( actionName === "cancelLibraryObjectCreate" )
	{
		this.showMenu("libraryObjects");
	}
	else if( actionName === "cancelLibraryModelCreate" )
	{
		this.showMenu("libraryModels");
	}
	else if( actionName === "generateNewItem" )
	{
		this.eventHandler("freezeInputs");

		if( actionData["shortcut"].value.length <= 2 )
		{
			this.error = new Error("Shortcuts must be at least 3 characters long.");
			this.eventHandler("error", this.error);
			this.eventHandler("unfreezeInputs");
			return;
		}

		// Generate the item.
		var file = actionData["shortcut"].value;

		var item = {};
		item.type = this.generateType(file);
		item.title = this.generateTitle(file, item.type);
		item.file = file;
		item.app = "";
		item.description = "";
		item.preview = "";
		item.reference = "";
		item.download = "";
		item.stream = "";
		item.model = "";
		item.marquee = "";
		item.screen = "";

		// Now that the item is constructed, let's check for an existing twin.
		this.findTwinLibraryObject("Item", item, function(twin)
		{
			if( !!twin )
			{
				this.error = new Error("A version of that item already exists!");
				this.eventListener("error", this.error);
				this.eventHandler("unfreezeInputs");
				return;
			}

			// FIXME: This must be changed into an async loop!! (Right now it just uses the 1st applicable database then quits.)
			// However, it's only worth it to check the next database if it at least has the POTENTIAL to fill in missing fields.
			// This means further complicating Database definitions to specify beforehand which item fields they are able to fill.

			// IDEA: Perhaps each database, in addition to their singleQuery handler, could also have a getInfo handler that responds
			// with field names that the database is able to fill. (BUT WOULD THIS BE BETTER THAN LOCALLY DEFINING??? ANY ADDITIONAL USES FOR THE INFO RESPONSE????? ABORT THIS IDEA IF NOT.)

			// REMEMBER: The 1st database hit will usually be enough, ESPECIALLY for singleQueries.
			// POSTPONE FOR NOW!!! Come back to this after you have added MORE than just singleQuery to the databases structure.

			var async = false;
			var x;
			for( x in this.library.databases )
			{
				if( !this.library.databases[x].current.typeAliases.hasOwnProperty(item.type) )
					continue;

				var x;
				for( x in this.library.databases )
				{
					// If no item twin has been found yet, let's scout some more info and try again.
					var encodedItem = this.encodeRFC5987ValueChars(encodeURIComponent(JSON.stringify(item)));
					var request = new XMLHttpRequest();
					var requestURL = this.library.databases[x].current.singleQueryUri + "?item=" + encodedItem;//"http://metaverse.anarchyarcade.com/tubeinfo.php?id=" + this.extractYouTubeId(item.file);

					request.onreadystatechange = function()
					{
						if( request.readyState === 4 && request.status === 200 )
						{
							var response = JSON.parse(request.responseText);
							//console.log(response);
							var data = response.data;

							// Types returned by databases may be aliases!!
							if( data.hasOwnProperty("type") && !this.library.types.hasOwnProperty(data.type) )
							{
								// probably an alias at this point.
								var typeAliases = this.library.databases[x].typeAliases;

								// FIXME: Needs to be updated to tokenize aliases!!

								var aliases;
								var foundType = false;
								var y;
								for( y in typeAliases )
								{
									var aliases = this.tokenize(typeAliases[y].aliases);
									//if( typeAliases[y].aliases === data.type )
									if( aliases.indexOf(data.type) >= 0)
									{
										data.type = y;
										foundType = true;
										break;
									}
								}
//console.log("yarbles");
								// If no type match was found, replace it with the original type.
								if( !foundType )
									data.type = item.type;
							}

							var x;
							for( x in data )
							{
								if( item.hasOwnProperty(x) )
									item[x] = data[x];
							}
							
							onShouldCreate.call(this);
						}
						else if( request.status >= 500 )
						{
							onShouldCreate.call(this);
						}
					}.bind(this);

					request.open("GET", requestURL, true);
					request.send();

					async = true;
					break;
				}
			}

			if( !async )
				onShouldCreate.call(this);

			function onShouldCreate()
			{
				// Add the item.
				this.createLibraryEntry("Item", item, null, function(itemId)
				{
					if( !!!itemId )
					{
						if( !!this.error )
						{
							this.eventHandler("error", this.error);
							this.eventHandler("unfreezeInputs");
						}
						return;
					}

					this.showMenu("libraryItems");
				}.bind(this));
			}
		}.bind(this));
	}
	else if( actionName === "saveLibraryItemEdit" )
	{
		this.eventHandler("freezeInputs");

		var data = {};
		var x;
		for( x in actionData )
		{
			if( actionData[x].type !== "button" && actionData[x].type !== "submit" && x !== "id" )
				data[x] = actionData[x].value;
		}

		data.info = {"id": actionData["id"].value};

		this.updateLibraryEntry("Item", data, function(itemId)
		{
			if( !!!itemId )
			{
				if( !!this.error )
				{
					this.eventHandler("error", this.error);
					this.eventHandler("unfreezeInputs");
					return;
				}

				return;
			}

			this.showMenu("libraryItems");
		}.bind(this));
	}
	else if( actionName === "showLibraryTypes" )
	{
		this.showMenu("libraryTypes");
	}
	else if( actionName === "showLibraryDatabases" )
	{
		this.showMenu("libraryDatabases");
	}
	else if( actionName === "showLibraryApps" )
	{
		this.showMenu("libraryApps");
	}
	else if( actionName === "cancelLibraryTypes" )
	{
		this.showMenu("libraryMenu");
	}
	else if( actionName === "cancelLibraryDatabases" )
	{
		this.showMenu("libraryMenu");
	}
	else if( actionName === "cancelLibraryApps" )
	{
		this.showMenu("libraryMenu");
	}
	else if( actionName === "showCreateType" )
	{
		this.showMenu("libraryTypesCreate");
	}
	else if( actionName === "showCreateApp" )
	{
		this.showMenu("libraryAppsCreate");
	}
	else if( actionName === "cancelLibraryTypeCreate" )
	{
		this.showMenu("libraryTypes");
	}
	else if( actionName === "cancelLibraryAppCreate" )
	{
		this.showMenu("libraryApps");
	}
	else if( actionName === "createLibraryMap" )
	{
		this.eventHandler("freezeInputs");

		var data = {};
		var x;
		for( x in actionData )
		{
			if( !(actionData[x] instanceof HTMLElement) )
			{
				data[x] = {};

				var y;
				for( y in actionData[x] )
				{
					data[x][y] = {};

					var z;
					for( z in actionData[x][y] )
						data[x][y][z] = actionData[x][y][z].value;
				}
			}
			else if( actionData[x].type !== "button" && actionData[x].type !== "submit" )
				data[x] = actionData[x].value;
		}

		// data MUST have platforms object
		//if( !data.hasOwnProperty("platforms") )
			//data.platforms = {};

		this.createLibraryEntry("Map", data, null, function(mapId)
		{
			if( !!!mapId )
			{
				if( !!this.error )
				{
					this.eventHandler("error", this.error);
					this.eventHandler("unfreezeInputs");
					return;
				}

				return;
			}

			this.showMenu("libraryMaps");
		}.bind(this));
	}
	else if( actionName === "createLibraryInstance" )
	{
		this.eventHandler("freezeInputs");

		var data = {};
		var x;
		for( x in actionData )
		{
			if( !(actionData[x] instanceof HTMLElement) )
			{
				data[x] = {};

				var y;
				for( y in actionData[x] )
				{
					data[x][y] = {};

					var z;
					for( z in actionData[x][y] )
						data[x][y][z] = actionData[x][y][z].value;
				}
			}
			else if( actionData[x].type !== "button" && actionData[x].type !== "submit" )
				data[x] = actionData[x].value;
		}

		// data MUST have platforms object
		//if( !data.hasOwnProperty("platforms") )
			//data.platforms = {};

		this.createLibraryEntry("Instance", data, null, function(instanceId)
		{
			if( !!!instanceId )
			{
				if( !!this.error )
				{
					this.eventHandler("error", this.error);
					this.eventHandler("unfreezeInputs");
					return;
				}

				return;
			}

			this.showMenu("libraryInstances");
		}.bind(this));
	}
	else if( actionName === "createLibraryEntry" )
	{
		this.eventHandler("freezeInputs");

		var data = {};
		var x;
		for( x in actionData )
		{
			if( !(actionData[x] instanceof HTMLElement) )
			{
				data[x] = {};

				var y;
				for( y in actionData[x] )
				{
					data[x][y] = {};

					var z;
					for( z in actionData[x][y] )
						data[x][y][z] = actionData[x][y][z].value;
				}
			}
			else if( actionData[x].type !== "button" && actionData[x].type !== "submit" )
				data[x] = actionData[x].value;
		}

		// data MUST have platforms object
		//if( !data.hasOwnProperty("platforms") )
			//data.platforms = {};

		this.createLibraryEntry("Object", data, null, function(objectId)
		{
			if( !!!objectId )
			{
				if( !!this.error )
				{
					this.eventHandler("error", this.error);
					this.eventHandler("unfreezeInputs");
					return;
				}

				return;
			}

			this.showMenu("libraryObjects");
		}.bind(this));
	}
	else if( actionName === "createLibraryModel" )
	{
		this.eventHandler("freezeInputs");

		var data = {};
		var x;
		for( x in actionData )
		{
			if( !(actionData[x] instanceof HTMLElement) )
			{
				data[x] = {};

				var y;
				for( y in actionData[x] )
				{
					data[x][y] = {};

					var z;
					for( z in actionData[x][y] )
					{
						//console.log("Yarblesssss");
						//console.log(actionData);
						//console.log(z);
						data[x][y][z] = actionData[x][y][z].value;
					}
				}
			}
			else if( actionData[x].type !== "button" && actionData[x].type !== "submit" )
				data[x] = actionData[x].value;
		}

		// data MUST have platforms object
		//if( !data.hasOwnProperty("platforms") )
			//data.platforms = {};

		this.createLibraryEntry("Model", data, null, function(modelId)
		{
			if( !!!modelId )
			{
				if( !!this.error )
				{
					this.eventHandler("error", this.error);
					this.eventHandler("unfreezeInputs");
					return;
				}

				return;
			}

			this.showMenu("libraryModels");
		}.bind(this));
	}
	else if( actionName === "createLibraryApp" )
	{
		this.eventHandler("freezeInputs");

		var data = {};
		var x;
		for( x in actionData )
		{
			if( !(actionData[x] instanceof HTMLElement) )
			{
				data[x] = {};

				var y;
				for( y in actionData[x] )
				{
					data[x][y] = {};

					var z;
					for( z in actionData[x][y] )
						data[x][y][z] = actionData[x][y][z].value;
				}
			}
			else if( actionData[x].type !== "button" && actionData[x].type !== "submit" )
				data[x] = actionData[x].value;
		}

		// data MUST have filepaths object
		//if( !data.hasOwnProperty("filepaths") )
		//	data.filepaths = {};

		this.createLibraryEntry("App", data, null, function(appId)
		{
			if( !!!appId )
			{
				if( !!this.error )
				{
					this.eventHandler("error", this.error);
					this.eventHandler("unfreezeInputs");
					return;
				}

				return;
			}

			this.showMenu("libraryApps");
		}.bind(this));
	}
	else if( actionName === "createLibraryType" )
	{
		this.eventHandler("freezeInputs");

		var data = {};
		var x;
		for( x in actionData )
		{
			if( actionData[x].type !== "button" && actionData[x].type !== "submit" )
				data[x] = actionData[x].value;
		}

		this.createLibraryEntry("Type", data, null, function(typeId)
		{
			if( !!!typeId )
			{
				if( !!this.error )
				{
					this.eventHandler("error", this.error);
					this.eventHandler("unfreezeInputs");
					return;
				}

				return;
			}

			this.showMenu("libraryTypes");
		}.bind(this));
	}
	else if( actionName === "showEditType" )
	{
		this.showMenu("libraryTypesEdit", {"typeId": actionData["typeSelect"].options[actionData["typeSelect"].selectedIndex].value});
	}
	else if( actionName === "showEditInstance" )
	{
		this.showMenu("libraryInstancesEdit", {"instanceId": actionData["instanceSelect"].options[actionData["instanceSelect"].selectedIndex].value});
	}
	else if( actionName === "showEditObject" )
	{
		this.showMenu("libraryObjectsEdit", {"objectId": actionData["objectSelect"].options[actionData["objectSelect"].selectedIndex].value});
	}
	else if( actionName === "showEditMap" )
	{
		this.showMenu("libraryMapsEdit", {"mapId": actionData["mapSelect"].options[actionData["mapSelect"].selectedIndex].value});
	}
	else if( actionName === "showEditModel" )
	{
		this.showMenu("libraryModelsEdit", {"modelId": actionData["modelSelect"].options[actionData["modelSelect"].selectedIndex].value});
	}
	else if( actionName === "showEditApp" )
	{
		this.showMenu("libraryAppsEdit", {"appId": actionData["appSelect"].options[actionData["appSelect"].selectedIndex].value});
	}
	else if( actionName === "cancelLibraryTypeEdit" )
	{
		this.showMenu("libraryTypes");
	}
	else if( actionName === "cancelLibraryInstanceEdit" )
	{
		this.showMenu("libraryInstances");
	}
	else if( actionName === "cancelLibraryObjectEdit" )
	{
		this.showMenu("libraryObjects");
	}
	else if( actionName === "cancelLibraryMapEdit" )
	{
		this.showMenu("libraryMaps");
	}
	else if( actionName === "cancelLibraryModelEdit" )
	{
		this.showMenu("libraryModels");
	}
	else if( actionName === "cancelLibraryAppEdit" )
	{
		this.showMenu("libraryApps");
	}
	else if( actionName === "updateLibraryInstance" )
	{
		this.eventHandler("freezeInputs");

		var data = {};
		var x;
		for( x in actionData )
		{
			if( actionData[x].type !== "button" && actionData[x].type !== "submit" && x !== "id" )
			{
				if( actionData[x].type === undefined )
				{
					data[x] = {};

					var y;
					for( y in actionData[x] )
					{
						data[x][y] = {};
						var z;
						for( z in actionData[x][y] )
							data[x][y][z] = actionData[x][y][z].value;
					}
				}
				else
					data[x] = actionData[x].value;
			}
		}

		data.info = {"id": actionData["id"].value};

		// data MUST have platforms object
		//if( !data.hasOwnProperty("platforms") )
			//data.platforms = {};

		this.updateLibraryEntry("Instance", data, function(instanceId)
		{
			if( !!!instanceId )
			{
				if( !!this.error )
				{
					this.eventHandler("error", this.error);
					this.eventHandler("unfreezeInputs");
					return;
				}

				return;
			}

			this.showMenu("libraryInstances");
		}.bind(this));
	}
	else if( actionName === "updateLibraryEntry" )
	{
		this.eventHandler("freezeInputs");

		var data = {};
		var x;
		for( x in actionData )
		{
			if( actionData[x].type !== "button" && actionData[x].type !== "submit" && x !== "id" )
			{
				if( actionData[x].type === undefined )
				{
					data[x] = {};

					var y;
					for( y in actionData[x] )
					{
						data[x][y] = {};
						var z;
						for( z in actionData[x][y] )
							data[x][y][z] = actionData[x][y][z].value;
					}
				}
				else
					data[x] = actionData[x].value;
			}
		}

		data.info = {"id": actionData["id"].value};

		// data MUST have platforms object
		//if( !data.hasOwnProperty("platforms") )
			//data.platforms = {};

		this.updateLibraryEntry("Object", data, function(objectId)
		{
			if( !!!objectId )
			{
				if( !!this.error )
				{
					this.eventHandler("error", this.error);
					this.eventHandler("unfreezeInputs");
					return;
				}

				return;
			}

			this.showMenu("libraryObjects");
		}.bind(this));
	}
	else if( actionName === "updateLibraryMap" )
	{
		this.eventHandler("freezeInputs");

		var data = {};
		var x;
		for( x in actionData )
		{
			if( actionData[x].type !== "button" && actionData[x].type !== "submit" && x !== "id" )
			{
				if( actionData[x].type === undefined )
				{
					data[x] = {};

					var y;
					for( y in actionData[x] )
					{
						data[x][y] = {};
						var z;
						for( z in actionData[x][y] )
							data[x][y][z] = actionData[x][y][z].value;
					}
				}
				else
					data[x] = actionData[x].value;
			}
		}

		data.info = {"id": actionData["id"].value};

		// data MUST have platforms object
		//if( !data.hasOwnProperty("platforms") )
			//data.platforms = {};

		this.updateLibraryEntry("Map", data, function(mapId)
		{
			if( !!!mapId )
			{
				if( !!this.error )
				{
					this.eventHandler("error", this.error);
					this.eventHandler("unfreezeInputs");
					return;
				}

				return;
			}

			this.showMenu("libraryMaps");
		}.bind(this));
	}
	else if( actionName === "updateLibraryModel" )
	{
		this.eventHandler("freezeInputs");

		var data = {};
		var x;
		for( x in actionData )
		{
			if( actionData[x].type !== "button" && actionData[x].type !== "submit" && x !== "id" )
			{
				if( actionData[x].type === undefined )
				{
					data[x] = {};

					var y;
					for( y in actionData[x] )
					{
						data[x][y] = {};
						var z;
						for( z in actionData[x][y] )
							data[x][y][z] = actionData[x][y][z].value;
					}
				}
				else
					data[x] = actionData[x].value;
			}
		}

		data.info = {"id": actionData["id"].value};

		// data MUST have platforms object
		//if( !data.hasOwnProperty("platforms") )
			//data.platforms = {};

		this.updateLibraryEntry("Model", data, function(modelId)
		{
			if( !!!modelId )
			{
				if( !!this.error )
				{
					this.eventHandler("error", this.error);
					this.eventHandler("unfreezeInputs");
					return;
				}

				return;
			}

			this.showMenu("libraryModels");
		}.bind(this));
	}
	else if( actionName === "updateLibraryApp" )
	{
		this.eventHandler("freezeInputs");

		var data = {};
		var x;
		for( x in actionData )
		{
			if( actionData[x].type !== "button" && actionData[x].type !== "submit" && x !== "id" )
			{
				if( actionData[x].type === undefined )
				{
					data[x] = {};

					var y;
					for( y in actionData[x] )
					{
						data[x][y] = {};
						var z;
						for( z in actionData[x][y] )
							data[x][y][z] = actionData[x][y][z].value;
					}
				}
				else
					data[x] = actionData[x].value;
			}
		}

		data.info = {"id": actionData["id"].value};

		// data MUST have filepaths object
//		if( !data.hasOwnProperty("filepaths") )
//			data.filepaths = {};

		this.updateLibraryEntry("App", data, function(appId)
		{
			if( !!!appId )
			{
				if( !!this.error )
				{
					this.eventHandler("error", this.error);
					this.eventHandler("unfreezeInputs");
					return;
				}

				return;
			}

			this.showMenu("libraryApps");
		}.bind(this));
	}
	else if( actionName === "updateLibraryType" )
	{
		this.eventHandler("freezeInputs");

		var data = {};
		var x;
		for( x in actionData )
		{
			if( actionData[x].type !== "button" && actionData[x].type !== "submit" && x !== "id" )
				data[x] = actionData[x].value;
		}

		data.info = {"id": actionData["id"].value};

		this.updateLibraryEntry("Type", data, function(typeId)
		{
			if( !!!typeId )
			{
				if( !!this.error )
				{
					this.eventHandler("error", this.error);
					this.eventHandler("unfreezeInputs");
					return;
				}

				return;
			}

			this.showMenu("libraryTypes");
		}.bind(this));
	}
	else if( actionName === "showLibraryPlatforms" )
	{
		this.showMenu("libraryPlatforms");
	}
	else if( actionName === "cancelLibraryPlatforms" )
	{
		this.showMenu("libraryMenu");
	}
	else if( actionName === "cancelLibraryDatabaseCreate" )
	{
		this.showMenu("libraryDatabases");
	}
	else if( actionName === "cancelLibraryPlatformCreate" )
	{
		this.showMenu("libraryPlatforms");
	}
	else if( actionName === "cancelLibraryDatabaseEdit" )
	{
		this.showMenu("libraryDatabases");
	}
	else if( actionName === "cancelLibraryPlatformEdit" )
	{
		this.showMenu("libraryPlatforms");
	}
	else if( actionName === "showCreateDatabase" )
	{
		this.showMenu("libraryDatabasesCreate");
	}
	else if( actionName === "showCreatePlatform" )
	{
		this.showMenu("libraryPlatformsCreate");
	}
	else if( actionName === "showEditDatabase" )
	{
		this.showMenu("libraryDatabasesEdit", {"databaseId": actionData["databaseSelect"].options[actionData["databaseSelect"].selectedIndex].value});
	}
	else if( actionName === "showEditPlatform" )
	{
		this.showMenu("libraryPlatformsEdit", {"platformId": actionData["platformSelect"].options[actionData["platformSelect"].selectedIndex].value});
	}
	else if( actionName === "createLibraryDatabase" )
	{
		this.eventHandler("freezeInputs");

		var data = {};
		var x;
		for( x in actionData )
		{
			if( !(actionData[x] instanceof HTMLElement) )
			{
				data[x] = {};

				var y;
				for( y in actionData[x] )
				{
					data[x][y] = {};

					var z;
					for( z in actionData[x][y] )
						data[x][y][z] = actionData[x][y][z].value;
				}
			}
			else if( actionData[x].type !== "button" && actionData[x].type !== "submit" )
				data[x] = actionData[x].value;
		}

		this.createLibraryEntry("Database", data, null, function(databaseId)
		{
			if( !!!databaseId )
			{
				if( !!this.error )
				{
					this.eventHandler("error", this.error);
					this.eventHandler("unfreezeInputs");
					return;
				}

				return;
			}

			this.showMenu("libraryDatabases");
		}.bind(this));
	}
	else if( actionName === "createLibraryPlatform" )
	{
		this.eventHandler("freezeInputs");

		var data = {};
		var x;
		for( x in actionData )
		{
			if( actionData[x].type !== "button" && actionData[x].type !== "submit" )
				data[x] = actionData[x].value;
		}

		this.createLibraryEntry("Platform", data, null, function(platformId)
		{
			if( !!!platformId )
			{
				if( !!this.error )
				{
					this.eventHandler("error", this.error);
					this.eventHandler("unfreezeInputs");
					return;
				}

				return;
			}

			this.showMenu("libraryPlatforms");
		}.bind(this));
	}
	else if( actionName === "updateLibraryDatabase" )
	{
		this.eventHandler("freezeInputs");

		var data = {};
		var x;
		for( x in actionData )
		{
			if( actionData[x].type !== "button" && actionData[x].type !== "submit" && x !== "id" )
			{
				if( actionData[x].type === undefined )
				{
					data[x] = {};

					var y;
					for( y in actionData[x] )
					{
						data[x][y] = {};
						var z;
						for( z in actionData[x][y] )
							data[x][y][z] = actionData[x][y][z].value;
					}
				}
				else
					data[x] = actionData[x].value;
			}
		}

		data.info = {"id": actionData["id"].value};

		// data MUST have platforms object
		//if( !data.hasOwnProperty("platforms") )
			//data.platforms = {};

		this.updateLibraryEntry("Database", data, function(databaseId)
		{
			if( !!!databaseId )
			{
				if( !!this.error )
				{
					this.eventHandler("error", this.error);
					this.eventHandler("unfreezeInputs");
					return;
				}

				return;
			}

			this.showMenu("libraryDatabases");
		}.bind(this));
	}
	else if( actionName === "updateLibraryPlatform" )
	{
		this.eventHandler("freezeInputs");

		var data = {};
		var x;
		for( x in actionData )
		{
			if( actionData[x].type !== "button" && actionData[x].type !== "submit" && x !== "id" )
				data[x] = actionData[x].value;
		}

		data.info = {"id": actionData["id"].value};

		this.updateLibraryEntry("Platform", data, function(platformId)
		{
			if( !!!platformId )
			{
				if( !!this.error )
				{
					this.eventHandler("error", this.error);
					this.eventHandler("unfreezeInputs");
					return;
				}

				return;
			}

			this.showMenu("libraryPlatforms");
		}.bind(this));
	}
	*/
};

Metaverse.prototype.findType = function(title)
{
	var x, type;
	for( x in this.library.types )
	{
		type = this.library.types[x].current;
		if( type.title == title )
			return type;
	}

	return null;
};

// Based on: https://gist.github.com/mikelehen/3596a30bd69384624c11
Metaverse.prototype.generatePushId = function()
{
	var PUSH_CHARS = '-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz';

	var now = new Date().getTime();
	var duplicateTime = (now === this.lastPushTime);
	this.lastPushTime = now;

	var timeStampChars = new Array(8);
	for (var i = 7; i >= 0; i--) {
		timeStampChars[i] = PUSH_CHARS.charAt(now % 64);
		// NOTE: Can't use << here because javascript will convert to int and lose the upper bits.
		now = Math.floor(now / 64);
	}
	if (now !== 0) throw new Error('We should have converted the entire timestamp.');

	var id = timeStampChars.join('');

	if (!duplicateTime) {
		for (i = 0; i < 12; i++) {
			this.lastRandChars[i] = Math.floor(Math.random() * 64);
		}
	} else {
		// If the timestamp hasn't changed since last push, use the same random number, except incremented by 1.
		for (i = 11; i >= 0 && this.lastRandChars[i] === 63; i--) {
			this.lastRandChars[i] = 0;
		}
		this.lastRandChars[i]++;
	}
	for (i = 0; i < 12; i++) {
		id += PUSH_CHARS.charAt(this.lastRandChars[i]);
	}
	if(id.length != 20) throw new Error('Length should be 20.');

	return id;
};

Metaverse.prototype.reset = function()
{
	// Unregister change listeners
	var x;

	if( this.libraryRef )
	{
		/*
		this.libraryRef.child("items").off();
		for( x in this.library.items )
			this.libraryRef.child("items").child(this.library.items[x].current.info.id).child("current").off();

		this.libraryRef.child("models").off();
		for( x in this.library.models )
			this.libraryRef.child("models").child(this.library.models[x].current.info.id).child("current").off();
	
		this.libraryRef.child("types").off();
		for( x in this.library.types )
			this.libraryRef.child("types").child(this.library.types[x].current.info.id).child("current").off();

		this.libraryRef.child("databases").off();
		for( x in this.library.databases )
			this.libraryRef.child("databases").child(this.library.databases[x].current.info.id).child("current").off();

		this.libraryRef.child("platforms").off();
		for( x in this.library.platforms )
			this.libraryRef.child("platforms").child(this.library.platforms[x].current.info.id).child("current").off();

		this.libraryRef.child("maps").off();
		for( x in this.library.maps )
			this.libraryRef.child("maps").child(this.library.maps[x].current.info.id).child("current").off();

		// FIXME: Update how the cleanup is handled!!
		this.libraryRef.child("instances").off();
		for( x in this.library.instances )
			this.libraryRef.child("instances").child(this.library.instances[x].current.info.id).child("current").off();

		// FIXME: Update how the cleanup is handled!!
		this.libraryRef.child("objects").off();
		for( x in this.library.objects )
			this.libraryRef.child("objects").child(this.library.objects[x].current.info.id).child("current").off();
		*/
	}

	if( this.usersRef )
		this.usersRef.off();

	if( this.connectedRef )
		this.connectedRef.off();

	if( this.localUserRef )
		this.localUserRef.off();

	Firebase.goOffline();

	this.users = {};
	this.library = {
		"items": {},
		"models": {},
		"types": {},
		"apps": {},
		"databases": {},
		"platforms": {},
		"maps": {},
		"instances": {},
		"objects": {}
	};

	this.localUser = 
	{
		"id": "annon",
		"username": "annon",
		//"displayName": "Annon",
		"lastUpdate": {}
	};

	//this.freshInstanceObjects = {};

	this.root = null;
	this.universe = null;
	this.universeTitles = {};
	this.local = {};
	this.rootRef = null;
	this.universeRef = null;
	this.usersRef = null;
	this.localUserRef = null;
	this.connectedRef = null;
	
	this.setStatus("Offline");

	for( x in this.listeners.status )
		this.listeners.status[x](this.status);

	for( x in this.listeners.reset )
		this.listeners.reset[x](this.reset);

	Firebase.goOnline();
};

Metaverse.prototype.connect = function(server, callback)
{
	if( this.status !== "Offline" )
	{
		callback("ERROR: Not ready to connect.");
		return;
	}

	if( server !== "local" && !this.isUrl(server) )
	{
		callback("ERROR: Invalid server.");
		return;
	}

	this.root = server;
	if( this.root !== "local" )
	{
		this.rootRef = new Firebase(this.root);
		//this.setStatus("Connecting");
		this.setStatus("Select Universe");
		callback();

		// OBSOLETE STUFF UNDER HERE
		function getUniverseTitles()
		{
	console.log("mark 2");
			var request = new XMLHttpRequest();
			request.onreadystatechange = function()
			{
				if( request.readyState === 4 )
				{
					if( request.status === 200 )
					{
						var universeKeys = JSON.parse(request.responseText);

						if( !universeKeys )
							universeKeys = {};

						var objectKeys = Object.keys(universeKeys)
						var len = objectKeys.length;
						if( len > 0 )
						{
							var count = 0;
							this.rootRef.child(objectKeys[count]).child("info").child("title").once("value", function(snapshot)
							{
								count++;

								if( snapshot.exists() )
								{
									var val = snapshot.val();
									var key = snapshot.key();
									universeKeys[objectKeys[count-1]] = val;

									if( count < len )
										this.rootRef.child(objectKeys[count]).child("info").child("title").once("value", arguments.callee.bind(this));
								}

								if( count >= len )
									onGotUniverseTitles.call(this, universeKeys);
							}.bind(this));
						}
						else
							onGotUniverseTitles.call(this, universeKeys);

						function onGotUniverseTitles(universeKeys)
						{
							this.universeTitles = universeKeys;
							this.setStatus("Select Universe");

							callback();
						}
					}
					else if( request.status === 404 )
					{
						this.setStatus("Offline");

						callback("ERROR: Failed to connect.");
					}
				}
			}.bind(this);

	console.log("mark 1");
	console.log(this.root);
			request.open("GET", this.root + ".json?shallow=true", true);
			request.send(null);
		}

		//this.universeTitles = {};
		//getUniverseTitles.call(this);
	}
	else
	{
		/*
		this.local = {
			"info":
			{
				"owner": "12354151616753",
				"remover": "",
				"removed": "",
				"created": Firebase.ServerValue.TIMESTAMP
			}
		};
*/

/*
		this.local[this.universe] = {
			"servers": {},
			"users": {},
			"items": {},
			"models": {},
			"apps": {},
			"types": {}
		};
		*/
		this.setStatus("Select Universe");
	}
};

Metaverse.prototype.unbanUser = function(userId)
{
	this.universeInfoRef.child("banned").child(userId).set(null);
};

Metaverse.prototype.createUniverse = function(title, key, userBans, isPublic, callback)
{

	if( !this.validateData({"title": title}, this.defaultUniverse.info, callback) )
		return;

	var ref = (key === "") ? this.rootRef.push() : this.rootRef.child(key);
	var key = ref.key();
	var data = {
		"info":
		{
			"id": key,
			"isPublic": isPublic,
			"title": title,
			"owner": this.localUser.id,
			"admins": {},
			"banned": userBans,
			"restricted": {},
			"remover": "",
			"removed": 0,
			"created": Firebase.ServerValue.TIMESTAMP
		}
	};

	this.universeInfo = data.info;

	ref.set(data, function(error)
	{
		if( !!!error )
		{
			this.universeTitles[key] = data.info.name;
			callback(key);
		}
	}.bind(this));
};

Metaverse.prototype.banUser = function(userId)
{
	var data = {
		"id": userId,
		"date": Firebase.ServerValue.TIMESTAMP,
		"creator": this.localUser.id,
		"duration": 0,
		"reason": "BLT drive went AWOL."
	};

	this.universeInfoRef.child("banned").child(userId).set(data, function(e)
	{
		// finished adding user to ban list.

		// remove user from instanceUsers
		this.instanceUsersRef.child(userId).set(null, function()
		{
			// fin
		}.bind(this));
	}.bind(this));
};

Metaverse.prototype.setStatus = function(status)
{
	this.status = status;

	var x;
	for( x in this.listeners.status )
		this.listeners.status[x](this.status);
};

Metaverse.prototype.doesUniverseExist = function(universeKey, callback)
{
	this.fetchAllUniverseIds(function(universeIds)
	{
		callback(!!universeIds && !!universeIds[universeKey]);
	}.bind(this));
};

Metaverse.prototype.doesMapExist = function(universeKey, mapKey, callback)
{
	this.fetchAllMapIds(universeKey, function(mapIds)
	{
		callback(!!mapIds && !!mapIds[mapKey]);
	}.bind(this));
};

Metaverse.prototype.doesMapExist = function(universeKey, mapKey, callback)
{
	this.fetchAllMapIds(universeKey, function(mapIds)
	{
		callback(!!mapIds && !!mapIds[mapKey]);
	}.bind(this));
};

Metaverse.prototype.syncPano = function(panoId, data, callback)
{
	//var ref = (!!panoId && panoId !== "") ? this.instancePanosRef.child(panoId) : this.instancePanosRef.push();
	var ref = (!!panoId && panoId !== "") ? this.libraryRef.child("panos").child(panoId) : this.libraryRef.child("panos").push();
	var goodPanoId = ref.key();
	data.id = goodPanoId;
	ref.set(data, function()
	{
		this.instanceRef.child("panos").child(goodPanoId).set(this.localUser.id, function()
		{
			callback(goodPanoId);
		}.bind(this));
	}.bind(this));
};

Metaverse.prototype.joinUniverse = function(universeKey, userId, callback)
{
	//console.log("join universe called");
	// grab the universe's
	this.universe = universeKey;
	this.universeRef = this.rootRef.child(this.universe);
	this.universeInfoRef = this.universeRef.child("info");
	this.needsUniverseInfo = true;
	this.universeInfoRef.on("value", function(snapshot)
	{
		if( this.needsUniverseInfo )
		{
			this.needsUniverseInfo = false;

			var val = snapshot.val();
			this.universeInfo = val;

			if( !!this.universeInfo && !!this.universeInfo.banned && !!this.universeInfo.banned[userId] )
			{
				//console.log("Banish'ed! - Shakespeare");
				delete this.universeRef;

				if( typeof window.onNotAllowedToJoin === "function" )
					window.onNotAllowedToJoin(this.universeInfo.banned[userId]);

				return;
			}

			this.usersRef = this.universeRef.child("users");
			this.libraryRef = this.universeRef.child("library");

			this.library = {
				"items": {},
				"models": {},
				"types": {},
				"apps": {},
				"databases": {},
				"platforms": {},
				"maps": {},
				"instances": {},
				"objects": {}
			};

			this.setStatus("Universe Ready");

			if( !!callback )
				callback();	// TODO: might as well pass this the universe info as a param.
		}
		else
			this.onUniverseInfoChanged(snapshot);
	}.bind(this));
};

Metaverse.prototype.onUniverseInfoChanged = function(snapshot)
{
	var val = snapshot.val();
	this.universeInfo = val;

	//console.log(JSON.stringify(val));

	// cycle thru ban list
	/*
	var ban, bannedUser;
	for( var x in val.banned )
	{
		ban = val.banned[x];
		bannedUser = this.users[ban.id];

		if( !!bannedUser )
		{
			// will want to do post-mordum removal of all chat msgs from this user.
			// TODO: work
			// ...
		}
	}
	*/
};

Metaverse.prototype.fetchAllMapIds = function(universeId, callback)
{
	var infoRequest = new XMLHttpRequest();

	infoRequest.onreadystatechange = function() {
	  if (infoRequest.readyState == 4 && infoRequest.status == 200) {
	  	var mapIds = (!!infoRequest.responseText) ? JSON.parse(infoRequest.responseText) : [];
	  	callback(mapIds);
	  }
	}.bind(this);

	infoRequest.open("GET", "https://metaverse.firebaseio.com/" + universeId + "/library/maps/.json?shallow=true", true);
	infoRequest.send(null);
};

Metaverse.prototype.fetchAllModelIds = function(universeId, callback)
{
	var infoRequest = new XMLHttpRequest();

	infoRequest.onreadystatechange = function() {
	  if (infoRequest.readyState == 4 && infoRequest.status == 200) {
	  	var modelIds = (!!infoRequest.responseText) ? JSON.parse(infoRequest.responseText) : [];
	  	callback(modelIds);
	  }
	}.bind(this);

	infoRequest.open("GET", "https://metaverse.firebaseio.com/" + universeId + "/library/models/.json?shallow=true", true);
	infoRequest.send(null);
};

Metaverse.prototype.fetchAllUniverseIds = function(callback)
{
	var infoRequest = new XMLHttpRequest();

	infoRequest.onreadystatechange = function() {
	  if (infoRequest.readyState == 4 && infoRequest.status == 200) {
	  	var universeIds = (!!infoRequest.responseText) ? JSON.parse(infoRequest.responseText) : [];
	  	callback(universeIds);
	  }
	}.bind(this);

	infoRequest.open("GET", "https://metaverse.firebaseio.com/.json?shallow=true", true);
	infoRequest.send(null);
};

Metaverse.prototype.getAllChildren = function()
{
	/*
	if( g_mapInfoQueue.length === 0 )
	{
		var elems = document.getElementsByClassName("retrieveAnchorParent");

		var i;
		for( i = 0; i < elems.length; i++ )
		{
			elems[i].parentNode.removeChild(elems[i]);
		//	elems[i].innerHTML = "";
		}

		g_mapsSlateElem.style.display = "block";

		return;
	}
	*/

	//var entry = g_mapInfoQueue.pop();

	var infoRequest = new XMLHttpRequest();

	infoRequest.onreadystatechange = function() {
	  if (infoRequest.readyState == 4 && infoRequest.status == 200) {
	  	var children = JSON.parse(infoRequest.responseText);
	  	//console.log(mapObjects);

	  	// Count the objects and add them to the list.
	  	var count = 0;
	  	var x;
	  	for( x in children )
	  	{
//				  		if( mapObjects[x].hasOwnProperty("info") && mapObjects[x].info.hasOwnProperty("removed") && mapObjects[x].info.removed !== "" )
//				  			continue;
			this.libraryRef.child("instances/" + x + "/current").on("value", this.instanceChanged.bind(this));
			//this.libraryRef.child("instances/" + x + "/current").on("removed", this.instanceRemoved.bind(this));

	  		count++;
	  	}

	  	console.log("number of children: " + count);
/*
	  	var text = document.createTextNode(" (" + count + ")");
	  	entry.anchor.appendChild(text);
	  	entry.anchor.objectCount = count;

	  	// Now place it where it needs to be.
	  	var parent = entry.anchor.parentNode;
	  	var elems = document.getElementsByClassName("mapAnchor");
	  	var foundOrder = false;
	  	var i;
	  	for( i = 0; i < elems.length; i++ )
	  	{
	  		if( elems[i] === entry.anchor )
	  			continue;

	  		if( elems[i].hasOwnProperty("objectCount") && elems[i].objectCount <= count )
	  		{
	  			parent.removeChild(entry.anchor);
	  			parent.insertBefore(entry.anchor, elems[i]);
	  			foundOrder = true;
	  			break;
	  		}
	  	}

	  	if( !foundOrder )
	  	{
	  		//parent.removeChild(entry.anchor);
	  		parent.insertBefore(entry.anchor, parent.childNodes[0]);
	  	}

	  	GetNextMapInfo(true);
	    //text.innerText += mapInfoRequest.responseText;
	    */
	  }
	}.bind(this);

	//mapInfoRequest.open("GET", "https://phantomplayermmo.firebaseio.com/maps/" + entry.mapName + "/objects/.json?shallow=true", true);

	infoRequest.open("GET", "https://metaverse.firebaseio.com/" + this.universe + "/library/instances/.json?shallow=true", true);
	infoRequest.send(null);
};
/*
Metaverse.prototype.userAdded = function(child, prevChildKey)
{
	var key = child.key();
	console.log("Downloaded metaverse information for user " + key);
	this.users[key] = child.val();
};*/

Metaverse.prototype.userRemoved = function(child)
{
	console.log("User removed.");
	delete this.users[child.key()];
};
/*
Metaverse.prototype.itemAdded = function(child, prevChildKey)
{
	var key = child.key();
	console.log("Downloaded metaverse information for item " + key);
	this.library.items[key] = child.val();
	this.libraryRef.child("items").child(key).child("current").on("value", this.itemChanged.bind(this));
};
*/
Metaverse.prototype.itemRemoved = function(child)
{
	console.log("Item removed.");
	delete this.library.items[child.key()];
};

/*
// NOTE: Most places that call this method are never executed & call it invalidly.  TODO: Fix that.
Metaverse.prototype.itemChanged = function(snapshot)
{
	var val = snapshot.val();
	var key = val.info.id;

	// Determine which fields have changed...
	var fields = {};
	var oldData = this.library.items[key].current;
	for( var x in this.defaultObject )
	{
		if( x === "info" )
			continue;

		if( oldData[x] !== val[x] )
			fields[x] = val[x];
	}

	this.library.items[key].current = val;

	// Note that the 2nd phantom update is the timestamp updating, and can safely be from here.
	if( typeof window.onItemChanged === "function" )
		window.onItemChanged(val, fields);
};
*/
Metaverse.prototype.onEntryFetched = function(snapshot, category, id)
{
	var data = (!!snapshot) ? snapshot.val() : undefined;

	if( !!!data )
	{
		// If this is a NEW entry, its snapshot will be null.
		// If this ID is in the fetch list, then we have an initial callback waiting for it.
		if( !!this.fetching[id] )
		{
			// Calling initial callback w/ a miss...
			var callback = this.fetching[id];
			delete this.fetching[id];
			//this.libraryRef.child(category).child(id).child("current").off();
			callback();
		}

		return;
	}

	if( !!!category || !!!id )
	{
		console.log("WARNING: Ignoring invalid entry changed callback.");
		return;
	}

	if( !!this.library[category][id] )
	{

		//else
		//{
			// This entry already existed, so we must determine which
			// fields have changed and sent them to AArcade.
			// TODO: work
			// ...
console.log("UNFINISHED CODE BLOCK ACCESSED! TELL SMITH LARZO!");
			// Aaannnnd theeeennnnnn:
			console.log("Updated existing entry.");
			this.library[category][id].current = data;
		//}
	}
	else if( !!this.fetching[id] )
	{
		// If this ID is in the fetch list, then we have an initial callback waiting for it.

		//console.log("Adding NEW entry to cache.");
		// This is a NEW entry and must be added to the library as a fresh entry.
		this.library[category][id] = {"current": data};

		//this.libraryRef.child(category).child(id).child("current").off();

		//console.log("Calling initial callback...");
		var callback = this.fetching[id];
		delete this.fetching[id];

		callback(data);//this.library[category][id].current);
	}
};

Metaverse.prototype.fetchEntry = function(type, id, callback)
{
	var category = type.toLowerCase() + "s";

	// 1st, check if it's already cached
	var entryData = (!!this.library[category] && !!this.library[category][id] && !!this.library[category][id].current ) ? this.library[category][id].current : undefined;
	if( !!entryData )
	{
		//console.log("Entry found in cache.");
		callback(entryData);
	}
	else
	{
		//console.log("Fetching entry from remote server...");

		// otherwise, fetch it
		this.fetching[id] = callback;
		this.libraryRef.child(category).child(id).child("current").once("value", function(snapshot)
		{
			this.onEntryFetched.call(this, snapshot, category, id);
		}.bind(this));
	}
	/*
	var key = child.key();
	console.log("Downloaded metaverse information for model " + key);

	// Make sure the model has all required fields.
	var val = child.val();
	var x;
	for( x in this.defaultModel )
	{
		if( !val.current.hasOwnProperty(x) )
		{
			// Assume it is an empty container object.
			val.current[x] = {};
		}
	}

	this.library.models[key] = val;
	this.libraryRef.child("models").child(key).child("current").on("value", this.modelChanged.bind(this));
	*/
};

Metaverse.prototype.modelAdded = function(child, prevChildKey)
{
	var key = child.key();
	console.log("Downloaded metaverse information for model " + key);

	// Make sure the model has all required fields.
	var val = child.val();
	var x;
	for( x in this.defaultModel )
	{
		if( !val.current.hasOwnProperty(x) )
		{
			// Assume it is an empty container object.
			val.current[x] = {};
		}
	}

	this.library.models[key] = val;
	this.libraryRef.child("models").child(key).child("current").on("value", this.modelChanged.bind(this));
};

Metaverse.prototype.modelRemoved = function(child)
{
	console.log("Model removed.");
	delete this.library.models[child.key()];
};
/*
Metaverse.prototype.modelChanged = function(child, prevChildKey)
{
	var val = child.val();

	// Determine which fields have changed...
	var fields = {};
	var oldData = this.library.models[val.info.id].current;
	for( var x in this.defaultModel )
	{
		if( x === "info" )
			continue;

		if( oldData[x] !== val[x] )
			fields[x] = val[x];
	}

	this.library.models[val.info.id].current = val;

	// Note that the 2nd phantom update is the timestamp updating, and can safely be from here.
	if( typeof window.onModelChanged === "function" )
		window.onModelChanged(val, fields);
};
*/
/*
Metaverse.prototype.typeAdded = function(child, prevChildKey)
{
	var key = child.key();
	console.log("Downloaded metaverse information for type " + key);

	this.library.types[key] = child.val();
	this.libraryRef.child("types").child(key).child("current").on("value", this.typeChanged.bind(this));
};
*/
/*
Metaverse.prototype.typeRemoved = function(child)
{
	console.log("Type removed.");
	delete this.library.types[child.key()];
};*/
/*
Metaverse.prototype.appAdded = function(child, prevChildKey)
{
	var key = child.key();
	console.log("Downloaded metaverse information for app " + key);

	// Make sure the app has all required fields.
	var val = child.val();
	var x;
	for( x in this.defaultApp )
	{
		if( !val.current.hasOwnProperty(x) )
		{
			// Assume it is an empty container object.
			val.current[x] = {};
		}
	}

	this.library.apps[key] = val;
	this.libraryRef.child("apps").child(key).child("current").on("value", this.appChanged.bind(this));
};
*/
Metaverse.prototype.appRemoved = function(child)
{
	console.log("App removed.");
	delete this.library.apps[child.key()];
};
/*
Metaverse.prototype.appChanged = function(snapshot)
{
	var val = snapshot.val();

	// Determine which fields have changed...
	var fields = {};
	var oldData = this.library.apps[val.info.id].current;
	for( var x in this.defaultObject )
	{
		if( x === "info" )
			continue;

		if( oldData[x] !== val[x] )
			fields[x] = val[x];
	}

	this.library.apps[val.info.id].current = val;

	// Note that the 2nd phantom update is the timestamp updating, and can safely be from here.
	if( typeof window.onAppChanged === "function" )
		window.onAppChanged(val, fields);
};*/

Metaverse.prototype.entryChanged = function(mode, snapshot)
{
	if( !snapshot.exists() )
		return;
	
	//console.log(mode);
	var category = mode.toLowerCase() + "s";

	var val = snapshot.val();
	//console.log(JSON.stringify(val));
	var key = val.info.id;

	if( !!val.info.removed && val.info.removed !== "" )
	{
		// This is a REMOVAL
		if( category === "objects" )
		{
			var oldData = val;
			if( !!oldData )
			{
				// Remove model listeners, if needed.
				var oldModel = (!!this.library.models[oldData.model]) ? this.library.models[oldData.model].current : undefined;
				if( !!oldModel )
					this.modelBookkeepingRemove.call(this, key, oldData.model);

				// Remove item listeners, if needed.
				var oldItem = (!!this.library.items[oldData.item]) ? this.library.items[oldData.item].current : undefined;
				if( !!oldItem )
					this.itemBookkeepingRemove.call(this, key, oldData.item);

				// remove the object from the cache
				console.log("Removed object " + key + " from the cache.");
				this.libraryRef.child("objects").child(key).child("current").off();
				delete this.library.objects[key];

				if( typeof window.onEntryRemoved === "function" )
					window.onEntryRemoved(mode, val);
			}
		}
	}
	else
	{
		// Determine which fields have changed
		var isNewEntry = false;
		var fields = {};
		var oldData = (!!this.library[category][val.info.id]) ? this.library[category][val.info.id].current : {};

		//if( category !== "objects" )
		//{
			if( !!this.pendingInstanceEntries[key] )
				oldData = (!!this.pendingInstanceEntries[key].current) ? this.pendingInstanceEntries[key].current : this.pendingInstanceEntries[key];	// this is just {} set earlier
			else if( mode === "Object" && typeof window.isOwnInitialObject === "function" && window.isOwnInitialObject(key) )
				oldData = {};

		// Determine if this is a local player's entry
//		var isOwnInitialEntry = (mode === "Object" && typeof window.isOwnInitialObject === "function") ? window.isOwnInitialObject(key) : false;

			//console.log(JSON.stringify(oldData));
		//}
		//else if( category === "items" )
		//{
		//	if( !!this.pendingInstanceEntries[key] )
		//		oldData = (!!this.pendingInstanceEntries[key].current) ? this.pendingInstanceEntries[key].current : this.pendingInstanceEntries[key];	// this is just {} set earlier
		//}

		

		// Determine if this is a NEW entry, or just an update.
		if( Object.keys(oldData).length === 0 )
			isNewEntry = true;
//console.log(mode);
//console.log(JSON.stringify(this["default" + mode]));
		for( var x in this["default" + mode] )
		{
			if( x === "info" )
				continue;

			// Skip "filepaths" & "platforms" for now.
			// TODO: Make this recursive to support nested keys.
			if( x === "filepaths" || x === "platforms" )
				continue;

			// "Type" stuff is a special case for a lazy fix to Types not having all the fields they should have when they are passed to us from aaapi.
			//isOwnInitialEntry || (
			if( oldData[x] != val[x] && (mode !== "Type" || oldData.hasOwnProperty(x)))
			{
				//console.log(oldData[x] + " : " + val[x]);
				fields[x] = val[x];
			}
		}

		if( !!fields.model )
		{
			if( this.verbose )
				console.log("Model has changed from " + oldData.model + " to " + fields.model);

			// Remove old model listeners, if needed.
			var oldModel = (!!this.library.models[oldData.model]) ? this.library.models[oldData.model].current : undefined;
			if( !!oldModel )
				this.modelBookkeepingRemove.call(this, key, oldData.model);

			// Add new model listeners
			//var model = (!!this.library.models[fields.model]) ? this.library.models[fields.model].current : undefined;
			//if( !!model )
			if( fields.model !== "" )
				this.modelBookkeepingAdd.call(this, key, fields.model);
		}

		if( !!fields.item )
		{
			if( this.verbose )
				console.log("Item has changed from " + oldData.item + " to " + fields.item);

			// Remove old item listeners, if needed.
			var oldItem = (!!this.library.items[oldData.item]) ? this.library.items[oldData.item].current : undefined;
			if( !!oldItem )
				this.itemBookkeepingRemove.call(this, key, oldData.item);

			// Add new item listeners, if needed.
			//var item = (!!this.library.items[fields.item]) ? this.library.items[fields.item].current : undefined;
			//if( !!item )
			if( fields.item !== "" )
				this.itemBookkeepingAdd.call(this, key, fields.item);
		}

		if( !!fields.type )
		{
			if( this.verbose )
				console.log("Type has changed from " + oldData.type + " to " + fields.type);

			// Remove old type listeners, if needed.
			var oldType = (!!this.library.types[oldData.type]) ? this.library.types[oldData.type].current : undefined;
			if( !!oldType )
				this.typeBookkeepingRemove.call(this, key, oldData.type);

			// Add new item listeners, if needed.
			var type = (!!this.library.types[fields.type]) ? this.library.types[fields.type].current : undefined;
			//if( !!type )
			if( type !== "" )
				this.typeBookkeepingAdd.call(this, key, fields.type);
		}

		if( !!fields.app )
		{
			if( this.verbose )
				console.log("App has changed from " + oldData.app + " to " + fields.app);

			// Remove old app listeners, if needed.
			var oldApp = (!!this.library.apps[oldData.app]) ? this.library.apps[oldData.app].current : undefined;
			if( !!oldApp )
				this.appBookkeepingRemove.call(this, key, oldData.app);

			// Add new app listeners, if needed.
			var app = (!!this.library.apps[fields.app]) ? this.library.apps[fields.app].current : undefined;
			if( !!app )
				this.appBookkeepingAdd.call(this, key, fields.app);
		}

		if( !!!this.library[category][val.info.id] )
			this.library[category][val.info.id] = {};

		this.library[category][val.info.id].current = val;

		if( !!this.pendingInstanceEntries[key] )
			delete this.pendingInstanceEntries[key];

		//if( category === "apps" )
		//{
			// Note that the 2nd phantom update is the timestamp updating, and can safely be from here.
		//	if( typeof window.onAppChanged === "function" )
		//		window.onAppChanged(val, fields, isNewEntry);
		//}
		//else
		//{
			// Note that the 2nd phantom update is the timestamp updating, and can safely be from here.
			if( typeof window.onEntryChanged === "function" )
				window.onEntryChanged(mode, val, fields, isNewEntry);
		//}
	}
};

Metaverse.prototype.databaseAdded = function(child, prevChildKey)
{
	var key = child.key();
	console.log("Downloaded metaverse information for database " + key);

	// Make sure the database has all required fields.
	var val = child.val();
	var x;
	for( x in this.defaultDatabase )
	{
		if( !val.current.hasOwnProperty(x) )
		{
			// Assume it is an empty container object.
			val.current[x] = {};
		}
	}

	this.library.databases[key] = val;
	this.libraryRef.child("databases").child(key).child("current").on("value", this.databaseChanged.bind(this));
};

Metaverse.prototype.databaseRemoved = function(child)
{
	console.log("Database removed.");
	delete this.library.databases[child.key()];
};

Metaverse.prototype.databaseChanged = function(child, prevChildKey)
{
	var val = child.val();

	// Make sure the database has all required fields.
	var x;
	for( x in this.defaultDatabase )
	{
		if( !val.hasOwnProperty(x) )
		{
			// Assume it is an empty container object.
			val[x] = {};
		}
	}
	
	this.library.databases[val.info.id].current = val;
};

Metaverse.prototype.platformAdded = function(child, prevChildKey)
{
	var key = child.key();
	console.log("Downloaded metaverse information for platform " + key);

	this.library.platforms[key] = child.val();
	this.libraryRef.child("platforms").child(key).child("current").on("value", this.platformChanged.bind(this));
};

Metaverse.prototype.platformRemoved = function(child)
{
	console.log("Platform removed.");
	delete this.library.platforms[child.key()];
};

Metaverse.prototype.platformChanged = function(child, prevChildKey)
{
	var val = child.val();
	this.library.platforms[val.info.id].current = val;
};

Metaverse.prototype.instanceChanged = function(child, prevChildKey)
{
	var val = {
		"current": child.val()
	}

	if( !val.current )
		return;	// If there is no val, then that means the child_removed handler will handle it.

	var key = val.current.info.id;

	// Make sure the instance has all required fields.
	var x;
	for( x in this.defaultInstance )
	{
		if( !val.current.hasOwnProperty(x) )
		{
			// Assume it is an empty container object.
			val.current[x] = {};
		}
	}
//this.library.instances[instanceId]
	this.library.instances[key] = val;

//console.log(JSON.stringify(val));
	if( typeof window.onInstanceChanged === "function" )
		window.onInstanceChanged(key);
};

Metaverse.prototype.instanceRemoved = function(child)
{
	console.log("Instance removed.");
	delete this.library.instances[child.key()];
};

Metaverse.prototype.objectRemoved = function(child)
{
	console.log("Object removed.");
	delete this.library.objects[child.key()];
};
/*
Metaverse.prototype.instanceChanged = function(child, prevChildKey)
{
	var val = child.val();
//console.log(JSON.stringify(child);
	// Make sure the instance has all required fields.
	var x;
	for( x in this.defaultInstance )
	{
		if( !val.hasOwnProperty(x) )
		{
			// Assume it is an empty container object.
			val[x] = {};
		}
	}

	this.library.instances[val.info.id].current = val;
};
*/

Metaverse.prototype.instanceUserAdded = function(child, prevChildKey)
{
	var sessionId = child.val();
	var userId = child.key();

	//if( userId === this.localUser.id )
	//{
	//	this.localUser.session =
	//	return;
	//}
//console.log(userId + " vs " + this.localUser.id);
	if( userId === this.localUser.id )
	{
		//console.log(sessionId);
		this.usersRef.child(userId).child("sessions").child(sessionId).once("value", function(snapshot)
		{
			this.userSessionUpdated.call(this, userId, snapshot);
		}.bind(this));
	}
	else
	{
		if( !!this.users[userId] )
		{
			console.log("ERROR: instanceUserAdded received for a user that already exists!");
			return;
		}

		this.usersRef.child(userId).child("sessions").child(sessionId).on("value", function(snapshot)
		{
			this.userSessionUpdated.call(this, userId, snapshot);
		}.bind(this));
	}
};

Metaverse.prototype.instanceUserRemoved = function(child, prevChildKey)
{
	var val = child.val();
	var key = child.key();

	if( key === this.localUser.id )
	{
		// we have been kicked and/or banned
		//console.log("Kick'eshed! - Shakespeare");
		delete this.universeRef;

		if( typeof window.onKicked === "function" )
			window.onKicked(this.localUser);
	}
	else if( !!this.users[key] )
	{
		// remove listeners
		this.usersRef.child(key).child("sessions").child(val).off();

		var victim = this.users[key];
		delete this.users[key];

		if( typeof window.onInstanceUserRemoved === "function" )
			window.onInstanceUserRemoved(victim);
	}
};

Metaverse.prototype.fetchPano = function(panoInfo, callback)
{
	//console.log(JSON.stringify(panoInfo));
	this.libraryRef.child("panos").child(panoInfo.id).once("value", function(snapshot)
	{
		var key = snapshot.key();
		this.panos[key] = snapshot.val();
		callback(this.panos[key]);
	}.bind(this));
};

Metaverse.prototype.syncOverview = function(instanceId, data, callback)
{
	var ref = this.libraryRef.child("instances").child(instanceId).child("overview");
	ref.set(data, function()
	{
		callback();
	}.bind(this));
};

Metaverse.prototype.instancePanoAdded = function(child, prevChildKey)
{
	var panoId = child.key();
	var userId = child.val();

	if( !!this.panos[panoId] )
	{
		console.log("ERROR: instancePanosAdded received for a pano that already exists!");
		return;
	}

	//if( typeof window.onPanoAdded === "function" )
	//	window.onPanoAdded(panoId);

	this.libraryRef.child("panos").child(panoId).child("body").once("value", function(snapshot)
	{
		this.panos[panoId] = {"id": panoId, "body": snapshot.val()};

		if( typeof window.onPanoAdded === "function" )
			window.onPanoAdded(this.panos[panoId]);
	}.bind(this));
};

Metaverse.prototype.instancePanoRemoved = function(child, prevChildKey)
{
	/*
	var val = child.val();
	var key = child.key();

	if( key === this.localUser.id )
	{
		// we have been kicked and/or banned
		console.log("Kick'eshed! - Shakespeare");
		delete this.universeRef;

		if( typeof window.onKicked === "function" )
			window.onKicked(this.localUser);
	}
	else if( !!this.users[key] )
	{
		// remove listeners
		this.usersRef.child(key).child("sessions").child(val).off();

		var victim = this.users[key];
		delete this.users[key];

		if( typeof window.onInstanceUserRemoved === "function" )
			window.onInstanceUserRemoved(victim);
	}*/
};

Metaverse.prototype.instanceObjectAdded = function(child, prevChildKey)
{
	var key = child.key();

	var instanceObjectRef = this.libraryRef.child("objects").child(key).child("current");

	if( this.verbose )
		console.log("Adding object listener for " + key);

	instanceObjectRef.on("value", function(snapshot)
	{
		this.entryChanged.call(this, "Object", snapshot);
	}.bind(this));
};

Metaverse.prototype.appBookkeepingAdd = function(key, appId)
{
	// Add new app type listeners, if needed.
	var app = this.library.apps[appId].current;
	//console.log(JSON.stringify(app.type));
	//var appType = (!!this.library.types[app.type]) ? this.library.types[app.type].current : undefined;
	if( !!app.type && app.type !== "" )
		this.typeBookkeepingAdd.call(this, appId, app.type);

	if( !!!this.objectListeners[appId] )
	{
		// If this is the first listener, add a Firebase listener.
		this.objectListeners[appId] = {};
		this.libraryRef.child("apps").child(appId).child("current").on("value", function(snapshot)
		{
			this.entryChanged.call(this, "App", snapshot);
		}.bind(this));

		if( this.verbose )
			console.log("Adding app listener for " + appId);
	}
	this.objectListeners[appId][key] = true;
};

Metaverse.prototype.typeBookkeepingAdd = function(key, typeId)
{
	//console.log(JSON.stringify(this.objectListeners[typeId]));
	if( !!!this.objectListeners[typeId] )
	{
		// If this is the first listener, add a Firebase listener.
		this.objectListeners[typeId] = {};
		this.libraryRef.child("types").child(typeId).child("current").on("value", function(snapshot)
		{
			this.entryChanged.call(this, "Type", snapshot);
		}.bind(this));

		if( this.verbose )
			console.log("Adding type listener for " + typeId);
	}
	this.objectListeners[typeId][key] = true;
};

Metaverse.prototype.itemBookkeepingAdd = function(key, itemId)
{
	// Add new item type listeners, if needed.
	var item = (!!this.library.items[itemId]) ? this.library.items[itemId].current : undefined;
	if( !!item )
	{
		var type = (!!this.library.types[item.type]) ? this.library.types[item.type].current : undefined;
		if( !!type )
			this.typeBookkeepingAdd.call(this, itemId, item.type);

		// Add new app listeners, if needed.
		var app = (!!this.library.apps[item.app]) ? this.library.apps[item.app].current : undefined;
		if( !!app )
			this.appBookkeepingAdd.call(this, itemId, item.app);
	}

	if( !!!this.objectListeners[itemId] )
	{
		// If this is the first listener, add a Firebase listener.
		this.objectListeners[itemId] = {};
		this.libraryRef.child("items").child(itemId).child("current").on("value", function(snapshot)
		{
			this.entryChanged.call(this, "Item", snapshot);
		}.bind(this));

		if( this.verbose )
			console.log("Adding item listener for " + itemId);
	}
	this.objectListeners[itemId][key] = true;
};

Metaverse.prototype.modelBookkeepingAdd = function(key, modelId)
{
	if( !!!this.objectListeners[modelId] )
	{
		// If this is the first listener, add a Firebase listener.
		this.objectListeners[modelId] = {};
		this.libraryRef.child("models").child(modelId).child("current").on("value", function(snapshot)
		{
			this.entryChanged.call(this, "Model", snapshot);
		}.bind(this));

		if( this.verbose )
			console.log("Adding model listener for " + modelId);
	}
	this.objectListeners[modelId][key] = true;
};

Metaverse.prototype.appBookkeepingRemove = function(key, appId)
{
	// remove the app's listener for the item.
	delete this.objectListeners[appId][key];

	// if that was the last listener on this app, remove the appType listener too.
	if( Object.keys(this.objectListeners[appId]).length === 0 )
	{
		delete this.objectListeners[appId];

		// only if there was an oldAppType
		var oldApp = this.library.apps[appId].current;
		var oldAppType = (!!this.library.types[oldApp.type]) ? this.library.types[oldApp.type].current : undefined;
		if( !!oldAppType )
			this.typeBookkeepingRemove.call(this, appId, oldApp.type);

		// remove the item from the cache
		delete this.library.apps[appId];
		this.libraryRef.child("apps").child(appId).child("current").off();
		console.log("Removed app " + appId + " from the cache.");
	}
};

Metaverse.prototype.typeBookkeepingRemove = function(key, typeId)
{
	// remove the type's listener for the old item.
	delete this.objectListeners[typeId][key];

	// if that was the last listener for this type, remove it from the cache.
	if( Object.keys(this.objectListeners[typeId]).length === 0 )
	{
		delete this.objectListeners[typeId];

		// remove the type from the cache
		delete this.library.types[typeId];
		this.libraryRef.child("types").child(typeId).child("current").off();
		console.log("Removed type " + typeId + " from the cache.");
	}
};

Metaverse.prototype.modelBookkeepingRemove = function(key, modelId)
{
	// remove the model's listener for the object.
	if(!!this.objectListeners[modelId])
	{
		delete this.objectListeners[modelId][key];

		// if that was the last listener for this type, remove it from the cache.
		if( Object.keys(this.objectListeners[modelId]).length === 0 )
		{
			delete this.objectListeners[modelId];

			// remove the model from the cache
			delete this.library.models[modelId];
			this.libraryRef.child("models").child(modelId).child("current").off();
			console.log("Removed model " + modelId + " from the cache.");
		}
	}
};

Metaverse.prototype.itemBookkeepingRemove = function(key, itemId)
{
	// remove the item's listener for the object.
	if( !!this.objectListeners[itemId] )
	{
		delete this.objectListeners[itemId][key];

		// if that was the last listener for this type, remove it from the cache.
		if( Object.keys(this.objectListeners[itemId]).length === 0 )
		{
			delete this.objectListeners[itemId];

			// Remove old type listeners, if needed.
			var oldItem = this.library.items[itemId].current;
			var oldItemType = (!!this.library.types[oldItem.type]) ? this.library.types[oldItem.type].current : undefined;
			if( !!oldItemType )
				this.typeBookkeepingRemove.call(this, itemId, oldItem.type);

			// Remove old app listeners, if needed.
			var oldApp = (!!this.library.apps[oldItem.app]) ? this.library.apps[oldItem.app].current : undefined;
			if( !!oldApp )
				this.appBookkeepingRemove.call(this, itemId, oldItem.app);

			// remove the item from the cache
			delete this.library.items[itemId];
			this.libraryRef.child("items").child(itemId).child("current").off();					
			console.log("Removed item " + itemId + " from the cache.");
		}
	}
};

Metaverse.prototype.userSessionUpdated = function(userId, snapshot)
{
	var session = snapshot.val();
	var sessionId = snapshot.key();

	if( this.localUser.id === userId )
	{
		if( typeof window.onInstanceUserAdded === "function" )
			window.onInstanceUserAdded(userId, session, true);
	}
	else if( !!session )
	{
		var oldSession;
		if( !!!this.users[userId] || !!!this.users[userId].session )
		{
			this.users[userId] = {
				"id": userId,
				"session": session
			};

			if( typeof window.onInstanceUserAdded === "function" )
				window.onInstanceUserAdded(userId, session, false);
		}
		else
		{
			oldSession = this.users[userId].session;
			this.users[userId].session = session;
		}


		// This way only the minimal amount of logic needs to be done from here-on-in.
		var fields = {};
		var testVal = (!!session.instance) ? session.instance : this.defaultSession.instance;
		if( !!!oldSession || oldSession.instance != testVal )
			fields.instance = testVal;

		var testVal = (!!session.displayName) ? session.displayName : this.defaultSession.displayName;
		if( !!!oldSession || oldSession.displayName != testVal )
			fields.displayName = testVal;

		// IF there is nothing in the old one
		// OR if the old one does not match the new one

		var testVal = (!!session.item) ? session.item.id : this.defaultSession.item.id;
		if( !!!oldSession || !!!oldSession.item || oldSession.item.id != testVal )
			fields.item = {"id": testVal};

		var testVal = (!!session.object) ? session.object.id : this.defaultSession.object.id;
		if( !!!oldSession || !!!oldSession.object || oldSession.object.id != testVal )
			fields.object = {"id": testVal};

		var testVal = (!!session.say) ? session.say.text : this.defaultSession.say.text;
		if( !!!oldSession || !!!oldSession.say || oldSession.say.text != testVal )
			fields.say = {"text": testVal};

		var testBodyOriginVal = (!!session.transform && !!session.transform.body) ? session.transform.body.origin : this.defaultSession.transform.body.origin;
		var testBodyAnglesVal = (!!session.transform && !!session.transform.body) ? session.transform.body.angles : this.defaultSession.transform.body.angles;
		var testHeadOriginVal = (!!session.transform && !!session.transform.head) ? session.transform.head.origin : this.defaultSession.transform.head.origin;
		var testHeadAnglesVal = (!!session.transform && !!session.transform.head) ? session.transform.head.angles : this.defaultSession.transform.head.angles;
		if( !!!oldSession || !!!oldSession.transform || !!!oldSession.transform.body || !!!oldSession.transform.head || oldSession.transform.body.origin != testBodyOriginVal || oldSession.transform.body.angles != testBodyAnglesVal || oldSession.transform.head.origin != testHeadOriginVal || oldSession.transform.head.angles != testHeadAnglesVal )
		{
			fields.transform =
			{
				"body":
				{
					"origin": testBodyOriginVal,
					"angles": testBodyAnglesVal
				},
				"head":
				{
					"origin": testHeadOriginVal,
					"angles": testHeadAnglesVal
				}
			};
		}

		var testMouseXVal = (!!session.mouse) ? session.mouse.x : this.defaultSession.mouse.x;
		var testMouseYVal = (!!session.mouse) ? session.mouse.y : this.defaultSession.mouse.y;
		if( !!!oldSession || !!!oldSession.mouse || oldSession.mouse.x != testMouseXVal || oldSession.mouse.y != testMouseYVal )
			fields.mouse = {"x": testMouseXVal, "y": testMouseYVal};

		var testVal = (!!session.web) ? session.web.url : this.defaultSession.web.url;
		if( !!!oldSession || !!!oldSession.web || oldSession.web.url != testVal )
			fields.web = {"url": testVal};

		var testVal = (!!session.avatar) ? session.avatar.url : this.defaultSession.avatar.url;
		if( testVal !== "" && (!!!oldSession || !!!oldSession.avatar || oldSession.avatar.url != testVal) )
			fields.avatar = {"url": testVal};

		if( typeof window.onUserSessionUpdated === "function" )
			window.onUserSessionUpdated(userId, this.users[userId].session, oldSession, fields);
	}
};

Metaverse.prototype.mapAdded = function(child, prevChildKey)
{
	var key = child.key();
	console.log("Downloaded metaverse information for map " + key);

	// Make sure the map has all required fields.
	var val = child.val();
	var x;
	for( x in this.defaultMap )
	{
		if( !val.current.hasOwnProperty(x) )
		{
			// Assume it is an empty container object.
			val.current[x] = {};
		}
	}

	this.library.maps[key] = val;
	this.libraryRef.child("maps").child(key).child("current").on("value", this.mapChanged.bind(this));
};

Metaverse.prototype.mapRemoved = function(child)
{
	console.log("Map removed.");
	delete this.library.maps[child.key()];
};

Metaverse.prototype.mapChanged = function(child, prevChildKey)
{
	var val = child.val();

	// Make sure the map has all required fields.
	var x;
	for( x in this.defaultMap )
	{
		if( !val.hasOwnProperty(x) )
		{
			// Assume it is an empty container object.
			val[x] = {};
		}
	}

	if( !!this.library.maps[val.info.id] )
	{
		console.log("NEW MAP ADDED as change");
		this.library.maps[val.info.id].current = val;
	}
	else
		this.library.maps[val.info.id] = {"current": val};
};

Metaverse.prototype.getUniverseKey = function(universeTitle)
{
	var x;
	for( x in this.universeTitles)
	{
		if( this.universeTitles[x] === universeTitle )
			return x;
	}

	return;
};

Metaverse.prototype.getUniverseName = function(universeKey)
{
	return this.universeTitles[universeKey];
};

Metaverse.prototype.encodePasscode = function(passcode)
{
	return md5(passcode + "katchup");
};

Metaverse.prototype.getUserId = function(username, callback)
{
	this.usersRef.orderByChild("username").equalTo(username).child("id").once("value", function(snapshot)
	{
		if( snapshot.exists() )
		{
			var val = snapshot.val();
			console.log("Value is: " + val);
		}
	});
};

Metaverse.prototype.logIn = function(username, passcode, callback, userIdOverride, displayNameOverride, avatarUrl)
{
	//console.log(avatarUrl);
	passcode = this.encodePasscode(passcode);
	var ref = (!!!userIdOverride) ? this.usersRef.orderByChild("username").equalTo(username) : this.usersRef.child(userIdOverride);
	ref.once("value", function(snapshot)
	{
		if( snapshot.exists() )
		{
			var val = snapshot.val();
			var key = snapshot.key();
			if( val.passcode === passcode )
			{
				// Only allow ONE login
				if( !!val.sessions && Object.keys(val.sessions).length > 0 )
				{
					callback(new Error("NOTICE: You were *not* logged in because there is already an active web client login from your IP."));
				}
				else
				{
					// If there is no owner of this universe, then set US as the owner.
					this.universeRef.child("info").child("owner").once("value", function(ownerSnapshot)
					{
						if( !ownerSnapshot.exists() || ownerSnapshot.val() === "" )
						{
							var data = {
								"owner": key,
								"admins": {}
							};

							data.admins[key] = {
								"id": key,
								"privileges":
								{
									"ban": true,
									"purge": true,
									"rollback": true,
									"unban": true
								}
							};

							this.universeRef.child("info").update(data, function()
							{
								onOwnerResolved.call(this, false);//true);
							}.bind(this));
						}
						else
							onOwnerResolved.call(this, false);

						function onOwnerResolved(needsDefaults)
						{	
							this.localUserRef = this.usersRef.child(key);
							this.connectedRef = new Firebase(this.root + ".info/connected");
							this.connectedRef.on("value", connectedRefUpdate.bind(this));

							function connectedRefUpdate(connectedSnapshot)
							{
								var needsCallback = false;

								if( connectedSnapshot.val() === true )
								{
									if( this.localUserRef )
									{
										this.sessionRef = this.localUserRef.child("sessions").push();

										this.sessionRef.onDisconnect().remove();
										this.localUserRef.onDisconnect().update({"lastLogOut": Firebase.ServerValue.TIMESTAMP});

										var sessionKey = this.sessionRef.key();
										var updateData = {};
										//updateData["sessions/" + sessionKey + "/status"] = "Online";
										updateData["sessions/" + sessionKey + "/created"] = Firebase.ServerValue.TIMESTAMP;
										updateData["lastLogIn"] = Firebase.ServerValue.TIMESTAMP;

										if(!!displayNameOverride)
										{
											if( displayNameOverride === "" )
												displayNameOverride = "Human Player";

											updateData["sessions/" + sessionKey + "/displayName"] = displayNameOverride;
										}
										else
											updateData["sessions/" + sessionKey + "/displayName"] = "Human Player";

										if( !!avatarUrl )
											updateData["sessions/" + sessionKey + "/avatar/url"] = avatarUrl;
										else
											updateData["sessions/" + sessionKey + "/avatar/url"] = "";

										this.localUserRef.update(updateData, function(stuff)
										{
											//this.localUserRef.child("sessions").child(sessionKey).once("value", function(localUserSessionSnapshot)
											//{
												needsCallback = true;
												this.localUserRef.once("value", function(localUserSnapshot)
												{
													localUserUpdate.call(this, localUserSnapshot, sessionKey);
												}.bind(this));
											//}.bind(this));
										}.bind(this));

										//needsCallback = true;
										//this.localUserRef.once("value", localUserUpdate.bind(this));

										/*
										this.sessionRef.set({"presence": {}, "status": "Online", "timestamp": Firebase.ServerValue.TIMESTAMP}, function(error)
										{
											//// Monitor our own user for changes
											needsCallback = true;
											this.localUserRef.once("value", localUserUpdate.bind(this));
										}.bind(this));
										*/
									}
									else
										console.log("NOTICE: Connection reestablished.");
								}
								else
								{
									console.log("NOTICE: Connection lost.");
									//this.connectedRef.off("value", connectedRefUpdate.bind(this));
									//this.localUserRef.off("value", localUserUpdate.bind(this));
								}

								function localUserUpdate(userSnapshot, sessionKey)
								{
									function onCallbackReady()
									{
										this.setStatus("Online");
										//console.log("heloooo");
										callback();
									}

									this.localUser = userSnapshot.val();
									this.localUser.session = this.localUser.sessions[sessionKey];
									delete this.localUser.sessions;
									this.localUser.session.id = sessionKey;

									//console.log(this.localUser);
									this.localUser["lastUpdate"] = {};

									if( needsCallback )
									{
										needsCallback = false;

	//console.log(needsDefaults);
										if( needsDefaults )
										{
											/*
											// Import all of the default platforms.
											var numDefaultPlatforms = this.defaultPlatforms.length;
											var resolvedPlatformIndex = -1;
											createPlatformHelper.call(this);

											function createPlatformHelper(platformId)
											{
												if( !!platformId )
													resolvedPlatformIndex++;

												if( resolvedPlatformIndex+1 < numDefaultPlatforms )
													this.createLibraryEntry("Platform", this.defaultPlatforms[resolvedPlatformIndex+1], null, arguments.callee.bind(this));
												else
													onPlatformsCreated.call(this);
											}

											function onPlatformsCreated()
											{
												// Now import all of the default types.
												var numDefaultTypes = this.defaultTypes.length;
												var resolvedIndex = -1;
												createTypeHelper.call(this);

												function createTypeHelper(typeId)
												{
													if( !!typeId )
														resolvedIndex++;

													if( resolvedIndex+1 < numDefaultTypes )
														this.createLibraryEntry("Type", this.defaultTypes[resolvedIndex+1], null, arguments.callee.bind(this));
													else
														onTypesCreated.call(this);
												}
											}
											*/
										}
										else
											onCallbackReady.call(this);
/*
										function onTypesCreated()
										{
											// Now import all of the default databases.
											var numDefaultDatabases = this.defaultDatabases.length;
											var resolvedDatabaseIndex = -1;
											createDatabaseHelper.call(this);

											function createDatabaseHelper(databaseId)
											{
												if( !!databaseId )
													resolvedDatabaseIndex++;

												if( resolvedDatabaseIndex+1 < numDefaultDatabases )
												{
													// Resolve type names to type ID's
													var database = this.defaultDatabases[resolvedDatabaseIndex+1];

													var realTypes = {};
													var realTypeAliases = {};
													
													var x, realType;

													for( x in database.typeAliases )
													{
														// find "x" in types, and replace x with the actual type id
														realType = this.findType(x);

														if( !realType )
															continue;

														realTypeAliases[realType.info.id] = {"id": realType.info.id, "aliases": database.typeAliases[x].aliases};
													}
													database.typeAliases = realTypeAliases;
		
													this.createLibraryEntry("Database", database, null, arguments.callee.bind(this));
												}
												else
													onDatabasesCreated.call(this);
											}

											function onDatabasesCreated()
											{
												onCallbackReady.call(this);
											}
										}
										*/
									}
								}
							}
						}
					}.bind(this));
				}
			}
			else
				callback(new Error("Invalid username or passcode."));
		}
		else
			callback(new Error("Invalid username or passcode."));
	}.bind(this));
}

Metaverse.prototype.createUser = function(data, callback, userIdOverride)
{
	var x;
	for( x in this.users )
	{
		if( this.users[x].username === data.username )
		{
			this.error = new Error("Username already exists.");
			callback();
			return;
		}
	}

	if( !this.validateData(data, this.defaultUser, callback) )
		return;

	var ref = (!!!userIdOverride) ? this.usersRef.push() : this.usersRef.child(userIdOverride);
	data.id = ref.key();
	data.passcode = this.encodePasscode(data.passcode);
	data.lastLogOut = 0;
	data.lastLogIn = Firebase.ServerValue.TIMESTAMP;
	ref.set(data);

	callback(data);
	return;
};

Metaverse.prototype.updateLocalUser = function(data, callback)
{
	if( !this.localUserRef )
	{
		callback("No local user.");
		return;
	}

	this.localUserRef.update(data, function(error)
	{
		if( !!error )
			callback(error);
		else
		{
			var x;
			for( x in data )
				this.localUser[x] = data[x];

			callback();
		}
	}.bind(this));
};

Metaverse.prototype.addEventListener = function(eventType, handler)
{
	var lastNum = -1;
	var x;
	for( x in this.listeners[eventType] )
		lastNum = parseInt(x[5]);

	var handlerId = "local" + (lastNum + 1);
	this.listeners[eventType][handlerId] = handler;
};

Metaverse.prototype.removeEventListener = function(eventType, handler)
{
	var x;
	for( x in this.listeners[eventType] )
	{
		if( this.listeners[eventType][x] === handler )
		{
			delete this.listeners[eventType][x];
			break;
		}
	}
};

Metaverse.prototype.validateData = function(data, defaultData, callback)
{
	var x, y;
	for( x in defaultData )
	{
		//if( x === "info" || !data.hasOwnProperty(x) )
		if( typeof data[x] === "undefined" )
			data[x] = "";

		if( defaultData[x] === true || !data.hasOwnProperty(x) )
			continue;
//		console.log(x);
//console.log(data[x]);
		if( defaultData[x].types === "integer" )
		{
			if( typeof data[x] === "string" && data[x] === "" )
				data[x] = defaultData[x].default;
			else if( (typeof data[x] === "number" && isNaN(data[x])) ||
				(typeof data[x] !== "number" && parseInt(data[x], 10) + "" != data[x] ) )
			{
				this.error = new Error(defaultData[x].formatDescription);
			//	if( !!callback )
			//		callback();
			//	else
					this.eventHandler("error", this.error);
			//			console.log(JSON.stringify(defaultData));
			//			console.log(isNaN(data[x]) + " : " + parseInt(data[x], 10) + " vs " + data[x]);
				return false;
			}
		}
		else if( defaultData[x].types === "number" )
		{
			if( typeof data[x] === "string" && data[x] === "" )
				data[x] = defaultData[x].default;
			else if( (typeof data[x] === "number" && isNaN(data[x])))// ||
				//(typeof data[x] !== "number" && parseFloat(data[x], 10) + "" !== data[x] ) )
			{
				this.error = new Error(defaultData[x].formatDescription);
				if( !!callback )
					callback();
				else
					this.eventHandler("error", this.error);
						console.log(x);
				return false;
			}
		}
		else if( typeof data[x] === "object" )
		{
			//console.log("data object, no validation protocol defined.")
			var r;
			for( r in data[x] )
			{
				for( s in data[x][r] )
				{
					if( typeof data[x][r][s] === "undfined" )
						data[x][r][s] = "";

					if( !defaultData[x].hasOwnProperty(s) )
						continue;

					//console.log(defaultData[x]);
					//console.log(data[x][r][s]);
					if( data[x][r][s].search(eval(defaultData[x][s].format)) === -1 )
					{
						this.error = new Error(defaultData[x][s].formatDescription);

						if( !!callback )
							callback();
						else
							this.eventHandler("error", this.error);
						console.log(x);
						return false;
					}
				}
			}
		}
		else if( (data[x] + "").search(eval(defaultData[x].format)) === -1 )
		{
			this.error = new Error(defaultData[x].formatDescription);
			if( !!callback )
				callback();
			else
				this.eventHandler("error", this.error);
						console.log(x + ": " + data[x]);
			return false;
		}
		else
		{
			if( data[x] === "" )
				data[x] = defaultData[x].default;
		}
	}

	return true;
};

Metaverse.prototype.findTwinPlatform = function(original, callback)
{
	callback();
};

Metaverse.prototype.findTwinMap = function(original, callback)
{
	callback();
};

Metaverse.prototype.findTwinInstance = function(original, callback)
{
	callback();
};

Metaverse.prototype.findTwinObject = function(original, callback)
{
	callback();
};

Metaverse.prototype.findTwinEntry = function(type, child, value, callback)
{
	// NOTE: For this method to truly be multi-purpose, it must
	// accept the child to order by in its params as well.

	// TODO: need to actually search the server for a similar entry.
	// If one is found, return its ID so that the callback can add
	// it to the local dictionary.

	//var sourcePlatformId = "-KJvcne3IKMZQTaG7lPo";
	var category = type.toLowerCase() + "s";
	//this.usersRef.orderByChild("username").equalTo(username).child("id").once("value", function(snapshot)
	//var ref = this.libraryRef.child(category).orderByChild("current/platforms/" + sourcePlatformId + "/file").equalTo(data.platforms[sourcePlatformId].file).once("value", function(snapshot)
	var ref = this.libraryRef.child(category).orderByChild("current/" + child).equalTo(value).once("value", function(snapshot)
	{
		var twinData = snapshot.val();
		if( twinData )
		{
			console.log(JSON.stringify(twinData));
			callback(twinData.info.id);
		}
		else
			callback();
	});
};

// This method handles updates & new objects.
// For new objects, an objectOverride *must* be provided & the local user is assumed owner
// and the creation date is assumed to be the moment the method is called.
// TODO: Change this to updateLibraryEntry
// Note: In the case of type = Object, this method assumes ALL model/item/type/app IDs already exist on the server!!
Metaverse.prototype.updateLibraryEntry = function(mode, data, callback, overrideId)
{
	// Ignore invalid calls
	if( (!!overrideId && !!data && !!data.info && !!data.info.id && overrideId !== data.info.id) || (!!!overrideId && (!!!data || !!!data.info || !!!data.info.id || data.info.id === "")))
	{
		console.log("WARNING: Invalid call to updateLibraryEntry ignored.");
		return;
	}

	// We will be determining if we actually need to update.  (Useless updates are aborted.)
	var needsUpdate = false;
	var updateData = {};

	// Timestamps will be used in various places.
	var timestamp = Firebase.ServerValue.TIMESTAMP;
	var category = mode.toLowerCase() + "s";

	// Get the ID we'll be using
	// TODO: This should use .push() if no override is given so it can be used even if no override is given.
	var entryId = (!!overrideId) ? overrideId : data.info.id;

	// Retrieve the data that already exists in our local metaverse library for this entry.
	var isNewEntry = false;
	var rawData = this.library[category][entryId];
	if( !!!rawData )
	{
		// If data does not exist yet, we are a new entry.
		isNewEntry = true;

		// Give it info
		data.info = {
			"created": timestamp,
			"id": entryId,
			"owner": this.localUser.id,
			"modified": timestamp,
			"modifier": this.localUser.id,
			"removed": 0,
			"remover": ""
		};

		// Validate the data
		console.log(mode);
		console.log(JSON.stringify(data));
		console.log(JSON.stringify(this["default" + mode]));
		if( !this.validateData(data, this["default" + mode], callback) )
		{
			console.log("WARNING: Data failed to validate in updateLibraryEntry");
			return;
		}

		updateData["current/info/created"] = data.info.created;
		updateData[this.localUser.id + "/info/created"] = data.info.created;

		updateData["current/info/id"] = data.info.id;
		updateData[this.localUser.id + "/info/id"] = data.info.id;

		updateData["current/info/owner"] = data.info.owner;
		updateData[this.localUser.id + "/info/owner"] = data.info.owner;

		updateData["current/info/modified"] = data.info.modified;
		updateData[this.localUser.id + "/info/modified"] = data.info.modified;

		updateData["current/info/modifier"] = data.info.modifier;
		updateData[this.localUser.id + "/info/modifier"] = data.info.modifier;

		updateData["current/info/removed"] = data.info.removed;
		updateData[this.localUser.id + "/info/removed"] = data.info.removed;

		updateData["current/info/remover"] = data.info.remover;
		updateData[this.localUser.id + "/info/remover"] = data.info.remover;

		needsUpdate = true;

		// Remember it
		this.library[category][entryId] = {"current": data};
		rawData = this.library[category][entryId];
		//rawData = {"current": data};
	}

	// Detect which fields have actually changed.
	var x;
	for( x in data )
	{
		if( x === "info" )
			continue;

		if( isNewEntry || rawData.current[x] !== data[x] )
		{
			rawData.current[x] = data[x];
			updateData["current/" + x] = data[x];
			updateData[this.localUser.id + "/" + x] = data[x];
			needsUpdate = true;
		}
	}

	if( needsUpdate )
	{
		rawData.current.info.modified = timestamp;
		rawData.current.info.modifier = this.localUser.id;
		updateData["current/info/modified"] = rawData.current.info.modified;
		updateData["current/info/modifier"] = rawData.current.info.modifier;
		updateData[this.localUser.id + "/info/modified"] = rawData.current.info.modified;
		updateData[this.localUser.id + "/info/modifier"] = rawData.current.info.modifier;

		this.libraryRef.child(category).child(rawData.current.info.id).update(updateData, function(error)
		{
			if( !!error )
			{
				console.log("Failed update: " + error);
				callback();
			}
			else
				callback(entryId);
		}.bind(this));
	}
	else
		callback();
};

Metaverse.prototype.sendAppUpdate = function(app, title, file, commandformat, type, download, reference, screen, description, filepaths, callback)
{
	var category = "apps";
	var id = app;
	var data = {
		"title": title,
		"file": file,
		"commandformat": commandformat,
		"type": type,
		"download": download,
		"reference": reference,
		"screen": screen,
		"description": description//,
		//"filepaths": filepaths	// disable filepaths for now
	};

	var isNewEntry = (!!!this.library[category][id]);
	if( !isNewEntry )
	{
		// deep-copy the CURRENT data so we can correctly handle listeners in the other callback
		var oldEntry = this.library[category][id].current;
		//console.log(oldEntry.type);
		var oldData = {
			"title": oldEntry.title,
			"file": oldEntry.file,
			"commandformat": oldEntry.commandformat,
			"type": oldEntry.type,
			"download": oldEntry.download,
			"reference": oldEntry.reference,
			"screen": oldEntry.screen,
			"description": oldEntry.description//,
			//"filepaths": oldEntry.filepaths
		};

		this.pendingInstanceEntries[id] = oldData;
	}
	else
		this.pendingInstanceEntries[id] = {};

	this.updateLibraryEntry("App", data, function(entryId)
	{
		if( isNewEntry && category === "objects")
		{
			//this.instanceObjectsRef.child(object).set(true, function()
			//{
			//	callback(entryId);
			//}.bind(this));
		}
		else
			callback(entryId);
	}.bind(this), id);
};

Metaverse.prototype.sendModelUpdate = function(model, dynamic, keywords, file, mountIds, workshopIds, title, screen, preview, download, callback)
{
	//console.log(JSON.stringify(arguments));
	var category = "models";
	var id = model;
	var data = {
		"dynamic": parseInt(dynamic),
		"keywords": keywords,
		"platforms":
		{
			"-KJvcne3IKMZQTaG7lPo":
			{
				"id": "-KJvcne3IKMZQTaG7lPo",
				"file": file,
				"mountIds": mountIds,
				"workshopIds": workshopIds,
				"download": download
			}
		},
		"title": title,
		"screen": screen,
		"preview": preview
	};

	var isNewEntry = (!!!this.library[category][id]);
	if( !isNewEntry )
	{
		// deep-copy the CURRENT data so we can correctly handle listeners in the other callback
		var oldEntry = this.library[category][id].current;
		//console.log(oldEntry.type);
		var oldData = {
			"dynamic": oldEntry.dynamic,
			"keywords": oldEntry.keywords,
			"platforms":
			{
				"-KJvcne3IKMZQTaG7lPo":
				{
					"id": "-KJvcne3IKMZQTaG7lPo",
					"file": (!!oldEntry.platforms && !!oldEntry.platforms["-KJvcne3IKMZQTaG7lPo"] && !!oldEntry.platforms["-KJvcne3IKMZQTaG7lPo"].file) ? oldEntry.platforms["-KJvcne3IKMZQTaG7lPo"].file : "",
					"mountIds": (!!oldEntry.platforms && !!oldEntry.platforms["-KJvcne3IKMZQTaG7lPo"] && !!oldEntry.platforms["-KJvcne3IKMZQTaG7lPo"].mountIds) ? oldEntry.platforms["-KJvcne3IKMZQTaG7lPo"].mountIds : "",
					"workshopIds": (!!oldEntry.platforms && !!oldEntry.platforms["-KJvcne3IKMZQTaG7lPo"] && !!oldEntry.platforms["-KJvcne3IKMZQTaG7lPo"].workshopIds) ? oldEntry.platforms["-KJvcne3IKMZQTaG7lPo"].workshopIds : "",
					"download": (!!oldEntry.platforms && !!oldEntry.platforms["-KJvcne3IKMZQTaG7lPo"] && !!oldEntry.platforms["-KJvcne3IKMZQTaG7lPo"].download) ? oldEntry.platforms["-KJvcne3IKMZQTaG7lPo"].download : ""
				}
			},
			"title": oldEntry.title,
			"screen": oldEntry.screen,
			"preview": oldEntry.preview
		};

		this.pendingInstanceEntries[id] = oldData;
	}
	else
		this.pendingInstanceEntries[id] = {};

	this.updateLibraryEntry("Model", data, function(entryId)
	{
		if( isNewEntry && category === "objects")
		{
			//this.instanceObjectsRef.child(object).set(true, function()
			//{
			//	callback(entryId);
			//}.bind(this));
		}
		else
			callback(entryId);
	}.bind(this), id);
};

Metaverse.prototype.sendTypeUpdate = function(type, fileformat, titleformat, title, priority, callback)
{
	var category = "types";
	var id = type;
	var data = {
		"fileformat": fileformat,
		"titleformat": titleformat,
		"title": title,
		"priority": parseInt(priority)
	};

	var isNewEntry = (!!!this.library[category][id]);
	if( !isNewEntry )
	{
		// deep-copy the CURRENT data so we can correctly handle listeners in the other callback
		var oldEntry = this.library[category][id].current;
		//console.log(oldEntry.type);
		var oldData = {
			"fileformat": oldEntry.fileformat,
			"titleformat": oldEntry.titleformat,
			"title": oldEntry.title,
			"priority": oldEntry.priority
		};

		this.pendingInstanceEntries[id] = oldData;
	}
	else
		this.pendingInstanceEntries[id] = {};

	this.updateLibraryEntry("Type", data, function(entryId)
	{
		if( isNewEntry && category === "objects")
		{
			//this.instanceObjectsRef.child(object).set(true, function()
			//{
			//	callback(entryId);
			//}.bind(this));
		}
		else
			callback(entryId);
	}.bind(this), id);
};

Metaverse.prototype.sendItemUpdate = function(item, app, description, download, file, marquee, preview, reference, screen, stream, title, type, callback)
{
	var category = "items";
	var id = item;
	var data = {
		"app": app,
		"description": description,
		"download": download,
		"file": file,
		"marquee": marquee,
		"preview": preview,
		"reference": reference,
		"screen": screen,
		"stream": stream,
		"title": title,
		"type": type
	};

	var isNewEntry = (!!!this.library[category][id]);
	if( !isNewEntry )
	{
		// deep-copy the CURRENT data so we can correctly handle listeners in the other callback
		var oldEntry = this.library[category][id].current;
		//console.log(oldEntry.type);
		var oldData = {
			"app": oldEntry.app,
			"description": oldEntry.description,
			"download": oldEntry.download,
			"file": oldEntry.file,
			"marquee": oldEntry.marquee,
			"preview": oldEntry.preview,
			"reference": oldEntry.reference,
			"screen": oldEntry.screen,
			"stream": oldEntry.stream,
			"title": oldEntry.title,
			"type": oldEntry.type
		};

		this.pendingInstanceEntries[id] = oldData;
	}
	else
		this.pendingInstanceEntries[id] = {};

	this.updateLibraryEntry("Item", data, function(entryId)
	{
		if( isNewEntry && category === "objects")
		{
			//this.instanceObjectsRef.child(object).set(true, function()
			//{
			//	callback(entryId);
			//}.bind(this));
		}
		else
			callback(entryId);
	}.bind(this), id);

	/*
	var data = {
		"item": item,
		"app": app,
		"description": description,
		"download": download,
		"file": file,
		"marquee": marquee,
		"preview": preview,
		"reference": reference,
		"screen": screen,
		"stream": stream,
		"title": title,
		"type": type
	};

	this.updateLibraryEntry("Item", data, function(itemId)
	{
		callback(itemId);
	}, item);
	*/
};

Metaverse.prototype.sendInstanceObjectRemove = function(object, instance, item, model, slave, child, parentObject, scale, origin, angles, callback)
{
	//this.instanceObjectsRef.child(object).remove();
	this.instanceObjectsRef.child(object).set(null, function()
	{
		var updateData = {};
		updateData["current/info/remover"] = this.localUser.id;
		updateData[this.localUser.id + "/info/remover"] = this.localUser.id;
		updateData["current/info/removed"] = Firebase.ServerValue.TIMESTAMP;
		updateData[this.localUser.id + "/info/removed"] = Firebase.ServerValue.TIMESTAMP;

		this.libraryRef.child("objects").child(object).update(updateData);
	}.bind(this));

};

Metaverse.prototype.sendInstanceObjectUpdate = function(object, instance, item, model, slave, child, parentObject, scale, origin, angles, callback)
{
	//var objectRef = this.libraryRef.child("objects").child(object);
	// FIXME: this is where we'd remove non-changed data fields...
	// do work

	var data = {
		"instance": instance,
		"item": item,
		"model": model,
		"slave": slave,
		"child": child,
		"parentObject": parentObject,
		"scale": scale,
		"origin": origin,
		"angles": angles
	};

	var isNewObject = (!!!this.library.objects[object]);
	if( !isNewObject )
	{
		// deep-copy the CURRENT data so we can correctly handle listeners in the other callback
		var oldObject = this.library.objects[object].current;
		var oldData = {
			"instance": oldObject.instance,
			"item": oldObject.item,
			"model": oldObject.model,
			"slave": oldObject.slave,
			"child": oldObject.child,
			"parentObject": oldObject.parentObject,
			"scale": oldObject.scale,
			"origin": oldObject.origin,
			"angles": oldObject.angles
		};

		this.pendingInstanceEntries[object] = oldData;
	}
	else
	{
		//console.log(object + " is a NEW object!");
		this.pendingInstanceEntries[object] = {};
	}

	this.updateLibraryEntry("Object", data, function(objectId)
	{		
		// If we're new, add us to the instance objects too
		if( isNewObject )
		{
			this.instanceObjectsRef.child(object).set(true, function()
			{
				callback(objectId);
			}.bind(this));
		}
		else
			callback(objectId);
	}.bind(this), object);
};

Metaverse.prototype.sendInstanceUserUpdate = function(instance, say, bodyOrigin, bodyAngles, headOrigin, headAngles, item, object, mouseX, mouseY, webURL, avatarURL, displayName)
{
	if( !!!this.sessionRef )
		return;

	// TODO: Add displayNameOverride to this session info update.

	var updates = {};
	if( !!displayName && displayName !== "" && (!this.localUser.lastUpdate.hasOwnProperty("displayName") || displayName != this.localUser.lastUpdate.displayName) )
	{
		updates['/displayName'] = displayName;
		this.localUser.displayName = displayName;
	}
	
	if( (!!instance && !this.localUser.lastUpdate.hasOwnProperty("instance")) || (!!instance && instance != this.localUser.lastUpdate.instance) || (!!!instance && this.localuser.lastUpdate.instance !== "") )
	{
		if( !!!instance )
			instance = "";

		updates['/instance'] = instance;
		this.localUser.lastUpdate.instance = instance;
	}

	if( (!!say && !this.localUser.lastUpdate.hasOwnProperty("say")) || (!!say && say != this.localUser.lastUpdate.say) || (!!!say && this.localUser.lastUpdate.say !== "") )
	{
		if( !!!say )
			say = "";

		updates['/say/modified'] = Firebase.ServerValue.TIMESTAMP;
		updates['/say/text'] = say;
		this.localUser.lastUpdate.say = say;
	}

	if( (!!bodyOrigin && !this.localUser.lastUpdate.hasOwnProperty("bodyOrigin")) || (!!bodyOrigin && bodyOrigin != this.localUser.lastUpdate.bodyOrigin) || (!!!bodyOrigin && this.localUser.lastUpdate.bodyorigin !== "") )
	{
		if( !!!bodyOrigin )
			bodyOrigin = "0 0 0";

		updates['/transform/body/modified'] = Firebase.ServerValue.TIMESTAMP;
		updates['/transform/body/origin'] = bodyOrigin;
		this.localUser.lastUpdate.bodyOrigin = bodyOrigin;
	}

	if( (!!bodyAngles && !this.localUser.lastUpdate.hasOwnProperty("bodyAngles")) || (!!bodyAngles && bodyAngles != this.localUser.lastUpdate.bodyAngles) || (!!!bodyAngles && this.localUser.lastUpdate.bodyAngles !== "") )
	{
		if( !!!bodyAngles )
			bodyAngles = "0 0 0";

		updates['/transform/body/modified'] = Firebase.ServerValue.TIMESTAMP;
		updates['/transform/body/angles'] = bodyAngles;
		this.localUser.lastUpdate.bodyAngles = bodyAngles;
	}

	if( (!!headOrigin && !this.localUser.lastUpdate.hasOwnProperty("headOrigin")) || (!!headOrigin && headOrigin != this.localUser.lastUpdate.headOrigin) || (!!!headOrigin && this.localUser.lastUpdate.headOrigin !== "") )
	{
		if( !!!headOrigin )
			headOrigin = "0 0 0";

		updates['/transform/head/modified'] = Firebase.ServerValue.TIMESTAMP;
		updates['/transform/head/origin'] = headOrigin;
		this.localUser.lastUpdate.headOrigin = headOrigin;
	}

	if( (!!headAngles && !this.localUser.lastUpdate.hasOwnProperty("headAngles")) || (!!headAngles && headAngles != this.localUser.lastUpdate.headAngles) || (!!!headAngles && this.localUser.lastUpdate.headAngles !== "") )
	{
		if( !!!headAngles )
			headAngles = "0 0 0";

		updates['/transform/head/modified'] = Firebase.ServerValue.TIMESTAMP;
		updates['/transform/head/angles'] = headAngles;
		this.localUser.lastUpdate.headAngles = headAngles;
	}

	if( (!!item && !this.localUser.lastUpdate.hasOwnProperty("item")) || (!!item && item != this.localUser.lastUpdate.item) || (!!!item && this.localUser.lastUpdate.item !== "") )
	{
		if( !!!item )
			item = "";

		updates['/item/modified'] = Firebase.ServerValue.TIMESTAMP;
		updates['/item/id'] = item;
		this.localUser.lastUpdate.item = item;
	}

	if( (!!object && !this.localUser.lastUpdate.hasOwnProperty("object")) || (!!object && object != this.localUser.lastUpdate.object) || (!!!object && this.localUser.lastUpdate.object !== "") )
	{
		if( !!!object )
			object = "";

		updates['/object/modified'] = Firebase.ServerValue.TIMESTAMP;
		updates['/object/id'] = object;
		this.localUser.lastUpdate.object = object;
	}

	if( (!!mouseX && !this.localUser.lastUpdate.hasOwnProperty("mouseX")) || (!!mouseX && mouseX != this.localUser.lastUpdate.mouseX) || (!!!mouseX && this.localUser.lastUpdate.mouseX !== parseFloat("0")) )
	{
		//console.log(mouseX != this.localUser.lastUpdate.mouseX);
		//console.log(mouseX + " vs " + this.localUser.lastUpdate.mouseX);
		if( !!!mouseX )
			mouseX = "0";

		updates['/mouse/modified'] = Firebase.ServerValue.TIMESTAMP;
		updates['/mouse/x'] = parseFloat(mouseX);
		this.localUser.lastUpdate.mouseX = parseFloat(mouseX);
	}

	if( (!!mouseY && !this.localUser.lastUpdate.hasOwnProperty("mouseY")) || (!!mouseY && mouseY != this.localUser.lastUpdate.mouseY) || (!!!mouseY && this.localUser.lastUpdate.mouseY !== parseFloat("0")) )
	{
		//console.log(mouseX != this.localUser.lastUpdate.mouseX);
		//console.log(mouseX + " vs " + this.localUser.lastUpdate.mouseX);
		if( !!!mouseY )
			mouseY = "0";

		updates['/mouse/modified'] = Firebase.ServerValue.TIMESTAMP;
		updates['/mouse/y'] = parseFloat(mouseY);
		this.localUser.lastUpdate.mouseY = parseFloat(mouseY);
	}

	if( (!!webURL && !this.localUser.lastUpdate.hasOwnProperty("webURL")) || (!!webURL && webURL != this.localUser.lastUpdate.webURL) || (!!!webURL && this.localUser.lastUpdate.webURL !== "") )
	{
		if( !!!webURL )
			webURL = "";

		updates['/web/modified'] = Firebase.ServerValue.TIMESTAMP;
		updates['/web/url'] = webURL;
		this.localUser.lastUpdate.webURL = webURL;
	}
	
	if( (!!avatarURL && !this.localUser.lastUpdate.hasOwnProperty("avatarURL")) || (!!avatarURL && avatarURL != this.localUser.lastUpdate.avatarURL) || (!!!avatarURL && this.localUser.lastUpdate.avatarURL !== "") )
	{
		if( !!!avatarURL )
			avatarURL = "";

		updates['/avatar/modified'] = Firebase.ServerValue.TIMESTAMP;
		updates['/avatar/url'] = avatarURL;
		this.localUser.lastUpdate.avatarURL = avatarURL;
	}

	var updateKeys = Object.keys(updates);
	if( updateKeys.length > 0 )
		this.sessionRef.update(updates);
		//this.sessionRef.child("presence").update(updates);
};
/*
Metaverse.prototype.instanceOverviewChanged = function(snapshot)
{
	if( typeof window.onInstanceOverviewChanged === "function" )
		window.onInstanceOverviewChanged(snapshot);
};
*/
Metaverse.prototype.connectInstance = function(instanceId, callback)
{
	this.instanceRef = this.libraryRef.child("instances").child(instanceId);
	this.instanceRef.child("current").on("value", function(snapshot)
	{
		this.instanceInfo = snapshot.val();

		if( !!!this.library.instances[instanceId] || window.isQuickHosting === true )
		{
			//var instanceVal = snapshot.val();
			//console.log(instanceVal);
			if( !!this.instanceInfo && !!this.instanceInfo.map )
			{
				// get the map 1st
				this.libraryRef.child("maps").child(this.instanceInfo.map).on("value", function(mapSnapshot)
				{
					if( !!!this.library.maps[this.instanceInfo.info.map] )
					{
						//this.library.maps[this.instanceInfo.info.map] = {"current": mapSnapshot.val()};
						this.instanceObjectsRef = this.instanceRef.child("objects");
						this.instanceUsersRef = this.instanceRef.child("users");
						this.instancePanosRef = this.instanceRef.child("panos");
						this.instanceOverviewRef = this.instanceRef.child("overview");

						//this.instanceObjectsRef.on("child_added", this.instanceObjectAdded.bind(this));
						//this.instanceUsersRef.on("child_added", this.instanceUserAdded.bind(this));
						this.instanceOverviewRef.once("value", function(snapshot)
						{
							window.g_overview = snapshot.val();
							//this.instanceOverviewChanged.call(this, snapshot);
							this.instanceUsersRef.on("child_added", this.instanceUserAdded.bind(this));
							this.instanceUsersRef.on("child_removed", this.instanceUserRemoved.bind(this));
							this.instancePanosRef.on("child_added", this.instancePanoAdded.bind(this));
							this.instancePanosRef.on("child_removed", this.instancePanoRemoved.bind(this));
							this.instanceObjectsRef.on("child_added", this.instanceObjectAdded.bind(this));

							this.isConnectedToInstance = true;

							if( typeof callback === "function" )
								callback({"instance": this.instanceInfo, "map": mapSnapshot.val()});
						}.bind(this));

						//if( typeof window.onMapAdde)
					}
					else
						this.mapChanged.call(this, mapSnapshot);
				}.bind(this));
			}
			else
				callback();
		}
		else
			this.instanceChanged.call(this, snapshot);
	}.bind(this));
};

Metaverse.prototype.addInstanceUser = function(instanceId, sessionId, userId, callback)
{
	//console.log(sessionId);
	this.instanceRef = this.libraryRef.child("instances").child(instanceId);
	this.instanceObjectsRef = this.instanceRef.child("objects");
	this.instanceUsersRef = this.instanceRef.child("users");
	this.instancePanosRef = this.instanceRef.child("panos");

	var ref = this.instanceRef.child("users").child(userId);
	var key = ref.key();

	var rawData = sessionId;
	ref.set(rawData, function(error)
	{
		if( !!!error )
		{
			ref.onDisconnect().remove();

			//var updates = {};
			//updates['/instance'] = instanceId;
			//updates['/transform/head/origin'] = "";
			//updates['/transform/head/angles'] = "";
			//updates['/transform/head/angles'] = "";

			//return firebase.database().ref().update(updates);
/*
			var rawData = {
				"instance": instanceId,
				"say":
				{
					"text": "",
					"modified": Firebase.ServerValue.TIMESTAMP
				},
				"transform":
				{
					"body":
					{
						"origin": "",
						"angles": "",
						"modified": Firebase.ServerValue.TIMESTAMP
					},
					"head":
					{
						"origin": "",
						"angles": "",
						"modified": Firebase.ServerValue.TIMESTAMP
					}
				},
				"item":
				{
					"id": "",
					"modified": Firebase.ServerValue.TIMESTAMP
				},
				"object":
				{
					"id": "",
					"modified": Firebase.ServerValue.TIMESTAMP
				},
				"mouse":
				{
					"x": 0,
					"y": 0,
					"modified": Firebase.ServerValue.TIMESTAMP
				},
				"web":
				{
					"url": "",
					"modified": Firebase.ServerValue.TIMESTAMP
				}
			};
*/
			var timestamp = 0; //Firebase.ServerValue.TIMESTAMP
			var updateData = {};
			updateData["instance"] = instanceId;
			updateData["say/text"] = "";
			updateData["say/modified"] = timestamp;
			updateData["transform/body/origin"] = "";
			updateData["transform/body/angles"] = "";
			updateData["transform/body/modified"] = timestamp;
			updateData["transform/head/origin"] = "";
			updateData["transform/head/angles"] = "";
			updateData["transform/head/modified"] = timestamp;
			updateData["item/id"] = "";
			updateData["item/modified"] = timestamp;
			updateData["object/id"] = "";
			updateData["object/modified"] = timestamp;
			updateData["mouse/x"] = "";
			updateData["mouse/y"] = "";
			updateData["mouse/modified"] = timestamp;
			updateData["web/url"] = "";
			updateData["web/modified"] = timestamp;
			updateData["avatar/url"] = "";
			updateData["avatar/modified"] = timestamp;

			this.sessionRef.update(updateData, function()
			{
				if( !this.isConnectedToInstance )
				{
					//console.log("connect bra...");
					this.connectInstance.call(this, instanceId, function()
					{
						//console.log("Connected.");
						callback(key);
						//aaapi.network.networkEvent("itemCreate", uniqueItemKeys.length, uniqueItemKeys.length);
						//aaapi.network.networkEvent("objectCreate", uniqueObjectKeys.length, uniqueObjectKeys.length);
						//aaapi.network.networkEvent("hostReady", quickConnectInfo.address, quickConnectInfo.universe, quickConnectInfo.instance, metaverse.sessionRef.key());
					});
				}
				else
					callback(key);
			}.bind(this));

/*
			var presenceRef = this.sessionRef.child("presence");
			presenceRef.set(rawData, function(error2)
			{
				if( !!!error2 )
				{
					presenceRef.onDisconnect().remove();

					this.connectInstance.call(this, instanceId);
					//this.instanceUsersRef.on("child_added", this.instanceUserAdded.bind(this));
					//this.instanceUsersRef.on("child_removed", this.instanceUserRemoved.bind(this));
					//this.instanceObjectsRef.on("child_added", this.instanceObjectAdded.bind(this));
					//this.instanceObjectsRef.on("child_removed", this.instanceObjectRemoved.bind(this));
					callback(key);
				}
				else
					callback();
			}.bind(this));*/

		}
		else
			callback();
	}.bind(this));
};

Metaverse.prototype.addInstanceObject = function(instanceId, keyOverride)//, callback, keyOverride)
{
	//console.log(keyOverride);
	var ref = (!!!keyOverride) ? this.libraryRef.child("instances").child(instanceId).child("objects").push() : this.libraryRef.child("instances").child(instanceId).child("objects").child(keyOverride);
	var key = ref.key();

	var rawData = true;

	var updateData = {};
	updateData[key] = rawData;

	//ref.set(rawData, function(error){callback(key);}.bind(this));
	this.libraryRef.child("instances").child(instanceId).child("objects").update(updateData);
	//callback(key);
	return key;
};

Metaverse.prototype.createLibraryEntry = function(type, data, overrideId)//, callback)
{
	// If data is given but NO overrideId, then this is a brand new
	// entry that must have info created for it too!

	if( !this.validateData(data, this["default" + type]) )//, callback) )
	{
		//callback();
		return;
	}

	var category = type.toLowerCase() + "s";

	// Lazy fix for hard-coded Source platform ID
	var ref;
	var parentRef;
	if( type === "Platform" && data.title === "AArcade: Source" )
	{
		parentRef = this.universeRef.child("library").child(category);
		ref = this.universeRef.child("library").child(category).child("-KJvcne3IKMZQTaG7lPo");
	}
	else
	{
		parentRef = (!!overrideId) ? this.universeRef.child("library").child(category) : this.universeRef.child("library").child(category);
		ref = (!!overrideId) ? this.universeRef.child("library").child(category).child(overrideId) : this.universeRef.child("library").child(category).push();
	}

	var id = (!!overrideId) ? overrideId : ref.key();

	// Assume this is a new object created by the local user at this moment in time.
	data["info"] = {
		"created": Firebase.ServerValue.TIMESTAMP,
		"id": id,
		"owner": this.localUser.id,
		"modified": Firebase.ServerValue.TIMESTAMP,
		"modifier": this.localUser.id,
		"removed": 0,
		"remover": ""
	};

	var rawData = {};
	rawData["current"] = data;
	rawData[this.localUser.id] = data;

	/*
	ref.set(rawData, function(error)
	{
		if( !!!error )
		{
			// add it to the local cache now
			if( !!this.library[category][id] )
				this.library[category][id].current = data;
			else
				this.library[category][id] = {"current": data};

			callback(id);
		}
		else
			callback();
	}.bind(this));
	*/

	var updateData = {};
	updateData[id] = rawData;
	parentRef.update(updateData);

	// add it to the local cache now
	if( !!this.library[category][id] )
		this.library[category][id].current = data;
	else
		this.library[category][id] = {"current": data};

	return id;
	//callback(id);
};

// OBSOLETE!! PHASE OUT!!

/*
Metaverse.prototype.createLibraryObject = function(type, data, callback, keyOverride)
{
	if( !this.validateData(data, this["default" + type], callback) )
		return;

	var category = type.toLowerCase() + "s";

	var shouldOverrideRef = (type === "Platform" && data.title === "AArcade: Source") ? true : false;
	var ref = (!shouldOverrideRef) ? this.universeRef.child("library").child(category).push() : this.universeRef.child("library").child(category + "/-KJvcne3IKMZQTaG7lPo");
	if( !!keyOverride )
		ref = this.universeRef.child("library").child(category + "/" + keyOverride);

	var key = ref.key();

	data["info"] = {
		"created": Firebase.ServerValue.TIMESTAMP,
		"id": key,
		"owner": this.localUser.id,
		"modified": Firebase.ServerValue.TIMESTAMP,
		"modifier": this.localUser.id,
		"removed": 0,
		"remover": ""
	};

	var rawData = {};
	rawData["current"] = data;
	rawData[this.localUser.id] = data;

	ref.set(rawData, function(error)
	{
		if( !!!error )
		{
			// add it to the local cache now
			if( !!this.library[category][key] )
				this.library[category][key].current = data;
			else
				this.library[category][key] = {"current": data};

			callback(key);
		}
		else
			callback();
	}.bind(this));
};
*/

Metaverse.prototype.findTwinLibraryObject = function(type, original, callback)
{
	callback();
};

Metaverse.prototype.generateHash = function(text)
{
	var hash = 0, i, chr, len;
	if( text.length === 0 )
		return hash;

	for( i = 0, len = text.length; i < len; i++ )
	{
		chr = text.charCodeAt(i);
		hash = ((hash << 5) - hash) + chr;
		hash |= 0;
	}

	return hash;
};

Metaverse.prototype.generateTitle = function(file, typeId)
{
	var title = file;
	//var cookedType = this.cookType(this.library.types[typeId]);

	if( this.library.types[typeId].current.titleformat !== "" )
	{
		var regEx = eval(this.library.types[typeId].current.titleformat);
		var matches = regEx.exec(file);
		if( matches )
			title = matches[matches.length-1];
	}

	return title;
};

Metaverse.prototype.generateType = function(file)
{
	var typeMatches = new Array();
	var x, cookedType;
	for( x in this.library.types )
	{
		// FIX ME: use new RegExp instead of eval for security reasons.
		// new RegExp
		//var regEx = new RegExp(this.types[x].format);

		//cookedType = metaverse.cookType(this.library.types[x]);
		if( file.search(eval(this.library.types[x].current.fileformat)) !== -1 )
			typeMatches.push(this.library.types[x].current);
	}

	typeMatches.sort(function(a, b)
	{
		return b.priority-a.priority;
	});

	if( typeMatches.length === 0 )
	{
		console.log("ERROR: No type matches found!");
		return;
	}

	return typeMatches[0].info.id;
};

Metaverse.prototype.isUrl = function(text)
{
	return (text.search(/((http|https):\/\/|(www\.|www\d\.))([^\-][a-zA-Z0-9\-]+)?(\.\w+)(\/\w+){0,}(\.\w+){0,}(\?\w+\=\w+){0,}(\&\w+\=\w+)?/i) !== -1);
};

// Originally from https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/encodeURIComponent
Metaverse.prototype.encodeRFC5987ValueChars = function(str){
    return encodeURIComponent(str).
        // Note that although RFC3986 reserves "!", RFC5987 does not,
        // so we do not need to escape it
        replace(/['()]/g, escape). // i.e., %27 %28 %29
        replace(/\*/g, '%2A').
            // The following are not required for percent-encoding per RFC5987, 
            // so we can allow for a little better readability over the wire: |`^
            replace(/%(?:7C|60|5E)/g, unescape);
};

/*
Metaverse.prototype.extractYouTubeId = function(trailerURL) {
  if( typeof trailerURL === "undefined" )
    return trailerURL;

  var youtubeid;
  if( trailerURL.indexOf("youtube") != -1 && trailerURL.indexOf("v=") != -1 ) {
    youtubeid = trailerURL.substr(trailerURL.indexOf("v=")+2);

    var found = youtubeid.indexOf("&");
    if( found > -1 ) {
      youtubeid = youtubeid.substr(0, found);
    }
  }
  else {
    var found = trailerURL.indexOf("youtu.be/");
    if( found != -1 ) {
      youtubeid = trailerURL.substr(found+9);

      found = youtubeid.indexOf("&");
      if( found != -1 ) {
        youtubeid = youtubeid.substr(0, found);
      }
    }
  }

  return youtubeid;
};
*/
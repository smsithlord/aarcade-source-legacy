var panos = ["multiplayer"];//outset", "outset2", "outset3", "corner", "cowboy", "elevator", "evilwall", "haunts", "ghostdance", "ghostknight", "hiest", "orchard", "orchard2", "orchard3", "orchard4", "militia", "militia2", "militia3", "militia4", "mcdonalds", "mcdonalds2", "mcdonalds3", "goldeneye", "guns", "cowboysanders", "acake", "raider", "cgi", "coldsanta", "snowy", "retro", "retro2", "club", "club2", "club3", "club4", "club5", "club6", "club7", "music", "beach", "inferno", "inferno2", "inferno3", "inferno4", "inferno5", "inferno6", "inferno7", "skate", "addams", "toon", "south", "cyber", "cyber2", "la", "donkey"];

function shuffleArray()
{
	var rando = Math.floor(Math.random() * panos.length);
	var front = panos.splice(0, rando);
	var shuffledArray = panos.concat(front);
	panos = shuffledArray;
}

shuffleArray();
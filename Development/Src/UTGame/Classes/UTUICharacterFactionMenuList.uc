/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Specific version of the menu list that draws an icon in addition to the menu text for the current faction.
 */
class UTUICharacterFactionMenuList extends UTUIIconMenuList;

event SelectItem(int NewSelection)
{
	local string OutValue;

	Super.SelectItem(NewSelection);

	// IconImage
	GetCellFieldString(self, 'IconImage', Selection, OutValue);
	IconImage = Texture2D(DynamicLoadObject(OutValue, class'Texture2D'));

	// IconU
	GetCellFieldString(self, 'IconU', Selection, OutValue);
	IconU = float(OutValue);

	// IconV
	GetCellFieldString(self, 'IconV', Selection, OutValue);
	IconV = float(OutValue);

	// IconUL
	GetCellFieldString(self, 'IconUL', Selection, OutValue);
	IconUL = float(OutValue);

	// IconVL
	GetCellFieldString(self, 'IconVL', Selection, OutValue);
	IconVL = float(OutValue);
}

defaultproperties
{
	IconPadding=(X=0.05,Y=0.05)
	IconImage=Texture2D'UI_HUD.HUD.UI_HUD_BaseD';
	IconU=442;
	IconV=76;
	IconUL=129;
	IconVL=104;
	IconColor=(R=1.0,G=1.0,B=1.0,A=0.99);
}

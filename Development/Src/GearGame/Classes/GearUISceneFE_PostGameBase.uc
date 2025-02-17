/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneFE_PostGameBase extends GearUISceneFrontEnd_Base
	abstract
	ClassRedirect(GearUISceneLobby_PostGameBase)
	Config(inherit);

defaultproperties
{
	bAllowPlayerJoin=false
}

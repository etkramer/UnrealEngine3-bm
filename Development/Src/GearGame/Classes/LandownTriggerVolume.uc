class LandownTriggerVolume extends DynamicTriggerVolume;

// Turn off navigation points as you go
event Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
{
	local NavigationPoint Nav;
	local CoverSlotMarker Marker;
	
	Nav = NavigationPoint(Other);
	if( Nav != None && !Nav.bBlocked )
	{
		Nav.bBlocked = TRUE;

		Marker = CoverSlotMarker(Nav);
		if( Marker != None )
		{
			Marker.SetSlotEnabled( FALSE );
		}

		WorldInfo.Game.NotifyNavigationChanged(Nav);
	}

	Super.Touch( Other, OtherComp, HitLocation, HitNormal );
}

defaultproperties
{
}
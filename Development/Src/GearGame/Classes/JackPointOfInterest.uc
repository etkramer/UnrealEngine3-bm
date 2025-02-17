class JackPointOfInterest extends TargetPoint;

/** Point jack in the direction of the rotation of point */
var()	bool	bDirectional;
/** Trace hit location gotten at startup that jack should lookat */
var		Vector	FocalPoint;
/** Jack has already investigated this object */
var		bool	bInvestigated;

simulated function PostBeginPlay()
{
	local Vector	HitNormal, EndTrace;
	local Actor		HitActor;

	Super.PostBeginPlay();

	if( bDirectional )
	{
		EndTrace = Location + Vector(Rotation)*128.f;
		HitActor = Trace( FocalPoint, HitNormal, EndTrace, Location );
		if( HitActor == None )
		{
			FocalPoint = EndTrace;
		}
	}
}


defaultproperties
{
	bDirectional=TRUE
}

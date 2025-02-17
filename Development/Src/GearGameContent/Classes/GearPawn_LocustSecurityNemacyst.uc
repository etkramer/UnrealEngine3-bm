class GearPawn_LocustSecurityNemacyst extends Gearpawn_LocustNemacyst
	config(Pawn);

var SpotLightComponent SpotLightComp;

var name SpotLightSocketName;

var instanced AIAvoidanceCylinderComponent AvoidCylComp;

/** on when we're homing in on the enemy */
var Pawn HomingTarget;
/** Sound that plays when an enemy is first noticed */
var SoundCue EnemyAcquiredNoise;

var config float DefaultAirSpeed;


simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	Mesh.AttachComponentToSocket(SpotLightComp, SpotLightSocketName);

}


function PossessedBy(Controller C, bool bVehicleTransition)
{
	Super.PossessedBy(C,bVehicleTransition);
	
	SetTimer(0.1f,false,nameof(SetAirSpeedDefault));
}

function SetAirSpeedDefault()
{
	//`log(GetFuncName());
	AirSpeed = DefaultAirSpeed;
}

simulated function SetupInkTrail();

simulated function Touch(Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal)
{
	if(VSizeSq(HitLocation - Location) > (GetCollisionRadius() * GetCollisionRadius()))
	{
		return;
	}

	Super.Touch(Other,OtherComp,HitLocation,HitNormal);
}

function HomingInOnEnemy(Pawn Enemy)
{
	PlaySound(EnemyAcquiredNoise);
	HomingTarget = Enemy;
}

simulated event Tick(float DeltaTime)
{
	local rotator bone_rot;
	local vector bone_loc;
	if(HomingTarget != none)
	{
		Mesh.TransformToBoneSpace('b_cyst_Head',GetPawnViewLocation(),rotator((HomingTarget.Location - Location)),bone_loc,bone_rot);
		Mesh.DetachComponent(SpotLightComp);
		Mesh.AttachComponent(SpotLightComp,'b_cyst_Head',bone_loc,bone_rot);
		//DrawDebugLine(Location,HomingTarget.Location,255,255,0);
	}

	Super.Tick(DeltaTime);
}

defaultproperties
{
	Begin Object Class=SpotLightComponent Name=SpotLightComponent0 
		LightAffectsClassification=LAC_DYNAMIC_AND_STATIC_AFFECTING
		CastShadows=TRUE
		CastStaticShadows=TRUE
		CastDynamicShadows=FALSE
		bForceDynamicLight=FALSE
		UseDirectLightMap=FALSE
		LightingChannels=(BSP=TRUE,Static=TRUE,Dynamic=TRUE,bInitialized=TRUE)
		OuterConeAngle=27.000000	
		Radius=2048.000000
		Brightness=5.000000
		bEnabled=TRUE
	End Object
	SpotLightComp=SpotLightComponent0

	Begin Object Class=StaticMeshComponent Name=SpotLightCone0
		StaticMesh=StaticMesh'Locust_Nemacyst.Mesh.S_Nemacyst_SimpleLightBeam'
		bAcceptsDynamicDecals=FALSE
		bAcceptsStaticDecals=FALSE
		CollideActors=FALSE
		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=FALSE
		bCastDynamicShadow=TRUE
		bAcceptsLights=FALSE
		bAcceptsDynamicLights=FALSE
		Scale3D=(X=3,Y=3,Z=5)
		Translation=(z=-35)
	End Object
	Components.Add(SpotLightCone0)



	Begin object Class=AIAvoidanceCylinderComponent Name=AvoidCyl0
		CollisionHeight=1024
		CollisionRadius=512
		Translation=(x=512)
		TeamThatShouldFleeMe=TEAM_COG
	End Object
	AvoidCylComp=AvoidCyl0
	Components.Add(AvoidCyl0)

	PeripheralVision=0.37f
	SightBoneName=SightBone
	SightRadius=2048.f
	SpotLightSocketName=SpotLightSocket
	PawnViewSocketName=SpotLightSocket
	AirSpeed=1.f

	EnemyAcquiredNoise=SoundCue'Locust_Nemecyst_Efforts.Nemecyst.Nemecyst_AlertCue'

	PSC_InkTrail=none
}
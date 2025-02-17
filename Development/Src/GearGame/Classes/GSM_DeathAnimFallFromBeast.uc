
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_DeathAnimFallFromBeast extends GSM_DeathAnimBase;

var BodyStance	BS_FallStart, BS_FallLoop;

function bool PlayDeathAnimation(class<GearDamageType> DamageType, vector HitLoc)
{
	local vector DeathVel;
	local float SideVel, AngularVelocityScale;

	PawnOwner.BS_Play(BS_FallStart, 1.f, 0.33f, -1.f);
	PawnOwner.BS_SetAnimEndNotify(BS_FallStart, TRUE);

	// If we died on a bloodmount, detach us.
	AngularVelocityScale = 1.f;
	if( PawnOwner.BloodMountIDiedOn != None )
	{
		AngularVelocityScale = 0.25f;
		PawnOwner.BloodMountIDiedOn.DetachDeadDriver();
	}

	// make sure we're not based on anything anymore.
	PawnOwner.SetBase(None);
	if( PawnOwner.Mesh != None )
	{
		PawnOwner.Mesh.SetCullDistance(PawnOwner.default.Mesh.CachedMaxDrawDistance);
		PawnOwner.Mesh.SetShadowParent(None);
		PawnOwner.UpdatePawnLightEnvironment(PawnOwner.LightEnvironment);
		// Setup notification when mesh is going to hit the ground
		PawnOwner.Mesh.ScriptRigidBodyCollisionThreshold = 0;
		PawnOwner.Mesh.SetNotifyRigidBodyCollision(TRUE);
		PawnOwner.Mesh.SetRBCollidesWithChannel(RBCC_Vehicle, TRUE);
	}

 	DeathAnimBlendToMotors();

	DeathVel = vect(0,0,400);
	if( PawnOwner.BaseWhenDead != None && !PawnOwner.BaseWhenDead.bWorldGeometry )
	{
		SideVel = (FRand() > 0.5) ? -150 : 150;
		DeathVel += SideVel * (vect(0,1,0) >> PawnOwner.BaseWhenDead.Rotation);
	}

// 	PawnOwner.DrawDebugCoordinateSystem(PawnOwner.Location, Rotator(DeathVel), 50.f, TRUE);
// 	`Log("DeathVel:" @ DeathVel);
	PawnOwner.Mesh.SetRBLinearVelocity(DeathVel, TRUE);
	PawnOwner.Mesh.SetRBAngularVelocity(VRand() * 25.f * AngularVelocityScale);

	return TRUE;
}

/** Notification when character hits the ground. Disable notification, and turn into ragdoll */
function RigidBodyWorldCollision(PrimitiveComponent HitComponent, PrimitiveComponent OtherComponent, const out CollisionImpactData RigidCollisionData)
{
	PawnOwner.Mesh.SetNotifyRigidBodyCollision(FALSE);
	DeathAnimRagDoll();
}

function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	PawnOwner.BS_SetAnimEndNotify(BS_FallStart, FALSE);
	PawnOwner.BS_Play(BS_FallLoop, 1.f, 0.1f, -1.f, TRUE);
}

function DeathAnimBlendToMotors()
{
	// Turn our guy into rag doll, as he's going to be falling off the beast.
	TurnToRagDollPhysics();

	Super.DeathAnimBlendToMotors();
}

defaultproperties
{
	BS_FallStart=(AnimName[BS_FullBody]="Reaver_Driver_Fall_Off")
	BS_FallLoop=(AnimName[BS_FullBody]="Reaver_Driver_Fall_Idle")
}
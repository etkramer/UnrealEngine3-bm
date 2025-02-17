
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_RecoverFromRagdoll_Ticker extends GSM_RecoverFromRagdoll;


/** 
 * Figure out what the Actor's Rotation should be.
 * And which recovery animation should be played.
 */
function SetupRotationAndAnimation()
{
	local Rotator NewRotation;

	// restart any animations on the pawn
	PawnOwner.ResumeAllAnimations();

	// Make sure we have a bone set to do these things.
	if( PawnOwner.Mesh.MatchRefBone(OrientationBoneName) != INDEX_NONE )
	{
		// force rotation to match the body's direction so the blend to the getup animation looks more natural
		NewRotation.Yaw = rotator(PawnOwner.Mesh.GetBoneAxis(OrientationBoneName, OrientationAxis)).Yaw;
		//PawnOwner.DrawDebugLine(PawnOwner.Location, PawnOwner.Location + PawnOwner.Mesh.GetBoneAxis(OrientationBoneName, OrientationAxis) * 100.f, 255, 0, 0, TRUE);

		bGetUpFromBack = (PawnOwner.Mesh.GetBoneAxis(UpDownBoneName, UpDownAxis).Z > 0.0);
		//PawnOwner.DrawDebugLine(PawnOwner.Location, PawnOwner.Location + PawnOwner.Mesh.GetBoneAxis(UpDownBoneName, UpDownAxis) * 100.f, 0, 0, 255, TRUE);

		// flip it around if the head is facing upwards, since the animation for that makes the character
		// end up facing in the opposite direction that its body is pointing on the ground
		if( bGetUpFromBack )
		{
			NewRotation.Yaw += GetUpFromBackYawOffset;
		}
		PawnOwner.SetRotation(NewRotation);
		//PawnOwner.DrawDebugCoordinateSystem(PawnOwner.Location, NewRotation, 30.f, TRUE);
	}
	else
	{
		NewRotation.Yaw = PawnOwner.Rotation.Yaw;
		PawnOwner.SetRotation(NewRotation);
	}

}


/** 
 *	Done with blend, start the get-up animation, start updating physics bones again, and show result.
 *	Bones should all be fixed at this point.
 */
simulated event FinishedBlendToGetUp()
{
	bBlendToGetUp = FALSE;

	PawnOwner.Mesh.RootMotionMode = RMM_Accel;
	PawnOwner.SetPhysics(PHYS_Falling);
	PawnOwner.CollisionComponent = PawnOwner.CylinderComponent;
	//PawnOwner.PreRagdollCollisionComponent = None;

	// Retore params to defaults.
	if( !PawnOwner.default.Mesh.bHasPhysicsAssetInstance )
	{
		PawnOwner.Mesh.SetHasPhysicsAssetInstance(FALSE);
	}
	PawnOwner.Mesh.bUpdateKinematicBonesFromAnimation = PawnOwner.default.Mesh.bUpdateKinematicBonesFromAnimation;
	PawnOwner.Mesh.bUpdateJointsFromAnimation = PawnOwner.default.Mesh.bUpdateJointsFromAnimation;
	PawnOwner.Mesh.PhysicsWeight = PawnOwner.default.Mesh.PhysicsWeight;
	PawnOwner.EndSpecialMove();
}

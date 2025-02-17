/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearCoverMeshComponent extends CoverMeshComponent
	native;

cpptext
{
	void UpdateMeshes();
}

var array<CoverMeshes> GameplayMeshRefs;
var StaticMesh GameplayAutoAdjustOn, GameplayAutoAdjustOff;
var StaticMesh GameplayDisabled;

defaultproperties
{
	GameplayAutoAdjustOn=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy_AutoAdjust'
	GameplayAutoAdjustOff=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy_AutoAdjustOff'
	GameplayDisabled=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy_Enabled'

	GameplayMeshRefs(CT_None)=(Base=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy__BASE_TALL')
	GameplayMeshRefs(CT_Standing)={(
						  Base=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy__BASE_TALL',
						  LeanLeft=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy_LeanLeftS',
						  LeanRight=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy_LeanRightS',
						  Mantle=None,
						  SlipLeft=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy_CoverSlipLeft',
						  SlipRight=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy_CoverSlipRight',
						  SwatLeft=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy_SwatLeft',
						  SwatRight=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy_SwatRight',
						  PopUp=None,
						)}
	GameplayMeshRefs(CT_MidLevel)={(
						  Base=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy__BASE_SHORT',
						  LeanLeft=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy_LeanLeftM',
						  LeanRight=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy_LeanRightM',
						  Climb=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy_Climb',
						  Mantle=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy_Mantle',
						  SlipLeft=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy_CoverSlipLeft',
						  SlipRight=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy_CoverSlipRight',
						  SwatLeft=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy_SwatLeft',
						  SwatRight=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy_SwatRight',
						  PopUp=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy_PopUp',
						)}
}

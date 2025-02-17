/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class GearAnim_Wretch_JumpAttack extends GearAnim_BlendList
	native(Anim);

cpptext
{
	virtual void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
	virtual void OnChildAnimEnd(UAnimNodeSequence* Child, FLOAT PlayedTime, FLOAT ExcessTime);
}

enum EWretchAttackState
{
	WAS_Init,
	WAS_AirLoop,
	WAS_Attached,
	WAS_KnockOff,
};
var EWretchAttackState AttackState;

function InitAttack( Actor Target )
{
	AttackState = WAS_Init;
}
function AttachAttack()
{
	AttackState = WAS_Attached;
}
function EndAttack()
{
	AttackState = WAS_KnockOff;
}

defaultproperties
{
	bFixNumChildren=TRUE
	Children(0)=(Name="Init")
	Children(1)=(Name="AirLoop")
	Children(2)=(Name="Attached")
	Children(3)=(Name="KnockOff")

	ChildBlendInTime(0)=0.20
	ChildBlendInTime(1)=0.20
	ChildBlendInTime(2)=0.20
	ChildBlendInTime(3)=0.20
}

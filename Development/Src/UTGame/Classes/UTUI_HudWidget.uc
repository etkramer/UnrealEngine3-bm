/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * These widgets are just for use in the hud.
 */

class UTUI_HudWidget extends UTUI_Widget
	abstract
//	HideCategories(Focus,States)
	native(UI);

/** Cached link to the UTUIScene_Hud that owns this widget */
var UTUIScene_Hud UTHudSceneOwner;

/** What is the Target opacity for this Widget */
var float OpacityTarget;

/** How long before we reach our target */
var float OpacityTimer;

/** If true, this widget will manage it's own visibily */
var bool bManualVisibility;

/** Visibiliy Flags - These determine if the widget should be visible at a given point in the game */
var(Visibility) bool bVisibleBeforeMatch;
var(Visibility) bool bVisibleDuringMatch;
var(Visibility) bool bVisibleAfterMatch;
var(Visibility) bool bVisibleWhileSpectating;

/****************************************************
 This is a quickly hacked in animation system for Gamers Day!  It will
 be replaced with a real animation system in the future, do not use */

enum	EAnimPropType
{
	EAPT_None,
	EAPT_PositionLeft,
	EAPT_PositionTop,
};

struct Native WidgetAnimSequence
{
	var() name 			Tag;
	var() float 		Rate;
	var() float			StartValue;
	var() float			EndValue;
};

struct Native WidgetAnimation
{
	var() name			Tag;
	var() EAnimPropType	Property;
	var() bool			bNotifyWhenFinished;

    var() editinline Array<WidgetAnimSequence> Sequences;


	var transient bool	bIsPlaying;
	var transient int   SeqIndex;
	var transient float	Time;
	var transient float Value;

};

var(Animation) editinline array<WidgetAnimation> Animations;


cpptext
{
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner=NULL );
	virtual void Tick_Widget(FLOAT DeltaTime);
	virtual void UpdateAnimations(FLOAT DeltaTime);
}

native function bool PlayAnimation(name AnimTag, name SeqTag, optional bool bForceBeginning);
native function StopAnimation(name AnimTag, optional bool bForceEnd);

function bool bIsAnimating()
{
	local int i;
	for (i=0;i<Animations.Length;i++)
	{
		if (Animations[i].bIsPlaying)
		{
			return true;
		}
	}
	return false;
}

function CancelAnimations()
{
	local int i;
	for (i=0;i<Animations.Length;i++)
	{
		if (Animations[i].bIsPlaying)
		{
			Animations[i].bIsPlaying = false;
			Animations[i].SeqIndex = 0;
			Animations[i].Time = 0;
			Animations[i].Value = 0;
		}
	}
}


/**
 * @Returns a reference to the hud that owns the scene that owns this widget
 */
function UTHud GetHudOwner()
{
	local UTPlayerController PC;
	if ( UTHudSceneOwner != none )
	{
		PC = UTHudSceneOwner.GetUTPlayerOwner();
		if ( PC != none )
		{
			return UTHud(PC.MyHud);
		}
	}
	return none;
}

/**
 * Calling FadeTo will cause the widget to fade to a new opacity value.
 *
 * @NewOpacity 		The desired opacity value
 * @NewFadeTime		How fast should we reach the value
 * @bTimeFromExtent	If true, FadeTo will consider NewFadeTime to be "if fading from min or max it would take x seconds"
 *					and then calculate the adjusted time given the current opacity value.
 */

event FadeTo(float NewOpacity, float NewFadeTime, optional bool bTimeFromExtent)
{
	local float Perc;

	OpacityTarget = NewOpacity;
	OpacityTimer = NewFadeTime;

	if (bTimeFromExtent)
	{
		Perc = 1- ( (NewOpacity > Opacity) ? (Opacity / NewOpacity) : (NewOpacity / Opacity) );
		OpacityTimer *= Perc;
	}
}


/** Fade the widget if needed */
event WidgetTick(FLOAT DeltaTime)
{
	if ( Opacity != OpacityTarget )
	{
		Opacity += (Opacity - OpacityTarget) * (OpacityTimer * Deltatime);
		OpacityTimer -= Deltatime;
		if ( OpacityTimer <= 0.0f )
		{
			Opacity = OpacityTarget;
			OpacityTimer = 0.0f;
		}
	}
}


event SetOpacity(float NewOpacity)
{
	Opacity = NewOpacity;
	OpacityTarget = NewOpacity;
	OpacityTimer = 0.0f;
}

event bool PlayerOwnerIsSpectating()
{
	local UTPlayerController PC;
	if ( UTHudSceneOwner != none )
	{
		PC = UTHudSceneOwner.GetUTPlayerOwner();
		if ( PC != none )
		{
			if (PC.Pawn == none || PC.IsDead() || PC.Pawn.Health <= 0)
			{
				return true;
			}

			return PC.IsSpectating();
		}
	}
	return false;
}

defaultproperties
{
	OpacityTarget = 1.0;
	bVisibleBeforeMatch=true
	bVisibleDuringMatch=true
	bVisibleAfterMatch=true
	bVisibleWhileSpectating=true
}

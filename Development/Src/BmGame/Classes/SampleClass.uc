//=============================================================================
// This is a sample native class, to serve as a minimal
// example of a class shared between UnrealScript and C++.
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class SampleClass extends Actor
	config(Game)
	placeable
	native;

// Declare some variables.
var			int		MyInteger;
var	config	string	MyString;
var			bool	MyBool;
var			vector	MyVector;
var	native	pointer MyPointer{FString};

// Here is a "native function", a function that is
// callable from UnrealScript but implemented in C++.
native function int SampleNativeFunction( int i, string s, vector v );

// Here is an "event", a function that is callable
// from C++ but implemented in UnrealScript.
event SampleEvent(int i)
{
	`Log("We are in SampleEvent in UnrealScript i="$i);
}

// Called when gameplay begins.
event PostBeginPlay()
{
	SetTimer(1,true);
}

// Called every 1 second, due to SetTimer call.
event Timer()
{
	MyInteger = MyInteger + 1;
	MyBool = !MyBool;
	`Log("SampleClass is counting "$MyInteger);
	`Log("Calling SampleNativeFunction from UnrealScript");
	SampleNativeFunction(MyInteger,"Testing",Location);
	`Log("String set via DefaultProperties and potentially overridden using the [BmGame.SampleClass] section: "$MyString);
}

defaultproperties
{
	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EditorResources.S_Actor'
		HiddenGame=True
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite)

	bHidden=False
}


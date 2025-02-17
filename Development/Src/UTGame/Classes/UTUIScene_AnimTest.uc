/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTUIScene_AnimTest extends UTUIScene;

var transient UIButton OpacityTest;
var transient UIButton ColorTest;
var transient UIButton VisTest;
var transient UIButton RotTest;

var transient UIImage TestImg;
var transient UIImage TestImg2;


event PostInitialize( )
{
	OpacityTest = UIButton(FindChild('OpacityTest',true));
	OpacityTest.OnClicked=TestOpacity;

	ColorTest = UIButton(FindChild('ColorTest',true));
	ColorTest.OnClicked=TestColor;

	VisTest = UIButton(FindChild('VisTest',true));
	VisTest.OnClicked=TestVis;

	RotTest = UIButton(FindChild('RotTest',true));
	RotTest.OnClicked=TestRot;


	TestImg = UIImage(FindChild('TestImg',true));
	TestImg2 = UIImage(FindChild('TestImg2',true));
}


function bool TestOpacity(UIScreenObject EventObject, int PlayerIndex)
{
	TestImg.PlayUIAnimation('TestOpacity');
	return true;
}

function bool TestColor(UIScreenObject EventObject, int PlayerIndex)
{
	TestImg2.PlayUIAnimation('TestRelPos');
	return true;
}

function bool TestVis(UIScreenObject EventObject, int PlayerIndex)
{
	TestImg.PlayUIAnimation('TestPos');
	return true;
}

function bool TestRot(UIScreenObject EventObject, int PlayerIndex)
{
	TestImg.PlayUIAnimation('TestScale');
	return true;
}


defaultproperties
{
}

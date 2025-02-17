/**
 * Gears-specific version of UIOptionList - for setting different default properties.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearOptionList extends UIOptionList;

DefaultProperties
{

	Begin Object Name=LabelStringRenderer
		StringStyle=(DefaultStyleTag="BodyCenter",RequiredStyleClass=class'Engine.UIStyle_Combo')
	End Object
	Begin Object Name=BackgroundImageTemplate
		ImageStyle=(DefaultStyleTag="imgBlank",RequiredStyleClass=class'Engine.UIStyle_Image')
	End Object
}

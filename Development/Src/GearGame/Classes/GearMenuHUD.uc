/**
 * HUD class for the front-end.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearMenuHUD extends HUD;

/**
 * PostRender is the main draw loop.
 */
event PostRender();

/**
 * The Main Draw loop for the hud.  Gets called before any messaging.  Should be subclassed
 */
function DrawHUD();

/**
 * Special HUD for Engine demo, overridden to prevent copyright info, etc.
 */
function DrawEngineHUD();

DefaultProperties
{

}

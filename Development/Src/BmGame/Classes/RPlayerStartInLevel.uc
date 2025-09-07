class RPlayerStartInLevel extends PlayerStart
    placeable;

var() editconst name Level;
var() string TeleportLabel;
var() bool bSaveWhenTeleportedTo;
var() bool bOnlySaveOnce;
var() bool bHideOtherLevelWhenTeleportedTo;
var() bool bStartCrouched;
var bool PreloadedLevelsFromKismet;
var bool TeleportFromKismet;
var bool bForceSave;
var bool bWasInsideBox;
var bool bStreamingInLevel;
var bool bEnableCheckpointSystem;
var bool bForceDisableCheckpointSystem;
var bool bForceCinematicModeOnTeleport;
var() float CheckpointWidth;
var() float CheckpointLength;

defaultproperties
{
    bSaveWhenTeleportedTo=true
    bHideOtherLevelWhenTeleportedTo=true
    begin object name=CollisionCylinder
    end object
    CylinderComponent=CollisionCylinder
    GoodSprite=none
    BadSprite=none
    // DefaultReachSpecClass=Class'CommonGame.RReachSpec'
    bStatic=false
    Components[0]=none
    Components[1]=none
    Components[2]=none
    Components[3]=CollisionCylinder
    Components[4]=none
    Components[5]=none
    CollisionComponent=CollisionCylinder
}

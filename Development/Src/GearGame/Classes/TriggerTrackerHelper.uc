class TriggerTrackerHelper extends Info
	native(Sequence);

var SeqAct_TriggerTracker SeqObj;

event Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
{
	SeqObj.NotifyTouch(Other, OtherComp, HitLocation, HitNormal);
}

defaultproperties
{
	Components.Remove(Sprite)
}

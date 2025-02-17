class SeqAct_CameraBlood extends SequenceAction;

enum BloodTypes
{
	BT_Shotgun,
	BT_Chainsaw,
	BT_Ink,
	BT_HOD,
	BT_Scorch,
	BT_Execution
};

var() BloodTypes Type;

var const array<class<Emitter> > Emitters;

defaultproperties
{
	ObjName="Camera Blood"
	ObjCategory="Camera"

	Emitters(BT_Execution)=class'Emit_CameraBlood_PunchSplatter'
	Emitters(BT_Shotgun)=class'Emit_CameraBlood_Shotgun'
	Emitters(BT_Chainsaw)=class'Emit_CameraBlood_Chainsaw'
	Emitters(BT_Ink)=class'Emit_CameraInk'
	Emitters(BT_HOD)=class'Emit_CameraHOD'
	Emitters(BT_Scorch)=class'Emit_CameraScorch'
}

class AICmd_ScriptedThrowGrenade extends AICommand;

/** target for throw */
var Actor ScriptedTarget;

var Actor OldFireTarget;
var Actor OldFocus;
var transient float OldThrowTime;

static function bool InitCommandUserActor(GearAI AI, Actor UserActor)
{
	local AICmd_ScriptedThrowGrenade Cmd;

	if (AI != None)
	{
		Cmd = new(AI) Default.Class;
		if (Cmd != None)
		{
			Cmd.ScriptedTarget = UserActor;
			AI.PushCommand(Cmd);
			return TRUE;
		}
	}
}

function Pushed()
{
	local Weapon GrenadeWeap;

	Super.Pushed();

	GrenadeWeap = Weapon(Pawn.InvManager.FindInventoryType(class'GearWeap_GrenadeBase', true));
	if (GrenadeWeap == None || !GrenadeWeap.HasAnyAmmo())
	{
		Status = 'Failed';
		PopCommand(self);
	}
	else
	{
		Pawn.InvManager.SetCurrentWeapon(GrenadeWeap);
	}

}

function Popped()
{
	Super.Popped();
	
	FireTarget = OldFireTarget;
	Focus = OldFocus;
	//`log(self@GetFuncName()@outer);
	ClearLatentAction(class'SeqAct_AIThrowGrenade', Status != 'Success');
}

function rotator GetAdjustedAimFor(Weapon W, vector StartFireLoc)
{
	FireTarget = ScriptedTarget;
	return Super.GetAdjustedAimFor(W, StartFireLoc);
}

auto state ThrowingGrenade
{
Begin:
	// save off old settings so we can restore it once we're done
	OldFireTarget = FireTarget;
	OldFocus = Focus;

	FireTarget = ScriptedTarget;	
	Focus = FireTarget;

	while (IsSwitchingWeapons())
	{
		Sleep(0.25);
	}

	bAllowedToFireWeapon = true;
	OldThrowTime=LastGrenadeTime;
	StartFiring();

	do
	{
		Sleep(0.25);
	} until (LastGrenadeTime!=OldThrowTime);

	Status = 'Success';
	PopCommand(self);
}

defaultproperties
{
	bAllowedToFireWeapon=false
}

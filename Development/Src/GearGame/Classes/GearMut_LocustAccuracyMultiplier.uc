class GearMut_LocustAccuracyMultiplier extends GearHordeExtendedMutator;

function ModifyPlayer(Pawn Other)
{
	local GearAI AI;

	AI = GearAI(Other.Controller);
	if (AI != None && AI.GetTeamNum() == 1)
	{
		AI.AimErrorMultiplier = 1.0 / Multiplier;
	}

	Super.ModifyPlayer(Other);
}

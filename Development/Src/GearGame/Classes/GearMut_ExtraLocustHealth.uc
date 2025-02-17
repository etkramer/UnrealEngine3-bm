class GearMut_ExtraLocustHealth extends GearHordeExtendedMutator;

function ModifyPlayer(Pawn Other)
{
	local GearPawn GP;

	GP = GearPawn(Other);
	if (GP != None && GP.GetTeamNum() == 1)
	{
		GP.DefaultHealth *= Multiplier;
	}

	Super.ModifyPlayer(Other);
}

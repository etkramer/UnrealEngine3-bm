/** mutator for Horde extended waves that applies a variable multiplier to some aspect of gameplay */
class GearHordeExtendedMutator extends Mutator;

var protected float Multiplier;

function SetMultiplier(float InMultiplier)
{
	Multiplier = InMultiplier;
}

defaultproperties
{
	Multiplier=1.0
}

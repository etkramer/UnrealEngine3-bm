class GearAI_Skorge extends GearAI;

var() GearPawn_LocustSkorgeBase SkorgePawn;

/** Current kismet sequence action */
var SeqAct_Latent CurrentAttackSeq;

/** Skorge is doing pistol strafe attack */
var bool bPistolStrafeAttack;

var GearWeap_GrenadeBase	GrenWeap;


function Possess( Pawn NewPawn, bool bVehicleTransition )
{
	Super.Possess( NewPawn, bVehicleTransition );

	SkorgePawn = GearPawn_LocustSkorgeBase(Pawn);
}

function OnSkorge_PistolStrafe( SeqAct_Skorge_PistolStrafe inAction )
{
	//debug
	`AILog( GetFuncName() );

	CurrentAttackSeq = inAction;

	BeginCombatCommand( class'AICmd_Attack_Skorge_PistolStrafe', 'OnSkorge_PistalStrafe' );
}

function OnSkorge_ChargeAndDuel( SeqAct_Skorge_ChargeAndDuel inAction )
{
	//debug
	`AILog( GetFuncName() );

	CurrentAttackSeq = inAction;

	BeginCombatCommand( class'AICmd_Attack_Skorge_ChargeAndDuel', 'OnSkorge_ChargeAndDuel' );
}

function OnSkorge_LeapToCeiling( SeqAct_Skorge_LeapToCeiling inAction )
{
	local bool bClearLatent;
	//debug
	`AILog( GetFuncName() );

	if( inAction.InputLinks[0].bHasImpulse )
	{
		bClearLatent = !class'AICmd_Move_JumpToCeiling_Skorge'.static.Jump( self, inAction.Destination );
	}
	else
	if( inAction.InputLinks[1].bHasImpulse )
	{
		bClearLatent = !class'AICmd_Move_DropFromCeiling_Skorge'.static.Jump( self, inAction.Destination );
	}

	if( bClearLatent )
	{
		ClearLatentAction( class'SeqAct_Skorge_LeapToCeiling', TRUE );
	}
}

function ForceEnemiesToUpdateTarget()
{
	local GearPawn  P;
	local GearAI	AI;

	foreach Squad.AllEnemies( class'GearPawn', P )
	{
		AI = GearAI(P.Controller);
		if( AI == None )
			continue;
		
//		AI.SelectTarget();
//		if( AI.FireTarget != Pawn )
//		{
//			AI.StopFiring();
//		}
	}	
}

function bool WantsInfiniteMagazineSize(GearWeapon Wpn)
{
	return TRUE;
}

// Skorge moves never time out
function float GetMoveTimeOutDuration(vector dest, bool bDontCare)
{
	return 0.f;
}

function bool NotifyBump( Actor Other, vector HitNormal );

function NotifyCeilingTransitionFall()
{
	//debug
	`AILog( GetFuncName()@MyGearPawn.SpecialMove@Pawn.Physics@Pawn.Velocity );

	if( MyGearPawn.IsDoingSpecialMove(GSM_LeapToCeiling) )
	{
		// Set velocity of pawn
		Pawn.ZeroMovementVariables();

		Pawn.Velocity.Z		= 5000.f;

		// Change physics to falling
		Pawn.SetPhysics( PHYS_Falling );

		//debug
		`AILog( Pawn.Velocity@Pawn.Physics );
	}
	else
	if( MyGearPawn.IsDoingSpecialMove(GSM_DropFromCeiling) )
	{
		// Just set physics to falling
		Pawn.SetPhysics( PHYS_Falling );
		Pawn.Velocity = vect(0,0,-450);
	}
}

event StartFiring(optional int InBurstsToFire=-1)
{
	local GearWeap_PistolBase Pistol;

	Super.StartFiring( InBurstsToFire );

	if( bPistolStrafeAttack )
	{
		if( GrenWeap == None )
		{
			foreach Pawn.InvManager.InventoryActors( class'GearWeap_PistolBase', Pistol )
			{
				if( Pistol.VerifyGrenadeWeapon() )
				{
					GrenWeap = Pistol.GrenWeap;
				}
				break;
			}
		}

		if( GrenWeap != None && !GrenWeap.IsTimerActive('GetWeaponRating') )
		{
			GrenWeap.SetTimer( 0.5f, TRUE, nameof(GrenWeap.GetWeaponRating) );
		}
	}
}

event StopFiring()
{
	Super.StopFiring();

	if( GrenWeap != None )
	{
		GrenWeap.ClearTimer( 'GetWeaponRating' );
	}
}

function ForceThrowGrenade()
{
	local GearWeap_PistolBase Pistol;

	foreach Pawn.InvManager.InventoryActors( class'GearWeap_PistolBase', Pistol )
	{
		Pistol.ForceThrowGrenade( self );
		break;
	}
}

function ThrowingInkGrenade(float Delay);

// Skorge only targets players
function AdjustEnemyRating(out float out_Rating, Pawn EnemyPawn)
{
	if( !EnemyPawn.IsPlayerOwned() )
	{
		out_Rating = -1.f;
	}
}

defaultproperties
{
	DefaultCommand=class'AICmd_Base_Skorge'

	DefaultReactConditionClasses.Empty
	bUseFireTickets=FALSE
}

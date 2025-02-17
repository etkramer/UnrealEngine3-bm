/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 * Structure defining information about a particular 'family' (eg. Ironguard Male)
 */

class UTFamilyInfo extends Object
	native
	abstract;

/**  @TODO - hookup character portraits   */
var Texture DefaultHeadPortrait;
var array<Texture> DefaultTeamHeadPortrait;

/** Matches the FamilyID in the CustomCharData */
var string FamilyID;

/** Faction that this family belongs to. */
var string Faction;

/************************************************************************/
/*  3P Character Info                                                   */
/************************************************************************/

/** Name of the mesh to use for this character */
var string CharacterMeshName;
/** Mesh reference for this character */
var SkeletalMesh CharacterMesh;

/** Material applied to the character head/body in team games */
var array<MaterialInterface> CharacterTeamBodyMaterials;
var array<MaterialInterface> CharacterTeamHeadMaterials;

/************************************************************************/
/*  1P Character Info                                                   */
/************************************************************************/

/** Package to load to find the arm mesh for this char. */
var string	ArmMeshPackageName;

/** Name of mesh within ArmMeshPackageName to use for arms. */
var string	ArmMeshName;

/** Package that contains team-skin materials for first-person arms. */
var		string					ArmSkinPackageName;
/** Name of red team material for first-person arms. */
var		string					RedArmMaterialName;
/** Name of blue team material for first-person arms. */
var		string					BlueArmMaterialName;

/** Name of 'neck stump' mesh to use if head is enclosed by helmet. */
var		string				NeckStumpName;
/** Extra offset to apply to mesh when rendering portrait for this family. */
var		vector				PortraitExtraOffset;

/** Physics Asset to use  */
var PhysicsAsset		PhysAsset;

/** Animation sets to use for a character in this 'family' */
var	array<AnimSet>		AnimSets;

/** Names for specific bones in the skeleton */
var name			LeftFootBone;
var name			RightFootBone;
var array<name>		TakeHitPhysicsFixedBones;

var class<UTPawnSoundGroup> SoundGroupClass;

var class<UTVoice>	VoiceClass;

var MaterialInstanceConstant	BaseMICParent;
var MaterialInstanceConstant	BioDeathMICParent;

/** This is the blood splatter effect to use on the walls when this pawn is shot @see LeaveABloodSplatterDecal **/
var MaterialInstance			BloodSplatterDecalMaterial;

/** When not in a team game, this is the color to use for glowy bits. */
var	LinearColor					NonTeamEmissiveColor;

/** When not in a team game, this is the color to tint character at a distance. */
var LinearColor					NonTeamTintColor;

/** Structure containing information about a specific emote */
struct native EmoteInfo
{
	/** Category to which this emote belongs. */
	var name		CategoryName;
	/** This is a unique tag used to look up this emote */
	var name		EmoteTag;
	/** Friendly name of this emote (eg for menu) */
	var localized string		EmoteName;
	/** Name of animation to play. Should be in AnimSets above. */
	var name		EmoteAnim;
	/** Indicates that this is a whole body 'victory' emote which should only be offered at the end of the game. */
	var bool		bVictoryEmote;
	/** Emote should only be played on top half of body. */
	var bool		bTopHalfEmote;
	/** The command that goes with this emote */
	var name  		Command;
	/** if true, the command requires a PRI */
	var bool		bRequiresPlayer;
};

/** Set of all emotes for this family. */
var array<EmoteInfo>	FamilyEmotes;

//// Gibs

/** information on what gibs to spawn and where */
struct native GibInfo
{
	/** the bone to spawn the gib at */
	var name BoneName;
	/** the gib class to spawn */
	var class<UTGib> GibClass;
	var bool bHighDetailOnly;
};
var array<GibInfo> Gibs;

/** Head gib */
var GibInfo HeadGib;

// NOTE:  this can probably be moved to the DamageType.  As the damage type is probably not going to have different types of mesh per race (???)
/** This is the skeleton skel mesh that will replace the character's mesh for various death effects **/
var SkeletalMesh DeathMeshSkelMesh;
var PhysicsAsset DeathMeshPhysAsset;

/** This is the number of materials on the DeathSkeleton **/
var int DeathMeshNumMaterialsToSetResident;

/** Which joints we can break when applying damage **/
var array<Name> DeathMeshBreakableJoints;

/** These are the materials that the skeleton for this race uses (i.e. some of them have more than one material **/
var array<MaterialInstanceTimeVarying> SkeletonBurnOutMaterials;

/** The visual effect to play when a headshot gibs a head. */
var ParticleSystem HeadShotEffect;

/** Name of the HeadShotGoreSocket **/
var name HeadShotGoreSocketName;

/** 
 * This is attached to the HeadShotGoreSocket on the pawn if there exists one.  Some pawns do no need to have this as their mesh already
 * has gore pieces.  But some do not.
 **/
var StaticMesh HeadShotNeckGoreAttachment;

var class<UTEmit_HitEffect> BloodEmitterClass;
/** Hit impact effects.  Sprays when you get shot **/
var array<DistanceBasedParticleTemplate> BloodEffects;

/** When you are gibbed this is the particle effect to play **/
var ParticleSystem GibExplosionTemplate;

/** scale for meshes in this family when driving a vehicle */
var float DrivingDrawScale;

/** Whether these are female characters */
var bool bIsFemale;

/** Mesh scaling */
var float DefaultMeshScale;
var float BaseTranslationOffset;


/** Return the 1P arm skeletal mesh representation for the class */ 
function static SkeletalMesh GetFirstPersonArms()
{
	local SkeletalMesh FirstPersonArmMesh;

	FirstPersonArmMesh = SkeletalMesh(DynamicLoadObject(default.ArmMeshName, class'SkeletalMesh'));
	if (FirstPersonArmMesh == None)
	{
		`warn("Unable to load first person arms"@default.ArmMeshName);
	}

	return FirstPersonArmMesh;
}

/** Return the material used for the 1P arm skeletal mesh given a team */
function static MaterialInterface GetFirstPersonArmsMaterial(int TeamNum)
{
	local string MaterialName;
	local MaterialInterface ArmMaterial;

	if (TeamNum == 0 || TeamNum == 1)
	{
		if (TeamNum == 0)
		{
			MaterialName = default.RedArmMaterialName;
		}
		else if (TeamNum == 1)
		{
			MaterialName = default.BlueArmMaterialName;
		}

		ArmMaterial = MaterialInterface(DynamicLoadObject(MaterialName, class'MaterialInterface'));
		if (ArmMaterial == None)
		{
			`warn("Unable to load first person arm material"@MaterialName);
		}

		return ArmMaterial;
	}

   return GetFirstPersonArms().Materials[0];
}

/** Return the appropriate team materails for this character class given a team */
function static GetTeamMaterials(int TeamNum, out MaterialInterface TeamMaterialHead, out MaterialInterface TeamMaterialBody)
{
	TeamMaterialHead = (TeamNum < default.CharacterTeamHeadMaterials.length) ? default.CharacterTeamHeadMaterials[TeamNum] : default.CharacterMesh.Materials[0];
	TeamMaterialBody = (TeamNum < default.CharacterTeamBodyMaterials.length) ? default.CharacterTeamBodyMaterials[TeamNum] : default.CharacterMesh.Materials[1];
}

/** Return the texture portrait stored for this character */
function static Texture GetCharPortrait(int TeamNum)
{
	return (TeamNum < default.DefaultTeamHeadPortrait.length) ? default.DefaultTeamHeadPortrait[TeamNum] : default.DefaultHeadPortrait;
}

/**
 * Returns the # of emotes in a given group
 */
function static int GetEmoteGroupCnt(name Category)
{
	local int i,cnt;
	for (i=0;i<default.FamilyEmotes.length;i++)
	{
		if (default.FamilyEmotes[i].CategoryName == Category )
		{
			cnt++;
		}
	}

	return cnt;
}

static function class<UTVoice> GetVoiceClass()
{
	return Default.VoiceClass;
}

/**
 * returns all the Emotes in a group
 */
function static GetEmotes(name Category, out array<string> Captions, out array<name> EmoteTags)
{
	local int i;
	local int cnt;
	for (i=0;i<default.FamilyEmotes.length;i++)
	{
		if (default.FamilyEmotes[i].CategoryName == Category )
		{
			Captions[cnt] = default.FamilyEmotes[i].EmoteName;
			EmoteTags[cnt] = default.FamilyEmotes[i].EmoteTag;
			cnt++;
		}
	}
}

/**
 * Finds the index of the emote given a tag
 */
function static int GetEmoteIndex(name EmoteTag)
{
	local int i;
	for (i=0;i<default.FamilyEmotes.length;i++)
	{
		if ( default.FamilyEmotes[i].EmoteTag == EmoteTag )
		{
			return i;
		}
	}
	return -1;
}

defaultproperties
{
	LeftFootBone=b_LeftAnkle
	RightFootBone=b_RightAnkle
	TakeHitPhysicsFixedBones[0]=b_LeftAnkle
	TakeHitPhysicsFixedBones[1]=b_RightAnkle
	SoundGroupClass=class'UTPawnSoundGroup'
	VoiceClass=class'UTGame.UTVoice_DefaultMale'

	FamilyEmotes[0]=(CategoryName="Taunt",EmoteTag="TauntA",EmoteAnim="Taunt_FB_BringItOn")
	FamilyEmotes[1]=(CategoryName="Taunt",EmoteTag="TauntB",EmoteAnim="Taunt_FB_Hoolahoop")
	FamilyEmotes[2]=(CategoryName="Taunt",EmoteTag="TauntC",EmoteAnim="Taunt_FB_Pelvic_Thrust_A")
	FamilyEmotes[3]=(CategoryName="Taunt",EmoteTag="TauntD",EmoteAnim="Taunt_UB_BulletToTheHead",bTopHalfEmote=true)
	FamilyEmotes[4]=(CategoryName="Taunt",EmoteTag="TauntE",EmoteAnim="Taunt_UB_ComeHere",bTopHalfEmote=true)
	FamilyEmotes[5]=(CategoryName="Taunt",EmoteTag="TauntF",EmoteAnim="Taunt_UB_Slit_Throat",bTopHalfEmote=true)

	FamilyEmotes[6]=(CategoryName="Order",EmoteTag="OrderA",EmoteAnim="Taunt_UB_Flag_Pickup",bTopHalfEmote=true,Command="Attack",bRequiresPlayer=true)
	FamilyEmotes[7]=(CategoryName="Order",EmoteTag="OrderB",EmoteAnim="Taunt_UB_Flag_Pickup",bTopHalfEmote=true,Command="Defend",bRequiresPlayer=true)
	FamilyEmotes[8]=(CategoryName="Order",EmoteTag="OrderC",EmoteAnim="Taunt_UB_Flag_Pickup",bTopHalfEmote=true,Command="Hold",bRequiresPlayer=true)
	FamilyEmotes[9]=(CategoryName="Order",EmoteTag="OrderD",EmoteAnim="Taunt_UB_Flag_Pickup",bTopHalfEmote=true,Command="Follow",bRequiresPlayer=true)
	FamilyEmotes[10]=(CategoryName="Order",EmoteTag="OrderE",EmoteAnim="Taunt_UB_Flag_Pickup",bTopHalfEmote=true,Command="Freelance",bRequiresPlayer=true)
	FamilyEmotes[11]=(CategoryName="Order",EmoteTag="OrderF",EmoteAnim="Taunt_UB_Flag_Pickup",bTopHalfEmote=true,Command="DropFlag",bRequiresPlayer=false)

	FamilyEmotes[12]=(CategoryName="Status",EmoteTag="Encouragement",EmoteAnim="Taunt_UB_Flag_Pickup",bTopHalfEmote=true)
	FamilyEmotes[13]=(CategoryName="Status",EmoteTag="Ack",EmoteAnim="Taunt_UB_Flag_Pickup",bTopHalfEmote=true)
	FamilyEmotes[14]=(CategoryName="Status",EmoteTag="InPosition",EmoteAnim="Taunt_UB_Flag_Pickup",bTopHalfEmote=true)
	FamilyEmotes[15]=(CategoryName="Status",EmoteTag="UnderAttack",EmoteAnim="Taunt_UB_Flag_Pickup",bTopHalfEmote=true)
	FamilyEmotes[16]=(CategoryName="Status",EmoteTag="AreaSecure",EmoteAnim="Taunt_UB_Flag_Pickup",bTopHalfEmote=true)

	FamilyEmotes[17]=(CategoryName="Class",EmoteTag="HumanM",EmoteAnim="",Command="HumanM")
	FamilyEmotes[18]=(CategoryName="Class",EmoteTag="HumanF",EmoteAnim="",Command="HumanF")
	FamilyEmotes[19]=(CategoryName="Class",EmoteTag="NecrisM",EmoteAnim="",Command="NecrisM")
	FamilyEmotes[20]=(CategoryName="Class",EmoteTag="NecrisF",EmoteAnim="",Command="NecrisF")
	FamilyEmotes[21]=(CategoryName="Class",EmoteTag="Krall",EmoteAnim="",Command="Krall")
	FamilyEmotes[22]=(CategoryName="Class",EmoteTag="Corrupt",EmoteAnim="",Command="Corrupt")
	FamilyEmotes[23]=(CategoryName="Class",EmoteTag="RAT",EmoteAnim="",Command="RAT")
	FamilyEmotes[24]=(CategoryName="Class",EmoteTag="Skaarj",EmoteAnim="",Command="Skaarj")

	NonTeamEmissiveColor=(R=10.0,G=0.2,B=0.2)
	NonTeamTintColor=(R=4.0,G=2.0,B=0.5)

	HeadShotEffect=ParticleSystem'T_FX.Effects.P_FX_HeadShot'

	HeadShotGoreSocketName="HeadShotGoreSocket"
	HeadShotNeckGoreAttachment=StaticMesh'CH_Gore.S_CH_Headshot_Gore'

	BloodEmitterClass=class'UTGame.UTEmit_BloodSpray'
	BloodSplatterDecalMaterial=MaterialInstanceTimeVarying'T_FX.DecalMaterials.MITV_FX_BloodDecal_Small01'

	GibExplosionTemplate=ParticleSystem'T_FX.Effects.P_FX_gibexplode'

	BloodEffects[0]=(Template=ParticleSystem'T_FX.Effects.P_FX_Bloodhit_01_Far',MinDistance=750.0)
	BloodEffects[1]=(Template=ParticleSystem'T_FX.Effects.P_FX_Bloodhit_01_Mid',MinDistance=350.0)
	BloodEffects[2]=(Template=ParticleSystem'T_FX.Effects.P_FX_Bloodhit_01_Near',MinDistance=0.0)

	DrivingDrawScale=1.0

	DefaultMeshScale=1.075
	BaseTranslationOffset=7.0

	/** TEMP */
	DefaultHeadPortrait=Texture'CH_IronGuard_Headshot.T_IronGuard_HeadShot_DM'
	DefaultTeamHeadPortrait[0]=Texture'CH_IronGuard_Headshot.T_IronGuard_HeadShot_Red'
	DefaultTeamHeadPortrait[1]=Texture'CH_IronGuard_Headshot.T_IronGuard_HeadShot_Blue'
}

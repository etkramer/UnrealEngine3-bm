/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
/** Manager which handles reaction channels and updating **/
class AIReactionManager extends Object within GearAI
	native
	dependsOn(AIReactChannel,GearAI);

struct native IntrinsicChannel
{
	var name ChannelName;
	var class<AIReactChannel> ChannelClass;
};

/** List of channels that should be set up initially, and have a custom channel class **/
var array<IntrinsicChannel> IntrinsicChannels;
/** map containing all our subscribed reactions **/
var instanced private const native Map{FName,UAIReactChannel*} ChannelMap;
/** Class to use when creating brand new channels that we don't have other data for **/
var() class<AIReactChannel> DefaultChannelClass;
/** List of channels that are expecting to be polled every frame **/
var array<AIReactChannel>   PollChannels;

/** if this bool is on, all channels will not fire **/
var bool bAllReactionsSuppressed;

/** array of cached channels (to avoid map.find) for the most basic channels (sight, hearing, force, etc..) */
var array<AIReactChannel> BasicPerceptionChannels;

/** --> Public Interface **/

/** Adds the passed condition to the channel of the given name (or creates a new channel with that name if none exists) **/
final native function Subscribe(AIReactCondition_Base Condition, Name ChannelName);

/** removes the passed condition from the list of the given channel, returning true if successful **/
final native function bool UnSubscribe(AIReactCondition_Base Condition, Name ChannelName);

/** disables (but does not remove) react conditions of the passed type, returns the reaction that was supressed if one is found, none otherwise **/
final native function SuppressReactionsByType(class<AIReactCondition_Base> Type, bool bDeepSearch);

/** suppresses each channel individually, rather than turning on the bool that will keep all channels (even new ones) from firing this suppresses each channel one by one so you can then unsuppress them selectively */
final native function SuppressAllChannels();

/** Unsuppresses each channel individually, rather than turning off the bool that will keep all channels (even new ones) from firing this unsuppresses each channel one by one */
final native function UnSuppressAllChannels();

/** Disables ALL reactions (even newly added ones)**/
final event SuppressAll()
{
	`AILog(self@"Suppressing all reaction channels!");
	bAllReactionsSuppressed=true;
}

/** Re-Enables reactions after being supressed **/
final event UnSuppressAll()
{
	`AILog(self@"UnSuppressing all reaction channels!");
	bAllReactionsSuppressed=false;
}

/** Suppress reactions for a given channel **/
final native function SuppressChannel(Name ChannelName);

/** UnSuppress reactions for a given channel **/
final native function UnSuppressChannel(Name ChannelName);

/** disables the given reaction **/
final native function SuppressReaction(AIReactCondition_Base ReactCondition);

/**
 * re-enables a ReactCondition of the given type, returns FALSE if one was not found
 * @param	bDeepSearch		indicates whether we should search the whole list of reactions if we don't find it in the auto subscribe channels
 **/
final native function UnSuppressReactionsByType(class<AIReactCondition_Base> Type, bool bDeepSearch);

/** Re-Enables a given reactcondition **/
final native function UnSuppressReaction(AIReactCondition_Base ReactCondition);

/** Find the _FIRST_ ReactCondition by a given type.
 * @param	bDeepSearch		indicates whether we should search the whole list of reactions if we don't find it in the auto subscribe channels
 **/
final native function AIReactCondition_Base FindReactionByType(class<AIReactCondition_Base> Type,bool bDeepSearch);


/**
* Notify the passed channel that a the passed perception channel just had an impulse
* @param EventInstigator the actor responsible for the impulse
* @param PT The perception (sight,hearing,etc..) that is causing this nudge
**/
native function NudgePerceptionChannel(Actor EventInstigator,EPerceptionType PT);

/**
 * Notify the passed channel that a named stimulus just had an impulse
 * @param EventInstigator the actor responsible for the impulse
 * @param ChannelName the channel that should be nudged
 **/
native function NudgeChannel(Actor EventInstigator, Name ChannelName);

/** Initialization function that should be called whent his AI first is created **/
native function Initialize();

/** removes all subscriptions and channels **/
native function Wipe();

cpptext
{
	void AddReferencedObjects( TArray<UObject*>& ObjectArray );
	void Serialize( FArchive& Ar );
	void InitializeChannel( FName ChannelName, UAIReactChannel* ReactChan );
	typedef void (UAIReactCondition_Base::*ForEachFunction)();
	void ForEachReactionOfType(UClass* Type, ForEachFunction CallThisFunction);
	UClass* GetClassForChannelName(FName Name);
	void Tick(FLOAT DeltaTime);
}



final private native function AIReactChannel GetChannelFor(name ChannelName);


/** Special Case for damage, since it takes so many custom params **/
function IncomingDamage( Pawn DamageInstigator, bool bDirectDamage, class<GearDamageType> damageType, optional TraceHitInfo HitInfo, optional Vector LastInstigatorLoc, optional int DamageAmt, optional name ChannelName='Damage' )
{
	local AIReactChan_Damage DamageChannel;

	//`log(GetFuncName()@outer@DamageInstigator@bDirectDamage@damageType@LastInstigatorLoc@DamageAmt@bAllReactionsSuppressed);
	DamageChannel = AIReactChan_Damage(GetChannelFor(ChannelName));
	if (DamageChannel != None)
	{
		DamageChannel.DamageInstigator = DamageInstigator;
		DamageChannel.bDirectDamage = bDirectDamage;
		DamageChannel.damageType = damageType;
		DamageChannel.HitInfo = HitInfo;
		DamageChannel.LastInstigatorLoc = LastInstigatorLoc;
		DamageChannel.DamageAmt = DamageAmt;
		//`log("NUDGING Damage channel for "@outer);
		NudgeChannel(DamageInstigator,ChannelName);
	}
}


defaultproperties
{
	DefaultChannelClass=class'GearGame.AIReactChannel'
	IntrinsicChannels(0)=(Channelname=Damage,ChannelClass=class'GearGame.AIReactChan_Damage')
	IntrinsicChannels(1)=(Channelname=HeadDamage,ChannelClass=class'GearGame.AIReactChan_Damage')
	IntrinsicChannels(2)=(ChannelName=Timer,ChannelClass=class'GearGame.AIReactChan_Timer')
}


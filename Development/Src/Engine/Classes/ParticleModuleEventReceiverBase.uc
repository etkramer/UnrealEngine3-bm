/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ParticleModuleEventReceiverBase extends ParticleModuleEventBase
	native(Particle)
	editinlinenew
	hidecategories(Object)
	abstract;

cpptext
{
	/**
	 *	Is the module interested in events of the given type?
	 *
	 *	@param	InEventType		The event type to check
	 *
	 *	@return	UBOOL			TRUE if interested.
	 */
	virtual UBOOL WillProcessEvent(EParticleEventType InEventType)
	{
		return FALSE;
	}

	/**
	 *	Process the event...
	 *
	 *	@param	Owner		The FParticleEmitterInstance this module is contained in.
	 *	@param	InEvent		The FParticleEventData that occurred.
	 *	@param	DeltaTime	The time slice of this frame.
	 *
	 *	@return	UBOOL		TRUE if the event was processed; FALSE if not.
	 */
	virtual UBOOL ProcessEvent(FParticleEmitterInstance* Owner, FParticleEventData& InEvent, FLOAT DeltaTime)
	{
		return FALSE;
	}
}

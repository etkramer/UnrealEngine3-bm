/*=============================================================================
	UnPostProcess.h: Post process scene rendering definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
 * Encapsulates the data which is mirrored to render a post porcess effect parallel to the game thread.
 */
class FPostProcessSceneProxy
{
public:

	/** 
	 * Initialization constructor. 
	 * @param InEffect - post process effect to mirror in this proxy
	 */
	FPostProcessSceneProxy(const UPostProcessEffect* InEffect);

	/** 
	 * Destructor. 
	 */
	virtual ~FPostProcessSceneProxy()
	{

	}

	/**
	 * Indicate that this is the final post-processing effect in the DPG
	 */
	void TerminatesPostProcessChain(UBOOL bIsFinalEffect) 
	{ 
		FinalEffectInGroup = bIsFinalEffect ? 1 : 0; 
	}

	/**
	 * Render the post process effect
	 * Called by the rendering thread during scene rendering
	 * @param InDepthPriorityGroup - scene DPG currently being rendered
	 * @param View - current view
	 * @param CanvasTransform - same canvas transform used to render the scene
	 * @return TRUE if anything was rendered
	 */
	virtual UBOOL Render(const class FScene* Scene, UINT InDepthPriorityGroup,class FViewInfo& View,const FMatrix& CanvasTransform)
	{ 
		return FALSE; 
	}

	/** 
	 * Accessor
	 * @return DPG the pp effect should be rendered in
	 */
	FORCEINLINE UINT GetDepthPriorityGroup() const
	{
		return DepthPriorityGroup;
	}

	UBOOL GetAffectsLightingOnly() const
	{
		return bAffectsLightingOnly;
	}

	/**
	 * Informs the view what to do during pre-pass. Overriden by the motion blur effect.
	 * @param MotionBlurParameters	- The parameters for the motion blur effect are returned in this struct.
	 * @return whether the effect needs to have velocities written during pre-pass.
	 */
	virtual UBOOL RequiresVelocities( struct FMotionBlurParameters &MotionBlurParameters ) const
	{
		return FALSE;
	}

	/**
	 * Tells the view whether to store the previous frame's transforms.
	 */
	virtual UBOOL RequiresPreviousTransforms(const FViewInfo& View) const
	{
		return FALSE;
	}

protected:
	/** DPG the pp effect should be rendered in */
	BITFIELD DepthPriorityGroup : UCONST_SDPG_NumBits;
	/** Whether this is the final post processing effect in the group */
	BITFIELD FinalEffectInGroup : 1;
	/** Whether the effect only affects lighting */
	BITFIELD bAffectsLightingOnly : 1;
};



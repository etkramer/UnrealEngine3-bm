//------------------------------------------------------------------------------
// A special animation player that bakes out Face Graph nodes into animation
// curves.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxFastFaceGraphBaker_H__
#define FxFastFaceGraphBaker_H__

#include "FxPlatform.h"
#include "FxActor.h"
#include "FxAnim.h"

namespace OC3Ent
{

namespace Face
{

/// A %Face Graph baker.
/// A %Face Graph baker will hook into the %Face Graph of the passed in actor
/// and temporarily modify it by adding and then removing delta nodes and links.
/// Because of this you should never use a %Face Graph baker inside of a 
/// %Face Graph iteration loop.
class FxFastFaceGraphBaker
{
public:
	/// Constructor.
	FxFastFaceGraphBaker( FxActor* pActor = NULL );
	/// Destructor.
	virtual ~FxFastFaceGraphBaker();

	/// Connects the Face Graph baker to an actor.
	void SetActor( FxActor* pActor );
	/// Returns the actor this Face Graph baker is connected to.
	FxActor* GetActor( void );

	/// Sets the animation that the Face Graph baker is baking.
	void SetAnim( const FxName& animGroupName, const FxName& animName );
	/// Returns the animation group name that the Face Graph baker is baking.
	const FxName& GetAnimGroupName( void ) const;
	/// Returns the animation name that the Face Graph baker is baking.
	const FxName& GetAnimName( void ) const;

	/// Bakes each node in the nodesToBake array (if found in the Face Graph)
	/// to separate key-reduced FxAnimCurve objects.  Each resulting FxAnimCurve
	/// will be named with the same name as the Face Graph node it was baked
	/// from.  Returns FxTrue if there are results or FxFalse if there are not.
	FxBool Bake( const FxArray<FxName>& nodesToBake );

	/// Returns the number of animation curves resulting from the baking
	/// process.
	FxSize GetNumResultAnimCurves( void ) const;
	/// Returns the animation curve result at index.
	const FxAnimCurve& GetResultAnimCurve( FxSize index ) const;

protected:
	/// The FxActor this Face Graph baker is connected to.
	FxActor* _pActor;
	/// The name of the animation group that contains the animation that this
	/// Face Graph baker is baking.
	FxName _animGroupName;
	/// The name of the animation that this Face Graph baker is baking.
	FxName _animName;
	/// The resulting animation curves that this Face Graph baker produced.
	FxArray<FxAnimCurve> _resultingAnimCurves;
};

} // namespace Face

} // namespace OC3Ent

#endif

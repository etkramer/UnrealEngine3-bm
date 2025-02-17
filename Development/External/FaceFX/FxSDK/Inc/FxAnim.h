//------------------------------------------------------------------------------
// A FaceFx animation.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAnim_H__
#define FxAnim_H__

#include "FxPlatform.h"
#include "FxNamedObject.h"
#include "FxArray.h"
#include "FxAnimCurve.h"
#include "FxFaceGraph.h"

namespace OC3Ent
{

namespace Face
{

/// A per-animation bone weight structure.
struct FxAnimBoneWeight
{
	/// Tests for equality.
	FX_INLINE FxBool operator==( const FxAnimBoneWeight& other ) const
	{
		return boneName == other.boneName && FxRealEquality(boneWeight, other.boneWeight);
	}
	/// Test for inequality.
	FX_INLINE FxBool operator!=( const FxAnimBoneWeight& other ) const
	{
		return !(operator==(other));
	}

	/// The name of the bone.
	FxName boneName;
	/// The weight of the bone.
	FxReal boneWeight;
};

/// An animation.  
/// In FaceFX, the FxAnim class is responsible for grouping together the various
/// curves controlling the animation.
///
/// \ingroup animation
class FxAnim : public FxNamedObject, public FxDataContainer
{
	/// Declare the class.
	FX_DECLARE_CLASS(FxAnim, FxNamedObject)
public:
	/// Constructor.
	FxAnim();
	/// Copy constructor.
	FxAnim( const FxAnim& other );
	/// Assignment.
	FxAnim& operator=( const FxAnim& other );
	/// Destructor.
	virtual ~FxAnim();

	/// Tests for equality.
	FxBool operator==( const FxAnim& other ) const;
	/// Test for inequality.
	FxBool operator!=( const FxAnim& other ) const;

	/// Returns the number of animation curves in the animation.
	FX_INLINE FxSize GetNumAnimCurves( void ) const { return _animCurves.Length(); }
	/// Returns the index of the animation curve with the specified name.
	/// \param name The name of the curve to find.
	/// \return The index of the animation curve with the specified name, or 
	/// \p FxInvalidIndex if an animation curve named \a name was not found.
	FxSize FindAnimCurve( const FxName& name ) const;
	/// Returns a const reference to the animation curve at index.
	FX_INLINE const FxAnimCurve& GetAnimCurve( FxSize index ) const { return _animCurves[index]; }
	/// Returns a mutable reference to the animation curve at index.
	FX_INLINE FxAnimCurve& GetAnimCurveM( FxSize index ) { return _animCurves[index]; }

	/// Adds the animation curve to the animation.  If FxFalse is returned then
	/// the animation already contains the curve.
	FxBool AddAnimCurve( const FxAnimCurve& curve );
	/// Inserts an animation curve at a specific slot in the animation.  If 
	/// FxFalse is returned then the animation already contains the curve.
	FxBool InsertAnimCurve( const FxAnimCurve& curve, FxSize index = FxInvalidIndex );
	/// Removes the animation curve named 'name'.  If FxFalse is returned then
	/// the animation does not contain the curve named 'name'.
	FxBool RemoveAnimCurve( const FxName& name );

	/// Returns the start time of the animation.
	FX_INLINE FxReal GetStartTime( void ) const { return _startTime; }
	/// Returns the end time of the animation.
	FX_INLINE FxReal GetEndTime( void ) const { return _endTime; }
	/// Returns the duration of the animation.
	FX_INLINE FxReal GetDuration( void ) const { return _endTime - _startTime; }
	/// Returns the blend in time of the animation.
	FX_INLINE FxReal GetBlendInTime( void ) const { return _blendInTime; }
	/// Returns the blend out time of the animation.
	FX_INLINE FxReal GetBlendOutTime( void ) const { return _blendOutTime; }

	/// Sets the blend in time of the animation.
	void SetBlendInTime( FxReal blendInTime );
	/// Sets the blend out time of the animation.
	void SetBlendOutTime( FxReal blendOutTime );

	/// Returns the number of animation bone weights in the animation.
	FX_INLINE FxSize GetNumBoneWeights( void ) const { return _boneWeights.Length(); }
	/// Returns the animation bone weight at index.
	FX_INLINE const FxAnimBoneWeight& GetBoneWeight( FxSize index ) const { return _boneWeights[index]; }
	/// Adds an animation bone weight to the animation.  Returns \p FxTrue if
	/// the bone weight was added or \p FxFalse it it was a duplicate and could
	/// not be added.
	FxBool AddBoneWeight( const FxAnimBoneWeight& animBoneWeight );
	/// Removes the animation bone weight containing the bone named 'boneName'.
	/// Returns \p FxTrue if the bone weight was removed or \p FxFalse if it
	/// was not found.
	FxBool RemoveBoneWeight( const FxName& boneName );
	/// Removes all of the animation bone weights contained in the animation.
	void RemoveAllBoneWeights( void );

	/// Serialization.
	virtual void Serialize( FxArchive& arc );

	// Unreal-specific stuff.
	// Sets the USoundCue path for the animation.
	void SetSoundCuePath( const FxString& soundCuePath );
	// Gets the USoundCue path for the animation.
	FX_INLINE const FxString& GetSoundCuePath( void ) const { return _soundCuePath; }
	// Sets the USoundNodeWave name for the animation.
	void SetSoundNodeWave( const FxString& soundNodeWave );
	// Gets the USoundNodeWave name for the animation.
	FX_INLINE const FxString& GetSoundNodeWave( void ) const { return _soundNodeWave; }
	// Sets the USoundCue index for the animation.
	void SetSoundCueIndex( FxSize soundCueIndex );
	// Gets the USoundCue index for the animation.
	FX_INLINE FxSize GetSoundCueIndex( void ) const { return _soundCueIndex; }
	// Sets the USoundCue pointer for the animation.
	void SetSoundCuePointer( void* soundCuePointer );
	// Gets the USoundCue pointer for the animation.
	FX_INLINE void* GetSoundCuePointer( void ) const { return _soundCuePointer; }

//------------------------------------------------------------------------------
// Internal use only
//------------------------------------------------------------------------------
	/// Finds and sets _startTime and _endTime.  
	/// \note For internal use only.
	void FindBounds( void );

private:
	/// The animation curves.
	FxArray<FxAnimCurve> _animCurves;
	
	/// The start time of the animation.
	FxReal _startTime;
	/// The end time of the animation.
	FxReal _endTime;
	/// The duration of the blend in operation.
	FxReal _blendInTime;
	/// The duration of the blend out operation.
	FxReal _blendOutTime;

	/// The animation bone weights.
	FxArray<FxAnimBoneWeight> _boneWeights;

	// Unreal-specific stuff.
	// The USoundCue path for the animation.
	FxString _soundCuePath;
	// The USoundNodeWave name for the animation.
	FxString _soundNodeWave;
	// The USoundCue index for the animation.
	FxSize _soundCueIndex;
	// The USoundCue pointer for the animation.
	void* _soundCuePointer;
	
	/// Returns the index of the animation bone weight that contains the bone
	/// named 'boneName' or \p FxInvalidIndex if it does not exist.
	FxSize _findAnimBoneWeight( const FxName& boneName ) const;
};

FX_INLINE FxBool FxAnim::operator==( const FxAnim& other ) const
{
	return _name == other._name && _animCurves == other._animCurves &&
		   _startTime == other._startTime && _endTime == other._endTime &&
		   _blendInTime == other._blendInTime && _blendOutTime == other._blendOutTime &&
		   _boneWeights == other._boneWeights && _soundCuePath == other._soundCuePath &&
		   _soundNodeWave == other._soundNodeWave;
}

FX_INLINE FxBool FxAnim::operator!=( const FxAnim& other ) const
{
	return !(operator==(other));
}

/// An animation group.
/// Animation groups provide a way of associating related animations.  For example,
/// all the animations for use in the first level might go in the "LevelOne" group.  
/// Using animation groups, you can easily conserve memory by only loading the 
/// appropriate groups for a specific game situation.
/// \ingroup animation
class FxAnimGroup : public FxNamedObject
{
	/// Declare the class.
	FX_DECLARE_CLASS(FxAnimGroup, FxNamedObject)
public:
	/// Constructor.
	FxAnimGroup();
	/// Copy constructor.
	FxAnimGroup( const FxAnimGroup& other );
	/// Assignment.
	FxAnimGroup& operator=( const FxAnimGroup& other );
	/// Destructor.
	virtual ~FxAnimGroup();

	/// Tests for equality.
	FxBool operator==( const FxAnimGroup& other ) const;
	/// Test for inequality.
	FxBool operator!=( const FxAnimGroup& other ) const;

	/// The "Default" animation group name.
	static FxName Default;
	
	/// Returns the number of animations in the group.
	FX_INLINE FxSize GetNumAnims( void ) const { return _anims.Length(); }
	/// Finds an animation in the group.
	/// \param name The name of the animation to find.
	/// \return The index of the animation with the specified name, or
	/// \p FxInvalidIndex if an animation named \p name was not found
	/// in the group.
	FxSize FindAnim( const FxName& name ) const;
	/// Returns the animation at index.
	FX_INLINE const FxAnim& GetAnim( FxSize index ) const { return _anims[index]; }
	/// Returns a pointer to the animation at index.
	FX_INLINE FxAnim* GetAnimPtr( FxSize index ) { return &_anims[index]; }
	
	/// Adds the animation to the group.
	FxBool AddAnim( const FxAnim& anim );
	/// Removes the animation named 'name'.
	FxBool RemoveAnim( const FxName& name );

	/// Serialization.
	virtual void Serialize( FxArchive& arc );

private:
	/// The animations in the group.
	FxArray<FxAnim> _anims;
};

FX_INLINE FxBool FxAnimGroup::operator==( const FxAnimGroup& other ) const
{
	return _name == other._name && _anims == other._anims;
}

FX_INLINE FxBool FxAnimGroup::operator!=( const FxAnimGroup& other ) const
{
	return !(operator==(other));
}

FX_INLINE FxSize FxAnimGroup::FindAnim( const FxName& name ) const
{
	FxSize numAnims = _anims.Length();
	for( FxSize i = 0; i < numAnims; ++i )
	{
		if( _anims[i].GetName() == name )
		{
			return i;
		}
	}
	return FxInvalidIndex;
}

} // namespace Face

} // namespace OC3Ent

#endif

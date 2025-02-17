//------------------------------------------------------------------------------
// A FaceFx animation.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxAnim.h"
#include "FxUtil.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxAnimBoneWeightVersion 0

// Serializes an FxAnimBoneWeight to an archive.
FxArchive& operator<<( FxArchive& arc, FxAnimBoneWeight& animBoneWeight )
{
	arc.SerializeClassVersion("FxAnimBoneWeight", FxTrue, kCurrentFxAnimBoneWeightVersion);
	arc << animBoneWeight.boneName << animBoneWeight.boneWeight;
	return arc;
}

#define kCurrentFxAnimVersion 6

FX_IMPLEMENT_CLASS(FxAnim, kCurrentFxAnimVersion, FxNamedObject)

FxAnim::FxAnim()
	: _startTime(0.0f)
	, _endTime(1.0f)
	, _blendInTime(0.16f)
	, _blendOutTime(0.22f)
	, _soundCueIndex(FxInvalidIndex)
	, _soundCuePointer(NULL)
{
}

FxAnim::FxAnim( const FxAnim& other )
	: Super(other)
	, FxDataContainer(other)
	, _animCurves(other._animCurves)
	, _startTime(other._startTime)
	, _endTime(other._endTime)
	, _blendInTime(other._blendInTime)
	, _blendOutTime(other._blendOutTime)
	, _boneWeights(other._boneWeights)
	, _soundCuePath(other._soundCuePath)
	, _soundNodeWave(other._soundNodeWave)
	, _soundCueIndex(other._soundCueIndex)
	, _soundCuePointer(other._soundCuePointer)
{
}

FxAnim& FxAnim::operator=( const FxAnim& other )
{
	if( this == &other ) return *this;
	Super::operator=(other);
	FxDataContainer::operator=(other);
	_animCurves      = other._animCurves;
	_startTime       = other._startTime;
	_endTime         = other._endTime;
	_blendInTime     = other._blendInTime;
	_blendOutTime    = other._blendOutTime;
	_boneWeights     = other._boneWeights;
	_soundCuePath    = other._soundCuePath;
	_soundNodeWave   = other._soundNodeWave;
	_soundCueIndex   = other._soundCueIndex;
	_soundCuePointer = other._soundCuePointer;
	return *this;
}

FxAnim::~FxAnim()
{
}

FxSize FxAnim::FindAnimCurve( const FxName& name ) const
{
	FxSize numAnimCurves = _animCurves.Length();
	for( FxSize i = 0; i < numAnimCurves; ++i )
	{
		if( _animCurves[i].GetName() == name )
		{
			return i;
		}
	}
	return FxInvalidIndex;
}

FxBool FxAnim::AddAnimCurve( const FxAnimCurve& curve )
{
	if( FindAnimCurve(curve.GetName()) == FxInvalidIndex )
	{
		_animCurves.PushBack(curve);
		FindBounds();
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxAnim::InsertAnimCurve( const FxAnimCurve& curve, FxSize index )
{
	// If the index wasn't provided or wasn't valid, append the curve.
	if( index == FxInvalidIndex || index >= _animCurves.Length() )
	{
		return AddAnimCurve(curve);
	}
	else if( FindAnimCurve(curve.GetName()) == FxInvalidIndex )
	{
		// Otherwise, insert the curve into its proper location.
		_animCurves.Insert(curve, index);
		FindBounds();
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxAnim::RemoveAnimCurve( const FxName& name )
{
	FxSize index = FindAnimCurve(name);
	if( index != FxInvalidIndex )
	{
		_animCurves.Remove(index);
		FindBounds();
		return FxTrue;
	}
	return FxFalse;
}

void FxAnim::SetBlendInTime( FxReal blendInTime )
{
	_blendInTime = blendInTime;
}

void FxAnim::SetBlendOutTime( FxReal blendOutTime )
{
	_blendOutTime = blendOutTime;
}

FxBool FxAnim::AddBoneWeight( const FxAnimBoneWeight& animBoneWeight )
{
	if( FxInvalidIndex == _findAnimBoneWeight(animBoneWeight.boneName) )
	{
		_boneWeights.PushBack(animBoneWeight);
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxAnim::RemoveBoneWeight( const FxName& boneName )
{
	FxSize index = _findAnimBoneWeight(boneName);
	if( FxInvalidIndex != index )
	{
		_boneWeights.Remove(index);
		return FxTrue;
	}
	return FxFalse;
}

void FxAnim::RemoveAllBoneWeights( void )
{
	_boneWeights.Clear();
}

void FxAnim::Serialize( FxArchive& arc )
{
	Super::Serialize(arc);

	FxUInt16 version = arc.SerializeClassVersion("FxAnim");
		
	if( arc.IsSaving() )
	{
#ifndef NO_SAVE_VERSION
		arc << _animCurves;

		// Optimized key saving.
		// Count the total number of keys and cache the number of keys for each curve.
		FxSize numCurves = _animCurves.Length();
		FxSize numKeys   = 0; 
		FxArray<FxSize> curveNumKeys;
		curveNumKeys.Reserve(numCurves);
		for( FxSize i = 0; i < numCurves; ++i )
		{
			FxSize tempNumKeys = _animCurves[i].GetNumKeys();
			curveNumKeys.PushBack(tempNumKeys);
			numKeys += tempNumKeys;
		}

		arc << numKeys;

		if( numKeys > 0 )
		{
			// Allocate the POD array.
			FxReal* pKeys = static_cast<FxReal*>(FxAlloc(numKeys * 4 * sizeof(FxReal), "OptimizedKeySaving"));

			// Fill in the POD array.
			FxSize keyIndex = 0;
			for( FxSize i = 0; i < numCurves; ++i )
			{
				for( FxSize k = 0; k < curveNumKeys[i]; ++k )
				{
					const FxAnimKey& key = _animCurves[i].GetKey(k);
					pKeys[keyIndex++] = key.GetTime();
					pKeys[keyIndex++] = key.GetValue();
					pKeys[keyIndex++] = key.GetSlopeIn();
					pKeys[keyIndex++] = key.GetSlopeOut();
				}
			}

			// Serialize.
			arc.SerializePODArray(pKeys, numKeys * 4);
			
			arc << curveNumKeys;

			// Cleanup.
			FxFree(pKeys, numKeys * 4 * sizeof(FxReal));
		}

		arc << _blendInTime  << _blendOutTime 
			<< _boneWeights << _soundCuePath << _soundNodeWave 
			<< _soundCueIndex;
#endif
	}
	else
	{
		if( version < 4 )
		{
			FxInt32 seekPos;
			arc << seekPos;
		}
		arc << _animCurves;
		if( version >= 6 )
		{
			// Optimized key loading.
			FxSize numKeys;
			arc << numKeys;

			if( numKeys > 0 )
			{
				// Allocate the POD array.
				FxReal* pKeys = static_cast<FxReal*>(FxAlloc(numKeys * 4 * sizeof(FxReal), "OptimizedKeyLoading"));

				arc.SerializePODArray(pKeys, numKeys * 4);

				FxArray<FxSize> curveNumKeys;
				arc << curveNumKeys;

				FxSize keysOffset = 0;
				FxSize numCurves  = _animCurves.Length();
				for( FxSize i = 0; i < numCurves; ++i )
				{
					_animCurves[i].SetKeys(pKeys + keysOffset, curveNumKeys[i]);
					keysOffset += 4 * curveNumKeys[i];
				}

				// Cleanup.
				FxFree(pKeys, numKeys * 4 * sizeof(FxReal));
			}
		}
		if( version < 5 )
		{
			// Trash what was once an array of 'bone curves' - those curve
			// types no longer exist and there was never a way to get them
			// into an animation, so simply read in the array version and
			// the length of the array which should *always* be zero.
			FxUInt16 dummyVersion = 0;
			FxSize   dummySize    = 0;
			arc << dummyVersion << dummySize;
			FxAssert(0 == dummySize);
		}
		arc << _blendInTime << _blendOutTime;
		if( version >= 2 )
		{
			arc << _boneWeights;
		}
		if( version >= 1 )
		{
			arc << _soundCuePath << _soundNodeWave;
		}
		if( version >= 3 )
		{
			arc << _soundCueIndex;
		}
		FindBounds();
	}
}

void FxAnim::SetSoundCuePath( const FxString& soundCuePath )
{
	_soundCuePath = soundCuePath;
}

void FxAnim::SetSoundNodeWave( const FxString& soundNodeWave )
{
	_soundNodeWave = soundNodeWave;
}

void FxAnim::SetSoundCueIndex( FxSize soundCueIndex )
{
	_soundCueIndex = soundCueIndex;
}

void FxAnim::SetSoundCuePointer( void* soundCuePointer )
{
	_soundCuePointer = soundCuePointer;
}

void FxAnim::FindBounds( void )
{
	// Start with a very large start time and a very small end time.
	_startTime = FX_REAL_MAX;
	_endTime   = FX_REAL_MIN;

	FxReal curveMinTime = 0.0f;
	FxReal curveMaxTime = 0.0f;

	// Make a pass through all the curve types in the animation.
	FxSize numAnimCurves = _animCurves.Length();
	for( FxSize i = 0; i < numAnimCurves; ++i )
	{
		_animCurves[i].FindTimeExtents(curveMinTime, curveMaxTime);
		_startTime = curveMinTime < _startTime ? curveMinTime : _startTime;
		_endTime   = curveMaxTime > _endTime ? curveMaxTime : _endTime;
	}
	
	// If no keys were found, clamp _startTime and _endTime to reasonable 
	// values.  0.0f and 1.0f were chosen so that creating an "empty" (silent)
	// animation in FaceFX Studio would result in a somewhat reasonable timeline
	// state.
	if( FX_REAL_MAX == _startTime && FX_REAL_MIN == _endTime )
	{
		_startTime = 0.0f;
		_endTime   = 1.0f;
	}
}

FxSize FxAnim::_findAnimBoneWeight( const FxName& boneName ) const
{
	FxSize numBoneWeights = _boneWeights.Length();
	for( FxSize i = 0; i < numBoneWeights; ++i )
	{
		if( _boneWeights[i].boneName == boneName )
		{
			return i;
		}
	}
	return FxInvalidIndex;
}

#define kCurrentFxAnimGroupVersion 0

FX_IMPLEMENT_CLASS(FxAnimGroup, kCurrentFxAnimGroupVersion, FxNamedObject)

FxName FxAnimGroup::Default;

FxAnimGroup::FxAnimGroup()
{
}

FxAnimGroup::FxAnimGroup( const FxAnimGroup& other )
	: Super(other)
	, _anims(other._anims)
{
}

FxAnimGroup& FxAnimGroup::operator=( const FxAnimGroup& other )
{
	if( this == &other ) return *this;
	Super::operator=(other);
	_anims = other._anims;
	return *this;
}

FxAnimGroup::~FxAnimGroup()
{
}

FxBool FxAnimGroup::AddAnim( const FxAnim& anim )
{
	if( FindAnim(anim.GetName()) == FxInvalidIndex )
	{
		_anims.PushBack(anim);
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxAnimGroup::RemoveAnim( const FxName& name )
{
	FxSize index = FindAnim(name);
	if( index != FxInvalidIndex )
	{
		_anims.Remove(index);
		return FxTrue;

	}
	return FxFalse;
}

void FxAnimGroup::Serialize( FxArchive& arc )
{
	Super::Serialize(arc);

	arc.SerializeClassVersion("FxAnimGroup");
	
	arc << _anims;
}

} // namespace Face

} // namespace OC3Ent

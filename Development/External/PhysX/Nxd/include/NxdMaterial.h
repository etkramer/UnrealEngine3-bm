#ifndef NX_DOUBLEBUFFER_NXD_MATERIAL
#define NX_DOUBLEBUFFER_NXD_MATERIAL
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/


#include "NxdAllocateable.h"
#include "NxMaterial.h"
#include "NxMaterialDesc.h"
#include "NxdCasts.h"

class NxdScene;

class NxdMaterial : public NxMaterial, public NxdAllocateable
{
public:
	NxMaterialIndex getMaterialIndex();

	NxScene&		getScene() const;

	void			loadFromDesc(const NxMaterialDesc& desc);
	void			saveToDesc(NxMaterialDesc& desc) const;
	
	void			setDynamicFriction(NxReal coef);
	NxReal			getDynamicFriction() const;
	
	void			setStaticFriction(NxReal coef);
	NxReal			getStaticFriction() const;
	
	void			setRestitution(NxReal rest);
	NxReal			getRestitution() const;
	
	void			setDynamicFrictionV(NxReal coef);
	NxReal			getDynamicFrictionV() const;
	
	void			setStaticFrictionV(NxReal coef);
	NxReal			getStaticFrictionV() const;
	
	void			setDirOfAnisotropy(const NxVec3 &vec);
	NxVec3			getDirOfAnisotropy() const;
	
	void			setFlags(NxU32 flags);
	NxU32			getFlags() const;

	void			setFrictionCombineMode(NxCombineMode combMode);
	NxCombineMode	getFrictionCombineMode() const;

	void			setRestitutionCombineMode(NxCombineMode combMode);
	NxCombineMode	getRestitutionCombineMode() const;

	NxMaterial *	getNxMaterial()	const	{	return mNxMaterial; }
	bool			isAwaitingDelete() const;	

private:
	friend class NxdScene;

	NxdMaterial(NxdScene &scene, const NxMaterialDesc &desc, NxMaterialIndex index);
	NxdMaterial(NxdScene &scene, NxMaterial *material);	// used to construct default material proxy
	~NxdMaterial();

	bool			commitChanges();
	bool			writable();
	bool			instance();

	NxdScene &		mNxdScene;
	NxU16			mIndex;
	NxU32			mBufferFlags;
	NxMaterial *	mNxMaterial;
	NxMaterialDesc  mDesc;
	NxU32			mDirtyIndex;
	class DirtyIndexer
	{
	public:
		static NxU32 &index(NxdMaterial &m)	{	return m.mDirtyIndex;		}
	};

	enum
	{
		BUFFERED_DESC					= 1<<0,
		BUFFERED_DYNAMICFRICTION		= 1<<1,
		BUFFERED_STATICFRICTION			= 1<<2,
		BUFFERED_RESTITUTION			= 1<<3,
		BUFFERED_DYNAMICFRICTIONV		= 1<<4,
		BUFFERED_STATICFRICTIONV		= 1<<5,
		BUFFERED_DIROFANISOTROPY		= 1<<6,
		BUFFERED_FLAGS					= 1<<7,
		BUFFERED_FRICTIONCOMBINEMODE	= 1<<8,
		BUFFERED_RESTITUTIONCOMBINEMODE	= 1<<9,
	};
};

NXD_DECLARE_BUFFERED_CASTS(NxMaterial)

#endif

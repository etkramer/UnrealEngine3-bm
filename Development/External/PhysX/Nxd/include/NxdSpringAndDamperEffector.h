#ifndef NX_DOUBLEBUFFER_NXD_SPRINGANDDAMPEREFFECTOR
#define NX_DOUBLEBUFFER_NXD_SPRINGANDDAMPEREFFECTOR
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/


#include "NxdAllocateable.h"
#include "NxSpringAndDamperEffector.h"
#include "NxSpringAndDamperEffectorDesc.h"
#include "NxdCasts.h"

class NxdSpringAndDamperEffector : public NxSpringAndDamperEffector, public NxdAllocateable
{
public:
	NxScene&		getScene() const;
	NxEffectorType  getType() const;
	void			setName(const char* name);
	const char*		getName() const;
	void			saveToDesc(NxSpringAndDamperEffectorDesc &desc);
	void			setBodies(NxActor* body1, const NxVec3  & global1, NxActor* body2, const NxVec3  & global2);
	void			setLinearSpring(NxReal distCompressSaturate, NxReal distRelaxed, NxReal distStretchSaturate, NxReal maxCompressForce, NxReal maxStretchForce);
	void			getLinearSpring(NxReal & distCompressSaturate, NxReal & distRelaxed, NxReal & distStretchSaturate, NxReal & maxCompressForce, NxReal & maxStretchForce);
	void			setLinearDamper(NxReal velCompressSaturate, NxReal velStretchSaturate, NxReal maxCompressForce, NxReal maxStretchForce);
	void			getLinearDamper(NxReal & velCompressSaturate, NxReal & velStretchSaturate, NxReal & maxCompressForce, NxReal & maxStretchForce);

	bool			isAwaitingDelete()	const;
	NxSpringAndDamperEffector *		
					getNxSpringAndDamperEffector()		const	{ return mEffector; }

private:
	friend class NxdScene;

	NxSpringAndDamperEffectorDesc	mDesc;
	NxSpringAndDamperEffector *		mEffector;
	NxdScene &						mNxdScene;
	NxU32							mSceneIndex;
	NxU32							mDirtyIndex;
	NxU32							mBufferedFlags;

	NxdSpringAndDamperEffector(NxdScene &scene, const NxSpringAndDamperEffectorDesc &desc);
	~NxdSpringAndDamperEffector();

	bool					instance();
	bool					commitChanges();
	bool					writable()							const;

	class SceneIndexer
	{
	public:
		static NxU32 &index(NxdSpringAndDamperEffector &s)	{	return s.mSceneIndex;		}
	};

	class DirtyIndexer
	{
	public:
		static NxU32 &index(NxdSpringAndDamperEffector &s)	{	return s.mDirtyIndex;		}
	};

	enum
	{
		BUFFERED_BODIES = 1<<0,
		BUFFERED_SPRING	= 1<<1,
		BUFFERED_DAMPER = 1<<2
	};
};

NXD_DECLARE_BUFFERED_CASTS(NxSpringAndDamperEffector)



#endif

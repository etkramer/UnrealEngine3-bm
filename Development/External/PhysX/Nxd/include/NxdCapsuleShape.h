#ifndef NX_DOUBLEBUFFER_NXD_CAPSULESHAPE
#define NX_DOUBLEBUFFER_NXD_CAPSULESHAPE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/


#include "NxdAllocateable.h"
#include "NxCapsuleShape.h"
#include "NxCapsuleShapeDesc.h"
#include "NxdShapeHelper.h"
#include "NxdCasts.h"

class NxdScene;
class NxScene;

class NxdCapsuleShape : public NxCapsuleShape, public NxdAllocateable
{
public:

	DECLARE_SHAPE_FORWARDING_METHODS

	void	setDimensions(NxReal radius, NxReal height);
	void	setRadius(NxReal radius);
	NxReal	getRadius()									const;
	void	setHeight(NxReal height);
	NxReal	getHeight()									const;
	void	getWorldCapsule(NxCapsule& worldCapsule)	const;
	void	saveToDesc(NxCapsuleShapeDesc& desc)		const;

private:
	friend class NxdShapeShared;
	friend class NxdScene;

	NxdCapsuleShape(NxdActor &, const NxCapsuleShapeDesc &);
	~NxdCapsuleShape() {};

	NxdShapeShared &		getShared()			{ return mShared;			}
	const NxdShapeShared &	getShared() const	{ return mShared;			}

	NxShapeType				getType()	const	{ return NX_SHAPE_CAPSULE;	}
	NxCapsuleShape *		getShape()	const	{ return static_cast<NxCapsuleShape *>(mShared.getNxShape()); }


	NxdShapeShared		mShared;
	NxCapsuleShapeDesc	mDesc;
};

NXD_DECLARE_BUFFERED_CASTS(NxCapsuleShape)

#endif

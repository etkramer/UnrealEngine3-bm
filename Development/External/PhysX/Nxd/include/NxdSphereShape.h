#ifndef NX_DOUBLEBUFFER_NXD_SPHERESHAPE
#define NX_DOUBLEBUFFER_NXD_SPHERESHAPE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/


#include "NxdAllocateable.h"
#include "NxSphereShape.h"
#include "NxSphereShapeDesc.h"
#include "NxdShapeHelper.h"
#include "NxdCasts.h"

class NxdScene;
class NxScene;

class NxdSphereShape : public NxSphereShape, public NxdAllocateable
{
public:

	DECLARE_SHAPE_FORWARDING_METHODS

	void		setRadius(NxReal radius);
	NxReal		getRadius()								const;
	void		getWorldSphere(NxSphere& worldSphere)	const;
	void		saveToDesc(NxSphereShapeDesc& desc)		const;


private:
	friend class NxdShapeShared;
	friend class NxdScene;

	NxdSphereShape(NxdActor &, const NxSphereShapeDesc &);
	~NxdSphereShape() {};

	NxdShapeShared &		getShared()			{ return mShared;			}
	const NxdShapeShared &	getShared() const	{ return mShared;			}

	NxShapeType				getType()	const	{ return NX_SHAPE_SPHERE;		}
	NxSphereShape *			getShape()	const	{ return static_cast<NxSphereShape *>(mShared.getNxShape()); }

	NxdShapeShared		mShared;
	NxSphereShapeDesc		mDesc;
};

NXD_DECLARE_BUFFERED_CASTS(NxSphereShape)

#endif

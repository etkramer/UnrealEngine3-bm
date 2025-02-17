#ifndef NX_DOUBLEBUFFER_NXD_PLANESHAPE
#define NX_DOUBLEBUFFER_NXD_PLANESHAPE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/


#include "NxdAllocateable.h"
#include "NxPlaneShape.h"
#include "NxPlaneShapeDesc.h"
#include "NxdShapeHelper.h"
#include "NxdCasts.h"

class NxdScene;
class NxScene;

class NxdPlaneShape : public NxPlaneShape, public NxdAllocateable
{
public:

	DECLARE_SHAPE_FORWARDING_METHODS

	NxPlane	getPlane()									const;
	void	setPlane(const NxVec3 & normal, NxReal d);
	void	saveToDesc(NxPlaneShapeDesc& desc)			const;


private:
	friend class NxdShapeShared;
	friend class NxdScene;

	NxdPlaneShape(NxdActor &, const NxPlaneShapeDesc &);
	~NxdPlaneShape() {};

	NxdShapeShared &		getShared()			{ return mShared;			}
	const NxdShapeShared &	getShared() const	{ return mShared;			}

	NxShapeType				getType()	const	{ return NX_SHAPE_PLANE;		}
	NxPlaneShape *			getShape()	const	{ return static_cast<NxPlaneShape *>(mShared.getNxShape()); }

	NxdShapeShared		mShared;
	NxPlaneShapeDesc		mDesc;
};

NXD_DECLARE_BUFFERED_CASTS(NxPlaneShape)

#endif

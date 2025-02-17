#ifndef NX_DOUBLEBUFFER_NXD_CONVEXSHAPE
#define NX_DOUBLEBUFFER_NXD_CONVEXSHAPE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/


#include "NxdAllocateable.h"
#include "NxConvexShape.h"
#include "NxConvexShapeDesc.h"
#include "NxdShapeHelper.h"
#include "NxdCasts.h"

class NxdScene;
class NxScene;

class NxdConvexShape : public NxConvexShape, public NxdAllocateable
{
public:

	DECLARE_SHAPE_FORWARDING_METHODS

	void				saveToDesc(NxConvexShapeDesc& desc)	const;
	NxConvexMesh&		getConvexMesh();
	const NxConvexMesh&	getConvexMesh() const;

private:
	friend class NxdShapeShared;
	friend class NxdScene;

	NxdConvexShape(NxdActor &, const NxConvexShapeDesc &);
	~NxdConvexShape() {};

	NxdShapeShared &		getShared()			{ return mShared;			}
	const NxdShapeShared &	getShared() const	{ return mShared;			}

	NxShapeType				getType()	const	{ return NX_SHAPE_CONVEX;	}
	NxConvexShape *			getShape()	const	{ return static_cast<NxConvexShape *>(mShared.getNxShape()); }

	NxdShapeShared		mShared;
	NxConvexShapeDesc		mDesc;
};

NXD_DECLARE_BUFFERED_CASTS(NxConvexShape)

#endif

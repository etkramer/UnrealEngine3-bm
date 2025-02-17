#ifndef NX_DOUBLEBUFFER_NXD_BOXSHAPE
#define NX_DOUBLEBUFFER_NXD_BOXSHAPE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/


#include "NxdAllocateable.h"
#include "NxBoxShape.h"
#include "NxBoxShapeDesc.h"
#include "NxdShapeHelper.h"
#include "NxdCasts.h"

class NxdScene;
class NxScene;

class NxdBoxShape : public NxBoxShape, public NxdAllocateable
{
public:

	DECLARE_SHAPE_FORWARDING_METHODS

	void setDimensions(const NxVec3& vec);
	NxVec3 getDimensions()						const;
	void getWorldOBB(NxBox& obb)				const;
	void saveToDesc(NxBoxShapeDesc& desc)		const;

private:
	friend class NxdScene;
	friend class NxdShapeShared;

	NxdBoxShape(NxdActor &, const NxBoxShapeDesc &);
	~NxdBoxShape() {};

	NxdShapeShared &		getShared()			{ return mShared;			}
	const NxdShapeShared &	getShared() const	{ return mShared;			}

	NxShapeType				getType()	const	{ return NX_SHAPE_BOX;		}
	NxBoxShape *			getShape()	const	{ return static_cast<NxBoxShape *>(mShared.getNxShape()); }


	NxdShapeShared		mShared;
	NxBoxShapeDesc		mDesc;
};

NXD_DECLARE_BUFFERED_CASTS(NxBoxShape)
#endif

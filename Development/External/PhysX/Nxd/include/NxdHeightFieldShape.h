#ifndef NX_DOUBLEBUFFER_NXD_HEIGHTFIELDSHAPE
#define NX_DOUBLEBUFFER_NXD_HEIGHTFIELDSHAPE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/


#include "NxdAllocateable.h"
#include "NxHeightFieldShape.h"
#include "NxHeightFieldShapeDesc.h"
#include "NxdShapeHelper.h"
#include "NxdCasts.h"

class NxdScene;
class NxScene;

class NxdHeightFieldShape : public NxHeightFieldShape, public NxdAllocateable
{
public:

	DECLARE_SHAPE_FORWARDING_METHODS


	void				saveToDesc(NxHeightFieldShapeDesc& desc)	const;

	NxHeightField&		getHeightField()							const;
	NxReal				getHeightScale()							const;
	NxReal				getRowScale()								const;
	NxReal				getColumnScale()							const;
	void				setHeightScale(NxReal scale);
	void				setRowScale(NxReal scale);
	void				setColumnScale(NxReal scale);
	NxU32				getTriangle(NxTriangle& worldTri, NxTriangle* edgeTri, NxU32* flags, NxTriangleID triangleIndex, bool worldSpaceTranslation=true, bool worldSpaceRotation=true) const	;
	bool				overlapAABBTrianglesDeprecated(const NxBounds3& bounds, NxU32 flags, NxU32& nb, const NxU32*& indices)	const	;
	bool				overlapAABBTriangles(const NxBounds3& bounds, NxU32 flags, NxUserEntityReport<NxU32>* callback) const;
	bool				isShapePointOnHeightField(NxReal x, NxReal z) const;
	NxReal				getHeightAtShapePoint(NxReal x, NxReal z) const;
	NxMaterialIndex		getMaterialAtShapePoint(NxReal x, NxReal z) const;
	NxVec3				getNormalAtShapePoint(NxReal x, NxReal z) const;
	NxVec3				getSmoothNormalAtShapePoint(NxReal x, NxReal z) const;

private:
	friend class NxdShapeShared;
	friend class NxdScene;

	NxdHeightFieldShape(NxdActor &, const NxHeightFieldShapeDesc &);
	~NxdHeightFieldShape() {};

	NxdShapeShared &		getShared()			{ return mShared;			}
	const NxdShapeShared &	getShared() const	{ return mShared;			}

	NxShapeType				getType()	const	{ return NX_SHAPE_HEIGHTFIELD;	}
	NxHeightFieldShape *	getShape()	const	{ return static_cast<NxHeightFieldShape *>(mShared.getNxShape()); }

	NxdShapeShared		mShared;
	NxHeightFieldShapeDesc		mDesc;
};

NXD_DECLARE_BUFFERED_CASTS(NxHeightFieldShape)

#endif

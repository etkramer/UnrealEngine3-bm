#ifndef NX_DOUBLEBUFFER_NXD_TRIANGLEMESHSHAPE
#define NX_DOUBLEBUFFER_NXD_TRIANGLEMESHSHAPE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/


#include "NxdAllocateable.h"
#include "NxTriangleMeshShape.h"
#include "NxTriangleMeshShapeDesc.h"
#include "NxdShapeHelper.h"
#include "NxdCasts.h"

class NxdScene;
class NxScene;

class NxdTriangleMeshShape : public NxTriangleMeshShape, public NxdAllocateable
{
public:

	DECLARE_SHAPE_FORWARDING_METHODS

	public:
	void					saveToDesc(NxTriangleMeshShapeDesc& desc)	const;
	NxTriangleMesh&			getTriangleMesh();
	const NxTriangleMesh&	getTriangleMesh() const;
	NxU32					getTriangle(NxTriangle& triangle, NxTriangle* edgeTri, NxU32* flags, NxTriangleID triangleIndex, bool worldSpaceTranslation=true, bool worldSpaceRotation=true) const;

	bool					overlapAABBTrianglesDeprecated(const NxBounds3& bounds, NxU32 flags, NxU32& nb, const NxU32*& indices)	const;
	bool					overlapAABBTriangles(const NxBounds3& bounds, NxU32 flags, NxUserEntityReport<NxU32>* callback) const;
	bool					mapPageInstance(NxU32 pageIndex);
	void					unmapPageInstance(NxU32 pageIndex);
	bool					isPageInstanceMapped(NxU32 pageIndex) const;


private:
	friend class NxdShapeShared;
	friend class NxdScene;

	NxdTriangleMeshShape(NxdActor &, const NxTriangleMeshShapeDesc &);
	~NxdTriangleMeshShape() {};

	NxdShapeShared &		getShared()			{ return mShared;			}
	const NxdShapeShared &	getShared() const	{ return mShared;			}

	NxShapeType				getType()	const	{ return NX_SHAPE_MESH;		}
	NxTriangleMeshShape *	getShape()	const	{ return static_cast<NxTriangleMeshShape *>(mShared.getNxShape()); }

	NxdShapeShared		mShared;
	NxTriangleMeshShapeDesc		mDesc;
};

NXD_DECLARE_BUFFERED_CASTS(NxTriangleMeshShape)

#endif

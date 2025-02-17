#ifndef NX_DOUBLEBUFFER_NXD_CASTS
#define NX_DOUBLEBUFFER_NXD_CASTS
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

class NxActor;
class NxScene;
class NxShape;
class NxJoint;
class NxMaterial;
class NxSpringAndDamperEffector;
class NxForceField;


bool NxdIsBuffered(const NxActor *);
bool NxdIsBuffered(const NxShape *);
bool NxdIsBuffered(const NxScene *);
bool NxdIsBuffered(const NxJoint *);
bool NxdIsBuffered(const NxMaterial *);
bool NxdIsBuffered(const NxSpringAndDamperEffector *);
bool NxdIsBuffered(const NxForceField *);

enum NxdBufferingType
{
	NXD_HAS_NO_BUFFERING = 0,		// raw NxScene created through NxPhysicsSDK::createScene
	NXD_IS_UNBUFFERED = 1,			// unbuffered scene with buffered counterpart
	NXD_IS_BUFFERED = 2				// buffered scene with unbuffered counterpart
};

NxdBufferingType NxdGetBufferingType(const NxActor *);
NxdBufferingType NxdGetBufferingType(const NxShape *);
NxdBufferingType NxdGetBufferingType(const NxScene *);
NxdBufferingType NxdGetBufferingType(const NxJoint *);

bool NxdIsAwaitingDelete(NxActor *);
bool NxdIsAwaitingDelete(NxShape *);
bool NxdIsAwaitingDelete(NxJoint *);

#define NXD_DECLARE_BUFFERED_CASTS(_type)				\
_type *NxdGetBuffered(_type *);							\
const _type *NxdGetBuffered(const _type *);				\
_type *NxdGetUnbuffered(_type *);						\
const _type *NxdGetUnbuffered(const _type *);			\

#endif

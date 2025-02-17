//------------------------------------------------------------------------------
// A built-in FaceFx class.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxClass_H__
#define FxClass_H__

#include "FxPlatform.h"
#include "FxString.h"
#include "FxArray.h"
#include "FxName.h"
#include "FxUtil.h"
#include "FxMemory.h"

namespace OC3Ent
{

namespace Face
{

// Forward declare FxObject.
class FxObject;

/// Class information for an FxObject, for serialization and RTTI. 
/// A static FxClass pointer is contained inside
/// of each object derived from FxObject, as part of the rather involved 
/// solution to the problem of serializing pointers to objects.  It also provides
/// very basic RTTI capabilities.
/// \ingroup object
class FxClass : public FxUseAllocator
{
public:
	/// The function signature for object construction.
	typedef FxObject* (FX_CALL *FxConstructObjectFunc)(void);

	/// Constructor.
	FxClass( const FxChar* className, FxUInt16 currentVersion, 
			 const FxClass* pBaseClassDesc, FxSize classSize, 
			 FxConstructObjectFunc constructObjectFn );
	/// Destructor.
	~FxClass();

	/// Starts up the FaceFX class system.
	/// \note For internal use only.  Never call this directly.
	static void FX_CALL Startup( void );
	/// Shuts down the FaceFX class system.
	/// \note For internal use only.  Never call this directly.
	static void FX_CALL Shutdown( void );

	/// Returns the name of the class.
	const FxName& GetName( void ) const;
	/// Returns the current version of the class.
	const FxUInt16 GetCurrentVersion( void ) const;
	/// Returns a pointer to this class' base class (\p NULL if none).
	const FxClass* GetBaseClassDesc( void ) const;
	/// Returns the size of the class in memory.
	const FxSize GetSize( void ) const;
	/// Returns the number of classes that directly derive from the class.
	const FxSize GetNumChildren( void ) const;
	/// Constructs an object of this class and returns a pointer to it (or \p NULL).
	FxObject* ConstructObject( void ) const;
	/// Returns a pointer to an FxClass given a class name (\p NULL if not found).
	static const FxClass* FX_CALL FindClassDesc( const FxName& className );
	/// Returns the total number of classes in the system.
	static FxSize FX_CALL GetNumClasses( void );
	/// Returns the class at index.
	static const FxClass* FX_CALL GetClass( FxSize index );

	/// Tests if the class is of a specific type.
	FxBool IsExactKindOf( const FxClass* classDesc ) const;
	/// Tests if the class is derived from a specific type (or is that type itself).
	FxBool IsKindOf( const FxClass* classDesc ) const;
	/// Tests if this class is a specific type.
	FxBool IsA( const FxClass* classDesc ) const;
	
private:
	/// The name of the class.
	FxName _name;
	/// The current version of the class.
	FxUInt16 _currentVersion;
	/// A pointer to this class' base class.
	const FxClass* _pBaseClassDesc;
	/// The number of classes directly deriving from this class.
	FxSize _numChildren;
	/// The size of this class.
	FxSize _size;
	/// A pointer to a function that can construct an object of this class.
	FxConstructObjectFunc _constructObjectFn;
	/// An array of all known FaceFX classes.
	static FxArray<FxClass*>* _classTable;

	/// Disable copy construction.
	FxClass( const FxClass& other );
	/// Disable assignment.
	FxClass& operator=( const FxClass& other );
};

/// \def FX_MAKE_CLASS_SERIALIZABLE(classname)
/// Defines a class-specific overload of the output operator to support
/// object serialization.
/// \ingroup object
#define FX_MAKE_CLASS_SERIALIZABLE(classname) \
	public: \
	friend ::OC3Ent::Face::FxArchive& operator<<( ::OC3Ent::Face::FxArchive& arc, classname*& obj ) \
	{ \
		/* If we're loading, we need to see if the object at this index 
		 * in the object table is valid.  If it is, we should return a 
		 * pointer to it.  If it isn't, the object is in the archive 
		 * immediately following the index we just loaded, so we should 
		 * load it, put it into the object table, and return a pointer 
		 * to it. 
		 */ \
		if( arc.IsLoading() ) \
		{ \
			/* Load the object index. */ \
			::OC3Ent::Face::FxSize index = ::OC3Ent::Face::FxInvalidIndex; \
			arc << index; \
			/* If it is a valid index, see if it should be loaded. */ \
			if( index != ::OC3Ent::Face::FxInvalidIndex ) \
			{ \
				/* Check to see if the object has been loaded yet. */ \
				::OC3Ent::Face::FxObject* foundObject = arc.FindObject(index); \
				if( foundObject ) \
				{ \
					/* It was in the table, so get the pointer to it. */ \
					obj = static_cast<classname*>(foundObject); \
				} \
				/* If the object isn't in the table, load it from the arc  
				 * and add it to the table. 
				 */ \
				else \
				{ \
					const ::OC3Ent::Face::FxClass* objectClass = NULL; \
					/* As of version 1.6 class names are stored as FxNames rather
					 * than FxStrings.
					 */ \
					if( arc.GetSDKVersion() >= 1600 ) \
					{ \
						/* Serialize the class information. */ \
						::OC3Ent::Face::FxName className; \
						arc << className; \
						/* Find the appropriate constructor for this class. */ \
						objectClass = ::OC3Ent::Face::FxClass::FindClassDesc(className); \
					} \
					else \
					{ \
						/* Serialize the class information. */ \
						::OC3Ent::Face::FxString classNameStr; \
						arc << classNameStr; \
						/* Find the appropriate constructor for this class. */ \
						objectClass = ::OC3Ent::Face::FxClass::FindClassDesc(classNameStr.GetData()); \
					} \
					::OC3Ent::Face::FxSize classSize; \
					arc << classSize; \
					if( objectClass ) \
					{ \
						::OC3Ent::Face::FxObject* newObject = objectClass->ConstructObject(); \
						if( newObject ) \
						{ \
							newObject->Serialize(arc); \
							obj = static_cast<classname*>(newObject); \
							arc.AddObject(obj, index); \
						} \
						else \
						{ \
							FxAssert(!"Unable to construct FxObject!"); \
						} \
					} \
					else \
					{ \
						FxAssert(!"Unknown FxClass in archive!"); \
					} \
				} \
			} \
			/* It wasn't a valid index, so it must be NULL. */ \
			else \
			{ \
				obj = NULL; \
			} \
		} \
		/* If we're saving, we need to see if we have saved this pointer
		 * before.  If we have, we should skip serializing the object.
		 * If we haven't, we should serialize the object and make a note
		 * that we have saved it.
		 */ \
		else \
		{ \
			if( obj ) \
			{ \
				/* See if we have saved this object already. */ \
				::OC3Ent::Face::FxBool needToSaveObject = \
				(arc.FindObject(obj) == ::OC3Ent::Face::FxInvalidIndex); \
				/* AddObject won't add duplicates. */ \
				::OC3Ent::Face::FxSize objIndex = arc.AddObject(obj); \
				/* Serialize the object index. */ \
				arc << objIndex; \
				/* If this is the first time we have saved this object,
				 * serialize it.
				 */ \
				if( needToSaveObject ) \
				{ \
					/* Serialize the class information. */ \
					::OC3Ent::Face::FxName className = \
						obj->GetClassDesc()->GetName(); \
					::OC3Ent::Face::FxSize classSize = obj->GetClassDesc()->GetSize(); \
					arc << className << classSize; \
					obj->Serialize(arc); \
				} \
			} \
			/* The object pointer is NULL, so save an invalid index. */ \
			else \
			{ \
				::OC3Ent::Face::FxSize index = ::OC3Ent::Face::FxInvalidIndex; \
				arc << index; \
			} \
		} \
		return arc; \
	} \
	friend ::OC3Ent::Face::FxArchive& operator<<( ::OC3Ent::Face::FxArchive& arc, classname& obj ) \
	{ \
		obj.Serialize(arc); \
		return arc; \
	} \
	private:

/// Defines some common class functionality.
/// \ingroup object
#define FX_MAKE_CLASS(classname) \
	private: \
		/* The class. */ \
		static ::OC3Ent::Face::FxClass* _pClassDesc; \
	public: \
		/* Allocates an instance of the class description. */ \
		static ::OC3Ent::Face::FxClass* FX_CALL AllocateStaticClass##classname( void ); \
		/* Returns a pointer to the static class. */ \
		static const ::OC3Ent::Face::FxClass* FX_CALL StaticClass( void ) \
		{ \
			if( !_pClassDesc ) \
			{ \
				_pClassDesc = AllocateStaticClass##classname(); \
			} \
			return _pClassDesc; \
		} \
		/* Returns a pointer to the class. */ \
		virtual const ::OC3Ent::Face::FxClass* GetClassDesc( void ) const \
		{ \
			/* Make sure _pClassDesc is valid. */ \
			if( !_pClassDesc ) \
			{ \
				_pClassDesc = AllocateStaticClass##classname(); \
			} \
			return _pClassDesc; \
		} \
		/* Returns the current version of the class. */ \
		virtual const ::OC3Ent::Face::FxUInt16 GetCurrentVersion( void ) const \
		{ \
			/* Make sure _pClassDesc is valid. */ \
			if( !_pClassDesc ) \
			{ \
				_pClassDesc = AllocateStaticClass##classname(); \
			} \
			return _pClassDesc->GetCurrentVersion(); \
		} \
		/* In the functions below, FxTrue and FxFalse are replaced with 1 and 0 
		 * static casted to the FxBool type because when FX_USE_BUILT_IN_BOOL_TYPE 
		 * is defined they are preprocessor defines but when it is not
		 * defined they are scoped in the ::OC3Ent::Face namespace.
		 * Writing the code this way prevents the nastiness of special 
		 * case scoping code here. */ \
		/* Tests if this class is of a specific type. */ \
		::OC3Ent::Face::FxBool IsExactKindOf( const ::OC3Ent::Face::FxClass* classDesc ) const \
		{ \
			const ::OC3Ent::Face::FxClass* testClass = GetClassDesc(); \
			if( testClass ) \
			{ \
				return testClass->IsExactKindOf(classDesc); \
			} \
			return static_cast< ::OC3Ent::Face::FxBool >(0); \
		} \
		/* Tests if this class is derived from a specific type (or is that
         * type itself).
		 */ \
		::OC3Ent::Face::FxBool IsKindOf( const ::OC3Ent::Face::FxClass* classDesc ) const \
		{ \
			const ::OC3Ent::Face::FxClass* testClass = GetClassDesc(); \
			if( testClass ) \
			{ \
				return testClass->IsKindOf(classDesc); \
			} \
			return static_cast< ::OC3Ent::Face::FxBool >(0); \
		} \
		/* Tests if this class is a specific type. */ \
		::OC3Ent::Face::FxBool IsA( const ::OC3Ent::Face::FxClass* classDesc ) const \
		{ \
			return IsExactKindOf(classDesc); \
		} \
		private:

/// Defines a class that can be dynamically created.
/// To serialize pointers to objects the object's class must be able to be dynamically created.
/// \ingroup object
#define FX_MAKE_DYNAMIC_CLASS(classname) \
	public: \
	/* Constructs and returns a pointer to an object of this class. */ \
	static ::OC3Ent::Face::FxObject* FX_CALL ConstructObject( void ) \
	{ \
		return new classname; \
	} \
	private:

/// Defines a class that should not be dynamically created.
/// \ingroup object
#define FX_MAKE_NON_DYNAMIC_CLASS(classname) \
	public: \
	/* Assert if this is ever called and return NULL. */ \
	static ::OC3Ent::Face::FxObject* FX_CALL ConstructObject( void ) \
	{ \
		FxAssert(!"Attempt to dynamically create an object whose class was declared as non-dynamic!"); \
		return NULL; \
	} \
	private:

/// Defines a class that should not be serialized.
/// \ingroup object
#define FX_MAKE_CLASS_NON_SERIALIZABLE(classname) \
	public: \
	/* Assert if this is ever called. */ \
	virtual void Serialize( FxArchive& FxUnused(arc) ) \
	{ \
		FxAssert(!"Attempt to serialize an object whose class was declared as non-serializable!"); \
	} \
	private:

/// Declares and defines a protected default constructor.
/// \ingroup object
#define FX_NO_DEFAULT_CONSTRUCTOR(classname) \
	protected: \
		classname() {} \
	private:

/// Declares (but doesn't define) a private copy constructor and assignment 
/// operator to prevent assignment or copying.
/// \ingroup object
#define FX_NO_COPY_OR_ASSIGNMENT(classname) \
	private: \
		classname( const classname& other ); \
		classname& operator=( const classname& other ); 

/// Defines a base class.
/// \ingroup object
#define FX_DECLARE_BASE_CLASS(classname) \
		FX_MAKE_CLASS(classname) \
		FX_MAKE_DYNAMIC_CLASS(classname) \
		FX_MAKE_CLASS_SERIALIZABLE(classname)

/// Defines a class.
/// \ingroup object
#define FX_DECLARE_CLASS(classname,baseclassname) \
	    FX_MAKE_CLASS(classname) \
	    FX_MAKE_DYNAMIC_CLASS(classname) \
	    FX_MAKE_CLASS_SERIALIZABLE(classname) \
	public: \
		typedef baseclassname Super; \
	private:

/// Defines a non-dynamic class.
/// \ingroup object
#define FX_DECLARE_CLASS_NO_DYNCREATE(classname,baseclassname) \
		FX_MAKE_CLASS(classname) \
		FX_MAKE_NON_DYNAMIC_CLASS(classname) \
		FX_MAKE_CLASS_SERIALIZABLE(classname) \
	public: \
		typedef baseclassname Super; \
	private:

/// Defines a non-serializable class.
/// \ingroup object
#define FX_DECLARE_CLASS_NO_SERIALIZE(classname,baseclassname) \
		FX_MAKE_CLASS(classname) \
		FX_MAKE_DYNAMIC_CLASS(classname) \
		FX_MAKE_CLASS_NON_SERIALIZABLE(classname) \
	public: \
		typedef baseclassname Super; \
	private:

/// Defines a non-dynamic, non-serializable class.
/// \ingroup object
#define FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(classname,baseclassname) \
		FX_MAKE_CLASS(classname) \
		FX_MAKE_NON_DYNAMIC_CLASS(classname) \
		FX_MAKE_CLASS_NON_SERIALIZABLE(classname) \
	public: \
		typedef baseclassname Super; \
	private:

/// Implements a base class.
/// \ingroup object
#define FX_IMPLEMENT_BASE_CLASS(baseclassname,version) \
	/* Initialize the class description pointer to NULL. */ \
	::OC3Ent::Face::FxClass* baseclassname::_pClassDesc = NULL; \
	/* Allocates an instance of the class description. */ \
	::OC3Ent::Face::FxClass* FX_CALL baseclassname::AllocateStaticClass##baseclassname( void ) \
	{ \
		return new ::OC3Ent::Face::FxClass(#baseclassname,version,NULL, \
			sizeof(baseclassname),&baseclassname::ConstructObject); \
	}

/// Implements a class.
/// \ingroup object
#define FX_IMPLEMENT_CLASS(classname,version,baseclassname) \
	/* Initialize the class description pointer to NULL. */ \
	::OC3Ent::Face::FxClass* classname::_pClassDesc = NULL; \
	/* Allocates an instance of the class description. */ \
	::OC3Ent::Face::FxClass* FX_CALL classname::AllocateStaticClass##classname( void ) \
	{ \
		return new ::OC3Ent::Face::FxClass(#classname,version,baseclassname::StaticClass(), \
			sizeof(classname),&classname::ConstructObject); \
	}

} // namespace Face

} // namespace OC3Ent

#endif

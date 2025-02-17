//------------------------------------------------------------------------------
// Defines various type traits.  Based on the boost type_traits implementation.
//
// Owner: John Briggs
// 
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxTypeTraits_H__
#define FxTypeTraits_H__

#include "FxPlatform.h"

namespace OC3Ent
{

namespace Face
{

/// Holds various utility structures for compile-time type traits determination.
/// The type traits system is used mainly to speed up loading of arrays of 
/// simple types by serializing the entire array \e en \e masse.  The system is
/// roughly based on the Boost type traits system.
namespace FxTypeTraits
{

/// A struct that can resolve to a value during compilation.
template<class T, T val> struct constant_type
{
	typedef constant_type<T, val> 	type;
	typedef T 						value_type;
	enum
	{
		value = val
	};
};

/// A struct that resolves to true during compilation.
typedef constant_type<FxBool, FxTrue>  true_type;
/// A struct that resolves to false during compilation.
typedef constant_type<FxBool, FxFalse> false_type;

/// The simplest of the type traits, is_void.
template<typename T> struct is_void : public false_type {};
template<> struct is_void<void> : public true_type {};

#if defined(FX_PLATFORM_SUPPORTS_PARTIAL_TEMPLATE_SPECIALIZATION)
/// Whether or not a type is a pointer type.
template<typename T> struct is_pointer	   : public false_type {};
template<typename T> struct is_pointer<T*> : public true_type {};
#else

// Based on code by Adobe:  http://opensource.adobe.com
/// Workaround for compilers without partial template specialization.
namespace FxIntimate 
{
	struct FxPointerShim
	{
		// Since the compiler only allows at most one non-trivial
		// implicit conversion we can make use of a shim class to
		// be sure that IsPtr below doesn't accept classes with
		// implicit pointer conversion operators.
		FxPointerShim(const volatile void*); // No implementation.
	};

	// These are the discriminating functions
	char IsPtr(FxPointerShim);
	int IsPtr(...);
} // namespace FxIntimate

/// Whether or not a type is a pointer type.
template <typename T> struct is_pointer
{
	typedef T value_type;
	// This template meta function takes a type T
	// and returns true exactly when T is a pointer.
	enum
	{
		value = (sizeof(FxIntimate::IsPtr(*(T*)0)) == 1)
	};
};
#endif // defined(FX_PLATFORM_SUPPORTS_PARTIAL_TEMPLATE_SPECIALIZATION)

/// Whether or not a type is an arithmetic type.
template<typename T> struct is_arithmetic	: public false_type {};
template<> struct is_arithmetic<FxByte>		: public true_type {};
template<> struct is_arithmetic<FxChar>		: public true_type {};
template<> struct is_arithmetic<FxInt16>	: public true_type {};
template<> struct is_arithmetic<FxUInt16>	: public true_type {};
template<> struct is_arithmetic<FxInt32>	: public true_type {};
template<> struct is_arithmetic<FxUInt32>	: public true_type {};
template<> struct is_arithmetic<FxReal>		: public true_type {};
template<> struct is_arithmetic<FxDReal>	: public true_type {};
#ifdef FX_USE_BUILT_IN_BOOL_TYPE
template<> struct is_arithmetic<FxBool>		: public true_type {};
#endif

/// Whether or not a type is a fundamental type.
template<typename T> struct is_fundamental : 
	public constant_type< FxBool, is_arithmetic<T>::value || is_void<T>::value > {};

/// Whether or not a type has a trivial assignment operator.
template<typename T> struct has_trivial_assign : 
	public constant_type< FxBool, is_arithmetic<T>::value || is_pointer<T>::value > {};

/// Whether or not a type has a trivial constructor
template<typename T> struct has_trivial_constructor : 
	public constant_type< FxBool, is_arithmetic<T>::value || is_pointer<T>::value > {};

/// Whether or not a type has a trivial copy-constructor.
template<typename T> struct has_trivial_copy : 
	public constant_type< FxBool, is_arithmetic<T>::value || is_pointer<T>::value > {};

/// Whether or not a type has a trivial destructor.
template<typename T> struct has_trivial_destructor : 
	public constant_type< FxBool, is_arithmetic<T>::value || is_pointer<T>::value > {};

} // namespace FxTypeTraits

} // namespace Face
 
} // namespace OC3Ent

#endif

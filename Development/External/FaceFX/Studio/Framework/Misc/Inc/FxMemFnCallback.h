//------------------------------------------------------------------------------
// Implements several useful pointer-to-member function callback functors
//
// Note: Any new derivation from FxMemFnCallback is responsible for overloading
// the appropriate signature of operator(), as well as providing its own storage
// for the actual pointer-to-member function (since the signature may not be the
// same as the one of the base class). (See FxProgressCallback for an example.)
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMemFnCallback_H__
#define FxMemFnCallback_H__

#include "FxPlatform.h"

namespace OC3Ent
{

namespace Face
{

class FxFunctor
{
public:
	virtual ~FxFunctor()
	{
	}
	
	virtual void operator()(void)
	{
	}
	virtual void operator()(FxInt32)
	{
	}
	virtual void operator()(FxReal)
	{
	}
};

template<class ClassName> class FxMemFnCallback : public FxFunctor
{
public:
	FxMemFnCallback( ClassName* pObject, void (ClassName::*fn)(void) = NULL )
		: _pObject( pObject )
		, _fn( fn )
	{
	}

	void operator()(void)
	{
		(*_pObject.*_fn)();
	}

protected:
	ClassName* _pObject;
private:
	void (ClassName::*_fn)(void);
};

template<class ClassName> class FxProgressCallback : public FxMemFnCallback<ClassName>
{
public:

	FxProgressCallback( ClassName* pObject, void (ClassName::*fn)(FxReal) )
		: FxMemFnCallback<ClassName>( pObject )
		, _fn( fn )
	{
	}

	void operator()( FxReal progress )
	{
		(*FxMemFnCallback<ClassName>::_pObject.*_fn)(progress);
	}
private:
	void (ClassName::*_fn)(FxReal);
};


} // namespace Face

} // namespace OC3Ent

#endif

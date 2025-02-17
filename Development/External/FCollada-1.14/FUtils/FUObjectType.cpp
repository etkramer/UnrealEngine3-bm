/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FUtils/FUObjectType.h"

FUObjectType::FUObjectType(const char* UNUSED_NDEBUG(_typeName))
: parent(NULL)
{
#ifdef UE3_DEBUG
	typeName = _typeName;
#endif
}

FUObjectType::FUObjectType(const FUObjectType& _parent, const char* UNUSED_NDEBUG(_typeName))
: parent(&_parent)
{
#ifdef UE3_DEBUG
	typeName = _typeName;
#endif
}

bool FUObjectType::Includes(const FUObjectType& otherType) const
{
	if (otherType == *this) return true;
	else if (parent != NULL) return parent->Includes(otherType);
	else return false;
}

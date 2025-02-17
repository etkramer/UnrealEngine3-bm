/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDEffectParameter.h"
#include "FCDocument/FCDEffectParameterFactory.h"
#include "FCDocument/FCDEffectParameterList.h"

ImplementObjectType(FCDEffectParameterList);

FCDEffectParameterList::FCDEffectParameterList(FCDocument* document, bool _ownParameters)
:	FCDObject(document)
,	ownParameters(_ownParameters)
{
}

FCDEffectParameterList::~FCDEffectParameterList()
{
	if (ownParameters)
	{
		while (!empty())
		{
			FCDEffectParameter* p = back();
			SAFE_DELETE(p);
		}
	}
	clear();
	ownParameters = false;
}

FCDEffectParameter* FCDEffectParameterList::AddParameter(uint32 type)
{
	FCDEffectParameter* parameter = NULL;
	if (ownParameters)
	{
		parameter = FCDEffectParameterFactory::Create(GetDocument(), type);
		push_back(parameter);
	}
	SetDirtyFlag();
	return parameter;
}

FCDEffectParameter* FCDEffectParameterList::FindReference(const char* reference)
{
	for (iterator it = begin(); it != end(); ++it)
	{
		if ((*it)->GetReference() == reference) return (*it);
	}
	return NULL;
}
const FCDEffectParameter* FCDEffectParameterList::FindReference(const char* reference) const
{
	for (const_iterator it = begin(); it != end(); ++it)
	{
		if ((*it)->GetReference() == reference) return (*it);
	}
	return NULL;
}

FCDEffectParameter* FCDEffectParameterList::FindSemantic(const char* semantic)
{
	for (iterator it = begin(); it != end(); ++it)
	{
		if ((*it)->GetSemantic() == semantic) return (*it);
	}
	return NULL;
}
const FCDEffectParameter* FCDEffectParameterList::FindSemantic(const char* semantic) const
{
	for (const_iterator it = begin(); it != end(); ++it)
	{
		if ((*it)->GetSemantic() == semantic) return (*it);
	}
	return NULL;
}

void FCDEffectParameterList::FindReference(const char* reference, FCDEffectParameterList& list)
{
	for (iterator it = begin(); it != end(); ++it)
	{
		if ((*it)->GetReference() == reference) list.push_back(*it);
	}
}

void FCDEffectParameterList::FindSemantic(const char* semantic, FCDEffectParameterList& list)
{
	for (iterator it = begin(); it != end(); ++it)
	{
		if ((*it)->GetSemantic() == semantic) list.push_back(*it);
	}
}

void FCDEffectParameterList::FindType(uint32 type, FCDEffectParameterList& list) const
{
	for (const_iterator it = begin(); it != end(); ++it)
	{
		if ((*it)->GetType() == (FCDEffectParameter::Type) type) list.push_back(*it);
	}
}

// Copy this list
FCDEffectParameterList* FCDEffectParameterList::Clone(FCDEffectParameterList* clone) const
{
	if (clone == NULL)
	{
		clone = new FCDEffectParameterList(const_cast<FCDocument*>(GetDocument()), ownParameters);
	}

	if (!empty())
	{
		clone->reserve(size());

		if (ownParameters)
		{
			for (const_iterator it = begin(); it != end(); ++it)
			{
				FCDEffectParameter* clonedParam = FCDEffectParameterFactory::Create(clone->GetDocument(), (*it)->GetType());
				clone->push_back((*it)->Clone(clonedParam));
			}
		}
		else
		{
			(*clone) = (*this);
		}
	}
	return clone;
}

/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDEffect.h"
#include "FCDocument/FCDEffectProfile.h"
#include "FCDocument/FCDEffectParameter.h"
#include "FCDocument/FCDEffectParameterFactory.h"
#include "FCDocument/FCDEffectParameterList.h"
#include "FCDocument/FCDImage.h"
#include "FCDocument/FCDLibrary.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;

ImplementObjectType(FCDEffectProfile);

FCDEffectProfile::FCDEffectProfile(FCDEffect* _parent)
:	FCDObject(_parent->GetDocument()), parent(_parent)
{
	parameters = new FCDEffectParameterList(GetDocument(), true); 
}

FCDEffectProfile::~FCDEffectProfile()
{
	SAFE_DELETE(parameters);
	parent = NULL;
}

const string& FCDEffectProfile::GetDaeId() const
{
	return parent->GetDaeId();
}

FCDEffectProfile* FCDEffectProfile::Clone(FCDEffectProfile* clone) const
{
	if (clone == NULL) return NULL;
	parameters->Clone(clone->parameters);
	return clone;
}

void FCDEffectProfile::AddParameter(FCDEffectParameter* parameter)
{
	if (parameter != NULL)
	{
		parameters->push_back(parameter);
		SetDirtyFlag();
	}
}

// Look for the parameter with the given reference.
const FCDEffectParameter* FCDEffectProfile::FindParameter(const char* reference) const
{
	return parameters->FindReference(reference);
}

// Look for the effect parameter with the correct semantic, in order to bind/set its value
FCDEffectParameter* FCDEffectProfile::FindParameterBySemantic(const string& semantic)
{
	return parameters->FindSemantic(semantic);
}

void FCDEffectProfile::FindParametersBySemantic(const string& semantic, FCDEffectParameterList& _parameters)
{
	parameters->FindSemantic(semantic, _parameters);
}

void FCDEffectProfile::FindParametersByReference(const string& reference, FCDEffectParameterList& _parameters)
{
	parameters->FindReference(reference, _parameters);
}

FUStatus FCDEffectProfile::LoadFromXML(xmlNode* profileNode)
{
	FUStatus status;

	// Verify that we are given a valid XML input node.
	const char* profileName = FUDaeProfileType::ToString(GetType());
	if (!IsEquivalent(profileNode->name, profileName))
	{
		return status.Warning(FS("Invalid profile input node for effect") + TO_FSTRING(GetDaeId()), profileNode->line);
	}

	// Parse in the child elements: parameters and techniques
	for (xmlNode* child = profileNode->children; child != NULL; child = child->next)
	{
		if (child->type != XML_ELEMENT_NODE) continue;

		if (IsEquivalent(child->name, DAE_FXCMN_NEWPARAM_ELEMENT))
		{
			AddParameter(FCDEffectParameterFactory::LoadFromXML(GetDocument(), child, &status));
		}
		else if (IsEquivalent(child->name, DAE_IMAGE_ELEMENT))
		{
			// You can create images in the profile: tell the image library about it.
			FCDImage* image = GetDocument()->GetImageLibrary()->AddEntity();
			status.AppendStatus(image->LoadFromXML(child));
		}
	}

	SetDirtyFlag();
	return status;
}

xmlNode* FCDEffectProfile::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* profileNode = FUDaeWriter::AddChild(parentNode, FUDaeProfileType::ToString(GetType()));

	// Write out the parameters
	for (FCDEffectParameterList::const_iterator itP = parameters->begin(); itP != parameters->end(); ++itP)
	{
		(*itP)->WriteToXML(profileNode);
	}

	return profileNode;
}

// Links the effect profile and its parameters.
void FCDEffectProfile::Link()
{
	// Make up a list of all the available parameters.
	FCDEffectParameterList availableParameters;
	if (GetParent() != NULL)
	{
		availableParameters.insert(availableParameters.end(), GetParent()->GetParameters()->begin(), GetParent()->GetParameters()->end());
	}
	availableParameters.insert(availableParameters.end(), parameters->begin(), parameters->end());

	for (FCDEffectParameterList::iterator itP = parameters->begin(); itP != parameters->end(); ++itP)
	{
		(*itP)->Link(availableParameters);
	}
}

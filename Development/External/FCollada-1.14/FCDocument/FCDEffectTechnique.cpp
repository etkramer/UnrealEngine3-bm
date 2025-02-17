/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDEffect.h"
#include "FCDocument/FCDEffectCode.h"
#include "FCDocument/FCDEffectPass.h"
#include "FCDocument/FCDEffectProfileFX.h"
#include "FCDocument/FCDEffectParameter.h"
#include "FCDocument/FCDEffectParameterFactory.h"
#include "FCDocument/FCDEffectParameterList.h"
#include "FCDocument/FCDEffectTechnique.h"
#include "FCDocument/FCDLibrary.h"
#include "FCDocument/FCDImage.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

ImplementObjectType(FCDEffectTechnique);

FCDEffectTechnique::FCDEffectTechnique(FCDEffectProfileFX *_parent)
:	FCDObject(_parent->GetDocument()), parent(_parent)
{
	parameters = new FCDEffectParameterList(GetDocument(), true);;
}

FCDEffectTechnique::~FCDEffectTechnique()
{
	SAFE_DELETE(parameters);
	parent = NULL;
}

// Adds a new pass to this effect technique.
FCDEffectPass* FCDEffectTechnique::AddPass()
{
	FCDEffectPass* pass = passes.Add(this);
	SetDirtyFlag();
	return pass;
}

// Adds a new code inclusion to this effect profile.
FCDEffectCode* FCDEffectTechnique::AddCode()
{
	FCDEffectCode* code = codes.Add(GetDocument());
	SetDirtyFlag();
	return code;
}

FCDEffectTechnique* FCDEffectTechnique::Clone(FCDEffectTechnique* clone)
{
	if (clone == NULL) clone = new FCDEffectTechnique(NULL);

	clone->name = name;
	parameters->Clone(clone->parameters);

	// Clone the codes: need to happen before the passes are cloned
	clone->codes.reserve(codes.size());
	for (FCDEffectCodeContainer::const_iterator itC = codes.begin(); itC != codes.end(); ++itC)
	{
		(*itC)->Clone(clone->AddCode());
	}

	// Clone the passes
	clone->passes.reserve(passes.size());
	for (FCDEffectPassContainer::iterator itP = passes.begin(); itP != passes.end(); ++itP)
	{
		(*itP)->Clone(clone->AddPass());
	}

	return clone;
}

const string& FCDEffectTechnique::GetDaeId() const
{
	return parent->GetDaeId();
}

void FCDEffectTechnique::AddParameter(FCDEffectParameter* parameter)
{
	parameters->push_back(parameter);
	SetDirtyFlag();
}

// Flatten this effect technique: merge the parameter modifiers and generators
void FCDEffectTechnique::Flatten()
{
	for (FCDEffectParameterList::iterator itP = parameters->begin(); itP != parameters->end();)
	{
		FCDEffectParameterList generators(GetDocument());
		if ((*itP)->IsModifier())
		{
			// Overwrite the generators
			FindParametersByReference((*itP)->GetReference(), generators);
			for (FCDEffectParameterList::iterator itQ = generators.begin(); itQ != generators.end(); ++itQ)
			{
				if ((*itQ)->IsGenerator())
				{
					(*itP)->Overwrite(*itQ);
				}
			}
			SAFE_DELETE(*itP);
			parameters->erase(itP);
		}
		else
		{
			++itP;
		}
	}
	
	SetDirtyFlag();
}

FUStatus FCDEffectTechnique::LoadFromXML(xmlNode* techniqueNode, xmlNode* profileNode)
{
	FUStatus status;
	if (!IsEquivalent(techniqueNode->name, DAE_TECHNIQUE_ELEMENT))
	{
		return status.Warning(FS("Technique contains unknown element."), techniqueNode->line);
	}
	
	string techniqueName = ReadNodeProperty(techniqueNode, DAE_SID_ATTRIBUTE);
	name = TO_FSTRING(techniqueName);
	
	// Look for the pass and parameter elements
	SAFE_DELETE(parameters);
	parameters = new FCDEffectParameterList(GetDocument(), true);
	for (xmlNode* child = techniqueNode->children; child != NULL; child = child->next)
	{
		if (child->type != XML_ELEMENT_NODE) continue;

		if (IsEquivalent(child->name, DAE_PASS_ELEMENT))
		{
			FCDEffectPass* pass = AddPass();
			status.AppendStatus(pass->LoadFromXML(child, techniqueNode, profileNode));
		}
		else if (IsEquivalent(child->name, DAE_FXCMN_NEWPARAM_ELEMENT) || IsEquivalent(child->name, DAE_FXCMN_SETPARAM_ELEMENT))
		{
			AddParameter(FCDEffectParameterFactory::LoadFromXML(GetDocument(), child, &status));
		}
		else if (IsEquivalent(child->name, DAE_FXCMN_CODE_ELEMENT) || IsEquivalent(child->name, DAE_FXCMN_INCLUDE_ELEMENT))
		{
			FCDEffectCode* code = new FCDEffectCode(GetDocument());
			codes.push_back(code);
			status.AppendStatus(code->LoadFromXML(child));
		}
		else if (IsEquivalent(child->name, DAE_IMAGE_ELEMENT))
		{
			FCDImage* image = GetDocument()->GetImageLibrary()->AddEntity();
			status.AppendStatus(image->LoadFromXML(child));
		}
	}
	
	SetDirtyFlag();
	return status;
}

// Links the effect and its parameters.
void FCDEffectTechnique::Link()
{
	// Make up a list of all the parameters available for linkage.
	FCDEffectParameterList availableParameters;
	if (GetParent() != NULL)
	{
		if (GetParent()->GetParent() != NULL)
		{
			availableParameters.insert(availableParameters.end(), GetParent()->GetParent()->GetParameters()->begin(), GetParent()->GetParent()->GetParameters()->end());
		}
		availableParameters.insert(availableParameters.end(), GetParent()->GetParameters()->begin(), GetParent()->GetParameters()->end());
	}
	availableParameters.insert(availableParameters.end(), parameters->begin(), parameters->end());

	for (FCDEffectParameterList::iterator itP = parameters->begin(); itP != parameters->end(); ++itP)
	{
		(*itP)->Link(availableParameters);
	}
}

// Write out the effect techniques to the COLLADA XML node tree
xmlNode* FCDEffectTechnique::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* techniqueNode = AddChild(parentNode, DAE_TECHNIQUE_ELEMENT);
	fstring& _name = const_cast<fstring&>(name);
	if (_name.empty()) _name = FC("common");
	AddNodeSid(techniqueNode, _name);

	// Write out the code/includes
	for (FCDEffectCodeContainer::const_iterator itC = codes.begin(); itC != codes.end(); ++itC)
	{
		(*itC)->WriteToXML(techniqueNode);
	}

	// Write out the effect parameters at this level
	for (FCDEffectParameterList::const_iterator itP = parameters->begin(); itP != parameters->end(); ++itP)
	{
		(*itP)->WriteToXML(techniqueNode);
	}

	// Write out the passes.
	// In COLLADA 1.4: there should always be at least one pass.
	if (!passes.empty())
	{
		for (FCDEffectPassContainer::const_iterator itP = passes.begin(); itP != passes.end(); ++itP)
		{
			(*itP)->WriteToXML(techniqueNode);
		}
	}
	else
	{
		UNUSED(xmlNode* dummyPassNode =) AddChild(techniqueNode, DAE_PASS_ELEMENT);
	}

	return techniqueNode;
}

// Look for the parameter with the given reference.
const FCDEffectParameter* FCDEffectTechnique::FindParameter(const char* ref) const
{
	return parameters->FindReference(ref);
}

// Look for the effect parameter with the correct semantic, in order to bind/set its value
FCDEffectParameter* FCDEffectTechnique::FindParameterBySemantic(const string& semantic)
{
	return parameters->FindSemantic(semantic);
}

void FCDEffectTechnique::FindParametersBySemantic(const string& semantic, FCDEffectParameterList& _parameters)
{
	for (FCDEffectParameterList::iterator it = parameters->begin(); it != parameters->end(); ++it)
	{
		if ((*it)->GetSemantic() == semantic) _parameters.push_back(*it);
	}
}

void FCDEffectTechnique::FindParametersByReference(const string& reference, FCDEffectParameterList& _parameters)
{
	for (FCDEffectParameterList::iterator it = parameters->begin(); it != parameters->end(); ++it)
	{
		if ((*it)->GetReference() == reference) _parameters.push_back(*it);
	}
}

FCDEffectCode* FCDEffectTechnique::FindCode(const string& sid)
{
	for (FCDEffectCodeContainer::iterator itC = codes.begin(); itC != codes.end(); ++itC)
	{
		if ((*itC)->GetSubId() == sid) return (*itC);
	}
	return NULL;
}
const FCDEffectCode* FCDEffectTechnique::FindCode(const string& sid) const
{
	for (FCDEffectCodeContainer::const_iterator itC = codes.begin(); itC != codes.end(); ++itC)
	{
		if ((*itC)->GetSubId() == sid) return (*itC);
	}
	return NULL;
}

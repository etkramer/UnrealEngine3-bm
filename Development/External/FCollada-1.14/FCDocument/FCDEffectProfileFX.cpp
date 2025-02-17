/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/
/*
	Based on the FS Import classes:
	Copyright (C) 2005-2006 Feeling Software Inc
	Copyright (C) 2005-2006 Autodesk Media Entertainment
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDEffect.h"
#include "FCDocument/FCDEffectCode.h"
#include "FCDocument/FCDEffectProfileFX.h"
#include "FCDocument/FCDEffectParameter.h"
#include "FCDocument/FCDEffectParameterList.h"
#include "FCDocument/FCDEffectParameterFactory.h"
#include "FCDocument/FCDEffectTechnique.h"
#include "FCDocument/FCDLibrary.h"
#include "FCDocument/FCDImage.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

ImplementObjectType(FCDEffectProfileFX);

FCDEffectProfileFX::FCDEffectProfileFX(FCDEffect* _parent, FUDaeProfileType::Type _type)
:	FCDEffectProfile(_parent), type(_type)
{
}

FCDEffectProfileFX::~FCDEffectProfileFX()
{
}

FCDEffectTechnique* FCDEffectProfileFX::AddTechnique()
{
	FCDEffectTechnique* technique = techniques.Add(this);
	SetDirtyFlag();
	return technique;
}

FCDEffectCode* FCDEffectProfileFX::FindCode(const string& sid)
{
	for (FCDEffectCodeContainer::iterator itC = codes.begin(); itC != codes.end(); ++itC)
	{
		if ((*itC)->GetSubId() == sid) return (*itC);
	}
	return NULL;
}
const FCDEffectCode* FCDEffectProfileFX::FindCode(const string& sid) const
{
	for (FCDEffectCodeContainer::const_iterator itC = codes.begin(); itC != codes.end(); ++itC)
	{
		if ((*itC)->GetSubId() == sid) return (*itC);
	}
	return NULL;
}

// Look for the parameter with the given reference.
const FCDEffectParameter* FCDEffectProfileFX::FindParameter(const char* reference) const
{
	const FCDEffectParameter* parameter = FCDEffectProfile::FindParameter(reference);
	for (FCDEffectTechniqueContainer::const_iterator it = techniques.begin(); it != techniques.end() && parameter == NULL; ++it)
	{
		parameter = (*it)->FindParameter(reference);
	}
	return parameter;
}

// Look for the effect parameter with the correct semantic, in order to bind/set its value
FCDEffectParameter* FCDEffectProfileFX::FindParameterBySemantic(const string& semantic)
{
	FCDEffectParameter* parameter = FCDEffectProfile::FindParameterBySemantic(semantic);
	for (FCDEffectTechniqueContainer::iterator it = techniques.begin(); it != techniques.end() && parameter == NULL; ++it)
	{
		parameter = (*it)->FindParameterBySemantic(semantic);
	}
	return parameter;
}

void FCDEffectProfileFX::FindParametersBySemantic(const string& semantic, FCDEffectParameterList& _parameters)
{
	// Look in the local parameters
	FCDEffectProfile::FindParametersBySemantic(semantic, _parameters);

	// Look in the techniques
	for( FCDEffectTechniqueContainer::iterator it = techniques.begin(); it != techniques.end(); ++it)
	{
		(*it)->FindParametersBySemantic(semantic, _parameters);
	}
}

void FCDEffectProfileFX::FindParametersByReference(const string& reference, FCDEffectParameterList& _parameters)
{
	// Look in the local parameters
	FCDEffectProfile::FindParametersByReference(reference, _parameters);

	// Look in the techniques
	for( FCDEffectTechniqueContainer::iterator it = techniques.begin(); it != techniques.end(); ++it)
	{
		(*it)->FindParametersBySemantic(reference, _parameters);
	}
}

// Adds a new code inclusion to this effect profile.
FCDEffectCode* FCDEffectProfileFX::AddCode()
{
	FCDEffectCode* code = codes.Add(GetDocument());
	SetDirtyFlag();
	return code;
}

// Clone the profile effect and its parameters
FCDEffectProfile* FCDEffectProfileFX::Clone(FCDEffectProfile* _clone) const
{
	FCDEffectProfileFX* clone = NULL;
	if (_clone == NULL) { _clone = clone = new FCDEffectProfileFX(const_cast<FCDEffect*>(GetParent()), type); }
	else if (_clone->GetObjectType() == FCDEffectProfileFX::GetClassType()) clone = (FCDEffectProfileFX*) _clone;

	if (_clone != NULL) FCDEffectProfile::Clone(_clone);
	if (clone != NULL)
	{
		// Clone the codes: needs to happen before the techniques are cloned.
		clone->codes.reserve(codes.size());
		for (FCDEffectCodeContainer::const_iterator itC = codes.begin(); itC != codes.end(); ++itC)
		{
			FCDEffectCode* clonedCode = clone->AddCode();
			(*itC)->Clone(clonedCode);
		}

		// Clone the techniques
		clone->techniques.reserve(techniques.size());
		for (FCDEffectTechniqueContainer::const_iterator itT = techniques.begin(); itT != techniques.end(); ++itT)
		{
			FCDEffectTechnique* clonedTechnique = clone->AddTechnique();
            (*itT)->Clone(clonedTechnique);
		}
	}

	return _clone;
}

// Flatten this effect profile: trickling down all the parameters to the techniques
void FCDEffectProfileFX::Flatten()
{
	FCDEffectParameterList* parameters = GetParameters();
	for (FCDEffectParameterList::iterator itP = parameters->begin(); itP != parameters->end(); ++itP)
	{
		FCDEffectParameterList generators;
		if ((*itP)->IsModifier())
		{
			// Overwrite the generators
			FindParametersByReference((*itP)->GetReference(), generators);
			for (FCDEffectParameterList::iterator itQ = generators.begin(); itQ != generators.end(); ++itQ)
			{
				if ((*itP) != (*itQ))
				{
					(*itP)->Overwrite(*itQ);
				}
			}
		}
		else
		{
			// Add this parameter to the techniques
			for (FCDEffectTechniqueContainer::iterator itT = techniques.begin(); itT != techniques.end(); ++itT)
			{
                (*itT)->AddParameter((*itP)->Clone());
			}
		}
	}
	while (!parameters->empty())
	{
		delete parameters->back();
	}

	// Flatten the techniques
	for (FCDEffectTechniqueContainer::iterator itT = techniques.begin(); itT != techniques.end(); ++itT)
	{
        (*itT)->Flatten();
	}
	SetDirtyFlag();
}

// Read a <profile_X> node for a given COLLADA effect
// Note that this function should do most of the work, except for profile-specific states
FUStatus FCDEffectProfileFX::LoadFromXML(xmlNode* profileNode)
{
	FUStatus status = FCDEffectProfile::LoadFromXML(profileNode);
	if (status.IsFailure()) return status;
    
	// Read in the target platform for this effect profile
	platform = TO_FSTRING(ReadNodeProperty(profileNode, DAE_PLATFORM_ATTRIBUTE));

	// Parse in the child technique/code/include elements.
	for (xmlNode* child = profileNode->children; child != NULL; child = child->next)
	{
		if (child->type != XML_ELEMENT_NODE) continue;

		if (IsEquivalent(child->name, DAE_TECHNIQUE_ELEMENT))
		{
			FCDEffectTechnique* technique = AddTechnique();
			status.AppendStatus(technique->LoadFromXML(child, profileNode));
		}
		else if (IsEquivalent(child->name, DAE_FXCMN_CODE_ELEMENT) || IsEquivalent(child->name, DAE_FXCMN_INCLUDE_ELEMENT))
		{
			FCDEffectCode* code = AddCode();
			status.AppendStatus(code->LoadFromXML(child));
		}
	}
	
	SetDirtyFlag();
	return status;
}

void FCDEffectProfileFX::Link()
{
	// Call the parent to link the effect parameters
	FCDEffectProfile::Link();

	for (FCDEffectTechniqueContainer::iterator itT = techniques.begin(); itT != techniques.end(); ++itT)
	{
		(*itT)->Link();
	}
}

xmlNode* FCDEffectProfileFX::WriteToXML(xmlNode* parentNode) const
{
	// Call the parent to create the profile node and to export the parameters.
	xmlNode* profileNode = FCDEffectProfile::WriteToXML(parentNode);

	// Write out the profile properties/base elements
	if (!platform.empty()) AddAttribute(profileNode, DAE_PLATFORM_ATTRIBUTE, platform);

	// Write out the code/includes
	// These will include themselves before the parameters
	for (FCDEffectCodeContainer::const_iterator itC = codes.begin(); itC != codes.end(); ++itC)
	{
		(*itC)->WriteToXML(profileNode);
	}

	// Write out the techniques
	for (FCDEffectTechniqueContainer::const_iterator itT = techniques.begin(); itT != techniques.end(); ++itT)
	{
		(*itT)->WriteToXML(profileNode);
	}

	return profileNode;
}

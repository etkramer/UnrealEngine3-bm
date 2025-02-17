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
#include "FCDocument/FCDAnimated.h"
#include "FCDocument/FCDEffect.h"
#include "FCDocument/FCDEffectStandard.h"
#include "FCDocument/FCDEffectProfileFX.h"
#include "FCDocument/FCDEffectParameter.h"
#include "FCDocument/FCDEffectParameterFactory.h"
#include "FCDocument/FCDEffectParameterList.h"
#include "FCDocument/FCDLibrary.h"
#include "FCDocument/FCDImage.h"
#include "FUtils/FUStringConversion.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

ImplementObjectType(FCDEffect);

FCDEffect::FCDEffect(FCDocument* document) : FCDEntity(document, "Effect")
{
	parameters = new FCDEffectParameterList(GetDocument(), true);
}

FCDEffect::~FCDEffect()
{
	SAFE_DELETE(parameters);
}

void FCDEffect::AddParameter(FCDEffectParameter* parameter)
{
	if (parameter != NULL)
	{
		parameters->push_back(parameter);
		SetDirtyFlag(); 
	}
}

// Flatten this effect: trickling down all the parameters to the technique level
void FCDEffect::Flatten()
{
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
			// Add this parameter to hierarchies below
			for (FCDEffectProfileContainer::iterator itR = profiles.begin(); itR != profiles.end(); ++itR)
			{
				if ((*itR)->GetType() != FUDaeProfileType::COMMON)
				{
					((FCDEffectProfileFX*) (*itR))->AddParameter((*itP)->Clone());
				}
			}
		}
	}
	while (!parameters->empty())
	{
		delete parameters->back();
	}

	for (FCDEffectProfileContainer::iterator itR = profiles.begin(); itR != profiles.end(); ++itR)
	{
		(*itR)->Flatten();
	}

	SetDirtyFlag(); 
}

// Search for a profile of the given type
const FCDEffectProfile* FCDEffect::FindProfile(FUDaeProfileType::Type type) const
{
	for (FCDEffectProfileContainer::const_iterator itR = profiles.begin(); itR != profiles.end(); ++itR)
	{
		if ((*itR)->GetType() == type) return (*itR);
	}
	return NULL;
}

// Search for a profile of a given type and platform
FCDEffectProfile* FCDEffect::FindProfileByTypeAndPlatform(FUDaeProfileType::Type type, string platform)
{
	for (FCDEffectProfileContainer::iterator itR = profiles.begin(); itR != profiles.end(); ++itR)
	{
		if ((*itR)->GetType() == type) 
		{
			if(((FCDEffectProfileFX*)(*itR))->GetPlatform() == TO_FSTRING(platform)) return (*itR);
		}
	}
	return NULL;
}

const FCDEffectProfile* FCDEffect::FindProfileByTypeAndPlatform(FUDaeProfileType::Type type, string platform) const
{
	for (FCDEffectProfileContainer::const_iterator itR = profiles.begin(); itR != profiles.end(); ++itR)
	{
		if ((*itR)->GetType() == type) 
		{
			if(((FCDEffectProfileFX*)(*itR))->GetPlatform() == TO_FSTRING(platform)) return (*itR);
		}
	}
	return NULL;
}

// Create a new effect profile.
FCDEffectProfile* FCDEffect::AddProfile(FUDaeProfileType::Type type)
{
	FCDEffectProfile* profile = NULL;

	// Create the correct profile for this type.
	if (type == FUDaeProfileType::COMMON) profile = new FCDEffectStandard(this);
	else profile = new FCDEffectProfileFX(this, type);

	profiles.push_back(profile);
	SetDirtyFlag(); 
	return profile;
}

// Look for the effect parameter with the correct semantic, in order to bind/set its value
FCDEffectParameter* FCDEffect::FindParameterBySemantic(const string& semantic)
{
	FCDEffectParameter* p = parameters->FindSemantic(semantic);
	for (FCDEffectProfileContainer::iterator itR = profiles.begin(); itR != profiles.end() && p == NULL; ++itR)
	{
		p = (*itR)->FindParameterBySemantic(semantic);
	}
	return p;
}

void FCDEffect::FindParametersBySemantic(const string& semantic, FCDEffectParameterList& _parameters)
{
	parameters->FindSemantic(semantic, _parameters);
	for (FCDEffectProfileContainer::iterator itR = profiles.begin(); itR != profiles.end(); ++itR)
	{
		(*itR)->FindParametersBySemantic(semantic, _parameters);
	}
}

void FCDEffect::FindParametersByReference(const string& reference, FCDEffectParameterList& _parameters)
{
	parameters->FindReference(reference, _parameters);
	for (FCDEffectProfileContainer::iterator itR = profiles.begin(); itR != profiles.end(); ++itR)
	{
		(*itR)->FindParametersBySemantic(reference, _parameters);
	}
}

// Parse COLLADA document's <effect> element
// Also parses the <material> element for COLLADA 1.3 backward compatibility
FUStatus FCDEffect::LoadFromXML(xmlNode* effectNode)
{
	FUStatus status = FCDEntity::LoadFromXML(effectNode);
	if (!status) return status;

	while (!parameters->empty())
	{
		delete parameters->back();
	}

	// COLLADA 1.3 backward compatibility: look for a <material><shader> node and parse this into a standard effect.
	if (IsEquivalent(effectNode->name, DAE_MATERIAL_ELEMENT))
	{
		xmlNode* shaderNode = FindChildByType(effectNode, DAE_SHADER_ELEMENT);
		if (shaderNode != NULL)
		{
			FCDEffectProfile* profile = AddProfile(FUDaeProfileType::COMMON);
			status.AppendStatus(profile->LoadFromXML(shaderNode));
		}
	}
	else
	{
		// Accept solely <effect> elements at this point.
		if (!IsEquivalent(effectNode->name, DAE_EFFECT_ELEMENT))
		{
			return status.Warning(FS("Unknown element in effect library."), effectNode->line);
		}

		for (xmlNode* child = effectNode->children; child != NULL; child = child->next)
		{
			if (child->type != XML_ELEMENT_NODE) continue;

			if (IsEquivalent(child->name, DAE_IMAGE_ELEMENT))
			{
				FCDImage* image = GetDocument()->GetImageLibrary()->AddEntity();
				status.AppendStatus(image->LoadFromXML(child));
			}
			else if (IsEquivalent(child->name, DAE_FXCMN_SETPARAM_ELEMENT) || IsEquivalent(child->name, DAE_FXCMN_NEWPARAM_ELEMENT))
			{
				AddParameter(FCDEffectParameterFactory::LoadFromXML(GetDocument(), child, &status));
			}
			else if (IsEquivalent(child->name, DAE_EXTRA_ELEMENT))
			{
				// Valid element.. but not processed.
			}
			else
			{
				// Check for a valid profile element.
				FUDaeProfileType::Type type = FUDaeProfileType::FromString((const char*) child->name);
				if (type != FUDaeProfileType::UNKNOWN)
				{
					FCDEffectProfile* profile = AddProfile(type);
					status.AppendStatus(profile->LoadFromXML(child));
				}
				else
				{
					status.Warning(FS("Unsupported profile or unknown element in effect: ") + TO_FSTRING(GetDaeId()), child->line);
				}
			}
		}
	}

	SetDirtyFlag(); 
	return status;
}

// Links the effect and its parameters.
void FCDEffect::Link()
{
	// Link up the parameters
	for (FCDEffectParameterList::iterator itP = parameters->begin(); itP != parameters->end(); ++itP)
	{
		(*itP)->Link(*parameters);
	}

	// Link up the profiles and their parameters/textures/images
	for (FCDEffectProfileContainer::iterator itR = profiles.begin(); itR != profiles.end(); ++itR)
	{
		(*itR)->Link();
	}
}

// Returns a copy of the effect, with all the animations/textures attached
FCDEffect* FCDEffect::Clone()
{
	FCDEffect* clone = new FCDEffect(GetDocument());
	FCDEntity::Clone(clone);

	for (FCDEffectProfileContainer::iterator itR = profiles.begin(); itR != profiles.end(); ++itR)
	{
		FCDEffectProfile* clonedProfile = clone->AddProfile((*itR)->GetType());
		(*itR)->Clone(clonedProfile);
	}

	parameters->Clone(clone->parameters);
	return clone;
}

// Write out the <material> element to the COLLADA XML tree
xmlNode* FCDEffect::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* effectNode = WriteToEntityXML(parentNode, DAE_EFFECT_ELEMENT);

	// Write out the parameters
	for (FCDEffectParameterList::iterator itP = parameters->begin(); itP != parameters->end(); ++itP)
	{
		(*itP)->WriteToXML(effectNode);
	}

	// Write out the profiles
	for (FCDEffectProfileContainer::const_iterator itR = profiles.begin(); itR != profiles.end(); ++itR)
	{
		(*itR)->WriteToXML(effectNode);
	}

	FCDEffect::WriteToExtraXML(effectNode);
	return effectNode;
}

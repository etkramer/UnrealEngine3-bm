/*
	Copyright (C) 2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDAnimated.h"
#include "FCDocument/FCDEffectPass.h"
#include "FCDocument/FCDEffectProfile.h"
#include "FCDocument/FCDEffectTechnique.h"
#include "FCDocument/FCDEffectParameter.h"
#include "FCDocument/FCDEffectParameterList.h"
#include "FCDocument/FCDEffectParameterSurface.h"
#include "FCDocument/FCDEffectParameterSampler.h"
#include "FCDocument/FCDImage.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

//
// FCDEffectParameterSampler
//

ImplementObjectType(FCDEffectParameterSampler);

FCDEffectParameterSampler::FCDEffectParameterSampler(FCDocument* document) : FCDEffectParameter(document)
{
	samplerType = SAMPLER2D;
}

FCDEffectParameterSampler::~FCDEffectParameterSampler()
{
}

// Sets the surface parameter for the surface to sample.
void FCDEffectParameterSampler::SetSurface(FCDEffectParameterSurface* _surface)
{
	surface = _surface;
	SetDirtyFlag();
}

// compare value
bool FCDEffectParameterSampler::IsValueEqual( FCDEffectParameter* parameter )
{
	if (!FCDEffectParameter::IsValueEqual( parameter )) return false;
	if (parameter->GetObjectType() != FCDEffectParameterSampler::GetClassType()) return false;
	FCDEffectParameterSampler *param = (FCDEffectParameterSampler*)parameter;
	
	if (samplerType != param->GetSamplerType() ) return false;
	if (param->GetSurface() == NULL && surface == NULL) {}
	else if (param->GetSurface() == NULL || surface == NULL) return false;
	else if (!IsEquivalent(param->GetSurface()->GetReference(), surface->GetReference())) return false;

	return true;
}

// Clone
FCDEffectParameter* FCDEffectParameterSampler::Clone(FCDEffectParameter* _clone) const
{
	FCDEffectParameterSampler* clone = NULL;
	if (_clone == NULL) _clone = clone = new FCDEffectParameterSampler(const_cast<FCDocument*>(GetDocument()));
	else if (_clone->GetObjectType() == FCDEffectParameterSampler::GetClassType()) clone = (FCDEffectParameterSampler*) _clone;

	if (_clone != NULL) FCDEffectParameter::Clone(_clone);
	if (clone != NULL)
	{
		clone->surface = surface;
		clone->samplerType = samplerType;
	}
	return _clone;
}

// Flattening: overwrite the target parameter with this parameter
void FCDEffectParameterSampler::Overwrite(FCDEffectParameter* target)
{
	if (target->GetType() == SAMPLER)
	{
		FCDEffectParameterSampler* s = (FCDEffectParameterSampler*) target;
		if (samplerType == s->samplerType)
		{
			s->surface = surface;
			SetDirtyFlag();
		}
	}
}

FUStatus FCDEffectParameterSampler::LoadFromXML(xmlNode* parameterNode)
{
	FUStatus status = FCDEffectParameter::LoadFromXML(parameterNode);

	// Find the sampler node
	xmlNode* samplerNode = NULL;
	for (xmlNode* child = parameterNode->children; child != NULL; child = child->next)
	{
		if (child->type != XML_ELEMENT_NODE) continue;

		if (IsEquivalent(child->name, DAE_FXCMN_SAMPLER1D_ELEMENT)) { samplerType = SAMPLER1D; samplerNode = child; break; }
		else if (IsEquivalent(child->name, DAE_FXCMN_SAMPLER2D_ELEMENT)) { samplerType = SAMPLER2D; samplerNode = child; break; }
		else if (IsEquivalent(child->name, DAE_FXCMN_SAMPLER3D_ELEMENT)) { samplerType = SAMPLER3D; samplerNode = child; break; }
		else if (IsEquivalent(child->name, DAE_FXCMN_SAMPLERCUBE_ELEMENT)) { samplerType = SAMPLERCUBE; samplerNode = child; break; }
	}

	if (samplerNode == NULL)
	{
		return status.Warning(FS("Unable to find sampler node for sampler parameter: ") + TO_FSTRING(GetReference()), parameterNode->line);
	}

	// Parse the source node
	xmlNode* sourceNode = FindChildByType(samplerNode, DAE_SOURCE_ELEMENT);
	surfaceSid = ReadNodeContentDirect(sourceNode);
	if (surfaceSid.empty())
	{
		return status.Warning(FS("Empty surface source value for sampler parameter: ") + TO_FSTRING(GetReference()), parameterNode->line);
	}
	
	return status;
}

xmlNode* FCDEffectParameterSampler::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* parameterNode = FCDEffectParameter::WriteToXML(parentNode);
	const char* samplerName;
	switch(samplerType)
	{
	case SAMPLER1D: samplerName = DAE_FXCMN_SAMPLER1D_ELEMENT; break;
	case SAMPLER2D: samplerName = DAE_FXCMN_SAMPLER2D_ELEMENT; break;
	case SAMPLER3D: samplerName = DAE_FXCMN_SAMPLER3D_ELEMENT; break;
	case SAMPLERCUBE: samplerName = DAE_FXCMN_SAMPLERCUBE_ELEMENT; break;
	default: samplerName = DAEERR_UNKNOWN_ELEMENT; break;
	}
	xmlNode* samplerNode = AddChild(parameterNode, samplerName);
	if (surface != NULL) AddChild(samplerNode, DAE_SOURCE_ELEMENT, surface->GetReference());

	// Write out some default filtering parameters.
	AddChild(samplerNode, "minfilter", "LINEAR_MIPMAP_LINEAR");
	AddChild(samplerNode, "magfilter", "LINEAR");
	//AddChild(samplerNode, "mipfilter", "LINEAR");/*FXC seems not read this line*/

	return parameterNode;
}

void FCDEffectParameterSampler::Link(FCDEffectParameterList& parameters)
{
	FCDEffectParameter* _surface = parameters.FindReference(surfaceSid);
	if (_surface != NULL && _surface->GetObjectType() == FCDEffectParameterSurface::GetClassType())
	{
		surface = (FCDEffectParameterSurface*) _surface;
	}
	surfaceSid.clear();
}

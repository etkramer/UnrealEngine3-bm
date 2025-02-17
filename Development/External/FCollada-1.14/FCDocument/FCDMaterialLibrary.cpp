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

/*
* FCDMaterialLibrary covers the material and effect libraries.
* Covers as well the texture library for COLLADA 1.3 backward compatibility
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDLibrary.h"
#include "FCDocument/FCDMaterial.h"
#include "FCDocument/FCDMaterialLibrary.h"
#include "FCDocument/FCDEffect.h"
#include "FCDocument/FCDEntity.h"
#include "FCDocument/FCDTexture.h"
#include "FUtils/FUStringConversion.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

#include "FCDocument/FCDLibrary.hpp" // Must occur after the other inclusions, for GCC-sakes.

FCDMaterialLibrary::FCDMaterialLibrary(FCDocument* document) : FCDLibrary<FCDEntity>(document)
{
}

FCDMaterialLibrary::~FCDMaterialLibrary()
{
	// Textures, effects and material entities are deleted by the parent's destructor
	textures.clear();
	effects.clear();
	materials.clear();
}

// Search for specific material elements
FCDTexture* FCDMaterialLibrary::FindTexture(const string& _daeId)
{
	const char* daeId = SkipPound(_daeId);
	for (FCDTextureContainer::iterator it = textures.begin(); it != textures.end(); ++it)
	{
		if ((*it)->GetDaeId() == FUDaeWriter::CleanId(daeId)) return *it;
	}
	return NULL;
}
FCDEffect* FCDMaterialLibrary::FindEffect(const string& _daeId)
{
	const char* daeId = SkipPound(_daeId);
	for (FCDEffectContainer::iterator it = effects.begin(); it != effects.end(); ++it)
	{
		if ((*it)->GetDaeId() == FUDaeWriter::CleanId(daeId)) return *it;
	}
	return NULL;
}
FCDMaterial* FCDMaterialLibrary::FindMaterial(const string& _daeId)
{
	const char* daeId = SkipPound(_daeId);
	for (FCDMaterialContainer::iterator it = materials.begin(); it != materials.end(); ++it)
	{
		if ((*it)->GetDaeId() == FUDaeWriter::CleanId(daeId)) return *it;
	}
	return NULL;
}

// Add new entities
FCDEffect* FCDMaterialLibrary::AddEffect()
{
	FCDEffect* effect = effects.Add(GetDocument());
	entities.push_back(effect);
	SetDirtyFlag();
	return effect;
}

FCDMaterial* FCDMaterialLibrary::AddMaterial()
{
	FCDMaterial* material = materials.Add(GetDocument());
	entities.push_back(material);
	SetDirtyFlag();
	return material;
}

// Read in the COLLADA material/effect library nodes
// Also read in the texture library for COLLADA 1.3 backward compatibility
FUStatus FCDMaterialLibrary::LoadFromXML(xmlNode* node)
{
	FUStatus status;

	// Determine the library type
	string libraryType = ReadNodeProperty(node, DAE_TYPE_ATTRIBUTE);

	bool loadEffect = (IsEquivalent(node->name, DAE_LIBRARY_EFFECT_ELEMENT) ||
					   IsEquivalent(node->name, DAE_LIBRARY_ELEMENT) && (libraryType == DAE_EFFECT_TYPE || libraryType == DAE_MATERIAL_TYPE));
	bool loadMaterial = (IsEquivalent(node->name, DAE_LIBRARY_MATERIAL_ELEMENT) ||
						 IsEquivalent(node->name, DAE_LIBRARY_ELEMENT) && libraryType == DAE_MATERIAL_TYPE);
	bool loadTexture = (IsEquivalent(node->name, DAE_LIBRARY_ELEMENT) && libraryType == DAE_TEXTURE_TYPE);

	for (xmlNode* child = node->children; child != NULL; child = child->next)
	{
		if (child->type != XML_ELEMENT_NODE) continue;

		FCDEffect* effect = NULL;

		if(loadEffect)
		{
			// Parse the <effect> elements
			// COLLADA 1.3. backward compatibility: also parse the <material> elements to generate the standard effects.
			effect = AddEffect();
			status.AppendStatus(effect->LoadFromXML(child));
		}
		if(loadMaterial)
		{
			// Parse the <material> elements
			FCDMaterial* material = AddMaterial();
			if (effect != NULL) material->SetEffect(effect);
			status.AppendStatus(material->LoadFromXML(child));
		}
		if(loadTexture)
		{
			// Parse the <texture> elements
			FCDTexture* texture = textures.Add(GetDocument());
			status.AppendStatus(texture->LoadFromXML(child));
			entities.push_back(texture);
		}
	}

	SetDirtyFlag();
	return status;
}

// Write out the COLLADA material and effect library nodes
void FCDMaterialLibrary::WriteToXML(xmlNode* libraryNode) const
{
	// Write out the materials
	for (FCDMaterialContainer::const_iterator itM = materials.begin(); itM != materials.end(); ++itM)
	{
		(*itM)->WriteToXML(libraryNode);
	}

	// Also write out the effects in their library, as a sibling of this node
	xmlNode* effectLibraryNode = AddSibling(libraryNode, DAE_LIBRARY_EFFECT_ELEMENT);
	for (FCDEffectContainer::const_iterator itE = effects.begin(); itE != effects.end(); ++itE)
	{
		(*itE)->WriteToXML(effectLibraryNode);
	}
}

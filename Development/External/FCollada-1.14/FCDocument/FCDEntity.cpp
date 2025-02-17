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
#include "FCDocument/FCDEntity.h"
#include "FCDocument/FCDExtra.h"
#include "FCDocument/FCDSceneNode.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
#include "FUtils/FUUniqueStringMap.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

ImplementObjectType(FCDEntity);

FCDEntity::FCDEntity(FCDocument* document, const char* baseId) : FCDObjectWithId(document, baseId)
{
	extra = new FCDExtra(document);
}

FCDEntity::~FCDEntity()
{
}

// Structure cloning
FCDEntity* FCDEntity::Clone(FCDEntity* clone) const
{
	if (clone == NULL)
	{
		clone = new FCDEntity(const_cast<FCDocument*>(GetDocument()));
	}

	FCDObjectWithId::Clone(clone);
	clone->name = name;
	clone->note = note;
	if (extra != NULL)
	{
		extra->Clone(clone->extra);
	}
	return clone;
}

void FCDEntity::SetName(const fstring& _name) 
{
	name = CleanName(_name);
	SetDirtyFlag();
}

// Parse this entity information from the COLLADA XML document
FUStatus FCDEntity::LoadFromXML(xmlNode* entityNode)
{
	FUStatus status;

	string fileId = FUDaeParser::ReadNodeId(entityNode);
	if (!fileId.empty()) SetDaeId(fileId);
	else RemoveDaeId();

	name = TO_FSTRING(FUDaeParser::ReadNodeName(entityNode));
	if (name.empty()) name = TO_FSTRING(fileId);

	xmlNode* extraNode = FindChildByType(entityNode, DAE_EXTRA_ELEMENT);
	if (extraNode != NULL)
	{
		extra->LoadFromXML(extraNode);

		// Look for an extra node at this level and a Max/Maya-specific technique
		FCDETechnique* mayaTechnique = extra->FindTechnique(DAEMAYA_MAYA_PROFILE);
		FCDETechnique* maxTechnique = extra->FindTechnique(DAEMAX_MAX_PROFILE);

		// Read in all the extra parameters
		StringList parameterNames;
		FCDENodeList parameterNodes;
		if (mayaTechnique != NULL) mayaTechnique->FindParameters(parameterNodes, parameterNames);
		if (maxTechnique != NULL) maxTechnique->FindParameters(parameterNodes, parameterNames);

		// Look for the note and user-properties, which is the only parameter currently supported at this level
		size_t parameterCount = parameterNodes.size();
		for (size_t i = 0; i < parameterCount; ++i)
		{
			FCDENode* parameterNode = parameterNodes[i];
			const string& parameterName = parameterNames[i];

			if (parameterName == DAEMAX_USERPROPERTIES_NODE_PARAMETER || parameterName == DAEMAYA_NOTE_PARAMETER
				|| parameterName == DAEMAX_USERPROPERTIES_NODE_PARAMETER1_3 || parameterName == DAEMAYA_MAYA_NOTE_PARAMETER1_3)
			{
				note = parameterNode->GetContent();
				parameterNode->Release();
			}
		}
	}

	SetDirtyFlag();
	return status;
}

// Look for a children with the given COLLADA Id.
FCDEntity* FCDEntity::FindDaeId(const string& _daeId)
{
	if (GetDaeId() == _daeId) return this;
	return NULL;
}

xmlNode* FCDEntity::WriteToXML(xmlNode* parentNode) const
{
	return WriteToEntityXML(parentNode, DAEERR_UNKNOWN_ELEMENT);
}

xmlNode* FCDEntity::WriteToEntityXML(xmlNode* parentNode, const char* nodeName, bool writeId) const
{
	// Create the entity node and write out the id and name attributes
	xmlNode* entityNode = AddChild(parentNode, nodeName);
	if (writeId)
	{
		AddAttribute(entityNode, DAE_ID_ATTRIBUTE, GetDaeId());
	}
	if (!name.empty())
	{
		AddAttribute(entityNode, DAE_NAME_ATTRIBUTE, name);
	}

	return entityNode;
}

void FCDEntity::WriteToExtraXML(xmlNode* entityNode) const
{
	// Write out the note
	if (HasNote())
	{
		xmlNode* techniqueNode = AddExtraTechniqueChild(entityNode, DAEMAX_MAX_PROFILE);
		AddChild(techniqueNode, DAEMAX_USERPROPERTIES_NODE_PARAMETER, note);
	}

	// Write out the user-defined extra information.
	if (extra != NULL)
	{
		extra->WriteToXML(entityNode);
	}
}

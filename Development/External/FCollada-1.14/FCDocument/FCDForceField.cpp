/*
	Copyright (C) 2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDExtra.h"
#include "FCDocument/FCDForceField.h"
#include "FUtils/FUDaeParser.h"

ImplementObjectType(FCDForceField);

FCDForceField::FCDForceField(FCDocument* document)
:	FCDEntity(document, "ForceField")
{
	information = new FCDExtra(GetDocument());
}

FCDForceField::~FCDForceField()
{
}

FUStatus FCDForceField::LoadFromXML(xmlNode* node)
{
	FUStatus status = FCDEntity::LoadFromXML(node);
	if (!status) return status;
	if (!IsEquivalent(node->name, DAE_FORCE_FIELD_ELEMENT))
	{
		return status.Warning(FS("Force field library contains unknown element."), node->line);
	}

	if (information != NULL)
	{
		information = new FCDExtra(GetDocument());
	}
	status.AppendStatus(information->LoadFromXML(node));
	
	SetDirtyFlag();
	return status;
}

xmlNode* FCDForceField::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* forceFieldNode = FCDEntity::WriteToEntityXML(parentNode, DAE_FORCE_FIELD_ELEMENT);
	if (information != NULL)
	{
		information->WriteTechniquesToXML(forceFieldNode);
	}

	FCDEntity::WriteToExtraXML(forceFieldNode);
	return forceFieldNode;
}

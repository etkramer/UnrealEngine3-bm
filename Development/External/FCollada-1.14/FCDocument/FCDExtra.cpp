/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDAnimated.h"
#include "FCDocument/FCDExtra.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

//
// FCDExtra
//

ImplementObjectType(FCDExtra);

FCDExtra::FCDExtra(FCDocument* document)
:	FCDObject(document)
{
}

FCDExtra::~FCDExtra()
{
}

// Adds a technique of the given profile (or return the existing technique with this profile).
FCDETechnique* FCDExtra::AddTechnique(const char* profile)
{
	FCDETechnique* technique = FindTechnique(profile);
	if (technique == NULL)
	{
		technique = techniques.Add(GetDocument(), profile);
		SetDirtyFlag();
	}
	return technique;
}

// Search for a profile-specific technique
FCDETechnique* FCDExtra::FindTechnique(const char* profile)
{
	for (FCDETechniqueContainer::iterator itT = techniques.begin(); itT != techniques.end(); ++itT)
	{
		if (IsEquivalent((*itT)->GetProfile(), profile)) return *itT;
	}
	return NULL;
}
const FCDETechnique* FCDExtra::FindTechnique(const char* profile) const
{
	for (FCDETechniqueContainer::const_iterator itT = techniques.begin(); itT != techniques.end(); ++itT)
	{
		if (IsEquivalent((*itT)->GetProfile(), profile)) return *itT;
	}
	return NULL;
}

// Search for a root node with a specific element name
FCDENode* FCDExtra::FindRootNode(const char* name)
{
	FCDENode* rootNode = NULL;
	for (FCDETechniqueContainer::iterator itT = techniques.begin(); itT != techniques.end(); ++itT)
	{
		rootNode = (*itT)->FindChildNode(name);
		if (rootNode != NULL) break;
	}
	return rootNode;
}
const FCDENode* FCDExtra::FindRootNode(const char* name) const
{
	FCDENode* rootNode = NULL;
	for (FCDETechniqueContainer::const_iterator itT = techniques.begin(); itT != techniques.end(); ++itT)
	{
		rootNode = (*itT)->FindChildNode(name);
		if (rootNode != NULL) break;
	}
	return rootNode;
}

FCDExtra* FCDExtra::Clone(FCDExtra* clone) const
{
	// If no clone is given: create one
	if (clone == NULL)
	{
		clone = new FCDExtra(const_cast<FCDocument*>(GetDocument()));
	}

	clone->techniques.reserve(techniques.size());
	for (FCDETechniqueContainer::const_iterator itT = techniques.begin(); itT != techniques.end(); ++itT)
	{
		FCDETechnique* cloneT = clone->AddTechnique((*itT)->GetProfile());
		(*itT)->Clone(cloneT);
	}
	return clone;
}

// Read in/Write to a COLLADA XML document
FUStatus FCDExtra::LoadFromXML(xmlNode* extraNode)
{
	FUStatus status;

	// Do NOT verify that we have an <extra> element: we may be parsing a technique switch instead.

	// Read in the techniques
	xmlNodeList techniqueNodes;
	FindChildrenByType(extraNode, DAE_TECHNIQUE_ELEMENT, techniqueNodes);
	for (xmlNodeList::iterator itN = techniqueNodes.begin(); itN != techniqueNodes.end(); ++itN)
	{
		xmlNode* techniqueNode = (*itN);
		string profile = ReadNodeProperty(techniqueNode, DAE_PROFILE_ATTRIBUTE);
		if (!IsEquivalent(profile, DAE_COMMON_PROFILE))
		{
			FCDETechnique* technique = AddTechnique(profile);
			status.AppendStatus(technique->LoadFromXML(techniqueNode));
		}
	}

	SetDirtyFlag();
	return status;
}

xmlNode* FCDExtra::WriteToXML(xmlNode* parentNode) const
{
	if (techniques.empty()) return NULL;

	// Add the <extra> element and its techniques
	xmlNode* extraNode = AddChildOnce(parentNode, DAE_EXTRA_ELEMENT);
	WriteTechniquesToXML(extraNode);
	return extraNode;
}

void FCDExtra::WriteTechniquesToXML(xmlNode* parentNode) const
{
	// Write the techniques within the given parent XML tree node.
	for (FCDETechniqueContainer::const_iterator itT = techniques.begin(); itT != techniques.end(); ++itT)
	{
		(*itT)->WriteToXML(parentNode);
	}
}

//
// FCDENode
//

ImplementObjectType(FCDENode);

FCDENode::FCDENode(FCDocument* document, FCDENode* _parent)
:	FCDObject(document), parent(_parent)
{
	animated = FCDAnimatedCustom::Create(GetDocument());
}

FCDENode::~FCDENode()
{
	parent = NULL;
}

void FCDENode::SetContent(const fchar* _content)
{
	// As COLLADA doesn't allow for mixed content, release all the children.
	while (!children.empty())
	{
		children.back()->Release();
	}

	content = _content;
	SetDirtyFlag();
}

// Search for a children with a specific name
FCDENode* FCDENode::FindChildNode(const char* name)
{
	for (FCDENodeContainer::iterator itN = children.begin(); itN != children.end(); ++itN)
	{
		if (IsEquivalent((*itN)->GetName(), name)) return (*itN);
	}
	return NULL;
}

const FCDENode* FCDENode::FindChildNode(const char* name) const
{
	for (FCDENodeContainer::const_iterator itN = children.begin(); itN != children.end(); ++itN)
	{
		if (IsEquivalent((*itN)->GetName(), name)) return (*itN);
	}
	return NULL;
}

const FCDENode* FCDENode::FindParameter(const char* name) const
{
	for (FCDENodeContainer::const_iterator itN = children.begin(); itN != children.end(); ++itN)
	{
		FCDENode* node = (*itN);
		if (IsEquivalent(node->GetName(), name)) return node;
		else if (IsEquivalent(node->GetName(), DAE_PARAMETER_ELEMENT))
		{
			FCDEAttribute* nameAttribute = node->FindAttribute(DAE_NAME_ATTRIBUTE);
			if (nameAttribute != NULL && nameAttribute->value == TO_FSTRING(name)) return node;
		}
	}
	return NULL;
}

void FCDENode::FindParameters(FCDENodeList& nodes, StringList& names)
{
	for (FCDENodeContainer::iterator itN = children.begin(); itN != children.end(); ++itN)
	{
		FCDENode* node = (*itN);
		if (node->GetChildNodeCount() > 1) continue;

		if (IsEquivalent(node->GetName(), DAE_PARAMETER_ELEMENT))
		{
			FCDEAttribute* nameAttribute = node->FindAttribute(DAE_NAME_ATTRIBUTE);
			if (nameAttribute != NULL)
			{
				nodes.push_back(node);
				names.push_back(FUStringConversion::ToString(nameAttribute->value));
			}
		}
		else 
		{
			nodes.push_back(node);
			names.push_back(node->GetName());
		}
	}
}

// Adds a new attribute to this extra tree node.
FCDEAttribute* FCDENode::AddAttribute(const char* _name, const fchar* _value)
{
	FCDEAttribute* attribute = FindAttribute(_name);
	if (attribute == NULL)
	{
		attribute = attributes.Add(_name, _value);
	}

	attribute->value = _value;
	SetDirtyFlag();
	return attribute;
}

// Search for an attribute with a specific name
FCDEAttribute* FCDENode::FindAttribute(const char* name)
{
	for (FCDEAttributeContainer::iterator itA = attributes.begin(); itA != attributes.end(); ++itA)
	{
		if ((*itA)->name == name) return (*itA);
	}
	return NULL;
}
const FCDEAttribute* FCDENode::FindAttribute(const char* name) const
{
	for (FCDEAttributeContainer::const_iterator itA = attributes.begin(); itA != attributes.end(); ++itA)
	{
		if ((*itA)->name == name) return (*itA);
	}
	return NULL;
}

FCDENode* FCDENode::AddParameter(const char* name, const fchar* value)
{
	FCDENode* parameter = AddChildNode();
	parameter->SetName(name);
	parameter->SetContent(value);
	SetDirtyFlag();
	return parameter;
}

FCDENode* FCDENode::Clone(FCDENode* clone) const
{
	if (clone == NULL) return NULL;

	clone->name = name;
	clone->content = content;

	clone->attributes.reserve(attributes.size());
	for (FCDEAttributeContainer::const_iterator itA = attributes.begin(); itA != attributes.end(); ++itA)
	{
		clone->AddAttribute((*itA)->name, (*itA)->value);
	}

	clone->children.reserve(children.size());
	for (FCDENodeContainer::const_iterator itC = children.begin(); itC != children.end(); ++itC)
	{
		FCDENode* clonedChild = clone->AddChildNode();
		(*itC)->Clone(clonedChild);
	}

	// TODO: Clone the animated custom..

	return clone;
}

// Read in this extra node from a COLLADA XML document
FUStatus FCDENode::LoadFromXML(xmlNode* customNode)
{
	FUStatus status;

	// Read in the node's name and children
	name = (const char*) customNode->name;
	ReadChildrenFromXML(customNode);
	
	// If there are no child nodes, we have a tree leaf: parse in the content and its animation
	content = (children.empty()) ? TO_FSTRING(ReadNodeContentFull(customNode)) : FS("");
	SAFE_RELEASE(animated);
	animated = FCDAnimatedCustom::Create(GetDocument(), customNode);

	// Read in the node's attributes
	for (xmlAttr* a = customNode->properties; a != NULL; a = a->next)
	{
		AddAttribute((const char*) a->name, (a->children != NULL) ? TO_FSTRING((const char*) (a->children->content)) : FS(""));
	}

	SetDirtyFlag();
	return status;
}

// Write out this extra to a COLLADA XML document
xmlNode* FCDENode::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* customNode = AddChild(parentNode, name.c_str(), content);
	
	// Write out the attributes
	for (FCDEAttributeContainer::const_iterator itA = attributes.begin(); itA != attributes.end(); ++itA)
	{
		const FCDEAttribute* attribute = (*itA);
		FUXmlWriter::AddAttribute(customNode, attribute->name.c_str(), attribute->value);
	}

	// Write out the animated element
	if (animated != NULL && animated->HasCurve())
	{
		GetDocument()->WriteAnimatedValueToXML(animated, customNode, name.c_str());
	}

	// Write out the children
	WriteChildrenToXML(customNode);
	return customNode;
}

// Read in the child nodes from the XML tree node
FUStatus FCDENode::ReadChildrenFromXML(xmlNode* customNode)
{
	FUStatus status;

	// Read in the node's children
	for (xmlNode* k = customNode->children; k != NULL; k = k->next)
	{
		if (k->type != XML_ELEMENT_NODE) continue;

		FCDENode* node = AddChildNode();
		status.AppendStatus(node->LoadFromXML(k));
	}

	SetDirtyFlag();
	return status;
}

// Write out the child nodes to the XML tree node
void FCDENode::WriteChildrenToXML(xmlNode* customNode) const
{
	for (FCDENodeContainer::const_iterator itN = children.begin(); itN != children.end(); ++itN)
	{
		(*itN)->WriteToXML(customNode);
	}
}

//
// FCDETechnique
//

ImplementObjectType(FCDETechnique);

FCDETechnique::FCDETechnique(FCDocument* document, const char* _profile) : FCDENode(document, NULL)
{
	if(_profile)
		profile = _profile;
	else
		profile = "";
}

FCDETechnique::~FCDETechnique() {}

FCDENode* FCDETechnique::Clone(FCDENode* clone) const
{
	if (clone == NULL)
	{
		clone = new FCDETechnique(const_cast<FCDocument*>(GetDocument()), profile.c_str());
	}
	else if (clone->GetObjectType().Includes(FCDETechnique::GetClassType()))
	{
		((FCDETechnique*) clone)->profile = profile;
	}

	FCDENode::Clone(clone);
	return clone;
}

// Read in/Write to a COLLADA XML document
FUStatus FCDETechnique::LoadFromXML(xmlNode* techniqueNode)
{
	FUStatus status;

	// Read in only the child elements: none of the attributes
	status.AppendStatus(ReadChildrenFromXML(techniqueNode));
	return status;
}

xmlNode* FCDETechnique::WriteToXML(xmlNode* parentNode) const
{
	// Create the technique for this profile and write out the children
	xmlNode* customNode = AddTechniqueChild(parentNode, profile.c_str());
	WriteChildrenToXML(customNode);
	return customNode;
}

//
// FCDEAttribute
//

ImplementObjectType(FCDEAttribute);

FCDEAttribute::FCDEAttribute(const char* _name, const fchar* _value)
{
	name = _name;
	value = _value;
}

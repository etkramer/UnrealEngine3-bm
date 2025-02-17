/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDEffectTechnique.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDEffectPass.h"
#include "FCDocument/FCDEffectPassShader.h"
#include "FCDocument/FCDEffectParameter.h"
#include "FCDocument/FCDEffectParameterList.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

ImplementObjectType(FCDEffectPass);

FCDEffectPass::FCDEffectPass(FCDEffectTechnique *_parent)
:	FCDObject(_parent->GetDocument()), parent(_parent)
{
}

FCDEffectPass::~FCDEffectPass()
{
	parent = NULL;
}

// Adds a new shader to the pass.
FCDEffectPassShader* FCDEffectPass::AddShader()
{
	FCDEffectPassShader* shader = new FCDEffectPassShader(this);
	shaders.push_back(shader);
	SetDirtyFlag();
	return shader;
}

// Releases a shader contained within the pass.
void FCDEffectPass::ReleaseShader(FCDEffectPassShader* shader)
{
	SetDirtyFlag();
	shaders.release(shader);
}

// Adds a new vertex shader to the pass.
// If a vertex shader already exists within the pass, it will be released.
FCDEffectPassShader* FCDEffectPass::AddVertexShader()
{
	FCDEffectPassShader* vertexShader;
	for (vertexShader = GetVertexShader(); vertexShader != NULL; vertexShader = GetVertexShader())
	{
		ReleaseShader(vertexShader);
	}

	vertexShader = AddShader();
	vertexShader->AffectsVertices();
	SetDirtyFlag();
	return vertexShader;
}

// Adds a new fragment shader to the pass.
// If a fragment shader already exists within the pass, it will be released.
FCDEffectPassShader* FCDEffectPass::AddFragmentShader()
{
	FCDEffectPassShader* fragmentShader;
	for (fragmentShader = GetFragmentShader(); fragmentShader != NULL; fragmentShader = GetFragmentShader())
	{
		ReleaseShader(fragmentShader);
	}

	fragmentShader = AddShader();
	fragmentShader->AffectsFragments();
	SetDirtyFlag();
	return fragmentShader;
}

FCDEffectPass* FCDEffectPass::Clone(FCDEffectPass* clone) const
{
	if (clone == NULL) clone = new FCDEffectPass(parent);

	clone->name = name;

	// Clone the shaderss
	clone->shaders.reserve(shaders.size());
	for (FCDEffectPassShaderContainer::const_iterator itS = shaders.begin(); itS != shaders.end(); ++itS)
	{
		FCDEffectPassShader* clonedShader = clone->AddShader();
		(*itS)->Clone(clonedShader);
	}

	return clone;
}

const string& FCDEffectPass::GetDaeId() const
{
	return parent->GetDaeId();
}

// Retrieve the type-specific shaders
FCDEffectPassShader* FCDEffectPass::GetVertexShader()
{
	for (FCDEffectPassShaderContainer::iterator itS = shaders.begin(); itS != shaders.end(); ++itS)
	{
		if ((*itS)->IsVertexShader()) return (*itS);
	}
	return NULL;
}
const FCDEffectPassShader* FCDEffectPass::GetVertexShader() const
{
	for (FCDEffectPassShaderContainer::const_iterator itS = shaders.begin(); itS != shaders.end(); ++itS)
	{
		if ((*itS)->IsVertexShader()) return (*itS);
	}
	return NULL;
}

FCDEffectPassShader* FCDEffectPass::GetFragmentShader()
{
	for (FCDEffectPassShaderContainer::iterator itS = shaders.begin(); itS != shaders.end(); ++itS)
	{
		if ((*itS)->IsFragmentShader()) return (*itS);
	}
	return NULL;
}
const FCDEffectPassShader* FCDEffectPass::GetFragmentShader() const
{
	for (FCDEffectPassShaderContainer::const_iterator itS = shaders.begin(); itS != shaders.end(); ++itS)
	{
		if ((*itS)->IsFragmentShader()) return (*itS);
	}
	return NULL;
}

FUStatus FCDEffectPass::LoadFromXML(xmlNode* passNode, xmlNode* UNUSED(techniqueNode), xmlNode* UNUSED(profileNode))
{
	FUStatus status;
	if (!IsEquivalent(passNode->name, DAE_PASS_ELEMENT))
	{
		return status.Warning(FS("Pass contains unknown element."), passNode->line);
	}
	name = TO_FSTRING(ReadNodeProperty(passNode, DAE_SID_ATTRIBUTE));

	// Look for the <shader> elements
	xmlNodeList shaderNodes;
	FindChildrenByType(passNode, DAE_SHADER_ELEMENT, shaderNodes);
	for (xmlNodeList::iterator itS = shaderNodes.begin(); itS != shaderNodes.end(); ++itS)
	{
		FCDEffectPassShader* shader = new FCDEffectPassShader(this);
		shaders.push_back(shader);
		status.AppendStatus(shader->LoadFromXML(*itS));
	}

	SetDirtyFlag();
	return status;
}

// Write out the pass to the COLLADA XML node tree
xmlNode* FCDEffectPass::WriteToXML(xmlNode* parentNode) const
{
	// Write out the <pass> element, with the shader's name
	xmlNode* passNode = AddChild(parentNode, DAE_PASS_ELEMENT);
	if (!name.empty())
	{
		fstring& _name = const_cast<fstring&>(name);
		AddNodeSid(passNode, _name);
	}

	// [GLAFORTE] Just always dump these render states out, for now.
	// Later, we'll add the full list?
	xmlNode* cullNode = AddChild(passNode, "cull_face_enable");
	AddAttribute(cullNode, "value", "true");
	
	xmlNode* maskNode = AddChild(passNode, "depth_mask");
	AddAttribute(maskNode, "value", "true");
	
	xmlNode* testNode = AddChild(passNode, "depth_test_enable");
	AddAttribute(testNode, "value", "true");

	// Write out the shaders
	for (FCDEffectPassShaderContainer::const_iterator itS = shaders.begin(); itS != shaders.end(); ++itS)
	{
		(*itS)->WriteToXML(passNode);
	}

	return passNode;
}

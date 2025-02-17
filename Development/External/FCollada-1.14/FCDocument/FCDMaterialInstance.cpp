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
#include "FCDocument/FCDEffectParameter.h"
#include "FCDocument/FCDEffectParameterFactory.h"
#include "FCDocument/FCDEffectParameterList.h"
#include "FCDocument/FCDGeometry.h"
#include "FCDocument/FCDGeometryMesh.h"
#include "FCDocument/FCDGeometryPolygons.h"
#include "FCDocument/FCDController.h"
#include "FCDocument/FCDExternalReference.h"
#include "FCDocument/FCDGeometryInstance.h"
#include "FCDocument/FCDGeometryMesh.h"
#include "FCDocument/FCDGeometryPolygons.h"
#include "FCDocument/FCDMaterial.h"
#include "FCDocument/FCDMaterialInstance.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

ImplementObjectType(FCDMaterialInstance);

FCDMaterialInstance::FCDMaterialInstance(FCDGeometryInstance* _parent)
:	FCDEntityInstance(_parent->GetDocument(), NULL, FCDEntity::MATERIAL), parent(_parent)
{
}

FCDMaterialInstance::~FCDMaterialInstance()
{
	bindings.clear();
	vertexBindings.clear();
	parent = NULL;
}

FCDGeometryPolygons* FCDMaterialInstance::GetPolygons()
{
	if (parent != NULL && parent->GetEntity() != NULL)
	{
		FCDEntity* e = parent->GetEntity();
		if (e->GetObjectType().Includes(FCDController::GetClassType()))
		{
			e = ((FCDController*) e)->GetBaseGeometry();
		}
		if (e->GetObjectType().Includes(FCDGeometry::GetClassType()))
		{
			FCDGeometry* geometry = (FCDGeometry*) e;
			if (geometry->IsMesh())
			{
				FCDGeometryMesh* mesh = geometry->GetMesh();
				size_t polygonsCount = mesh->GetPolygonsCount();
				for (size_t i = 0; i < polygonsCount; ++i)
				{
					FCDGeometryPolygons* polygons = mesh->GetPolygons(i);
					if (IsEquivalent(mesh->GetPolygons(i)->GetMaterialSemantic(), semantic))
					{
						return polygons;
					}
				}
			}
		}
	}
	return NULL;
}

FCDMaterialInstanceBindTextureSurface* FCDMaterialInstance::AddTextureSurfaceBinding()
{
	texSurfBindings.push_back( FCDMaterialInstanceBindTextureSurface() );
	SetDirtyFlag();
	return &texSurfBindings.back();
}

FCDMaterialInstanceBindTextureSurface* FCDMaterialInstance::AddTextureSurfaceBinding(const char* semantic, int32 surfIndex, int32 texSurfIndex )
{
	FCDMaterialInstanceBindTextureSurface* tsbinding = AddTextureSurfaceBinding();
	tsbinding->semantic = semantic;
	tsbinding->surfaceIndex = surfIndex;
	tsbinding->textureIndex = texSurfIndex;
	return tsbinding;
}


FCDMaterialInstanceBindVertexInput* FCDMaterialInstance::AddVertexInputBinding()
{
	vertexBindings.push_back(FCDMaterialInstanceBindVertexInput());
	SetDirtyFlag();
	return &vertexBindings.back();
}

FCDMaterialInstanceBindVertexInput* FCDMaterialInstance::AddVertexInputBinding(const char* semantic, FUDaeGeometryInput::Semantic inputSemantic, int32 inputSet)
{
	FCDMaterialInstanceBindVertexInput* vbinding = AddVertexInputBinding();
	vbinding->semantic = semantic;
	vbinding->inputSemantic = inputSemantic;
	vbinding->inputSet = inputSet;
	return vbinding;
}

FCDMaterialInstanceBind* FCDMaterialInstance::AddBinding()
{
	bindings.push_back(FCDMaterialInstanceBind());
	SetDirtyFlag();
	return &bindings.back();
}

FCDMaterialInstanceBind* FCDMaterialInstance::AddBinding(const char* semantic, const char* target)
{
	FCDMaterialInstanceBind* binding = AddBinding();
	binding->semantic = semantic;
	binding->target = target;
	return binding;
}

void FCDMaterialInstance::ReleaseBinding(size_t index)
{
	FUAssert(index < bindings.size(), return);
	bindings.erase(index);
}

// Create a flattened version of the instantiated material: this is the
// prefered way to generate DCC/viewer materials from a COLLADA document
FCDMaterial* FCDMaterialInstance::FlattenMaterial()
{
	// Retrieve the necessary geometry and material information
	FCDGeometry* geometry = NULL;
	if (parent->GetEntity() == NULL)
		return NULL;
	if (parent->GetEntity()->GetType() == FCDEntity::CONTROLLER)
	{
		FCDController* controller = (FCDController*) parent->GetEntity();
		FCDEntity* baseTarget = controller->GetBaseGeometry();
		if(baseTarget->GetType() == FCDEntity::GEOMETRY)
		{
			geometry = (FCDGeometry*) baseTarget;
		}
	}
	else if (parent->GetEntity()->GetType() == FCDEntity::GEOMETRY)
	{
		geometry = (FCDGeometry*) parent->GetEntity();
	}

	// Retrieve the original material and verify that we have all the information necessary.
	FCDMaterial* material = GetMaterial();
	if (material == NULL || geometry == NULL || (!geometry->IsMesh() && !geometry->IsPSurface()))
		return NULL;

	if( geometry->IsMesh() )
	{
		// Retrieve the correct polygons for this material semantic
		FCDGeometryMesh* mesh = geometry->GetMesh();
		size_t polygonsCount = mesh->GetPolygonsCount();
		FCDGeometryPolygons* polygons = NULL;
		for (size_t i = 0; i < polygonsCount; ++i)
		{
			FCDGeometryPolygons* p = mesh->GetPolygons(i);
			if (semantic == p->GetMaterialSemantic()) { polygons = p; break; }
		}
		if (polygons == NULL)
			return NULL;

		FCDMaterial* clone = material->Clone();
		clone->Flatten();

		// Flatten: Apply the bindings to the cloned material
		for (FCDMaterialInstanceBindList::iterator itB = bindings.begin(); itB != bindings.end(); ++itB)
		{
			FCDEffectParameterList parameters;
			clone->FindParametersBySemantic((*itB).semantic, parameters);
			for (FCDEffectParameterList::iterator itP = parameters.begin(); itP != parameters.end(); ++itP)
			{
				FCDEffectParameter* param = (*itP);
				if (param->GetType() == FCDEffectParameter::INTEGER)
				{
					FCDEffectParameterInt* intParam = (FCDEffectParameterInt*) param;

					// Fairly hacky: only supported bind type right now is the texture-texture coordinate sets, which are never animated

					// Resolve the target as a geometry source
					FCDGeometryPolygonsInput* input = polygons->FindInput((*itB).target);
					if (input != NULL) intParam->SetValue(input->set);
				}
			}
		}
		return clone;
	}
	else if( geometry->IsPSurface() )
	{
		FCDMaterial* clone = material->Clone();
		clone->Flatten();
		// anything else important to do here?
		return clone;
	}

	return NULL;
}

FCDEntityInstance* FCDMaterialInstance::Clone(FCDEntityInstance* _clone) const
{
	FCDMaterialInstance* clone = NULL;
	if (_clone == NULL) clone = new FCDMaterialInstance(const_cast<FCDGeometryInstance*>(parent));
	else if (!_clone->HasType(FCDMaterialInstance::GetClassType())) return Parent::Clone(_clone);
	else clone = (FCDMaterialInstance*) _clone;

	Parent::Clone(clone);

	// Clone the bindings and the semantic information.
	clone->semantic = semantic;
	clone->bindings = bindings; // since these two are not vectors of pointers, the copy constructors will work things out.
	clone->vertexBindings = vertexBindings;
	clone->texSurfBindings = texSurfBindings;
	return clone;
}

// Read in the <instance_material> element from the COLLADA document
FUStatus FCDMaterialInstance::LoadFromXML(xmlNode* instanceNode)
{
	// [glaforte] Do not execute the FCDEntityInstance::LoadFromXML parent function
	// as the <instance_material> element does not have the 'url' attribute.
	FUStatus status;
	bindings.clear();

	semantic = TO_FSTRING(ReadNodeProperty(instanceNode, DAE_SYMBOL_ATTRIBUTE));
	string materialId = ReadNodeProperty(instanceNode, DAE_TARGET_ATTRIBUTE);

	FUUri uri(materialId);
	if( !uri.prefix.empty() )
	{
		// this is an XRef material
		SetMaterial(NULL);
		FCDExternalReference *xref = GetExternalReference();
		xref->SetUri( uri );
	}
	else
	{
		FCDMaterial* material = GetDocument()->FindMaterial(materialId);
		if (material == NULL)
		{
			status.Warning(FS("Invalid material binding in geometry instantiation."), instanceNode->line);
		}
		SetMaterial(material);
	}

	// Read in the ColladaFX bindings
	xmlNodeList bindNodes;
	FindChildrenByType(instanceNode, DAE_BIND_ELEMENT, bindNodes);
	for (xmlNodeList::iterator itB = bindNodes.begin(); itB != bindNodes.end(); ++itB)
	{
		AddBinding(ReadNodeSemantic(*itB), ReadNodeProperty(*itB, DAE_TARGET_ATTRIBUTE));
	}

	// Read in the ColladaFX vertex inputs
	xmlNodeList bindVertexNodes;
	FindChildrenByType(instanceNode, DAE_BIND_VERTEX_INPUT_ELEMENT, bindVertexNodes);
	for (xmlNodeList::iterator itB = bindVertexNodes.begin(); itB != bindVertexNodes.end(); ++itB)
	{
		string inputSet = ReadNodeProperty(*itB, DAE_INPUT_SET_ATTRIBUTE);
		string inputSemantic = ReadNodeProperty(*itB, DAE_INPUT_SEMANTIC_ATTRIBUTE);
		AddVertexInputBinding(ReadNodeSemantic(*itB).c_str(), FUDaeGeometryInput::FromString(inputSemantic.c_str()), FUStringConversion::ToInt32(inputSet));
	}

	// Read in the ColladaFX texture surface bindings
	xmlNodeList bindTexSurfNodes;
	FindChildrenByType(instanceNode, DAE_BIND_TEXTURE_SURFACE_ELEMENT, bindTexSurfNodes);
	for( xmlNodeList::iterator itB = bindTexSurfNodes.begin(); itB != bindTexSurfNodes.end(); ++itB)
	{
		int32 surfIndex = FUStringConversion::ToInt32( ReadNodeProperty(*itB, DAE_SURFACE_ATTRIBUTE ) );
		int32 texSurfIndex = FUStringConversion::ToInt32( ReadNodeProperty(*itB, DAE_TEXTURE_ATTRIBUTE ) );
		AddTextureSurfaceBinding( ReadNodeSemantic(*itB).c_str(), surfIndex, texSurfIndex );
	}

	SetDirtyFlag();
	return status;
}

FUStatus FCDMaterialInstance::LoadFromId(const string& materialId)
{
	FUStatus status;
	bindings.clear();

	// Copy the semantic over
	semantic = TO_FSTRING(materialId);

	// Find the material associated with this Id and clone it.
	FCDMaterial* material = GetDocument()->FindMaterial(materialId);
	if (material == NULL)
	{
		return status.Warning(FS("Unknown material id or semantic: ") + TO_FSTRING(materialId));
	}
	SetMaterial(material);
	SetDirtyFlag();
	return status;
}

// Write out the instantiation information to the XML node tree
xmlNode* FCDMaterialInstance::WriteToXML(xmlNode* parentNode) const
{
	// Intentionally skip the parent WriteToXML class
	xmlNode* instanceNode = AddChild(parentNode, DAE_INSTANCE_MATERIAL_ELEMENT);

	AddAttribute(instanceNode, DAE_SYMBOL_ATTRIBUTE, semantic);
	if (IsExternalReference())
	{
		FUUri xrefUri = GetExternalReference()->GetUri();
		AddAttribute(instanceNode, DAE_TARGET_ATTRIBUTE, TO_STRING(xrefUri.prefix) + "#" + xrefUri.suffix);
	}
	else
	{
		const FCDMaterial* material = GetMaterial();
		AddAttribute(instanceNode, DAE_TARGET_ATTRIBUTE, (material != NULL) ? string("#") + material->GetDaeId() : "");
	}

	// Write out the bindings.
	for (FCDMaterialInstanceBindList::const_iterator itB = bindings.begin(); itB != bindings.end(); ++itB)
	{
		const FCDMaterialInstanceBind& bind = *itB;
		xmlNode* bindNode = AddChild(instanceNode, DAE_BIND_ELEMENT);
		AddAttribute(bindNode, DAE_SEMANTIC_ATTRIBUTE, bind.semantic);
		AddAttribute(bindNode, DAE_TARGET_ATTRIBUTE, bind.target);
	}
	
	// Write out the vertex input bindings.
	for (FCDMaterialInstanceBindVertexInputList::const_iterator itV = vertexBindings.begin(); itV != vertexBindings.end(); ++itV)
	{
		const FCDMaterialInstanceBindVertexInput& bind = *itV;
		xmlNode* bindNode = AddChild(instanceNode, DAE_BIND_VERTEX_INPUT_ELEMENT);
		AddAttribute(bindNode, DAE_SEMANTIC_ATTRIBUTE, bind.semantic);
		AddAttribute(bindNode, DAE_INPUT_SEMANTIC_ATTRIBUTE, FUDaeGeometryInput::ToString(bind.inputSemantic));
		AddAttribute(bindNode, DAE_INPUT_SET_ATTRIBUTE, bind.inputSet);
	}

	// Write out the texture surface bindings
	for (FCDMaterialInstanceBindTextureSurfaceList::const_iterator itT = texSurfBindings.begin(); itT != texSurfBindings.end(); ++itT)
	{
		const FCDMaterialInstanceBindTextureSurface& bind = *itT;
		xmlNode* bindNode = AddChild(instanceNode, DAE_BIND_TEXTURE_SURFACE_ELEMENT);
		AddAttribute(bindNode, DAE_SEMANTIC_ATTRIBUTE, bind.semantic);
		AddAttribute(bindNode, DAE_SURFACE_ATTRIBUTE, bind.surfaceIndex);
		AddAttribute(bindNode, DAE_TEXTURE_ATTRIBUTE, bind.textureIndex);
	}

	return instanceNode;
}

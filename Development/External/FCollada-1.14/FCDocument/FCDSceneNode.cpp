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
#include "FCDocument/FCDAsset.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDAnimated.h"
#include "FCDocument/FCDAnimationCurve.h"
#include "FCDocument/FCDCamera.h"
#include "FCDocument/FCDController.h"
#include "FCDocument/FCDControllerInstance.h"
#include "FCDocument/FCDEntityInstance.h"
#include "FCDocument/FCDExtra.h"
#include "FCDocument/FCDGeometry.h"
#include "FCDocument/FCDGeometryInstance.h"
#include "FCDocument/FCDLight.h"
#include "FCDocument/FCDPhysicsModelInstance.h"
#include "FCDocument/FCDPhysicsRigidBodyInstance.h"
#include "FCDocument/FCDSceneNode.h"
#include "FCDocument/FCDTransform.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
#include "FUtils/FUFileManager.h"
#include "FUtils/FUStringConversion.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

ImplementObjectType(FCDSceneNode);

FCDSceneNode::FCDSceneNode(FCDocument* document)
:	FCDEntity(document, "VisualSceneNode")
,	visibility(1.0f), targetCount(0), isJoint(false), asset(NULL)
{
}

FCDSceneNode::~FCDSceneNode()
{
	parents.clear();
	SAFE_DELETE(asset);

	// Delete the children, be watchful for the instantiated nodes
	while (!children.empty())
	{
		FCDSceneNode* child = children.front();
		child->parents.erase(this);
		
		if (child->parents.empty())
		{
			SAFE_DELETE(child);
		}
		else
		{
			// Check for external references in the parents
			bool hasLocalReferences = false;
			for (FCDSceneNodeTrackList::iterator itP = parents.begin(); itP != parents.end();)
			{
				if ((*itP) == this) children.erase(itP);
				else
				{
					hasLocalReferences |= (*itP)->GetDocument() == GetDocument();
					++itP;
				}
			}

			if (!hasLocalReferences) SAFE_DELETE(child);
		}
	}
}

// Add this scene node to the list of children scene node
bool FCDSceneNode::AddChildNode(FCDSceneNode* sceneNode)
{
	if (this == sceneNode || sceneNode == NULL)
	{
		return false;
	}

	// Verify that we don't already contain this child node.
	if (children.contains(sceneNode)) return false;

	// Verify that this node is not one of the parents in the full hierarchically.
	FCDSceneNodeList queue = parents;
	while (!queue.empty())
	{
		FCDSceneNode* parent = queue.back();
		queue.pop_back();
		if (parent == sceneNode) return false;
		queue.insert(queue.end(), parent->parents.begin(), parent->parents.end());
	}

	children.push_back(sceneNode);
	sceneNode->parents.push_back(this);
	SetDirtyFlag();
	return true;
}

FCDSceneNode* FCDSceneNode::AddChildNode()
{
	FCDSceneNode* node = new FCDSceneNode(GetDocument());
	AddChildNode(node);
	return node;
}

// Instantiates an entity
FCDEntityInstance* FCDSceneNode::AddInstance(FCDEntity* entity)
{
	FCDEntityInstance* instance = NULL;
	if (entity == NULL) return NULL;

	switch (entity->GetType())
	{
	case FCDEntity::CAMERA: instance = new FCDEntityInstance(GetDocument(), this, FCDEntity::CAMERA); break;
	case FCDEntity::CONTROLLER: instance = new FCDControllerInstance(GetDocument(), this, FCDEntity::CONTROLLER); break;
	case FCDEntity::LIGHT: instance = new FCDEntityInstance(GetDocument(), this, FCDEntity::LIGHT); break;
	case FCDEntity::GEOMETRY: instance = new FCDGeometryInstance(GetDocument(), this, FCDEntity::GEOMETRY); break;
	case FCDEntity::SCENE_NODE:
		instance = new FCDEntityInstance(GetDocument(), this, FCDEntity::SCENE_NODE);
		if (entity->GetDocument() == GetDocument())	AddChildNode((FCDSceneNode*) entity);
		break;

	default: return NULL;
	}

	instance->SetEntity(entity);
	instances.push_back(instance);
	SetDirtyFlag();
	return instance;
}

FCDEntityInstance* FCDSceneNode::AddInstance(FCDEntity::Type type)
{
	FCDEntityInstance* instance = NULL;
	switch (type)
	{
	case FCDEntity::CAMERA:
	case FCDEntity::LIGHT:
	case FCDEntity::SCENE_NODE:
		instance = new FCDEntityInstance(GetDocument(), this, type); break;
	case FCDEntity::GEOMETRY:
		instance = new FCDGeometryInstance(GetDocument(), this, type); break;
	case FCDEntity::CONTROLLER:
		instance = new FCDControllerInstance(GetDocument(), this, type); break;
	default: return NULL;
	}

	instances.push_back(instance);
	SetDirtyFlag();
	return instance;
}

// Adds a transform to the stack, at a given position.
FCDTransform* FCDSceneNode::AddTransform(FCDTransform::Type type, size_t index)
{
	FCDTransform* transform = FCDTFactory::CreateTransform(GetDocument(), this, type);
	if (transform != NULL)
	{
		if (index > transforms.size()) transforms.push_back(transform);
		else transforms.insert(transforms.begin() + index, transform);
	}
	SetDirtyFlag();
	return transform;
}

// Traverse the scene graph, searching for a node with the given COLLADA id
FCDEntity* FCDSceneNode::FindDaeId(const string& daeId)
{
	if (GetDaeId() == daeId) return this;
	
	for (FCDSceneNodeTrackList::iterator it = children.begin(); it != children.end(); ++it)
	{
		FCDEntity* found = (*it)->FindDaeId(daeId);
		if (found != NULL) return found;
	}
	return NULL;
}

// Calculate the transform matrix for a given scene node
FMMatrix44 FCDSceneNode::ToMatrix() const
{
	FMMatrix44 localTransform = FMMatrix44::Identity;
	for (FCDTransformContainer::const_iterator it = transforms.begin(); it != transforms.end(); ++it)
	{
		localTransform = localTransform * (*it)->ToMatrix();
	}
	return localTransform;
}

FMMatrix44 FCDSceneNode::CalculateWorldTransform() const
{
	const FCDSceneNode* parent = GetParent();
	if (parent != NULL)
	{
		return parent->CalculateWorldTransform() * CalculateLocalTransform();
	}
	else
	{
		return CalculateLocalTransform();
	}
}

void FCDSceneNode::GenerateSampledMatrixAnimation(FloatList& sampleKeys, FMMatrix44List& sampleValues)
{
	FCDAnimatedList animateds;
	
	// Collect all the animation curves
	for (FCDTransformContainer::iterator it = transforms.begin(); it != transforms.end(); ++it)
	{
		FCDAnimated* animated = (*it)->GetAnimated();
		if (animated != NULL && animated->HasCurve()) animateds.push_back(animated);
	}
	if (animateds.empty()) return;

	// Make a list of the ordered key times to sample
	for (FCDAnimatedList::iterator it = animateds.begin(); it != animateds.end(); ++it)
	{
		const FCDAnimationCurveListList& allCurves = (*it)->GetCurves();
		for (FCDAnimationCurveListList::const_iterator allCurveIt = allCurves.begin(); allCurveIt != allCurves.end(); ++allCurveIt)
		{
			const FCDAnimationCurveList& curves = (*allCurveIt);
			if (curves.empty()) continue;

			const FloatList& curveKeys = curves.front()->GetKeys();
			size_t sampleKeyCount = sampleKeys.size();
			size_t curveKeyCount = curveKeys.size();
			
			// Merge this curve's keys in with the sample keys
			// This assumes both key lists are in increasing order
			size_t s = 0, c = 0;
			while (s < sampleKeyCount && c < curveKeyCount)
			{
				float sampleKey = sampleKeys[s], curveKey = curveKeys[c];
				if (IsEquivalent(sampleKey, curveKey)) { ++s; ++c; }
				else if (sampleKey < curveKey) { ++s; }
				else
				{
					// Add this curve key to the sampling key list
					sampleKeys.insert(sampleKeys.begin() + (s++), curveKeys[c++]);
					sampleKeyCount++;
				}
			}

			// Add all the left-over curve keys to the sampling key list
			if (c < curveKeyCount) sampleKeys.insert(sampleKeys.end(), curveKeys.begin() + c, curveKeys.end());
		}
	}
	size_t sampleKeyCount = sampleKeys.size();
	if (sampleKeyCount == 0) return;

	// Pre-allocate the value array;
	sampleValues.reserve(sampleKeyCount);
	
	// Sample the scene node transform
	for (size_t i = 0; i < sampleKeyCount; ++i)
	{
		float sampleTime = sampleKeys[i];
		for (FCDAnimatedList::iterator it = animateds.begin(); it != animateds.end(); ++it)
		{
			// Sample each animated, which changes the transform values directly
			(*it)->Evaluate(sampleTime);
		}

		// Retrieve the new transform matrix for the COLLADA scene node
		sampleValues.push_back(ToMatrix());
	}
}

// Parse a <scene> or a <node> node from a COLLADA document
FUStatus FCDSceneNode::LoadFromXML(xmlNode* sceneNode)
{
	FUStatus status = FCDEntity::LoadFromXML(sceneNode);
	if (!status) return status;
	if (!IsEquivalent(sceneNode->name, DAE_VSCENE_ELEMENT) && !IsEquivalent(sceneNode->name, DAE_NODE_ELEMENT)
		&& !IsEquivalent(sceneNode->name, DAE_SCENE_ELEMENT))
	{
		// The <scene> element is accepted here only as COLLADA 1.3 backward compatibility
		return status.Fail(FS("Unknown scene node element with id: ") + TO_FSTRING(GetDaeId()), sceneNode->line);
	}

	// Read in the <node> element's type
	string nodeType = ReadNodeProperty(sceneNode, DAE_TYPE_ATTRIBUTE);
	if (nodeType == DAE_JOINT_NODE_TYPE) SetJointFlag(true);
	else if (nodeType.length() == 0 || nodeType == DAE_NODE_NODE_TYPE) {} // No special consideration
	else
	{
		status.Warning(FS("Unknown node type for scene's <node> element: ") + TO_FSTRING(GetDaeId()), sceneNode->line);
	}

	// The scene node has ordered elements, so process them directly and in order.
	for (xmlNode* child = sceneNode->children; child != NULL; child = child->next)
	{
		if (child->type != XML_ELEMENT_NODE) continue;

		if (IsEquivalent(child->name, DAE_NODE_ELEMENT))
		{
			// Load the child scene node
			FCDSceneNode* node = AddChildNode();
			status = node->LoadFromXML(child);
			if (!status) break;
		}

		// Look for instantiation elements
#define INSTANTIATE(name, instanceType, entityType) { \
			instanceType* instance = new instanceType(GetDocument(), this, entityType); \
			instances.push_back(instance); \
			status.AppendStatus(instance->LoadFromXML(child)); \
			continue; }

		else if (IsEquivalent(child->name, DAE_INSTANCE_CAMERA_ELEMENT)) { INSTANTIATE(Camera, FCDEntityInstance, FCDEntity::CAMERA); }
		else if (IsEquivalent(child->name, DAE_INSTANCE_CONTROLLER_ELEMENT)) { INSTANTIATE(Controller, FCDControllerInstance, FCDEntity::CONTROLLER); }
		else if (IsEquivalent(child->name, DAE_INSTANCE_GEOMETRY_ELEMENT)) { INSTANTIATE(Geometry, FCDGeometryInstance, FCDEntity::GEOMETRY); }
		else if (IsEquivalent(child->name, DAE_INSTANCE_LIGHT_ELEMENT)) { INSTANTIATE(Light, FCDEntityInstance, FCDEntity::LIGHT); }
		else if (IsEquivalent(child->name, DAE_INSTANCE_NODE_ELEMENT))
		{
			FUUri url = ReadNodeUrl(child);
			if (url.prefix.empty())
			{
				// cannot find the node
				FCDSceneNode* node = GetDocument()->FindSceneNode(url.suffix);
				if (node != NULL)
				{
					
					if (!AddChildNode(node))
					{
						status.Warning(FS("A cycle was found in the visual scene at node: ") + TO_FSTRING(GetDaeId()), child->line);
					}
				}
				else
				{
					
					status.Warning(FS("Unable to retrieve node instance for scene node: ") + TO_FSTRING(GetDaeId()), child->line);
				}
			}
			else
			{
				FCDEntityInstance* reference = new FCDEntityInstance(GetDocument(), this, FCDEntity::SCENE_NODE);
				reference->LoadFromXML(child);
				instances.push_back(reference);
			}
		}
#undef INSTANTIATE

		else if (IsEquivalent(child->name, DAE_INSTANCE_ELEMENT))
		{
			// COLLADA 1.3 backward compatibility: Weakly-typed instantiation
			// Might be a geometry, controller, camera or light.
			// No support for COLLADA 1.3 external references, as there is no way to determine the type directly.

			FUUri url = ReadNodeUrl(child);
			if (url.prefix.empty())
			{
#define INSTANTIATE(name, entityType, instanceType) { \
					FCD##name* entity = GetDocument()->Find##name(url.suffix); \
					if (entity != NULL) { \
						instanceType* instance = new instanceType(GetDocument(), this, entityType); \
						instances.push_back(instance); \
						instance->LoadFromXML(child); \
						continue; } }

				INSTANTIATE(Geometry, FCDEntity::GEOMETRY, FCDGeometryInstance);
				INSTANTIATE(Controller, FCDEntity::CONTROLLER, FCDControllerInstance);
				INSTANTIATE(Camera, FCDEntity::CAMERA, FCDEntityInstance);
				INSTANTIATE(Light, FCDEntity::LIGHT, FCDEntityInstance);
#undef INSTANTIATE

				FCDSceneNode* node = GetDocument()->FindSceneNode(url.suffix);
				if (node != NULL)
				{
					if (!AddChildNode(node))
					{
						status.Warning(FS("A cycle was found in the visual scene at node: ") + TO_FSTRING(GetDaeId()), child->line);
					}
				}
				else
				{
					status.Warning(FS("Unable to retrieve node instance for scene node: ") + TO_FSTRING(GetDaeId()), child->line);
				}

				status.Warning(FS("Unable to retrieve weakly-typed instance for scene node: ") + TO_FSTRING(GetDaeId()), child->line);
			}
			else
			{
				status.Warning(FS("FCollada does not support COLLADA 1.3 external references anymore: ") + TO_FSTRING(GetDaeId()), child->line);
			}
		}
		else if (IsEquivalent(child->name, DAE_EXTRA_ELEMENT)) 
		{
		}
		else if (IsEquivalent(child->name, DAE_ASSET_ELEMENT))
		{
			if(asset)
			{
				status.Warning(FS("Found more than one asset present in scene node: ") + TO_FSTRING(GetDaeId()), child->line);
				continue;
			}

			// Read in the asset information
			asset = new FCDAsset(GetDocument());
			status.AppendStatus(asset->LoadFromXML(child));
		}
		else
		{
			FCDTransform* transform = FCDTFactory::CreateTransform(GetDocument(), this, child);
			if (transform != NULL)
			{
				transforms.push_back(transform);
				status.AppendStatus(transform->LoadFromXML(child));
			}
			else
			{
				status.Warning(FS("Unknown element or bad transform in scene node: ") + TO_FSTRING(GetDaeId()), child->line);
			}
		}
	}

	status.AppendStatus(LoadFromExtra());
	SetDirtyFlag();
	return status;
}

FUStatus FCDSceneNode::LoadFromExtra()
{
	FUStatus status;

	FCDENodeList parameterNodes;
	StringList parameterNames;

	// Retrieve the extra information from the base entity class
	FCDExtra* extra = GetExtra();

	// List all the parameters
	size_t techniqueCount = extra->GetTechniqueCount();
	for (size_t i = 0; i < techniqueCount; ++i)
	{
		FCDETechnique* technique = extra->GetTechnique(i);
		technique->FindParameters(parameterNodes, parameterNames);
	}

	// Process the known parameters
	size_t parameterCount = parameterNodes.size();
	for (size_t i = 0; i < parameterCount; ++i)
	{
		FCDENode* parameterNode = parameterNodes[i];
		const string& parameterName = parameterNames[i];
		FCDEAttribute* parameterType = parameterNode->FindAttribute(DAE_TYPE_ATTRIBUTE);
		if (parameterName == DAEMAYA_STARTTIME_PARAMETER || parameterName == DAEMAYA_STARTTIME_PARAMETER1_3)
		{
			GetDocument()->SetStartTime(FUStringConversion::ToFloat(parameterNode->GetContent()));
		}
		else if (parameterName == DAEMAYA_ENDTIME_PARAMETER || parameterName == DAEMAYA_ENDTIME_PARAMETER1_3)
		{
			GetDocument()->SetEndTime(FUStringConversion::ToFloat(parameterNode->GetContent()));
		}
		else if (parameterName == DAEFC_VISIBILITY_PARAMETER || parameterName == DAEMAYA_VISIBILITY_PARAMETER1_3)
		{
			visibility = FUStringConversion::ToBoolean(parameterNode->GetContent()) ? 1.0f : 0.0f;
			FCDAnimatedFloat::Clone(GetDocument(), &parameterNode->GetAnimated()->GetDummy(), &visibility);
		}
		else if (parameterName == DAEMAYA_LAYER_PARAMETER || (parameterType != NULL && FUStringConversion::ToString(parameterType->value) == DAEMAYA_LAYER_PARAMETER))
		{
			FCDEAttribute* nameAttribute = parameterNode->FindAttribute(DAE_NAME_ATTRIBUTE);
			if (nameAttribute == NULL) continue;

			// Create a new layer object list
			FCDLayerList& layers = GetDocument()->GetLayers();
			FCDLayer* layer = new FCDLayer(); layers.push_back(layer);

			// Parse in the layer
			layer->name = FUStringConversion::ToString(nameAttribute->value);
			FUStringConversion::ToStringList(parameterNode->GetContent(), layer->objects);
		}
		else continue;

		SAFE_RELEASE(parameterNode);
	}

	SetDirtyFlag();
	return status;
}

// Write out a <visual_scene> element to a COLLADA XML document
xmlNode* FCDSceneNode::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* visualSceneNode = WriteToEntityXML(parentNode, DAE_VSCENE_ELEMENT);
	if (visualSceneNode != NULL)
	{
		WriteToNodeXML(visualSceneNode, true);
	}

	// For the main visual scene: export the layer information
	if (GetDocument()->GetVisualSceneRoot() == this)
	{
		const FCDLayerList& layers = GetDocument()->GetLayers();
		if (!layers.empty())
		{
			xmlNode* techniqueNode = AddExtraTechniqueChild(visualSceneNode, DAEMAYA_MAYA_PROFILE);
			for (FCDLayerList::const_iterator itL = layers.begin(); itL != layers.end(); ++itL)
			{
				xmlNode* layerNode = AddChild(techniqueNode, DAEMAYA_LAYER_PARAMETER);
				if (!(*itL)->name.empty()) AddAttribute(layerNode, DAE_NAME_ATTRIBUTE, (*itL)->name);
				FUSStringBuilder layerObjects;
				for (StringList::const_iterator itO = (*itL)->objects.begin(); itO != (*itL)->objects.end(); ++itO)
				{
					layerObjects.append(*itO);
					layerObjects.append(' ');
				}
				layerObjects.pop_back();
				AddContent(layerNode, layerObjects);
			}
		}
	}

	return visualSceneNode;
}

// Write out a <visual_scene> or a <node> element to a COLLADA XML document
void FCDSceneNode::WriteToNodeXML(xmlNode* node, bool isVisualScene) const
{
	// Write out the extra attributes
	if (!isVisualScene)
	{
		if (isJoint)
		{
			AddAttribute(node, DAE_SID_ATTRIBUTE, GetDaeId());
			AddAttribute(node, DAE_TYPE_ATTRIBUTE, DAE_JOINT_NODE_TYPE);
		}
		else
		{
			AddAttribute(node, DAE_TYPE_ATTRIBUTE, DAE_NODE_NODE_TYPE);
		}
	}

	//Write out the asset information
	if(asset)
		asset->WriteToXML(node);

	// Write out the transforms
	for (FCDTransformContainer::const_iterator itT = transforms.begin(); itT != transforms.end(); ++itT)
	{
		FCDTransform* transform = (*itT);
		transform->WriteToXML(node);
	}

	// Write out the instantiation
	for (FCDEntityInstanceContainer::const_iterator itI = instances.begin(); itI != instances.end(); ++itI)
	{
		FCDEntityInstance* instance = (*itI);
		instance->WriteToXML(node);
	}

	// Write out the child scene graph nodes as <node> elements
	if (!isVisualScene || !children.empty())
	{
		// First, write out the child nodes that we consider instances: there is more than one
		// parent node and we aren't the first one.
		for (FCDSceneNodeTrackList::const_iterator itC = children.begin(); itC != children.end(); ++itC)
		{
			FCDSceneNode* child = (*itC);
			if (child->GetParent() != this)
			{
				bool alreadyInstantiated = false;
				for (FCDEntityInstanceContainer::const_iterator itI = instances.begin(); itI != instances.end(); ++itI)
				{
					if (!(*itI)->IsExternalReference())
					{
						alreadyInstantiated |= (*itI)->GetEntity() == child;
					}
				}
				if (!alreadyInstantiated)
				{
					FCDEntityInstance nodeInstance(const_cast<FCDocument*>(GetDocument()), NULL, FCDEntity::SCENE_NODE);
					nodeInstance.SetEntity(child);
					nodeInstance.WriteToXML(node);
				}
			}
		}

		// Then, hierarchically write out the child nodes.
		for (FCDSceneNodeTrackList::const_iterator itC = children.begin(); itC != children.end(); ++itC)
		{
			FCDSceneNode* child = (*itC);
			if (child->GetParent() == this)
			{
				xmlNode* nodeNode = child->WriteToEntityXML(node, DAE_NODE_ELEMENT);
				if (nodeNode != NULL) child->WriteToNodeXML(nodeNode, false);
			}
		}
	}

	if (!isVisualScene)
	{
		// Write out the visibility of this node, if it is not visible or if it is animated.
		if (GetDocument()->IsValueAnimated(&visibility) || !visibility)
		{
			xmlNode* techniqueNode = AddExtraTechniqueChild(node, DAE_FCOLLADA_PROFILE);
			xmlNode* visibilityNode = AddChild(techniqueNode, DAEFC_VISIBILITY_PARAMETER, visibility >= 0.5f);
			GetDocument()->WriteAnimatedValueToXML(&visibility, visibilityNode, "visibility");
		}
	}

	// Write out the extra information
	FCDEntity::WriteToExtraXML(node);
}

FCDEntity* FCDSceneNode::Clone(FCDEntity* _clone) const
{
	FCDSceneNode* clone = NULL;
	if (_clone == NULL) clone = new FCDSceneNode(const_cast<FCDocument*>(GetDocument()));
	else if (!_clone->HasType(FCDSceneNode::GetClassType())) return Parent::Clone(_clone);
	else clone = (FCDSceneNode*) _clone;
	Parent::Clone(clone);

	// Copy over the simple information.
	clone->isJoint = isJoint;
	clone->visibility = visibility;

	// Don't copy the parents list but do clone all the children, transforms and instances
	for (FCDTransformContainer::const_iterator it = transforms.begin(); it != transforms.end(); ++it)
	{
		FCDTransform* transform = clone->AddTransform((*it)->GetType());
		(*it)->Clone(transform);
	}

	for (FCDSceneNodeTrackList::const_iterator it = children.begin(); it != children.end(); ++it)
	{
		FCDSceneNode* child = clone->AddChildNode();
		(*it)->Clone(child);
	}

	for (FCDEntityInstanceContainer::const_iterator it = instances.begin(); it != instances.end(); ++it)
	{
		FCDEntityInstance* instance = clone->AddInstance((*it)->GetEntityType());
		(*it)->Clone(instance);
	}

	return clone;
}

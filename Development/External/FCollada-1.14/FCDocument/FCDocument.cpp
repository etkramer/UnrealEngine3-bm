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
#include "FCDocument/FCDAnimation.h"
#include "FCDocument/FCDAnimationChannel.h"
#include "FCDocument/FCDAnimationClip.h"
#include "FCDocument/FCDAnimationCurve.h"
#include "FCDocument/FCDAsset.h"
#include "FCDocument/FCDCamera.h"
#include "FCDocument/FCDController.h"
#include "FCDocument/FCDEffect.h"
#include "FCDocument/FCDExternalReferenceManager.h"
#include "FCDocument/FCDForceField.h"
#include "FCDocument/FCDGeometry.h"
#include "FCDocument/FCDGeometryMesh.h"
#include "FCDocument/FCDImage.h"
#include "FCDocument/FCDLight.h"
#include "FCDocument/FCDLibrary.h"
#include "FCDocument/FCDLibrary.hpp"
#include "FCDocument/FCDMaterial.h"
#include "FCDocument/FCDMaterialLibrary.h"
#include "FCDocument/FCDPhysicsMaterial.h"
#include "FCDocument/FCDPhysicsModel.h"
#include "FCDocument/FCDPhysicsScene.h"
#include "FCDocument/FCDSceneNode.h"
#include "FCDocument/FCDTexture.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
#include "FUtils/FUFileManager.h"
#include "FUtils/FUUniqueStringMap.h"
#include "FUtils/FUXmlDocument.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

//
// FCDocument
//
ImplementObjectType(FCDocument);

FCDocument::FCDocument()
{
	fileManager = new FUFileManager();
	asset = new FCDAsset(this);
	uniqueNameMap = new FUSUniqueStringMap();
	externalReferenceManager = new FCDExternalReferenceManager(this);

	animationLibrary = new FCDAnimationLibrary(this);
	animationClipLibrary = new FCDAnimationClipLibrary(this);
	cameraLibrary = new FCDCameraLibrary(this);
	controllerLibrary = new FCDControllerLibrary(this);
	forceFieldLibrary = new FCDForceFieldLibrary(this);
	geometryLibrary = new FCDGeometryLibrary(this);
	imageLibrary = new FCDImageLibrary(this);
	lightLibrary = new FCDLightLibrary(this);
	materialLibrary = new FCDMaterialLibrary(this);
	visualSceneLibrary = new FCDVisualSceneNodeLibrary(this);
	physicsMaterialLibrary = new FCDPhysicsMaterialLibrary(this);
	physicsModelLibrary = new FCDPhysicsModelLibrary(this);
	physicsSceneLibrary = new FCDPhysicsSceneLibrary(this);

	visualSceneRoot = NULL;

	// Document global parameters
	lengthUnitConversion = 1.0f;
	lengthUnitWanted = -1.0f;
	hasStartTime = hasEndTime = false;
	startTime = endTime = 0.0f;

	// Set the current COLLADA document version.
	version.ParseVersionNumbers(DAE_SCHEMA_VERSION);
}

FCDocument::~FCDocument()
{
	// Release the external references to and from this document
	// before all clearing the entities.
	FUObject::Detach();
	
	externalReferenceManager = NULL;

	// Release the libraries and the asset
	animationLibrary = NULL;
	animationClipLibrary = NULL;
	cameraLibrary = NULL;
	controllerLibrary = NULL;
	forceFieldLibrary = NULL;
	geometryLibrary = NULL;
	imageLibrary = NULL;
	lightLibrary = NULL;
	materialLibrary = NULL;
	visualSceneLibrary = NULL;
	physicsMaterialLibrary = NULL;
	physicsModelLibrary = NULL;
	physicsSceneLibrary = NULL;
	asset = NULL;

	// Must be released last
	CLEAR_POINTER_VECTOR(layers);
	while(!animatedValues.empty()) { delete (*animatedValues.rbegin()).first; }
	animatedValueMap.clear();

	SAFE_DELETE(fileManager);
	SAFE_DELETE(uniqueNameMap);
}

// Adds an entity layer to the document.
FCDLayer* FCDocument::AddLayer()
{
	FCDLayer* layer = new FCDLayer();
	layers.push_back(layer);
	return layer;
}

// Releases an entity layer from the document.
void FCDocument::ReleaseLayer(FCDLayer* layer)
{
	layers.release(layer);
}

// These two parameters now belong to FCDAsset. Keep them here for a few version, for backward compatibility
const FMVector3& FCDocument::GetUpAxis() const { return asset->GetUpAxis(); }
float FCDocument::GetLengthUnit() const { return asset->GetUnitConversionFactor(); }

// Search for a driven curve that needs this animated value as a driver
bool FCDocument::LinkDriver(FCDAnimated* animated)
{
	if (animated->GetTargetPointer().empty()) return false;

	bool driven = false;
	size_t animationCount = animationLibrary->GetEntityCount();
	for (size_t i = 0; i < animationCount; ++i)
	{
		FCDAnimation* animation = animationLibrary->GetEntity(i);
		driven |= animation->LinkDriver(animated);
	}
	return driven;
}

// Search for an animation channel targeting the given pointer
void FCDocument::FindAnimationChannels(const string& pointer, FCDAnimationChannelList& channels)
{
	if (pointer.empty()) return;

	size_t animationCount = (uint32) animationLibrary->GetEntityCount();
	for (size_t i = 0; i < animationCount; ++i)
	{
		FCDAnimation* animation = animationLibrary->GetEntity(i);
		animation->FindAnimationChannels(pointer, channels);
	}
}

// Gather a list of the indices of animated array element belonging to the node
void FCDocument::FindAnimationChannelsArrayIndices(xmlNode* targetArray, Int32List& animatedIndices)
{
	// Calculte the node's pointer
	string pointer;
	CalculateNodeTargetPointer(targetArray, pointer);
	if (pointer.empty()) return;

	// Retrieve the channels for this pointer and extract their matrix indices.
	FCDAnimationChannelList channels;
	FindAnimationChannels(pointer, channels);
	for (vector<FCDAnimationChannel*>::iterator it = channels.begin(); it != channels.end(); ++it)
	{
		string qualifier = (*it)->GetTargetQualifier();
		int32 animatedIndex = ReadTargetMatrixElement(qualifier);
		if (animatedIndex != -1) animatedIndices.push_back(animatedIndex);
	}
}

// Search for a specific COLLADA library items with a given COLLADA id.
FCDAnimation* FCDocument::FindAnimation(const string& daeId) { return animationLibrary->FindDaeId(daeId); }
FCDAnimationClip* FCDocument::FindAnimationClip(const string& daeId) { return animationClipLibrary->FindDaeId(daeId); }
FCDCamera* FCDocument::FindCamera(const string& daeId) { return cameraLibrary->FindDaeId(daeId); }
FCDController* FCDocument::FindController(const string& daeId) { return controllerLibrary->FindDaeId(daeId); }
FCDGeometry* FCDocument::FindGeometry(const string& daeId) { return geometryLibrary->FindDaeId(daeId); }
FCDImage* FCDocument::FindImage(const string& daeId) { return imageLibrary->FindDaeId(daeId); }
FCDLayer* FCDocument::FindLayer(const string& name) { for (FCDLayerList::iterator itL = layers.begin(); itL != layers.end(); ++itL) { if ((*itL)->name == name) return *itL; } return NULL; }
FCDLight* FCDocument::FindLight(const string& daeId) { return lightLibrary->FindDaeId(daeId); }
FCDMaterial* FCDocument::FindMaterial(const string& daeId) { return  materialLibrary->FindMaterial(daeId); }
FCDEffect* FCDocument::FindEffect(const string& daeId) { return  materialLibrary->FindEffect(daeId); }
FCDSceneNode* FCDocument::FindVisualScene(const string& daeId) { return visualSceneLibrary->FindDaeId(daeId); }
FCDPhysicsScene* FCDocument::FindPhysicsScene(const string& daeId) { return physicsSceneLibrary->FindDaeId(daeId); }
FCDPhysicsMaterial* FCDocument::FindPhysicsMaterial(const string& daeId) { return physicsMaterialLibrary->FindDaeId(daeId); }
FCDPhysicsModel* FCDocument::FindPhysicsModel(const string& daeId) { return physicsModelLibrary->FindDaeId(daeId); }
FCDForceField* FCDocument::FindForceField(const string& daeId) { return forceFieldLibrary->FindDaeId(daeId); }
FCDSceneNode* FCDocument::FindSceneNode(const string& daeId) { return visualSceneLibrary->FindDaeId(daeId); }
FCDEntity* FCDocument::FindEntity(const string& daeId)
{
#define CHECK_LIB(libraryName, libraryFunction) { \
	FCDEntity* e = libraryName->libraryFunction(daeId); \
	if (e != NULL) return e; }

	CHECK_LIB(animationLibrary, FindDaeId);
	CHECK_LIB(animationClipLibrary, FindDaeId);
	CHECK_LIB(cameraLibrary, FindDaeId);
	CHECK_LIB(controllerLibrary, FindDaeId);
	CHECK_LIB(geometryLibrary, FindDaeId);
	CHECK_LIB(imageLibrary, FindDaeId);
	CHECK_LIB(lightLibrary, FindDaeId);
	CHECK_LIB(materialLibrary, FindMaterial);
	CHECK_LIB(materialLibrary, FindEffect);
	CHECK_LIB(visualSceneLibrary, FindDaeId);
	CHECK_LIB(physicsSceneLibrary, FindDaeId);
	CHECK_LIB(physicsMaterialLibrary, FindDaeId);
	CHECK_LIB(physicsModelLibrary, FindDaeId);
	CHECK_LIB(forceFieldLibrary, FindDaeId);
#undef CHECK_LIB

	return NULL;
}

// Add an animated value to the list
void FCDocument::RegisterAnimatedValue(FCDAnimated* animated)
{
	// Look for a duplicate in order to avoid memory loss
	//if (animated->GetValueCount() == 0 || FindAnimatedValue(animated->GetValue(0)) != NULL)
	if (animated->GetValueCount() == 0)
	{
		SAFE_DELETE(animated);
		return;
	}

	// List the new animated value
	animatedValues.insert(FCDAnimatedSet::value_type(animated, animated));

	// Also add to the map the individual values for easy retrieval
	size_t count = animated->GetValueCount();
	for (size_t i = 0; i < count; ++i)
	{
		const float* value = animated->GetValue(i);
		animatedValueMap[value] = animated;
	}
}

// Unregisters an animated value of the document.
void FCDocument::UnregisterAnimatedValue(FCDAnimated* animated)
{
	if (animated != NULL)
	{
		// Intentionally search from the end:
		// - In the destructor of the document, we delete from the end.
		// - In animation exporters, we add to the end and are likely to delete right away.
		FCDAnimatedSet::iterator it = animatedValues.find(animated);
		if (it != animatedValues.end())
		{
			animatedValues.erase(it);

			// Also remove to the map the individual values contained
			size_t count = animated->GetValueCount();
			for (size_t i = 0; i < count; ++i)
			{
				const float* value = animated->GetValue(i);
				FCDAnimatedValueMap::iterator itV = animatedValueMap.find(value);
				if (itV != animatedValueMap.end() && (*itV).second == animated)
				{
					animatedValueMap.erase(itV);
				}
			}
		}
	}
}

// Retrieve an animated value, given a value pointer
FCDAnimated* FCDocument::FindAnimatedValue(float* ptr)
{
	FCDAnimatedValueMap::iterator it = animatedValueMap.find((const float*) ptr);
	return (it != animatedValueMap.end()) ? (*it).second : NULL;
}

// Retrieve an animated value, given a value pointer
const FCDAnimated* FCDocument::FindAnimatedValue(const float* ptr) const
{
	FCDAnimatedValueMap::const_iterator it = animatedValueMap.find(ptr);
	return (it != animatedValueMap.end()) ? (*it).second : NULL;
}

// Retrieve an animated float value for a given fully qualified target
const float* FCDocument::FindAnimatedTarget(const string& fullyQualifiedTarget)
{
	if (fullyQualifiedTarget.empty()) return NULL;
	string target = (fullyQualifiedTarget[0] == '#') ? fullyQualifiedTarget.substr(1) : fullyQualifiedTarget;
	string pointer, qualifier;
	SplitTarget(target, pointer, qualifier);

	// Find the pointer
	FCDAnimated* animatedValue = NULL;
	for (FCDAnimatedSet::iterator itA = animatedValues.begin(); itA != animatedValues.end(); ++itA)
	{
		FCDAnimated* animated = (*itA).first;
		if (animated->GetTargetPointer() == pointer) { animatedValue = animated; break; }
	}
	if (animatedValue == NULL) return NULL;

	// Return the qualified value
	size_t index = animatedValue->FindQualifier(qualifier);
	if (index == size_t(-1)) return NULL;
	return animatedValue->GetValue(index);
}

// Returns whether a given value pointer is animated
bool FCDocument::IsValueAnimated(const float* ptr) const
{
	const FCDAnimated* animated = FindAnimatedValue(ptr);
	return (animated != NULL) ? animated->HasCurve() : false;
}

// Insert new library elements
FCDSceneNode* FCDocument::AddVisualScene()
{
	return visualSceneRoot = visualSceneLibrary->AddEntity();
}
FCDPhysicsScene* FCDocument::AddPhysicsScene()
{
	return physicsSceneRoot = physicsSceneLibrary->AddEntity();
}

void FCDocument::UpdateAnimationClips(float newStart)
{
	for (size_t i = 0; i < animationClipLibrary->GetEntityCount(); i++)
	{
		animationClipLibrary->GetEntity(i)->UpdateAnimationCurves(newStart);
	}
}

void FCDocument::SetFileUrl(const fstring& filename)
{
	fileManager->PopRootFile();
	fileUrl = GetFileManager()->MakeFilePathAbsolute(filename);
	fileManager->PushRootFile(fileUrl);
}

// Structure and enumeration used to order the libraries
enum nodeOrder { ANIMATION=0, ANIMATION_CLIP, IMAGE, TEXTURE, EFFECT, MATERIAL, GEOMETRY, CONTROLLER, CAMERA, LIGHT, VISUAL_SCENE, FORCE_FIELD, PHYSICS_MATERIAL, PHYSICS_MODEL, PHYSICS_SCENE, UNKNOWN };
struct xmlOrderedNode { xmlNode* node; nodeOrder order; };
typedef vector<xmlOrderedNode> xmlOrderedNodeList;

// Loads an entire COLLADA document file
FUStatus FCDocument::LoadFromFile(const fstring& filename)
{
	FUStatus status;

	SetFileUrl(filename);

#if FCOLLADA_EXCEPTION
	try {
#endif

	// Parse the document into a XML tree
	FUXmlDocument daeDocument(filename.c_str(), true);
	xmlNode* rootNode = daeDocument.GetRootNode();
	if (rootNode != NULL)
	{
		// Read in the whole document from the root node
		status.AppendStatus(LoadDocumentFromXML(rootNode));
	}
	else
	{
		status.Fail(FS("Corrupted COLLADA document: malformed XML."));
	}

	// Clean-up the XML reader
	xmlCleanupParser();

#if FCOLLADA_EXCEPTION
	} catch(const char* sz) {
		status.Fail(FS("Exception caught while parsing a COLLADA document from file: ") + TO_FSTRING(sz));
#ifdef UNICODE
	} catch(const fchar* sz) {
		status.Fail(FS("Exception caught while parsing a COLLADA document from file: ") + sz);
#endif
	} catch(...)	{
		status.Fail(FS("Exception caught while parsing a COLLADA document from file."));
	}
#endif

	if (status.IsSuccessful()) status.AppendString(FC("COLLADA document loaded successfully."));
	return status;
}

// Loads an entire COLLADA document from a given NULL-terminated fstring
FUStatus FCDocument::LoadFromText(const fstring& basePath, const fchar* text)
{
	FUStatus status;

	// Push the given path unto the file manager's stack
	fileManager->PushRootPath(basePath);

#if FCOLLADA_EXCEPTION
	try {
#endif

#ifdef UNICODE
	// Downsize the text document into something 8-bit
	string xmlTextString = FUStringConversion::ToString(text);
	const xmlChar* xmlText = (const xmlChar*) xmlTextString.c_str();
#else
	const xmlChar* xmlText = (const xmlChar*) text;
#endif

	// Parse the document into a XML tree
	xmlDoc* daeDocument = xmlParseDoc(const_cast<xmlChar*>(xmlText));
	if (daeDocument != NULL)
	{
		xmlNode* rootNode = xmlDocGetRootElement(daeDocument);

		// Read in the whole document from the root node
		status.AppendStatus(LoadDocumentFromXML(rootNode));

		// Free the XML document
		xmlFreeDoc(daeDocument);
	}
	else
	{
		status.Fail(FS("Corrupted COLLADA document: malformed XML."));
	}

	// Clean-up the XML reader
	xmlCleanupParser();

#if FCOLLADA_EXCEPTION
	} catch(const char* sz) {
		status.Fail(FS("Exception caught while parsing a COLLADA document from a string: ") + TO_FSTRING(sz));
#ifdef UNICODE
	} catch(const fchar* sz) {
		status.Fail(FS("Exception caught while parsing a COLLADA document from a string: ") + sz);
#endif
	} catch(...)	{
		status.Fail(FC("Exception caught while parsing a COLLADA document from a string."));
	}
#endif

	// Restore the original OS current folder
	fileManager->PopRootPath();

	if (status.IsSuccessful()) status.AppendString(FC("COLLADA document loaded successfully."));
	return status;
}

FUStatus FCDocument::LoadDocumentFromXML(xmlNode* colladaNode)
{
	FUStatus status;

	// The only root node supported is "COLLADA"
	if (!IsEquivalent(colladaNode->name, DAE_COLLADA_ELEMENT))
	{
		return status.Fail(FS("Valid document contain only the <COLLADA> root element."), colladaNode->line);
	}

	string strVersion = ReadNodeProperty(colladaNode, DAE_VERSION_ATTRIBUTE);
	version.ParseVersionNumbers(strVersion);

	// Bucket the libraries, so that we can read them in our specific order
	// COLLADA 1.4: the libraries are now strongly-typed, so process all the elements
	xmlNode* sceneNode = NULL;
	xmlOrderedNodeList orderedLibraryNodes;
	for (xmlNode* child = colladaNode->children; child != NULL; child = child->next)
	{
		if (child->type != XML_ELEMENT_NODE) continue;

		xmlOrderedNode n;
		n.node = child;
		n.order = UNKNOWN;
		if (IsEquivalent(child->name, DAE_LIBRARY_ELEMENT))
		{
			// COLLADA 1.3: Read in the type attribute of the library to know its content
			string libraryType = ReadNodeProperty(n.node, DAE_TYPE_ATTRIBUTE);
			if (libraryType == DAE_ANIMATION_TYPE) n.order = ANIMATION;
			else if (libraryType == DAE_EFFECT_TYPE) n.order = EFFECT;
			else if (libraryType == DAE_IMAGE_TYPE) n.order = IMAGE;
			else if (libraryType == DAE_TEXTURE_TYPE) n.order = TEXTURE;
			else if (libraryType == DAE_MATERIAL_TYPE) n.order = MATERIAL;
			else if (libraryType == DAE_GEOMETRY_TYPE) n.order = GEOMETRY;
			else if (libraryType == DAE_CAMERA_TYPE) n.order = CAMERA;
			else if (libraryType == DAE_CONTROLLER_TYPE) n.order = CONTROLLER;
			else if (libraryType == DAE_LIGHT_TYPE) n.order = LIGHT;
		}
		else if (IsEquivalent(child->name, DAE_LIBRARY_ANIMATION_ELEMENT)) n.order = ANIMATION;
		else if (IsEquivalent(child->name, DAE_LIBRARY_ANIMATION_CLIP_ELEMENT)) n.order = ANIMATION_CLIP;
		else if (IsEquivalent(child->name, DAE_LIBRARY_CAMERA_ELEMENT)) n.order = CAMERA;
		else if (IsEquivalent(child->name, DAE_LIBRARY_CONTROLLER_ELEMENT)) n.order = CONTROLLER;
		else if (IsEquivalent(child->name, DAE_LIBRARY_EFFECT_ELEMENT)) n.order = EFFECT;
		else if (IsEquivalent(child->name, DAE_LIBRARY_GEOMETRY_ELEMENT)) n.order = GEOMETRY;
		else if (IsEquivalent(child->name, DAE_LIBRARY_IMAGE_ELEMENT)) n.order = IMAGE;
		else if (IsEquivalent(child->name, DAE_LIBRARY_LIGHT_ELEMENT)) n.order = LIGHT;
		else if (IsEquivalent(child->name, DAE_LIBRARY_MATERIAL_ELEMENT)) n.order = MATERIAL;
		else if (IsEquivalent(child->name, DAE_LIBRARY_VSCENE_ELEMENT)) n.order = VISUAL_SCENE;
		else if (IsEquivalent(child->name, DAE_LIBRARY_FFIELDS_ELEMENT)) n.order = FORCE_FIELD;
		else if (IsEquivalent(child->name, DAE_LIBRARY_NODE_ELEMENT)) n.order = VISUAL_SCENE; // Process them as visual scenes.
		else if (IsEquivalent(child->name, DAE_LIBRARY_PMATERIAL_ELEMENT)) n.order = PHYSICS_MATERIAL;
		else if (IsEquivalent(child->name, DAE_LIBRARY_PMODEL_ELEMENT)) n.order = PHYSICS_MODEL;
		else if (IsEquivalent(child->name, DAE_LIBRARY_PSCENE_ELEMENT)) n.order = PHYSICS_SCENE;
		else if (IsEquivalent(child->name, DAE_ASSET_ELEMENT))
		{
			// Read in the asset information
			status.AppendStatus(asset->LoadFromXML(child));

			// Calculate the length conversion unit
			// If the wanted unit length is negative, it implies that no conversion is wanted or that the flag was not set
			if (lengthUnitWanted > 0.0f) lengthUnitConversion = asset->GetUnitConversionFactor() / lengthUnitWanted;
			continue;
		}
		else if (IsEquivalent(child->name, DAE_SCENE_ELEMENT))
		{
			// The <scene> element should be the last element of the document
			sceneNode = child;
			continue;
		}
		else
		{
			status.Warning(FS("Unknown base node type: ") + TO_FSTRING((const char*) child->name), child->line);
			continue;
		}

		xmlOrderedNodeList::iterator it;
		for (it = orderedLibraryNodes.begin(); it != orderedLibraryNodes.end(); ++it)
		{
			if ((uint32) n.order < (uint32) (*it).order) break;
		}
		orderedLibraryNodes.insert(it, n);
	}

	// Process the ordered libraries
	size_t libraryNodeCount = orderedLibraryNodes.size();
	for (size_t i = 0; i < libraryNodeCount; ++i)
	{
		xmlOrderedNode& n = orderedLibraryNodes[i];
		switch (n.order)
		{
		case ANIMATION: status.AppendStatus(animationLibrary->LoadFromXML(n.node)); break;
		case ANIMATION_CLIP: status.AppendStatus(animationClipLibrary->LoadFromXML(n.node)); break;
		case CAMERA: status.AppendStatus(cameraLibrary->LoadFromXML(n.node)); break;
		case CONTROLLER: status.AppendStatus(controllerLibrary->LoadFromXML(n.node)); break;
		case FORCE_FIELD: status.AppendStatus(forceFieldLibrary->LoadFromXML(n.node)); break;
		case GEOMETRY: status.AppendStatus(geometryLibrary->LoadFromXML(n.node)); break;
		case EFFECT: status.AppendStatus(materialLibrary->LoadFromXML(n.node)); break;
		case IMAGE: status.AppendStatus(imageLibrary->LoadFromXML(n.node)); break;
		case LIGHT: status.AppendStatus(lightLibrary->LoadFromXML(n.node)); break;
		case MATERIAL: status.AppendStatus(materialLibrary->LoadFromXML(n.node)); break;
		case TEXTURE: status.AppendStatus(materialLibrary->LoadFromXML(n.node)); break;
		case PHYSICS_MODEL: status.AppendStatus(physicsModelLibrary->LoadFromXML(n.node)); break;
		case PHYSICS_MATERIAL: status.AppendStatus(physicsMaterialLibrary->LoadFromXML(n.node)); break;
		case PHYSICS_SCENE: status.AppendStatus(physicsSceneLibrary->LoadFromXML(n.node)); break;
		case VISUAL_SCENE: status.AppendStatus(visualSceneLibrary->LoadFromXML(n.node)); break;
		case UNKNOWN: default: break;
		}
	}

	// Read in the <scene> element
	if (sceneNode != NULL)
	{
		// COLLADA 1.4: Look for a <instance_physics_scene> element
		xmlNode* instancePhysicsNode = FindChildByType(sceneNode, DAE_INSTANCE_PHYSICS_SCENE_ELEMENT);
		if (instancePhysicsNode != NULL)
		{
			FUUri instanceUri = ReadNodeUrl(instancePhysicsNode);
			if (instanceUri.prefix.length() > 0)
			{
				status.Fail(FS("Cannot externally reference a <physics_scene> element."), sceneNode->line);
			}
			else if (instanceUri.suffix.length() == 0)
			{
				status.Fail(FS("No valid URI fragment for the instantiation of the physics scene."), sceneNode->line);
			}
			else
			{
				// Look for the correct physics scene to instantiate in the libraries
				physicsSceneRoot = FindPhysicsScene(instanceUri.suffix);
				if (physicsSceneRoot == NULL)
				{
					status.Fail(FS("Cannot find the correct <physics_scene> element to instantiate."), sceneNode->line);
				}
			}
		}

		// COLLADA 1.4: Look for a <instance_visual_scene> element
		xmlNode* instanceSceneNode = FindChildByType(sceneNode, DAE_INSTANCE_VSCENE_ELEMENT);
		if (instanceSceneNode != NULL)
		{
			FUUri instanceUri = ReadNodeUrl(instanceSceneNode);
			if (instanceUri.prefix.length() > 0)
			{
				status.Fail(FS("Cannot externally reference a <visual_scene> element."), sceneNode->line);
			}
			else if (instanceUri.suffix.length() == 0)
			{
				status.Fail(FS("No valid URI fragment for the instantiation of the visual scene."), sceneNode->line);
			}
			else
			{
				// Look for the correct visual scene to instantiate in the libraries
				visualSceneRoot = FindVisualScene(instanceUri.suffix);
				if (visualSceneRoot == NULL)
				{
					status.Fail(FS("Cannot find the correct <visual_scene> element to instantiate."), sceneNode->line);
				}
			}
		}
		else
		{
			// COLLADA 1.3 backward-compatibility, use this <scene> as the <visual_scene> element
			visualSceneRoot = visualSceneLibrary->AddEntity();
			status.AppendStatus(visualSceneRoot->LoadFromXML(sceneNode));
		}
	}

	// Link the effect surface parameters with the images
	for (size_t i = 0; i < materialLibrary->GetMaterialCount(); ++i)
	{
		materialLibrary->GetMaterial(i)->Link();
	}
	for (size_t i = 0; i < materialLibrary->GetEffectCount(); ++i)
	{
		materialLibrary->GetEffect(i)->Link();
	}

	// Link the convex meshes with their point clouds (convex_hull_of)
	for (size_t i = 0; i < geometryLibrary->GetEntityCount(); ++i)
	{
		FCDGeometryMesh* mesh = geometryLibrary->GetEntity(i)->GetMesh();
		if(mesh) mesh->Link();
	}

	// Link the controllers and the joints
	size_t controllerCount = controllerLibrary->GetEntityCount();
	for (size_t i = 0; i < controllerCount; ++i)
	{
		FCDController* controller = controllerLibrary->GetEntity(i);
		status.AppendStatus(controller->Link());
	}

	// Link the targeted entities, for 3dsMax cameras and lights
	size_t cameraCount = cameraLibrary->GetEntityCount();
	for (size_t i = 0; i < cameraCount; ++i)
	{
		FCDCamera* camera = cameraLibrary->GetEntity(i);
		status.AppendStatus(camera->LinkTarget());
	}
	size_t lightCount = lightLibrary->GetEntityCount();
	for (size_t i = 0; i < lightCount; ++i)
	{
		FCDLight* light = lightLibrary->GetEntity(i);
		status.AppendStatus(light->LinkTarget());
	}

	// Check that all the animation curves that need them, have found drivers
	size_t animationCount = animationLibrary->GetEntityCount();
	for (size_t i = 0; i < animationCount; ++i)
	{
		FCDAnimation* animation = animationLibrary->GetEntity(i);
		status.AppendStatus(animation->Link());
	}
	
	UpdateAnimationClips(startTime);

	if (!fileUrl.empty())
	{
		FCDExternalReferenceManager::RegisterLoadedDocument(this);
	}
	return status;
}

// Writes out the COLLADA document to a file
FUStatus FCDocument::WriteToFile(const fstring& filename) const
{
	FUStatus status;

	const_cast<FCDocument*>(this)->SetFileUrl(filename);

#if FCOLLADA_EXCEPTION
	try {
#endif
	// Create a new XML document
	FUXmlDocument daeDocument(filename.c_str(), false);
	xmlNode* rootNode = daeDocument.CreateRootNode(DAE_COLLADA_ELEMENT);
	status = WriteDocumentToXML(rootNode);
	if (status.IsSuccessful())
	{
		// Create the XML document and write it out to the given filename
		if (!daeDocument.Write())
		{
			status.Fail(FS("Unable to write COLLADA document to file '") + filename + FS("'. Verify that the folder exists and the file is writable."), rootNode->line);
		}
		else
		{
			status.AppendString(FC("COLLADA document written successfully."));
		}
	}

#if FCOLLADA_EXCEPTION
	} catch(const char* sz) {
		status.Fail(FS("Exception caught while parsing a COLLADA document from a string: ") + TO_FSTRING(sz));
#ifdef UNICODE
	} catch(const fchar* sz) {
		status.Fail(FS("Exception caught while parsing a COLLADA document from a string: ") + sz);
#endif
	} catch(...)	{
		status.Fail(FC("Exception caught while parsing a COLLADA document from a string."));
	}
#endif

	return status;
}

// Writes out the entire COLLADA document to the given XML root node.
FUStatus FCDocument::WriteDocumentToXML(xmlNode* colladaNode) const
{
	FUStatus status;
	if (colladaNode != NULL)
	{
		// Write the COLLADA document version and namespace: schema-required attributes
		AddAttribute(colladaNode, DAE_NAMESPACE_ATTRIBUTE, DAE_SCHEMA_LOCATION);
		AddAttribute(colladaNode, DAE_VERSION_ATTRIBUTE, DAE_SCHEMA_VERSION);

		// Write out the asset tag
		asset->WriteToXML(colladaNode);

		// Calculate the length conversion unit
		// If the wanted unit length is negative, it implies that no conversion is wanted or that the flag was not set
		if (lengthUnitWanted > 0.0f) const_cast<FCDocument*>(this)->lengthUnitConversion = lengthUnitWanted / asset->GetUnitConversionFactor();

		// Record the animation library. This library is built at the end, but should appear before the <scene> element.
		xmlNode* animationLibraryNode = NULL;
		if (!animationLibrary->IsEmpty())
		{
			animationLibraryNode = AddChild(colladaNode, DAE_LIBRARY_ANIMATION_ELEMENT);
		}

		// Export the libraries
#define EXPORT_LIBRARY(memberName, daeElementName) if (!(memberName)->IsEmpty()) { \
			xmlNode* libraryNode = AddChild(colladaNode, daeElementName); \
			memberName->WriteToXML(libraryNode); }

		EXPORT_LIBRARY(animationClipLibrary, DAE_LIBRARY_ANIMATION_CLIP_ELEMENT);
		EXPORT_LIBRARY(physicsMaterialLibrary, DAE_LIBRARY_PMATERIAL_ELEMENT);
		EXPORT_LIBRARY(forceFieldLibrary, DAE_LIBRARY_FFIELDS_ELEMENT);
		EXPORT_LIBRARY(physicsModelLibrary, DAE_LIBRARY_PMODEL_ELEMENT);
		EXPORT_LIBRARY(physicsSceneLibrary, DAE_LIBRARY_PSCENE_ELEMENT);
		EXPORT_LIBRARY(cameraLibrary, DAE_LIBRARY_CAMERA_ELEMENT);
		EXPORT_LIBRARY(lightLibrary, DAE_LIBRARY_LIGHT_ELEMENT);
		EXPORT_LIBRARY(imageLibrary, DAE_LIBRARY_IMAGE_ELEMENT);
		EXPORT_LIBRARY(materialLibrary, DAE_LIBRARY_MATERIAL_ELEMENT);
		EXPORT_LIBRARY(geometryLibrary, DAE_LIBRARY_GEOMETRY_ELEMENT);
		EXPORT_LIBRARY(controllerLibrary, DAE_LIBRARY_CONTROLLER_ELEMENT);
		EXPORT_LIBRARY(visualSceneLibrary, DAE_LIBRARY_VSCENE_ELEMENT);

#undef EXPORT_LIBRARY

		// Create the <scene> element and instantiate the selected visual scene.
		xmlNode* sceneNode = AddChild(colladaNode, DAE_SCENE_ELEMENT);
		if (physicsSceneRoot != NULL)
		{
			xmlNode* instancePhysicsSceneNode = AddChild(sceneNode, DAE_INSTANCE_PHYSICS_SCENE_ELEMENT);
			AddAttribute(instancePhysicsSceneNode, DAE_URL_ATTRIBUTE, string("#") + physicsSceneRoot->GetDaeId());
		}
		if (visualSceneRoot != NULL)
		{
			xmlNode* instanceVisualSceneNode = AddChild(sceneNode, DAE_INSTANCE_VSCENE_ELEMENT);
			AddAttribute(instanceVisualSceneNode, DAE_URL_ATTRIBUTE, string("#") + visualSceneRoot->GetDaeId());
		}

		// Write out the animations
		if (animationLibraryNode != NULL)
		{
			animationLibrary->WriteToXML(animationLibraryNode);
		}
	}
	return status;
}

// Writes out a value's animations, if any, to the animation library of a COLLADA XML document.
bool FCDocument::WriteAnimatedValueToXML(const float* value, xmlNode* valueNode, const char* wantedSid, int32 arrayElement) const
{
	// Find the value's animations
	FCDAnimated* animated = const_cast<FCDAnimated*>(FindAnimatedValue(value));
	if (animated == NULL || !animated->HasCurve() || valueNode == NULL) return false;

	animated->SetArrayElement(arrayElement);

	WriteAnimatedValueToXML(animated, valueNode, wantedSid);
	return true;
}

void FCDocument::WriteAnimatedValueToXML(const FCDAnimated* _animated, xmlNode* valueNode, const char* wantedSid) const
{
	FCDAnimated* animated = const_cast<FCDAnimated*>(_animated);
	int32 arrayElement = animated->GetArrayElement();

	// Set a sid unto the XML tree node, in order to support animations
	if (!HasNodeProperty(valueNode, DAE_SID_ATTRIBUTE) && !HasNodeProperty(valueNode, DAE_ID_ATTRIBUTE))
	{
		AddNodeSid(valueNode, wantedSid);
	}

	// Calculate the XML tree node's target for the animation channel
	string target;
	CalculateNodeTargetPointer(valueNode, target);
	animated->SetTargetPointer(target);
	if (!target.empty())
	{
		FCDAnimationChannelList channels;

		// Enforce the target pointer on all the curves
		for (size_t i = 0; i < animated->GetValueCount(); ++i)
		{
			FCDAnimationCurveList& curves = animated->GetCurves()[i];
			for (FCDAnimationCurveList::iterator itC = curves.begin(); itC != curves.end(); ++itC)
			{
				(*itC)->SetTargetElement(arrayElement);
				(*itC)->SetTargetQualifier(animated->GetQualifier(i));

				FCDAnimationChannel* channel = (*itC)->GetParent();
				FUAssert(channel != NULL, continue);
				channel->SetTargetPointer(target);
				if (!channels.contains(channel)) channels.push_back(channel);
			}
		}

		// Enforce the default values on the channels. This is used for curve merging.
		for (FCDAnimationChannelList::iterator itC = channels.begin(); itC != channels.end(); ++itC)
		{
			FCDAnimationChannelDefaultValueList channelDefaultValues;
			FCDAnimationChannel* channel = (*itC);

			for (size_t i = 0; i < animated->GetValueCount(); ++i)
			{
				FCDAnimationCurveList& curves = animated->GetCurves()[i];

				// Find the curve, if any, that comes from this channel.
				FCDAnimationCurve* curve = NULL;
				for (size_t j = 0; j < curves.size() && curve == NULL; ++j)
				{
					if (curves[j]->GetParent() == channel)
					{
						curve = curves[j];
						break;
					}
				}

				float defaultValue = *animated->GetValue(i);
				const char* qualifier = animated->GetQualifier(i).c_str();
				channelDefaultValues.push_back(FCDAnimationChannelDefaultValue(curve, defaultValue, qualifier));
			}

			(*itC)->SetDefaultValues(channelDefaultValues);
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////
////  FCDVersion
///////////////////////////////////////////////////////////////////////////////////

void FCDVersion::ParseVersionNumbers(string v)
{
	major = (uint8)FUStringConversion::ToUInt32(v);
	v.erase(0, min(v.find('.'), v.size()-1)+1);
	minor = (uint8)FUStringConversion::ToUInt32(v);
	v.erase(0, min(v.find('.'), v.size()-1)+1);
	revision = (uint8)FUStringConversion::ToUInt32(v);
}


bool FCDVersion::Is(string v) const
{
	FCDVersion version;
	version.ParseVersionNumbers(v);
	if(major == version.major && minor == version.minor && revision == version.revision)
		return true;
	
	return false;
}


bool FCDVersion::IsLowerThan(string v) const
{
	FCDVersion version;
	version.ParseVersionNumbers(v);
	if(major <= version.major)
	{
		if(minor <= version.minor)
		{
			if(revision < version.revision)
				return true;
			
			return true;
		}
		return true;
	}
	
	return false;
}


bool FCDVersion::IsHigherThan(string v) const
{
	if(!IsLowerThan(v) && !Is(v))
		return true;
	return false;
}

/*
	Copyright (C) 2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _FC_TEST_SCENE_
#define _FC_TEST_SCENE_

#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDLibrary.h"

class FCDAnimation;
class FCDEffectProfileFX;
class FCDEffectStandard;
class FCDExternalReference;
class FCDExtra;
class FCDGeometryInstance;
class FCDGeometryMesh;
class FCDGeometrySpline;
class FCDMorphController;
class FCDPhysicsMaterial;
class FCDPhysicsModel;
class FCDPhysicsRigidBody;
class FCDPhysicsRigidConstraint;
class FCDSceneNode;
class FCDSkinController;

namespace FCTestExportImport
{
	// Information pushing functions for the export
	bool FillCameraLibrary(FULogFile& fileOut, FCDCameraLibrary* library);
	bool FillLightLibrary(FULogFile& fileOut, FCDLightLibrary* library);
	bool FillExtraTree(FULogFile& fileOut, FCDExtra* extra);
	
	bool FillGeometryLibrary(FULogFile& fileOut, FCDGeometryLibrary* library);
	bool FillGeometryMesh(FULogFile& fileOut, FCDGeometryMesh* mesh);
	bool FillGeometrySpline(FULogFile& fileOut, FCDGeometrySpline* spline);
	bool FillControllerLibrary(FULogFile& fileOut, FCDControllerLibrary* library);
	bool FillControllerMorph(FULogFile& fileOut, FCDMorphController* controller);
	bool FillControllerSkin(FULogFile& fileOut, FCDSkinController* controller);

	bool FillImageLibrary(FULogFile& fileOut, FCDImageLibrary* library);
	bool FillMaterialLibrary(FULogFile& fileOut, FCDMaterialLibrary* library);
	bool FillEffectStandard(FULogFile& fileOut, FCDEffectStandard* profile);
	bool FillEffectFX(FULogFile& fileOut, FCDEffectProfileFX* profile);

	bool FillAnimationLibrary(FULogFile& fileOut, FCDAnimationLibrary* library);
	bool FillAnimationLight(FULogFile& fileOut, FCDocument* document, FCDAnimation* animationTree);

	bool FillVisualScene(FULogFile& fileOut, FCDSceneNode* scene);
	bool FillLayers(FULogFile& fileOut, FCDocument* doc);
	bool FillTransforms(FULogFile& fileOut, FCDSceneNode* node);
	bool FillGeometryInstance(FULogFile& fileOut, FCDGeometryInstance* instance);
	bool FillControllerInstance(FULogFile& fileOut, FCDGeometryInstance* instance);

	bool FillPhysics(FULogFile& fileOut, FCDocument* document);
	bool FillPhysicsMaterialLibrary(FULogFile& fileOut, FCDPhysicsMaterialLibrary* library);
	bool FillPhysicsModelLibrary(FULogFile& fileOut, FCDPhysicsModelLibrary* library);
	bool FillPhysicsModel(FULogFile& fileOut, FCDPhysicsModel* model);
	bool FillPhysicsRigidBody(FULogFile& fileOut, FCDPhysicsRigidBody* rigidBody);
	bool FillPhysicsRigidConstraint(FULogFile& fileOut, FCDPhysicsRigidConstraint* rigidConstraint);

	// Re-import verification functions
	bool CheckCameraLibrary(FULogFile& fileOut, FCDCameraLibrary* library);
	bool CheckLightLibrary(FULogFile& fileOut, FCDLightLibrary* library);
	bool CheckExtraTree(FULogFile& fileOut, FCDExtra* extra);

	bool CheckGeometryLibrary(FULogFile& fileOut, FCDGeometryLibrary* library);
	bool CheckGeometryMesh(FULogFile& fileOut, FCDGeometryMesh* mesh);
	bool CheckGeometrySpline(FULogFile& fileOut, FCDGeometrySpline* spline);
	bool CheckControllerLibrary(FULogFile& fileOut, FCDControllerLibrary* library);
	bool CheckControllerMorph(FULogFile& fileOut, FCDMorphController* controller);
	bool CheckControllerSkin(FULogFile& fileOut, FCDSkinController* controller);

	bool CheckImageLibrary(FULogFile& fileOut, FCDImageLibrary* library);
	bool CheckMaterialLibrary(FULogFile& fileOut, FCDMaterialLibrary* library);
	bool CheckEffectStandard(FULogFile& fileOut, FCDEffectStandard* profile);
	bool CheckEffectFX(FULogFile& fileOut, FCDEffectProfileFX* profile);

	bool CheckAnimationLibrary(FULogFile& fileOut, FCDAnimationLibrary* library);
	bool CheckAnimationLight(FULogFile& fileOut, FCDocument* document, FCDAnimation* animationTree);

	bool CheckVisualScene(FULogFile& fileOut, FCDSceneNode* imported);
	bool CheckLayers(FULogFile& fileOut, FCDocument* doc);
	bool CheckTransforms(FULogFile& fileOut, FCDSceneNode* node);
	bool CheckGeometryInstance(FULogFile& fileOut, FCDGeometryInstance* scene);
	bool CheckControllerInstance(FULogFile& fileOut, FCDGeometryInstance* scene);

	bool CheckPhysics(FULogFile& fileOut, FCDocument* doc);
	bool CheckPhysicsMaterialLibrary(FULogFile& fileOut, FCDPhysicsMaterialLibrary* library);
	bool CheckPhysicsModelLibrary(FULogFile& fileOut, FCDPhysicsModelLibrary* library);
	bool CheckPhysicsModel(FULogFile& fileOut, FCDPhysicsModel* model);
	bool CheckPhysicsRigidBody(FULogFile& fileOut, FCDPhysicsRigidBody* rigidBody);
	bool CheckPhysicsRigidConstraint(FULogFile& fileOut, FCDPhysicsRigidConstraint* rigidConstraint);
};

#endif // _FC_TEST_SCENE_
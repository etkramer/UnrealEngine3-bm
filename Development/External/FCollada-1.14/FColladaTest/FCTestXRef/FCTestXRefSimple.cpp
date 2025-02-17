/*
	Copyright (C) 2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDAsset.h"
#include "FCDocument/FCDLibrary.h"
#include "FCDocument/FCDLight.h"
#include "FCDocument/FCDEntityInstance.h"
#include "FCDocument/FCDExternalReference.h"
#include "FCDocument/FCDExternalReferenceManager.h"
#include "FCDocument/FCDSceneNode.h"

TESTSUITE_START(FCTestXRefSimple)

TESTSUITE_TEST(Export)
	FCDocument* doc1 = FCollada::NewTopDocument();
	FCDocument* doc2 = FCollada::NewTopDocument();
	FCDSceneNode* sceneNode = doc1->AddVisualScene()->AddChildNode();
	FCDLight* light = doc2->GetLightLibrary()->AddEntity();
	light->SetLightType(FCDLight::AMBIENT);
	light->SetColor(FMVector3::XAxis);
	sceneNode->AddInstance(light);
	doc2->WriteToFile(FS("XRefDoc2.dae"));
	doc1->WriteToFile(FS("XRefDoc1.dae"));
	SAFE_DELETE(doc1);
	SAFE_DELETE(doc2);

TESTSUITE_TEST(ReimportSceneNode)
	FCollada::SetDereferenceFlag(true);
	FCDocument* doc1 = FCollada::NewTopDocument();
	FUStatus s = doc1->LoadFromFile(FS("XRefDoc1.dae"));
	FailIf(s.IsFailure());
	FCDSceneNode* visualScene = doc1->GetVisualSceneRoot();
	FailIf(visualScene == NULL || visualScene->GetChildrenCount() == 0);
	visualScene = visualScene->GetChild(0);
	FailIf(visualScene == NULL || visualScene->GetInstanceCount() == 0);
	FCDEntityInstance* instance = visualScene->GetInstance(0);
	FailIf(instance == NULL);
	PassIf(instance->GetEntityType() == FCDEntity::LIGHT);
	FCDLight* light = (FCDLight*) instance->GetEntity();
	FailIf(light == NULL);
	PassIf(light->GetLightType() == FCDLight::AMBIENT);
	PassIf(IsEquivalent(light->GetColor(), FMVector3::XAxis));
	SAFE_DELETE(doc1);

TESTSUITE_TEST(ReimportBothEasyOrder)
	FCollada::SetDereferenceFlag(false);
	FCDocument* doc1 = FCollada::NewTopDocument();
	FCDocument* doc2 = FCollada::NewTopDocument();
	FUStatus s = doc2->LoadFromFile(FS("XRefDoc2.dae"));
	s.AppendStatus(doc1->LoadFromFile(FS("XRefDoc1.dae")));
	FailIf(s.IsFailure());

	FCDSceneNode* visualScene = doc1->GetVisualSceneRoot();
	FailIf(visualScene == NULL || visualScene->GetChildrenCount() == 0);
	visualScene = visualScene->GetChild(0);
	FailIf(visualScene == NULL || visualScene->GetInstanceCount() == 0);
	FCDEntityInstance* instance = visualScene->GetInstance(0);
	FailIf(instance == NULL);
	PassIf(instance->GetEntityType() == FCDEntity::LIGHT);
	FCDLight* light = (FCDLight*) instance->GetEntity();
	FailIf(light == NULL);
	PassIf(light->GetLightType() == FCDLight::AMBIENT);
	PassIf(IsEquivalent(light->GetColor(), FMVector3::XAxis));
	SAFE_DELETE(doc1);
	SAFE_DELETE(doc2);

TESTSUITE_TEST(ReimportBothHardOrder)
	// Import the scene node document only, first.
	// And verify that the instance is created, but NULL.
	FCollada::SetDereferenceFlag(false);
	FCDocument* doc1 = FCollada::NewTopDocument();
	FUStatus s = doc1->LoadFromFile(FS("XRefDoc1.dae"));
	PassIf(s.IsSuccessful());
	FCDSceneNode* visualScene = doc1->GetVisualSceneRoot();
	FailIf(visualScene == NULL || visualScene->GetChildrenCount() == 0);
	visualScene = visualScene->GetChild(0);
	FailIf(visualScene == NULL || visualScene->GetInstanceCount() == 0);
	FCDEntityInstance* instance = visualScene->GetInstance(0);
	FailIf(instance == NULL);
	PassIf(instance->GetEntityType() == FCDEntity::LIGHT);
	FCDLight* light = (FCDLight*) instance->GetEntity();
	PassIf(light == NULL);

	// Now, import the second document and verify that the instance is now valid.
	FCDocument* doc2 = FCollada::NewTopDocument();
	s = doc2->LoadFromFile(FS("XRefDoc2.dae"));
	doc2->GetAsset()->SetUpAxis(FMVector3::XAxis);
	PassIf(s.IsSuccessful());
	light = (FCDLight*) instance->GetEntity();
	FailIf(light == NULL);
	PassIf(light->GetLightType() == FCDLight::AMBIENT);
	PassIf(IsEquivalent(light->GetColor(), FMVector3::XAxis));
	SAFE_DELETE(doc1);

	// Verify that the second document is still valid.
	const FMVector3& upAxis = doc2->GetUpAxis();
	PassIf(IsEquivalent(upAxis, FMVector3::XAxis));
	SAFE_DELETE(doc2);

TESTSUITE_END

/*
	Copyright (C) 2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDAsset.h"
#include "FCDocument/FCDLibrary.h"
#include "FCDocument/FCDCamera.h"
#include "FCDocument/FCDEntityInstance.h"
#include "FCDocument/FCDExternalReference.h"
#include "FCDocument/FCDExternalReferenceManager.h"
#include "FCDocument/FCDSceneNode.h"

TESTSUITE_START(FCTestXRefTree)

TESTSUITE_TEST(Export)
	FCDocument* topDoc = FCollada::NewTopDocument();
	FCDocument* midDoc = FCollada::NewTopDocument();
	FCDocument* botDoc = FCollada::NewTopDocument();

	// Create a camera at the bottom document.
	// This camera is referenced by the visual scene of the middle document.
	// The visual scene node of the middle document is referenced by the top document.
	FCDCamera* camera = botDoc->GetCameraLibrary()->AddEntity();
	camera->SetOrthographic();
	camera->SetMagX(2.0f);
	FCDSceneNode* midNode = midDoc->AddVisualScene();
	midNode = midNode->AddChildNode();
	midNode->AddInstance(camera);
	FCDSceneNode* topNode = topDoc->AddVisualScene();
	topNode = topNode->AddChildNode();
	topNode->AddInstance(midNode);

	// Save the files, out of order, for fun.
	midDoc->SetFileUrl(FS("XRefDocMid.dae"));
	topDoc->SetFileUrl(FS("XRefDocTop.dae"));
	botDoc->WriteToFile(FS("XRefDocBot.dae"));
	SAFE_DELETE(botDoc);
	midDoc->WriteToFile(FS("XRefDocMid.dae"));
	SAFE_DELETE(midDoc);
	topDoc->WriteToFile(FS("XRefDocTop.dae"));
	SAFE_DELETE(topDoc);

TESTSUITE_TEST(ReimportTopOnly)
	FCollada::SetDereferenceFlag(true);
	FCDocument* topDoc = FCollada::NewTopDocument();
	topDoc->LoadFromFile(FS("XRefDocTop.dae"));

	FCDSceneNode* node = topDoc->GetVisualSceneRoot();
	FailIf(node == NULL || node->GetChildrenCount() == 0);
	node = node->GetChild(0);
	FailIf(node == NULL || node->GetInstanceCount() == 0);
	FCDEntityInstance* instance = node->GetInstance(0);
	FailIf(instance == NULL || !instance->IsExternalReference());
	PassIf(instance->GetEntityType() == FCDEntity::SCENE_NODE);
	node = (FCDSceneNode*) instance->GetEntity();
	FailIf(node == NULL);
	PassIf(node->GetInstanceCount() == 1);
	instance = node->GetInstance(0);
	PassIf(instance->IsExternalReference());
	PassIf(instance->GetEntityType() == FCDEntity::CAMERA);
	FCDCamera* camera = (FCDCamera*) instance->GetEntity();
	PassIf(camera != NULL);
	PassIf(camera->IsOrthographic());
	PassIf(IsEquivalent(camera->GetMagX(), 2.0f));

	SAFE_DELETE(topDoc);

TESTSUITE_TEST(ReimportTopAndBot)
	FCollada::SetDereferenceFlag(true);
	FCDocument* topDoc = FCollada::NewTopDocument();
	FUStatus status = topDoc->LoadFromFile(FS("XRefDocTop.dae"));	
	FCDocument* botDoc = FCollada::NewTopDocument();
	status.AppendStatus(botDoc->LoadFromFile(FS("XRefDocBot.dae")));
	FailIf(status.IsFailure());

	FCDSceneNode* node = topDoc->GetVisualSceneRoot();
	FailIf(node == NULL || node->GetChildrenCount() == 0);
	node = node->GetChild(0);
	FailIf(node == NULL || node->GetInstanceCount() == 0);
	FCDEntityInstance* instance = node->GetInstance(0);
	FailIf(instance == NULL || !instance->IsExternalReference());
	PassIf(instance->GetEntityType() == FCDEntity::SCENE_NODE);
	node = (FCDSceneNode*) instance->GetEntity();
	FailIf(node == NULL);
	PassIf(node->GetInstanceCount() == 1);
	instance = node->GetInstance(0);
	PassIf(instance->IsExternalReference());
	PassIf(instance->GetEntityType() == FCDEntity::CAMERA);
	FCDCamera* camera = (FCDCamera*) instance->GetEntity();
	PassIf(camera != NULL);
	PassIf(camera->GetDocument() == botDoc);
	PassIf(camera->IsOrthographic());
	PassIf(IsEquivalent(camera->GetMagX(), 2.0f));

	SAFE_DELETE(topDoc);
	SAFE_DELETE(botDoc);

TESTSUITE_END
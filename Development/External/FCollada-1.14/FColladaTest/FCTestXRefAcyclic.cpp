/*
	Copyright (C) 2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDAsset.h"
#include "FCDocument/FCDLibrary.h"
#include "FCDocument/FCDGeometry.h"
#include "FCDocument/FCDEntityInstance.h"
#include "FCDocument/FCDExternalReference.h"
#include "FCDocument/FCDExternalReferenceManager.h"
#include "FCDocument/FCDSceneNode.h"

TESTSUITE_START(FCTestXRefAcyclic)

TESTSUITE_TEST(Export)
	FCDocument* firstDoc = FCollada::NewTopDocument();
	FCDocument* secondDoc = FCollada::NewTopDocument();

	FCDGeometry* mesh1 = firstDoc->GetGeometryLibrary()->AddEntity();
	FCDGeometry* mesh2 = secondDoc->GetGeometryLibrary()->AddEntity();

	FCDSceneNode* node1 = firstDoc->AddVisualScene();
	node1 = node1->AddChildNode();
	FCDSceneNode* node2 = secondDoc->AddVisualScene();
	node2 = node2->AddChildNode();

	node2->AddInstance(mesh1);
	node1->AddInstance(mesh2);

	firstDoc->SetFileUrl(FS("XRefDoc1.dae"));
	secondDoc->WriteToFile(FS("XRefDoc2.dae"));
	firstDoc->WriteToFile(FS("XRefDoc1.dae"));

	SAFE_DELETE(firstDoc);
	SAFE_DELETE(secondDoc);

TESTSUITE_TEST(ImportOne)
	FCollada::SetDereferenceFlag(false);
	FCDocument* firstDoc = FCollada::NewTopDocument();
	FUStatus status = firstDoc->LoadFromFile(FS("XRefDoc1.dae"));
	FCDocument* secondDoc = FCollada::NewTopDocument();
	status.AppendStatus(secondDoc->LoadFromFile(FS("XRefDoc2.dae")));

	FCDSceneNode* node1 = firstDoc->GetVisualSceneRoot();
	FCDSceneNode* node2 = secondDoc->GetVisualSceneRoot();
	FailIf(node1 == NULL || node2 == NULL || node1->GetChildrenCount() == 0 || node2->GetChildrenCount() == 0);
	node1 = node1->GetChild(0);
	node2 = node2->GetChild(0);
	FailIf(node1 == NULL || node2 == NULL);
	PassIf(node1->GetInstanceCount() == 1 && node2->GetInstanceCount() == 1);
	FCDEntityInstance* instance1 = node1->GetInstance(0);
	FCDEntityInstance* instance2 = node2->GetInstance(0);
	PassIf(instance1 != NULL && instance2 != NULL);
	PassIf(instance1->GetEntityType() == FCDEntity::GEOMETRY && instance2->GetEntityType() == FCDEntity::GEOMETRY);
	FCDGeometry* mesh1 = (FCDGeometry*) instance1->GetEntity();
	FCDGeometry* mesh2 = (FCDGeometry*) instance2->GetEntity();
	PassIf(mesh1 != NULL && mesh2 != NULL);
	PassIf(mesh1->GetDocument() == secondDoc);
	PassIf(mesh2->GetDocument() == firstDoc);
	

TESTSUITE_END
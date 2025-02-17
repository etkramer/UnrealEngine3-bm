/*
	Copyright (C) 2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDExtra.h"
#include "FCDocument/FCDImage.h"
#include "FCDocument/FCDLibrary.h"

TESTSUITE_START(FCTAMCrossCloning)

TESTSUITE_TEST(ImageCloning)
	// Create a first document with an image in the image library.
	FCDocument* doc1 = FCollada::NewTopDocument();
	FCDImage* image1 = doc1->GetImageLibrary()->AddEntity();
	image1->SetFilename(FS("TestImageGlou.bmp"));
	image1->SetDaeId("ImageGlou");
	FCDETechnique* extra1 = image1->GetExtra()->AddTechnique("TEST_PROFILE");
	extra1->AddParameter("GluedOn", 12);

	// Create a second document and clone the first document's image into it.
	FCDocument* doc2 = FCollada::NewTopDocument();
	FCDImage* image2 = doc2->GetImageLibrary()->AddEntity();
	image1->Clone(image2);
//	PassIf(IsEquivalent(image2->GetFilename(), FC("TestImageGlou.bmp")));
	PassIf(IsEquivalent(image1->GetDaeId(), image2->GetDaeId()));
	FCDETechnique* extra2 = image2->GetExtra()->FindTechnique("TEST_PROFILE");
	FailIf(extra2 == NULL);
	FCDENode* parameter = extra2->FindParameter("GluedOn");
	PassIf(parameter != NULL);
	PassIf(IsEquivalent(parameter->GetContent(), FC("12")));

TESTSUITE_END


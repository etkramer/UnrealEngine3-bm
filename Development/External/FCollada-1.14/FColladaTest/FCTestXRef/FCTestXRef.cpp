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

TESTSUITE_START(FCTestXRef)

TESTSUITE_TEST(SubTests)
	RUN_TESTSUITE(FCTestXRefSimple);
	RUN_TESTSUITE(FCTestXRefTree);
	RUN_TESTSUITE(FCTestXRefAcyclic);

TESTSUITE_END

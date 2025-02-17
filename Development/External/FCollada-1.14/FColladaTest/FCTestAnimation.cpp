/*
	Copyright (C) 2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDSceneNode.h"
#include "FCDocument/FCDAnimation.h"
#include "FCDocument/FCDAnimationCurve.h"
#include "FCDocument/FCDAnimationMultiCurve.h"

TESTSUITE_START(FCDAnimation)

TESTSUITE_TEST(Sampling)
	// Test import of the Eagle sample
	// Retrieves the "Bone09" joint and does the import sampling to verify its correctness.
	FCDocument document;
	FUStatus status = document.LoadFromFile(FC("Eagle.DAE"));
	PassIf(status.IsSuccessful());
	FCDSceneNode* node = document.FindSceneNode("Bone09");
	FailIf(node == NULL);

	FloatList keys; FMMatrix44List values;
	node->GenerateSampledMatrixAnimation(keys, values);
	FailIf(keys.size() > 30);
	PassIf(keys.size() == values.size());

TESTSUITE_TEST(CurveMerging)
	// Test the merge of single curves into multiple curves.

	// Create a first curve.
	static const size_t curve1KeyCount = 3;
	static const float curve1Keys[curve1KeyCount] = { 0.0f, 2.0f, 3.0f };
	static const float curve1Values[curve1KeyCount] = { 0.0f, -2.0f, 0.0f };
	static const FMVector2 curve1Intan[curve1KeyCount] = { FMVector2(-0.3f, 0.0f), FMVector2(1.2f, 1.0f), FMVector2(2.8f, -1.0f) };
	static const FMVector2 curve1Outtan[curve1KeyCount] = { FMVector2(0.5f, -4.0f), FMVector2(2.5f, 3.0f), FMVector2(3.0f, 0.0f) };
	FCDAnimationCurve* c1 = new FCDAnimationCurve(NULL, NULL);
	c1->GetKeys() = FloatList(curve1Keys, curve1KeyCount);
	c1->GetKeyValues() = FloatList(curve1Values, curve1KeyCount);
	c1->GetInTangents() = FMVector2List(curve1Intan, curve1KeyCount);
	c1->GetOutTangents() = FMVector2List(curve1Outtan, curve1KeyCount);
	c1->GetInterpolations() = UInt32List(curve1KeyCount, (uint32) FUDaeInterpolation::BEZIER);

	// Create a second curve.
	static const size_t curve2KeyCount = 3;
	static const float curve2Keys[curve2KeyCount] = { 0.0f, 1.0f, 3.0f };
	static const float curve2Values[curve2KeyCount] = { -10.0f, -12.0f, -10.0f };
	static const FMVector2 curve2Intan[curve2KeyCount] = { FMVector2(0.0f, 0.0f), FMVector2(0.8f, -2.0f), FMVector2(2.7f, -2.0f) };
	static const FMVector2 curve2Outtan[curve2KeyCount] = { FMVector2(0.2f, 0.0f), FMVector2(1.8f, -1.0f), FMVector2(3.5f, 2.0f) };
	FCDAnimationCurve* c2 = new FCDAnimationCurve(NULL, NULL);
	c2->GetKeys() = FloatList(curve2Keys, curve2KeyCount);
	c2->GetKeyValues() = FloatList(curve2Values, curve2KeyCount);
	c2->GetInTangents() = FMVector2List(curve2Intan, curve2KeyCount);
	c2->GetOutTangents() = FMVector2List(curve2Outtan, curve2KeyCount);
	c2->GetInterpolations() = UInt32List(curve2KeyCount, (uint32) FUDaeInterpolation::BEZIER);

	// Merge the curves
	FCDAnimationCurveList curves;
	curves.push_back(c1);
	curves.push_back(c2);
	FloatList defaultValues(2, 0.0f);
	FCDAnimationMultiCurve* multiCurve = FCDAnimationMultiCurve::MergeCurves(curves, defaultValues);
	FailIf(multiCurve == NULL);

	// Verify the created multi-curve
	static const size_t multiCurveKeyCount = 4;
	static const float multiCurveKeys[multiCurveKeyCount] = { 0.0f, 1.0f, 2.0f, 3.0f };
	static const float multiCurveValues[2][multiCurveKeyCount] = { { 0.0f, -1.9375f, -2.0f, 0.0f }, { -10.0f, -12.0f, -4.7291f, -10.0f } };
	static const FMVector2 multiCurveIntans[2][multiCurveKeyCount] = { { FMVector2(-0.3f, 0.0f), FMVector2(0.6666f, -2.4579f), FMVector2(1.6f, -0.5f), FMVector2(2.8f, -1.0f) }, { FMVector2(0.0f, 0.0f), FMVector2(0.8f, -2.0f), FMVector2(1.6666f, -4.52985f), FMVector2(2.85f, -6.0f) } };
	static const FMVector2 multiCurveOuttans[2][multiCurveKeyCount] = { { FMVector2(0.25f, -2.0f), FMVector2(1.3333f, -1.4171f), FMVector2(2.5f, 3.0f), FMVector2(3.0f, 0.0f) }, { FMVector2(0.2f, 0.0f), FMVector2(1.4f, -6.5f), FMVector2(2.3333f, -4.9285f), FMVector2(3.5f, 2.0f) } };
	PassIf(multiCurve->GetDimension() == 2);
	PassIf(multiCurve->GetKeys().size() == multiCurveKeyCount);
	PassIf(multiCurve->GetInterpolations().size() == multiCurveKeyCount);
	PassIf(IsEquivalent(multiCurve->GetKeys(), multiCurveKeys, multiCurveKeyCount));
	for (size_t i = 0; i < multiCurveKeyCount; ++i)
	{
		PassIf(multiCurve->GetInterpolations()[i] == FUDaeInterpolation::BEZIER);
	}
	for (uint32 i = 0; i < multiCurve->GetDimension(); ++i)
	{
		PassIf(multiCurve->GetKeyValues()[i].size() == multiCurveKeyCount);
		PassIf(multiCurve->GetInTangents()[i].size() == multiCurveKeyCount);
		PassIf(multiCurve->GetOutTangents()[i].size() == multiCurveKeyCount);

		PassIf(IsEquivalent(multiCurve->GetKeyValues()[i], multiCurveValues[i], multiCurveKeyCount));
		PassIf(IsEquivalent(multiCurve->GetInTangents()[i], multiCurveIntans[i], multiCurveKeyCount));
		PassIf(IsEquivalent(multiCurve->GetOutTangents()[i], multiCurveOuttans[i], multiCurveKeyCount));
	}

	SAFE_DELETE(c1);
	SAFE_DELETE(c2);
	SAFE_DELETE(multiCurve);

TESTSUITE_END

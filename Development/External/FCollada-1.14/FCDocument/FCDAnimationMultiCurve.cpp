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
#include "FCDocument/FCDAnimationCurve.h"
#include "FCDocument/FCDAnimationMultiCurve.h"
#include "FUtils/FUDaeEnum.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeWriter;

#define SMALL_DELTA	0.001f

ImplementObjectType(FCDAnimationMultiCurve);

FCDAnimationMultiCurve::FCDAnimationMultiCurve(FCDocument* document, uint32 _dimension) : FCDObject(document)
{
	dimension = _dimension;
	if (dimension == 0) dimension = 1;
	
	// Prepare the target information
	targetElement = -1;
	targetQualifiers = new string[dimension];

	// Allocate the key values and tangents to the wanted dimension
	keyValues = new FloatList[dimension];
	inTangents = new FMVector2List[dimension];
	outTangents = new FMVector2List[dimension];
}

FCDAnimationMultiCurve::~FCDAnimationMultiCurve()
{
	SAFE_DELETE_ARRAY(targetQualifiers);
	SAFE_DELETE_ARRAY(keyValues);
	SAFE_DELETE_ARRAY(inTangents);
	SAFE_DELETE_ARRAY(outTangents);
}

// Samples all the curves for a given input
void FCDAnimationMultiCurve::Evaluate(float input, float* output) const
{
	// Single key curves imply a constant value
	if (keys.size() == 1)
	{
		for (uint32 i = 0; i < dimension; ++i) output[i] = keyValues[i].front();
		return;
	}

	// Find the current interval
	uint32 index = 0;
	FloatList::const_iterator it;
	for (it = keys.begin(); it != keys.end(); ++it, ++index)
	{
		if ((*it) > input) break;
	}

	if (it == keys.end())
	{
		// We're sampling after the curve, return the last values
		for (uint32 i = 0; i < dimension; ++i) output[i] = keyValues[i].back();
		return;
	}
	else if (it == keys.begin())
	{
		// We're sampling before the curve, return the first values
		for (uint32 i = 0; i < dimension; ++i) output[i] = keyValues[i].front();
		return;
	}

	// Get the keys and values for this interval
	float endKey = *it;
	float startKey = *(it - 1);

	// Interpolate the outputs.
	// Similar code is found in FCDAnimationCurve.cpp. If you update this, update the other one too.
	uint32 interpolation = interpolations.empty() ? ((uint32) FUDaeInterpolation::DEFAULT) : interpolations[index];
	switch (FUDaeInterpolation::Interpolation(interpolation))
	{
	case FUDaeInterpolation::LINEAR: {
		for (uint32 i = 0; i < dimension; ++i)
		{
			float startValue = keyValues[i][index - 1];
			float endValue = keyValues[i][index];
			output[i] = (input - startKey) / (endKey - startKey) * (endValue - startValue) + startValue; 
		}
		break; }

	case FUDaeInterpolation::BEZIER: {
		for (uint32 i = 0; i < dimension; ++i)
		{
			float startValue = keyValues[i][index - 1];
			float endValue = keyValues[i][index];

			float t = (input - startKey) / (endKey - startKey);
			float b = outTangents[i][index - 1].v;
			float c = inTangents[i][index].v;
			float ti = 1.0f - t;
			float br = (endKey - startKey) / (outTangents[i][index - 1].u - startKey);
			float cr = (endKey - startKey) / (endKey - inTangents[i][index].u);
			br = FMath::Clamp(br, 0.01f, 100.0f);
			cr = FMath::Clamp(cr, 0.01f, 100.0f);

			output[i] = startValue * ti * ti * ti + br* b * ti * ti * t + cr * c * ti * t * t + endValue * t * t * t;
		}
		break; }

	case FUDaeInterpolation::UNKNOWN:
	case FUDaeInterpolation::STEP:
	default: {
		for (uint32 i = 0; i < dimension; ++i)
		{
			output[i] = keyValues[i][index - 1];
		}
		break; }
	}
}

FCDAnimationMultiCurve* FCDAnimationMultiCurve::MergeCurves(const vector<FCDAnimationCurve*>& _toMerge, const FloatList& defaultValues, const StringList& defaultQualifiers)
{
	vector<const FCDAnimationCurve*> toMerge;
	toMerge.reserve(_toMerge.size());
	for (vector<FCDAnimationCurve*>::const_iterator itC = _toMerge.begin(); itC != _toMerge.end(); ++itC)
	{
		toMerge.push_back(*itC);
	}
	return MergeCurves(toMerge, defaultValues, defaultQualifiers);
}

// Non-standard constructor used to merge together animation curves
FCDAnimationMultiCurve* FCDAnimationMultiCurve::MergeCurves(const vector<const FCDAnimationCurve*>& toMerge, const FloatList& defaultValues, const StringList& defaultQualifiers)
{
	size_t dimension = toMerge.size();
	if (dimension == 0) return NULL;

	// Look for the document pointer
	FCDocument* document = NULL;
	int32 targetElement = -1;
	bool validCurveFound = false;
	bool noTangents = true, noInterpolations = true;
	for (size_t i = 0; i < dimension; ++i)
	{
		if (toMerge[i] != NULL)
		{
			document = const_cast<FCDocument*>(toMerge[i]->GetDocument());
			targetElement = toMerge[i]->GetTargetElement();
			validCurveFound = true;
			noTangents &= toMerge[i]->GetInTangents().empty() && toMerge[i]->GetOutTangents().empty();
			noInterpolations &= toMerge[i]->GetInterpolations().empty();
			break;
		}
	}
	if (!validCurveFound) return NULL;

	// Allocate the output multiCurve.
	FCDAnimationMultiCurve* multiCurve = new FCDAnimationMultiCurve(document, (uint32) dimension);
	multiCurve->targetElement = targetElement;

	// Grab all the animation curve data element right away, to spare me some typing.
	FloatList& keys = multiCurve->GetKeys();
	FloatList* values = multiCurve->GetKeyValues();
	FMVector2List* inTangents = multiCurve->GetInTangents();
	FMVector2List* outTangents = multiCurve->GetOutTangents();
	UInt32List& interpolations = multiCurve->GetInterpolations();

	// Calculate the merged input keys
	for (size_t i = 0; i < dimension; ++i)
	{
		const FCDAnimationCurve* curve = toMerge[i];
		if (curve == NULL) continue;

		const FloatList& curveKeys = curve->GetKeys();

		// Merge each curve's keys, which should already be sorted, into the multi-curve's
		size_t multiCurveKeyCount = keys.size(), m = 0;
		size_t curveKeyCount = curveKeys.size(), c = 0;
		while (m < multiCurveKeyCount && c < curveKeyCount)
		{
			if (IsEquivalent(keys[m], curveKeys[c])) { ++c; ++m; }
			else if (keys[m] < curveKeys[c]) { ++m; }
			else { keys.insert(keys.begin() + (m++), curveKeys[c++]); ++multiCurveKeyCount; }
		}
		if (c < curveKeyCount) keys.insert(keys.end(), curveKeys.begin() + c, curveKeys.end());
	}
	size_t keyCount = keys.size();

	// Start with the unknown interpolation everywhere
	interpolations.resize(keyCount, (uint32) FUDaeInterpolation::UNKNOWN);

	// Merge the curves one by one into the multi-curve
	for (size_t i = 0; i < dimension; ++i)
	{
		// Pre-allocate the value and tangent data arrays
		values[i].resize(keyCount);
		inTangents[i].resize(keyCount);
		outTangents[i].resize(keyCount);

		const FCDAnimationCurve* curve = toMerge[i];
		if (curve == NULL || curve->GetKeys().empty() )
		{
			// No curve, or an empty curve, set the default value on all the keys
			float defaultValue = (i < defaultValues.size()) ? defaultValues[i] : 0.0f;
			for (size_t k = 0; k < keyCount; ++k)
			{
				float previousSpan = (k > 0 ? keys[k] - keys[k - 1] : 1.0f) / 3.0f;
				float nextSpan = (k < keyCount - 1 ? keys[k + 1] - keys[k] : 1.0f) / 3.0f;

				values[i][k] = defaultValue;
				inTangents[i][k] = FMVector2(keys[k] - previousSpan, values[i][k]);
				outTangents[i][k] = FMVector2(keys[k] + nextSpan, values[i][k]);
			}
			
			// Assign the default qualifier to this dimension
			if (i < defaultQualifiers.size())
			{
				multiCurve->targetQualifiers[i] = defaultQualifiers[i];
			}
			continue;
		}

		multiCurve->targetQualifiers[i] = curve->GetTargetQualifier();

		// Does the curve have per-key interpolations?
		bool hasDefaultInterpolation = curve->GetInterpolations().empty();
		bool hasTangents = !curve->GetInTangents().empty() && !curve->GetOutTangents().empty();

		// Merge in this curve's values, sampling when the multi-curve's key is not present in the curve.
		// Calculate/Retrieve the tangents as angles.
		const FloatList& curveKeys = curve->GetKeys();
		size_t curveKeyCount = curveKeys.size();
		for (size_t k = 0, c = 0; k < keyCount; ++k)
		{
			float previousSpan = (k > 0 ? keys[k] - keys[k - 1] : 1.0f) / 3.0f;
			float nextSpan = (k < keyCount - 1 ? keys[k + 1] - keys[k] : 1.0f) / 3.0f;

			uint32 interpolation = FUDaeInterpolation::UNKNOWN;
			if (c >= curveKeyCount || !IsEquivalent(keys[k], curveKeys[c]))
			{
				// Sample the curve
				float value = values[i][k] = curve->Evaluate(keys[k]);

				if (!noTangents)
				{
					// Calculate the slope at the sampled point.
					// Since the curve should be smooth: the in/out tangents should be equal.
					float slope = (value - curve->Evaluate(keys[k] - SMALL_DELTA)) / SMALL_DELTA;
					inTangents[i][k] = FMVector2(keys[k] - previousSpan, value - slope * previousSpan);
					outTangents[i][k] = FMVector2(keys[k] + nextSpan, value + slope * nextSpan);
				}
				if (!noInterpolations)
				{
					interpolation = FUDaeInterpolation::BEZIER;
				}
			}
			else
			{
				// Keys match, grab the value directly
				values[i][k] = curve->GetKeyValues()[c];

				if (!noTangents)
				{
					float oldPreviousSpan = (c > 0 ? curveKeys[c] - curveKeys[c - 1] : 1.0f) / 3.0f;
					float oldNextSpan = (c < curveKeyCount - 1 ? curveKeys[c + 1] - curveKeys[c] : 1.0f) / 3.0f;

					// Calculate the new tangent: keep the slope proportional
					if (hasTangents)
					{
						FMVector2 absolute(keys[k], values[i][k]);
						inTangents[i][k] = absolute + (curve->GetInTangents()[c] - absolute) / oldPreviousSpan * previousSpan;
						outTangents[i][k] = absolute + (curve->GetOutTangents()[c] - absolute) / oldNextSpan * nextSpan;
					}
					else
					{
						// Default to flat tangents.
						inTangents[i][k] = FMVector2(keys[k] - previousSpan, values[i][k]);
						outTangents[i][k] = FMVector2(keys[k] + nextSpan, values[i][k]);
					}
				}
				if (!noInterpolations)
				{
					interpolation = hasDefaultInterpolation ? ((uint32) FUDaeInterpolation::DEFAULT) : curve->GetInterpolations()[c];
				}
				++c;
			}

			// Merge the interpolation values, where bezier wins whenever interpolation values differ
			if (!noInterpolations)
			{
				uint32& oldInterpolation = interpolations[k];
				if (oldInterpolation == FUDaeInterpolation::UNKNOWN) oldInterpolation = interpolation;
				else if (oldInterpolation != interpolation) oldInterpolation = FUDaeInterpolation::BEZIER;
			}
		}
	}

	// Reset any unknown interpolation left
	std::replace(interpolations.begin(), interpolations.end(), (uint32) FUDaeInterpolation::UNKNOWN, (uint32) FUDaeInterpolation::DEFAULT);

	return multiCurve;
}

// Collapse this multi-dimensional curve into a one-dimensional curve, given a collapsing function
FCDAnimationCurve* FCDAnimationMultiCurve::Collapse(FCDCollapsingFunction collapse) const
{
	size_t keyCount = keys.size();
	if (keyCount == 0 || dimension == 0) return NULL;
	if (collapse == NULL) collapse = Average;
	bool hasTangents = (!inTangents[0].empty() && !outTangents[0].empty());

	// Create the output one-dimensional curve and retrieve its data list
	FCDAnimationCurve* out = new FCDAnimationCurve(const_cast<FCDocument*>(GetDocument()), NULL);
	out->SetTargetElement(targetElement);
	FloatList& outKeys = out->GetKeys();
	FloatList& outKeyValues = out->GetKeyValues();
	FMVector2List& outInTangents = out->GetInTangents();
	FMVector2List& outOutTangents = out->GetOutTangents();
	UInt32List& outInterpolations = out->GetInterpolations();

	// Pre-allocate the output arrays
	outKeys.resize(keyCount);
	outKeyValues.resize(keyCount);
	outInTangents.resize(keyCount);
	outOutTangents.resize(keyCount);
	outInterpolations.resize(keyCount);

	// Copy the key data over, collapsing the values
	float* buffer = new float[dimension];
	for (size_t i = 0; i < keyCount; ++i)
	{
		outKeys[i] = keys[i];
		outInterpolations[i] = interpolations[i];

		// Collapse the values and the tangents
		for (uint32 j = 0; j < dimension; ++j) buffer[j] = keyValues[j][i];
		outKeyValues[i] = (*collapse)(buffer, dimension);

		if (hasTangents)
		{
			for (uint32 j = 0; j < dimension; ++j) buffer[j] = inTangents[j][i].v;
			outInTangents[i] = FMVector2(inTangents[0][i].u, (*collapse)(buffer, dimension));
			for (uint32 j = 0; j < dimension; ++j) buffer[j] = outTangents[j][i].v;
			outOutTangents[i] = FMVector2(outTangents[0][i].u, (*collapse)(buffer, dimension));
		}
	}
	SAFE_DELETE_ARRAY(buffer);

	return out;
}

// Write out the specific animation elements to the COLLADA XML tree node
void FCDAnimationMultiCurve::WriteSourceToXML(xmlNode* parentNode, const string& baseId)
{
	if (keys.empty() || dimension == 0 || keyValues[0].empty()) return;

	// Generate the list of the parameters
	typedef const char* pchar;
	pchar* parameters = new pchar[dimension];
	for (size_t i = 0; i < dimension; ++i)
	{
		parameters[i] = targetQualifiers[i].c_str();
		if (*(parameters[i]) == '.') ++(parameters[i]);
	}

	// Export the key times
	AddSourceFloat(parentNode, baseId + "-input", keys, "TIME");

	// Interlace the key values and tangents for the export
	size_t valueCount = keyValues[0].size();
	FloatList sourceData; sourceData.reserve(dimension * valueCount);
	for (size_t v = 0; v < valueCount; ++v) for (uint32 n = 0; n < dimension; ++n) sourceData.push_back(keyValues[n].at(v));
	AddSourceFloat(parentNode, baseId + "-output", sourceData, dimension, parameters);
	
	if (!inTangents[0].empty() && !outTangents[0].empty())
	{
		sourceData.clear();
		for (size_t v = 0; v < valueCount; ++v) for (uint32 n = 0; n < dimension; ++n)
		{
			sourceData.push_back(inTangents[n].at(v).u);
			sourceData.push_back(inTangents[n].at(v).v);
		}
		AddSourceFloat(parentNode, baseId + "-intangents", sourceData, dimension * 2, FUDaeAccessor::XY);

		sourceData.clear();
		for (size_t v = 0; v < valueCount; ++v) for (uint32 n = 0; n < dimension; ++n)
		{
			sourceData.push_back(outTangents[n].at(v).u);
			sourceData.push_back(outTangents[n].at(v).v);
		}
		AddSourceFloat(parentNode, baseId + "-outtangents", sourceData, dimension * 2, FUDaeAccessor::XY);
	}

	if (!interpolations.empty())
	{
		// Weights not yet supported on multi-curve
		AddSourceInterpolation(parentNode, baseId + "-interpolations", *(FUDaeInterpolationList*)&interpolations);
	}

	SAFE_DELETE_ARRAY(parameters);
}

xmlNode* FCDAnimationMultiCurve::WriteSamplerToXML(xmlNode* parentNode, const string& baseId)
{
	xmlNode* samplerNode = AddChild(parentNode, DAE_SAMPLER_ELEMENT);
	AddAttribute(samplerNode, DAE_ID_ATTRIBUTE, baseId + "-sampler");

	// Add the sampler inputs
	AddInput(samplerNode, baseId + "-input", DAE_INPUT_ANIMATION_INPUT);
	AddInput(samplerNode, baseId + "-output", DAE_OUTPUT_ANIMATION_INPUT);
	if (!inTangents[0].empty() && !outTangents[0].empty())
	{
		AddInput(samplerNode, baseId + "-intangents", DAE_INTANGENT_ANIMATION_INPUT);
		AddInput(samplerNode, baseId + "-outtangents", DAE_OUTTANGENT_ANIMATION_INPUT);
	}
	if (!interpolations.empty())
	{
		AddInput(samplerNode, baseId + "-interpolations", DAE_INTERPOLATION_ANIMATION_INPUT);
	}
	return samplerNode;	
}

xmlNode* FCDAnimationMultiCurve::WriteChannelToXML(xmlNode* parentNode, const string& baseId, const string& pointer)
{
	xmlNode* channelNode = AddChild(parentNode, DAE_CHANNEL_ELEMENT);
	AddAttribute(channelNode, DAE_SOURCE_ATTRIBUTE, string("#") + baseId + "-sampler");

	// Generate and export the full target [no qualifiers]
	globalSBuilder.set(pointer);
	if (targetElement >= 0)
	{
		globalSBuilder.append('('); globalSBuilder.append(targetElement); globalSBuilder.append(')');
	}
	AddAttribute(channelNode, DAE_TARGET_ATTRIBUTE, globalSBuilder);
	return channelNode;
}

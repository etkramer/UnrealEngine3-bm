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
#include "FCDocument/FCDAnimated.h"
#include "FCDocument/FCDAnimationCurve.h"
#include "FCDocument/FCDAnimationClip.h"
#include "FUtils/FUDaeEnum.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeWriter;

ImplementObjectType(FCDAnimationCurve);

FCDAnimationCurve::FCDAnimationCurve(FCDocument* document, FCDAnimationChannel* _parent)
 :	FCDObject(document),
	parent(_parent),
	targetElement(-1),
	preInfinity(FUDaeInfinity::CONSTANT),
	postInfinity(FUDaeInfinity::CONSTANT),
	inputDriver(NULL), inputDriverIndex(0)
{
	currentClip = NULL;
	currentOffset = 0;
}

FCDAnimationCurve::~FCDAnimationCurve()
{
	inputDriver = NULL;
	parent = NULL;
	clips.clear();
	clipOffsets.clear();
}

bool FCDAnimationCurve::HasDriver() const
{
	return inputDriver != NULL;
}

void FCDAnimationCurve::GetDriver(FCDAnimated*& driver, int32& index)
{ const_cast<const FCDAnimationCurve*>(this)->GetDriver(const_cast<const FCDAnimated*&>(driver), index); }
void FCDAnimationCurve::GetDriver(const FCDAnimated*& driver, int32& index) const
{
	driver = inputDriver;
	index = inputDriverIndex;
}

void FCDAnimationCurve::SetDriver(FCDAnimated* driver, int32 index)
{
	inputDriver = driver;
	inputDriverIndex = index;
	SetDirtyFlag();
}

FCDAnimationCurve* FCDAnimationCurve::Clone(bool includeClips)
{
	FCDAnimationCurve* clone = new FCDAnimationCurve(GetDocument(), parent);

	clone->SetTargetElement(targetElement);
	string q;
	q.assign(targetQualifier);
	clone->SetTargetQualifier(q);
	
	clone->keys = keys;
	clone->keyValues = keyValues;
	clone->inTangents = inTangents;
	clone->outTangents = outTangents;
	clone->interpolations = interpolations;

	clone->preInfinity = preInfinity;
	clone->postInfinity = postInfinity;
	clone->inputDriver = inputDriver;
	clone->inputDriverIndex = inputDriverIndex;

	if (includeClips)
	{
		// Animation clips that depend on this curve
		for(FCDAnimationClipList::iterator it = clips.begin(); it != clips.end(); ++it)
		{
			clone->clips.push_back((*it)->Clone());
		}
		for (FloatList::iterator it = clipOffsets.begin(); 
				it != clipOffsets.end(); ++it)
		{
			clone->clipOffsets.push_back(*it);
		}
	}
	return clone;
}

// Prepare a curve for evaluation
void FCDAnimationCurve::Ready()
{
	if (keys.empty()) return;

	if (inTangents.empty() || outTangents.empty())
	{
		// Calculate the bezier tangents
		inTangents.reserve(keys.size());
		outTangents.reserve(keys.size());

		if (keys.size() > 1)
		{
			for (size_t i = 0; i < keys.size(); ++i)
			{
				float previousKeySpan = (i > 0) ? keys[i] - keys[i - 1] : keys[i + 1] - keys[i];
				float nextKeySpan = (i < keys.size() - 1) ? keys[i + 1] - keys[i] : previousKeySpan;
				float currentKeyValue = keyValues[i];
				float previousKeyValue = (i > 0) ? keyValues[i - 1] : currentKeyValue;
				float nextKeyValue = (i < keys.size() - 1) ? keyValues[i + 1] : currentKeyValue;
				float slope = (nextKeyValue - previousKeyValue) / (nextKeySpan + previousKeySpan);
				inTangents.push_back(FMVector2(keys[i] - previousKeySpan / 3.0f, currentKeyValue - previousKeySpan / 3.0f * slope));
				outTangents.push_back(FMVector2(keys[i] + nextKeySpan / 3.0f, currentKeyValue + nextKeySpan / 3.0f * slope));
			}
		}
		SetDirtyFlag();
	}

	if (interpolations.empty())
	{
		// Fill in the array with the default interpolation type
		interpolations.resize(keys.size(), FUDaeInterpolation::DEFAULT);
		SetDirtyFlag();
	}
}

void FCDAnimationCurve::SetCurrentAnimationClip(FCDAnimationClip* clip)
{
	for (size_t i = 0; i < clips.size(); ++i)
	{
		if (clips[i] == clip)
		{
			if (currentClip == clips[i]) return;
			currentClip = clips[i];
			
			float offset = clipOffsets[i] - currentOffset;
			currentOffset = clipOffsets[i];
			for (FloatList::iterator it = keys.begin(); it != keys.end(); ++it)
			{
				(*it) += offset;
			}
			SetDirtyFlag();
		}
	}
	Ready();
}

// Main workhorse for the animation system:
// Evaluates the curve for a given input
float FCDAnimationCurve::Evaluate(float input) const
{
	// Check for empty curves and poses (curves with 1 key).
	FUAssert(!keys.empty(), return 0.0f);
	if (keys.size() == 1) return keyValues.front();

	float outputStart = keyValues.front();
	float outputEnd = keyValues.back();
	float inputStart = keys.front();
	float inputEnd = keys.back();
	float inputSpan = inputEnd - inputStart;

	// Account for pre-infinity mode
	float outputOffset = 0.0f;
	if (input <= inputStart)
	{
		switch (preInfinity)
		{
		case FUDaeInfinity::CONSTANT: return outputStart;
		case FUDaeInfinity::LINEAR: return outputStart + (input - inputStart) * (keyValues[1] - outputStart) / (keys[1] - inputStart);
		case FUDaeInfinity::CYCLE: { float cycleCount = ceilf((inputStart - input) / inputSpan); input += cycleCount * inputSpan; break; }
		case FUDaeInfinity::CYCLE_RELATIVE: { float cycleCount = ceilf((inputStart - input) / inputSpan); input += cycleCount * inputSpan; outputOffset -= cycleCount * (outputEnd - outputStart); break; }
		case FUDaeInfinity::OSCILLATE: { float cycleCount = ceilf((inputStart - input) / (2.0f * inputSpan)); input += cycleCount * 2.0f * inputSpan; input = inputEnd - fabsf(input - inputEnd); break; }
		case FUDaeInfinity::UNKNOWN: default: return outputStart;
		}
	}

	// Account for post-infinity mode
	else if (input >= inputEnd)
	{
		switch (postInfinity)
		{
		case FUDaeInfinity::CONSTANT: return outputEnd;
		case FUDaeInfinity::LINEAR: return outputEnd + (input - inputEnd) * (keyValues[keys.size() - 2] - outputEnd) / (keys[keys.size() - 2] - inputEnd);
		case FUDaeInfinity::CYCLE: { float cycleCount = ceilf((input - inputEnd) / inputSpan); input -= cycleCount * inputSpan; break; }
		case FUDaeInfinity::CYCLE_RELATIVE: { float cycleCount = ceilf((input - inputEnd) / inputSpan); input -= cycleCount * inputSpan; outputOffset += cycleCount * (outputEnd - outputStart); break; }
		case FUDaeInfinity::OSCILLATE: { float cycleCount = ceilf((input - inputEnd) / (2.0f * inputSpan)); input -= cycleCount * 2.0f * inputSpan; input = inputStart + fabsf(input - inputStart); break; }
		case FUDaeInfinity::UNKNOWN: default: return outputEnd;
		}
	}

	// Find the current interval
	uint32 index = 0;
	FloatList::const_iterator it;
	for (it = keys.begin(); it != keys.end(); ++it, ++index)
	{
		if ((*it) > input) break;
	}

	// Get the keys and values for this interval
	float endKey = *it;
	float startKey = *(it - 1);
	float endValue = keyValues[index];
	float startValue = keyValues[index - 1];
	float output;

	// Interpolate the output.
	// Similar code is found in FCDAnimationMultiCurve.cpp. If you update this, update the other one too.
	uint32 interpolation = interpolations.empty() ? ((uint32) FUDaeInterpolation::DEFAULT) : interpolations[index];
	switch (FUDaeInterpolation::Interpolation(interpolation))
	{
	case FUDaeInterpolation::LINEAR:
		output = (input - startKey) / (endKey - startKey) * (endValue - startValue) + startValue;
		break;

	case FUDaeInterpolation::BEZIER: {
		float t = (input - startKey) / (endKey - startKey);
		float b = outTangents[index - 1].y;
		float c = inTangents[index].y;
		float ti = 1.0f - t;
		float br = (endKey - startKey) / (outTangents[index - 1].x - startKey);
		float cr = (endKey - startKey) / (endKey - inTangents[index].x);
		br = FMath::Clamp(br, 0.01f, 100.0f);
		cr = FMath::Clamp(cr, 0.01f, 100.0f);
		output = startValue * ti * ti * ti + br * b * ti * ti * t + cr * c * ti * t * t + endValue * t * t * t;
		break; }

	case FUDaeInterpolation::STEP:
	case FUDaeInterpolation::UNKNOWN:
	default:
		output = startValue;
		break;
	}

	return outputOffset + output;
}

// Apply a conversion function on the key values and tangents
void FCDAnimationCurve::ConvertValues(FCDConversionFunction valueConversion, FCDConversionFunction tangentConversion)
{
	size_t keyCount = keys.size();
	if (valueConversion != NULL)
	{
		for (size_t k = 0; k < keyCount; k++)
		{
			keyValues[k] = (*valueConversion)(keyValues[k]);
		}
	}
	if (tangentConversion != NULL)
	{
		for (size_t k = 0; k < keyCount; k++)
		{
			inTangents[k].v = (*tangentConversion)(inTangents[k].v);
			outTangents[k].v = (*tangentConversion)(outTangents[k].v);
		}
	}
	SetDirtyFlag();
}
void FCDAnimationCurve::ConvertValues(FCDConversionFunctor* valueConversion, FCDConversionFunctor* tangentConversion)
{
	size_t keyCount = keys.size();
	if (valueConversion != NULL)
	{
		for (size_t k = 0; k < keyCount; k++)
		{
			keyValues[k] = (*valueConversion)(keyValues[k]);
		}
	}
	if (tangentConversion != NULL 
		&& keyCount == inTangents.size() 
		&& keyCount == outTangents.size() )
	{
		for (size_t k = 0; k < keyCount; k++)
		{
			inTangents[k].v = (*tangentConversion)(inTangents[k].v);
			outTangents[k].v = (*tangentConversion)(outTangents[k].v);
		}
	}
	SetDirtyFlag();
}

// Apply a conversion function on the key times and tangent weights
void FCDAnimationCurve::ConvertInputs(FCDConversionFunction timeConversion, FCDConversionFunction tangentWeightConversion)
{
	size_t keyCount = keys.size();
	if (timeConversion != NULL)
	{
		for (size_t k = 0; k < keyCount; k++)
		{
			keys[k] = (*timeConversion)(keys[k]);
		}
	}
	if (tangentWeightConversion != NULL)
	{
		for (size_t k = 0; k < keyCount; k++)
		{
			inTangents[k].u = (*tangentWeightConversion)(inTangents[k].u);
			outTangents[k].u = (*tangentWeightConversion)(outTangents[k].u);
		}
	}
	SetDirtyFlag();
}
void FCDAnimationCurve::ConvertInputs(FCDConversionFunctor* timeConversion, FCDConversionFunctor* tangentWeightConversion)
{
	size_t keyCount = keys.size();
	if (timeConversion != NULL)
	{
		for (size_t k = 0; k < keyCount; k++)
		{
			keys[k] = (*timeConversion)(keys[k]);
		}
	}
	if (tangentWeightConversion != NULL)
	{
		for (size_t k = 0; k < keyCount; k++)
		{
			inTangents[k].u = (*tangentWeightConversion)(inTangents[k].u);
			outTangents[k].u = (*tangentWeightConversion)(outTangents[k].u);
		}
	}
	SetDirtyFlag();
}

void FCDAnimationCurve::SetClipOffset(const float offset, 
										 const FCDAnimationClip* clip)
{
	for (size_t i = 0; i < clips.size(); ++i)
	{
		if (clips[i] == clip)
		{
			clipOffsets[i] = offset;
			SetDirtyFlag();
		}
	}
}

void FCDAnimationCurve::RegisterAnimationClip(FCDAnimationClip* clip) 
{ 
	clips.push_back(clip); 
	clipOffsets.push_back(-clip->GetStart());
	SetDirtyFlag();
}

// Write out the specific animation elements to the COLLADA XML tree node
void FCDAnimationCurve::WriteSourceToXML(xmlNode* parentNode, const string& baseId) const
{
	const char* parameter = targetQualifier.c_str();
	if (*parameter == '.') ++parameter;

	xmlNode* sourceNode = AddSourceFloat(parentNode, baseId + "-input", keys, "TIME");
	AddSourceFloat(parentNode, baseId + "-output", keyValues, parameter);
	if (!inTangents.empty() && !outTangents.empty())
	{
		AddSourceTangent(parentNode, baseId + "-intangents", inTangents);
		AddSourceTangent(parentNode, baseId + "-outtangents", outTangents);
	}
	if (!interpolations.empty())
	{
		AddSourceInterpolation(parentNode, baseId + "-interpolations", *(FUDaeInterpolationList*)&interpolations);
	}

	// Export the infinity parameters
	xmlNode* mayaTechnique = AddTechniqueChild(sourceNode, DAEMAYA_MAYA_PROFILE);
	string infinityType = FUDaeInfinity::ToString(preInfinity);
	AddChild(mayaTechnique, DAEMAYA_PREINFINITY_PARAMETER, infinityType);
	infinityType = FUDaeInfinity::ToString(postInfinity);
	AddChild(mayaTechnique, DAEMAYA_POSTINFINITY_PARAMETER, infinityType);
}

xmlNode* FCDAnimationCurve::WriteSamplerToXML(xmlNode* parentNode, const string& baseId) const
{
	xmlNode* samplerNode = AddChild(parentNode, DAE_SAMPLER_ELEMENT);
	AddAttribute(samplerNode, DAE_ID_ATTRIBUTE, baseId + "-sampler");

	// Add the sampler inputs
	AddInput(samplerNode, baseId + "-input", DAE_INPUT_ANIMATION_INPUT);
	AddInput(samplerNode, baseId + "-output", DAE_OUTPUT_ANIMATION_INPUT);
	if (!inTangents.empty() && !outTangents.empty())
	{
		AddInput(samplerNode, baseId + "-intangents", DAE_INTANGENT_ANIMATION_INPUT);
		AddInput(samplerNode, baseId + "-outtangents", DAE_OUTTANGENT_ANIMATION_INPUT);
	}
	if (!interpolations.empty())
	{
		AddInput(samplerNode, baseId + "-interpolations", DAE_INTERPOLATION_ANIMATION_INPUT);
	}

	// Add the driver input
	if (inputDriver != NULL)
	{
		globalSBuilder.set(inputDriver->GetTargetPointer());
		int32 driverElement = inputDriver->GetArrayElement();
		if (driverElement >= 0)
		{
			globalSBuilder.append('('); globalSBuilder.append(driverElement); globalSBuilder.append(')');
		}
		if (inputDriverIndex >= 0)
		{
			globalSBuilder.append('('); globalSBuilder.append(inputDriverIndex); globalSBuilder.append(')');
		}
		AddInput(samplerNode, globalSBuilder.ToCharPtr(), DAEMAYA_DRIVER_INPUT);
	}

	return samplerNode;	
}

xmlNode* FCDAnimationCurve::WriteChannelToXML(xmlNode* parentNode, const string& baseId, const char* targetPointer) const
{
	xmlNode* channelNode = AddChild(parentNode, DAE_CHANNEL_ELEMENT);
	AddAttribute(channelNode, DAE_SOURCE_ATTRIBUTE, string("#") + baseId + "-sampler");

	// Generate and export the channel target
	globalSBuilder.set(targetPointer);
	if (targetElement >= 0)
	{
		globalSBuilder.append('('); globalSBuilder.append(targetElement); globalSBuilder.append(')');
	}
	globalSBuilder.append(targetQualifier);
	AddAttribute(channelNode, DAE_TARGET_ATTRIBUTE, globalSBuilder);
	return channelNode;
}

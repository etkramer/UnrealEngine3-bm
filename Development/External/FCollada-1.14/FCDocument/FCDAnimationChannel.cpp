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
#include "FCDocument/FCDAnimationCurve.h"
#include "FCDocument/FCDAnimationMultiCurve.h"
#include "FUtils/FUDaeEnum.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
#include "FUtils/FUStringConversion.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

ImplementObjectType(FCDAnimationChannel);

FCDAnimationChannel::FCDAnimationChannel(FCDocument* document, FCDAnimation* _parent)
:	FCDObject(document), parent(_parent)
{
}

FCDAnimationChannel::~FCDAnimationChannel()
{
	parent = NULL;
	defaultValues.clear();
}

FCDAnimationCurve* FCDAnimationChannel::AddCurve()
{
	FCDAnimationCurve* curve = curves.Add(GetDocument(), this);
	SetDirtyFlag();
	return curve;
}

// Consider this animated as the curve's driver
bool FCDAnimationChannel::LinkDriver(FCDAnimated* animated)
{
	bool driver = !driverPointer.empty();
	driver = driver && animated->GetTargetPointer() == driverPointer;
	if (driver && driverQualifier >= 0 && (uint32) driverQualifier < animated->GetValueCount())
	{
		// Retrieve the value pointer for the driver
		for (FCDAnimationCurveList::iterator itC = curves.begin(); itC != curves.end(); ++itC)
		{
			(*itC)->SetDriver(animated, driverQualifier);
		}
	}
	return driver;
}
FUStatus FCDAnimationChannel::CheckDriver()
{
	FUStatus status;
	if (!driverPointer.empty() && !curves.empty() && !curves.front()->HasDriver())
	{
		status.Fail(FS("Unable to find animation curve driver: ") + TO_FSTRING(driverPointer) + FS(" for animation: ") + TO_FSTRING(parent->GetDaeId()));
	}
	return status;
}

// Load a Collada animation channel from the XML document
FUStatus FCDAnimationChannel::LoadFromXML(xmlNode* channelNode)
{
	FUStatus status;

	// Read the channel-specific ID
	string daeId = ReadNodeId(channelNode);
	string samplerId = ReadNodeSource(channelNode);
	ReadNodeTargetProperty(channelNode, targetPointer, targetQualifier);

	xmlNode* samplerNode = parent->FindChildById(samplerId);
	if (samplerNode == NULL || !IsEquivalent(samplerNode->name, DAE_SAMPLER_ELEMENT))
	{
		return status.Fail(FS("Unable to find sampler node for channel node: ") + TO_FSTRING(daeId), channelNode->line);
	}

	// Find and process the sources
	xmlNode* inputSource = NULL,* outputSource = NULL,* inTangentSource = NULL,* outTangentSource = NULL,* interpolationSource = NULL;
	xmlNodeList samplerInputNodes;
	string inputDriver;
	FindChildrenByType(samplerNode, DAE_INPUT_ELEMENT, samplerInputNodes);
	for (size_t i = 0; i < samplerInputNodes.size(); ++i) // Don't use iterator here because we are possibly appending source nodes in the loop
	{
		xmlNode* inputNode = samplerInputNodes[i];
		string sourceId = ReadNodeSource(inputNode);
		xmlNode* sourceNode = parent->FindChildById(sourceId);
		string sourceSemantic = ReadNodeSemantic(inputNode);

		if (sourceSemantic == DAE_INPUT_ANIMATION_INPUT) inputSource = sourceNode;
		else if (sourceSemantic == DAE_OUTPUT_ANIMATION_INPUT) outputSource = sourceNode;
		else if (sourceSemantic == DAE_INTANGENT_ANIMATION_INPUT) inTangentSource = sourceNode;
		else if (sourceSemantic == DAE_OUTTANGENT_ANIMATION_INPUT) outTangentSource = sourceNode;
		else if (sourceSemantic == DAE_INTERPOLATION_ANIMATION_INPUT) interpolationSource = sourceNode;
		else if (sourceSemantic == DAEMAYA_DRIVER_INPUT) inputDriver = sourceId;
	}
	if (inputSource == NULL || outputSource == NULL)
	{
		return status.Fail(FS("Missing INPUT or OUTPUT sources in animation channel: ") + TO_FSTRING(parent->GetDaeId()), samplerNode->line);
	}

	// Calculate the number of curves that in contained by this channel
	xmlNode* outputAccessor = FindTechniqueAccessor(outputSource);
	string accessorStrideString = ReadNodeProperty(outputAccessor, DAE_STRIDE_ATTRIBUTE);
	uint32 curveCount = FUStringConversion::ToUInt32(accessorStrideString);
	if (curveCount == 0) curveCount = 1;

	// Create the animation curves
	curves.reserve(curveCount);
	for (uint32 i = 0; i < curveCount; ++i) AddCurve();

	// Read in the animation curves
	// The input keys and interpolations are shared by all the curves
	FloatList& inputKeys = curves.front()->GetKeys();
    ReadSource(inputSource, inputKeys);
	ReadSourceInterpolation(interpolationSource, curves.front()->GetInterpolations());
	for (uint32 i = 1; i < curveCount; ++i)
	{
		curves[i]->GetKeys() = inputKeys;
		curves[i]->GetInterpolations() = curves.front()->GetInterpolations();
	}

	// Read in the interleaved outputs and tangents as floats
	vector<FloatList*> outArrays(curveCount);
	for (uint32 i = 0; i < curveCount; ++i) outArrays[i] = &(curves[i]->GetKeyValues());
	ReadSourceInterleaved(outputSource, outArrays);

	// Read in the interleaved in_tangent source.
	if (inTangentSource != NULL)
	{
		vector<FMVector2List*> arrays(curveCount);
		for (uint32 i = 0; i < curveCount; ++i) arrays[i] = &(curves[i]->GetInTangents());

		uint32 stride = ReadSourceInterleaved(inTangentSource, arrays);
		if (stride == arrays.size() && !inputKeys.empty())
		{
			// Backward compatibility with 1D tangents.
			// Remove the relativity from the 1D tangents and calculate the second-dimension.
			for (uint32 i = 0; i < stride; ++i)
			{
				if (arrays[i]->empty()) continue;
				arrays[i]->front() = FMVector2(inputKeys[0], outArrays[i]->front());
				for (size_t j = 1; j < arrays[i]->size(); ++j)
				{
					if (j >= outArrays[i]->size() || j >= inputKeys.size()) break;
					FMVector2& t = arrays[i]->at(j);
					t.y = outArrays[i]->at(j) - t.x;
					t.x = (inputKeys[j - 1] + 2.0f * inputKeys[j]) / 3.0f;
				}
			}
		}
	}

	// Read in the interleaved out_tangent source
	if (outTangentSource != NULL)
	{
		vector<FMVector2List*> arrays(curveCount);
		for (uint32 i = 0; i < curveCount; ++i) arrays[i] = &(curves[i]->GetOutTangents());

		uint32 stride = ReadSourceInterleaved(outTangentSource, arrays);
		if (stride == arrays.size() && !inputKeys.empty())
		{
			// Backward compatibility with 1D tangents.
			// Remove the relativity from the 1D tangents and calculate the second-dimension.
			for (uint32 i = 0; i < stride; ++i)
			{
				if (arrays[i]->empty()) continue;
				for (size_t j = 0; j < arrays[i]->size() - 1; ++j)
				{
					if (j >= outArrays[i]->size() || j >= inputKeys.size() - 1) break;
					FMVector2& t = arrays[i]->at(j);
					t.y = outArrays[i]->at(j) + t.x;
					t.x = (inputKeys[j + 1] + 2.0f * inputKeys[j]) / 3.0f;
				}
				arrays[i]->back() = FMVector2(inputKeys[0], outArrays[i]->back());
			}
		}
	}

	// Read in the pre/post-infinity type
	xmlNodeList mayaParameterNodes; StringList mayaParameterNames;
	xmlNode* mayaTechnique = FindTechnique(inputSource, DAEMAYA_MAYA_PROFILE);
	FindParameters(mayaTechnique, mayaParameterNames, mayaParameterNodes);
	size_t parameterCount = mayaParameterNodes.size();
	for (size_t i = 0; i < parameterCount; ++i)
	{
		xmlNode* parameterNode = mayaParameterNodes[i];
		const string& paramName = mayaParameterNames[i];
		const char* content = ReadNodeContentDirect(parameterNode);

		if (paramName == DAEMAYA_PREINFINITY_PARAMETER || paramName == DAEMAYA_PREINFINITY_PARAMETER1_3)
		{
			for (FCDAnimationCurveList::iterator itC = curves.begin(); itC != curves.end(); ++itC)
			{
				(*itC)->SetPreInfinity(FUDaeInfinity::FromString(content));
			}
		}
		else if (paramName == DAEMAYA_POSTINFINITY_PARAMETER || paramName == DAEMAYA_POSTINFINITY_PARAMETER1_3)
		{
			for (FCDAnimationCurveList::iterator itC = curves.begin(); itC != curves.end(); ++itC)
			{
				(*itC)->SetPostInfinity(FUDaeInfinity::FromString(content));
			}
		}
		else
		{
			// Look for driven-key input target
			if (paramName == DAE_INPUT_ELEMENT)
			{
				string semantic = ReadNodeSemantic(parameterNode);
				if (semantic == DAEMAYA_DRIVER_INPUT)
				{
					inputDriver = ReadNodeSource(parameterNode);
				}
			}
		}
	}

	if (!inputDriver.empty())
	{
		const char* driverTarget = FUDaeParser::SkipPound(inputDriver);
		if (driverTarget != NULL)
		{
			string driverQualifierValue;
			FUDaeParser::SplitTarget(driverTarget, driverPointer, driverQualifierValue);
			driverQualifier = FUDaeParser::ReadTargetMatrixElement(driverQualifierValue);
		}
	}

	// Ready the curves for usage/evaluation.
	for (uint32 i = 0; i < curveCount; ++i)
	{
		curves[i]->Ready();
	}

	SetDirtyFlag();
	return status;
}

// Write out the animation curves for an animation channel to a COLLADA document
void FCDAnimationChannel::WriteToXML(xmlNode* parentNode) const
{
	string baseId = CleanId(parent->GetDaeId() + "_" + targetPointer);

	// Check for curve merging
	uint32 realCurveCount = 0;
	FCDAnimationCurve* masterCurve = NULL;
	FCDAnimationCurveList mergingCurves(defaultValues.size(), NULL);
	bool mergeCurves = true;
	for (FCDAnimationCurveList::const_iterator itC = curves.begin(); itC != curves.end() && mergeCurves; ++itC)
	{
		FCDAnimationCurve* curve = (*itC);
		if (curve != NULL)
		{
			// Check that we have a default placement for this curve in the default value listing
			size_t dv;
			for (dv = 0; dv < defaultValues.size(); ++dv)
			{
				if (defaultValues[dv].curve == curve)
				{
					mergingCurves[dv] = curve;
					break;
				}
			}
			mergeCurves &= dv != defaultValues.size();

			// Check that the curves can be merged correctly.
			++realCurveCount;
			if (masterCurve == NULL)
			{
				masterCurve = curve;
			}
			else
			{
				// Check the infinity types, the keys and the interpolations.
				mergeCurves &= IsEquivalent(masterCurve->GetKeys(), curve->GetKeys());
				if (!curve->GetInterpolations().empty() && !masterCurve->GetInterpolations().empty())
				{
					mergeCurves &= IsEquivalent(curve->GetInterpolations(), masterCurve->GetInterpolations());
				}
				mergeCurves &= curve->GetPostInfinity() == masterCurve->GetPostInfinity() && curve->GetPreInfinity() == masterCurve->GetPreInfinity();
			}

			// Disallow the merging of any curves with a driver.
			mergeCurves &= !curve->HasDriver();
		}
	}

	if (mergeCurves && realCurveCount > 1)
	{
		// Prepare the list of default values
		FloatList values;
		StringList qualifiers;
		values.reserve(defaultValues.size());
		qualifiers.reserve(defaultValues.size());
		for (FCDAnimationChannelDefaultValueList::const_iterator itDV = defaultValues.begin(); itDV != defaultValues.end(); ++itDV)
		{
			values.push_back((*itDV).defaultValue);
			qualifiers.push_back((*itDV).defaultQualifier);
		}

		// Merge and export the curves
		FCDAnimationMultiCurve* multiCurve = FCDAnimationMultiCurve::MergeCurves(mergingCurves, values, qualifiers);
		multiCurve->WriteSourceToXML(parentNode, baseId);
		multiCurve->WriteSamplerToXML(parentNode, baseId);
		multiCurve->WriteChannelToXML(parentNode, baseId, targetPointer);
		SAFE_DELETE(multiCurve);
	}
	else
	{
		// Interlace the curve's sources, samplers and channels
		// Generate new ids for each of the curve's data sources, to avoid collision in special cases
		size_t curveCount = curves.size();
		StringList ids; ids.resize(curves.size());
		FUSStringBuilder curveId;
		for (size_t c = 0; c < curveCount; ++c)
		{
			if (curves[c] != NULL)
			{
				// Generate a valid id for this curve
				curveId.set(baseId);
				if (curves[c]->GetTargetElement() >= 0)
				{
					curveId.append('_'); curveId.append(curves[c]->GetTargetElement()); curveId.append('_');
				}
				curveId.append(curves[c]->GetTargetQualifier());
				ids[c] = CleanId(curveId.ToCharPtr());

				// Write out the curve's sources
				curves[c]->WriteSourceToXML(parentNode, ids[c]);
			}
		}
		for (size_t c = 0; c < curveCount; ++c)
		{
			if (curves[c] != NULL) curves[c]->WriteSamplerToXML(parentNode, ids[c]);
		}
		for (size_t c = 0; c < curveCount; ++c)
		{
			if (curves[c] != NULL) curves[c]->WriteChannelToXML(parentNode, ids[c], targetPointer.c_str());
		}
	}

	const_cast<FCDAnimationChannel*>(this)->defaultValues.clear();
}

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
#include "FCDocument/FCDAnimationChannel.h"
#include "FCDocument/FCDAnimationCurve.h"
#include "FCDocument/FCDAnimationMultiCurve.h"
#include "FCDocument/FCDAnimated.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

//
// FCDAnimated
//

ImplementObjectType(FCDAnimated);

FCDAnimated::FCDAnimated(FCDocument* document, size_t valueCount)
:	FCDObject(document)
{
	arrayElement = -1;

	// Allocate the values/qualifiers/curves arrays
	values.resize(valueCount, NULL);
	qualifiers.resize(valueCount, "");
	curves.resize(valueCount);
}

FCDAnimated::~FCDAnimated()
{
	GetDocument()->UnregisterAnimatedValue(this);

	values.clear();
	qualifiers.clear();
	curves.clear();
}

// Assigns a curve to a value of the animated element.
bool FCDAnimated::AddCurve(size_t index, FCDAnimationCurve* curve)
{
	FUAssert(index < GetValueCount(), return false);
	curves.at(index).push_back(curve);
	SetDirtyFlag();
	return true;
}

bool FCDAnimated::AddCurve(size_t index, FCDAnimationCurveList& curve)
{
	FUAssert(index < GetValueCount() || curve.empty(), return false);
	curves.at(index).insert(curves.at(index).end(), curve.begin(), curve.end());
	SetDirtyFlag();
	return true;
}

// Removes the curves affecting a value of the animated element.
bool FCDAnimated::RemoveCurve(size_t index)
{
	FUAssert(index < GetValueCount(), return false);
	bool hasCurve = !curves[index].empty();
	curves[index].clear();
	SetDirtyFlag();
	return hasCurve;
}
	
bool FCDAnimated::Link(xmlNode* node)
{
	bool linked = false;

	if (node != NULL)
	{
		// Write down the expected target string for the given node
		FUDaeParser::CalculateNodeTargetPointer(node, pointer);

		// Check if this animated value is used as a driver
		linked |= GetDocument()->LinkDriver(this);

		// Retrieve the list of the channels pointing to this node
		FCDAnimationChannelList channels;
		GetDocument()->FindAnimationChannels(pointer, channels);
		linked |= ProcessChannels(channels);
	}
	else linked = true;

	if (linked)
	{
		// Register this animated value with the document
		GetDocument()->RegisterAnimatedValue(this);
	}

	SetDirtyFlag();
	return linked;
}

bool FCDAnimated::ProcessChannels(FCDAnimationChannelList& channels)
{
	bool linked = false;
	for (FCDAnimationChannelList::iterator it = channels.begin(); it != channels.end(); ++it)
	{
		FCDAnimationChannel* channel = *it;
		const FCDAnimationCurveList& channelCurves = channel->GetCurves();
		if (channelCurves.empty()) continue;
		
		// Retrieve the channel's qualifier and check for a requested matrix element
		string qualifier = channel->GetTargetQualifier();
		if (arrayElement != -1)
		{
			int32 element = ReadTargetMatrixElement(qualifier);
			if (arrayElement != element) continue;
		}

		if (qualifier.empty())
		{
			// An empty qualifier implies that the channel should provide ALL the curves
			for (size_t i = 0; i < channelCurves.size() && i < curves.size(); ++i)
			{
				AddCurve(i, channelCurves[i]);
				linked = true;
			}
		}
		else
		{
			// Attempt to match the qualifier with this animated qualifiers
			size_t index;
			for (index = 0; index < qualifiers.size(); ++index)
			{
				if (qualifiers[index] == qualifier) break;
			}

			// Check for a matrix element instead
			if (index == qualifiers.size()) index = ReadTargetMatrixElement(qualifier);
			if (index < qualifiers.size())
			{
				AddCurve(index, channelCurves.front());
				linked = true;
			}
			/* else return status.Fail(FS("Invalid qualifier for animation channel target: ") + TO_FSTRING(pointer)); */
		}
	}

	if (linked)
	{
		// Now that the curves are imported: set their target information
		for (size_t i = 0; i < curves.size(); ++i)
		{
			for (size_t j = 0; j < curves[i].size(); ++j)
			{
                curves[i][j]->SetTargetElement(arrayElement);
				curves[i][j]->SetTargetQualifier(qualifiers[i]);
			}
		}
	}

	SetDirtyFlag();
	return linked;
}

const string& FCDAnimated::GetQualifier(size_t index) const
{
	FUAssert(index < GetValueCount(), return emptyString);
	return qualifiers.at(index);
}

// Retrieves an animated value, given a valid qualifier
float* FCDAnimated::FindValue(const string& qualifier)
{
	for (size_t i = 0; i < qualifiers.size(); ++i)
	{
		if (qualifiers[i] == qualifier) return values[i];
	}
	return NULL;
}
const float* FCDAnimated::FindValue(const string& qualifier) const
{
	for (size_t i = 0; i < qualifiers.size(); ++i)
	{
		if (qualifiers[i] == qualifier) return values[i];
	}
	return NULL;
}

// Retrieve the index of a given qualifier
size_t FCDAnimated::FindQualifier(const char* qualifier) const
{
	for (size_t i = 0; i < qualifiers.size(); ++i)
	{
		if (qualifiers[i] == qualifier) return i;
	}

	// Otherwise, check for a matrix element
	string q = qualifier;
	int32 matrixElement = FUDaeParser::ReadTargetMatrixElement(q);
	if (matrixElement >= 0 && matrixElement < (int32) qualifiers.size()) return matrixElement;
	return size_t(-1);
}

// Retrieve the index of a given value pointer
size_t FCDAnimated::FindValue(const float* value) const
{
	for (size_t i = 0; i < values.size(); ++i)
	{
		if (values[i] == value) return i;
	}
	return size_t(-1);
}

// Returns whether any of the contained curves are non-NULL
bool FCDAnimated::HasCurve() const
{
	FCDAnimationCurveListList::const_iterator cit;
	for (cit = curves.begin(); cit != curves.end() && (*cit).empty(); ++cit) {}
	return cit != curves.end();
}

// Create one multi-curve out of this animated value's single curves
FCDAnimationMultiCurve* FCDAnimated::CreateMultiCurve() const
{
	FloatList defaultValues;
	size_t count = values.size();
	defaultValues.resize(count);
	for (size_t i = 0; i < count; ++i) defaultValues[i] = (*values[i]);

	vector<const FCDAnimationCurve*> toMerge;
	toMerge.resize(count);
	for (size_t i = 0; i < count; ++i) toMerge[i] = (!curves[i].empty()) ? curves[i][0] : NULL;
	return FCDAnimationMultiCurve::MergeCurves(toMerge, defaultValues);
}

// Create one multi-curve out of the single curves from many FCDAnimated objects
FCDAnimationMultiCurve* FCDAnimated::CreateMultiCurve(const FCDAnimatedList& toMerge)
{
	// Calculate the total dimension of the curve to create
	size_t count = 0;
	for (FCDAnimatedList::const_iterator cit = toMerge.begin(); cit != toMerge.end(); ++cit)
	{
		count += (*cit)->GetValueCount();
	}

	// Generate the list of default values and the list of curves
	FloatList defaultValues(count, 0.0f);
	vector<const FCDAnimationCurve*> curves(count, NULL);
	size_t offset = 0;
	for (FCDAnimatedList::const_iterator cit = toMerge.begin(); cit != toMerge.end(); ++cit)
	{
		size_t localCount = (*cit)->GetValueCount();
		for (size_t i = 0; i < localCount; ++i)
		{
			defaultValues[offset + i] = *(*cit)->GetValue(i);
			curves[offset + i] = (*cit)->GetCurve(i);
		}
		offset += localCount;
	}

	return FCDAnimationMultiCurve::MergeCurves(curves, defaultValues);
}

// Sample the animated values for a given time
void FCDAnimated::Evaluate(float time)
{
	size_t valueCount = values.size();
	size_t curveCount = curves.size();
	size_t count = min(curveCount, valueCount);
	for (size_t i = 0; i < count; ++i)
	{
		if (!curves[i].empty())
		{
			// Retrieve the curve and the corresponding value
			FCDAnimationCurve* curve = curves[i][0];
			if (curve == NULL) continue;
			float* value = values[i];
			if (value == NULL) continue;

			// Evaluate the curve at this time
			(*value) = curve->Evaluate(time);
		}
	}
}

FCDAnimated* FCDAnimated::Clone(FCDocument* document) const
{
	FCDAnimated* clone = new FCDAnimated(document, GetValueCount());
	clone->arrayElement = arrayElement;
	for(size_t i = 0; i < curves.size(); ++i) 
	{
		for (size_t j = 0; j < curves[i].size(); ++j)
		{
			clone->AddCurve(i, curves[i][j]->Clone());
		}
	}
	clone->SetTargetPointer(pointer);
	clone->qualifiers = qualifiers;
	clone->values = values;
	document->RegisterAnimatedValue(clone);
	return clone;
}


// Clones the whole animated associated with a given value.
FCDAnimated* FCDAnimated::Clone(FCDocument* document, const float* animatedValue, FloatPtrList& newAnimatedValues)
{
	const FCDAnimated* toClone = document->FindAnimatedValue(animatedValue);
	if (toClone == NULL) return NULL;

	FCDAnimated* clone = new FCDAnimated(document, toClone->GetValueCount());
	clone->arrayElement = toClone->arrayElement;
	for(size_t i = 0; i < toClone->curves.size(); ++i) 
	{
		for (size_t j = 0; j < toClone->curves[i].size(); ++j)
		{
			clone->AddCurve(i, toClone->curves[i][j]->Clone());
		}
	}

	clone->SetTargetPointer(toClone->pointer);
	clone->qualifiers = toClone->qualifiers;
	clone->values = newAnimatedValues;
	document->RegisterAnimatedValue(clone);
	return clone;
}

//
// FCDAnimatedFloat
//

ImplementObjectType(FCDAnimatedFloat);

FCDAnimatedFloat::FCDAnimatedFloat(FCDocument* document, float* value, int32 _arrayElement) : FCDAnimated(document, 1)
{
	values[0] = value;
	qualifiers[0] = "";
	arrayElement = _arrayElement;
}

void FCDAnimatedFloat::SetQualifier(const char* qualifier)
{
	qualifiers[0] = qualifier;
	SetDirtyFlag();
}

FCDAnimatedFloat* FCDAnimatedFloat::Create(FCDocument* document, float* value, int32 arrayElement)
{
	FCDAnimatedFloat* animated = new FCDAnimatedFloat(document, value, arrayElement);
	if (!animated->Link(NULL)) SAFE_DELETE(animated);
	return animated;
}
FCDAnimatedFloat* FCDAnimatedFloat::Create(FCDocument* document, xmlNode* node, float* value, int32 arrayElement)
{
	FCDAnimatedFloat* animated = new FCDAnimatedFloat(document, value, arrayElement);
	if (!animated->Link(node)) SAFE_DELETE(animated);
	return animated;
}

FCDAnimated* FCDAnimatedFloat::Clone(FCDocument* document, const float* oldValue, float* newValue)
{
	FloatPtrList newValues;
	newValues.push_back(newValue);
	return FCDAnimated::Clone(document, oldValue, newValues); 
}

//
// FCDAnimatedPoint3
//

ImplementObjectType(FCDAnimatedPoint3);

FCDAnimatedPoint3::FCDAnimatedPoint3(FCDocument* document, FMVector3* value, int32 _arrayElement) : FCDAnimated(document, 3)
{
	values[0] = &value->x; values[1] = &value->y; values[2] = &value->z;
	qualifiers[0] = ".X"; qualifiers[1] = ".Y"; qualifiers[2] = ".Z";
	arrayElement = _arrayElement;
}

FCDAnimatedPoint3* FCDAnimatedPoint3::Create(FCDocument* document, FMVector3* value, int32 arrayElement)
{
	FCDAnimatedPoint3* animated = new FCDAnimatedPoint3(document, value, arrayElement);
	if (!animated->Link(NULL)) SAFE_DELETE(animated);
	return animated;
}
FCDAnimatedPoint3* FCDAnimatedPoint3::Create(FCDocument* document, xmlNode* node, FMVector3* value, int32 arrayElement)
{
	FCDAnimatedPoint3* animated = new FCDAnimatedPoint3(document, value, arrayElement);
	if (!animated->Link(node)) SAFE_DELETE(animated);
	return animated;
}

FCDAnimated* FCDAnimatedPoint3::Clone(FCDocument* document, const FMVector3* oldValue, FMVector3* newValue)
{
	FloatPtrList newValues;
	newValues.push_back(&newValue->x); newValues.push_back(&newValue->y); newValues.push_back(&newValue->z);
	return FCDAnimated::Clone(document, &oldValue->x, newValues); 
}

//
// FCDAnimatedColor
//

ImplementObjectType(FCDAnimatedColor);

FCDAnimatedColor::FCDAnimatedColor(FCDocument* document, FMVector3* value, int32 _arrayElement) : FCDAnimated(document, 3)
{
	values[0] = &value->x; values[1] = &value->y; values[2] = &value->z; 
	qualifiers[0] = ".R"; qualifiers[1] = ".G"; qualifiers[2] = ".B";
	arrayElement = _arrayElement; 
}

FCDAnimatedColor::FCDAnimatedColor(FCDocument* document, FMVector4* value, int32 _arrayElement) : FCDAnimated(document, 4)
{
	values[0] = &value->x; values[1] = &value->y; values[2] = &value->z; values[3] = &value->w;
	qualifiers[0] = ".R"; qualifiers[1] = ".G"; qualifiers[2] = ".B"; qualifiers[3] = ".A";
	arrayElement = _arrayElement; 
}

FCDAnimatedColor* FCDAnimatedColor::Create(FCDocument* document, FMVector3* value, int32 arrayElement)
{
	FCDAnimatedColor* animated = new FCDAnimatedColor(document, value, arrayElement);
	if (!animated->Link(NULL)) 
		SAFE_DELETE(animated);
	return animated;
}

FCDAnimatedColor* FCDAnimatedColor::Create(FCDocument* document, FMVector4* value, int32 arrayElement)
{
	FCDAnimatedColor* animated = new FCDAnimatedColor(document, value, arrayElement);
	if (!animated->Link(NULL)) 
		SAFE_DELETE(animated);
	return animated;
}

FCDAnimatedColor* FCDAnimatedColor::Create(FCDocument* document, xmlNode* node, FMVector3* value, int32 arrayElement)
{
	FCDAnimatedColor* animated = new FCDAnimatedColor(document, value, arrayElement);
	if (!animated->Link(node)) 
		SAFE_DELETE(animated);
	return animated;
}

FCDAnimatedColor* FCDAnimatedColor::Create(FCDocument* document, xmlNode* node, FMVector4* value, int32 arrayElement)
{
	FCDAnimatedColor* animated = new FCDAnimatedColor(document, value, arrayElement);
	if (!animated->Link(node)) 
		SAFE_DELETE(animated);
	return animated;
}

FCDAnimated* FCDAnimatedColor::Clone(FCDocument* document, const FMVector3* oldValue, FMVector3* newValue)
{
	FloatPtrList newValues; newValues.push_back(&newValue->x); newValues.push_back(&newValue->y); newValues.push_back(&newValue->z);
	return FCDAnimated::Clone(document, &oldValue->x, newValues); 
}

FCDAnimated* FCDAnimatedColor::Clone(FCDocument* document, const FMVector4* oldValue, FMVector4* newValue)
{
	FloatPtrList newValues; newValues.push_back(&newValue->x); newValues.push_back(&newValue->y); newValues.push_back(&newValue->z); newValues.push_back(&newValue->w);
	return FCDAnimated::Clone(document, &oldValue->x, newValues); 
}

//
// FCDAnimatedAngle
//

ImplementObjectType(FCDAnimatedAngle);

FCDAnimatedAngle::FCDAnimatedAngle(FCDocument* document, float* value, int32 _arrayElement) : FCDAnimated(document, 1)
{
	values[0] = value;
	qualifiers[0] = ".ANGLE";
	arrayElement = _arrayElement;
}

FCDAnimatedAngle* FCDAnimatedAngle::Create(FCDocument* document, float* value, int32 arrayElement)
{
	FCDAnimatedAngle* animated = new FCDAnimatedAngle(document, value, arrayElement);
	if (!animated->Link(NULL)) SAFE_DELETE(animated);
	return animated;
}
FCDAnimatedAngle* FCDAnimatedAngle::Create(FCDocument* document, xmlNode* node, float* value, int32 arrayElement)
{
	FCDAnimatedAngle* animated = new FCDAnimatedAngle(document, value, arrayElement);
	if (!animated->Link(node)) SAFE_DELETE(animated);
	return animated;
}

FCDAnimated* FCDAnimatedAngle::Clone(FCDocument* document, const float* oldValue, float* newValue)
{
	FloatPtrList newValues;
	newValues.push_back(newValue); 
	return FCDAnimated::Clone(document, oldValue, newValues); 
}

//
// FCDAnimatedAngleAxis
//

ImplementObjectType(FCDAnimatedAngleAxis);

FCDAnimatedAngleAxis::FCDAnimatedAngleAxis(FCDocument* document, FMVector3* axis, float* angle, int32 _arrayElement) : FCDAnimated(document, 4)
{
	values[0] = &axis->x; values[1] = &axis->y; values[2] = &axis->z; values[3] = angle;
	qualifiers[0] = ".X"; qualifiers[1] = ".Y"; qualifiers[2] = ".Z"; qualifiers[3] = ".ANGLE";
	arrayElement = _arrayElement;
}

FCDAnimatedAngleAxis* FCDAnimatedAngleAxis::Create(FCDocument* document, FMVector3* axis, float* angle, int32 arrayElement)
{
	FCDAnimatedAngleAxis* animated = new FCDAnimatedAngleAxis(document, axis, angle, arrayElement);
	if (!animated->Link(NULL)) SAFE_DELETE(animated);
	return animated;
}
FCDAnimatedAngleAxis* FCDAnimatedAngleAxis::Create(FCDocument* document, xmlNode* node, FMVector3* axis, float* angle, int32 arrayElement)
{
	FCDAnimatedAngleAxis* animated = new FCDAnimatedAngleAxis(document, axis, angle, arrayElement);
	if (!animated->Link(node)) SAFE_DELETE(animated);
	return animated;
}

FCDAnimated* FCDAnimatedAngleAxis::Clone(FCDocument* document, const float* oldAngle, FMVector3* newAxis, float* newAngle)
{
	FloatPtrList newValues;
	newValues.push_back(&newAxis->x); newValues.push_back(&newAxis->y); newValues.push_back(&newAxis->z);
	newValues.push_back(newAngle);
	return FCDAnimated::Clone(document, oldAngle, newValues); 
}

//
// FCDAnimatedMatrix
//

ImplementObjectType(FCDAnimatedMatrix);

FCDAnimatedMatrix::FCDAnimatedMatrix(FCDocument* document, FMMatrix44* value, int32 _arrayElement) : FCDAnimated(document, 16)
{
	arrayElement = _arrayElement;

#define MX_V(a,b) values[b*4+a] = (&(*value)[a][b])
	MX_V(0,0); MX_V(1,0); MX_V(2,0); MX_V(3,0); 
	MX_V(0,1); MX_V(1,1); MX_V(2,1); MX_V(3,1); 
	MX_V(0,2); MX_V(1,2); MX_V(2,2); MX_V(3,2); 
	MX_V(0,3); MX_V(1,3); MX_V(2,3); MX_V(3,3);
#undef MX_V

#define MX_V(a,b) qualifiers[b*4+a] = ("(" #a ")(" #b ")");
	MX_V(0,0); MX_V(0,1); MX_V(0,2); MX_V(0,3); 
	MX_V(1,0); MX_V(1,1); MX_V(1,2); MX_V(1,3); 
	MX_V(2,0); MX_V(2,1); MX_V(2,2); MX_V(2,3); 
	MX_V(3,0); MX_V(3,1); MX_V(3,2); MX_V(3,3);
#undef MX_V
}

FCDAnimatedMatrix* FCDAnimatedMatrix::Create(FCDocument* document, FMMatrix44* value, int32 arrayElement)
{
	FCDAnimatedMatrix* animated = new FCDAnimatedMatrix(document, value, arrayElement);
	if (!animated->Link(NULL)) SAFE_DELETE(animated);
	return animated;
}
FCDAnimatedMatrix* FCDAnimatedMatrix::Create(FCDocument* document, xmlNode* node, FMMatrix44* value, int32 arrayElement)
{
	FCDAnimatedMatrix* animated = new FCDAnimatedMatrix(document, value, arrayElement);
	if (!animated->Link(node)) SAFE_DELETE(animated);
	return animated;
}

FCDAnimated* FCDAnimatedMatrix::Clone(FCDocument* document, const FMMatrix44* oldMx, FMMatrix44* newMx)
{
	FloatPtrList newValues; 

#define MX_V(a,b) newValues.push_back(&(*newMx)[a][b]);
	MX_V(0,0); MX_V(1,0); MX_V(2,0); MX_V(3,0); 
	MX_V(0,1); MX_V(1,1); MX_V(2,1); MX_V(3,1); 
	MX_V(0,2); MX_V(1,2); MX_V(2,2); MX_V(3,2); 
	MX_V(0,3); MX_V(1,3); MX_V(2,3); MX_V(3,3);
#undef MX_V

	return FCDAnimated::Clone(document, &(*oldMx)[0][0], newValues); 
}

//
// FCDAnimatedCustom
//

ImplementObjectType(FCDAnimatedCustom);

FCDAnimatedCustom::FCDAnimatedCustom(FCDocument* document) : FCDAnimated(document, 1)
{
	dummy = 0.0f;
	values[0] = &dummy;
}

bool FCDAnimatedCustom::Link(xmlNode* node)
{
    bool linked = false;

	if (node != NULL)
	{
		// Retrieve the list of the channels pointing to this node
		FUDaeParser::CalculateNodeTargetPointer(node, pointer);
		FCDAnimationChannelList channels;
		GetDocument()->FindAnimationChannels(pointer, channels);

		// Extract all the qualifiers needed to hold these channels
		qualifiers.clear();
		for (FCDAnimationChannelList::iterator itC = channels.begin(); itC != channels.end(); ++itC)
		{
			FCDAnimationChannel* channel = *itC;
			const FCDAnimationCurveList& channelCurves = channel->GetCurves();
			if (channelCurves.empty()) continue;
			
			// Retrieve the channel's qualifier
			string qualifier = channel->GetTargetQualifier();
			if (qualifier.empty())
			{
				// Implies one channel holding multiple curves
				qualifiers.resize(channelCurves.size());
				break;
			}
			else
			{
				qualifiers.push_back(qualifier);
			}
		}

		// Link the curves and check if this animated value is used as a driver
		values.resize(qualifiers.size());
		curves.resize(values.size());
		for (size_t i = 0; i < qualifiers.size(); ++i) { values[i] = &dummy; }
		linked |= ProcessChannels(channels);
		linked |= GetDocument()->LinkDriver(this);
	}
	else linked = true;

	if (linked)
	{
		// Register this animated value with the document
		GetDocument()->RegisterAnimatedValue(this);
	}
	SetDirtyFlag();
	return linked;
}

FCDAnimatedCustom* FCDAnimatedCustom::Create(FCDocument* document)
{
	FCDAnimatedCustom* animated = new FCDAnimatedCustom(document);
	if (!animated->Link(NULL)) SAFE_DELETE(animated);
	return animated;
}
FCDAnimatedCustom* FCDAnimatedCustom::Create(FCDocument* document, xmlNode* node)
{
	FCDAnimatedCustom* animated = new FCDAnimatedCustom(document);
	if (!animated->Link(node)) SAFE_DELETE(animated);
	return animated;
}

void FCDAnimatedCustom::Resize(size_t count, const char** _qualifiers, bool prependDot)
{
	values.resize(count, &dummy);
	qualifiers.resize(count);
	curves.resize(count);

	for (size_t i = 0; i < count && _qualifiers != NULL && *_qualifiers != 0; ++i)
	{
		qualifiers[i] = (prependDot ? string(".") : string("")) + *(_qualifiers++);
	}
}

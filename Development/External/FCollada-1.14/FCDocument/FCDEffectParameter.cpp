/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDAnimated.h"
#include "FCDocument/FCDEffectPass.h"
#include "FCDocument/FCDEffectProfile.h"
#include "FCDocument/FCDEffectTechnique.h"
#include "FCDocument/FCDEffectParameter.h"
#include "FCDocument/FCDEffectParameterFactory.h"
#include "FCDocument/FCDImage.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

//
// FCDEffectParameter
//

ImplementObjectType(FCDEffectParameter);

FCDEffectParameter::FCDEffectParameter(FCDocument* document)
:	FCDObject(document)
,	isGenerator(true)
,	reference(""), semantic("")
{
}

FCDEffectParameter::~FCDEffectParameter()
{
	CLEAR_POINTER_VECTOR(annotations);
}

FCDEffectParameterAnnotation* FCDEffectParameter::AddAnnotation()
{
	FCDEffectParameterAnnotation* annotation = new FCDEffectParameterAnnotation();
	annotations.push_back(annotation);
	SetDirtyFlag();
	return annotation;
}

void FCDEffectParameter::AddAnnotation(const fchar* name, FCDEffectParameter::Type type, const fchar* value)
{
	FCDEffectParameterAnnotation* annotation = AddAnnotation();
	annotation->name = name;
	annotation->type = type;
	annotation->value = value;
	SetDirtyFlag();
}

bool FCDEffectParameter::IsValueEqual( FCDEffectParameter *parameter )
{
	return ( parameter != NULL && this->GetType() == parameter->GetType() );
}

void FCDEffectParameter::Link(FCDEffectParameterList& UNUSED(parameters))
{
}

FUStatus FCDEffectParameter::LoadFromXML(xmlNode* parameterNode)
{
	FUStatus status;

	// Retrieves the annotations
	xmlNodeList annotationNodes;
	FindChildrenByType(parameterNode, DAE_ANNOTATE_ELEMENT, annotationNodes);
	for (xmlNodeList::iterator itN = annotationNodes.begin(); itN != annotationNodes.end(); ++itN)
	{
		xmlNode* annotateNode = (*itN);
		FCDEffectParameterAnnotation* annotation = AddAnnotation();
		annotation->name = TO_FSTRING(ReadNodeProperty(annotateNode, DAE_NAME_ATTRIBUTE));

		for (xmlNode* valueNode = annotateNode->children; valueNode != NULL; valueNode = valueNode->next)
		{
			if (valueNode->type != XML_ELEMENT_NODE) continue;
			if (IsEquivalent(valueNode->name, DAE_FXCMN_STRING_ELEMENT)) { annotation->type = STRING; annotation->value = TO_FSTRING(ReadNodeContentFull(valueNode)); }
			else if (IsEquivalent(valueNode->name, DAE_FXCMN_BOOL_ELEMENT)) { annotation->type = BOOLEAN; annotation->value = TO_FSTRING(ReadNodeContentDirect(valueNode)); }
			else if (IsEquivalent(valueNode->name, DAE_FXCMN_INT_ELEMENT)) { annotation->type = INTEGER; annotation->value = TO_FSTRING(ReadNodeContentDirect(valueNode)); }
			else if (IsEquivalent(valueNode->name, DAE_FXCMN_FLOAT_ELEMENT)) { annotation->type = FLOAT; annotation->value = TO_FSTRING(ReadNodeContentDirect(valueNode)); }
			else { status.Warning(FS("Annotation has not supported type: '") + TO_FSTRING((const char*) valueNode->name) + FC("'"), valueNode->line); }
			break;
		}
	}

	// This parameter is a generator if this is a <newparam> element. Otherwise, it modifies
	// an existing parameter (<bind>, <bind_semantic> or <setparam>.
	isGenerator = IsEquivalent(parameterNode->name, DAE_FXCMN_NEWPARAM_ELEMENT);
	if (isGenerator)
	{
		reference = ReadNodeProperty(parameterNode, DAE_SID_ATTRIBUTE);
		if (reference.empty())
		{
			return status.Warning(FS("No reference attribute on generator parameter."), parameterNode->line);
		}
	}
	else
	{
		reference = ReadNodeProperty(parameterNode, DAE_REF_ATTRIBUTE);
		if (reference.empty())
		{
			return status.Warning(FS("No reference attribute on modifier parameter."), parameterNode->line);
		}
	}
	
	xmlNode* valueNode = FindChildByType(parameterNode, DAE_FXCMN_SEMANTIC_ELEMENT);
	if (valueNode != NULL)
	{
		semantic = ReadNodeContentFull(valueNode);
	}

	SetDirtyFlag();
	return status;
}

// Write out this ColladaFX parameter to the XML node tree
xmlNode* FCDEffectParameter::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* parameterNode;
	if (isGenerator)
	{
		parameterNode = AddChild(parentNode, DAE_FXCMN_NEWPARAM_ELEMENT);
		AddAttribute(parameterNode, DAE_SID_ATTRIBUTE, reference);
	}
	else
	{
		parameterNode = AddChild(parentNode, DAE_FXCMN_SETPARAM_ELEMENT);
		AddAttribute(parameterNode, DAE_REF_ATTRIBUTE, reference);
	}

	// Write out the annotations
	for (FCDEffectParameterAnnotationList::const_iterator itA = annotations.begin(); itA != annotations.end(); ++itA)
	{
		xmlNode* annotateNode = AddChild(parameterNode, DAE_ANNOTATE_ELEMENT);
		AddAttribute(annotateNode, DAE_NAME_ATTRIBUTE, (*itA)->name);
		switch ((*itA)->type)
		{
		case BOOLEAN: AddChild(annotateNode, DAE_FXCMN_BOOL_ELEMENT, (*itA)->value); break;
		case STRING: AddChild(annotateNode, DAE_FXCMN_STRING_ELEMENT, (*itA)->value); break;
		case INTEGER: AddChild(annotateNode, DAE_FXCMN_INT_ELEMENT, (*itA)->value); break;
		case FLOAT: AddChild(annotateNode, DAE_FXCMN_FLOAT_ELEMENT, (*itA)->value); break;
		default: break;
		}
	}

	// Write out the semantic
	if (isGenerator && !semantic.empty())
	{
		AddChild(parameterNode, DAE_FXCMN_SEMANTIC_ELEMENT, semantic);
	}

	return parameterNode;
}

// Clones the base parameter values
FCDEffectParameter* FCDEffectParameter::Clone(FCDEffectParameter* clone) const
{
	if (clone == NULL)
	{
		// Recursively call the cloning function in an attempt to clone the up-class parameters
		clone = FCDEffectParameterFactory::Create(const_cast<FCDocument*>(GetDocument()), GetType());
		return clone != NULL ? Clone(clone) : NULL;
	}
	else
	{
		clone->reference = reference;
		clone->semantic = semantic;
		clone->isGenerator = isGenerator;
		clone->annotations.reserve(annotations.size());
		for (FCDEffectParameterAnnotationList::const_iterator itA = annotations.begin(); itA != annotations.end(); ++itA)
		{
			clone->AddAnnotation((*itA)->name, (*itA)->type, (*itA)->value);
		}
		return clone;
	}
}

// Flattening: overwrite the target parameter with this parameter
void FCDEffectParameter::Overwrite(FCDEffectParameter* UNUSED(target))
{
	// Do nothing on the base class, only values and animations should be overwritten
}

//
// FCDEffectParameterInt
//

ImplementObjectType(FCDEffectParameterInt);

FCDEffectParameterInt::FCDEffectParameterInt(FCDocument* document) : FCDEffectParameter(document) { value = 0; }
FCDEffectParameterInt::~FCDEffectParameterInt()
{
	value = 0;
}

// compare value
bool FCDEffectParameterInt::IsValueEqual( FCDEffectParameter *parameter )
{
	if( !FCDEffectParameter::IsValueEqual( parameter ) ) return false;
	FCDEffectParameterInt *param = (FCDEffectParameterInt*)parameter;
	
	if( value != param->GetValue() ) return false;

	return true;
}

// Clone
FCDEffectParameter* FCDEffectParameterInt::Clone(FCDEffectParameter* _clone) const
{
	FCDEffectParameterInt* clone = NULL;
	if (_clone == NULL) _clone = clone = new FCDEffectParameterInt(const_cast<FCDocument*>(GetDocument()));
	else if (_clone->GetObjectType() == FCDEffectParameterInt::GetClassType()) clone = (FCDEffectParameterInt*) _clone;

	if (_clone != NULL) FCDEffectParameter::Clone(_clone);
	if (clone != NULL)
	{
		clone->value = value;
	}
	return _clone;
}

// Flattening: overwrite the target parameter with this parameter
void FCDEffectParameterInt::Overwrite(FCDEffectParameter* target)
{
	if (target->GetType() == INTEGER)
	{
		FCDEffectParameterInt* s = (FCDEffectParameterInt*) target;
		s->value = value;
		SetDirtyFlag();
	}
}

// Parse in this ColladaFX integer from the document's XML node
FUStatus FCDEffectParameterInt::LoadFromXML(xmlNode* parameterNode)
{
	FUStatus status = FCDEffectParameter::LoadFromXML(parameterNode);
	xmlNode* valueNode = FindChildByType(parameterNode, DAE_FXCMN_INT_ELEMENT);
	const char* valueString = ReadNodeContentDirect(valueNode);
	if (valueString == NULL || *valueString == 0)
	{
		return status.Fail(FS("Bad value for float parameter in integer parameter: ") + TO_FSTRING(GetReference()), parameterNode->line);
	}
	value = FUStringConversion::ToInt32(valueString);
	SetDirtyFlag();
	return status;
}

xmlNode* FCDEffectParameterInt::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* parameterNode = FCDEffectParameter::WriteToXML(parentNode);
	AddChild(parameterNode, DAE_FXCMN_INT_ELEMENT, value);
	return parameterNode;
}

//
// FCDEffectParameterBool
//

ImplementObjectType(FCDEffectParameterBool);

// boolean type parameter
FCDEffectParameterBool::FCDEffectParameterBool(FCDocument* document) : FCDEffectParameter(document)
{
	value = 0;
}

FCDEffectParameterBool::~FCDEffectParameterBool() {}

// compare value
bool FCDEffectParameterBool::IsValueEqual( FCDEffectParameter *parameter )
{
	if( !FCDEffectParameter::IsValueEqual( parameter ) ) return false;
	FCDEffectParameterBool *param = (FCDEffectParameterBool*)parameter;
	
	if( value != param->GetValue() ) return false;

	return true;
}

// Clone
FCDEffectParameter* FCDEffectParameterBool::Clone(FCDEffectParameter* _clone) const
{
	FCDEffectParameterBool* clone = NULL;
	if (_clone == NULL) _clone = clone = new FCDEffectParameterBool(const_cast<FCDocument*>(GetDocument()));
	else if (_clone->GetObjectType() == FCDEffectParameterBool::GetClassType()) clone = (FCDEffectParameterBool*) _clone;

	if (_clone != NULL) FCDEffectParameter::Clone(_clone);
	if (clone != NULL)
	{
		clone->value = value;
	}
	return _clone;
}

// Flattening: overwrite the target parameter with this parameter
void FCDEffectParameterBool::Overwrite(FCDEffectParameter* target)
{
	if (target->GetType() == BOOLEAN)
	{
		FCDEffectParameterBool* s = (FCDEffectParameterBool*) target;
		s->value = value;
		SetDirtyFlag();
	}
}

FUStatus FCDEffectParameterBool::LoadFromXML(xmlNode* parameterNode)
{
	FUStatus status = FCDEffectParameter::LoadFromXML(parameterNode);
	xmlNode* valueNode = FindChildByType(parameterNode, DAE_FXCMN_BOOL_ELEMENT);
	const char* valueString = ReadNodeContentDirect(valueNode);
	if (valueString == NULL || *valueString == 0)
	{
		return status.Fail(FS("Bad value for boolean parameter in effect: ") + TO_FSTRING(GetReference()), parameterNode->line);
	}
	value = FUStringConversion::ToBoolean(valueString);
	SetDirtyFlag();
	return status;
}

xmlNode* FCDEffectParameterBool::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* parameterNode = FCDEffectParameter::WriteToXML(parentNode);
	AddChild(parameterNode, DAE_FXCMN_BOOL_ELEMENT, value);
	return parameterNode;
}

//
// FCDEffectParameterString
//

ImplementObjectType(FCDEffectParameterString);

// string type parameter
FCDEffectParameterString::FCDEffectParameterString(FCDocument* document) : FCDEffectParameter(document)
{
	value = "";
}

FCDEffectParameterString::~FCDEffectParameterString()
{
}

// compare value
bool FCDEffectParameterString::IsValueEqual( FCDEffectParameter *parameter )
{
	if( !FCDEffectParameter::IsValueEqual( parameter ) ) return false;
	FCDEffectParameterString *param = (FCDEffectParameterString*)parameter;
	
	if( value != param->GetValue() ) return false;

	return true;
}

// Clone
FCDEffectParameter* FCDEffectParameterString::Clone(FCDEffectParameter* _clone) const
{
	FCDEffectParameterString* clone = NULL;
	if (_clone == NULL) _clone = clone = new FCDEffectParameterString(const_cast<FCDocument*>(GetDocument()));
	else if (_clone->GetObjectType() == FCDEffectParameterString::GetClassType()) clone = (FCDEffectParameterString*) _clone;

	if (_clone != NULL) FCDEffectParameter::Clone(_clone);
	if (clone != NULL)
	{
		clone->value = value;
	}
	return _clone;
}

// Flattening: overwrite the target parameter with this parameter
void FCDEffectParameterString::Overwrite(FCDEffectParameter* target)
{
	if (target->GetType() == STRING)
	{
		FCDEffectParameterString* s = (FCDEffectParameterString*) target;
		s->value = value;
		SetDirtyFlag();
	}
}

FUStatus FCDEffectParameterString::LoadFromXML(xmlNode* parameterNode)
{
	FUStatus status = FCDEffectParameter::LoadFromXML(parameterNode);
	xmlNode* valueNode = FindChildByType(parameterNode, DAE_FXCMN_STRING_ELEMENT);
	value = ReadNodeContentFull(valueNode);
	SetDirtyFlag();
	return status;
}

xmlNode* FCDEffectParameterString::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* parameterNode = FCDEffectParameter::WriteToXML(parentNode);
	AddChild(parameterNode, DAE_FXCMN_STRING_ELEMENT, value);
	return parameterNode;
}

//
// FCDEffectParameterFloat
//

ImplementObjectType(FCDEffectParameterFloat);

// float type parameter
FCDEffectParameterFloat::FCDEffectParameterFloat(FCDocument* document) : FCDEffectParameter(document)
{
	floatType = FLOAT;
	value = 0.0f;
}

FCDEffectParameterFloat::~FCDEffectParameterFloat()
{
}

// compare value
bool FCDEffectParameterFloat::IsValueEqual( FCDEffectParameter *parameter )
{
	if( !FCDEffectParameter::IsValueEqual( parameter ) ) return false;
	FCDEffectParameterFloat *param = (FCDEffectParameterFloat*)parameter;
	
	if( floatType != param->GetFloatType() ) return false;
	if( value != param->GetValue() ) return false;

	return true;
}

// Clone
FCDEffectParameter* FCDEffectParameterFloat::Clone(FCDEffectParameter* _clone) const
{
	FCDEffectParameterFloat* clone = NULL;
	if (_clone == NULL) _clone = clone = new FCDEffectParameterFloat(const_cast<FCDocument*>(GetDocument()));
	else if (_clone->GetObjectType() == FCDEffectParameterFloat::GetClassType()) clone = (FCDEffectParameterFloat*) _clone;

	if (_clone != NULL) FCDEffectParameter::Clone(_clone);
	if (clone != NULL)
	{
		clone->floatType = floatType;
		clone->value = value;
	}
	return _clone;
}

// Flattening: overwrite the target parameter with this parameter
void FCDEffectParameterFloat::Overwrite(FCDEffectParameter* target)
{
	if (target->GetType() == FCDEffectParameter::FLOAT)
	{
		FCDEffectParameterFloat* s = (FCDEffectParameterFloat*) target;
		if (s->floatType == floatType)
		{
			s->value = value;
			SetDirtyFlag();
		}
	}
}

FUStatus FCDEffectParameterFloat::LoadFromXML(xmlNode* parameterNode)
{
	FUStatus status = FCDEffectParameter::LoadFromXML(parameterNode);
	xmlNode* valueNode = FindChildByType(parameterNode, DAE_FXCMN_FLOAT_ELEMENT);
	if (valueNode == NULL)
	{
		valueNode = FindChildByType(parameterNode, DAE_FXCMN_HALF_ELEMENT);
		floatType = HALF;
	}
	else floatType = FLOAT;
		
	const char* valueString = ReadNodeContentDirect(valueNode);
	if (valueString == NULL || *valueString == 0)
	{
		return status.Fail(FS("Bad float value for float parameter: ") + TO_FSTRING(GetReference()), parameterNode->line);
	}
	value = FUStringConversion::ToFloat(valueString);
	
	FCDAnimatedFloat::Create(GetDocument(), parameterNode, &value);
	
	SetDirtyFlag();
	return status;
}

xmlNode* FCDEffectParameterFloat::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* parameterNode = FCDEffectParameter::WriteToXML(parentNode);
	AddChild(parameterNode, (floatType == FLOAT) ? DAE_FXCMN_FLOAT_ELEMENT : DAE_FXCMN_HALF_ELEMENT, value);
	const char* wantedSubId = GetReference().c_str();
	if (*wantedSubId == 0) wantedSubId = GetSemantic().c_str();
	if (*wantedSubId == 0) wantedSubId = "flt";
	GetDocument()->WriteAnimatedValueToXML(&value, parameterNode, wantedSubId);
	return parameterNode;
}

//
// FCDEffectParameterFloat2
//

ImplementObjectType(FCDEffectParameterFloat2);

// float2 type parameter
FCDEffectParameterFloat2::FCDEffectParameterFloat2(FCDocument* document) : FCDEffectParameter(document)
{
	floatType = FLOAT;
	value_x = 0.0f;
	value_y = 0.0f;
}

FCDEffectParameterFloat2::~FCDEffectParameterFloat2()
{
}

// compare value
bool FCDEffectParameterFloat2::IsValueEqual( FCDEffectParameter *parameter )
{
	if( !FCDEffectParameter::IsValueEqual( parameter ) ) return false;
	FCDEffectParameterFloat2 *param = (FCDEffectParameterFloat2*)parameter;
	
	if( floatType != param->GetFloatType() ) return false;
	if( value_x != param->GetValueX() ||
		value_y != param->GetValueY() ) return false;

	return true;
}

// Clone
FCDEffectParameter* FCDEffectParameterFloat2::Clone(FCDEffectParameter* _clone) const
{
	FCDEffectParameterFloat2* clone = NULL;
	if (_clone == NULL) _clone = clone = new FCDEffectParameterFloat2(const_cast<FCDocument*>(GetDocument()));
	else if (_clone->GetObjectType() == FCDEffectParameterFloat2::GetClassType()) clone = (FCDEffectParameterFloat2*) _clone;

	if (_clone != NULL) FCDEffectParameter::Clone(_clone);
	if (clone != NULL)
	{
		clone->floatType = floatType;
		clone->value_x = value_x;
		clone->value_y = value_y;
	}
	return _clone;
}

// Flattening: overwrite the target parameter with this parameter
void FCDEffectParameterFloat2::Overwrite(FCDEffectParameter* target)
{
	if (target->GetType() == FLOAT2)
	{
		FCDEffectParameterFloat2* s = (FCDEffectParameterFloat2*) target;
		if (s->floatType == floatType)
		{
			s->value_x = value_x;
			s->value_y = value_y;
			SetDirtyFlag();
		}
	}
}

FUStatus FCDEffectParameterFloat2::LoadFromXML(xmlNode* parameterNode)
{
	FUStatus status = FCDEffectParameter::LoadFromXML(parameterNode);
	xmlNode* valueNode = FindChildByType(parameterNode, DAE_FXCMN_FLOAT2_ELEMENT);
	if (valueNode == NULL)
	{
		valueNode = FindChildByType(parameterNode, DAE_FXCMN_HALF2_ELEMENT);
		floatType = HALF;
	}
	else floatType = FLOAT;
		
	const char* valueString = ReadNodeContentDirect(valueNode);
	if (valueString == NULL || *valueString == 0)
	{
		return status.Fail(FS("Bad value for float2 parameter: ") + TO_FSTRING(GetReference()), parameterNode->line);
	}
	value_x = FUStringConversion::ToFloat(&valueString);
	value_y = FUStringConversion::ToFloat(&valueString);
	SetDirtyFlag();
	return status;
}

xmlNode* FCDEffectParameterFloat2::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* parameterNode = FCDEffectParameter::WriteToXML(parentNode);
	globalSBuilder.set(value_x); globalSBuilder.append(' '); globalSBuilder.append(value_y);
	AddChild(parameterNode, (floatType == FLOAT) ? DAE_FXCMN_FLOAT2_ELEMENT : DAE_FXCMN_HALF2_ELEMENT, globalSBuilder);
	return parameterNode;
}

//
// FCDEffectParameterFloat3
//

ImplementObjectType(FCDEffectParameterFloat3);

// float3 type parameter
FCDEffectParameterFloat3::FCDEffectParameterFloat3(FCDocument* document) : FCDEffectParameter(document)
{
	floatType = FLOAT;
	value = FMVector3(0.0f, 0.0f, 0.0f);
}

FCDEffectParameterFloat3::~FCDEffectParameterFloat3()
{
}

// compare value
bool FCDEffectParameterFloat3::IsValueEqual( FCDEffectParameter *parameter )
{
	if( !FCDEffectParameter::IsValueEqual( parameter ) ) return false;
	FCDEffectParameterFloat3 *param = (FCDEffectParameterFloat3*)parameter;
	
	if( floatType != param->GetFloatType() ) return false;
	if( value != param->GetValue() ) return false;

	return true;
}

// Clone
FCDEffectParameter* FCDEffectParameterFloat3::Clone(FCDEffectParameter* _clone) const
{
	FCDEffectParameterFloat3* clone = NULL;
	if (_clone == NULL) _clone = clone = new FCDEffectParameterFloat3(const_cast<FCDocument*>(GetDocument()));
	else if (_clone->GetObjectType() == FCDEffectParameterFloat3::GetClassType()) clone = (FCDEffectParameterFloat3*) _clone;

	if (_clone != NULL) FCDEffectParameter::Clone(_clone);
	if (clone != NULL)
	{
		clone->floatType = floatType;
		clone->value = value;
	}
	return _clone;
}

// Flattening: overwrite the target parameter with this parameter
void FCDEffectParameterFloat3::Overwrite(FCDEffectParameter* target)
{
	if (target->GetType() == FLOAT3)
	{
		FCDEffectParameterFloat3* s = (FCDEffectParameterFloat3*) target;
		if (s->floatType == floatType)
		{
			s->value = value;
			SetDirtyFlag();
		}
	}
}

FUStatus FCDEffectParameterFloat3::LoadFromXML(xmlNode* parameterNode)
{
	FUStatus status = FCDEffectParameter::LoadFromXML(parameterNode);
	xmlNode* valueNode = FindChildByType(parameterNode, DAE_FXCMN_FLOAT3_ELEMENT);
	if (valueNode == NULL)
	{
		valueNode = FindChildByType(parameterNode, DAE_FXCMN_HALF3_ELEMENT);
		floatType = HALF;
	}
	else floatType = FLOAT;
		
	const char* valueString = ReadNodeContentDirect(valueNode);
	if (valueString == NULL || *valueString == 0)
	{
		return status.Fail(FS("Bad value for float3 parameter: ") + TO_FSTRING(GetReference()), parameterNode->line);
	}
	value = FUStringConversion::ToPoint(valueString);
	FCDAnimatedColor::Create(GetDocument(), parameterNode, &value);
	
	SetDirtyFlag();
	return status;
}

xmlNode* FCDEffectParameterFloat3::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* parameterNode = FCDEffectParameter::WriteToXML(parentNode);
	string s = FUStringConversion::ToString(value);
	AddChild(parameterNode, (floatType == FLOAT) ? DAE_FXCMN_FLOAT3_ELEMENT : DAE_FXCMN_HALF3_ELEMENT, s);
	const char* wantedSubId = GetReference().c_str();
	if (*wantedSubId == 0) wantedSubId = GetSemantic().c_str();
	if (*wantedSubId == 0) wantedSubId = "flt3";
	GetDocument()->WriteAnimatedValueToXML(&value.x, parameterNode, wantedSubId);
	return parameterNode;
}

//
// FCDEffectParameterVector
//

ImplementObjectType(FCDEffectParameterVector);

FCDEffectParameterVector::FCDEffectParameterVector(FCDocument* document) : FCDEffectParameter(document), 
	value(0.0f,0.0f,0.0f,0.0f)
{
	floatType = FLOAT;
}

FCDEffectParameterVector::~FCDEffectParameterVector() {}

// compare value
bool FCDEffectParameterVector::IsValueEqual( FCDEffectParameter *parameter )
{
	if( !FCDEffectParameter::IsValueEqual( parameter ) ) return false;
	FCDEffectParameterVector *param = (FCDEffectParameterVector*)parameter;
	
	if( floatType != param->GetFloatType() ) return false;
	if( !(value.x == param->GetValueX() &&
		value.y == param->GetValueY() &&
		value.z == param->GetValueZ() &&
		value.w == param->GetValueW() ) ) return false;

	return true;
}

// Clone
FCDEffectParameter* FCDEffectParameterVector::Clone(FCDEffectParameter* _clone) const
{
	FCDEffectParameterVector* clone = NULL;
	if (_clone == NULL) _clone = clone = new FCDEffectParameterVector(const_cast<FCDocument*>(GetDocument()));
	else if (_clone->GetObjectType() == FCDEffectParameterVector::GetClassType()) clone = (FCDEffectParameterVector*) _clone;

	if (_clone != NULL) FCDEffectParameter::Clone(_clone);
	if (clone != NULL)
	{
		clone->floatType = floatType;
		clone->value = value;
	}
	return _clone;
}

// Flattening: overwrite the target parameter with this parameter
void FCDEffectParameterVector::Overwrite(FCDEffectParameter* target)
{
	if (target->GetType() == VECTOR)
	{
		FCDEffectParameterVector* s = (FCDEffectParameterVector*) target;
		if (s->floatType == floatType)
		{
			s->value = value;
			SetDirtyFlag();
		}
	}
}

FUStatus FCDEffectParameterVector::LoadFromXML(xmlNode* parameterNode)
{
	FUStatus status = FCDEffectParameter::LoadFromXML(parameterNode);
	xmlNode* valueNode = FindChildByType(parameterNode, DAE_FXCMN_FLOAT4_ELEMENT);
	if (valueNode == NULL)
	{
		valueNode = FindChildByType(parameterNode, DAE_FXCMN_HALF4_ELEMENT);
		floatType = HALF;
	}
	else floatType = FLOAT;
		
	const char* valueString = ReadNodeContentDirect(valueNode);
	if (valueString == NULL || *valueString == 0)
	{
		return status.Fail(FS("Bad value for float4 parameter: ") + TO_FSTRING(GetReference()), parameterNode->line);
	}
	value.x = FUStringConversion::ToFloat(&valueString);
	value.y = FUStringConversion::ToFloat(&valueString);
	value.z = FUStringConversion::ToFloat(&valueString);
	value.w = FUStringConversion::ToFloat(&valueString);
	FCDAnimatedColor::Create(GetDocument(), parameterNode, (FMVector3*)(float*)value);

	SetDirtyFlag();
	return status;
}

xmlNode* FCDEffectParameterVector::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* parameterNode = FCDEffectParameter::WriteToXML(parentNode);
	globalSBuilder.set(value.x); globalSBuilder.append(' '); globalSBuilder.append(value.y); globalSBuilder.append(' ');
	globalSBuilder.append(value.z); globalSBuilder.append(' '); globalSBuilder.append(value.w);
	AddChild(parameterNode, (floatType == FLOAT) ? DAE_FXCMN_FLOAT4_ELEMENT : DAE_FXCMN_HALF4_ELEMENT, globalSBuilder);
	const char* wantedSubId = GetReference().c_str();
	if (*wantedSubId == 0) wantedSubId = GetSemantic().c_str();
	if (*wantedSubId == 0) wantedSubId = "flt4";
	GetDocument()->WriteAnimatedValueToXML(&value.x, parameterNode, wantedSubId);
	return parameterNode;
}

//
// FCDEffectParameterMatrix
//

ImplementObjectType(FCDEffectParameterMatrix);

FCDEffectParameterMatrix::FCDEffectParameterMatrix(FCDocument* document) : FCDEffectParameter(document)
{
	floatType = FLOAT;
	matrix = FMMatrix44::Identity;
}

FCDEffectParameterMatrix::~FCDEffectParameterMatrix()
{
}

// compare value
bool FCDEffectParameterMatrix::IsValueEqual( FCDEffectParameter *parameter )
{
	if( !FCDEffectParameter::IsValueEqual( parameter ) ) return false;
	FCDEffectParameterMatrix *param = (FCDEffectParameterMatrix*)parameter;
	
	if( floatType != param->GetFloatType() ) return false;
	// Those will be optimized by the compiler
	for( int i = 0; i < 4; i++ )
	{
		for( int j = 0; j < 4; j++ )
		{
			if( matrix[i][j] != param->GetMatrix()[i][j] ) return false;
		}
	}

	return true;
}

// Clone
FCDEffectParameter* FCDEffectParameterMatrix::Clone(FCDEffectParameter* _clone) const
{
	FCDEffectParameterMatrix* clone = NULL;
	if (_clone == NULL) _clone = clone = new FCDEffectParameterMatrix(const_cast<FCDocument*>(GetDocument()));
	else if (_clone->GetObjectType() == FCDEffectParameterMatrix::GetClassType()) clone = (FCDEffectParameterMatrix*) _clone;

	if (_clone != NULL) FCDEffectParameter::Clone(_clone);
	if (clone != NULL)
	{
		clone->floatType = floatType;
		clone->matrix = matrix;
	}
	return _clone;
}

// Flattening: overwrite the target parameter with this parameter
void FCDEffectParameterMatrix::Overwrite(FCDEffectParameter* target)
{
	if (target->GetType() == MATRIX)
	{
		FCDEffectParameterMatrix* s = (FCDEffectParameterMatrix*) target;
		s->matrix = matrix;
		SetDirtyFlag();
	}
}

FUStatus FCDEffectParameterMatrix::LoadFromXML(xmlNode* parameterNode)
{
	FUStatus status = FCDEffectParameter::LoadFromXML(parameterNode);
	xmlNode* valueNode = FindChildByType(parameterNode, DAE_FXCMN_FLOAT4X4_ELEMENT);
	if (valueNode == NULL)
	{
		valueNode = FindChildByType(parameterNode, DAE_FXCMN_HALF4X4_ELEMENT);
		floatType = HALF;
	}
	else floatType = FLOAT;
		
	const char* valueString = ReadNodeContentDirect(valueNode);
	if (valueString == NULL || *valueString == 0)
	{
		return status.Fail(FS("Bad value for matrix parameter: ") + TO_FSTRING(GetReference()), parameterNode->line);
	}
	FUStringConversion::ToMatrix(valueString, matrix, GetDocument()->GetLengthUnitConversion());
	SetDirtyFlag();
	return status;
}

xmlNode* FCDEffectParameterMatrix::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* parameterNode = FCDEffectParameter::WriteToXML(parentNode);
	string s = FUStringConversion::ToString(matrix, GetDocument()->GetLengthUnitConversion()); 
	AddChild(parameterNode, (floatType == FLOAT) ? DAE_FXCMN_FLOAT4X4_ELEMENT : DAE_FXCMN_HALF4X4_ELEMENT, s);
	return parameterNode;
}

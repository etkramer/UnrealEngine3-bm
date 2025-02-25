/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FUtils/FUDaeSyntax.h"

template <class T>
FCDPhysicsParameter<T>::FCDPhysicsParameter(FCDocument* document, const string& ref) : FCDPhysicsParameterGeneric(document, ref)
{
	value = NULL;
}

template <class T>
FCDPhysicsParameter<T>::~FCDPhysicsParameter()
{
	SAFE_DELETE(value);
}

// Clone
template <class T>
FCDPhysicsParameterGeneric* FCDPhysicsParameter<T>::Clone()
{
	FCDPhysicsParameterGeneric *clone = new FCDPhysicsParameter<T>(GetDocument(), reference);
	((FCDPhysicsParameter<T>*)clone)->value = new T(*value);
	return clone;
}

template <class T>
void FCDPhysicsParameter<T>::SetValue(T val)
{
	SAFE_DELETE(value);
	value = new T();
	*value = val;
	SetDirtyFlag();
}


template <class T>
void FCDPhysicsParameter<T>::SetValue(T* val)
{
	SAFE_DELETE(value);
	value = val;
	SetDirtyFlag();
}

// Flattening: overwrite the target parameter with this parameter
template <class T>
void FCDPhysicsParameter<T>::Overwrite(FCDPhysicsParameterGeneric* target)
{
	((FCDPhysicsParameter<T>*) target)->SetValue(value);
}

// Write out this ColladaFX parameter to the XML node tree
template <class T>
xmlNode* FCDPhysicsParameter<T>::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* parameterNode = FUXmlWriter::AddChild(parentNode, reference.c_str());
	AddContent(parameterNode, FUStringConversion::ToString(*value));
	//the translate and rotate elements need to be in a <mass_frame> tag.
	if ((reference == DAE_TRANSLATE_ELEMENT) || 
			(reference == DAE_ROTATE_ELEMENT))
	{
		xmlNode* massFrameNode = 
				AddChild(parentNode, DAE_MASS_FRAME_ELEMENT);
		ReParentNode(parameterNode, massFrameNode);
	}
	return parameterNode;
}

/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDExtra.h"
#include "FCDocument/FCDGeometry.h"
#include "FCDocument/FCDGeometrySource.h"
#include "FCDocument/FCDGeometrySpline.h"
#include "FUtils/FUStringConversion.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

//
// FCDSpline
//

ImplementObjectType(FCDSpline);

FCDSpline::FCDSpline(FCDocument* document)
:	FCDObject(document)
{
	form = FUDaeSplineForm::OPEN;
}

FCDSpline::~FCDSpline()
{
	cvs.clear();
}

//
// FCDBezierSpline
//

ImplementObjectType(FCDBezierSpline);

FCDBezierSpline::FCDBezierSpline(FCDocument* document)
:	FCDSpline(document)
{
}

FCDBezierSpline::~FCDBezierSpline()
{
}

void FCDBezierSpline::ToNURBs( FCDNURBSSplineList &toFill ) const
{
	// calculate the number of nurb segments
	bool closed = IsClosed();
	int cvsCount = (int)cvs.size();
	int nurbCount = (!closed) ? ((cvsCount - 1) / 3) : (cvsCount / 3);

	// if the spline is closed, ignore the first CV as it is the in-tangent of the first knot
	size_t curCV = (!closed) ? 0 : 1;
	int lastNurb = (!closed) ? nurbCount : (nurbCount - 1);

	for( int i = 0; i < lastNurb; i++ )
	{
		FCDNURBSSpline* nurb = new FCDNURBSSpline(const_cast<FCDocument*>(GetDocument()));
		nurb->SetDegree(3);

		// add (degree + 1) CVs to the nurb
		for( size_t i = 0; i < (3+1); i++ )
		{
			nurb->AddCV( cvs[curCV++], 1.0f );
		}

		// the last CVs will be the starting point of the next nurb segment
		curCV--;

		// add (degree+1) knots on each side of the knot vector
		for( size_t i = 0; i < (3+1); i++ ) nurb->AddKnot( 0.0f );
		for( size_t i = 0; i < (3+1); i++ ) nurb->AddKnot( 1.0f );

		// nurbs created this way are never closed
		nurb->SetClosed( false );

		// add the nurb
		toFill.push_back( nurb );
	}

	if( closed )
	{
		// we still have one NURB to create
		FCDNURBSSpline* nurb = new FCDNURBSSpline(const_cast<FCDocument*>(GetDocument()));
		nurb->SetDegree(3);

		nurb->AddCV( cvs[cvsCount - 2], 1.0f ); // the last knot position
		nurb->AddCV( cvs[cvsCount - 1], 1.0f ); // the last knot out-tangent
		nurb->AddCV( cvs[0			 ], 1.0f ); // the first knot in-tangent
		nurb->AddCV( cvs[1			 ], 1.0f ); // the first knot position

		// add (degree+1) knots on each side of the knot vector
		for( size_t i = 0; i < (3+1); i++ ) nurb->AddKnot( 0.0f );
		for( size_t i = 0; i < (3+1); i++ ) nurb->AddKnot( 1.0f );

		// nurbs created this way are never closed
		nurb->SetClosed( false );

		// add the nurb
		toFill.push_back( nurb );
	}
}

bool FCDBezierSpline::IsValid(FUStatus* status) const
{
	FUStatus s;
	if( cvs.size() == 0 )
	{
		s.Warning(FS("No control vertice input in spline."));
	}

	if( status != NULL ) status->AppendStatus(s);
	return s.IsFailure();
}

FUStatus FCDBezierSpline::LoadFromXML(xmlNode* splineNode)
{
	FUStatus status;

	// Read the curve closed attribute
	SetClosed( FUStringConversion::ToBoolean( ReadNodeProperty(splineNode, DAE_CLOSED_ATTRIBUTE) ) );

	// Read in the <control_vertices> element, which define the base type for this curve
	xmlNode* controlVerticesNode = FindChildByType(splineNode, DAE_CONTROL_VERTICES_ELEMENT);
	if (controlVerticesNode == NULL)
	{
		return status.Warning(FS("No <control_vertices> element in spline."), splineNode->line);
	}

	// read the sources
	xmlNodeList inputElements;
	FindChildrenByType( controlVerticesNode, DAE_INPUT_ELEMENT, inputElements );

	for( size_t i = 0; i < inputElements.size(); i++ )
	{
		xmlNode *inputNode = inputElements[i];
		string sourceId = ReadNodeProperty( inputNode, DAE_SOURCE_ATTRIBUTE ).substr(1);
		if( sourceId.empty() ) { status.Fail(); return status; }
		xmlNode *sourceNode = FindChildById( splineNode, sourceId );
		if( sourceNode == NULL ) { status.Fail(); return status; }

		if( IsEquivalent( ReadNodeProperty( inputNode, DAE_SEMANTIC_ATTRIBUTE ), DAE_CVS_SPLINE_INPUT ) )
		{
			ReadSource( sourceNode, cvs );
		}
		else
		{
			status.Fail(); return status;
		}
	}

	IsValid(&status);

	return status;
}

xmlNode* FCDBezierSpline::WriteToXML(xmlNode* parentNode, const string& parentId, const string& splineId) const
{
	xmlNode* splineNode = AddChild(parentNode, DAE_SPLINE_ELEMENT);

	// set the "closed" attribute
	AddAttribute(splineNode, DAE_CLOSED_ATTRIBUTE, IsClosed() );

	// add CVs
	FUSStringBuilder controlPointSourceId(parentId); controlPointSourceId += "-cvs-" + splineId;
	AddSourcePosition(splineNode, controlPointSourceId.ToCharPtr(), cvs );

	// add the <control_vertices> element
	xmlNode* verticesNode = AddChild(splineNode, DAE_CONTROL_VERTICES_ELEMENT);
	
	// add inputs
	AddInput(verticesNode, controlPointSourceId.ToCharPtr(), DAE_CVS_SPLINE_INPUT);

	// create the <extra> FCOLLADA node
	xmlNode *extraNode = AddExtraTechniqueChild(splineNode,DAE_FCOLLADA_PROFILE);
	// add the type attribute
	AddChild( extraNode, DAE_TYPE_ATTRIBUTE, FUDaeSplineType::ToString(GetSplineType()) );

	return splineNode;
}

//
// FCDNURBSSpline
//

ImplementObjectType(FCDNURBSSpline);

FCDNURBSSpline::FCDNURBSSpline(FCDocument* document)
:	FCDSpline(document)
{
}

FCDNURBSSpline::~FCDNURBSSpline()
{
	weights.clear();
	knots.clear();
}

bool FCDNURBSSpline::AddCV( const FMVector3& cv, float weight )
{ 
	if( weight < 0.0f ) return false;

	cvs.push_back(cv);
	weights.push_back( weight );
	return true;
}

bool FCDNURBSSpline::IsValid(FUStatus* status) const
{
	FUStatus s;
	if( cvs.size() == 0 )
	{
		s.Warning(FS("No control vertice input in spline."));
	}

	if( cvs.size() != weights.size() )
	{
		s.Fail(FS("Numbers of CVs and weights are different in NURB spline."));
	}

	if( cvs.size() != knots.size() - degree - 1 )
	{
		s.Fail(FS("Invalid spline. Equation \"n = k - d - 1\" is not respected."));
	}

	if( status != NULL ) status->AppendStatus(s);
	return s.IsFailure();
}

FUStatus FCDNURBSSpline::LoadFromXML(xmlNode* splineNode)
{
	FUStatus status;

	xmlNode* extraNode = FindChildByType(splineNode, DAE_EXTRA_ELEMENT);
	if( extraNode == NULL ) { status.Fail(); return status; }
	xmlNode *fcolladaNode = FindTechnique( extraNode, DAE_FCOLLADA_PROFILE );
	if( fcolladaNode == NULL ) { status.Fail(); return status; }

	xmlNode *degreeNode = FindChildByType( fcolladaNode, DAE_DEGREE_ATTRIBUTE );
	// get the degree
	degree = 3;
	if( degreeNode != NULL )
	{
		degree = FUStringConversion::ToUInt32( ReadNodeContentFull(degreeNode) );
	}

	// Read the curve closed attribute
	SetClosed( FUStringConversion::ToBoolean( ReadNodeProperty(splineNode, DAE_CLOSED_ATTRIBUTE) ) );

	// Read in the <control_vertices> element, which define the base type for this curve
	xmlNode* controlVerticesNode = FindChildByType(splineNode, DAE_CONTROL_VERTICES_ELEMENT);
	if (controlVerticesNode == NULL)
	{
		return status.Warning(FS("No <control_vertices> element in spline."), splineNode->line);
	}

	// read the sources
	xmlNodeList inputElements;
	FindChildrenByType( controlVerticesNode, DAE_INPUT_ELEMENT, inputElements );

	for( size_t i = 0; i < inputElements.size(); i++ )
	{
		xmlNode *inputNode = inputElements[i];
		string sourceId = ReadNodeProperty( inputNode, DAE_SOURCE_ATTRIBUTE ).substr(1);
		if( sourceId.empty() ) { status.Fail(); return status; }
		xmlNode *sourceNode = FindChildById( splineNode, sourceId );
		if( sourceNode == NULL ) { status.Fail(); return status; }

		if( IsEquivalent( ReadNodeProperty( inputNode, DAE_SEMANTIC_ATTRIBUTE ), DAE_CVS_SPLINE_INPUT ) )
		{
			ReadSource( sourceNode, cvs );
		}
		else if( IsEquivalent( ReadNodeProperty( inputNode, DAE_SEMANTIC_ATTRIBUTE ), DAE_KNOT_SPLINE_INPUT ) )
		{
			ReadSource( sourceNode, knots );
		}
		else if( IsEquivalent( ReadNodeProperty( inputNode, DAE_SEMANTIC_ATTRIBUTE ), DAE_WEIGHT_SPLINE_INPUT ) )
		{
			ReadSource( sourceNode, weights );
		}
	}

	IsValid(&status);

	return status;
}

xmlNode* FCDNURBSSpline::WriteToXML(xmlNode* parentNode, const string& parentId, const string& splineId) const
{
	xmlNode* splineNode = AddChild(parentNode, DAE_SPLINE_ELEMENT);

	// set the "closed" attribute
	AddAttribute(splineNode, DAE_CLOSED_ATTRIBUTE, IsClosed() );

	// add CVs
	FUSStringBuilder controlPointSourceId(parentId); controlPointSourceId += "-cvs-" + splineId;
	AddSourcePosition(splineNode, controlPointSourceId.ToCharPtr(), cvs );
	
	// add weights
	FUSStringBuilder weightSourceId(parentId); weightSourceId += "-weights-" + splineId;
	AddSourceFloat(splineNode, weightSourceId.ToCharPtr(), weights, "WEIGHT");

	// add knots
	FUSStringBuilder knotSourceId(parentId); knotSourceId += "-knots-" + splineId;
	AddSourceFloat(splineNode, knotSourceId.ToCharPtr(), knots, "KNOT");

	// add the <control_vertices> element
	xmlNode* verticesNode = AddChild(splineNode, DAE_CONTROL_VERTICES_ELEMENT);
	
	// add inputs
	AddInput(verticesNode, controlPointSourceId.ToCharPtr(), DAE_CVS_SPLINE_INPUT);
	AddInput(verticesNode, weightSourceId.ToCharPtr(), DAE_WEIGHT_SPLINE_INPUT );
	AddInput(verticesNode, knotSourceId.ToCharPtr(), DAE_KNOT_SPLINE_INPUT);

	// create the <extra> FCOLLADA node
	xmlNode *extraNode = AddExtraTechniqueChild(splineNode,DAE_FCOLLADA_PROFILE);
	// add the type attribute
	AddChild( extraNode, DAE_TYPE_ATTRIBUTE, FUDaeSplineType::ToString(GetSplineType()) );
	// add the degree attribute
	AddChild( extraNode, DAE_DEGREE_ATTRIBUTE, FUStringConversion::ToString( degree ) );

	return splineNode;
}

//
// FCDGeometrySpline
//

ImplementObjectType(FCDGeometrySpline);

FCDGeometrySpline::FCDGeometrySpline(FCDocument* document, FCDGeometry* _parent, FUDaeSplineType::Type _type )
:	FCDObject(document)
{
	parent = _parent;
	type = _type;
}

FCDGeometrySpline::~FCDGeometrySpline()
{
	parent = NULL;
	// delete every spline
	for( size_t i = 0; i < splines.size(); i++ )
	{
		FCDSpline* spline = splines[i];
		delete spline;
		spline = NULL;
		splines[i] = spline;
	}
	// clear the array
	splines.clear();
}

bool FCDGeometrySpline::SetType( FUDaeSplineType::Type _type )
{
	for( size_t i = 0; i < splines.size(); i++ )
	{
		if( !splines[i] || splines[i]->GetSplineType() != _type )
		{
			return false;
		}
	}

	type = _type;
	SetDirtyFlag();
	return true;
}

bool FCDGeometrySpline::AddSpline( FCDSpline *spline )
{
	if( spline && spline->GetSplineType() == this->GetType() )
	{
		splines.push_back( spline );
		SetDirtyFlag();
		return true;
	}
	else
	{
		return false;
	}
}

FCDSpline* FCDGeometrySpline::RemoveSpline( FCDSpline* spline )
{
	FCDSplineList::iterator it = splines.end();
	if( (it = find( splines.begin(), splines.end(), spline )) != splines.end() )
	{
		FCDSpline *spline = *it;
		splines.erase( it, it++ );
		SetDirtyFlag();
		return spline;
	}
	else
	{
		return NULL;
	}
}

FCDSpline* FCDGeometrySpline::RemoveSpline( size_t index )
{
	if( index < splines.size() )
	{
		FCDSpline *spline = splines[index];
		splines.erase(index);
		SetDirtyFlag();
		return spline;
	}
	else
	{
		return NULL;
	}
}

size_t FCDGeometrySpline::GetTotalCVCount()
{
	size_t count = 0;
	for( size_t i = 0; i < splines.size(); i++ )
	{
		count += splines[i]->GetCVCount();
	}
	return count;
}

void FCDGeometrySpline::ConvertBezierToNURBS( FCDNURBSSplineList &toFill )
{
	if( type != FUDaeSplineType::BEZIER )
	{
		return;
	}

	for( size_t i = 0; i < splines.size(); i++ )
	{
		FCDBezierSpline *bez = static_cast<FCDBezierSpline *>(splines[i]);
		bez->ToNURBs( toFill );
	}
	SetDirtyFlag();
}

// Read in the <spline> node of the COLLADA document
FUStatus FCDGeometrySpline::LoadFromXML(xmlNode* splineNode)
{
	FUStatus status;

	// for each spline
	for(; splineNode != NULL; splineNode = splineNode->next )
	{
		// is it a spline?
		if( !IsEquivalent(splineNode->name, DAE_SPLINE_ELEMENT ) ) continue;

		// needed extra node
		// TODO. those will be moved to attributes
		xmlNode* extraNode = FindChildByType(splineNode, DAE_EXTRA_ELEMENT);
		if( extraNode == NULL ) continue;
		xmlNode *fcolladaNode = FindTechnique( extraNode, DAE_FCOLLADA_PROFILE );
		if( fcolladaNode == NULL ) continue;
		xmlNode *typeNode = FindChildByType( fcolladaNode, DAE_TYPE_ATTRIBUTE );
		if( typeNode == NULL ) continue;

		// get the spline type
		FUDaeSplineType::Type splineType = FUDaeSplineType::FromString( ReadNodeContentFull(typeNode) );

		// The types must be compatible
		if( !SetType( splineType ) )
		{
			return status.Warning( FS("Geometry contains different kinds of splines."), splineNode->line );
		}
		
		FUStatus s;
		FCDSpline* spline = NULL;
		
		switch( splineType )
		{
		case FUDaeSplineType::BEZIER:
			{
				FCDBezierSpline* bez = new FCDBezierSpline(GetDocument());
				s = bez->LoadFromXML(splineNode);
				spline = bez;
			}
			break;
		case FUDaeSplineType::NURBS:
			{	
				FCDNURBSSpline* nurbs = new FCDNURBSSpline(GetDocument());
				s = nurbs->LoadFromXML(splineNode);
				spline = nurbs;
			}
			break;
		default:
			{
				s.Fail();
			}
		}
		

		if( !s.IsFailure() )
		{
			AddSpline(spline);
		}
		else
		{
			delete spline;
		}
		status.AppendStatus(s);

	} // for each spline node
		
	SetDirtyFlag();
	return status;
}

// Write out the <spline> node to the COLLADA XML tree
xmlNode* FCDGeometrySpline::WriteToXML(xmlNode* parentNode) const
{
	// create as many <spline> node as there are splines in the array
	for( size_t spline = 0; spline < splines.size(); spline++ )
	{
		FCDSpline *colladaSpline = splines[spline];
		if( colladaSpline == NULL ) continue;

		string parentId = GetParent()->GetDaeId();
		string splineId = FUStringConversion::ToString( spline );

		switch( colladaSpline->GetSplineType() )
		{
		case FUDaeSplineType::BEZIER:
			{
				FCDBezierSpline* bez = (FCDBezierSpline*)colladaSpline;
				bez->WriteToXML(parentNode,parentId,splineId);
			}
			break;
		case FUDaeSplineType::NURBS:
			{
				FCDNURBSSpline* nurbs = (FCDNURBSSpline*)colladaSpline;
				nurbs->WriteToXML(parentNode,parentId,splineId);
			}
			break;
		default:
			break;
		}

		
	}

	return NULL;
}

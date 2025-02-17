/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDGeometry.h"
#include "FCDocument/FCDGeometrySpline.h"
#include "FCDocument/FCDGeometryNURBSSurface.h"
#include "FUtils/FUStringConversion.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

//
// FCDGeometryNURBSSurface
//

ImplementObjectType(FCDGeometryNURBSSurface);

FCDGeometryNURBSSurface::FCDGeometryNURBSSurface(FCDocument* document, FCDGeometryPSurface* _parentSet )
:	FCDObject(document)
,	name("NURBS Surface")
,	closedInU(false)
,	closedInV(false)
,	flipNormals(false)
,	textureSurface(NULL)
,	type(FUDaeNURBSSurface::VERTEX)
,	parentSet(_parentSet)
{
}

FCDGeometryNURBSSurface::~FCDGeometryNURBSSurface()
{
	cvs.clear();
	uKnots.clear();
	vKnots.clear();

	for( size_t g = 0; g < trimGroups.size(); g++ )
	{
		CLEAR_POINTER_VECTOR( trimGroups[g] );
	}

	SAFE_DELETE(textureSurface);
}

uint32 FCDGeometryNURBSSurface::AddTrimGroup()
{
	if( type == FUDaeNURBSSurface::VERTEX )
	{
		FCDTrimCurveGroup group;
		trimGroups.push_back( group );
	}
	return (uint32)trimGroups.size() - 1;
}

FCDNURBSSpline* FCDGeometryNURBSSurface::AddTrimCurve( uint32 groupIndex, FCDNURBSSpline* toAdd )
{
	if( toAdd == NULL || type != FUDaeNURBSSurface::VERTEX || groupIndex >= trimGroups.size() )
		return NULL;

	trimGroups[groupIndex].push_back( toAdd );
	return toAdd;
}

FCDNURBSSpline* FCDGeometryNURBSSurface::AddTrimCurve( uint32 groupIndex )
{
	if( type != FUDaeNURBSSurface::VERTEX || groupIndex >= trimGroups.size() )
		return NULL;

	FCDNURBSSpline* curve = new FCDNURBSSpline(GetDocument());
	trimGroups[groupIndex].push_back( curve );
	return curve;
}

FCDGeometryNURBSSurface* FCDGeometryNURBSSurface::GetTextureSurface(bool doCreate)
{
	if( type != FUDaeNURBSSurface::VERTEX ) return NULL;
	if( textureSurface != NULL || !doCreate ) return textureSurface;

	textureSurface = new FCDGeometryNURBSSurface(GetDocument(), GetParentSet() );
	textureSurface->SetType( FUDaeNURBSSurface::TEXTURE );
	string s("Texture Surface");
	textureSurface->SetName( s );

	return textureSurface;
}

void FCDGeometryNURBSSurface::SetMaterialSemantic( const fstring& ms )
{
	if( type == FUDaeNURBSSurface::VERTEX )
	{
		materialSemantic = ms;
	}
}

void FCDGeometryNURBSSurface::RemoveEmptyTrimGroups()
{
	FCDTrimCurveGroupList::iterator it = trimGroups.begin();
	while( it != trimGroups.end() )
	{
		if( it->size() == 0 )
			trimGroups.erase(it);
		else
			it++;
	}
}

void FCDGeometryNURBSSurface::GetKnotDomain( float& uStart, float& uEnd, float& vStart, float& vEnd )
{
	uStart = uKnots[p];
	uEnd = uKnots[ uKnots.size() - p - 1 ];
	vStart = vKnots[q];
	vEnd = vKnots[ vKnots.size() - q - 1 ];
}

bool FCDGeometryNURBSSurface::IsValid( FUStatus* status )
{
	bool log = (status != NULL);

	// verify that we have ascending knot vectors

	for( size_t i = 1; i < uKnots.size(); i++ )
	{
		if( uKnots[i] < uKnots[i-1] )
		{
			if( log ) status->Fail(TO_FSTRING("Found non-ascending U knot vector"));
			return false;
		}
	}
	for( size_t i = 1; i < vKnots.size(); i++ )
	{
		if( vKnots[i] < vKnots[i-1] )
		{
			if( log ) status->Fail(TO_FSTRING("Found non-ascending V knot vector"));
			return false;
		}
	}

	// verify the K = N + P + 1 relation using the knot vectors for basis.

	int a = (int)uKnots.size() - p - 1;
	int b = (int)vKnots.size() - q - 1;

	if( a <= 0 ){
		if( log ) status->Fail(TO_FSTRING("Not enough elements in the U knot vector."));
		return false;
	}

	if( b <= 0 ){
		if( log ) status->Fail(TO_FSTRING("Not enough elements in the V knot vector."));
		return false;
	}
	
	if( a*b != (int)cvs.size() )
	{
		if( log ) status->Fail(TO_FSTRING("Found unexpected number of control vertices."));
		return false;
	}

	return true;
}

FUStatus FCDGeometryNURBSSurface::LoadFromXML(xmlNode* nurbsNode)
{
	FUStatus status;
	
	if( parentSet == NULL || nurbsNode == NULL || !IsEquivalent( nurbsNode->name, DAE_PARAMETERIZED_SURFACE_ELEMENT ) )
		return status.Fail();

	fstring surfaceId = TO_FSTRING(parentSet->GetParent()->GetDaeId());

	// read the degrees
	p = FUStringConversion::ToInt32( ReadNodeProperty(nurbsNode,DAE_UDEGREE_ATTRIBUTE) );
	q = FUStringConversion::ToInt32( ReadNodeProperty(nurbsNode,DAE_VDEGREE_ATTRIBUTE) );

	// read the closures
	closedInU = FUStringConversion::ToBoolean( ReadNodeProperty(nurbsNode,DAE_CLOSEDU_ATTRIBUTE) );
	closedInV = FUStringConversion::ToBoolean( ReadNodeProperty(nurbsNode,DAE_CLOSEDV_ATTRIBUTE) );

	// read the inputs
	// Read in the <control_vertices> element
	xmlNode* controlVerticesNode = FindChildByType(nurbsNode, DAE_CONTROL_VERTICES_ELEMENT);
	if (controlVerticesNode == NULL)
	{
		return status.Fail( FS("No <control_vertices> element in NURBS surface: ") + surfaceId, nurbsNode->line );
	}

	// read the sources
	xmlNodeList inputElements;
	FindChildrenByType( controlVerticesNode, DAE_INPUT_ELEMENT, inputElements );

	for( size_t i = 0; i < inputElements.size(); i++ )
	{
		xmlNode *inputNode = inputElements[i];
		string sourceId = ReadNodeProperty( inputNode, DAE_SOURCE_ATTRIBUTE ).substr(1);
		if( sourceId.empty() ) { continue; }
		xmlNode *sourceNode = FindChildById( nurbsNode, sourceId );
		if( sourceNode == NULL ) { continue; }

		if( IsEquivalent( ReadNodeProperty( inputNode, DAE_SEMANTIC_ATTRIBUTE ), DAE_CVS_SPLINE_INPUT ) )
		{
			ReadSource( sourceNode, cvs );
		}
		else if( IsEquivalent( ReadNodeProperty( inputNode, DAE_SEMANTIC_ATTRIBUTE ), DAE_UKNOT_NURBS_INPUT ) )
		{
			ReadSource( sourceNode, uKnots );
		}
		else if( IsEquivalent( ReadNodeProperty( inputNode, DAE_SEMANTIC_ATTRIBUTE ), DAE_VKNOT_NURBS_INPUT ) )
		{
			ReadSource( sourceNode, vKnots );
		}
		else if( IsEquivalent( ReadNodeProperty( inputNode, DAE_SEMANTIC_ATTRIBUTE ), DAE_WEIGHT_SPLINE_INPUT ) )
		{
			ReadSource( sourceNode, weights );
		}
	}

	// read the texture surface
	xmlNode* textureNode = FindChildByType( nurbsNode, DAE_TEXTURE_SURFACE_ELEMENT );
	if( textureNode != NULL )
	{
		materialSemantic = TO_FSTRING(ReadNodeProperty( textureNode, DAE_MATERIAL_ATTRIBUTE ));
		xmlNode *texSurfNode = FindChildByType( textureNode, DAE_PARAMETERIZED_SURFACE_ELEMENT );
		if( texSurfNode != NULL )
		{
			FCDGeometryNURBSSurface* texSurf = GetTextureSurface(true);
			status.AppendStatus( texSurf->LoadFromXML(texSurfNode) );
		}
	}

	// read the trim curves (trim curves are optional elements)
	xmlNodeList trimGroupElements;
	FindChildrenByType( nurbsNode, DAE_TRIM_GROUP_ELEMENT, trimGroupElements );

	for( size_t i = 0; i < trimGroupElements.size(); i++ )
	{
		xmlNode* trimGroupNode = trimGroupElements[i];
		if( trimGroupNode == NULL ) continue;

		xmlNodeList trimCurveElements;
		FindChildrenByType( trimGroupNode, DAE_SPLINE_ELEMENT, trimCurveElements );

		if( trimCurveElements.empty() )
			continue;

		// add the trim group
		uint32 groupIndex = AddTrimGroup();

		for( size_t j = 0; j < trimCurveElements.size(); j++ )
		{
			xmlNode* trimCurveNode = trimCurveElements[j];
			if( trimCurveNode == NULL ) continue;

			// add the trim curve
			FCDNURBSSpline* trimCurve = new FCDNURBSSpline(GetDocument());
			if( trimCurve == NULL ) continue;

			FUStatus s = trimCurve->LoadFromXML(trimCurveNode);
			if( s.IsSuccessful() )
			{
				AddTrimCurve(groupIndex,trimCurve);
			}
			status.AppendStatus(s);
		}
	}
	
	// read in extra parameters
	xmlNode* extraNode = FindChildByType(nurbsNode,DAE_EXTRA_ELEMENT);
	if( extraNode != NULL )
	{
		xmlNode* maxTech = FindTechnique(extraNode,DAEMAX_MAX_PROFILE);
		if( maxTech != NULL )
		{
			xmlNode* flipNode = FindChildByType(maxTech, DAEMAX_NURBS_FLIP_NORMALS_PARAMETER);
			if( flipNode != NULL )
			{
				flipNormals = FUStringConversion::ToBoolean( ReadNodeContentFull(flipNode) );
			}
		}
	}

	return status;
}

xmlNode* FCDGeometryNURBSSurface::WriteToXML(xmlNode* parentNode) const
{
	if( parentNode == NULL )
		return NULL;

	string surfId = parentSet->GetParent()->GetDaeId() + "-";

	uint32 index;
	if( !parentSet->FindNURBSSurface(this,index) ) return NULL;

	surfId += FUStringConversion::ToString(index);

	if( type == FUDaeNURBSSurface::TEXTURE )
	{
		surfId += "-texture";
	}

	xmlNode* surfNode = AddChild(parentNode, DAE_PARAMETERIZED_SURFACE_ELEMENT );

	// add the name
	AddAttribute(surfNode, DAE_NAME_ATTRIBUTE, name);

	// add the degrees
	AddAttribute(surfNode, DAE_UDEGREE_ATTRIBUTE, FUStringConversion::ToString(p) );
	AddAttribute(surfNode, DAE_VDEGREE_ATTRIBUTE, FUStringConversion::ToString(q) );

	// add the closures
	AddAttribute(surfNode, DAE_CLOSEDU_ATTRIBUTE, FUStringConversion::ToString(closedInU));
	AddAttribute(surfNode, DAE_CLOSEDV_ATTRIBUTE, FUStringConversion::ToString(closedInV));

	// add CVS
	FUSStringBuilder controlPointSourceId(surfId); controlPointSourceId += "-cvs";
	AddSourcePosition(surfNode, controlPointSourceId.ToCharPtr(), cvs );
	
	// weights
	FUSStringBuilder weightSourceId(surfId); weightSourceId += "-weights";
	AddSourceFloat(surfNode,weightSourceId.ToCharPtr(),weights,"WEIGHT");

	// knots
	FUSStringBuilder uKnotSourceId(surfId); uKnotSourceId += "-uknots";
	AddSourceFloat(surfNode, uKnotSourceId.ToCharPtr(), uKnots, "KNOT" );
	FUSStringBuilder vKnotSourceId(surfId); vKnotSourceId += "-vknots";
	AddSourceFloat(surfNode, vKnotSourceId.ToCharPtr(), vKnots, "KNOT" );

	// add the surface inputs

	// add the <control_vertices> element
	xmlNode* verticesNode = AddChild(surfNode, DAE_CONTROL_VERTICES_ELEMENT);
	
	// add inputs
	AddInput(verticesNode, controlPointSourceId.ToCharPtr(), DAE_CVS_SPLINE_INPUT);
	AddInput(verticesNode, weightSourceId.ToCharPtr(), DAE_WEIGHT_SPLINE_INPUT );
	AddInput(verticesNode, uKnotSourceId.ToCharPtr(), DAE_UKNOT_NURBS_INPUT);
	AddInput(verticesNode, vKnotSourceId.ToCharPtr(), DAE_VKNOT_NURBS_INPUT);

	// add the texture surface
	if( textureSurface != NULL )
	{
		xmlNode* textureNode = AddChild(surfNode, DAE_TEXTURE_SURFACE_ELEMENT);
		AddAttribute( textureNode, DAE_MATERIAL_ATTRIBUTE, TO_STRING(materialSemantic) );
		textureSurface->WriteToXML(textureNode);
	}

	// add the trim groups and curves

	size_t trimCount = trimGroups.size();
	size_t realTrimCount = 0;
	for( size_t i=0; i<trimCount; i++ )
		if( trimGroups[i].size() > 0 )
			realTrimCount++;

	if( realTrimCount > 0 )
	{
		for( size_t i=0; i<trimCount; i++ )
		{
			if( trimGroups[i].empty() )
				continue;

			xmlNode* trimNode = AddChild(surfNode, DAE_TRIM_GROUP_ELEMENT );

			for( size_t j =0; j < trimGroups[i].size(); j++ )
			{
				FCDNURBSSpline* curve = trimGroups[i][j];
				if( curve == NULL )
					continue;

				string curveId = string("-trim_") + FUStringConversion::ToString(i) + "_" + FUStringConversion::ToString(j);
				curve->WriteToXML(trimNode,surfId,curveId);
			}
		}
	}

	// write the extra parameters
	xmlNode* maxTech = AddExtraTechniqueChild(surfNode,DAEMAX_MAX_PROFILE);
	AddChild( maxTech, DAEMAX_NURBS_FLIP_NORMALS_PARAMETER, FUStringConversion::ToString(flipNormals) );

	return surfNode;
}

///////////////////////////////////////////////////////////////////////////////
// FCDGeometryPSurface
///////////////////////////////////////////////////////////////////////////////

ImplementObjectType(FCDGeometryPSurface);

FCDGeometryPSurface::FCDGeometryPSurface(FCDocument* _document, FCDGeometry* _parent)
:	FCDObject(_document), parent(_parent)
{
}

FCDGeometryPSurface::~FCDGeometryPSurface()
{
	CLEAR_POINTER_VECTOR(nurbsSurfaceList);
}

FCDGeometryNURBSSurface* FCDGeometryPSurface::AddNURBSSurface()
{
	FCDGeometryNURBSSurface* surf = new FCDGeometryNURBSSurface(GetDocument(), this);
	nurbsSurfaceList.push_back( surf );
	return surf;
}


bool FCDGeometryPSurface::FindNURBSSurface( const FCDGeometryNURBSSurface* key, uint32& index ) const
{
	FCDNURBSSurfaceList::const_iterator it = nurbsSurfaceList.begin();
	index = 0;

	for(; it != nurbsSurfaceList.end(); it++ )
	{
		if( *it == key )
		{
			return true;
		}
		else
		{
			if( (*it)->GetTextureSurface() == key )
			{
				return true;
			}
		}
		index++;
	}

	return false;
}

FUStatus FCDGeometryPSurface::LoadFromXML(xmlNode* nurbsNode)
{
	FUStatus status;

	// for each NURBS surface in the set
	for(; nurbsNode != NULL; nurbsNode = nurbsNode->next)
	{
		if( !IsEquivalent( nurbsNode->name, DAE_PARAMETERIZED_SURFACE_ELEMENT ) )
			continue;

		FCDGeometryNURBSSurface* surf = AddNURBSSurface();
		status.AppendStatus( surf->LoadFromXML(nurbsNode) );
	}

	return status;
}

xmlNode* FCDGeometryPSurface::WriteToXML(xmlNode* parentNode) const
{
	for( size_t i = 0; i < nurbsSurfaceList.size(); i++ )
	{
		nurbsSurfaceList[i]->WriteToXML( parentNode );
	}
	return NULL;
}

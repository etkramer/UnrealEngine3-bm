//------------------------------------------------------------------------------
// Controls access to XSI bone animation curves.
//
//------------------------------------------------------------------------------

#include "FxXSIAnimController.h"

#include <xsi_application.h>
#include <xsi_fcurve.h>
#include <xsi_ref.h>
#include <xsi_model.h>
#include <xsi_kinematics.h>
#include <xsi_kinematicstate.h>
#include <xsi_parameter.h>


using namespace OC3Ent;
using namespace Face;
using namespace XSI;

// Constructor.
FxXSIAnimCurve::FxXSIAnimCurve
( 
	const OC3Ent::Face::FxString& inBoneName,
	const CRef& inBoneObject,
	const CRef& translateXNode,
	const CRef& translateYNode,
	const CRef& translateZNode,
	const CRef& rotateXNode,
	const CRef& rotateYNode,
	const CRef& rotateZNode,
	const CRef& scaleXNode,
	const CRef& scaleYNode,
	const CRef& scaleZNode 
): m_boneName(inBoneName)
	, m_boneObject(inBoneObject)
	, m_translateX(translateXNode)
	, m_translateY(translateYNode)
	, m_translateZ(translateZNode)
	, m_rotateX(rotateXNode)
	, m_rotateY(rotateYNode)
	, m_rotateZ(rotateZNode)
	, m_scaleX(scaleXNode)
	, m_scaleY(scaleYNode)
	, m_scaleZ(scaleZNode)
{
}

// Destructor.
FxXSIAnimCurve::~FxXSIAnimCurve()
{
}

// Constructor.
FxXSIAnimController::FxXSIAnimController()
{
}

// Destructor.
FxXSIAnimController::~FxXSIAnimController()
{
	_clear();
}

// Initializes the controller with animation curves for the specified
// bones.
bool FxXSIAnimController::Initialize( const OC3Ent::Face::FxArray<OC3Ent::Face::FxString>& bones )
{
	// Clear any previous entries.
	_clear();
	
	Application app;
	Model root = app.GetActiveSceneRoot();

	// Hook into each attribute for each bone.
	FxBool allMatching = FxTrue;
	for( FxSize i = 0; i < bones.Length(); ++i )
	{
		CString boneName;
		boneName.PutAsciiString(bones[i].GetData());		
		X3DObject bone = root.FindChild(boneName,L"",CStringArray());
		if(!bone.IsValid())
		{
			app.LogMessage(L"FxXSI: Bone not found");
			continue;
		}

		KinematicState localKine = bone.GetKinematics().GetLocal();
		//Translation
		Parameter posX = localKine.GetParameter(L"posx");
		Parameter posY = localKine.GetParameter(L"posy");
		Parameter posZ = localKine.GetParameter(L"posz");

		//Orientation
		Parameter rotX = localKine.GetParameter(L"rotx");
		Parameter rotY = localKine.GetParameter(L"roty");
		Parameter rotZ = localKine.GetParameter(L"rotz");

		//Scaling
		Parameter sclX = localKine.GetParameter(L"sclx");
		Parameter sclY = localKine.GetParameter(L"scly");
		Parameter sclZ = localKine.GetParameter(L"sclz");


		FCurve posXFCurve(posX.GetSource());
		if(!posXFCurve.IsValid())
		{
			posX.AddFCurve(siStandardFCurve,posXFCurve);
		}

		FCurve posYFCurve(posY.GetSource());
		if(!posYFCurve.IsValid())
		{
			posY.AddFCurve(siStandardFCurve,posYFCurve);
		}

		FCurve posZFCurve(posZ.GetSource());
		if(!posZFCurve.IsValid())
		{
			posZ.AddFCurve(siStandardFCurve,posZFCurve);
		}


		FCurve rotXFCurve(rotX.GetSource());
		if(!rotXFCurve.IsValid())
		{
			rotX.AddFCurve(siStandardFCurve,rotXFCurve);
		}

		FCurve rotYFCurve(rotY.GetSource());
		if(!rotYFCurve.IsValid())
		{
			rotY.AddFCurve(siStandardFCurve,rotYFCurve);
		}

		FCurve rotZFCurve(rotZ.GetSource());
		if(!rotZFCurve.IsValid())
		{
			rotZ.AddFCurve(siStandardFCurve,rotZFCurve);
		}

		FCurve sclXFCurve(sclX.GetSource());
		if(!sclXFCurve.IsValid())
		{
			sclX.AddFCurve(siStandardFCurve,sclXFCurve);
		}

		FCurve sclYFCurve(sclY.GetSource());
		if(!sclYFCurve.IsValid())
		{
			sclY.AddFCurve(siStandardFCurve,sclYFCurve);
		}

		FCurve sclZFCurve(sclZ.GetSource());
		if(!sclZFCurve.IsValid())
		{
			sclZ.AddFCurve(siStandardFCurve,sclZFCurve);
		}


		m_animCurves.PushBack(new FxXSIAnimCurve(bones[i], bone.GetRef(),
			posXFCurve, posYFCurve,  posZFCurve,
			rotXFCurve, rotYFCurve, rotZFCurve,
			sclXFCurve, sclYFCurve, sclZFCurve));

	}
	if( !allMatching )
	{
		return false;
	}
	return true;
}

// Finds the animation curve associated with the specified bone name and
// returns a pointer to it or NULL if it does not exist.
FxXSIAnimCurve* FxXSIAnimController::FindAnimCurve( const OC3Ent::Face::FxString& bone )
{
	for( FxSize i = 0; i < m_animCurves.Length(); ++i )
	{
		if( m_animCurves[i] )
		{
			if( m_animCurves[i]->m_boneName == bone )
			{
				return m_animCurves[i];
			}
		}
	}
	return NULL;
}

// Removes all keys from all bone animation curves.
void FxXSIAnimController::Clean( void )
{
	for( FxSize i = 0; i < m_animCurves.Length(); ++i )
	{
		if( m_animCurves[i] )
		{
			m_animCurves[i]->m_translateX.RemoveKeys();
			m_animCurves[i]->m_translateY.RemoveKeys();
			m_animCurves[i]->m_translateZ.RemoveKeys();
			m_animCurves[i]->m_rotateX.RemoveKeys();
			m_animCurves[i]->m_rotateY.RemoveKeys();
			m_animCurves[i]->m_rotateZ.RemoveKeys();
			m_animCurves[i]->m_scaleX.RemoveKeys();
			m_animCurves[i]->m_scaleY.RemoveKeys();
			m_animCurves[i]->m_scaleZ.RemoveKeys();
		}
	}
}

// Clears the list of bone animation curves.
void FxXSIAnimController::_clear( void )
{
	for( FxSize i = 0; i < m_animCurves.Length(); ++i )
	{
		delete m_animCurves[i];
		m_animCurves[i] = NULL;
	}
	m_animCurves.Clear();
}

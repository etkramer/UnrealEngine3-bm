//------------------------------------------------------------------------------
// Actor template support.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxActorTemplate_H__
#define FxActorTemplate_H__

#include "FxPlatform.h"
#include "FxNamedObject.h"
#include "FxActor.h"
#include "FxPhonemeMap.h"
#include "FxFaceGraphNodeGroupManager.h"
#include "FxCameraManager.h"
#include "FxWorkspaceManager.h"
#include "FxColourMapping.h"

namespace OC3Ent
{

namespace Face
{

// A FaceFX actor template.
class FxActorTemplate : public FxNamedObject
{
	// Declare the class.
	FX_DECLARE_CLASS(FxActorTemplate, FxNamedObject);
	// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxActorTemplate);
public:
	// Constructor.
	FxActorTemplate();
	// Destructor.
	virtual ~FxActorTemplate();

	// Sets the Face Graph contained in the template.
	void SetFaceGraph( const FxFaceGraph& faceGraph );
	// Gets the Face Graph contained in the template.
	FxFaceGraph& GetFaceGraph( void );

	// Sets the phoneme mapping contained in the template.
	void SetPhonemeMap( const FxPhonemeMap& phonemeMap );
	// Gets the phoneme mapping contained in the template.
	const FxPhonemeMap& GetPhonemeMap( void ) const;

	// Sets the node groups contained in the template.
	void SetNodeGroups( const FxArray<FxFaceGraphNodeGroup>& nodeGroups );
	// Gets the node groups contained in the template.
	const FxArray<FxFaceGraphNodeGroup>& GetNodeGroups( void ) const;

	// Sets the cameras contained in the template.
	void SetCameras( const FxArray<FxViewportCamera>& cameras );
	// Gets the cameras contained in the template.
	const FxArray<FxViewportCamera>& GetCameras( void ) const;

	// Sets the workspaces contained in the template.
	void SetWorkspaces( const FxArray<FxWorkspace>& workspaces );
	// Gets the workspaces contained in the template.
	const FxArray<FxWorkspace>& GetWorkspaces( void ) const;

	// Sets the colour map contained in the template.
	void SetColourMap( const FxArray<FxColourMapEntry>& colourMap );
	// Gets the colour map contained in the template.
	const FxArray<FxColourMapEntry>& GetColourMap( void ) const;

	// Serialization.
	virtual void Serialize( FxArchive& arc );

protected:
	// The Face Graph contained in the template (with the bones stripped from 
	// the bone poses).
	FxFaceGraph  _faceGraph;
	// The phoneme mapping contained in the template.
	FxPhonemeMap _phonemeMap;
	// The node groups contained in the template.
	FxArray<FxFaceGraphNodeGroup> _nodeGroups;
	// The cameras contained in the template.
	FxArray<FxViewportCamera> _cameras;
	// The workspaces contained in the template.
	FxArray<FxWorkspace> _workspaces;
	// The colour map contained in the template.
	FxArray<FxColourMapEntry> _colourMap;
};

} // namespace Face

} // namespace OC3Ent

#endif

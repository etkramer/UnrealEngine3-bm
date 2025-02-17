//------------------------------------------------------------------------------
// This class implements a generic target node in an FxFaceGraph.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxGenericTargetNode_H__
#define FxGenericTargetNode_H__

#include "FxPlatform.h"
#include "FxArchive.h"
#include "FxFaceGraphNode.h"

namespace OC3Ent
{

namespace Face
{

/// As of version 1.7, FxGenericTargetNode and FxGenericTargetProxy have been
/// deprecated.  FxGenericTargetNode is still here for backwards compatibility
/// but FxGenericTargetProxy has been completely removed from the FaceFX SDK.
/// Eventually FxGenericTargetNode itself will be completely removed from the
/// FaceFX SDK. See \ref whatsNew17 "What's new in 1.7" for more information.
/// \ingroup faceGraph
class FxGenericTargetNode : public FxFaceGraphNode
{
	/// Declare the class.
	FX_DECLARE_CLASS(FxGenericTargetNode, FxFaceGraphNode)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxGenericTargetNode)
public:
	/// Clones the generic target node.
	virtual FxFaceGraphNode* Clone( void );
	/// Copies the data from this object into the other object.
	virtual void CopyData( FxFaceGraphNode* pOther );
	/// Serializes an FxGenericTargetNode to an archive.
	virtual void Serialize( FxArchive& arc );

protected:
	/// Constructor.
	/// The constructor and destructor are protected to prevent actually 
	/// instantiating a base FxGenericTargetNode.
	FxGenericTargetNode();
	/// Destructor.
	virtual ~FxGenericTargetNode();
};

} // namespace Face

} // namespace OC3Ent

#endif

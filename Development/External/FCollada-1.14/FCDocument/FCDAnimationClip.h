/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _FCD_ANIMATION_CLIP_H_
#define _FCD_ANIMATION_CLIP_H_

class FCDocument;
class FCDAnimationCurve;

typedef FUObjectList<FCDAnimationCurve> FCDAnimationCurveTrackList;

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_

class FCOLLADA_EXPORT FCDAnimationClip : public FCDEntity
{
private:
	DeclareObjectType(FCDEntity);
	FCDAnimationCurveTrackList curves;
	float start, end;

public:
	FCDAnimationClip(FCDocument* document);
	virtual ~FCDAnimationClip();

	FCDAnimationClip* Clone();

	// FCDEntity overrides
	virtual Type GetType() const { return ANIMATION_CLIP; }

	// Accessors
	FCDAnimationCurveTrackList& GetClipCurves() { return curves; }
	const FCDAnimationCurveTrackList& GetClipCurves() const { return curves; }
	void AddClipCurve(FCDAnimationCurve* curve);

	// Timing Values
	float GetStart() const { return start; }
	void SetStart(float _start) { start = _start; SetDirtyFlag(); }
	float GetEnd() const { return end; }
	void SetEnd(float _end) { end = _end; SetDirtyFlag(); }

	void UpdateAnimationCurves(const float newStart);

	// Load a Collada animation node from the XML document
	virtual FUStatus LoadFromXML(xmlNode* clipNode);

	// Write out the COLLADA animations to the document
	virtual xmlNode* WriteToXML(xmlNode* parentNode) const;
};

#endif // _FCD_ANIMATION_CLIP_H_


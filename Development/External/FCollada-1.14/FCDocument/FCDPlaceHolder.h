/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDPlaceHolder.h
	This file contains the FCDPlaceHolder class.
*/

#ifndef _FCD_PLACEHOLDER_H_
#define _FCD_PLACEHOLDER_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_

class FCDExternalReference;
typedef FUObjectList<FCDExternalReference> FCDExternalReferenceTrackList;

class FCOLLADA_EXPORT FCDPlaceHolder : public FCDObject, FUObjectTracker
{
private:
	DeclareObjectType(FCDPlaceHolder);

	FCDocument* target;
	FUStatus loadStatus;
	FCDExternalReferenceTrackList references;
	fstring fileUrl;

public:
	FCDPlaceHolder(FCDocument* document, FCDocument* target = NULL);
	virtual ~FCDPlaceHolder();

	FCDocument* GetTarget(bool loadIfMissing = true);
	void LoadTarget(FCDocument* _target = NULL);
	void UnloadTarget();
	bool IsTargetLoaded() const { return target != NULL; }
	const FUStatus& GetLoadTargetStatus() const { return loadStatus; }

	const fstring& GetFileUrl() const;
	void SetFileUrl(const fstring& url);

	void AddExternalReference(FCDExternalReference* reference) { references.push_back(reference); SetDirtyFlag(); }
	void RemoveExternalReference(FCDExternalReference* reference) { references.erase(reference); SetDirtyFlag(); }

	size_t GetExternalReferenceCount() const { return references.size(); }
	FCDExternalReference *GetExternalReference( size_t idx ) const { FUAssert( idx < GetExternalReferenceCount(), return NULL ); return references[idx]; }

protected:
	void OnObjectReleased(FUObject* object);
};

#endif // _FCD_PLACEHOLDER_H_


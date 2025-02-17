/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDExternalReferenceManager.h"
#include "FCDocument/FCDPlaceHolder.h"

namespace FCollada
{
	FCOLLADA_EXPORT unsigned long GetVersion() { return FCOLLADA_VERSION; }

	static FUObjectContainer<FCDocument> topDocuments;
	static bool dereferenceFlag = true;

	FCOLLADA_EXPORT FCDocument* NewTopDocument()
	{
		// Just add the top documents to the above tracker: this will add one global tracker and the
		// document will not be released automatically by the document placeholders.
		return topDocuments.Add();
	}

	FCOLLADA_EXPORT size_t GetTopDocumentCount() { return topDocuments.size(); }
	FCOLLADA_EXPORT FCDocument* GetTopDocument(size_t index) { FUAssert(index < topDocuments.size(), return NULL); return topDocuments.at(index); }
	FCOLLADA_EXPORT bool IsTopDocument(FCDocument* document) { return topDocuments.contains(document); }

	FCOLLADA_EXPORT void GetAllDocuments(FCDocumentList& documents)
	{
		documents.clear();
		documents.insert(documents.end(), topDocuments.begin(), topDocuments.end());
		for (size_t index = 0; index < topDocuments.size(); ++index)
		{
			FCDocument* document = documents[index];
			FCDExternalReferenceManager* xrefManager = document->GetExternalReferenceManager();
			size_t placeHolderCount = xrefManager->GetPlaceHolderCount();
			for (size_t p = 0; p < placeHolderCount; ++p)
			{
				FCDPlaceHolder* placeHolder = xrefManager->GetPlaceHolder(p);
				FCDocument* target = placeHolder->GetTarget(false);
				if (target != NULL && !documents.contains(target)) documents.push_back(target);
			}
		}
	}

	FCOLLADA_EXPORT bool GetDereferenceFlag() { return dereferenceFlag; }
	FCOLLADA_EXPORT void SetDereferenceFlag(bool flag) { dereferenceFlag = flag; }
};

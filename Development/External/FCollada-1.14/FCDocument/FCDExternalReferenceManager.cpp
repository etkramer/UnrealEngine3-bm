/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDExternalReference.h"
#include "FCDocument/FCDExternalReferenceManager.h"
#include "FCDocument/FCDPlaceHolder.h"
#include "FUtils/FUFileManager.h"

//
// FCDExternalReferenceManager
//

ImplementObjectType(FCDExternalReferenceManager);

FCDExternalReferenceManager::FCDExternalReferenceManager(FCDocument* document)
:	FCDObject(document)
{
}

FCDExternalReferenceManager::~FCDExternalReferenceManager()
{
}

FCDExternalReference* FCDExternalReferenceManager::AddExternalReference(FCDEntityInstance* instance)
{
	FCDExternalReference* reference = references.Add(GetDocument(), instance);
	SetDirtyFlag();
	return reference;
}

FCDPlaceHolder* FCDExternalReferenceManager::AddPlaceHolder(const fstring& _fileUrl)
{
	fstring fileUrl = GetDocument()->GetFileManager()->MakeFilePathAbsolute(_fileUrl);
	FCDPlaceHolder* holder = placeHolders.Add(GetDocument());
	holder->SetFileUrl(fileUrl);
	SetDirtyFlag();
	return holder;
}

const FCDPlaceHolder* FCDExternalReferenceManager::FindPlaceHolder(const fstring& _fileUrl) const
{
	fstring fileUrl = GetDocument()->GetFileManager()->MakeFilePathAbsolute(_fileUrl);
	for (FCDPlaceHolderContainer::const_iterator it = placeHolders.begin(); it != placeHolders.end(); ++it)
	{
		if ((*it)->GetFileUrl() == fileUrl) return *it;
	}
	return NULL;
}

FCDPlaceHolder* FCDExternalReferenceManager::AddPlaceHolder(FCDocument* document)
{
	FCDPlaceHolder* placeHolder = placeHolders.Add(GetDocument(), document);
	SetDirtyFlag();
	return placeHolder;
}

const FCDPlaceHolder* FCDExternalReferenceManager::FindPlaceHolder(const FCDocument* document) const
{
	for (FCDPlaceHolderContainer::const_iterator it = placeHolders.begin(); it != placeHolders.end(); ++it)
	{
		if ((*it)->GetTarget(false) == document) return *it;
	}
	return NULL;
}

void FCDExternalReferenceManager::RegisterLoadedDocument(FCDocument* document)
{
	FCDocumentList allDocuments;
	FCollada::GetAllDocuments(allDocuments);
	for (FCDocumentList::iterator it = allDocuments.begin(); it != allDocuments.end(); ++it)
	{
		if ((*it) != document)
		{
			FCDExternalReferenceManager* xrefManager = (*it)->GetExternalReferenceManager();

			for (FCDPlaceHolderContainer::iterator itP = xrefManager->placeHolders.begin(); itP != xrefManager->placeHolders.end(); ++itP)
			{
				// Set the document to the placeholders that targets it.
				if ((*itP)->GetFileUrl() == document->GetFileUrl()) (*itP)->LoadTarget(document);
			}
		}
	}

	// On the newly-loaded document, there may be placeholders to process.
	FCDExternalReferenceManager* xrefManager = document->GetExternalReferenceManager();
	for (FCDPlaceHolderContainer::iterator itP = xrefManager->placeHolders.begin(); itP != xrefManager->placeHolders.end(); ++itP)
	{
		// Set the document to the placeholders that targets it.
		for (FCDocumentList::iterator itD = allDocuments.begin(); itD != allDocuments.end(); ++itD)
		{
			if ((*itP)->GetFileUrl() == (*itD)->GetFileUrl()) (*itP)->LoadTarget(*itD);
		}
	}
}

/*
	Copyright (C) 2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FUtils/FUXmlDocument.h"
#include "FUtils/FUXmlWriter.h"
#include "FUtils/FUFile.h"

//
// FUXmlDocument
// 

FUXmlDocument::FUXmlDocument(const fchar* _filename, bool _isParsing)
:	isParsing(_isParsing), filename(_filename)
,	xmlDocument(NULL)
{
	if (isParsing)
	{
		FUFile file(filename, FUFile::READ);
		size_t fileLength = file.GetLength();
		uint8* fileData = new uint8[fileLength];
		file.Read(fileData, fileLength);
		file.Close();

		// Open the given XML file.
		xmlDocument = xmlParseMemory((const char*) fileData, fileLength);
		SAFE_DELETE_ARRAY(fileData);
	}
	else
	{
		xmlDocument = xmlNewDoc(NULL); // NULL implies version 1.0.
	}
}

FUXmlDocument::~FUXmlDocument()
{
	// Release the XML document
	if (xmlDocument != NULL)
	{
		if (!isParsing)
		{
		}
		xmlFreeDoc(xmlDocument);
	}
	xmlCleanupParser();
}

xmlNode* FUXmlDocument::GetRootNode()
{
	if (xmlDocument == NULL)
	{
		return NULL;
	}

	return xmlDocGetRootElement(xmlDocument);
}

xmlNode* FUXmlDocument::CreateRootNode(const char* name)
{
	xmlNode* rootNode = NULL;
	if (!isParsing)
	{
		rootNode = FUXmlWriter::CreateNode(name);
		xmlDocSetRootElement(xmlDocument, rootNode);
	}
	return rootNode;
}

// Writes out the XML document.
bool FUXmlDocument::Write(const char* encoding)
{
	FUFile file(filename, FUFile::WRITE);
	xmlDocument->encoding = xmlStrdup((const xmlChar*) encoding);
	return xmlDocFormatDump(file.GetHandle(), xmlDocument, 1) > 0;
}

/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/
/*
	Based on the FS Import classes:
	Copyright (C) 2005-2006 Feeling Software Inc
	Copyright (C) 2005-2006 Autodesk Media Entertainment
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _FU_DAE_PARSER_
#define _FU_DAE_PARSER_

#ifdef HAS_LIBXML

#ifndef _DAE_SYNTAX_H_
#include "FUtils/FUDaeSyntax.h"
#endif // _DAE_SYNTAX_H_
#ifndef _FU_URI_H_
#include "FUtils/FUUri.h"
#endif // _FU_URI_H_
#ifndef _FU_XML_PARSER_H_
#include "FUtils/FUXmlParser.h"
#endif // _FU_XML_PARSER_H_
#ifndef _FU_XML_NODE_ID_PAIR_H_
#include "FUtils/FUXmlNodeIdPair.h"
#endif // _FU_XML_NODE_ID_PAIR_H_

namespace FUDaeParser
{
	using namespace FUXmlParser;

	// Retrieve specific child nodes
	FCOLLADA_EXPORT xmlNode* FindChildById(xmlNode* parent, const string& id);
	FCOLLADA_EXPORT xmlNode* FindChildByRef(xmlNode* parent, const char* ref);
	FCOLLADA_EXPORT xmlNode* FindHierarchyChildBySid(xmlNode* hierarchyRoot, const char* sid);
	FCOLLADA_EXPORT xmlNode* FindTechnique(xmlNode* parent, const char* profile);
	FCOLLADA_EXPORT xmlNode* FindTechniqueAccessor(xmlNode* parent);
	FCOLLADA_EXPORT xmlNode* FindArray(xmlNode* parent, const char* arrayType);
	FCOLLADA_EXPORT void FindParameters(xmlNode* parent, StringList& parameterNames, xmlNodeList& parameterNodes);

	// Import source arrays
	FCOLLADA_EXPORT uint32 ReadSource(xmlNode* sourceNode, FloatList& array);
	FCOLLADA_EXPORT void ReadSource(xmlNode* sourceNode, Int32List& array);
	FCOLLADA_EXPORT void ReadSource(xmlNode* sourceNode, StringList& array);
	FCOLLADA_EXPORT void ReadSource(xmlNode* sourceNode, FMVector3List& array, float lengthFactor=1.0f);
	FCOLLADA_EXPORT void ReadSource(xmlNode* sourceNode, FMMatrix44List& array, float lengthFactor=1.0f);
	FCOLLADA_EXPORT void ReadSourceInterleaved(xmlNode* sourceNode, vector<FloatList*>& arrays);
	FCOLLADA_EXPORT uint32 ReadSourceInterleaved(xmlNode* sourceNode, vector<FMVector2List*>& arrays);
	FCOLLADA_EXPORT void ReadSourceInterpolation(xmlNode* sourceNode, UInt32List& array);
	FCOLLADA_EXPORT void ReadSourceInterpolationInterleaved(xmlNode* sourceNode, vector<UInt32List*>& arrays);

	// Target support
	FCOLLADA_EXPORT void ReadNodeTargetProperty(xmlNode* targetingNode, string& pointer, string& qualifier);
	FCOLLADA_EXPORT void SplitTarget(const string& target, string& pointer, string& qualifier);
	FCOLLADA_EXPORT void CalculateNodeTargetPointer(xmlNode* targetedNode, string& pointer);
	FCOLLADA_EXPORT int32 ReadTargetMatrixElement(string& qualifier);

	// Retrieve common node properties
	inline string ReadNodeId(xmlNode* node) { return ReadNodeProperty(node, DAE_ID_ATTRIBUTE); }
	inline string ReadNodeSid(xmlNode* node) { return ReadNodeProperty(node, DAE_SID_ATTRIBUTE); }
	inline string ReadNodeSemantic(xmlNode* node) { return ReadNodeProperty(node, DAE_SEMANTIC_ATTRIBUTE); }
	inline string ReadNodeName(xmlNode* node) { return ReadNodeProperty(node, DAE_NAME_ATTRIBUTE); }
	inline string ReadNodeSource(xmlNode* node) { return ReadNodeProperty(node, DAE_SOURCE_ATTRIBUTE); }
	inline string ReadNodeStage(xmlNode* node) { return ReadNodeProperty(node, DAE_STAGE_ATTRIBUTE); }
	FCOLLADA_EXPORT FUUri ReadNodeUrl(xmlNode* node, const char* attribute=DAE_URL_ATTRIBUTE);
	FCOLLADA_EXPORT uint32 ReadNodeCount(xmlNode* node);
	FCOLLADA_EXPORT uint32 ReadNodeStride(xmlNode* node);

	// Pre-buffer the children of a node, with their ids, for performance optimization
	FCOLLADA_EXPORT void ReadChildrenIds(xmlNode* node, FUXmlNodeIdPairList& pairs);

	// Skip the pound(#) character from a COLLADA id string
	FCOLLADA_EXPORT const char* SkipPound(const string& id);
};

#endif // HAS_LIBXML

#endif // _FU_DAE_PARSER_

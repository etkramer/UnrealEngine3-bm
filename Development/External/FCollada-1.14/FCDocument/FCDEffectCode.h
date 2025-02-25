/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDEffectCode.h
	This file contains the FCDEffectCode class.
*/

#ifndef _FCD_EFFECT_CODE_H_
#define _FCD_EFFECT_CODE_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_

class FCDocument;

/**
	A COLLADA code inclusion.

	Code inclusions come in two forms: file includes and inline code.
	For file includes, you will want to grab the filename of the file
	using the GetFilename function and for inline code, you can get
	the code directly through the GetCode function.

	Code inclusions are referenced through sub-ids by the effect pass
	shaders. Regardless of the extension of the filename of file
	includes, the code inclusions' language is solely determined by
	the effect profile they belong to.
*/
class FCDEffectCode : public FCDObject
{
public:
	/** The list of support code inclusion types. */
	enum Type
	{
		INCLUDE, /** A file include. @see GetFilename */
		CODE /** Inlined code. @see GetCode */
	};

private:
	DeclareObjectType(FCDObject);

	Type type;
	string sid;
	fstring code;
	fstring filename;

public:
	/** Constructor: do not use directly.
		Instead, use the FCDEffectProfile::AddCode
		or the FCDEffectTechnique::AddCode functions.
		@param document The COLLADA document that owns this code inclusion. */
	FCDEffectCode(FCDocument* document);

	/** Destructor. */
	virtual ~FCDEffectCode();

	/** Retrieves the form of the code inclusion.
		@return The form. */
	inline Type GetType() const { return type; }

	/** Sets the form of the code inclusion.
		Changing the form of the code inclusion will not
		remove the inline code or the filename.
		@param _type The form. */
	inline void SetType(Type _type) { type = _type; SetDirtyFlag(); }

	/** Retrieves the sub-id of the code inclusion.
		Used to match the code inclusion within the effect pass shaders.
		@return The sub-id. */
	inline const string& GetSubId() const { return sid; }

	/** Sets the sub-id of the code inclusion.
		This value may change on export, as the sub-id must be unique within its scope.
		@param _sid The sub-id. */
	inline void SetSubId(const string& _sid) { sid = _sid; SetDirtyFlag(); }

	/** Retrieves the inlined code.
		First verify that this code inclusion contains inlined code
		using the GetType function.
		@return The inlined code. */
	inline const fstring& GetCode() const { return code; }

	/** Sets the inlined code.
		As a side-effect, calling this function forces the type of the code inclusion.
		@param _code The inlined code. */
	inline void SetCode(const fstring& _code) { code = _code; type = CODE; SetDirtyFlag(); }

	/** Retrieves the filename of the code file to open.
		First verify that this code inclusion contains a filename
		using the GetType function.
		@return The code filename. */
	inline const fstring& GetFilename() const { return filename; }

	/** Sets the filename of the code file.
		As a side-effect, calling this function forces the type of the code inclusion.
		@param _filename The code filename. */
	inline void SetFilename(const fstring& _filename) { filename = _filename; type = INCLUDE; SetDirtyFlag(); }

	/** Clones the code inclusion.
		@param clone The cloned code inclusion.
			If this pointer is NULL, a new code inclusion is created and
			you will need to release this new code inclusion.
		@return The cloned effect object. This pointer will never be NULL. */
	FCDEffectCode* Clone(FCDEffectCode* clone = NULL) const;

	/** [INTERNAL] Reads in the code inclusion from a given COLLADA XML tree node.
		Code inclusions cover the \<code\> element and the \<include\> element.
		@param codeNode The COLLADA XML tree node.
		@return The status of the import. If the status is not successful,
			it may be dangerous to extract information from the code inclusion.*/
	FUStatus LoadFromXML(xmlNode* codeNode);

	/** [INTERNAL] Writes out the code inclusion to the given COLLADA XML tree node.
		@param parentNode The COLLADA XML parent node in which to insert the code inclusion.
		@return The created element XML tree node. */
	xmlNode* WriteToXML(xmlNode* parentNode) const;
};

#endif // _FCD_EFFECT_CODE_H_

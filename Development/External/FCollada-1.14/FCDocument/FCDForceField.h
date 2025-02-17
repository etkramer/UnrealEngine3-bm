/*
	Copyright (C) 2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _FCD_FORCE_FIELD_H_
#define _FCD_FORCE_FIELD_H_

class FCDocument;
class FCDExtra;

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_

/**
	A COLLADA physics force field.

	This class does not have any parameters in the COMMON profile.
	You can use the custom extra tree to enter/retrieve your
	specific customized information.

	@ingroup FCDocument
*/

class FCDForceField : public FCDEntity
{
private:
	DeclareObjectType(FCDEntity);
	FUObjectRef<FCDExtra> information;

public:
	/** Constructor.
		@param document The COLLADA document that owns the force field. */
	FCDForceField(FCDocument* document);

	/** Destructor. */
	~FCDForceField();

	/** Retrieves the extra tree for all the force field information.
		@return The extra tree. */
	FCDExtra* GetInformation() { return information; }
	const FCDExtra* GetInformation() const { return information; }

	/** Retrieves the entity class type.
		@return The entity class type: FORCE_FIELD */
	virtual Type GetType() const { return FORCE_FIELD; }

	/** [INTERNAL] Reads in the force field from a given COLLADA XML tree node.
		@param node The COLLADA XML tree node.
		@return The status of the import. If the status is not successful,
			it may be dangerous to extract information from the force field.*/
	virtual FUStatus LoadFromXML(xmlNode* node);

	/** [INTERNAL] Writes out the \<geometry\> element to the given COLLADA XML tree node.
		@param parentNode The COLLADA XML parent node in which to insert the geometry information.
		@return The created \<geometry\> element XML tree node. */
	virtual xmlNode* WriteToXML(xmlNode* parentNode) const;
};

#endif // _FCD_FORCE_FIELD_H_
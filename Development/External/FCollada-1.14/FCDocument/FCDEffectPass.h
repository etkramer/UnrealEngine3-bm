/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDEffectPass.h
	This file contains the FCDEffectPass class.
*/

#ifndef _FCD_EFFECT_PASS_H_
#define _FCD_EFFECT_PASS_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_

class FCDEffectTechnique;
class FCDEffectParameter;
class FCDEffectPassShader;

typedef FUObjectContainer<FCDEffectPassShader> FCDEffectPassShaderContainer; /**< A dynamically-sized containment array for shaders. */

/**
	A COLLADA effect pass.

	The effect pass contains a list of effect shaders. While they
	may be missing, it does not make sense for the effect pass to
	contain more than two shaders: a vertex shader and a fragment/pixel shader.

	For this reason, we provide the GetVertexShader and the GetFragmentShader
	which we expect will be used for most applications, rather than looking
	through the list of shader objects.
	
	@ingroup FCDEffect
*/
class FCOLLADA_EXPORT FCDEffectPass : public FCDObject
{
private:
	DeclareObjectType(FCDObject);
	fstring name;
	FCDEffectTechnique* parent;
	FCDEffectPassShaderContainer shaders;

public:
	/** Constructor: do not use directly.
		Instead, use the FCDEffectTechnique::AddPass function.
		@param parent The effect technique that contains this effect pass. */
	FCDEffectPass(FCDEffectTechnique* parent);

	/** Destructor. */
	virtual ~FCDEffectPass();

	/** Retrieves the effect techniques which contains this effect pass.
		@return The parent technique. */
	FCDEffectTechnique* GetParent() { return parent; }
	const FCDEffectTechnique* GetParent() const { return parent; } /**< See above. */

	/** Retrieves the COLLADA id of the parent effect.
		This function is mostly useful as a shortcut for debugging and reporting.
		@return The COLLADA id of the parent effect. */
	const string& GetDaeId() const;

	/** Retrieves the sub-id of the effect pass.
		This sub-id is optional.
		@return The sub-id. */
	const fstring& GetPassName() const { return name; }

	/** Sets the optional sub-id for the effect pass.
		This sub-id is optional.
		@param _name The sub-id. */
	void SetPassName(const fstring& _name) { name = _name; SetDirtyFlag(); }

	/** Retrieves the number of shaders contained within the effect pass.
		@return The number of shaders. */
	size_t GetShaderCount() const { return shaders.size(); }

	/** Retrieves a specific shader.
		@param index The index of the shader.
		@return The shader. This pointer will be NULL if the index is out-of-bounds. */
	FCDEffectPassShader* GetShader(size_t index) { FUAssert(index < GetShaderCount(), return NULL); return shaders.at(index); }
	const FCDEffectPassShader* GetShader(size_t index) const { FUAssert(index < GetShaderCount(), return NULL); return shaders.at(index); } /**< See above. */

	/** Adds a new shader to the pass.
		@return The new shader. */
	FCDEffectPassShader* AddShader();

	/** Releases a shader contained within the pass.
		@param shader The shader to release. */
	void ReleaseShader(FCDEffectPassShader* shader);

	/** Retrieves the vertex shader for this effect pass.
		@return The vertex shader. This pointer will be NULL if no
			shader within the pass affects vertices. */
	FCDEffectPassShader* GetVertexShader();
	const FCDEffectPassShader* GetVertexShader() const; /**< See above. */

	/** Retrieves the fragment shader for this effect pass.
		@return The fragment shader. This pointer will be NULL if no
			shader within the pass affects pixels/fragments. */
	FCDEffectPassShader* GetFragmentShader();
	const FCDEffectPassShader* GetFragmentShader() const; /**< See above. */

	/** Adds a new vertex shader to the pass.
		If a vertex shader already exists within the pass, it will be released.
		@return The new vertex shader. */
	FCDEffectPassShader* AddVertexShader();

	/** Adds a new fragment shader to the pass.
		If a fragment shader already exists within the pass, it will be released.
		@return The new fragment shader. */
	FCDEffectPassShader* AddFragmentShader();

	/** Clones the effect pass and shaders.
		@param clone The cloned pass.
			If this pointer is NULL, a new pass is created and
			you will need to release this new pass.
		@return The cloned pass. */
	FCDEffectPass* Clone(FCDEffectPass* clone) const;

	/** [INTERNAL] Reads in the effect pass from a given COLLADA XML tree node.
		@param passNode The COLLADA XML tree node.
		@param techniqueNode X @deprecated bad interface : this dependency must be taken out[3]
		@param profileNode X @deprecated bad interface : this dependency must be taken out[2]
		@return The status of the import. If the status is not successful,
			it may be dangerous to extract information from the effect pass.*/
	FUStatus LoadFromXML(xmlNode* passNode, xmlNode* techniqueNode, xmlNode* profileNode);

	/** [INTERNAL] Writes out the effect pass to the given COLLADA XML tree node.
		@param parentNode The COLLADA XML parent node in which to insert the effect pass.
		@return The created element XML tree node. */
	xmlNode* WriteToXML(xmlNode* parentNode) const;
};

#endif

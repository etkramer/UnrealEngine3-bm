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

/**
	@file FCDTexture.h
	This file contains the FCDTexture class.
*/

#ifndef _FCD_TEXTURE_H_
#define _FCD_TEXTURE_H_

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_
#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_

class FCDocument;
class FCDEffectParameter;
class FCDEffectParameterInt;
class FCDEffectParameterList;
class FCDEffectParameterSampler;
class FCDEffectStandard;
class FCDImage;

/**
	A COLLADA texture.
	
	Textures are used by the COMMON profile materials. For COLLADA 1.3 backward
	compatibility, the textures may also be entities contained within
	the material library. As per the COLLADA 1.4 specification, a texture is
	used to match some texture coordinates with a surface sampler, on a given
	texturing channel.

	Therefore: textures hold the extra information necessary to place an image correctly onto
	polygon sets. This extra information includes the texturing coordinate transformations and
	the blend mode.

	@ingroup FCDEffect
*/
class FCOLLADA_EXPORT FCDTexture : public FCDEntity
{
private:
	DeclareObjectType(FCDEntity);

	FCDEffectStandard* parent;
	FUDaeTextureChannel::Channel textureChannel; // COLLADA 1.3 backward compatibility
	FUObjectPtr<FCDEffectParameterSampler> sampler; // Points to the surface, which points to the image.
	FUObjectRef<FCDEffectParameterInt> set; // Always preset, this parameter hold the map channel/uv set index
	FUObjectRef<FCDExtra> extra;

	string samplerSid; // Used during the import-only: this sid will be resolved at link-time.

public:
	/** Constructor: do not use directly.
		Instead, use the FCDEffectStandard::AddTexture function.
		@param document The COLLADA document that owns this texture. */
	FCDTexture(FCDocument* document, FCDEffectStandard* parent = NULL);

	/** Destructor: do not use directly.
		Instead, use the FCDEffectStandard::ReleaseTexture function. */
	virtual ~FCDTexture();

	/** Retrieves the type of the entity class.
		This function is part of the FCDEntity interface and is used
		for COLLADA 1.3 backward compatibility.
		@return The entity class type: TEXTURE. */
	virtual Type GetType() const { return TEXTURE; }

	/** Retrieves which texture channel to use for this texture.
		As it is directly tied in with which buckets holds this texture, you
		cannot modify this value. Instead, create a new texture in the correct
		bucket of the FCDEffectStandard object.
		@return The texture channel. */
	inline FUDaeTextureChannel::Channel GetTextureChannel() const { return textureChannel; }

	/** Retrieves the image information for this texture.
		@return The image. This pointer will be NULL if this texture is not yet
			tied to a valid image. */
	inline FCDImage* GetImage() { return const_cast<FCDImage*>(const_cast<const FCDTexture*>(this)->GetImage()); }
	const FCDImage* GetImage() const; /**< See above. */

	/** Set the image information for this texture.
		This is a shortcut that generates the sampler/surface parameters
		to access the given image.
		@parameter image The image information. This pointer may be
			NULL to disconnect an image. */
	void SetImage(FCDImage* image);

	/** Retrieves the surface sampler for this texture.
		@return The sampler. In the non-const method: the sampler
			will be created if it is currently missing and the parent is available. */
	FCDEffectParameterSampler* GetSampler();
	const FCDEffectParameterSampler* GetSampler() const { return sampler; } /**< See above. */

	/** Retrieves the texture coordinate set to use with this texture.
		This information is duplicated from the material instance abstraction level.
		@return The effect parameter containing the set. */
	inline FCDEffectParameterInt* GetSet() { return set; }
	inline const FCDEffectParameterInt* GetSet() const { return set; } /**< See above. */

	/** Retrieves the extra information tied to this texture.
		@return The extra tree. */
	inline FCDExtra* GetExtra() { return extra; }
	inline const FCDExtra* GetExtra() const { return extra; } /**< See above. */

	/** Retrieves an effect parameter.
		Looks for the effect parameter with the correct semantic, in order to bind or override its value.
		This function compares the local effect parameters with the given semantic.
		@param semantic The effect parameter semantic to match.
		@return The effect parameter that matches the semantic. This pointer may be
			NULL if no effect parameter matches the given semantic. */
	FCDEffectParameter* FindParameterBySemantic(const string& semantic);

	/** Retrieves a subset of the local effect parameter list.
		Look for local effect parameters with the correct semantic.
		This function searches through the effect profile and the level of abstractions below.
		@param semantic The effect parameter semantic to match.
		@param parameters The list of parameters to fill in. This list is not cleared. */
	void FindParametersBySemantic(const string& semantic, FCDEffectParameterList& parameters);

	/** Retrieves a subset of the local effect parameter list.
		Look for the local effect parameter with the correct reference.
		@param reference The effect parameter reference to match.
		@param parameters The list of parameters to fill in. This list is not cleared. */
	void FindParametersByReference(const string& reference, FCDEffectParameterList& parameters);

	/** Clones the texture.
		@param clone The cloned texture. If this pointer is NULL,
			a new texture is created and you will need to release this new texture.
		@return The cloned texture. This pointer will never be NULL. */
	virtual FCDEntity* Clone(FCDEntity* clone) const;

	/** [INTERNAL] Reads in the texture from a given COLLADA XML tree node.
		@param textureNode The COLLADA XML tree node.
		@return The status of the import. If the status is not successful,
			it may be dangerous to extract information from the texture.*/
	FUStatus LoadFromTextureXML(xmlNode* textureNode);

	/** [INTERNAL] Reads in the texture from a given COLLADA XML tree node.
		This function is useful only for COLLADA 1.3 backward compatibility.
		@param textureNode The COLLADA XML tree node.
		@return The status of the import. If the status is not successful,
			it may be dangerous to extract information from the texture.*/
	virtual FUStatus LoadFromXML(xmlNode* textureNode);

	/** [INTERNAL] Writes out the texture to the given COLLADA XML tree node.
		@param parentNode The COLLADA XML parent node in which to insert the texture.
		@return The created element XML tree node. */
	virtual xmlNode* WriteToXML(xmlNode* parentNode) const;

	/** [INTERNAL] During the import, link the texture with the samplers.
		@param parameters The list of parameters available at this abstraction level. */
	void Link(FCDEffectParameterList& parameters);
};

#endif // _FCD_TEXTURE_H_

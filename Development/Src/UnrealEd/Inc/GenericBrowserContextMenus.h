/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __GENERICBROWSERCONTEXTMENUS_H__
#define __GENERICBROWSERCONTEXTMENUS_H__

class WxMBGenericBrowserContextBase : public wxMenu
{
public:
	/** Enables/disables menu items supported by the specified selection set. */
	virtual void SetObjects(const class USelection* Selection);

protected:
	/** Enables/disables menu items based on whether the selection set contains cooked objects. */
	virtual void ToggleItemsForCookedContent(UBOOL bSomeObjectsAreCooked) {}
};

/**
 * Baseclass for all generic browser type context menus.
 */
class WxMBGenericBrowserContext : public WxMBGenericBrowserContextBase
{
public:
	typedef WxMBGenericBrowserContextBase Super;
	WxMBGenericBrowserContext();

	void AppendObjectMenu();
	void AppendLocObjectMenu();

protected:
	wxMenuItem* ExportItem;
	wxMenuItem* DuplicateItem;
	wxMenuItem* RenameItem;
	wxMenuItem* RenameLocItem;
	wxMenuItem* DeleteItem;
	wxMenuItem* DeleteWithReferencesItem;

	/** Enables/disables menu items based on whether the selection set contains cooked objects. */
	virtual void ToggleItemsForCookedContent(UBOOL bSomeObjectsAreCooked);
};

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_Object
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_Object : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_Object();
};

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_Archetype.
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_Archetype : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_Archetype();
};

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_Material.
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_Material : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_Material();

protected:
	wxMenuItem* CreateNewMaterialItem;
	wxMenuItem* CreateNewMaterialTimeVaryingItem;

	/** Enables/disables menu items based on whether the selection set contains cooked objects. */
	virtual void ToggleItemsForCookedContent(UBOOL bSomeObjectsAreCooked);
};

/*-----------------------------------------------------------------------------
WxMBGenericBrowserContext_MaterialInstanceConstant
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_MaterialInstanceConstant : public WxMBGenericBrowserContext_Material
{
public:
	typedef WxMBGenericBrowserContext_Material Super;
};


/*-----------------------------------------------------------------------------
WxMBGenericBrowserContext_MaterialInstanceTimeVarying
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_MaterialInstanceTimeVarying : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_MaterialInstanceTimeVarying();

protected:
	wxMenuItem* CreateNewMaterialTimeVaryingItem;

	/** Enables/disables menu items based on whether the selection set contains cooked objects. */
	virtual void ToggleItemsForCookedContent(UBOOL bSomeObjectsAreCooked);
};



/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_Texture.
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_Texture : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_Texture();

protected:
	wxMenuItem* ReimportTextItem;
	wxMenuItem* CreateNewMaterialItem;

	/** Enables/disables menu items based on whether the selection set contains cooked objects. */
	virtual void ToggleItemsForCookedContent(UBOOL bSomeObjectsAreCooked);
};

/*-----------------------------------------------------------------------------
WxMBGenericBrowserContext_RenderTexture
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_RenderTexture : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_RenderTexture();

protected:
	wxMenuItem* CreateStaticTextureItem;

	/** Enables/disables menu items based on whether the selection set contains cooked objects. */
	virtual void ToggleItemsForCookedContent(UBOOL bSomeObjectsAreCooked);
};

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_StaticMesh.
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_StaticMesh : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_StaticMesh();

	/** Enables/disables menu items supported by the specified selection set. */
	void SetObjects(const class USelection* Selection);
};

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_FracturedStaticMesh.
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_FracturedStaticMesh : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_FracturedStaticMesh();
};

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_Sound.
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_Sound : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_Sound();
};

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_SoundCue.
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_SoundCue : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_SoundCue();

protected:
	wxMenuItem* EditNodesItem;

	/** Enables/disables menu items based on whether the selection set contains cooked objects. */
	virtual void ToggleItemsForCookedContent(UBOOL bSomeObjectsAreCooked);
};

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_SoundMode.
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_SoundMode : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_SoundMode();
};

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_SpeechRecognition.
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_SpeechRecognition : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_SpeechRecognition();
};

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_PhysicsAsset.
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_PhysicsAsset : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_PhysicsAsset();
};

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_FaceFXAsset.
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_FaceFXAsset : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	typedef WxMBGenericBrowserContext Super;

	WxMBGenericBrowserContext_FaceFXAsset();

protected:
	wxMenuItem* CreateFaceFXAnimSetItem;
	wxMenuItem* ImportFromFXAItem;
	wxMenuItem* ExporttoFXAItem;

	/** Enables/disables menu items based on whether the selection set contains cooked objects. */
	virtual void ToggleItemsForCookedContent(UBOOL bSomeObjectsAreCooked);
};

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_FaceFXAnimSet.
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_FaceFXAnimSet : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_FaceFXAnimSet();
};

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_Skeletal.
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_Skeletal : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_Skeletal();

protected:
	wxMenuItem* CreateNewPhysicsAssetItem;
	wxMenuItem* CreateNewFaceFXAssetItem;

	/** Enables/disables menu items based on whether the selection set contains cooked objects. */
	virtual void ToggleItemsForCookedContent(UBOOL bSomeObjectsAreCooked);

};

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_LensFlare
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_LensFlare : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_LensFlare();
};


/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_ParticleSystem.
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_ParticleSystem : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_ParticleSystem();
};

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_PhysXParticleSystem.
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_PhysXParticleSystem : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_PhysXParticleSystem();
};

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_AnimSet.
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_AnimSet : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_AnimSet();
};

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_MorphTargetSet.
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_MorphTargetSet : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_MorphTargetSet();
};

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_AnimTree.
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_AnimTree : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_AnimTree();
};

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_PostProcess.
-----------------------------------------------------------------------------*/

class WxMBGenericBrowserContext_PostProcess : public WxMBGenericBrowserContext
{
public:
	typedef WxMBGenericBrowserContext Super;
	WxMBGenericBrowserContext_PostProcess();
};

#endif // __GENERICBROWSERCONTEXTMENUS_H__

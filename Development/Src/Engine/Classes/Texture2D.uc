/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Texture2D extends Texture
	native
	hidecategories(Object);

/**
 * A mip-map of the texture.
 */
struct native Texture2DMipMap
{
	var native TextureMipBulkData_Mirror Data{FTextureMipBulkData};	
	var native int SizeX;
	var native int SizeY;

	structcpptext
	{
		/**
		 * Special serialize function passing the owning UObject along as required by FUnytpedBulkData
		 * serialization.
		 *
		 * @param	Ar		Archive to serialize with
		 * @param	Owner	UObject this structure is serialized within
		 * @param	MipIdx	Current mip being serialized
		 */
		void Serialize( FArchive& Ar, UObject* Owner, INT MipIdx );
	}
};

/** The texture's mip-map data.												*/
var native const IndirectArray_Mirror Mips{TIndirectArray<FTexture2DMipMap>};

/** The width of the texture.												*/
var const int SizeX;

/** The height of the texture.												*/
var const int SizeY;

/** The format of the texture data.											*/
var const EPixelFormat Format;

/** The addressing mode to use for the X axis.								*/
var() TextureAddress AddressX;

/** The addressing mode to use for the Y axis.								*/
var() TextureAddress AddressY;

/** Whether the texture is currently streamable or not.						*/
var transient const bool						bIsStreamable;
/** Whether the current texture mip change request is pending cancelation.	*/
var transient const bool						bHasCancelationPending;
/**
 * Whether the texture has been loaded from a persistent archive. We keep track of this in order to not stream 
 * textures that are being re-imported over as they will have a linker but won't have been serialized from disk 
 * and are therefore not streamable.
 */
var transient const bool						bHasBeenLoadedFromPersistentArchive;

/** Override whether to fully stream even if texture hasn't been rendered.	*/
var transient bool								bForceMiplevelsToBeResident;
/** Global/ serialized version of ForceMiplevelsToBeResident.				*/
var() const bool								bGlobalForceMipLevelsToBeResident;
/** WorldInfo timestamp that tells the streamer to force all miplevels to be resident up until that time. */ 
var private transient float						ForceMipLevelsToBeResidentTimestamp;

/** Name of texture file cache texture mips are stored in, NAME_None if it is not part of one. */
var		name									TextureFileCacheName;
/** ID generated whenever the texture is changed so that its bulk data can be updated in the TextureFileCache during cook */
var native const guid							TextureFileCacheGuid;

/** Number of miplevels the texture should have resident.					*/
var transient const int							RequestedMips;
/** Number of miplevels currently resident.									*/
var transient const int							ResidentMips;
/**
 * Thread safe counter indicating status of mip change request.	The below defines are mirrored in UnTex.h.
 *
 * >=  3 == TEXTURE_STATUS_REQUEST_IN_FLIGHT	- a request has been kicked off and is in flight
 * ==  2 == TEXTURE_READY_FOR_FINALIZATION		- initial request has completed and finalization needs to be kicked off
 * ==  1 == TEXTURE_FINALIZATION_IN_PROGRESS	- finalization has been kicked off and is in progress
 * ==  0 == TEXTURE_READY_FOR_REQUESTS			- there are no pending requests/ all requests have been fulfilled
 * == -1 == TEXTURE_PENDING_INITIALIZATION		- the renderer hasn't created the resource yet
 */
var native transient const ThreadSafeCounter	PendingMipChangeRequestStatus{mutable FThreadSafeCounter};

/**
 * Mirror helper structure for linked list of texture objects. The linked list should NOT be traversed by the
 * garbage collector, which is why Element is declared as a pointer.
 */
struct TextureLinkedListMirror
{
	var native const POINTER Element;
	var native const POINTER Next;
	var native const POINTER PrevLink;
};

/** This texture's link in the global streamable texture list. */
var private{private} native const duplicatetransient noimport TextureLinkedListMirror StreamableTexturesLink{TLinkedList<UTexture2D*>};

/** 
* Keep track of the first mip level stored in the packed miptail.
* it's set to highest mip level if no there's no packed miptail 
*/
var const int MipTailBaseIdx; 

/** memory used for directly loading bulk mip data */
var private const native transient pointer		ResourceMem{FTexture2DResourceMem};
/** keep track of first mip level used for ResourceMem creation */
var private const native transient int			FirstResourceMemMip;

/** Tells the streaming system that it should force all mip-levels to be resident for a number of seconds. */
native final function							SetForceMipLevelsToBeResident( float Seconds );

cpptext
{
	// Static prviate variables.
private:
	/** First streamable texture link. Not handled by GC as BeginDestroy automatically unlinks.	*/
	static TLinkedList<UTexture2D*>* FirstStreamableLink;
	/** Current streamable texture link for iteration over textures. Not handled by GC as BeginDestroy automatically unlinks. */
	static TLinkedList<UTexture2D*>* CurrentStreamableLink;
	/** Number of streamable textures. */
	static INT NumStreamableTextures;

public:

	// UObject interface.
	virtual void InitializeIntrinsicPropertyValues();
	virtual void Serialize(FArchive& Ar);
#if !CONSOLE
	// SetLinker is only virtual on consoles.
	virtual void SetLinker( ULinkerLoad* L, INT I );
#endif
	/**
	 * Called after the garbage collection mark phase on unreachable objects.
	 */
	virtual void BeginDestroy();
	/**
 	 * Called after object and all its dependencies have been serialized.
	 */
	virtual void PostLoad();
	/**
 	 * Called after object has been duplicated.
	 */
	virtual void PostDuplicate();

	/** 
	 * Generates a GUID for the texture if one doesn't already exist. 
	 *
	 * @param bForceGeneration	Whether we should generate a GUID even if it is already valid.
	 */
	void GenerateTextureFileCacheGUID(UBOOL bForceGeneration=FALSE);

	// USurface interface
	virtual FLOAT GetSurfaceWidth() const { return SizeX; }
	virtual FLOAT GetSurfaceHeight() const { return SizeY; }

	// UTexture interface.
	virtual FTextureResource* CreateResource();
	virtual void Compress();
	virtual EMaterialValueType GetMaterialType() { return MCT_Texture2D; }
	/**
	 * Creates a new resource for the texture, and updates any cached references to the resource.
	 */
	virtual void UpdateResource();

	// UTexture2D interface.
	void Init(UINT InSizeX,UINT InSizeY,EPixelFormat InFormat);
	void LegacySerialize(FArchive& Ar);

	/** 
	 * Returns a one line description of an object for viewing in the thumbnail view of the generic browser
	 */
	virtual FString GetDesc();

	/** 
	 * Returns detailed info to populate listview columns
	 */
	virtual FString GetDetailedDescription( INT InIndex );

	/**
	 * Returns the size of this texture in bytes if it had MipCount miplevels streamed in.
	 *
	 * @param	MipCount	Number of toplevel mips to calculate size for
	 * @return	size of top mipcount mips in bytes
	 */
	INT GetSize( INT MipCount ) const;

	/**
	 * Returns the size of this texture in bytes on 360 if it had MipCount miplevels streamed in.
	 *
	 * @param	MipCount	Number of toplevel mips to calculate size for
	 * @return	size of top mipcount mips in bytes
	 */
	INT Get360Size( INT MipCount ) const;

	/**
	 * Returns whether the texture is ready for streaming aka whether it has had InitRHI called on it.
	 *
	 * @return TRUE if initialized and ready for streaming, FALSE otherwise
	 */
	UBOOL IsReadyForStreaming();

	/**
	 * Updates the streaming status of the texture and performs finalization when appropriate. The function returns
	 * TRUE while there are pending requests in flight and updating needs to continue.
	 *
	 * @return	TRUE if there are requests in flight, FALSE otherwise
	 */
	virtual UBOOL UpdateStreamingStatus();

	/**
	 * Tries to cancel a pending mip change request. Requests cannot be canceled if they are in the
	 * finalization phase.
	 *
	 * @param	TRUE if cancelation was successful, FALSE otherwise
	 */
	UBOOL CancelPendingMipChangeRequest();

	/**
	 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
	 *
	 * @return size of resource as to be displayed to artists/ LDs in the Editor.
	 */
	virtual INT GetResourceSize();

	/**
	 * Returns whether miplevels should be forced resident.
	 *
	 * @return TRUE if either transient or serialized override requests miplevels to be resident, FALSE otherwise
	 */
	UBOOL ShouldMipLevelsBeForcedResident() const
	{
		return bGlobalForceMipLevelsToBeResident || bForceMiplevelsToBeResident;
	}

	/**
	 * Whether all miplevels of this texture have been fully streamed in, LOD settings permitting.
	 */
	UBOOL IsFullyStreamedIn();

	/**
	 * Returns a reference to the global list of streamable textures.
	 *
	 * @return reference to global list of streamable textures.
	 */
	static TLinkedList<UTexture2D*>*& GetStreamableList();

	/**
	 * Returns a reference to the current streamable link.
	 *
	 * @return reference to current streamable link
	 */
	static TLinkedList<UTexture2D*>*& GetCurrentStreamableLink();

	/**
	 * Links texture to streamable list and updates streamable texture count.
	 */
	void LinkStreaming();

	/**
	 * Unlinks texture from streamable list, resets CurrentStreamableLink if it matches
	 * StreamableTexturesLink and also updates the streamable texture count.
	 */
	void UnlinkStreaming();
	
	/**
	 * Returns the number of streamable textures, maintained by link/ unlink code
	 *
	 * @return	Number of streamable textures
	 */
	static INT GetNumStreamableTextures();

	/**
	 * Cancels any pending texture streaming actions if possible.
	 * Returns when no more async loading requests are in flight.
	 */
	static void CancelPendingTextureStreaming();

	/**
	* Initialize the GPU resource memory that will be used for the bulk mip data
	* This memory is allocated based on the SizeX,SizeY of the texture and the first mip used
	*
	* @param FirstMipIdx first mip that will be resident	
	* @return FTexture2DResourceMem container for the allocated GPU resource mem
	*/
	class FTexture2DResourceMem* InitResourceMem(INT FirstMipIdx);
}

/** creates and initializes a new Texture2D with the requested settings */
static native noexport final function Texture2D Create(int InSizeX, int InSizeY, optional EPixelFormat InFormat = PF_A8R8G8B8);

defaultproperties
{
}

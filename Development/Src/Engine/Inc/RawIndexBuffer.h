/*=============================================================================
	RawIndexBuffer.h: Raw index buffer definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

class FRawIndexBuffer : public FIndexBuffer
{
public:

	TArray<WORD> Indices;

	/**
	 * Converts a triangle list into a triangle strip.
	 */
	INT Stripify();

	/**
	 * Orders a triangle list for better vertex cache coherency.
	 */
	void CacheOptimize();

	// FRenderResource interface.
	virtual void InitRHI();

	// Serialization.
	friend FArchive& operator<<(FArchive& Ar,FRawIndexBuffer& I);
};

class FRawIndexBuffer32 : public FIndexBuffer
{
public:

	TArray<DWORD> Indices;

	// FRenderResource interface.
	virtual void InitRHI();

	// Serialization.
	friend FArchive& operator<<(FArchive& Ar,FRawIndexBuffer32& I);
};

class FRawStaticIndexBuffer : public FIndexBuffer
{
public:
	TResourceArray<WORD,INDEXBUFFER_ALIGNMENT> Indices;

	/**
	* Constructor
	* @param InNeedsCPUAccess - TRUE if resource array data should be CPU accessible
	*/
	FRawStaticIndexBuffer(UBOOL InNeedsCPUAccess=FALSE)
	:	Indices(InNeedsCPUAccess)
	,	NumVertsPerInstance(0)
	,	bSetupForInstancing(FALSE)
	{
	}

	/**
	* Create the index buffer RHI resource and initialize its data
	*/
	virtual void InitRHI();

	/**
	* Serializer for this class
	* @param Ar - archive to serialize to
	* @param I - data to serialize
	*/
	friend FArchive& operator<<(FArchive& Ar,FRawStaticIndexBuffer& I);

	/**
	* Converts a triangle list into a triangle strip.
	*/
	INT Stripify();

	/**
	* Orders a triangle list for better vertex cache coherency.
	*/
	void CacheOptimize();

	/** Sets the index buffer up for hardware vertex instancing. */
	void SetupForInstancing(UINT InNumVertsPerInstance)
	{
		bSetupForInstancing = TRUE;
		NumVertsPerInstance = InNumVertsPerInstance;
	}

	UINT GetNumVertsPerInstance() const	{ return NumVertsPerInstance; }
	UBOOL GetSetupForInstancing() const	{ return bSetupForInstancing; }

private:
	UINT			NumVertsPerInstance;  // provided by the user (often 1+ the max index in the buffer, but not always)
	UBOOL			bSetupForInstancing;
};


/**
 *	FRawGPUIndexBuffer represents a basic index buffer GPU resource with
 *	no CPU-side data.
 *	The member functions are meant to be called from the renderthread only.
 */
class FRawGPUIndexBuffer : public FIndexBuffer
{
public:
	/**
	 *	Default constructor
	 */
	FRawGPUIndexBuffer();

	/**
	 *	Setup constructor
	 *	@param InNumIndices		- Number of indices to allocate space for
	 *	@param InIsDynamic		- TRUE if the index buffer should be dynamic
	 *	@param InStride			- Number of bytes per index
	 */
	FRawGPUIndexBuffer(UINT InNumIndices, UBOOL InIsDynamic=FALSE, UINT InStride=sizeof(WORD));

	/**
	 *	Sets up the index buffer, if the default constructor was used.
	 *	@param InNumIndices		- Number of indices to allocate space for
	 *	@param InIsDynamic		- TRUE if the index buffer should be dynamic
	 *	@param InStride			- Number of bytes per index
	 */
	void Setup(UINT InNumIndices, UBOOL InIsDynamic=FALSE, UINT InStride=sizeof(WORD));

	/**
	 *	Returns TRUE if the index buffer hasn't been filled in with data yet,
	 *	or if it's a dynamic resource that has been re-created due to Device Lost.
	 *	Calling Lock + Unlock will make the index buffer non-empty again.
	 */
	UBOOL IsEmpty() const
	{
		return bIsEmpty;
	}

	/**
	 *	Returns the number of indices that are allocated in the buffer.
	 */
	UINT GetNumIndices() const
	{
		return NumIndices;
	}

	/**
	 *	Create an empty static index buffer RHI resource.
	 */
	virtual void InitRHI();

	/**
	 *	Releases a dynamic index buffer RHI resource.
	 *	Called when the resource is released, or when reseting all RHI resources.
	 */
	virtual void ReleaseRHI();

	/**
	 *	Create an empty dynamic index buffer RHI resource.
	 */
	virtual void InitDynamicRHI();

	/**
	 *	Releases a dynamic index buffer RHI resource.
	 *	Called when the resource is released, or when reseting all RHI resources.
	 */
	virtual void ReleaseDynamicRHI();

	/**
	 *	Locks the index buffer and returns a pointer to the first index in the locked region.
	 *
	 *	@param FirstIndex		- First index in the locked region. Defaults to the first index in the buffer.
	 *	@param NumIndices		- Number of indices to lock. Defaults to the remainder of the buffer.
	 */
	void*	Lock( UINT FirstIndex=0, UINT NumIndices=0 );

	/**
	 *	Unlocks the index buffer.
	 */
	void	Unlock( );

protected:
	UINT	NumIndices;
	UINT	Stride;
	UBOOL	bIsDynamic;
	UBOOL	bIsEmpty;
};

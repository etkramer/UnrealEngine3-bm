/*=============================================================================
	XeD3DResourceArray.h: Xe D3D Resource array definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/** alignment for supported resource types */
enum EResourceAlignment
{
	VERTEXBUFFER_ALIGNMENT = D3DVERTEXBUFFER_ALIGNMENT,
	INDEXBUFFER_ALIGNMENT = D3DINDEXBUFFER_ALIGNMENT
};

#define TResourceArray TXeD3DResourceArray

/** The Xe GPU resource allocation policy always allocates the elements indirectly. */
template<DWORD Alignment = DEFAULT_ALIGNMENT>
class TXeGPUResourceAllocator
{
public:

	enum { NeedsElementType = TRUE };

	template<typename ElementType>
	class ForElementType
	{
	public:

		/** Default constructor. */
		ForElementType():
			Data(NULL)
		{}

		/** ENoInit constructor. */
		ForElementType(ENoInit)
		{}

		/** Destructor. */
		~ForElementType()
		{
			if( Data )
			{
				appPhysicalFree(Data);
				Data = NULL;
			}
		}

		// FContainerAllocatorInterface
		FORCEINLINE ElementType* GetAllocation() const
		{
			return Data;
		}
		void ResizeAllocation(
			INT PreviousNumElements,
			INT NumElements,
			INT NumBytesPerElement
			)
		{
			if( Data )
			{
				appPhysicalFree(Data);
				Data = NULL;
			}

			if (NumElements > 0)
			{
				Data = (ElementType*)appPhysicalAlloc(NumElements * NumBytesPerElement, bNeedsCPUAccess ? CACHE_Normal : CACHE_WriteCombine);
			}
		}
		INT CalculateSlack(
			INT NumElements,
			INT CurrentNumSlackElements,
			INT NumBytesPerElement
			) const
		{
			// Don't allocate slack for GPU resources.
			return NumElements;
		}

		UBOOL GetAllowCPUAccess() const
		{
			return bNeedsCPUAccess;
		}

		void SetAllowCPUAccess(UBOOL bInNeedsCPUAccess)
		{
			bNeedsCPUAccess = bInNeedsCPUAccess;
		}

	private:

		/** A pointer to the container's elements. */
		ElementType* Data;

		/** Whether the elements need to be accessed by the CPU. */
		UBOOL bNeedsCPUAccess;
	};
	
	typedef ForElementType<FScriptContainerElement> ForAnyElementType;
};

/**
* Resource array that uses physically contiguous memory so that its data
* can be used in-place by a D3D resource.  Resource data is serialized directly 
* into the physical allocation and the array data can not be changed after that.
*
* @param Alignment - memory alignment to use for the allocation
*/
template< typename ElementType, DWORD Alignment=DEFAULT_ALIGNMENT >
class TXeD3DResourceArray
:	public FResourceArrayInterface
,	public TArray<ElementType,TXeGPUResourceAllocator<Alignment> >
{
public:
	typedef TXeGPUResourceAllocator<Alignment> Allocator;
	typedef TArray<ElementType,Allocator> Super;

	/** 
	* Constructor 
	*/
	TXeD3DResourceArray(UBOOL bInNeedsCPUAccess=FALSE)
	:	TArray()
	{
		this->AllocatorInstance.SetAllowCPUAccess(bInNeedsCPUAccess);
	}

	// FResourceArrayInterface

	/**
	* Access the resource data
	* @return ptr to start of resource data array
	*/
	virtual const void* GetResourceData() const 
	{ 
		return this->GetData(); 
	}

	/**
	 * @return size of resource data allocation
	 */
	virtual DWORD GetResourceDataSize() const
	{
		return Super::Num() * sizeof(ElementType);
	}
	
	/**
	* Called after resource for this array has been created.
	* Always keep resource array data on Xe since it is used directly as a D3D resource
	*/
	virtual void Discard()
	{
	}

	/**
	 * @return TRUE if the resource array is static and shouldn't be modified
	 */
	virtual UBOOL IsStatic() const
	{
		// resource arrays are always static
		return TRUE;
	}

	/**
	* @return TRUE if the resource keeps a copy of its resource data after the RHI resource has been created
	*/
	virtual UBOOL GetAllowCPUAccess() const
	{
		return this->AllocatorInstance.GetAllowCPUAccess();
	}

	// FScriptArray interface - don't allow array modification

	void Insert( INT /*Index*/, INT /*Count*/, INT /*ElementSize*/, DWORD /*Alignment*/ )
	{
		check(0);
	}
	INT Add( INT /*Count*/, INT /*ElementSize*/, DWORD /*Alignment*/ )
	{
		check(0); return 0;
	}
	void Empty( INT /*ElementSize*/, DWORD /*Alignment*/, INT /*Slack=0*/ )
	{
		check(0);
	}
	void Remove( INT /*Index*/, INT /*Count*/, INT /*ElementSize*/, DWORD /*Alignment*/ )
	{
		check(0);
	}

	// TArray interface - don't allow array modification

	INT Add( INT /*Count=1*/ )
	{
		check(0);
		return 0;
	}
	INT AddItem( const ElementType& /*Item*/ )
	{
		check(0);
		return 0;
	}
	void Insert( INT /*Index*/, INT /*Count=1*/ )
	{
		check(0);
	}
	INT InsertItem( const ElementType& /*Item*/, INT /*Index*/ )
	{
		check(0);
		return 0;
	}
	void Remove( INT /*Index*/, INT /*Count=1*/ )
	{
		check(0);
	}
	void Empty( INT /*Slack=0*/ )
	{
		check(0);
	}
	void Append(const Super& /*Source*/)
	{
		check(0);
	}	

	// Assignment operators.
	
	/**
	 * Assignment operator. This is currently the only method which allows for 
	 * modifying an existing resource array. These are very slow however for GPU arrays
	 * @todo: Put a printout/breakpoint so we know when this happens?
	 */
	template<typename Allocator>
	TXeD3DResourceArray& operator=(const TArray<ElementType,Allocator>& Other)
	{
		Super::Empty(Other.Num());
		Super::Add(Other.Num());
		appMemcpy(this->GetData(), Other.GetData(), Other.Num() * sizeof(ElementType));
		return *this;
	}

	TXeD3DResourceArray& operator=(const TXeD3DResourceArray<ElementType,Alignment>& Other)
	{
		Super::Empty(Other.Num());
		Super::Add(Other.Num());
		appMemcpy(this->GetData(), Other.GetData(), Other.Num() * sizeof(ElementType));
		return *this;
	}



	/**
	* Serializer for this class
	* @param Ar - archive to serialize to
	* @param ResourceArray - resource array data to serialize
	*/
	friend FArchive& operator<<(FArchive& Ar,TXeD3DResourceArray& ResourceArray)
	{
		ResourceArray.CountBytes( Ar );
		if( sizeof(ElementType)==1 )
		{
			// Serialize simple bytes which require no construction or destruction.
			Ar << ResourceArray.ArrayNum;
			check( ResourceArray.ArrayNum >= 0 );
			if( Ar.IsLoading() )
			{
				ResourceArray.Super::Empty(ResourceArray.ArrayNum);
				ResourceArray.Super::Add(ResourceArray.ArrayNum);
			}
			Ar.Serialize( ResourceArray.GetData(), ResourceArray.Num() );
		}
		else if(Ar.IsLoading())
		{
			// Load array.
			INT NewNum;
			Ar << NewNum;
			ResourceArray.Super::Empty(NewNum);
			ResourceArray.Super::Add(NewNum);
			for (INT i=0; i<NewNum; i++)
			{
				Ar << ResourceArray(i);
			}
		}
		else if( Ar.IsSaving() )
		{
			// no saving allowed/needed for Xe resource arrays
			check(0);
		}
		else
		{			
			Ar << ResourceArray.ArrayNum;
			for( INT i=0; i<ResourceArray.ArrayNum; i++ )
			{
				Ar << ResourceArray(i);
			}
		}
		return Ar;
	}

	/**
	 * Serialize data as a single block. See TArray::BulkSerialize for more info.
	 *
	 * IMPORTANT:
	 *   - This is Overridden from UnTemplate.h TArray::BulkSerialize  Please make certain changes are propogated accordingly
	 *
	 * @param Ar	FArchive to bulk serialize this TArray to/from
	 */
	void BulkSerialize(FArchive& Ar)
	{
		// Serialize element size to detect mismatch across platforms.
		INT SerializedElementSize = 0;
		Ar << SerializedElementSize;

		if( Ar.IsSaving() || Ar.Ver() < GPackageFileVersion || Ar.LicenseeVer() < GPackageFileLicenseeVersion )
		{
			Ar << *this;
		}
		else 
		{
			this->CountBytes(Ar); 
			if( Ar.IsLoading() )
			{
				// Basic sanity checking to ensure that sizes match.
				checkf(SerializedElementSize==0 || SerializedElementSize==sizeof(ElementType),TEXT("Expected %i, Got: %i"),sizeof(ElementType),SerializedElementSize);
				INT NewArrayNum;
				Ar << NewArrayNum;
				Super::Empty(NewArrayNum);
				Super::Add(NewArrayNum);
				Ar.Serialize(this->GetData(), NewArrayNum * sizeof(ElementType));
			}
		}
	}
};


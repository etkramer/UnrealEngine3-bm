/*=============================================================================
	UnTextureLayout.h: Texture space allocation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UNTEXTURELAYOUT_H__
#define __UNTEXTURELAYOUT_H__

/**
 * An incremental texture space allocator.
 * For best results, add the elements ordered descending in size.
 */
class FTextureLayout
{
public:

	/**
	 * Minimal initialization constructor.
	 * @param	MinSizeX - The minimum width of the texture.
	 * @param	MinSizeY - The minimum height of the texture.
	 * @param	MaxSizeX - The maximum width of the texture.
	 * @param	MaxSizeY - The maximum height of the texture.
	 * @param	InPowerOfTwoSize - True if the texture size must be a power of two.
	 */
	FTextureLayout(UINT MinSizeX,UINT MinSizeY,UINT MaxSizeX,UINT MaxSizeY,bool InPowerOfTwoSize = false):
		SizeX(MinSizeX),
		SizeY(MinSizeY),
		PowerOfTwoSize(InPowerOfTwoSize)
	{
		new(Nodes) FTextureLayoutNode(0,0,MaxSizeX,MaxSizeY);
	}

	/**
	 * Finds a free area in the texture large enough to contain a surface with the given size.
	 * If a large enough area is found, it is marked as in use, the output parameters OutBaseX and OutBaseY are
	 * set to the coordinates of the upper left corner of the free area and the function return true.
	 * Otherwise, the function returns false and OutBaseX and OutBaseY remain uninitialized.
	 * @param	OutBaseX - If the function succeedes, contains the X coordinate of the upper left corner of the free area on return.
	 * @param	OutBaseY - If the function succeedes, contains the Y coordinate of the upper left corner of the free area on return.
	 * @param	ElementSizeX - The size of the surface to allocate in horizontal pixels.
	 * @param	ElementSizeY - The size of the surface to allocate in vertical pixels.
	 * @return	True if succeeded, false otherwise.
	 */
	UBOOL AddElement(UINT* OutBaseX,UINT* OutBaseY,UINT ElementSizeX,UINT ElementSizeY)
	{
		// Pad to 4 to ensure alignment
		ElementSizeX = (ElementSizeX + 3) & ~3;
		ElementSizeY = (ElementSizeY + 3) & ~3;

		// Try allocating space without enlarging the texture.
		INT	NodeIndex = AddSurfaceInner(0,ElementSizeX,ElementSizeY,false);
		if(NodeIndex == INDEX_NONE)
		{
			// Try allocating space which might enlarge the texture.
			NodeIndex = AddSurfaceInner(0,ElementSizeX,ElementSizeY,true);
		}

		if(NodeIndex != INDEX_NONE)
		{
			FTextureLayoutNode&	Node = Nodes(NodeIndex);
			Node.Used = TRUE;
			*OutBaseX = Node.MinX;
			*OutBaseY = Node.MinY;
			if(PowerOfTwoSize)
			{
				SizeX = Max<UINT>(SizeX,appRoundUpToPowerOfTwo(Node.MinX + ElementSizeX));
				SizeY = Max<UINT>(SizeY,appRoundUpToPowerOfTwo(Node.MinY + ElementSizeY));
			}
			else
			{
				SizeX = Max<UINT>(SizeX,Node.MinX + ElementSizeX);
				SizeY = Max<UINT>(SizeY,Node.MinY + ElementSizeY);
			}
			return 1;
		}
		else
		{
			return 0;
		}
	}

	/**
	 * Returns the minimum texture width which will contain the allocated surfaces.
	 */
	UINT GetSizeX() const { return SizeX; }

	/**
	 * Returns the minimum texture height which will contain the allocated surfaces.
	 */
	UINT GetSizeY() const { return SizeY; }

private:

	struct FTextureLayoutNode
	{
		INT		ChildA,
				ChildB;
		WORD	MinX,
				MinY,
				SizeX,
				SizeY;
		UBOOL	Used;

		FTextureLayoutNode() {}

		FTextureLayoutNode(WORD InMinX,WORD InMinY,WORD InSizeX,WORD InSizeY):
			ChildA(INDEX_NONE),
			ChildB(INDEX_NONE),
			MinX(InMinX),
			MinY(InMinY),
			SizeX(InSizeX),
			SizeY(InSizeY),
			Used(FALSE)
		{}
	};

	UINT SizeX;
	UINT SizeY;
	UBOOL PowerOfTwoSize;
	TArray<FTextureLayoutNode,TInlineAllocator<5> > Nodes;

	INT AddSurfaceInner(INT NodeIndex,UINT ElementSizeX,UINT ElementSizeY,bool AllowTextureEnlargement)
	{
		if(Nodes(NodeIndex).ChildA != INDEX_NONE)
		{
			check(Nodes(NodeIndex).ChildB != INDEX_NONE);
			INT	Result = AddSurfaceInner(Nodes(NodeIndex).ChildA,ElementSizeX,ElementSizeY,AllowTextureEnlargement);
			if(Result != INDEX_NONE)
				return Result;
			return AddSurfaceInner(Nodes(NodeIndex).ChildB,ElementSizeX,ElementSizeY,AllowTextureEnlargement);
		}
		else
		{
			if(Nodes(NodeIndex).Used)
				return INDEX_NONE;

			if(Nodes(NodeIndex).SizeX < ElementSizeX || Nodes(NodeIndex).SizeY < ElementSizeY)
				return INDEX_NONE;

			if(!AllowTextureEnlargement)
			{
				if(Nodes(NodeIndex).MinX + ElementSizeX > SizeX || Nodes(NodeIndex).MinY + ElementSizeY > SizeY)
				{
					// If this is an attempt to allocate space without enlarging the texture, and this node cannot hold the element
					// without enlarging the texture, fail.
					return INDEX_NONE;
				}
			}

			if(Nodes(NodeIndex).SizeX == ElementSizeX && Nodes(NodeIndex).SizeY == ElementSizeY)
				return NodeIndex;

			UINT	ExcessWidth = Nodes(NodeIndex).SizeX - ElementSizeX,
					ExcessHeight = Nodes(NodeIndex).SizeY - ElementSizeY;

			// Add new nodes, and link them as children of the current node.
			// This needs to access Node as Nodes(NodeIndex) since adding the new nodes may reallocate the Nodes array,
			// invalidate the Node pointer.
			if(ExcessWidth > ExcessHeight)
			{
				Nodes(NodeIndex).ChildA = Nodes.Num();
                new(Nodes) FTextureLayoutNode(
					Nodes(NodeIndex).MinX,
					Nodes(NodeIndex).MinY,
					ElementSizeX,
					Nodes(NodeIndex).SizeY
					);

				Nodes(NodeIndex).ChildB = Nodes.Num();
				new(Nodes) FTextureLayoutNode(
					Nodes(NodeIndex).MinX + ElementSizeX,
					Nodes(NodeIndex).MinY,
					Nodes(NodeIndex).SizeX - ElementSizeX,
					Nodes(NodeIndex).SizeY
					);
			}
			else
			{
				Nodes(NodeIndex).ChildA = Nodes.Num();
                new(Nodes) FTextureLayoutNode(
					Nodes(NodeIndex).MinX,
					Nodes(NodeIndex).MinY,
					Nodes(NodeIndex).SizeX,
					ElementSizeY
					);

				Nodes(NodeIndex).ChildB = Nodes.Num();
				new(Nodes) FTextureLayoutNode(
					Nodes(NodeIndex).MinX,
					Nodes(NodeIndex).MinY + ElementSizeY,
					Nodes(NodeIndex).SizeX,
					Nodes(NodeIndex).SizeY - ElementSizeY
					);
			}

			return AddSurfaceInner(Nodes(NodeIndex).ChildA,ElementSizeX,ElementSizeY,AllowTextureEnlargement);
		}
	}
};

#endif

// Copyright NVIDIA Corporation 2007 -- Ignacio Castano <icastano@nvidia.com>
// 
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

#ifndef NV_TT_BLOCKDXT_H
#define NV_TT_BLOCKDXT_H

#include <nvmath/Color.h>
#include "nvtt.h"

namespace nv
{
	struct ColorBlock;

	/// DXT1 block.
	struct BlockDXT1
	{
		Color16 col0;
		Color16 col1;
		union {
			uint8 row[4];
			uint indices;
		};
	
		bool isFourColorMode() const;
	
		uint evaluatePalette(Color32 color_array[4]) const;
		uint evaluatePaletteFast(Color32 color_array[4]) const;
		void evaluatePalette3(Color32 color_array[4]) const;
		void evaluatePalette4(Color32 color_array[4]) const;
		
		void decodeBlock(ColorBlock * block) const;
		
		void setIndices(int * idx);

		void flip4();
		void flip2();
	};
	
	/// Return true if the block uses four color mode, false otherwise.
	inline bool BlockDXT1::isFourColorMode() const
	{
		return col0.u >= col1.u;	// @@ > or >= ?
	}
	
	
	
	
	/// DXT3 alpha block with explicit alpha.
	struct AlphaBlockDXT3
	{
		union {
			struct {
				uint alpha0 : 4;
				uint alpha1 : 4;
				uint alpha2 : 4;
				uint alpha3 : 4;
				uint alpha4 : 4;
				uint alpha5 : 4;
				uint alpha6 : 4;
				uint alpha7 : 4;
				uint alpha8 : 4;
				uint alpha9 : 4;
				uint alphaA : 4;
				uint alphaB : 4;
				uint alphaC : 4;
				uint alphaD : 4;
				uint alphaE : 4;
				uint alphaF : 4;
			};
			uint16 row[4];
		};
		
		void flip4();
		void flip2();
	};
	
	
	/// DXT3 block.
	struct BlockDXT3
	{
		AlphaBlockDXT3 alpha;
		BlockDXT1 color;
		
		void decodeBlock(ColorBlock * block) const;
		
		void flip4();
		void flip2();
	};
	
	
	/// DXT5 alpha block.
	struct AlphaBlockDXT5
	{
		union {
			struct {
				uint64 alpha0 : 8;	// 8
				uint64 alpha1 : 8;	// 16
				uint64 bits0 : 3;	// 3 - 19
				uint64 bits1 : 3; 	// 6 - 22
				uint64 bits2 : 3; 	// 9 - 25
				uint64 bits3 : 3;	// 12 - 28
				uint64 bits4 : 3;	// 15 - 31
				uint64 bits5 : 3;	// 18 - 34
				uint64 bits6 : 3;	// 21 - 37
				uint64 bits7 : 3;	// 24 - 40
				uint64 bits8 : 3;	// 27 - 43
				uint64 bits9 : 3; 	// 30 - 46
				uint64 bitsA : 3; 	// 33 - 49
				uint64 bitsB : 3;	// 36 - 52
				uint64 bitsC : 3;	// 39 - 55
				uint64 bitsD : 3;	// 42 - 58
				uint64 bitsE : 3;	// 45 - 61
				uint64 bitsF : 3;	// 48 - 64
			};
			uint64 u;
		};
		
		void evaluatePalette(uint8 alpha[8]) const;
		void evaluatePalette8(uint8 alpha[8]) const;
		void evaluatePalette6(uint8 alpha[8]) const;
		void indices(uint8 index_array[16]) const;

		uint index(uint index) const;
		void setIndex(uint index, uint value);
		
		void flip4();
		void flip2();
	};
	
	/// DXT5 block.
	struct BlockDXT5
	{
		AlphaBlockDXT5 alpha;
		BlockDXT1 color;
		
		void decodeBlock(ColorBlock * block) const;
		
		void flip4();
		void flip2();
	};
	
	/// 3DC block.
	struct Block3DC
	{
		AlphaBlockDXT5 y;
		AlphaBlockDXT5 x;
		
		void decodeBlock(ColorBlock * block) const;
		
		void flip4();
		void flip2();
	};

} // nv namespace

#endif // NV_TT_BLOCKDXT_H

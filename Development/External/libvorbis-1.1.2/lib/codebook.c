/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2002             *
 * by the XIPHOPHORUS Company http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: basic codebook pack/unpack/code/decode operations
 last mod: $Id: codebook.c 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ogg/ogg.h>
#include "vorbis/codec.h"
#include "codebook.h"
#include "scales.h"
#include "misc.h"
#include "os.h"
#ifdef __SSE__												/* SSE Optimize */
#include "xmmlib.h"
#endif														/* SSE Optimize */

/* packs the given codebook into the bitstream */
int vorbis_staticbook_pack( const static_codebook* c, oggpack_buffer* opb )
{
	long i, j;
	int ordered = 0;

	/* first the basic parameters */
	oggpack_write( opb, 0x564342, 24 );
	oggpack_write( opb, c->dim, 16 );
	oggpack_write( opb, c->entries, 24 );

	/* pack the codewords.  There are two packings; length ordered and length random.  Decide between the two now. */
	for( i = 1; i < c->entries; i++ )
	{
		if( c->lengthlist[i-1] == 0 || c->lengthlist[i] < c->lengthlist[i - 1]) 
		{
			break; 
		}

		if( i == c->entries )
		{
			ordered = 1;
		}
	}

	if( ordered )
	{
		/* length ordered.  We only need to say how many codewords of each length.  The actual codewords are generated deterministically */
		long count = 0;
	
		oggpack_write( opb, 1, 1 );  /* ordered */
		oggpack_write( opb, c->lengthlist[0] - 1, 5 ); /* 1 to 32 */

		for( i = 1; i < c->entries; i++ )
		{
			long this = c->lengthlist[i];
			long last = c->lengthlist[i - 1];
			if( this > last )
			{
				for( j =last; j < this; j++ )
				{
					oggpack_write( opb, i - count, ilog( c->entries - count ) );
					count = i;
				}
			}
		}

		oggpack_write( opb, i - count, ilog( c->entries - count ) );
	}
	else
	{
		/* length random.  Again, we don't code the codeword itself, just the length.  This time, though, we have to encode each length */
		oggpack_write( opb, 0, 1 );   /* unordered */

		/* algortihmic mapping has use for 'unused entries', which we tag here.  The algorithmic mapping happens as usual, but the unused entry has no codeword. */
		for( i = 0; i < c->entries; i++ )
		{
			if( c->lengthlist[i] == 0 )
			{
				break;
			}
		}

		if( i == c->entries )
		{
			oggpack_write( opb, 0, 1 ); /* no unused entries */
			for( i = 0; i < c->entries; i++ )
			{
				oggpack_write( opb, c->lengthlist[i] - 1, 5 );
			}
		}
		else
		{
			oggpack_write( opb, 1, 1 ); /* we have unused entries; thus we tag */
			for( i = 0; i < c->entries; i++ )
			{
				if( c->lengthlist[i] == 0 )
				{
					oggpack_write( opb, 0, 1 );
				}
				else
				{
					oggpack_write( opb, 1, 1 );
					oggpack_write( opb, c->lengthlist[i] - 1, 5 );
				}
			}
		}
	}

	/* is the entry number the desired return value, or do we have a mapping? If we have a mapping, what type? */
	oggpack_write( opb, c->maptype, 4 );
	switch( c->maptype )
	{
		case 0:
			/* no mapping */
			break;

		case 1:
		case 2:
			/* implicitly populated value mapping */
			/* explicitly populated value mapping */
			if( !c->quantlist )
			{
				/* no quantlist?  error */
				return( -1 );
			}

			/* values that define the dequantization */
			oggpack_write( opb, c->q_min, 32 );
			oggpack_write( opb, c->q_delta, 32 );
			oggpack_write( opb, c->q_quant - 1, 4 );
			oggpack_write( opb, c->q_sequencep, 1 );

			{
				int quantvals;
				switch( c->maptype )
				{
					case 1:
						/* a single column of (c->entries/c->dim) quantized values for building a full value list algorithmically (square lattice) */
						quantvals = _book_maptype1_quantvals( c );
						break;

					case 2:
						/* every value (c->entries*c->dim total) specified explicitly */
						quantvals = c->entries * c->dim;
						break;
					default: 
						/* NOT_REACHABLE */
						quantvals = -1;
				}

				/* quantized values */
				for( i = 0; i < quantvals; i++ )
				{
					oggpack_write( opb, labs( c->quantlist[i] ), c->q_quant );
				}
			}
		break;

		default:
			/* error case; we don't have any other map types now */
			return( -1 );
	}

	return( 0 );
}

/* unpacks a codebook from the packet buffer into the codebook struct, readies the codebook auxiliary structures for decode */
int vorbis_staticbook_unpack( oggpack_buffer* opb, static_codebook* s )
{
	long i,j;

	memset( s, 0, sizeof( *s ) );
	s->allocedp = 1;

	/* make sure alignment is correct */
	if( oggpack_read( opb, 24 ) != 0x564342 )
	{ 
		goto _eofout; 
	}

	/* first the basic parameters */
	s->dim = oggpack_read( opb, 16 );
	s->entries = oggpack_read( opb, 24 );
	if( s->entries == -1 )
	{ 
		goto _eofout; 
	}

	/* codeword ordering.... length ordered or unordered? */
	switch( ( int )oggpack_read( opb, 1 ) )
	{
		case 0:
			/* unordered */
			s->lengthlist = ( long* )_ogg_malloc( sizeof( *s->lengthlist ) * s->entries );

			/* allocated but unused entries? */
			if( oggpack_read( opb, 1 ) )
			{
				/* yes, unused entries */
				for( i = 0; i < s->entries; i++ )
				{
					if( oggpack_read( opb, 1 ) )
					{
						long num = oggpack_read( opb, 5 );
						if( num == -1 )
						{ 
							goto _eofout; 
						}
						s->lengthlist[i] = num + 1;
					}
					else
					{
						s->lengthlist[i] = 0;
					}
				}
			}
			else
			{
				/* all entries used; no tagging */
				for( i = 0; i < s->entries; i++ )
				{
					long num = oggpack_read( opb, 5 );
					if( num == -1 )
					{ 
						goto _eofout; 
					}
					s->lengthlist[i] = num + 1;
				}
			}
			break;

		case 1:
			/* ordered */
			{
				long length = oggpack_read( opb, 5 ) + 1;
				s->lengthlist = ( long* )_ogg_malloc( sizeof( *s->lengthlist ) * s->entries );

				for( i = 0; i < s->entries; )
				{
					long num = oggpack_read( opb, ilog( s->entries - i ) );
					if( num == -1 )
					{
						goto _eofout;
					}

					for( j = 0; j < num && i < s->entries; j++, i++ )
					{
						s->lengthlist[i] = length;
					}

					length++;
				}
			}
			break;

		default:
			/* EOF */
			return( -1 );
		}
  
	/* Do we have a mapping to unpack? */
	switch( ( s->maptype = oggpack_read( opb, 4 ) ) )
	{
		case 0:
			/* no mapping */
			break;

		case 1: 
		case 2:
			/* implicitly populated value mapping */
			/* explicitly populated value mapping */
			s->q_min = oggpack_read( opb, 32 );
			s->q_delta = oggpack_read( opb, 32 );
			s->q_quant = oggpack_read( opb, 4 ) + 1;
			s->q_sequencep = oggpack_read( opb, 1 );

			{
				int quantvals = 0;
				switch( s->maptype )
				{
					case 1:
						quantvals = _book_maptype1_quantvals( s );
						break;

					case 2:
						quantvals = s->entries * s->dim;
						break;
				}

				/* quantized values */
				s->quantlist = ( long* )_ogg_malloc( sizeof( *s->quantlist ) * quantvals );
				for( i = 0; i < quantvals; i++ )
				{
					s->quantlist[i] = oggpack_read( opb, s->q_quant );
				}

				if( quantvals && s->quantlist[quantvals-1] == -1 )
				{
					goto _eofout;
				}
			}
			break;

		default:
			goto _errout;
	}

	/* all set */
	return( 0 );

_errout:
_eofout:
	vorbis_staticbook_clear( s );
	return( -1 ); 
}

/* returns the number of bits */
int vorbis_book_encode( codebook* book, int a, oggpack_buffer* b )
{
	oggpack_write( b, book->codelist[a], book->c->lengthlist[a] );
	return( book->c->lengthlist[a] );
}

/* One the encode side, our vector writers are each designed for a specific purpose, and the encoder is not flexible without modification:

The LSP vector coder uses a single stage nearest-match with no interleave, so no step and no error return.  This is specced by floor0
and doesn't change.

Residue0 encoding interleaves, uses multiple stages, and each stage peels of a specific amount of resolution from a lattice (thus we want
to match by threshold, not nearest match).  Residue doesn't *have* to be encoded that way, but to change it, one will need to add more
infrastructure on the encode side (decode side is specced and simpler) */

/* floor0 LSP (single stage, non interleaved, nearest match) */
/* returns entry number and *modifies a* to the quantization value *****/
int vorbis_book_errorv( codebook* book, float* a )
{
	int dim = book->dim, k;
	int best = _best( book, a, 1 );
	for( k = 0; k < dim; k++ )
	{
		a[k] = ( book->valuelist + best * dim )[k];
	}

	return( best );
}

/* returns the number of bits and *modifies a* to the quantization value */
int vorbis_book_encodev( codebook* book, int best, float* a, oggpack_buffer* b )
{
	int k, dim = book->dim;

	for( k = 0; k < dim; k++ )
	{
		a[k] = ( book->valuelist + best * dim )[k];
	}

	return( vorbis_book_encode( book, best, b ) );
}

/* the 'eliminate the decode tree' optimization actually requires the codewords to be MSb first, not LSb.  This is an annoying inelegancy
   (and one of the first places where carefully thought out design turned out to be wrong; Vorbis II and future Ogg codecs should go
   to an MSb bitpacker), but not actually the huge hit it appears to be.  The first-stage decode table catches most words so that
   bitreverse is not in the main execution path. */

ogg_uint32_t bitreverse( ogg_uint32_t x )
{
	x = ( ( x >> 16 ) & 0x0000ffff ) | ( ( x << 16 ) & 0xffff0000 );
	x = ( ( x >> 8 ) & 0x00ff00ff ) | ( ( x << 8 ) & 0xff00ff00 );
	x = ( ( x >> 4 ) & 0x0f0f0f0f ) | ( ( x << 4 ) & 0xf0f0f0f0 );
	x = ( ( x >> 2 ) & 0x33333333 ) | ( ( x << 2 ) & 0xcccccccc );
	return( ( x >> 1 ) & 0x55555555 ) | ( ( x << 1 ) & 0xaaaaaaaa );
}

STIN long decode_packed_entry_number( codebook* book, oggpack_buffer* b )
{
	int  read = book->dec_maxlength;
	long lo, hi;
	long lok = oggpack_look( b, book->dec_firsttablen );

	if( lok >= 0 ) 
	{
		long entry = book->dec_firsttable[lok];
		if( entry & 0x80000000 )
		{
			lo = ( entry >> 15 ) & 0x7fff;
			hi = book->used_entries - ( entry & 0x7fff );
		}
		else
		{
			oggpack_adv( b, book->dec_codelengths[entry - 1] );
			return( entry - 1 );
		}
	}
	else
	{
		lo = 0;
		hi = book->used_entries;
	}

	lok = oggpack_look( b, read );

	while( lok < 0 && read > 1)
	{
		lok = oggpack_look( b, --read );
	}

	if( lok < 0 )
	{
		return( -1 );
	}

	/* bisect search for the codeword in the ordered list */
	{
		ogg_uint32_t testword = bitreverse( ( ogg_uint32_t )lok );

		while( hi - lo > 1 )
		{
			long p = ( hi - lo ) >> 1;
			long test = book->codelist[lo + p] > testword;    
			lo += p & ( test - 1 );
			hi -= p & ( -test );
		}

		if( book->dec_codelengths[lo] <= read )
		{
			oggpack_adv( b, book->dec_codelengths[lo] );
			return( lo );
		}
	}

	oggpack_adv( b, read );
	return( -1 );
}

/* Decode side is specced and easier, because we don't need to find matches using different criteria; we simply read and map.  There are
   two things we need to do 'depending':
   
   We may need to support interleave.  We don't really, but it's convenient to do it here rather than rebuild the vector later.

   Cascades may be additive or multiplicitive; this is not inherent in the codebook, but set in the code using the codebook.  Like
   interleaving, it's easiest to do it here.  
   addmul==0 -> declarative (set the value)
   addmul==1 -> additive
   addmul==2 -> multiplicitive */

/* returns the [original, not compacted] entry number or -1 on eof */
long vorbis_book_decode( codebook* book, oggpack_buffer* b )
{
	long packed_entry = decode_packed_entry_number( book, b );
	if(	packed_entry >= 0 )
	{
		return( book->dec_index[packed_entry] );
	}

	/* if there's no dec_index, the codebook unpacking isn't collapsed */
	return( packed_entry );
}

/* returns 0 on OK or -1 on eof */
long vorbis_book_decodevs_add( codebook* book, float* a, oggpack_buffer* b, int n )
{
	int step = n / book->dim;
	long* entry = alloca( sizeof( *entry ) * step );
	float** t = alloca( sizeof( *t ) * step );
	int i, j, o;

	for( i = 0; i < step; i++ ) 
	{
		entry[i] = decode_packed_entry_number( book, b );
		if( entry[i] == -1 )
		{
			return( -1 );
		}
		t[i] = book->valuelist + entry[i] * book->dim;
	}

	for( i = 0, o = 0; i < book->dim; i++, o += step )
	{
		for( j = 0; j < step; j++ )
		{
			a[o + j] += t[j][i];
		}
	}

	return( 0 );
}

long vorbis_book_decodev_add( codebook* book, float* a, oggpack_buffer* b, int n )
{
	int i, j, entry;
	float* t;

	if( book->dim > 8 )
	{
		for( i = 0; i < n; )
		{
			entry = decode_packed_entry_number( book, b );
			if( entry == -1 )
			{
				return( -1 );
			}

			t = book->valuelist + entry * book->dim;
			for( j = 0; j < book->dim; )
			{
				a[i++] += t[j++];
			}
		}
	}
	else
	{
		for( i = 0; i < n; )
		{
			entry = decode_packed_entry_number( book, b );
			if( entry == -1 )
			{
				return( -1 );
			}

			t = book->valuelist + entry * book->dim;
			j = 0;
			switch( ( int )book->dim )
			{
				case 8:
					a[i++] += t[j++];
				case 7:
					a[i++] += t[j++];
				case 6:
					a[i++] += t[j++];
				case 5:
					a[i++] += t[j++];
				case 4:
					a[i++] += t[j++];
				case 3:
					a[i++] += t[j++];
				case 2:
					a[i++] += t[j++];
				case 1:
					a[i++] += t[j++];
				case 0:
				break;
			}
		}
	}    

	return( 0 );
}

long vorbis_book_decodev_set( codebook* book, float* a, oggpack_buffer* b, int n )
{
	int i, j, entry;
	float* t;

	for( i = 0; i < n; )
	{
		entry = decode_packed_entry_number( book, b );
		if( entry == -1 )
		{
			return( -1 );
		}

		t = book->valuelist + entry * book->dim;
		for( j = 0; j < book->dim; )
		{
			a[i++] = t[j++];
		}
	}

	return( 0 );
}

long vorbis_book_decodevv_add( codebook* book, float** a, long offset, int ch, oggpack_buffer* b, int n )
{
#ifdef __SSE__												/* SSE Optimize */
	long i, j;
	int chptr = 0;

	if( ch == 2 )
	{
		int mid0 = ( offset / 2 + 3 ) & ( ~3 );
		int mid1 = ( ( offset + n ) / 2 ) & ( ~3 );
		float *bvl = book->valuelist;
		float *a0 = a[0];
		float *a1 = a[1];
		switch( book->dim )
		{
			default :
				for( i = offset / 2; i < ( offset + n ) / 2; )
				{
					long entry = decode_packed_entry_number( book, b );
					if( entry == -1 )
					{
						return( -1 );
					}
					{
						const float* t = bvl + entry * book->dim;
						for( j = 0; j < book->dim; j++ )
						{
							a[chptr++][i] += t[j];
							if( chptr == 2 )
							{
								chptr = 0;
								i++;
							}
						}
					}
				}
				break;

			case 2:
				for( i = offset / 2; i < mid0; )
				{
					long entry = decode_packed_entry_number( book, b );
					if( entry == -1 )
					{
						return( -1 );
					}
					{
						const float* t = bvl + entry * 2;
						__m128 XMM0 = _mm_load_ss( t );
						__m128 XMM1 = _mm_load_ss( a0 + i );
						__m128 XMM2 = _mm_load_ss( t );
						__m128 XMM3 = _mm_load_ss( a1 + i );
						XMM0 = _mm_add_ss( XMM0, XMM1 );
						XMM2 = _mm_add_ss( XMM2, XMM3 );
						_mm_store_ss( a0 + i, XMM0 );
						_mm_store_ss( a1 + i++, XMM2 );
					}
				}

				for( ; i < mid1; )
				{
					/*
						XMM0	(T11 T10 T01 T00)
						XMM2	(T31 T30 T21 T20)
					*/
					__m128	XMM0, XMM1, XMM2, XMM3, XMM4;

					const float* t0;
					const float* t1;
					const float* t2;
					const float* t3;

					long entry = decode_packed_entry_number( book, b );
					if( entry == -1 )
					{
						return( -1 );
					}

					t0 = bvl + entry * 2;
					entry = decode_packed_entry_number( book, b );
					if( entry == -1 )
					{
						return( -1 );
					}

					t1 = bvl + entry * 2;
					entry = decode_packed_entry_number( book, b );
					if( entry == -1 )
					{
						return( -1 );
					}

					t2 = bvl + entry * 2;
					entry = decode_packed_entry_number( book, b );
					if( entry == -1 )
					{
						return( -1 );
					}

					t3 = bvl + entry * 2;
					XMM0 = _mm_loadl_pi( PFV_0, ( __m64* )t0 );
					XMM2 = _mm_loadl_pi( PFV_0, ( __m64* )t2 );
					XMM3 = _mm_load_ps( a0 + i );
					XMM0 = _mm_loadh_pi( XMM0, ( __m64* )t1 );
					XMM2 = _mm_loadh_pi( XMM2, ( __m64* )t3 );

					/*
						XMM0	(T30 T20 T10 T00)
						XMM2	(T31 T21 T11 T01)
					*/
					XMM4 = _mm_load_ps( a1 + i );
					XMM1 = XMM0;
					XMM0 = _mm_shuffle_ps( XMM0, XMM2, _MM_SHUFFLE( 2, 0, 2, 0 ) );
					XMM1 = _mm_shuffle_ps( XMM1, XMM2, _MM_SHUFFLE( 3, 1, 3, 1 ) );
					XMM0 = _mm_add_ps( XMM0, XMM3 );
					XMM1 = _mm_add_ps( XMM1, XMM4 );
					_mm_store_ps( a0 + i, XMM0 );
					_mm_store_ps( a1 + i, XMM1 );
					i += 4;
				}

				for( ; i < ( offset + n ) / 2; )
				{
					long entry = decode_packed_entry_number( book, b );
					if( entry == -1 )
					{
						return( -1 );
					}
					{
						const float* t = bvl + entry * 2;
						__m128 XMM0 = _mm_load_ss( t );
						__m128 XMM1 = _mm_load_ss( a0 + i );
						__m128 XMM2 = _mm_load_ss( t );
						__m128 XMM3 = _mm_load_ss( a1 + i );
						XMM0 = _mm_add_ss( XMM0, XMM1 );
						XMM2 = _mm_add_ss( XMM2, XMM3 );
						_mm_store_ss( a0 + i, XMM0 );
						_mm_store_ss( a1 + i++, XMM2 );
					}
				}
				break;

			case 4:
				for( i = offset / 2; i < mid0; )
				{
					long entry = decode_packed_entry_number( book, b );
					if( entry == -1 )
					{
						return( -1 );
					}
					{
						const float* t = bvl + entry * 4;
						__m128 XMM0 = _mm_load_ss( t );
						__m128 XMM1 = _mm_load_ss( a0 + i );
						__m128 XMM2 = _mm_load_ss( t + 1 );
						__m128 XMM3 = _mm_load_ss( a1 + i );
						__m128 XMM4 = _mm_load_ss( t + 2 );
						__m128 XMM5 = _mm_load_ss( a0 + i + 1 );
						__m128 XMM6 = _mm_load_ss( t + 3 );
						__m128 XMM7 = _mm_load_ss( a1 + i + 1 );
						XMM0 = _mm_add_ss( XMM0, XMM1 );
						XMM2 = _mm_add_ss( XMM2, XMM3 );
						XMM4 = _mm_add_ss( XMM4, XMM5 );
						XMM6 = _mm_add_ss( XMM6, XMM7 );
						_mm_store_ss( a0 + i, XMM0 );
						_mm_store_ss( a1 + i, XMM2 );
						_mm_store_ss( a0 + i + 1, XMM4 );
						_mm_store_ss( a1 + i + 1, XMM6 );
						i += 2;
					}
				}

				for( ; i < mid1; )
				{
					/*
						XMM0	(T03 T02 T01 T00)
						XMM1	(T13 T12 T11 T10)
						XMM2	(T23 T22 T21 T20)
						XMM3	(T33 T32 T31 T30)
					*/
					__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;

					long entry = decode_packed_entry_number( book, b );
					if( entry == -1 )
					{
						return( -1 );
					}

					XMM0 = _mm_lddqu_ps( bvl + entry * 4 );
					entry = decode_packed_entry_number( book, b );
					if( entry == -1 )
					{
						return( -1 );
					}

					XMM1 = _mm_lddqu_ps( bvl + entry * 4 );
					entry = decode_packed_entry_number( book, b );
					if( entry == -1 )
					{
						return( -1 );
					}

					XMM2 = _mm_lddqu_ps( bvl + entry * 4 );
					entry = decode_packed_entry_number( book, b );
					if( entry == -1 )
					{
						return( -1 );
					}

					XMM3 = _mm_lddqu_ps( bvl + entry * 4 );
					/*
						XMM0	(T12 T10 T02 T00)
						XMM4	(T13 T11 T03 T01)
						XMM2	(T32 T20 T12 T10)
						XMM5	(T33 T21 T13 T11)
					*/
					XMM4 = XMM0;
					XMM5 = XMM2;
					XMM0 = _mm_shuffle_ps( XMM0, XMM1, _MM_SHUFFLE( 2, 0, 2, 0 ) );
					XMM4 = _mm_shuffle_ps( XMM4, XMM1, _MM_SHUFFLE( 3, 1, 3, 1 ) );
					XMM1 = _mm_load_ps( a0+i  );
					XMM2 = _mm_shuffle_ps( XMM2, XMM3, _MM_SHUFFLE( 2, 0, 2, 0 ) );
					XMM5 = _mm_shuffle_ps( XMM5, XMM3, _MM_SHUFFLE( 3, 1, 3, 1 ) );
					XMM3 = _mm_load_ps( a1 + i );
					XMM6 = _mm_load_ps( a0 + i + 4 );
					XMM7 = _mm_load_ps( a1 + i + 4 );
					XMM0 = _mm_add_ps( XMM0, XMM1 );
					XMM4 = _mm_add_ps( XMM4, XMM3 );
					XMM2 = _mm_add_ps( XMM2, XMM6 );
					XMM5 = _mm_add_ps( XMM5, XMM7 );
					_mm_store_ps( a0 + i, XMM0 );
					_mm_store_ps( a1 + i, XMM4 );
					_mm_store_ps( a0 + i + 4, XMM2 );
					_mm_store_ps( a1 + i + 4, XMM5 );
					i += 8;
				}

				for( ; i < ( offset + n ) / 2; )
				{
					long entry = decode_packed_entry_number( book, b );
					if( entry == -1 )
					{
						return( -1 );
					}

					{
						const float* t = bvl + entry * 4;
						__m128 XMM0 = _mm_load_ss( t );
						__m128 XMM1 = _mm_load_ss( a0 + i );
						__m128 XMM2 = _mm_load_ss( t + 1 );
						__m128 XMM3 = _mm_load_ss( a1 + i );
						__m128 XMM4 = _mm_load_ss( t + 2 );
						__m128 XMM5 = _mm_load_ss( a0 + i + 1 );
						__m128 XMM6 = _mm_load_ss( t + 3 );
						__m128 XMM7 = _mm_load_ss( a1 + i + 1 );
						XMM0 = _mm_add_ss( XMM0, XMM1 );
						XMM2 = _mm_add_ss( XMM2, XMM3 );
						XMM4 = _mm_add_ss( XMM4, XMM5 );
						XMM6 = _mm_add_ss( XMM6, XMM7 );
						_mm_store_ss( a0 + i, XMM0 );
						_mm_store_ss( a1 + i, XMM2 );
						_mm_store_ss( a0 + i + 1, XMM4 );
						_mm_store_ss( a1 + i + 1, XMM6 );
						i += 2;
					}
				}
				break;

			case 8:
				for( i = offset / 2; i < mid0; )
				{
					long entry = decode_packed_entry_number( book, b );
					if( entry == -1 )
					{
						return( -1 );
					}
					{
						const float* t = bvl + entry * 8;
						__m128 XMM0 = _mm_lddqu_ps( t );
						__m128 XMM1 = _mm_lddqu_ps( t + 4 );
						__m128 XMM2 = _mm_load_ps( a0 + i );
						__m128 XMM3 = _mm_load_ps( a1 + i );
						__m128 XMM4 = XMM0;
						XMM0 = _mm_shuffle_ps( XMM0, XMM1, _MM_SHUFFLE( 2, 0, 2, 0 ) );
						XMM4 = _mm_shuffle_ps( XMM4, XMM1, _MM_SHUFFLE( 3, 1, 3, 1 ) );
						XMM0 = _mm_add_ps( XMM0, XMM2 );
						XMM4 = _mm_add_ps( XMM4, XMM3 );
						_mm_store_ps( a0 + i, XMM0 );
						_mm_store_ps( a1 + i, XMM4 );
						i += 4;
					}
				}

				for( ; i < mid1; )
				{
					/*
						XMM0	(T03 T02 T01 T00)
						XMM1	(T13 T12 T11 T10)
						XMM2	(T07 T06 T05 T04)
						XMM2	(T17 T16 T15 T14)
					*/
					__m128	XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7;
					const float* t;

					long entry = decode_packed_entry_number( book, b );
					if( entry == -1 )
					{
						return( -1 );
					}

					t = bvl + entry * 8;
					XMM0 = _mm_lddqu_ps( t );
					XMM1 = _mm_lddqu_ps( t + 4 );
					entry = decode_packed_entry_number( book, b );
					if( entry == -1 )
					{
						return( -1 );
					}

					t = bvl + entry * 8;
					XMM2 = _mm_lddqu_ps( t );
					XMM3 = _mm_lddqu_ps( t + 4 );
					/*
						XMM0	(T12 T10 T02 T00)
						XMM4	(T13 T11 T03 T01)
						XMM2	(T16 T14 T06 T04)
						XMM5	(T17 T15 T07 T05)
					*/
					XMM4 = XMM0;
					XMM5 = XMM2;
					XMM0 = _mm_shuffle_ps( XMM0, XMM1, _MM_SHUFFLE( 2, 0, 2, 0 ) );
					XMM4 = _mm_shuffle_ps( XMM4, XMM1, _MM_SHUFFLE( 3, 1, 3, 1 ) );
					XMM2 = _mm_shuffle_ps( XMM2, XMM3, _MM_SHUFFLE( 2, 0, 2, 0 ) );
					XMM5 = _mm_shuffle_ps( XMM5, XMM3, _MM_SHUFFLE( 3, 1, 3, 1 ) );
					XMM1 = _mm_load_ps( a0 + i );
					XMM3 = _mm_load_ps( a1 + i );
					XMM6 = _mm_load_ps( a0 + i + 4 );
					XMM7 = _mm_load_ps( a1 + i + 4 );
					XMM0 = _mm_add_ps( XMM0, XMM1 );
					XMM4 = _mm_add_ps( XMM4, XMM3 );
					XMM2 = _mm_add_ps( XMM2, XMM6 );
					XMM5 = _mm_add_ps( XMM5, XMM7 );
					_mm_store_ps( a0 + i, XMM0 );
					_mm_store_ps( a1 + i, XMM4 );
					_mm_store_ps( a0 + i + 4, XMM2 );
					_mm_store_ps( a1 + i + 4, XMM5 );
					i += 8;
				}

				for( ; i < ( offset + n ) / 2; )
				{
					long entry = decode_packed_entry_number( book, b );
					if( entry == -1 )
					{
						return( -1 );
					}
					{
						const float* t = bvl + entry * 8;
						__m128 XMM0 = _mm_lddqu_ps( t );
						__m128 XMM1 = _mm_lddqu_ps( t + 4 );
						__m128 XMM4 = XMM0;
						__m128 XMM2 = _mm_load_ps( a0 + i );
						__m128 XMM3 = _mm_load_ps( a1 + i );
						XMM0 = _mm_shuffle_ps( XMM0, XMM1, _MM_SHUFFLE( 2, 0, 2, 0 ) );
						XMM4 = _mm_shuffle_ps( XMM4, XMM1, _MM_SHUFFLE( 3, 1, 3, 1 ) );
						XMM0 = _mm_add_ps( XMM0, XMM2 );
						XMM4 = _mm_add_ps( XMM4, XMM3 );
						_mm_store_ps( a0 + i, XMM0 );
						_mm_store_ps( a1 + i, XMM4 );
						i += 4;
					}
				}
				break;
		}
	}
	else
	{
		for( i = offset / ch; i < ( offset + n ) / ch; )
		{
			long entry = decode_packed_entry_number( book, b );
			if( entry == -1 )
			{
				return( -1 );
			}
			{
				const float* t = book->valuelist + entry * book->dim;
				for( j = 0; j < book->dim; j++ )
				{
					a[chptr++][i] += t[j];
					if( chptr == ch )
					{
						chptr = 0;
						i++;
					}
				}
			}
		}
	}
#else														/* SSE Optimize */
	long i, j, entry;
	int chptr = 0;

	for( i = offset / ch; i < ( offset + n ) / ch; )
	{
		entry = decode_packed_entry_number( book, b );
		if( entry == -1 )
		{
			return( -1 );
		}
		{
			const float* t = book->valuelist + entry * book->dim;
			for( j = 0; j < book->dim; j++ )
			{
				a[chptr++][i] += t[j];
				if( chptr == ch )
				{
					chptr = 0;
					i++;
				}
			}
		}
	}
#endif														/* SSE Optimize */
	return( 0 );
}


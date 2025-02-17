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

 function: basic shared codebook operations
 last mod: $Id: sharedbook.c 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ogg/ogg.h>
#include "os.h"
#include "misc.h"
#include "vorbis/codec.h"
#include "codebook.h"
#include "scales.h"
#include "xmmlib.h"

/**** pack/unpack helpers ******************************************/

/* 32 bit float (not IEEE; nonnormalized mantissa + biased exponent) : neeeeeee eeemmmmm mmmmmmmm mmmmmmmm 
   Why not IEEE?  It's just not that important here. */
#define VQ_FEXP			10
#define VQ_FMAN			21
#define VQ_FEXP_BIAS	768 /* bias toward values smaller than 1. */

float _float32_unpack( long val )
{
	double mant = val & 0x1fffff;
	int sign = val & 0x80000000;
	long exp = ( val & 0x7fe00000 ) >> VQ_FMAN;
	if( sign )
	{
		mant = -mant;
	}
	return( ( float )ldexp( mant, exp - ( VQ_FMAN - 1 ) - VQ_FEXP_BIAS ) );
}

/* given a list of word lengths, generate a list of codewords.  Works for length ordered or unordered, always assigns the lowest valued
   codewords first.  Extended to handle unused entries (length 0) */
ogg_uint32_t* _make_words( long* l, long n, long sparsecount )
{
	long i, j, count = 0;
	ogg_uint32_t marker[33] = { 0 };
	ogg_uint32_t* r = ( ogg_uint32_t* )_ogg_malloc( ( sparsecount ? sparsecount : n ) * sizeof( *r ) );

	for( i = 0; i < n; i++ )
	{
		long length = l[i];
		if( length > 0 )
		{
			ogg_uint32_t entry = marker[length];

			/* when we claim a node for an entry, we also claim the nodes below it (pruning off the imagined tree that may have dangled
			   from it) as well as blocking the use of any nodes directly above for leaves */
		    /* update ourself */
			if( length < 32 && ( entry >> length ) )
			{
				/* error condition; the lengths must specify an overpopulated tree */
				_ogg_free( r );
				return( NULL );
			}

			r[count++] = entry;

			/* Look to see if the next shorter marker points to the node above. if so, update it and repeat. */
			{
				for( j = length; j > 0; j-- )
				{
					if( marker[j] & 1 )
					{
						/* have to jump branches */
						if( j == 1 )
						{
							marker[1]++;
						}
						else
						{
							marker[j] = marker[j - 1] << 1;
						}

						/* invariant says next upper marker would already have been moved if it was on the same path */
						break; 
					}

					marker[j]++;
				}
			}
      
			/* prune the tree; the implicit invariant says all the longer markers were dangling from our just-taken node.  Dangle them from our *new* node. */
			for( j = length + 1; j < 33; j++ )
			{
				if( ( marker[j] >> 1 ) == entry )
				{
					entry = marker[j];
					marker[j] = marker[j - 1] << 1;
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			if( sparsecount == 0 )
			{
				count++;
			}
		}
	}
    
	/* bitreverse the words because our bitwise packer/unpacker is LSb endian */
	for( i = 0, count = 0; i < n; i++ )
	{
		ogg_uint32_t temp = 0;
		for( j = 0; j < l[i]; j++ )
		{
			temp <<= 1;
			temp |= ( r[count] >> j ) & 1;
		}

		if( sparsecount )
		{
			if( l[i] )
			{
				r[count++] = temp;
			}
		}
		else
		{
			r[count++] = temp;
		}
	}

	return( r );
}

/* there might be a straightforward one-line way to do the below that's portable and totally safe against roundoff, but I haven't
   thought of it.  Therefore, we opt on the side of caution */
long _book_maptype1_quantvals( const static_codebook* b )
{
	long vals = ( long )floorf( powf( ( float )b->entries, 1.0f / b->dim ) );

	/* the above *should* be reliable, but we'll not assume that FP is ever reliable when bitstream sync is at stake; verify via integer
	   means that vals really is the greatest value of dim for which vals^b->dim <= b->entries */
	/* treat the above as an initial guess */
	while( 1 )
	{
		long acc = 1;
		long acc1 = 1;
		int i;

		for( i = 0; i < b->dim; i++ )
		{
			acc *= vals;
			acc1 *= vals + 1;
		}

		if( acc <= b->entries && acc1 > b->entries )
		{
			return( vals );
		}
		else
		{
			if( acc > b->entries )
			{
				vals--;
			}
			else
			{
				vals++;
			}
		}
	}
}

/* unpack the quantized list of values for encode/decode ***********/
/* we need to deal with two map types: in map type 1, the values are generated algorithmically (each column of the vector counts through
   the values in the quant vector). in map type 2, all the values came in in an explicit list.  Both value lists must be unpacked */
float* _book_unquantize( const static_codebook* b, int n, int* sparsemap )
{
	long j, k, count = 0;

	if( b->maptype == 1 || b->maptype == 2 )
	{
		int quantvals;
		float mindel= _float32_unpack( b->q_min );
		float delta = _float32_unpack( b->q_delta );
		float* r = ( float* )_ogg_calloc( n * b->dim, sizeof( *r ) );

		/* maptype 1 and 2 both use a quantized value vector, but different sizes */
		switch( b->maptype )
		{
			case 1:
				/* most of the time, entries%dimensions == 0, but we need to be well defined.  We define that the possible vales at each
				   scalar is values == entries/dim.  If entries%dim != 0, we'll have 'too few' values (values*dim<entries), which means that
				   we'll have 'left over' entries; left over entries use zeroed values (and are wasted).  So don't generate codebooks like that */
				quantvals = _book_maptype1_quantvals( b );
				for( j = 0; j < b->entries; j++ )
				{
					if( ( sparsemap && b->lengthlist[j] ) || !sparsemap )
					{
						float last = 0.0f;
						int indexdiv = 1;
						for( k = 0; k < b->dim; k++ )
						{
							int index = ( j / indexdiv ) % quantvals;
							float val = ( float )b->quantlist[index];
							val = fabsf( val ) * delta + mindel + last;
							if( b->q_sequencep )
							{
								last = val;	  
							}
							if( sparsemap )
							{
								r[sparsemap[count] * b->dim + k] = val;
							}
							else
							{
								r[count * b->dim + k] = val;
							}

							indexdiv *= quantvals;
						}

						count++;
					}
				}
				break;

			case 2:
				for( j = 0; j < b->entries; j++ )
				{
					if( ( sparsemap && b->lengthlist[j] ) || !sparsemap )
					{
						float last = 0.0f;

						for( k = 0; k < b->dim; k++ )
						{
							float val = ( float )b->quantlist[j * b->dim + k];
							val = fabsf( val ) * delta + mindel + last;
							if( b->q_sequencep )
							{
								last = val;	  
							}

							if( sparsemap )
							{
								r[sparsemap[count] * b->dim + k] = val;
							}
							else
							{
								r[count * b->dim + k] = val;
							}
						}

						count++;
					}
				}
				break;
		}

		return( r );
	}

	return( NULL );
}

void vorbis_staticbook_clear( static_codebook* b )
{
	if( b->allocedp )
	{
		if( b->quantlist )
		{
			_ogg_free( b->quantlist );
		}

		if( b->lengthlist )
		{
			_ogg_free( b->lengthlist );
		}

		if( b->nearest_tree )
		{
			_ogg_free( b->nearest_tree->ptr0 );
			_ogg_free( b->nearest_tree->ptr1 );
			_ogg_free( b->nearest_tree->p );
			_ogg_free( b->nearest_tree->q );
			_ogg_free( b->nearest_tree );
		}

		if( b->thresh_tree )
		{
			_ogg_free( b->thresh_tree->quantthresh );
			_ogg_free( b->thresh_tree->quantmap );
			_ogg_free( b->thresh_tree );
		}

		memset( b, 0, sizeof( *b ) );
	}
}

void vorbis_staticbook_destroy( static_codebook* b )
{
	if( b->allocedp )
	{
		vorbis_staticbook_clear( b );
		_ogg_free( b );
	}
}

void vorbis_book_clear( codebook* b )
{
	/* static book is not cleared; we're likely called on the lookup and the static codebook belongs to the info struct */
	if( b->valuelist )
	{
		_ogg_free( b->valuelist );
	}

	if( b->codelist )
	{
		_ogg_free( b->codelist );
	}

	if( b->dec_index )
	{
		_ogg_free( b->dec_index );
	}

	if( b->dec_codelengths )
	{
		_ogg_free( b->dec_codelengths );
	}

	if( b->dec_firsttable )
	{
		_ogg_free( b->dec_firsttable );
	}

	memset( b, 0, sizeof( *b ) );
}

int vorbis_book_init_encode( codebook* c, const static_codebook* s )
{
	memset( c, 0, sizeof( *c ) );
	c->c = s;
	c->entries = s->entries;
	c->used_entries = s->entries;
	c->dim = s->dim;
	c->codelist = _make_words( s->lengthlist, s->entries, 0 );
	c->valuelist = _book_unquantize( s, s->entries, NULL );

	return( 0 );
}

static int sort32a( const void* a, const void* b )
{
	return( **( ogg_uint32_t** )a > **( ogg_uint32_t** )b ) - ( **( ogg_uint32_t** )a < **( ogg_uint32_t** )b );
}

/* decode codebook arrangement is more heavily optimized than encode */
int vorbis_book_init_decode( codebook* c, const static_codebook* s )
{
	int i, j, n = 0, tabn;
	int* sortindex;
	memset( c, 0, sizeof( *c ) );

	/* count actually used entries */
	for( i = 0; i < s->entries; i++ )
	{
		if( s->lengthlist[i] > 0 )
		{
			n++;
		}
	}

	c->entries = s->entries;
	c->used_entries = n;
	c->dim = s->dim;

	/* two different remappings go on here.  
	   First, we collapse the likely sparse codebook down only to actually represented values/words.  This collapsing needs to be
	   indexed as map-valueless books are used to encode original entry positions as integers.
	   Second, we reorder all vectors, including the entry index above, by sorted bitreversed codeword to allow treeless decode. */
	{
		/* perform sort */
		ogg_uint32_t* codes = _make_words( s->lengthlist, s->entries, c->used_entries );
		ogg_uint32_t** codep = alloca( sizeof( *codep ) * n );

		if( codes == NULL )
		{
			goto err_out;
		}

		for( i = 0; i < n; i++ )
		{
			codes[i] = bitreverse( codes[i] );
			codep[i] = codes + i;
		}

		qsort( codep, n, sizeof( *codep ), sort32a );

		sortindex = alloca( n * sizeof( *sortindex ) );
		c->codelist = ( ogg_uint32_t* )_ogg_malloc( n * sizeof( *c->codelist ) );

		/* the index is a reverse index */
		for( i = 0; i < n; i++ )
		{
			int position = codep[i] - codes;
			sortindex[position] = i;
		}

		for( i = 0; i < n; i++ )
		{
			c->codelist[sortindex[i]] = codes[i];
		}

		_ogg_free( codes );
	}

	c->valuelist = _book_unquantize( s, n, sortindex );
	c->dec_index = ( int* )_ogg_malloc( n * sizeof( *c->dec_index ) );

	for( n = 0, i = 0; i < s->entries; i++ )
	{
		if( s->lengthlist[i] > 0 )
		{
			c->dec_index[sortindex[n++]] = i;
		}
	}

	c->dec_codelengths = ( char* )_ogg_malloc( n * sizeof( *c->dec_codelengths ) );
	for( n = 0, i = 0; i < s->entries; i++ )
	{
		if( s->lengthlist[i] > 0 )
		{
			c->dec_codelengths[sortindex[n++]] = ( char )s->lengthlist[i];
		}
	}

	/* this is magic */
	c->dec_firsttablen = ilog( c->used_entries ) - 4; 
	if( c->dec_firsttablen < 5 )
	{
		c->dec_firsttablen = 5;
	}

	if( c->dec_firsttablen > 8 )
	{
		c->dec_firsttablen = 8;
	}

	tabn = 1 << c->dec_firsttablen;
	c->dec_firsttable = ( ogg_uint32_t* )_ogg_calloc( tabn, sizeof( *c->dec_firsttable ) );
	c->dec_maxlength = 0;

	for( i = 0; i < n; i++ )
	{
		if( c->dec_maxlength < c->dec_codelengths[i] )
		{
			c->dec_maxlength = c->dec_codelengths[i];	
		}

		if( c->dec_codelengths[i] <= c->dec_firsttablen )
		{
			ogg_uint32_t orig = bitreverse( c->codelist[i] );
			for( j = 0; j < ( 1 << ( c->dec_firsttablen - c->dec_codelengths[i] ) ); j++ )
			{
				c->dec_firsttable[orig | ( j << c->dec_codelengths[i])] = i + 1;
			}
		}
	}

	/* now fill in 'unused' entries in the firsttable with hi/lo search hints for the non-direct-hits */
	{
		ogg_uint32_t mask = 0xfffffffeUL << ( 31 - c->dec_firsttablen );
		long lo = 0, hi = 0;

		for( i = 0; i < tabn; i++ )
		{
			ogg_uint32_t word = i << ( 32 - c->dec_firsttablen );
			if( c->dec_firsttable[bitreverse( word )] == 0 )
			{
				while( ( lo + 1 ) < n && c->codelist[lo + 1] <= word )
				{
					lo++;
				}

				while( hi < n && word >= ( c->codelist[hi] & mask ) )
				{
					hi++;
				}

				/* we only actually have 15 bits per hint to play with here. In order to overflow gracefully (nothing breaks, efficiency
				   just drops), encode as the difference from the extremes. */
				{
					unsigned long loval = lo;
					unsigned long hival = n - hi;

					if( loval > 0x7fff )
					{
						loval = 0x7fff;
					}

					if( hival > 0x7fff )
					{
						hival = 0x7fff;
					}

					c->dec_firsttable[bitreverse( word )] = 0x80000000UL | ( loval << 15 ) | hival;
				}
			}
		}
	}

	return( 0 );

err_out:
	vorbis_book_clear( c );
	return( -1 );
}

static float _dist( int el, float* ref, float* b, int step )
{
	int i;
	float acc = 0.0f;

	for( i = 0; i < el; i++ )
	{
		float val = ( ref[i] - b[i * step] );
		acc += val * val;
	}
	return( acc );
}

int _best( codebook* book, float* a, int step )
{
	encode_aux_threshmatch* tt = book->c->thresh_tree;
	int dim = book->dim;
	int k, o;

	/* do we have a threshhold encode hint? */
	if( tt )
	{
		int index = 0, i;
		/* find the quant val of each scalar */
		for( k = 0, o = step * ( dim - 1 ); k < dim; k++, o -= step )
		{
			i = tt->threshvals >> 1;
			if( a[o] < tt->quantthresh[i] )
			{
				for( ; i > 0; i-- )
				{
					if( a[o] >= tt->quantthresh[i - 1] )
					{
						break;
					}
				}
			}
			else
			{
				for( i++; i < tt->threshvals - 1; i++ )
				{
					if( a[o] < tt->quantthresh[i] )
					{
						break;
					}
				}
			}

			index = ( index * tt->quantvals ) + tt->quantmap[i];
		}

		/* regular lattices are easy :-) */
		/* is this unused?  If so, we'll use a decision tree after all	and fall through*/
		if( book->c->lengthlist[index] > 0 ) 
		{
			return( index );
		}
	}

	/* brute force it! */
	{
		const static_codebook* c = book->c;
		int i, besti = -1;
		float best = 0.0f;
		float* e = book->valuelist;
		for( i = 0; i < book->entries; i++ )
		{
			if( c->lengthlist[i] > 0 )
			{
				float this = _dist( dim, e, a, step );
				if( besti == -1 || this < best )
				{
					best = this;
					besti = i;
				}
			}
			e += dim;
		}

		return( besti );
	}
}

long vorbis_book_codeword( codebook* book, int entry )
{
	/* only use with encode; decode optimizations are allowed to break this */
	if( book->c ) 
	{
		return( book->codelist[entry] );
	}
	return( -1 );
}

long vorbis_book_codelen( codebook* book, int entry )
{
	/* only use with encode; decode optimizations are allowed to break this */
	if( book->c ) 
	{
		return( book->c->lengthlist[entry] );
	}
	return( -1 );
}


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

 function: bitrate tracking and management
 last mod: $Id: bitrate.c 7497 2004-08-08 04:31:40Z xiphmont $

 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ogg/ogg.h>
#include "vorbis/codec.h"
#include "codec_internal.h"
#include "os.h"
#include "misc.h"
#include "bitrate.h"

/* compute bitrate tracking setup  */
void vorbis_bitrate_init( vorbis_info* vi, bitrate_manager_state* bm )
{
	codec_setup_info* ci = vi->codec_setup;
	bitrate_manager_info* bi = &ci->bi;

	memset( bm, 0, sizeof( *bm ) );

	if( bi && ( bi->reservoir_bits > 0 ) )
	{
		long desired_fill;
		long ratesamples = vi->rate;
		int halfsamples = ci->blocksizes[0] >> 1;

		bm->short_per_long = ci->blocksizes[1] / ci->blocksizes[0];
		bm->managed = 1;

		bm->avg_bitsper = rint( 1.0f * bi->avg_rate * halfsamples / ratesamples );
		bm->min_bitsper = rint( 1.0f * bi->min_rate * halfsamples / ratesamples );
		bm->max_bitsper = rint( 1.0f * bi->max_rate * halfsamples / ratesamples );

		bm->avgfloat = PACKETBLOBS / 2.0;    

		/* not a necessary fix, but one that leads to a more balanced typical initialization */
		desired_fill = ( long )( bi->reservoir_bits * bi->reservoir_bias );
		bm->minmax_reservoir = desired_fill;
		bm->avg_reservoir = desired_fill;
	}    
}

void vorbis_bitrate_clear( bitrate_manager_state* bm )
{
	memset( bm, 0, sizeof( *bm ) );
}

int vorbis_bitrate_managed( vorbis_block* vb )
{
	vorbis_dsp_state* vd = vb->vd;
	private_state* b = vd->backend_state; 
	bitrate_manager_state* bm = &b->bms;

	if( bm && bm->managed )
	{
		return( 1 );
	}

	return( 0 );
}

/* finish taking in the block we just processed */
int vorbis_bitrate_addblock( vorbis_block* vb )
{
	vorbis_block_internal* vbi = vb->internal;
	vorbis_dsp_state* vd = vb->vd;
	private_state* b = vd->backend_state; 
	bitrate_manager_state* bm = &b->bms;
	vorbis_info* vi = vd->vi;
	codec_setup_info* ci = vi->codec_setup;
	bitrate_manager_info* bi = &ci->bi;

	int choice = rint( ( float )bm->avgfloat );
	long this_bits = oggpack_bytes( vbi->packetblob[choice] ) << 3;
	long min_target_bits = ( vb->W ? bm->min_bitsper * bm->short_per_long : bm->min_bitsper );
	long max_target_bits = ( vb->W ? bm->max_bitsper * bm->short_per_long : bm->max_bitsper );
	int samples = ci->blocksizes[vb->W] >> 1;
	long desired_fill = ( long )( bi->reservoir_bits * bi->reservoir_bias );

	if( !bm->managed )
	{
		/* not a bitrate managed stream, but for API simplicity, we'll buffer the packet to keep the code path clean */

		/* one has been submitted without being claimed */
		if( bm->vb )
		{
			return( -1 ); 
		}

		bm->vb = vb;
		return( 0 );
	}

	bm->vb = vb;
  
	/* look ahead for avg floater */
	if( bm->avg_bitsper > 0 )
	{
		double slew = 0.0;
		long avg_target_bits = ( vb->W ? bm->avg_bitsper * bm->short_per_long : bm->avg_bitsper );
		double slewlimit = 15.0 / bi->slew_damp;

		/* choosing a new floater:
		if we're over target, we slew down
		if we're under target, we slew up

		choose slew as follows: look through packetblobs of this frame and set slew as the first in the appropriate direction that
		gives us the slew we want.  This may mean no slew if delta is already favorable.

		Then limit slew to slew max */

		if( bm->avg_reservoir + ( this_bits - avg_target_bits ) > desired_fill )
		{
			while( ( choice > 0 ) && ( this_bits > avg_target_bits ) && ( bm->avg_reservoir + ( this_bits - avg_target_bits ) > desired_fill ) )
			{
				choice--;
				this_bits = oggpack_bytes( vbi->packetblob[choice] ) << 3;
			}
		}
		else if( bm->avg_reservoir + ( this_bits - avg_target_bits ) < desired_fill )
		{
			while( choice + 1 < PACKETBLOBS && this_bits < avg_target_bits && bm->avg_reservoir + ( this_bits - avg_target_bits ) < desired_fill )
			{
				choice++;
				this_bits = oggpack_bytes( vbi->packetblob[choice] ) << 3;
			}
		}

		slew = rint( ( float )( choice - bm->avgfloat ) ) / samples * vi->rate;
		if( slew < -slewlimit ) 
		{	
			slew = -slewlimit;
		}
		else if( slew > slewlimit )
		{
			slew = slewlimit;
		}

		bm->avgfloat += slew / vi->rate * samples;
		choice = rint( ( float )bm->avgfloat );
		this_bits = oggpack_bytes( vbi->packetblob[choice] ) << 3;
	}

	/* enforce min(if used) on the current floater (if used) */
	if( bm->min_bitsper > 0 )
	{
		/* do we need to force the bitrate up? */
		if( this_bits < min_target_bits )
		{
			while( bm->minmax_reservoir - ( min_target_bits - this_bits ) < 0 )
			{
				choice++;
				if( choice >= PACKETBLOBS )
				{
					break;
				}

				this_bits = oggpack_bytes( vbi->packetblob[choice] ) << 3;
			}
		}
	}
  
	/* enforce max (if used) on the current floater (if used) */
	if( bm->max_bitsper > 0 )
	{
		/* do we need to force the bitrate down? */
		if( this_bits > max_target_bits )
		{
			while( bm->minmax_reservoir + ( this_bits - max_target_bits ) > bi->reservoir_bits )
			{
				choice--;
				if( choice < 0 )
				{
					break;
				}

				this_bits = oggpack_bytes( vbi->packetblob[choice] ) << 3;
			}
		}
	}

	/* Choice of packetblobs now made based on floater, and min/max requirements. Now boundary check extreme choices */

	if( choice < 0 )
	{
		/* choosing a smaller packetblob is insufficient to trim bitrate. frame will need to be truncated */
		long maxsize = ( max_target_bits + ( bi->reservoir_bits - bm->minmax_reservoir ) ) / 8;
		bm->choice = 0;
		choice = 0;

		if( oggpack_bytes( vbi->packetblob[choice] ) > maxsize )
		{
			oggpack_writetrunc( vbi->packetblob[choice], maxsize << 3 );
			this_bits = oggpack_bytes( vbi->packetblob[choice] ) << 3;
		}
	}
	else
	{
		long minsize = ( min_target_bits - bm->minmax_reservoir + 7 ) / 8;
		if( choice >= PACKETBLOBS )
		{
			choice = PACKETBLOBS - 1;
		}

		bm->choice = choice;

		/* prop up bitrate according to demand. pad this frame out with zeroes */
		minsize -= oggpack_bytes( vbi->packetblob[choice] );
		while( minsize-- > 0 )
		{
			oggpack_write( vbi->packetblob[choice], 0, 8 );
		}

		this_bits = oggpack_bytes( vbi->packetblob[choice] ) << 3;
	}

	/* now we have the final packet and the final packet size.  Update statistics */
	/* min and max reservoir */
	if( bm->min_bitsper > 0 || bm->max_bitsper > 0 )
	{
		if( max_target_bits > 0 && this_bits > max_target_bits )
		{
			bm->minmax_reservoir += ( this_bits - max_target_bits );
		}
		else if( min_target_bits > 0 && this_bits < min_target_bits )
		{
			bm->minmax_reservoir += ( this_bits - min_target_bits );
		}
		else
		{
			/* inbetween; we want to take reservoir toward but not past desired_fill */
			if( bm->minmax_reservoir > desired_fill )
			{
				/* logical bulletproofing against initialization state */
				if( max_target_bits > 0 )
				{ 
					bm->minmax_reservoir += ( this_bits - max_target_bits );
					if( bm->minmax_reservoir < desired_fill )
					{
						bm->minmax_reservoir = desired_fill;
					}
				}
				else
				{
					bm->minmax_reservoir = desired_fill;
				}
			}
			else
			{
				/* logical bulletproofing against initialization state */
				if( min_target_bits > 0 )
				{ 
					bm->minmax_reservoir += ( this_bits - min_target_bits );
					if( bm->minmax_reservoir > desired_fill )
					{
						bm->minmax_reservoir = desired_fill;
					}
				}
				else
				{
					bm->minmax_reservoir = desired_fill;
				}
			}
		}
	}

	/* avg reservoir */
	if( bm->avg_bitsper > 0 )
	{
		long avg_target_bits = ( vb->W ? bm->avg_bitsper * bm->short_per_long : bm->avg_bitsper );    
		bm->avg_reservoir += this_bits - avg_target_bits;
	}

	return( 0 );
}

int vorbis_bitrate_flushpacket( vorbis_dsp_state* vd, ogg_packet* op )
{
	private_state* b = vd->backend_state;
	bitrate_manager_state* bm = &b->bms;
	vorbis_block* vb= bm->vb;
	int choice = PACKETBLOBS / 2;

	if( !vb )
	{
		return( 0 );
	}

	if( op )
	{
		vorbis_block_internal* vbi = vb->internal;

		if( vorbis_bitrate_managed( vb ) )
		{
			choice = bm->choice;
		}

		op->packet = oggpack_get_buffer( vbi->packetblob[choice] );
		op->bytes = oggpack_bytes( vbi->packetblob[choice] );
		op->b_o_s = 0;
		op->e_o_s = vb->eofflag;
		op->granulepos = vb->granulepos;
		/* for sake of completeness */
		op->packetno = vb->sequence; 
	}

	bm->vb = 0;
	return( 1 );
}

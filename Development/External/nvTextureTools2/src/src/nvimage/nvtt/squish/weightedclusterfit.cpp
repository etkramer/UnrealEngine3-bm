/* -----------------------------------------------------------------------------

	Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk
	Copyright (c) 2006 Ignacio Castano                      icastano@nvidia.com

	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files (the 
	"Software"), to	deal in the Software without restriction, including
	without limitation the rights to use, copy, modify, merge, publish,
	distribute, sublicense, and/or sell copies of the Software, and to 
	permit persons to whom the Software is furnished to do so, subject to 
	the following conditions:

	The above copyright notice and this permission notice shall be included
	in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
	OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
	CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
	TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
	
   -------------------------------------------------------------------------- */

#include "weightedclusterfit.h"
#include "colourset.h"
#include "colourblock.h"
#include <cfloat>


namespace squish {

WeightedClusterFit::WeightedClusterFit( ColourSet const* colours, int flags ) :
	ColourFit( colours, flags )
{
	// initialise the best error
#if SQUISH_USE_SIMD
	m_besterror = VEC4_CONST( FLT_MAX );
#else
	m_besterror = FLT_MAX;
#endif

	// cache some values
	int const count = m_colours->GetCount();
	Vec3 const* values = m_colours->GetPoints();
	
	// get the covariance matrix
	Sym3x3 covariance = ComputeWeightedCovariance( count, values, m_colours->GetWeights() );
	
	// compute the principle component
	Vec3 principle = ComputePrincipleComponent( covariance );

	// build the list of values
	float dps[16];
	for( int i = 0; i < count; ++i )
	{
		dps[i] = Dot( values[i], principle );
		m_order[i] = i;
	}
	
	// stable sort
	for( int i = 0; i < count; ++i )
	{
		for( int j = i; j > 0 && dps[j] < dps[j - 1]; --j )
		{
			std::swap( dps[j], dps[j - 1] );
			std::swap( m_order[j], m_order[j - 1] );
		}
	}
	
	// weight all the points
#if SQUISH_USE_SIMD
	Vec4 const* unweighted = m_colours->GetPointsSimd();
	Vec4 const* weights = m_colours->GetWeightsSimd();
	m_xxsum = VEC4_CONST( 0.0f );
	m_xsum = VEC4_CONST( 0.0f );
#else
	Vec3 const* unweighted = m_colours->GetPoints();
	float const* weights = m_colours->GetWeights();
	m_xxsum = Vec3( 0.0f );
	m_xsum = Vec3( 0.0f );
	m_wsum = 0.0f;	
#endif
	
	for( int i = 0; i < count; ++i )
	{
		int p = m_order[i];
		m_weighted[i] = weights[p] * unweighted[p];
		m_xxsum += m_weighted[i] * m_weighted[i];
		m_xsum += m_weighted[i];
#if !SQUISH_USE_SIMD		
		m_weights[i] = weights[p];
		m_wsum += m_weights[i];
#endif
	}
}


void WeightedClusterFit::setMetric(float r, float g, float b)
{
#if SQUISH_USE_SIMD
	m_metric = Vec4(r, g, b, 0);
#else
	m_metric = Vec3(r, g, b);
#endif
}

float WeightedClusterFit::bestError() const
{
#if SQUISH_USE_SIMD
	Vec4 x = m_xxsum * m_metric;
	Vec4 error = m_besterror + x.SplatX() + x.SplatY() + x.SplatZ();
	return error.GetVec3().X();
#else
	return m_besterror + Dot(m_xxsum, m_metric);
#endif

}

#if SQUISH_USE_SIMD

void WeightedClusterFit::Compress3( void* block )
{
	Vec4 const one = VEC4_CONST(1.0f);
	Vec4 const zero = VEC4_CONST(0.0f);
	Vec4 const half(0.5f, 0.5f, 0.5f, 0.25f);
	Vec4 const two = VEC4_CONST(2.0);
	 
	// declare variables
	Vec4 beststart = VEC4_CONST( 0.0f );
	Vec4 bestend = VEC4_CONST( 0.0f );
	Vec4 besterror = VEC4_CONST( FLT_MAX );

	Vec4 x0 = zero;
	
	int b0 = 0, b1 = 0;

	// check all possible clusters for this total order
	for( int c0 = 0; c0 <= 16; c0++)
	{	
		Vec4 x1 = zero;
		
		for( int c1 = 0; c1 <= 16-c0; c1++)
		{
			Vec4 const x2 = m_xsum - x1 - x0;
			
			//Vec3 const alphax_sum = x0 + x1 * 0.5f;
			//float const alpha2_sum = w0 + w1 * 0.25f;
			Vec4 const alphax_sum = MultiplyAdd(x1, half, x0); // alphax_sum, alpha2_sum
			Vec4 const alpha2_sum = alphax_sum.SplatW();
			
			//Vec3 const betax_sum = x2 + x1 * 0.5f;
			//float const beta2_sum = w2 + w1 * 0.25f;
			Vec4 const betax_sum = MultiplyAdd(x1, half, x2); // betax_sum, beta2_sum
			Vec4 const beta2_sum = betax_sum.SplatW();
			
			//float const alphabeta_sum = w1 * 0.25f;
			Vec4 const alphabeta_sum = (x1 * half).SplatW(); // alphabeta_sum
			
			// float const factor = 1.0f / (alpha2_sum * beta2_sum - alphabeta_sum * alphabeta_sum);
			Vec4 const factor = Reciprocal( NegativeMultiplySubtract(alphabeta_sum, alphabeta_sum, alpha2_sum*beta2_sum) );
			
			Vec4 a = NegativeMultiplySubtract(betax_sum, alphabeta_sum, alphax_sum*beta2_sum) * factor;
			Vec4 b = NegativeMultiplySubtract(alphax_sum, alphabeta_sum, betax_sum*alpha2_sum) * factor;
			
			// clamp the output to [0, 1]
			a = Min( one, Max( zero, a ) );
			b = Min( one, Max( zero, b ) );
			
			// clamp to the grid
			Vec4 const grid( 31.0f, 63.0f, 31.0f, 0.0f );
			Vec4 const gridrcp( 0.03227752766457f, 0.01583151765563f, 0.03227752766457f, 0.0f );
			a = Truncate( MultiplyAdd( grid, a, half ) ) * gridrcp;
			b = Truncate( MultiplyAdd( grid, b, half ) ) * gridrcp;
			
			// compute the error
			Vec4 e1 = MultiplyAdd( a, alphax_sum, b*betax_sum );
			Vec4 e2 = MultiplyAdd( a*a, alpha2_sum, b*b*beta2_sum );
			Vec4 e3 = MultiplyAdd( a*b*alphabeta_sum - e1, two, e2 );
			
			// apply the metric to the error term
			Vec4 e4 = e3 * m_metric;
			Vec4 error = e4.SplatX() + e4.SplatY() + e4.SplatZ();
			
			// keep the solution if it wins
			if( CompareAnyLessThan( error, besterror ) )
			{
				besterror = error;
				beststart = a;
				bestend = b;
				b0 = c0;
				b1 = c1;
			}
			
			x1 += m_weighted[c0+c1];
		}
		
		x0 += m_weighted[c0];
	}

	// save the block if necessary
	if( CompareAnyLessThan( besterror, m_besterror ) )
	{
		// compute indices from cluster sizes.
		u8 bestindices[16];
		{
			int i = 0;
			for(; i < b0; i++) {
				bestindices[i] = 0;
			}
			for(; i < b0+b1; i++) {
				bestindices[i] = 2;
			}
			for(; i < 16; i++) {
				bestindices[i] = 1;
			}
		}
		
		// remap the indices
		u8 ordered[16];
		for( int i = 0; i < 16; ++i )
			ordered[m_order[i]] = bestindices[i];
		
		// save the block
		WriteColourBlock3( beststart.GetVec3(), bestend.GetVec3(), ordered, block );
		
		// save the error
		m_besterror = besterror;
	}
}

void WeightedClusterFit::Compress4( void* block )
{
	Vec4 const one = VEC4_CONST(1.0f);
	Vec4 const zero = VEC4_CONST(0.0f);
	Vec4 const half = VEC4_CONST(0.5f);
	Vec4 const two = VEC4_CONST(2.0);
	Vec4 const onethird( 1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f, 1.0f/9.0f );
	Vec4 const twothirds( 2.0f/3.0f, 2.0f/3.0f, 2.0f/3.0f, 4.0f/9.0f );
	
	// declare variables
	Vec4 beststart = VEC4_CONST( 0.0f );
	Vec4 bestend = VEC4_CONST( 0.0f );
	Vec4 besterror = VEC4_CONST( FLT_MAX );

	Vec4 x0 = zero;
	int b0 = 0, b1 = 0, b2 = 0;

	// check all possible clusters for this total order
	for( int c0 = 0; c0 <= 16; c0++)
	{	
		Vec4 x1 = zero;
		
		for( int c1 = 0; c1 <= 16-c0; c1++)
		{	
			Vec4 x2 = zero;
			
			for( int c2 = 0; c2 <= 16-c0-c1; c2++)
			{
				Vec4 const x3 = m_xsum - x2 - x1 - x0;
				
				//Vec3 const alphax_sum = x0 + x1 * (2.0f / 3.0f) + x2 * (1.0f / 3.0f);
				//float const alpha2_sum = w0 + w1 * (4.0f/9.0f) + w2 * (1.0f/9.0f);
				Vec4 const alphax_sum = x0 + MultiplyAdd(x1, twothirds, x2 * onethird); // alphax_sum, alpha2_sum
				Vec4 const alpha2_sum = alphax_sum.SplatW();
				
				//Vec3 const betax_sum = x3 + x2 * (2.0f / 3.0f) + x1 * (1.0f / 3.0f);
				//float const beta2_sum = w3 + w2 * (4.0f/9.0f) + w1 * (1.0f/9.0f);
				Vec4 const betax_sum = x3 + MultiplyAdd(x2, twothirds, x1 * onethird); // betax_sum, beta2_sum
				Vec4 const beta2_sum = betax_sum.SplatW();
				
				//float const alphabeta_sum = w1 * (2.0f/9.0f) + w2 * (2.0f/9.0f);
				Vec4 const alphabeta_sum = two * (x1 * onethird + x2 * onethird).SplatW(); // alphabeta_sum
				
				// float const factor = 1.0f / (alpha2_sum * beta2_sum - alphabeta_sum * alphabeta_sum);
				Vec4 const factor = Reciprocal( NegativeMultiplySubtract(alphabeta_sum, alphabeta_sum, alpha2_sum*beta2_sum) );
				
				Vec4 a = NegativeMultiplySubtract(betax_sum, alphabeta_sum, alphax_sum*beta2_sum) * factor;
				Vec4 b = NegativeMultiplySubtract(alphax_sum, alphabeta_sum, betax_sum*alpha2_sum) * factor;
				
				// clamp the output to [0, 1]
				a = Min( one, Max( zero, a ) );
				b = Min( one, Max( zero, b ) );
				
				// clamp to the grid
				Vec4 const grid( 31.0f, 63.0f, 31.0f, 0.0f );
				Vec4 const gridrcp( 0.03227752766457f, 0.01583151765563f, 0.03227752766457f, 0.0f );
				a = Truncate( MultiplyAdd( grid, a, half ) ) * gridrcp;
				b = Truncate( MultiplyAdd( grid, b, half ) ) * gridrcp;
				
				// compute the error
				Vec4 e1 = MultiplyAdd( a, alphax_sum, b*betax_sum );
				Vec4 e2 = MultiplyAdd( a*a, alpha2_sum, b*b*beta2_sum );
				Vec4 e3 = MultiplyAdd( a*b*alphabeta_sum - e1, two, e2 );
				
				// apply the metric to the error term
				Vec4 e4 = e3 * m_metric;
				Vec4 error = e4.SplatX() + e4.SplatY() + e4.SplatZ();
				
				// keep the solution if it wins
				if( CompareAnyLessThan( error, besterror ) )
				{
					besterror = error;
					beststart = a;
					bestend = b;
					b0 = c0;
					b1 = c1;
					b2 = c2;
				}
				
				x2 += m_weighted[c0+c1+c2];
			}
			
			x1 += m_weighted[c0+c1];
		}
		
		x0 += m_weighted[c0];
	}

	// save the block if necessary
	if( CompareAnyLessThan( besterror, m_besterror ) )
	{
		// compute indices from cluster sizes.
		u8 bestindices[16];
		{
			int i = 0;
			for(; i < b0; i++) {
				bestindices[i] = 0;
			}
			for(; i < b0+b1; i++) {
				bestindices[i] = 2;
			}
			for(; i < b0+b1+b2; i++) {
				bestindices[i] = 3;
			}
			for(; i < 16; i++) {
				bestindices[i] = 1;
			}
		}
		
		// remap the indices
		u8 ordered[16];
		for( int i = 0; i < 16; ++i )
			ordered[m_order[i]] = bestindices[i];
		
		// save the block
		WriteColourBlock4( beststart.GetVec3(), bestend.GetVec3(), ordered, block );
		
		// save the error
		m_besterror = besterror;
	}
}

#else

void WeightedClusterFit::Compress3( void* block )
{
	// declare variables
	Vec3 beststart( 0.0f );
	Vec3 bestend( 0.0f );
	float besterror = FLT_MAX;

	Vec3 x0(0.0f);
	float w0 = 0.0f;
	
	int b0 = 0, b1 = 0;

	// check all possible clusters for this total order
	for( int c0 = 0; c0 <= 16; c0++)
	{	
		Vec3 x1(0.0f);
		float w1 = 0.0f;
		
		for( int c1 = 0; c1 <= 16-c0; c1++)
		{	
			float w2 = m_wsum - w0 - w1;
			
			// These factors could be entirely precomputed.
			float const alpha2_sum = w0 + w1 * 0.25f;
			float const beta2_sum = w2 + w1 * 0.25f;
			float const alphabeta_sum = w1 * 0.25f;
			float const factor = 1.0f / (alpha2_sum * beta2_sum - alphabeta_sum * alphabeta_sum);
			
			Vec3 const alphax_sum = x0 + x1 * 0.5f;
			Vec3 const betax_sum = m_xsum - alphax_sum;
			
			Vec3 a = (alphax_sum*beta2_sum - betax_sum*alphabeta_sum) * factor;
			Vec3 b = (betax_sum*alpha2_sum - alphax_sum*alphabeta_sum) * factor;
			
			// clamp the output to [0, 1]
			Vec3 const one( 1.0f );
			Vec3 const zero( 0.0f );
			a = Min( one, Max( zero, a ) );
			b = Min( one, Max( zero, b ) );
			
			// clamp to the grid
			Vec3 const grid( 31.0f, 63.0f, 31.0f );
			Vec3 const gridrcp( 0.03227752766457f, 0.01583151765563f, 0.03227752766457f );
			Vec3 const half( 0.5f );
			a = Floor( grid*a + half )*gridrcp;
			b = Floor( grid*b + half )*gridrcp;
			
			// compute the error
			Vec3 e1 = a*a*alpha2_sum + b*b*beta2_sum + 2.0f*( a*b*alphabeta_sum - a*alphax_sum - b*betax_sum );
			
			// apply the metric to the error term
			float error = Dot( e1, m_metric );
			
			// keep the solution if it wins
			if( error < besterror )
			{
				besterror = error;
				beststart = a;
				bestend = b;
				b0 = c0;
				b1 = c1;
			}
			
			x1 += m_weighted[c0+c1];
			w1 += m_weights[c0+c1];
		}
		
		x0 += m_weighted[c0];
		w0 += m_weights[c0];
	}

	// save the block if necessary
	if( besterror < m_besterror )
	{
		// compute indices from cluster sizes.
		u8 bestindices[16];
		{
			int i = 0;
			for(; i < b0; i++) {
				bestindices[i] = 0;
			}
			for(; i < b0+b1; i++) {
				bestindices[i] = 2;
			}
			for(; i < 16; i++) {
				bestindices[i] = 1;
			}
		}
		
		// remap the indices
		u8 ordered[16];
		for( int i = 0; i < 16; ++i )
			ordered[m_order[i]] = bestindices[i];
		
		// save the block
		WriteColourBlock3( beststart, bestend, ordered, block );
		
		// save the error
		m_besterror = besterror;
	}
}

void WeightedClusterFit::Compress4( void* block )
{
	// declare variables
	Vec3 beststart( 0.0f );
	Vec3 bestend( 0.0f );
	float besterror = FLT_MAX;

	Vec3 x0(0.0f);
	float w0 = 0.0f;
	int b0 = 0, b1 = 0, b2 = 0;
	int i = 0;

	// check all possible clusters for this total order
	for( int c0 = 0; c0 <= 16; c0++)
	{	
		Vec3 x1(0.0f);
		float w1 = 0.0f;
		
		for( int c1 = 0; c1 <= 16-c0; c1++)
		{	
			Vec3 x2(0.0f);
			float w2 = 0.0f;
			
			for( int c2 = 0; c2 <= 16-c0-c1; c2++)
			{
				float w3 = m_wsum - w0 - w1 - w2;
				
				float const alpha2_sum = w0 + w1 * (4.0f/9.0f) + w2 * (1.0f/9.0f);
				float const beta2_sum = w3 + w2 * (4.0f/9.0f) + w1 * (1.0f/9.0f);
				float const alphabeta_sum = (w1 + w2) * (2.0f/9.0f);
				float const factor = 1.0f / (alpha2_sum * beta2_sum - alphabeta_sum * alphabeta_sum);
				
				Vec3 const alphax_sum = x0 + x1 * (2.0f / 3.0f) + x2 * (1.0f / 3.0f);
				Vec3 const betax_sum = m_xsum - alphax_sum;
				
				Vec3 a = ( alphax_sum*beta2_sum - betax_sum*alphabeta_sum )*factor;
				Vec3 b = ( betax_sum*alpha2_sum - alphax_sum*alphabeta_sum )*factor;
				
				// clamp the output to [0, 1]
				Vec3 const one( 1.0f );
				Vec3 const zero( 0.0f );
				a = Min( one, Max( zero, a ) );
				b = Min( one, Max( zero, b ) );
				
				// clamp to the grid
				Vec3 const grid( 31.0f, 63.0f, 31.0f );
				Vec3 const gridrcp( 0.03227752766457f, 0.01583151765563f, 0.03227752766457f );
				Vec3 const half( 0.5f );
				a = Floor( grid*a + half )*gridrcp;
				b = Floor( grid*b + half )*gridrcp;
				
				// compute the error
				Vec3 e1 = a*a*alpha2_sum + b*b*beta2_sum + 2.0f*( a*b*alphabeta_sum - a*alphax_sum - b*betax_sum );
				
				// apply the metric to the error term
				float error = Dot( e1, m_metric );
				
				// keep the solution if it wins
				if( error < besterror )
				{
					besterror = error;
					beststart = a;
					bestend = b;
					b0 = c0;
					b1 = c1;
					b2 = c2;
				}
				
				x2 += m_weighted[c0+c1+c2];
				w2 += m_weights[c0+c1+c2];
			}
			
			x1 += m_weighted[c0+c1];
			w1 += m_weights[c0+c1];
		}
		
		x0 += m_weighted[c0];
		w0 += m_weights[c0];
	}

	// save the block if necessary
	if( besterror < m_besterror )
	{
		// compute indices from cluster sizes.
		u8 bestindices[16];
		{
			int i = 0;
			for(; i < b0; i++) {
				bestindices[i] = 0;
			}
			for(; i < b0+b1; i++) {
				bestindices[i] = 2;
			}
			for(; i < b0+b1+b2; i++) {
				bestindices[i] = 3;
			}
			for(; i < 16; i++) {
				bestindices[i] = 1;
			}
		}
		
		// remap the indices
		u8 ordered[16];
		for( int i = 0; i < 16; ++i )
			ordered[m_order[i]] = bestindices[i];
		
		// save the block
		WriteColourBlock4( beststart, bestend, ordered, block );

		// save the error
		m_besterror = besterror;
	}
}

#endif

} // namespace squish

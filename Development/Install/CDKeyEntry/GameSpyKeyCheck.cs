using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;

namespace CDKeyEntry
{
    class GameSpyKeyCheck
    {
        // Base-32 character set
        const string Base32Set = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
        const int GameID = 1727;

        static private bool ConvertFromBase32( byte[] Result, String CleanKey )
        {
            foreach( char Letter in CleanKey )
            {
                // Grab the index
                int Index = Base32Set.IndexOf( Letter );
                if( Index < 0 )
                {
                    return ( false );
                }

                // Shift the array left
                for( int i = Result.Length - 1; i > 0; --i )
                {
                    Result[i] <<= 5;
                    Result[i] |= ( byte )( Result[i - 1] >> 3 );
                }
                Result[0] <<= 5;
                Result[0] |= ( byte )Index;
            }

            return ( true );
        }

        static private UInt16 CreateCheck( byte[] Key )
        {
	        int i;
            uint Check = 0;
            uint XorValue = 0x9CCF9319;

	        for( i = 0 ; i < 8 ; i++ )
	        {
                Check = Check * XorValue + Key[i];
	        }
            return ( ( UInt16 )( ( Check % 65521 ) ^ GameID ) );
        }

        static private string MangleKey( string CleanKey )
        {
            string MangledKey = CleanKey.Substring( 7, 1 );
            MangledKey += CleanKey.Substring( 6, 1 );
            MangledKey += CleanKey.Substring( 5, 1 );
            MangledKey += CleanKey.Substring( 4, 1 );
            MangledKey += CleanKey.Substring( 3, 1 );
            MangledKey += CleanKey.Substring( 2, 1 );
            MangledKey += CleanKey.Substring( 1, 1 );
            MangledKey += CleanKey.Substring( 0, 1 );

            MangledKey += CleanKey.Substring( 15, 1 );
            MangledKey += CleanKey.Substring( 14, 1 );
            MangledKey += CleanKey.Substring( 13, 1 );
            MangledKey += CleanKey.Substring( 12, 1 );
            MangledKey += CleanKey.Substring( 11, 1 );
            MangledKey += CleanKey.Substring( 10, 1 );
            MangledKey += CleanKey.Substring( 9, 1 );
            MangledKey += CleanKey.Substring( 8, 1 );

            return ( MangledKey );
        }

        static private byte[] UnmangleKey( byte[] MangledKeyAndCheck )
        {
            byte[] KeyAndCheck = new byte[10] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

            KeyAndCheck[0] = MangledKeyAndCheck[5];
            KeyAndCheck[1] = MangledKeyAndCheck[6];
            KeyAndCheck[2] = MangledKeyAndCheck[7];
            KeyAndCheck[3] = MangledKeyAndCheck[8];
            KeyAndCheck[4] = MangledKeyAndCheck[9];
            KeyAndCheck[5] = MangledKeyAndCheck[0];
            KeyAndCheck[6] = MangledKeyAndCheck[1];
            KeyAndCheck[7] = MangledKeyAndCheck[2];
            KeyAndCheck[8] = MangledKeyAndCheck[3];
            KeyAndCheck[9] = MangledKeyAndCheck[4];

            return( KeyAndCheck );
        }

        static public bool VerifyClientCheck( string CleanKey )
        {
            byte[] MangledKeyAndCheck = new byte[10] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

            string MangledKey = MangleKey( CleanKey );

            if( !ConvertFromBase32( MangledKeyAndCheck, MangledKey ) )
            {
                return ( false );
            }

            byte[] KeyAndCheck = UnmangleKey( MangledKeyAndCheck );
            
	        UInt16 CheckSum = CreateCheck( KeyAndCheck );
            byte First = ( byte )( CheckSum >> 8 );
            byte Second = ( byte )( CheckSum & 0xff );

            if( First != KeyAndCheck[9] )
            {
                return ( false );
            }

            if( Second != KeyAndCheck[8] )
            {
                return ( false );
            }

            return ( true );
        }
    }
}

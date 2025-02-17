using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Management;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;

namespace CDKeyEntry
{
    public partial class CDKeyEntry : Form
    {
        // Import the relevant functions and stucts
        [StructLayout( LayoutKind.Sequential, CharSet = CharSet.Unicode )]
        internal struct DATA_BLOB
        {
            public int cbData;
            public IntPtr pbData;
        }

        [DllImport( "crypt32.dll", SetLastError = true, CharSet = System.Runtime.InteropServices.CharSet.Auto )]
        private static extern bool CryptProtectData( 
                                    ref DATA_BLOB Source,
                                        string Description,
                                    ref DATA_BLOB Entropy,
                                        IntPtr Reserved,
                                        IntPtr Prompt,
                                        int Flags,
                                    ref DATA_BLOB Final );
#if DEBUG
        [DllImport( "crypt32.dll", SetLastError = true, CharSet = System.Runtime.InteropServices.CharSet.Auto )]
        private static extern bool CryptUnprotectData(
                                    ref DATA_BLOB Source,
                                        string Destination,
                                    ref DATA_BLOB Entropy,
                                        IntPtr Reserved,
                                        IntPtr Prompt,
                                        int Flags,
                                    ref DATA_BLOB Final );
#endif
        // End of interface

        private void InitBlob( ref DATA_BLOB Blob, string InData )
        {
            Blob.cbData = InData.Length;
            Blob.pbData = Marshal.AllocHGlobal( Blob.cbData );
            Marshal.Copy( Encoding.ASCII.GetBytes( InData ), 0, Blob.pbData, Blob.cbData );
        }

        private void InitBlob( ref DATA_BLOB Blob )
        {
            Blob.cbData = 0;
            Blob.pbData = IntPtr.Zero;
        }

        private void InitBlob( ref DATA_BLOB Blob, byte[] InData )
        {
            Blob.cbData = InData.Length;
            Blob.pbData = Marshal.AllocHGlobal( Blob.cbData );
            Marshal.Copy( InData, 0, Blob.pbData, Blob.cbData );
        }

        private void InitBlob( ref DATA_BLOB Blob, long InData )
        {
            Blob.cbData = sizeof( long );
            Blob.pbData = Marshal.AllocHGlobal( Blob.cbData );
            Marshal.Copy( BitConverter.GetBytes( InData ), 0, Blob.pbData, Blob.cbData );
        }

        public long GetMACAddress()
        {
            string MACAddressString = "";
            long MACAddress = 0;

            ManagementClass ManClass = new ManagementClass( "Win32_NetworkAdapterConfiguration" );
            ManagementObjectCollection ManObjCollection = ManClass.GetInstances();

            foreach( ManagementObject ManObj in ManObjCollection )
            {
                if( ManObj["MacAddress"] != null )
                {
                    MACAddressString = ManObj["MacAddress"].ToString().ToUpper();
                }
            }

            if( MACAddressString.Length == 17 )
            {
                string HexLookup = "0123456789ABCDEF";

                for( int i = MACAddressString.Length - 2; i >= 0; i -= 3 )
                {
                    MACAddress <<= 8;
                    MACAddress += HexLookup.IndexOf( MACAddressString[i] ) << 4;
                    MACAddress += HexLookup.IndexOf( MACAddressString[i + 1] );
                }
            }

            return ( MACAddress );
        }

        public CDKeyEntry()
        {
            InitializeComponent();
        }

        private bool ValidateText( string FourChars )
        {
            const string ValidChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

            if( FourChars.Length != 4 )
            {
                return ( false );
            }

            foreach( char Letter in FourChars )
            {
                if( ValidChars.IndexOf( Letter ) < 0 )
                {
                    return ( false );
                }
            }

            return ( true );
        }

        private long GetEntropy()
        {
            byte[] EntropyArray = BitConverter.GetBytes( 0x47726166004A6F65 );
            byte[] MACAddressArray = BitConverter.GetBytes( GetMACAddress() );
            byte[] ResultArray = new byte[8] { 0, 0, 0, 0, 0, 0, 0, 0 };
            for( int i = 0; i < sizeof( long ); i++ )
            {
                ResultArray[i] = ( byte )( ( int )EntropyArray[i] ^ ( int )MACAddressArray[i] );
            }

            long Entropy = BitConverter.ToInt64( ResultArray, 0 );
            return ( Entropy );
        }

        private string EncryptKey( string FullKey )
        {
            string Result = "";

            // Create BLOBs to hold data.
            DATA_BLOB SourceBlob = new DATA_BLOB();
            DATA_BLOB EntropyBlob = new DATA_BLOB();
            DATA_BLOB FinalBlob = new DATA_BLOB();

            long Entropy = GetEntropy();
            InitBlob( ref SourceBlob, FullKey );
            InitBlob( ref EntropyBlob, Entropy );
            InitBlob( ref FinalBlob );

            if( CryptProtectData( ref SourceBlob, null, ref EntropyBlob, IntPtr.Zero, IntPtr.Zero, 1, ref FinalBlob ) )
            {
                byte[] FinalArray = new byte[FinalBlob.cbData];
                Marshal.Copy( FinalBlob.pbData, FinalArray, 0, FinalBlob.cbData );

                foreach( byte EncByte in FinalArray )
                {
                    Result += EncByte.ToString( "000" );
                }
            }

            Marshal.FreeHGlobal( SourceBlob.pbData );
            Marshal.FreeHGlobal( EntropyBlob.pbData );
            Marshal.FreeHGlobal( FinalBlob.pbData );

            return( Result );
        }

#if DEBUG
        private string DecryptKey( string EncKey )
        {
            string Result = "";

            // Create BLOBs to hold data.
            DATA_BLOB SourceBlob = new DATA_BLOB();
            DATA_BLOB EntropyBlob = new DATA_BLOB();
            DATA_BLOB FinalBlob = new DATA_BLOB();

            byte[] FinalArray = new byte[EncKey.Length / 3];
            for( int i = 0; i < EncKey.Length; i += 3 )
            {
                string Num = EncKey.Substring( i, 3 );
                FinalArray[i / 3] = byte.Parse( Num );
            }

            long Entropy = GetEntropy();
            InitBlob( ref SourceBlob, FinalArray );
            InitBlob( ref EntropyBlob, Entropy );
            InitBlob( ref FinalBlob );

            if( CryptUnprotectData( ref SourceBlob, null, ref EntropyBlob, IntPtr.Zero, IntPtr.Zero, 1, ref FinalBlob ) )
            {
                byte[] ResultArray = new byte[FinalBlob.cbData];
                Marshal.Copy( FinalBlob.pbData, ResultArray, 0, FinalBlob.cbData );

                Result = Encoding.ASCII.GetString( ResultArray );
            }

            Marshal.FreeHGlobal( SourceBlob.pbData );
            Marshal.FreeHGlobal( EntropyBlob.pbData );
            Marshal.FreeHGlobal( FinalBlob.pbData ); 
            
            return ( Result );
        }
#endif

        private void ValidateCDKey_Click( object sender, EventArgs e )
        {
            // Local lite validation
            bool IsValid = ValidateText( CDEntry0.Text );
            IsValid &= ValidateText( CDEntry1.Text );
            IsValid &= ValidateText( CDEntry2.Text );
            IsValid &= ValidateText( CDEntry3.Text );

            if( IsValid )
            {
                // Do the GameSpy client side validation
                string CleanKey = CDEntry0.Text + CDEntry1.Text + CDEntry2.Text + CDEntry3.Text;
                IsValid &= GameSpyKeyCheck.VerifyClientCheck( CleanKey );
            }

            // Feedback
            FileInfo Info = new FileInfo( "ValidCDKey" );
            if( Info.Exists )
            {
                Info.Delete();
            }

            if( IsValid )
            {
                string FullKey = CDEntry0.Text + "-" + CDEntry1.Text + "-" + CDEntry2.Text + "-" + CDEntry3.Text;
                string EncryptedKey = EncryptKey( FullKey );

#if DEBUG
                MessageBox.Show( "Your private CD Key is in the correct format. Press OK to continue installation.", "CD Key Valid", MessageBoxButtons.OK );

                string DecryptedKey = DecryptKey( EncryptedKey );
                if( DecryptedKey != FullKey )
                {
                    MessageBox.Show( "Decrypt/Encrypt failure", "Encryption (debug)", MessageBoxButtons.OK );
                }
                else
                {
                    MessageBox.Show( "Decrypt/Encrypt strings match", "Encryption (debug)", MessageBoxButtons.OK );
                }
#endif

                StreamWriter Writer = new StreamWriter( "ValidCDKey" );
                Writer.Write( EncryptedKey );
                Writer.Close();

                Application.Exit();
            }
            else
            {
                MessageBox.Show( Properties.Resources.String_Typo, Properties.Resources.String_Error, MessageBoxButtons.OK );
            }
        }

        private void ExitButton_Click( object sender, EventArgs e )
        {
            Application.Exit();
        }

        private void CDEntry0_TextChanged( object sender, EventArgs e )
        {
            Control InputArea = ( Control )sender;
            if( InputArea.Text.Length == 4 )
            {
                Control Next = GetNextControl( InputArea, true );
                Next.Focus();
            }
        }
    }
}
using System;
using System.Collections.Generic;
using System.Text;

namespace UnrealLoc
{
    class Validator
    {
        private UnrealLoc Main = null;
        private char BadChar = ' ';
        private int Offset = 0;
        private bool DoValidateTags = false;

        enum ValidationError
        {
            NoError,
            IllegalCharacter,
            LocalisedTags,
            MismatchedSquareBrackets,
            MismatchedParentheses,
            MismatchedCurlyBrackets,
            MismatchedTags,
            MismatchedTagCount,
            MismatchedQuotes
        };

        public Validator( UnrealLoc InMain )
        {
            Main = InMain;
        }

        private List<string> ExtractTags( string Value )
        {
            List<string> Tags = new List<string>();

            int OpenIndex = Value.IndexOf( '<' );
            int CloseIndex = Value.IndexOf( '>' );
            while( OpenIndex >= 0 && CloseIndex >= 0 )
            {
                string Tag = Value.Substring( OpenIndex, CloseIndex - OpenIndex + 1 );
                Tags.Add( Tag );

                Value = Value.Substring( CloseIndex + 1 );
                OpenIndex = Value.IndexOf( '<' );
                CloseIndex = Value.IndexOf( '>' );
            }

            return ( Tags );
        }

        private ValidationError ValidateTags( LocEntry LE )
        {
            // Can't check to see if the INT versions have over localised
            if( LE.DefaultLE == null )
            {
                return ( ValidationError.NoError );
            }

            List<string> DefaultTags = ExtractTags( LE.DefaultLE.Value );
            List<string> LocTags = ExtractTags( LE.Value );

            if( LocTags.Count != DefaultTags.Count )
            {
                return ( ValidationError.MismatchedTagCount );
            }

            for( int TagIndex = 0; TagIndex < DefaultTags.Count; TagIndex++ )
            {
                if( DefaultTags[TagIndex] != LocTags[TagIndex] )
                {
                    return ( ValidationError.LocalisedTags );
                }
            }

            return ( ValidationError.NoError );
        }

        private bool CheckAlphaNumeric( char Character )
        {
            if( Character >= 'a' && Character <= 'z' )
            {
                return ( true );
            }
            if( Character >= 'A' && Character <= 'Z' )
            {
                return ( true );
            }
            if( Character >= '0' && Character <= '9' )
            {
                return ( true );
            }

            return ( false );
        }

        private bool CheckAdditional( char Character )
        {
            // Standard punctuation available for all languages - 32 <= c < 128
            const string Punct = " `~!@#$%^&*()-_=+[]{}\\|;:'\",.<>/?";
            // High ascii characters for Western European - 192 <= c < 256 (except multiply and divide)
            const string HiAscii = "��������������������������������������������������������������";
            // Misc characters (word join (two minuses), ellipsis, OE combined (upper), OE combined (lower), umlauted Y (upper)
            string Misc = "���������" + ( char )0x2014 + ( char )0x2026 + ( char )0x0152 + ( char )0x0153 + ( char )0x0178;

            if( Punct.IndexOf( Character ) >= 0 )
            {
                return ( true );
            }

            if( HiAscii.IndexOf( Character ) >= 0 )
            {
                return ( true );
            }

            if( Misc.IndexOf( Character ) >= 0 )
            {
                return ( true );
            }

            if( Character == ( char )0x00a0 )
            {
                return ( true );
            }

            return ( false );
        }

        private void FixupTabs( LanguageInfo Lang, ref string Value )
        {
            if( Value.IndexOf( ( char )0x0009 ) >= 0 )
            {
                Lang.AddWarning( "Replacing tab with four spaces in '" + Value + "'" );
                Value = Value.Replace( "\t", "    " );
            }
        }

        private void FixupSmartQuotes( LanguageInfo Lang, ref string Value )
        {
            char OpenSmartQuote = ( char )0x201c;
            char CloseSmartQuote = ( char )0x201d;
            if( Value.IndexOf( OpenSmartQuote ) >= 0 || Value.IndexOf( CloseSmartQuote ) >= 0 )
            {
                Lang.AddWarning( "Replacing smart quotes with normal quotes in '" + Value + "'" );
                Value = Value.Replace( OpenSmartQuote, '\"' );
                Value = Value.Replace( CloseSmartQuote, '\"' );
            }
        }

        private void FixupSmartSingleQuotes( LanguageInfo Lang, ref string Value )
        {
            char OpenSmartSingleQuote = ( char )0x2018;
            char CloseSmartSingleQuote = ( char )0x2019;
            if( Value.IndexOf( OpenSmartSingleQuote ) >= 0 || Value.IndexOf( CloseSmartSingleQuote ) >= 0 )
            {
                Lang.AddWarning( "Replacing smart single quotes with normal quotes in '" + Value + "'" );
                Value = Value.Replace( OpenSmartSingleQuote, '\'' );
                Value = Value.Replace( CloseSmartSingleQuote, '\'' );
            }
        }

        private void FixupDoubleSingleQuotes( LanguageInfo Lang, ref string Value )
        {
            string DoubleSingleQuote = "''";
            if( Value.IndexOf( DoubleSingleQuote ) >= 0 )
            {
                Lang.AddWarning( "Replacing double single quotes with single double quote in '" + Value + "'" );
                Value = Value.Replace( DoubleSingleQuote, "\"" );
            }
        }

        private void FixupEllipses( LanguageInfo Lang, ref string Value, bool Asian )
        {
            string FivePeriods = ".....";
            string FourPeriods = "....";
            string ThreePeriods = "...";
            char Ellipsis = ( char )0x2026;

            if( Main.Options.bAddEllipses && !Asian )
            {
                if( Value.IndexOf( FivePeriods ) >= 0 )
                {
                    Value = Value.Replace( FivePeriods, Ellipsis.ToString() );
                }

                if( Value.IndexOf( FourPeriods ) >= 0 )
                {
                    Value = Value.Replace( FourPeriods, Ellipsis.ToString() );
                }
                
                if( Value.IndexOf( ThreePeriods ) >= 0 )
                {
                    Lang.AddWarning( "Replacing '...' with ellipses in '" + Value + "'" );
                    Value = Value.Replace( ThreePeriods, Ellipsis.ToString() );
                }
            }
            else if( Main.Options.bRemoveEllipses )
            {
                // Ellipses don't work on XP =(
                if( Value.IndexOf( Ellipsis ) >= 0 )
                {
                    Value = Value.Replace( Ellipsis.ToString(), ThreePeriods );
                    Lang.AddWarning( "Replacing ellipsis with '...' in '" + Value + "'" );
                }
            }
        }

        private void FixupDashes( LanguageInfo Lang, ref string Value, bool Asian )
        {
            char EMDash = ( char )0x2014;

            char ENDash = ( char )0x2013;
            if( Value.IndexOf( ENDash ) >= 0 )
            {
                Lang.AddWarning( "Replacing ENDash with hyphen in '" + Value + "'" );
                Value = Value.Replace( ENDash, '-' );
            }

            if( Asian )
            {
                if( Value.IndexOf( EMDash ) >= 0 )
                {
                    Lang.AddWarning( "Replacing EMDash with hyphen in '" + Value + "'" );
                    Value = Value.Replace( EMDash, '-' );
                }
            }

            if( !Asian )
            {
                if( Value.IndexOf( "--" ) >= 0 )
                {
                    Lang.AddWarning( "Replacing -- with EMDash in '" + Value + "'" );
                    Value = Value.Replace( "--", EMDash.ToString() );
                }
            }
        }

        // Check for valid exceptions e.g. emoticons
        private bool CheckValidTagExceptions( string Line )
        {
            if( Line.Contains( "<3" ) )
            {
                return ( true );
            }

            if( Line.Contains( "\\\\<" ) )
            {
                return ( true );
            }

            return ( false );
        }

        private ValidationError MatchPairs( ref string Value, bool CheckChar )
        {
            int ParensCount = 0;
            int CurlyBracketCount = 0;
            int QuoteCount = 0;
            int TagCount = 0;
            char LastChar = ' ';
            char EastEuroOpenQuote = ( char )0x201e;

            Offset = 0;

            foreach( char C in Value )
            {
                Offset++;

                if( C == '(' )
                {
                    ParensCount++;
                }
                else if( C == ')' && LastChar != ':' && LastChar != ';' )
                {
                    ParensCount--;
                    if( ParensCount < 0 )
                    {
                        return ( ValidationError.MismatchedParentheses );
                    }
                }
                else if( C == '{' )
                {
                    CurlyBracketCount++;
                }
                else if( C == '}' )
                {
                    CurlyBracketCount--;
                    if( CurlyBracketCount < 0 )
                    {
                        return ( ValidationError.MismatchedCurlyBrackets );
                    }
                }
                else if( C == '<' )
                {
                    DoValidateTags = true;
                    TagCount++;
                }
                else if( C == '>' && LastChar != '\\' )
                {
                    TagCount--;
                }
                else if( C == '\"' || C == EastEuroOpenQuote )
                {
                    QuoteCount++;
                }

                if( !CheckChar )
                {
                    LastChar = C;
                    continue;
                }

                if( CheckAlphaNumeric( C ) )
                {
                    LastChar = C;
                    continue;
                }

                if( CheckAdditional( C ) )
                {
                    LastChar = C;
                    continue;
                }

                BadChar = C;
                return ( ValidationError.IllegalCharacter );
            }

            if( QuoteCount % 2 != 0 )
            {
                return ( ValidationError.MismatchedQuotes );
            }

            if( ParensCount > 0 )
            {
                return ( ValidationError.MismatchedParentheses );
            }

            if( CurlyBracketCount > 0 )
            {
                return ( ValidationError.MismatchedCurlyBrackets );
            }

            if( TagCount != 0 )
            {
                if( !CheckValidTagExceptions( Value ) )
                {
                    return ( ValidationError.MismatchedTags );
                }
            }

            return ( ValidationError.NoError );
        }

        private ValidationError ValidateWesternEuro( LanguageInfo Lang, ref string Value )
        {
            FixupEllipses( Lang, ref Value, false );
            FixupDashes( Lang, ref Value, false );

            return ( MatchPairs( ref Value, true ) );
        }

        private ValidationError ValidateRussian( LanguageInfo Lang, ref string Value )
        {
            FixupEllipses( Lang, ref Value, false );
            FixupDashes( Lang, ref Value, false );

            return ( MatchPairs( ref Value, false ) );
        }

        private ValidationError ValidateEasternEuro( LanguageInfo Lang, ref string Value )
        {
            FixupEllipses( Lang, ref Value, false );
            FixupDashes( Lang, ref Value, false );

            return ( MatchPairs( ref Value, false ) );
        }

        private ValidationError ValidateChinese( LanguageInfo Lang, ref string Value )
        {
            FixupEllipses( Lang, ref Value, true );
            FixupDashes( Lang, ref Value, true );

            return ( MatchPairs( ref Value, false ) );
        }

        private ValidationError ValidateKorean( LanguageInfo Lang, ref string Value )
        {
            FixupEllipses( Lang, ref Value, true );
            FixupDashes( Lang, ref Value, true );

            return ( MatchPairs( ref Value, false ) );
        }

        public string Validate( LanguageInfo Lang, ref LocEntry LE )
        {
            ValidationError ReturnCode = ValidationError.NoError;
            string Value = LE.Value;
            Offset = 0;
            BadChar = '0';
            DoValidateTags = false;

            FixupTabs( Lang, ref Value );
            FixupSmartQuotes( Lang, ref Value );
            FixupSmartSingleQuotes( Lang, ref Value );
            FixupDoubleSingleQuotes( Lang, ref Value );

            switch( Lang.LangID )
            {
                case "INT":
                case "FRA":
                case "ITA":
                case "DEU":
                case "ESN":
                case "ESM":
                    ReturnCode = ValidateWesternEuro( Lang, ref Value );
                    break;

                case "RUS":
                    ReturnCode = ValidateRussian( Lang, ref Value );
                    break;

                case "HUN":
                case "POL":
                case "CZE":
                case "SLO":
                    ReturnCode = ValidateEasternEuro( Lang, ref Value );
                    break;

                case "CHT":
                    ReturnCode = ValidateChinese( Lang, ref Value );
                    break;

                case "JPN":
                    break;

                case "KOR":
                    ReturnCode = ValidateKorean( Lang, ref Value );
                    break;

                default:
                    break;
            }

            LE.Value = Value;

            // Don't return errors for notes
            if( Value.IndexOf( "Notes[Folder" ) >= 0 )
            {
                return ( ReturnCode.ToString() );
            }

            // Return if we have found an error so far
            if( ReturnCode != ValidationError.NoError )
            {
                string ReturnString = ReturnCode.ToString();
                switch( ReturnCode )
                {
                    case ValidationError.IllegalCharacter:
                        ReturnString += " '" + BadChar + "' (0x" + ( ( int )BadChar ).ToString( "X" ) + ") at offset " + Offset.ToString() + " in '" + Value + "' in '" + LE.Owner.Owner.Owner.Owner.FileName + "'";
                        break;

                    case ValidationError.MismatchedCurlyBrackets:
                    case ValidationError.MismatchedParentheses:
                    case ValidationError.MismatchedQuotes:
                    case ValidationError.MismatchedSquareBrackets:
                    case ValidationError.MismatchedTagCount:
                    case ValidationError.MismatchedTags:
                    case ValidationError.LocalisedTags:
                        ReturnString += " in '" + Value + "' in '" + LE.Owner.Owner.Owner.Owner.FileName + "'";
                        break;
                }

                Main.Error( Lang, ReturnString );
                return ( ReturnString );
            }

            if( DoValidateTags )
            {
                // Make sure the tags have not been localised
                ReturnCode = ValidateTags( LE );
            }

            return ( ReturnCode.ToString() );
        }
    }
}

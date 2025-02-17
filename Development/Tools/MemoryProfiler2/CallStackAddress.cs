using System;
using System.IO;

namespace MemoryProfiler2
{
    /**
     * Information about an address in a callstack.
     */
    public class FCallStackAddress
    {
        /** Program counter */
        public Int64 ProgramCounter;
        /** Index of filename in name array */
        public Int32 FilenameIndex;
        /** Index of function in name array */
        public Int32 FunctionIndex;
        /** Line number */
        public Int32 LineNumber;

        /**
         * Constructor, initializing indices to a passed in name index and other values to 0
         * 
         * @param	NameIndex	Name index to propagate to all indices.
         */
        public FCallStackAddress(int NameIndex)
        {
            ProgramCounter = 0;
            FilenameIndex = NameIndex;
            FunctionIndex = NameIndex;
            LineNumber = 0;
        }

        /**
         * Serializing constructor.
         * 
         * @param	BinaryStream	                Stream to serialize data from.
         * @param   bShouldSerializeSymbolInfo      Whether symbol info is being serialized.
         */
        public FCallStackAddress(BinaryReader BinaryStream,bool bShouldSerializeSymbolInfo)
        {
            ProgramCounter = BinaryStream.ReadInt64();
            // Platforms not supporting run-time symbol lookup won't serialize the below
            if( bShouldSerializeSymbolInfo )
            {
                FilenameIndex = BinaryStream.ReadInt32();
                FunctionIndex = BinaryStream.ReadInt32();
                LineNumber = BinaryStream.ReadInt32();
            }
        }
    }
}
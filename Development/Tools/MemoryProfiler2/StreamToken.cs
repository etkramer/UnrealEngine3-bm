using System;
using System.IO;

namespace MemoryProfiler2
{
    /**
     * The lower 2 bits of a pointer are piggy-bagged to store what kind of data follows it. This enum lists
     * the possible types.
     */
    public enum EProfilingPayloadType
    {
        TYPE_Malloc		= 0,
        TYPE_Free		= 1,
        TYPE_Realloc	= 2,
        TYPE_Other		= 3,
        // Don't add more than 4 values - we only have 2 bits to store this.
    }

    /**
     *  The the case of TYPE_Other, this enum determines the subtype of the token.
     */
    public enum EProfilingPayloadSubType
    {
        SUBTYPE_EndOfStreamMarker	= 0,
		SUBTYPE_EndOfFileMarker		= 1,
        SUBTYPE_SnapshotMarker		= 2,
		SUBTYPE_Unknown,
    }

    /**
     * Variable sized token emitted by capture code. The parsing code ReadNextToken deals with this and updates
     * internal data. The calling code is responsible for only accessing member variables associated with the type.
     */
    public class FStreamToken
    {
        /** Type of token */
        public EProfilingPayloadType Type;
        /** Subtype of token if it's of TYPE_Other */
        public EProfilingPayloadSubType SubType;
        /** Pointer in the caes of alloc/ free */
        public UInt32 Pointer;
        /** Old pointer in the case of realloc */
        public UInt32 OldPointer;
        /** New pointer in the case of realloc */
        public UInt32 NewPointer;
        /** Index into callstack array */
        public Int32 CallStackIndex;
        /** Size of allocation in alloc/ realloc case */
        public Int32 Size;
        /** Payload if type is TYPE_Other. */
        public UInt32 Payload;

        /** 
         * Updates the token with data read from passed in stream and returns whether we've reached the end.
         */
        public bool ReadNextToken(BinaryReader BinaryStream)
        {
            bool bReachedEndOfStream = false;

            // Read the pointer and convert to token type by looking at lowest 2 bits. Pointers are always
            // 4 byte aligned so need to clear them again after the conversion.
            Pointer = BinaryStream.ReadUInt32();
            int TokenType = ((int)Pointer) & 3;
            Pointer = (UInt32)((long)Pointer & ~3);
            SubType = EProfilingPayloadSubType.SUBTYPE_Unknown;

            // Serialize based on toke type.
            switch (TokenType)
            {
                // Malloc
                case (int)EProfilingPayloadType.TYPE_Malloc:
                    Type = EProfilingPayloadType.TYPE_Malloc;
                    CallStackIndex = BinaryStream.ReadInt32();
                    Size = BinaryStream.ReadInt32();
                    break;
                // Free
                case (int)EProfilingPayloadType.TYPE_Free:
                    Type = EProfilingPayloadType.TYPE_Free;
                    break;
                // Realloc
                case (int)EProfilingPayloadType.TYPE_Realloc:
                    Type = EProfilingPayloadType.TYPE_Realloc;
                    OldPointer = Pointer;
                    NewPointer = BinaryStream.ReadUInt32();
                    CallStackIndex = BinaryStream.ReadInt32();
                    Size = BinaryStream.ReadInt32();
                    break;
                // Other
                case (int)EProfilingPayloadType.TYPE_Other:
                    Type = EProfilingPayloadType.TYPE_Other;
                    // Read subtype.
                    switch (BinaryStream.ReadInt32())
                    {
                        // End of stream!
                        case (int)EProfilingPayloadSubType.SUBTYPE_EndOfStreamMarker:
                            SubType = EProfilingPayloadSubType.SUBTYPE_EndOfStreamMarker;
                            bReachedEndOfStream = true;
                            break;
                        case (int)EProfilingPayloadSubType.SUBTYPE_SnapshotMarker:
                            SubType = EProfilingPayloadSubType.SUBTYPE_SnapshotMarker;
							break;
                        default:
                            throw new InvalidDataException();
                    }
                    Payload = BinaryStream.ReadUInt32();
                    break;
            }

            return !bReachedEndOfStream;
        }
    }
}	
// BM1
class RFMODSound extends Object
    native(Sound)
    noexport;

var() editconst string SourceFilePath;
var native const name FileType;
var native const int cues;
var native const array<string> cueNames;
var native const UntypedBulkData_Mirror FMODRawData;
var native int RawDataSize;
var native Pointer eventProject;
var array<SoundCue> refCueList;
var native int importTime;
var native int ReadVersion;
var native array<string> bankNames;
var native const UntypedBulkData_Mirror FMODXboxRawData;
var native const UntypedBulkData_Mirror FMODPS3RawData;
var native int CategoryCount;
var native Pointer categoryList;
var native int Flags;
var native byte flagMasterCategoryList;
var native byte flagFullyLoaded;
var native byte flagIsDirty;
var native byte flagRegistered;
var native byte flagCanStream;
var native byte flagPS3RSX;
var native byte flagMemoryStream;
var native byte NumberOfPCStreams;
var native byte NumberOfX360Streams;
var native byte NumberOfPS3Streams;
var native Pointer FSBBankData;
var native int FSBBankDataSize;
var native Pointer FSBBankSound;
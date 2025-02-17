/*=============================================================================
	UnScript.h: UnrealScript execution engine.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*-----------------------------------------------------------------------------
	Native functions.
-----------------------------------------------------------------------------*/

//
// Native function table.
//
typedef void (UObject::*Native)( FFrame& TheStack, RESULT_DECL );
extern Native GNatives[];
BYTE GRegisterNative( INT iNative, const Native& Func );

extern Native GCasts[];
BYTE GRegisterCast( INT CastCode, const Native& Func );


//
// Registering a native function.
//
#define IMPLEMENT_FUNCTION(cls,num,func) \
	extern "C" { Native int##cls##func = (Native)&cls::func; } \
	static BYTE cls##func##Temp = GRegisterNative( num, int##cls##func );


/*-----------------------------------------------------------------------------
	FFrame implementation.
-----------------------------------------------------------------------------*/

inline FFrame::FFrame( UObject* InObject )
:	Node		(InObject ? InObject->GetClass() : NULL)
,	Object		(InObject)
,	Code		(NULL)
,	Locals		(NULL)
,	PreviousFrame	(NULL)
,	OutParms	(NULL)
{}
inline FFrame::FFrame( UObject* InObject, UStruct* InNode, INT CodeOffset, void* InLocals, FFrame* InPreviousFrame )
:	Node		(InNode)
,	Object		(InObject)
,	Code		(&InNode->Script(CodeOffset))
,	Locals		((BYTE*)InLocals)
,	PreviousFrame	(InPreviousFrame)
,	OutParms	(NULL)
{}
inline INT FFrame::ReadInt()
{
	INT Result;
#ifdef REQUIRES_ALIGNED_ACCESS
	appMemcpy(&Result, Code, sizeof(INT));
#else
	Result = *(INT*)Code;
#endif
	Code += sizeof(INT);
	return Result;
}
inline UObject* FFrame::ReadObject()
{
	// we pull 32-bits of data out, which is really a UObject* in some representation (depending on platform)
	DWORD TempCode;
#if defined(REQUIRES_ALIGNED_ACCESS)
	appMemcpy(&TempCode, Code, sizeof(DWORD));
#else
	TempCode = *(DWORD*)Code;
#endif
	// turn that DWORD into a UObject pointer
	UObject* Result = (UObject*)appDWORDToPointer(TempCode);
	Code += sizeof(DWORD);
	return Result;
}
inline FLOAT FFrame::ReadFloat()
{
	FLOAT Result;
#ifdef REQUIRES_ALIGNED_ACCESS
	appMemcpy(&Result, Code, sizeof(FLOAT));
#else
	Result = *(FLOAT*)Code;
#endif
	Code += sizeof(FLOAT);
	return Result;
}
inline INT FFrame::ReadWord()
{
	INT Result;
#ifdef REQUIRES_ALIGNED_ACCESS
	WORD Temporary;
	appMemcpy(&Temporary, Code, sizeof(WORD));
	Result = Temporary;
#else
	Result = *(WORD*)Code;
#endif
	Code += sizeof(WORD);
	return Result;
}
inline VariableSizeType FFrame::ReadVariableSize()
{
	VariableSizeType Result;
#ifdef REQUIRES_ALIGNED_ACCESS
	appMemcpy(&Result, Code, sizeof(VariableSizeType));
#else
	Result = *(VariableSizeType*)Code;
#endif
	Code += sizeof(VariableSizeType);
	return Result;
}
inline FName FFrame::ReadName()
{
	FName Result;
#ifdef REQUIRES_ALIGNED_ACCESS
	appMemcpy(&Result, Code, sizeof(FName));
#else
	Result = *(FName*)Code;
#endif
	Code += sizeof(FName);
	return Result;
}
void GInitRunaway();
FORCEINLINE void FFrame::Step(UObject *Context, RESULT_DECL)
{
	INT B = *Code++;
	(Context->*GNatives[B])(*this,Result);
}


/**
 * This will return the StackTrace of the current callstack from .uc land
 **/
inline FString FFrame::GetStackTrace() const
{
	FString Retval;

	// travel down the stack recording the frames
	TArray<const FFrame*> FrameStack;
	const FFrame* CurrFrame = this;
	while( CurrFrame != NULL )
	{
		FrameStack.AddItem(CurrFrame);
		CurrFrame = CurrFrame->PreviousFrame;
	}
	
	// and then dump them to a string
	Retval += FString( TEXT("Script call stack:\n") );
	for( INT idx = FrameStack.Num() - 1; idx >= 0; idx-- )
	{
		Retval += FString::Printf( TEXT("\t%s\n"), *FrameStack(idx)->Node->GetFullName() );
	}

	return Retval;
}


/*-----------------------------------------------------------------------------
	FStateFrame implementation.
-----------------------------------------------------------------------------*/

inline FStateFrame::FStateFrame(UObject* InObject)
:	FFrame		( InObject )
,	StateNode	( InObject->GetClass() )
,	ProbeMask	( ~(QWORD)0 )
,	bContinuedState(FALSE)
{}

inline FString FStateFrame::Describe()
{
	return Node ? Node->GetFullName() : TEXT("None");
}



/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


/**
 * This header contains the code for serialization of script bytecode and [eventually] tagged property values.
 * Extracted to header file to allow custom definitions of the macros used by these methods
 */
#ifdef VISUAL_ASSIST_HACK
	EExprToken SerializeExpr( INT&, FArchive& );
	INT iCode=0;
	FArchive Ar;
	TArray<BYTE> Script;
	EExprToken Expr=(EExprToken)0;
#endif

#ifndef XFER
	#define XFER(T) {Ar << *(T*)&Script(iCode); iCode += sizeof(T); }
#endif

#ifndef XFERNAME
#if SUPPORTS_SCRIPTPATCH_CREATION
	#define XFERNAME() \
	{ \
		if( !GIsScriptPatcherActive ) \
		{ \
			XFER(FName) \
		} \
		else \
		{ \
			XFER(NAME_INDEX); XFER(INT); \
		} \
	}
#else
	#define XFERNAME() XFER(FName)
#endif	//SUPPORTS_SCRIPTPATCH_CREATION
#endif	//XFERNAME

#ifndef XFERPTR 
#if SUPPORTS_SCRIPTPATCH_CREATION
	#define XFERPTR(T) \
	if( !GIsScriptPatcherActive ) \
	{ \
   	    T AlignedPtr = NULL; \
		DWORD TempCode; \
        if (!Ar.IsLoading()) \
		{ \
			appMemcpy( &TempCode, &Script(iCode), sizeof(T) ); \
			AlignedPtr = (T)appDWORDToPointer(TempCode); \
		} \
		Ar << AlignedPtr; \
		if (!Ar.IsSaving()) \
		{ \
			TempCode = appPointerToDWORD(AlignedPtr); \
			appMemcpy( &Script(iCode), &TempCode, sizeof(T) ); \
		} \
		iCode += sizeof(DWORD); \
	} \
	else \
	{ \
		XFER(INT) \
	}
#else
	#define XFERPTR(T) \
	{ \
   	    T AlignedPtr = NULL; \
		DWORD TempCode; \
        if (!Ar.IsLoading()) \
		{ \
			appMemcpy( &TempCode, &Script(iCode), sizeof(T) ); \
			AlignedPtr = (T)appDWORDToPointer(TempCode); \
		} \
		Ar << AlignedPtr; \
		if (!Ar.IsSaving()) \
		{ \
			TempCode = appPointerToDWORD(AlignedPtr); \
			appMemcpy( &Script(iCode), &TempCode, sizeof(T) ); \
		} \
		iCode += sizeof(DWORD); \
	}
#endif	//	SUPPORTS_SCRIPTPATCH_CREATION
#endif	//	XFERPTR


#ifndef XFER_FUNC_POINTER
	#define XFER_FUNC_POINTER	XFERPTR(UStruct*)
#endif	// XFER_FUNC_POINTER

#ifndef XFER_FUNC_NAME
	#define XFER_FUNC_NAME		XFERNAME()
#endif	// XFER_FUNC_NAME

#ifndef XFER_PROP_POINTER
	#define XFER_PROP_POINTER	XFERPTR(UProperty*)
#endif

#ifndef XFER_OBJECT_POINTER
	#define XFER_OBJECT_POINTER(Type)	XFERPTR(Type)
#endif

#ifndef XFER_LABELTABLE
//@script patcher
#if SUPPORTS_SCRIPTPATCH_CREATION
	#define XFER_LABELTABLE											\
	if ( !GIsScriptPatcherActive )									\
	{																\
		for( ; ; )													\
		{															\
			FLabelEntry* E = (FLabelEntry*)&Script(iCode);			\
			XFER(FLabelEntry);										\
			if( E->Name == NAME_None )								\
				break;												\
		}															\
	}																\
	else															\
	{																\
		/* @script patcher: when script patches are being generated, serialized FName indexes aren't converted into actual FNames, */	\
		/* so in order to determine whether this label is the "break" label, we need to look at the linker's NameMap */					\
		for( ; ; )													\
		{															\
			FLabelEntry* E = (FLabelEntry*)&Script(iCode);			\
			XFER(FLabelEntry);										\
			const INT NameIndex = E->Name.GetIndex();				\
			if ( GetLinker() )										\
			{														\
				if ( GetLinker()->NameMap(NameIndex) == NAME_None )	\
				{													\
					break;											\
				}													\
			}														\
			else													\
			{														\
				if ( E->iCode == MAXWORD )							\
				{													\
					break;											\
				}													\
			}														\
		}															\
	}
#else
	#define XFER_LABELTABLE											\
	for( ; ; )														\
	{																\
		FLabelEntry* E = (FLabelEntry*)&Script(iCode);				\
		XFER(FLabelEntry);											\
		if ( E->Name == NAME_None )									\
		{															\
			break;													\
		}															\
	}
#endif	//SUPPORTS_SCRIPTPATCH_CREATION
#endif	//XFER_LABELTABLE

//DEBUGGER: To mantain compatability between debug and non-debug clases
#ifndef HANDLE_OPTIONAL_DEBUG_INFO
#if CONSOLE
	#define HANDLE_OPTIONAL_DEBUG_INFO (void(0))
#else
	#define HANDLE_OPTIONAL_DEBUG_INFO								\
    if ( iCode < Script.Num() )										\
    {																\
	    int RemPos = Ar.Tell();										\
	    int OldiCode = iCode;										\
	    XFER(BYTE);													\
	    int NextCode = Script(iCode-1);								\
	    int GVERSION = -1;											\
	    if ( NextCode == EX_DebugInfo )								\
	    {															\
		    XFER(INT);												\
		    GVERSION = *(INT*)&Script(iCode-sizeof(INT));			\
	    }															\
	    iCode = OldiCode;											\
	    Ar.Seek( RemPos );											\
	    if ( GVERSION == 100 )										\
		    SerializeExpr( iCode, Ar );								\
    }
#endif
#endif


/** UStruct::SerializeExpr() */
#ifdef SERIALIZEEXPR_INC
	EExprToken Expr=(EExprToken)0;

	// Get expr token.
	XFER(BYTE);
	Expr = (EExprToken)Script(iCode-1);
	if( Expr >= EX_FirstNative )
	{
		// Native final function with id 1-127.
		while( SerializeExpr( iCode, Ar ) != EX_EndFunctionParms );
		HANDLE_OPTIONAL_DEBUG_INFO; //DEBUGGER
	}
	else if( Expr >= EX_ExtendedNative )
	{
		// Native final function with id 256-16383.
		XFER(BYTE);
		while( SerializeExpr( iCode, Ar ) != EX_EndFunctionParms );
		HANDLE_OPTIONAL_DEBUG_INFO; //DEBUGGER
	}
	else switch( Expr )
	{
		case EX_PrimitiveCast:
		{
			// A type conversion.
			XFER(BYTE); //which kind of conversion
			SerializeExpr( iCode, Ar );
			break;
		}
		case EX_InterfaceCast:
		{
			// A conversion from an object varible to a native interface variable.  We use a different bytecode to avoid the branching each time we process a cast token
			XFERPTR(UClass*);	// the interface class to convert to
			SerializeExpr( iCode, Ar );
			break;
		}
		case EX_Let:
		case EX_LetBool:
		case EX_LetDelegate:
		{
			SerializeExpr( iCode, Ar ); // Variable expr.
			SerializeExpr( iCode, Ar ); // Assignment expr.
			break;
		}
		case EX_Jump:
		{
			XFER(WORD); // Code offset.
			break;
		}
		case EX_LocalVariable:
		case EX_InstanceVariable:
		case EX_DefaultVariable:
		case EX_LocalOutVariable:
		{
			XFER_PROP_POINTER;
			break;
		}
		case EX_DebugInfo:
		{
			XFER(INT);	// Version
			XFER(INT);	// Line number
			XFER(INT);	// Character pos
			XFER(BYTE); // OpCode
			break;
		}
		case EX_BoolVariable:
		case EX_InterfaceContext:
		{
			SerializeExpr(iCode,Ar);
			break;
		}
		case EX_Nothing:
		case EX_EndOfScript:
		case EX_EndFunctionParms:
		case EX_EmptyParmValue:
		case EX_IntZero:
		case EX_IntOne:
		case EX_True:
		case EX_False:
		case EX_NoObject:
		case EX_EmptyDelegate:
		case EX_Self:
		case EX_IteratorPop:
		case EX_Stop:
		case EX_IteratorNext:
		case EX_EndParmValue:
		{
			break;
		}
		case EX_ReturnNothing:
		{
			XFER_PROP_POINTER; // the return value property
			break;
		}
		case EX_EatReturnValue:
		{
			XFER_PROP_POINTER; // the return value property
			break;
		}
		case EX_Return:
		{
			SerializeExpr( iCode, Ar ); // Return expression.
			break;
		}
		case EX_FinalFunction:
		{
			XFER_FUNC_POINTER;											// Stack node.
			while( SerializeExpr( iCode, Ar ) != EX_EndFunctionParms ); // Parms.
			HANDLE_OPTIONAL_DEBUG_INFO;									// DEBUGGER
			break;
		}
		case EX_VirtualFunction:
		case EX_GlobalFunction:
		{
			XFER_FUNC_NAME;												// Virtual function name.
			while( SerializeExpr( iCode, Ar ) != EX_EndFunctionParms );	// Parms.
			HANDLE_OPTIONAL_DEBUG_INFO;									//DEBUGGER
			break;
		}
		case EX_DelegateFunction:
		{
			XFER(BYTE);													// local prop
			XFER_PROP_POINTER;											// Delegate property
			XFERNAME();													// Delegate function name (in case the delegate is NULL)
			break;
		}
		case EX_NativeParm:
		{
			XFERPTR(UProperty*);
			break;
		}
		case EX_ClassContext:
		case EX_Context:
		{
			SerializeExpr( iCode, Ar ); // Object expression.
			XFER(WORD);					// Code offset for NULL expressions.
			XFER(VariableSizeType);		// Zero-fill size if skipped.
			SerializeExpr( iCode, Ar ); // Context expression.
			break;
		}
		case EX_DynArrayIterator:
		{
			SerializeExpr( iCode, Ar ); // Array expression
			SerializeExpr( iCode, Ar ); // Array item expression
			XFER(BYTE);	// Index parm present
			SerializeExpr( iCode, Ar ); // Index parm
			XFER(WORD); // Code offset
			break;
		}
		case EX_ArrayElement:
		case EX_DynArrayElement:
		{
			SerializeExpr( iCode, Ar ); // Index expression.
			SerializeExpr( iCode, Ar ); // Base expression.
			break;
		}
		case EX_DynArrayLength:
		{
			SerializeExpr( iCode, Ar ); // Base expression.
			break;
		}
		case EX_DynArrayAdd:
		{
			SerializeExpr( iCode, Ar ); // Base expression
			SerializeExpr( iCode, Ar );	// Count
			break;
		}
		case EX_DynArrayInsert:
		case EX_DynArrayRemove:
		{
			SerializeExpr( iCode, Ar ); // Base expression
			SerializeExpr( iCode, Ar ); // Index
			SerializeExpr( iCode, Ar ); // Count
			break;
 		}
		case EX_DynArrayAddItem:
		case EX_DynArrayRemoveItem:
		{
			SerializeExpr( iCode, Ar ); // Array property expression
			XFER(WORD);					// Number of bytes to skip if NULL context encountered
			SerializeExpr( iCode, Ar ); // Item
			break;
		}
		case EX_DynArrayInsertItem:
		{
			SerializeExpr( iCode, Ar );	// Base expression
			XFER(WORD);					// Number of bytes to skip if NULL context encountered
			SerializeExpr( iCode, Ar ); // Index
			SerializeExpr( iCode, Ar );	// Item
			break;
		}
		case EX_DynArrayFind:
		{
			SerializeExpr( iCode, Ar ); // Array property expression
			XFER(WORD);					// Number of bytes to skip if NULL context encountered
			SerializeExpr( iCode, Ar ); // Search item
			break;
		}
		case EX_DynArrayFindStruct:
		{
			SerializeExpr( iCode, Ar ); // Array property expression
			XFER(WORD);					// Number of bytes to skip if NULL context encountered
			SerializeExpr( iCode, Ar );	// Property name
			SerializeExpr( iCode, Ar ); // Search item
			break;
		}
		case EX_Conditional:
		{
			SerializeExpr( iCode, Ar ); // Bool Expr
			XFER(WORD); // Skip
			SerializeExpr( iCode, Ar ); // Result Expr 1
			XFER(WORD); // Skip2
			SerializeExpr( iCode, Ar ); // Result Expr 2
			break;
		}
		case EX_New:
		{
			SerializeExpr( iCode, Ar ); // Parent expression.
			SerializeExpr( iCode, Ar ); // Name expression.
			SerializeExpr( iCode, Ar ); // Flags expression.
			SerializeExpr( iCode, Ar ); // Class expression.
			break;
		}
		case EX_IntConst:
		{
			XFER(INT);
			break;
		}
		case EX_FloatConst:
		{
			XFER(FLOAT);
			break;
		}
		case EX_StringConst:
		{
			do XFER(BYTE) while( Script(iCode-1) );
			break;
		}
		case EX_UnicodeStringConst:
		{
			do XFER(WORD) while( Script(iCode-1) );
			break;
		}
		case EX_ObjectConst:
		{
			XFER_OBJECT_POINTER(UObject*);
			break;
		}
		case EX_NameConst:
		{
			XFERNAME();
			break;
		}
		case EX_RotationConst:
		{
			XFER(INT); XFER(INT); XFER(INT);
			break;
		}
		case EX_VectorConst:
		{
			XFER(FLOAT); XFER(FLOAT); XFER(FLOAT);
			break;
		}
		case EX_ByteConst:
		case EX_IntConstByte:
		{
			XFER(BYTE);
			break;
		}
		case EX_MetaCast:
		{
			XFER_OBJECT_POINTER(UClass*);
			SerializeExpr( iCode, Ar );
			break;
		}
		case EX_DynamicCast:
		{
			XFER_OBJECT_POINTER(UClass*);
			SerializeExpr( iCode, Ar );
			break;
		}
		case EX_JumpIfNot:
		{
			XFER(WORD); // Code offset.
			SerializeExpr( iCode, Ar ); // Boolean expr.
			break;
		}
		case EX_Iterator:
		{
			SerializeExpr( iCode, Ar ); // Iterator expr.
			XFER(WORD); // Code offset.
			break;
		}
		case EX_Switch:
		{
			XFER(VariableSizeType);		// Value size.
			SerializeExpr( iCode, Ar ); // Switch expr.
			break;
		}
		case EX_Assert:
		{
			XFER(WORD); // Line number.
			XFER(BYTE); // debug mode or not
			SerializeExpr( iCode, Ar ); // Assert expr.
			break;
		}
		case EX_Case:
		{
			WORD W;
			XFER(WORD); // Code offset.
			appMemcpy(&W, &Script(iCode-sizeof(WORD)), sizeof(WORD));
			if( W != MAXWORD )
			{
				SerializeExpr( iCode, Ar ); // Boolean expr.
			}
			break;
		}
		case EX_LabelTable:
		{
			check((iCode&3)==0);

			XFER_LABELTABLE
			break;
		}
		case EX_GotoLabel:
		{
			SerializeExpr( iCode, Ar ); // Label name expr.
			break;
		}
		case EX_Skip:
		{
			XFER(WORD);					// Skip size.
			SerializeExpr( iCode, Ar ); // Expression to possibly skip.
			break;
		}
		case EX_DefaultParmValue:
		{
			XFER(WORD); // Size of the expression for this default parameter - used by the VM to skip over the expression
						// if a value was specified in the function call

			HANDLE_OPTIONAL_DEBUG_INFO;	// DI_NewStack
			SerializeExpr( iCode, Ar ); // Expression for this default parameter value
			HANDLE_OPTIONAL_DEBUG_INFO;	// DI_PrevStack
			XFER(BYTE);	// EX_EndParmValue
			break;
		}
		case EX_StructCmpEq:
		case EX_StructCmpNe:
		{
			XFERPTR(UStruct*); // Struct.
			SerializeExpr( iCode, Ar ); // Left expr.
			SerializeExpr( iCode, Ar ); // Right expr.
			break;
		}
		case EX_StructMember:
		{
			XFER_PROP_POINTER;		// the struct property we're accessing
			XFERPTR(UStruct*);			// the struct which contains the property
			XFER(BYTE);					// byte indicating whether a local copy of the struct must be created in order to access the member property
			XFER(BYTE);					// byte indicating whether the struct member will be modified by the expression it's being used in
			SerializeExpr( iCode, Ar ); // expression corresponding to the struct member property.
			break;
		}
		case EX_DelegateProperty:
		{
			XFER_FUNC_NAME;				// Name of function we're assigning to the delegate.
			break;
		}
		case EX_InstanceDelegate:
		{
			XFER_FUNC_NAME;				// the name of the function assigned to the delegate.
			break;
		}
		case EX_EqualEqual_DelDel:
		case EX_NotEqual_DelDel:
		case EX_EqualEqual_DelFunc:
		case EX_NotEqual_DelFunc:
		{
			while( SerializeExpr( iCode, Ar ) != EX_EndFunctionParms );
			HANDLE_OPTIONAL_DEBUG_INFO; //DEBUGGER
			break;
		}
		default:
		{
			// This should never occur.
			appErrorf( TEXT("Bad expr token %02x"), (BYTE)Expr );
			break;
		}
	}

#endif	//!TAGGED_PROPERTIES_ONLY || SERIALIZEEXPR_ONLY


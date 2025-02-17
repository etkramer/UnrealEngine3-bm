
/*=============================================================================
	UnCorSc.cpp: UnrealScript execution and support code.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.

Description:
	UnrealScript execution and support code.

=============================================================================*/

#include "CorePrivate.h"
#include "OpCode.h"
#include "UMemoryDefines.h"

/**
 * PERF_ISSUE_FINDER
 *
 * DLO is super slow on consoles and should not be used. 
 *
 * Turn this on to have the engine log out when it is DLOing an object so you can see what it is 
 * and hopefully modify how that object is loaded.
 *
 **/
//#define LOG_DYNAMIC_LOAD_OBJECT 1
//#define SHOW_SLOW_UNREALSCRIPT_FUNCTION_CALLS 1
#define SHOW_SLOW_UNREALSCRIPT_FUNCTION_CALLS_TAKING_LONG_TIME_AMOUNT 3.0f // modify this value to look at larger or smaller sets of "bad" actors


#if defined(SHOW_SLOW_UNREALSCRIPT_FUNCTION_CALLS) || LOOKING_FOR_PERF_ISSUES

// due to an bad compiler optimization, we force these functions to be non-optimized (and
// non-inline so that the code isn't inlined and optimized where it's used)
#pragma DISABLE_OPTIMIZATION
FORCENOINLINE void StartMSFunctionTimer( DWORD& Timer )
{
	CLOCK_CYCLES( Timer );
}

FORCENOINLINE void StopMSFunctionTimer( DWORD& Timer, const TCHAR* Desc, const TCHAR *ClassName, const TCHAR* Function, const TCHAR *ObjectName )
{
	UNCLOCK_CYCLES( Timer );
	const DOUBLE MSec = ( DOUBLE )Timer * GSecondsPerCycle * 1000.0f;
	if( MSec > SHOW_SLOW_UNREALSCRIPT_FUNCTION_CALLS_TAKING_LONG_TIME_AMOUNT )
	{
		debugf( NAME_PerfWarning, TEXT( "Time: %10f  UObject::CallFunction %s Function: \"%s::%s\" on \"%s\"" ), MSec, Desc, ClassName, Function, ObjectName );
	}
}
#pragma ENABLE_OPTIMIZATION

#endif

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

void (UObject::*GNatives[EX_Max])( FFrame &Stack, RESULT_DECL );
INT GNativeDuplicate=0;

void (UObject::*GCasts[CST_Max])( FFrame &Stack, RESULT_DECL );
INT GCastDuplicate=0;

#define RUNAWAY_LIMIT 1000000
#if CONSOLE
	#define RECURSE_LIMIT 120
#else
	#define RECURSE_LIMIT 250
#endif

#if !FINAL_RELEASE
	#define DO_GUARD 1
#endif

#if DO_GUARD
	static INT Runaway=0;
	static INT Recurse=0;
	#define CHECK_RUNAWAY {if( ++Runaway > RUNAWAY_LIMIT ) {if(!GDebugger||!GDebugger->NotifyInfiniteLoop()) { debugf(*Stack.GetStackTrace()); Stack.Logf( NAME_Critical, TEXT("Runaway loop detected (over %i iterations)"), RUNAWAY_LIMIT ); } Runaway=0;}}
	void GInitRunaway() {Recurse=Runaway=0;}
#else
	#define CHECK_RUNAWAY
	void GInitRunaway() {}
#endif

#define IMPLEMENT_CAST_FUNCTION(cls,num,func) \
		IMPLEMENT_FUNCTION(cls,-1,func); \
		static BYTE cls##func##CastTemp = GRegisterCast( num, int##cls##func );
 
TCHAR* GetOpCodeName( BYTE OpCode )
{
	switch ( OpCode )
	{
	case DI_Let:				return TEXT("Let");
	case DI_SimpleIf:			return TEXT("SimpleIf");
	case DI_Switch:				return TEXT("Switch");
	case DI_While:				return TEXT("While");
	case DI_Assert:				return TEXT("Assert");
	case DI_Return:				return TEXT("Return");
	case DI_ReturnNothing:		return TEXT("ReturnNothing");
	case DI_NewStack:			return TEXT("NewStack");
	case DI_NewStackLatent:		return TEXT("NewStackLatent");
	case DI_NewStackState:		return TEXT("NewStackState");
	case DI_NewStackLabel:		return TEXT("NewStackLabel");
	case DI_PrevStack:			return TEXT("PrevStack");
	case DI_PrevStackLatent:	return TEXT("PrevStackLatent");
	case DI_PrevStackLabel:		return TEXT("PrevStackLabel");
	case DI_PrevStackState:		return TEXT("PrevStackState");
	case DI_EFP:				return TEXT("EFP");
	case DI_EFPOper:			return TEXT("EFPOper");
	case DI_EFPIter:			return TEXT("EFPIter");
	case DI_ForInit:			return TEXT("ForInit");
	case DI_ForEval:			return TEXT("ForEval");
	case DI_ForInc:				return TEXT("ForInc");
	case DI_BreakLoop:			return TEXT("BreakLoop");
	case DI_BreakFor:			return TEXT("BreakFor");
	case DI_BreakForEach:		return TEXT("BreakForEach");
	case DI_BreakSwitch:		return TEXT("BreakSwitch");
	case DI_ContinueLoop:		return TEXT("ContinueLoop");
	case DI_ContinueForeach:	return TEXT("ContinueForeach");	
	case DI_ContinueFor:		return TEXT("ContinueFor");
	}

	return NULL;
}

#if ENABLE_SCRIPT_TRACING
static INT UTraceIndent=0;
#endif

/*-----------------------------------------------------------------------------
	FFrame implementation.
-----------------------------------------------------------------------------*/

//
// Error or warning handler.
//
void FFrame::Serialize( const TCHAR* V, EName Event )
{
	if( Event==NAME_Critical )
	{
		appErrorf
		(
			TEXT("%s (%s:%04X) %s"),
			*Object->GetFullName(),
			*Node->GetFullName(),
			Code - &Node->Script(0),
			V
		);
	}
	else
	{
		debugf
		(
			NAME_ScriptWarning,
			TEXT("%s (%s:%04X) %s"),
			*Object->GetFullName(),
			*Node->GetFullName(),
			Code - &Node->Script(0),
			V
		);
	}
}

/*-----------------------------------------------------------------------------
	Global script execution functions.
-----------------------------------------------------------------------------*/

//
// Have an object go to a named state, and idle at no label.
// If state is NAME_None or was not found, goes to no state.
// Returns 1 if we went to a state, 0 if went to no state.
//
EGotoState UObject::GotoState( FName NewState, UBOOL bForceEvents, UBOOL bKeepStack )
{
	// check to see if this object even supports states
	if (StateFrame == NULL) 
	{
		return GOTOSTATE_NotFound;
	}
	// find the new state
	UState* StateNode = NULL;
	FName OldStateName = StateFrame->StateNode != Class ? StateFrame->StateNode->GetFName() : FName(NAME_None);
	if (NewState != NAME_Auto) 
	{
		// Find regular state.
		StateNode = FindState( NewState );
	}
	else
	{
		// find the auto state.
        // the script compiler marks the class itself as auto when no state is so we can skip the iterator
		if (!(GetClass()->StateFlags & STATE_Auto))
		{
			for (TFieldIterator<UState,CASTCLASS_UState> It(GetClass()); It && !StateNode; ++It)
			{
				if (It->StateFlags & STATE_Auto)
				{
					StateNode = *It;
				}
			}
		}
	}
	// if no state was found, then go to the default class state
	if (!StateNode) 
	{
		NewState  = NAME_None;
		StateNode = GetClass();
	}
	else
	if (NewState == NAME_Auto)
	{
		// going to auto state.
		NewState = StateNode->GetFName();
	}

	// Clear the nested state stack
	if( !bKeepStack )
	{
		// If we had a built up state stack
		if( StateFrame->StateNode != NULL &&
			StateFrame->StateStack.Num() )
		{
			// Pop all states
			PopState( *StateFrame, TRUE );
		}
		else
		{
			// Otherwise, just empty the stack
			StateFrame->StateStack.Empty();
		}
	}

	// Send EndState notification.
	if (bForceEvents ||
		(OldStateName!=NAME_None && 
		 NewState!=OldStateName) )
	{
		if ( IsProbing(NAME_EndState) && !HasAnyFlags(RF_InEndState) )
		{
			ClearFlags( RF_StateChanged );
			SetFlags( RF_InEndState );
			eventEndState(NewState);
			ClearFlags( RF_InEndState );
			if( HasAnyFlags(RF_StateChanged) )
			{
				return GOTOSTATE_Preempted;
			}
		}
	
		/*
		When changing states from within state code, the UDebugger must be notified about the state change
		in order to clear the debugger stack node that entered this state.  The PREVSTACK debugger opcode
		that would normally handle this will not be processed since it is part of the old state node's bytecode,
		which will not be processed after the state node is changed (AActor::ProcessState() would simply start
		processing the new state's bytecode).  By using the DI_PrevStackState opcode, the UDebugger will defer removal
		of the debugger stack node corresponding to this state if the debugger node is not currently the top node, such
		as when a state change happens inside a function called from state code.
		-- UDebugger */
		if ( GDebugger && StateFrame->Node == StateFrame->StateNode )
		{
			GDebugger->DebugInfo(this, StateFrame, DI_PrevStackState, 0, 0);
		}
	}

	// clear the previous latent action if any
	StateFrame->LatentAction = 0;
	// set the new state
	StateFrame->Node = StateNode;
	StateFrame->StateNode = StateNode;
	StateFrame->Code = NULL;
	StateFrame->bContinuedState = FALSE;
	StateFrame->ProbeMask = (StateNode->ProbeMask | GetClass()->ProbeMask) & StateNode->IgnoreMask;
	// Send BeginState notification.
	if (bForceEvents ||
		(NewState!=NAME_None &&
		 NewState!=OldStateName &&
		 IsProbing(NAME_BeginState)))
	{
		ClearFlags( RF_StateChanged );
		eventBeginState(OldStateName);
		// check to see if a new state change was instigated
		if( HasAnyFlags(RF_StateChanged) ) 
		{
			return GOTOSTATE_Preempted;
		}
	}
	// Return result.
	if( NewState != NAME_None )
	{
		SetFlags(RF_StateChanged);
		return GOTOSTATE_Success;
	}
	return GOTOSTATE_NotFound;
}

UBOOL UObject::IsInState(FName StateName, UBOOL bTestStateStack)
{
	// if in a valid state
	if( StateFrame != NULL ) 
	{
		// test the current state (and inheritance)
		for( UState* Test=StateFrame->StateNode; Test; Test=Test->GetSuperState() )
		{
			if( Test->GetFName()==StateName )
			{
				return TRUE;
			}
		}

		// if allowed to test the stack
		if (bTestStateStack)
		{
			// walk up the stack looking for a state
			for (INT Idx = 0; Idx < StateFrame->StateStack.Num(); Idx++)
			{
				if (StateFrame->StateStack(Idx).State->GetFName() == StateName)
				{
					return TRUE;		
				}
			}
		}
	}
	return FALSE;
}

//
// Goto a label in the current state.
// Returns 1 if went, 0 if not found.
//
UBOOL UObject::GotoLabel( FName FindLabel )
{
	if( StateFrame )
	{
		StateFrame->LatentAction = 0;
		if( FindLabel != NAME_None )
		{
			for( UState* SourceState=StateFrame->StateNode; SourceState; SourceState=SourceState->GetSuperState() )
			{
				if( SourceState->LabelTableOffset != MAXWORD )
				{
					for( FLabelEntry* Label = (FLabelEntry *)&SourceState->Script(SourceState->LabelTableOffset); Label->Name!=NAME_None; Label++ )
					{
						if( Label->Name==FindLabel )
						{
							StateFrame->Node = SourceState;
							StateFrame->Code = &SourceState->Script(Label->iCode);
							return 1;
						}
					}
				}
			}
		}

		if ( GDebugger != NULL )
		{
			GDebugger->DebugInfo(this, StateFrame, DI_PrevStackState, 0, 0);
		}
		StateFrame->Code = NULL;
	}
	return 0;
}

/*-----------------------------------------------------------------------------
	Natives.
-----------------------------------------------------------------------------*/

void UObject::execDebugInfo( FFrame& Stack, RESULT_DECL ) //DEBUGGER
{
	INT DebugToken	= Stack.ReadInt();
	
	// We check for this to guarantee that we don't attempt to call GDebugger->DebugInfo in situations where
	// the next byte in the stream happened to be the same value as EX_DebugInfo, even though it isn't a real
	// debug opcode.  This often happens when running with release mode script where the next byte in the stream
	// is part of e.g. a pointer address.
	if( DebugToken != 100 )
	{
		Stack.Code -= sizeof(INT);
		Stack.Code--;
		return;
	}
	
	INT LineNumber	= Stack.ReadInt();
	INT InputPos	= Stack.ReadInt();
	BYTE OpCode		= *Stack.Code++;

	// Only valid when the debugger is running.
	if ( GDebugger != NULL )
		GDebugger->DebugInfo( this, &Stack, OpCode, LineNumber, InputPos );
}
IMPLEMENT_FUNCTION( UObject, EX_DebugInfo, execDebugInfo );

//////////////////////////////
// Undefined native handler //
//////////////////////////////

void UObject::execUndefined( FFrame& Stack, RESULT_DECL  )
{
	Stack.Logf( NAME_Critical, TEXT("Unknown code token %02X"), Stack.Code[-1] );
}

///////////////
// Variables //
///////////////

void UObject::execLocalVariable( FFrame& Stack, RESULT_DECL )
{
	checkSlow(Stack.Object==this);
	checkSlow(Stack.Locals!=NULL);
	GProperty = (UProperty*)Stack.ReadObject();
	GPropAddr = Stack.Locals + GProperty->Offset;
	GPropObject = NULL;
	if( Result )
		GProperty->CopyCompleteValue( Result, GPropAddr );
}
IMPLEMENT_FUNCTION( UObject, EX_LocalVariable, execLocalVariable );

void UObject::execInstanceVariable( FFrame& Stack, RESULT_DECL)
{
	GProperty = (UProperty*)Stack.ReadObject();
	GPropAddr = (BYTE*)this + GProperty->Offset;
	GPropObject = this;
	if( Result )
		GProperty->CopyCompleteValue( Result, GPropAddr );
}
IMPLEMENT_FUNCTION( UObject, EX_InstanceVariable, execInstanceVariable );

void UObject::execDefaultVariable( FFrame& Stack, RESULT_DECL )
{
	GProperty = (UProperty*)Stack.ReadObject();

	UObject* DefaultObject = NULL;
	if ( HasAnyFlags(RF_ClassDefaultObject) )
	{
		// we will already be in the context of the class default object
		// if the variable is being accessed from a static function
		DefaultObject = this;
	}
	else
	{
		DefaultObject = GetArchetype();

		// an ObjectArchetype doesn't necessarily have to be of the same class and
		// it's possible that the ObjectArchetype for this object doesn't contain
		// the property we're referencing.  If that's the case, we'll have to use
		// the class defaults instead
		if ( GProperty->Offset >= DefaultObject->GetClass()->GetPropertiesSize() )
		{
			DefaultObject = GetClass()->GetDefaultObject();
		}
	}

	check(DefaultObject);
	BYTE* DefaultData = (BYTE*)DefaultObject;
	GPropAddr = DefaultData + GProperty->Offset;

	GPropObject = NULL;
	if( Result )
		GProperty->CopyCompleteValue( Result, GPropAddr );
}
IMPLEMENT_FUNCTION( UObject, EX_DefaultVariable, execDefaultVariable );

void UObject::execLocalOutVariable(FFrame& Stack, RESULT_DECL)
{
	checkSlow(Stack.Object == this);
	// get the property we need to find
	GProperty = (UProperty*)Stack.ReadObject();
	GPropObject = NULL;
	
	// look through the out parameter infos and find the one that has the address of this property
	FOutParmRec* Out = Stack.OutParms;
	checkSlow(Out);
	while (Out->Property != GProperty)
	{
		Out = Out->NextOutParm;
		checkSlow(Out);
	}
	GPropAddr = Out->PropAddr;

	// if desired, copy the value in that address to Result
	if (Result)
	{
		GProperty->CopyCompleteValue(Result, GPropAddr);
	}
}
IMPLEMENT_FUNCTION(UObject, EX_LocalOutVariable, execLocalOutVariable);

void UObject::execDefaultParmValue( FFrame& Stack, RESULT_DECL )
{
	checkSlow(Stack.Object==this);
	checkSlow(Result!=NULL);

	WORD Offset = Stack.ReadWord();

	// a value was specified for this optional parameter when the function was called
	if ( (GRuntimeUCFlags&RUC_SkippedOptionalParm) == 0 )
	{
		// in this case, just skip over the default value expression
		Stack.Code += Offset;
	}
	else
	{
		// evaluate the default value into the local property address space
		while( *Stack.Code != EX_EndParmValue )
			Stack.Step( Stack.Object, Result );

		// advance past the EX_EndParmValue
		Stack.Code++;
	}

	// reset the runtime flag
	GRuntimeUCFlags &= ~RUC_SkippedOptionalParm;
}
IMPLEMENT_FUNCTION(UObject, EX_DefaultParmValue, execDefaultParmValue);

void UObject::execInterfaceContext( FFrame& Stack, RESULT_DECL)
{
	// get the value of the interface variable
	FScriptInterface InterfaceValue;
	Stack.Step( this, &InterfaceValue );

	if ( Result != NULL )
	{
		// copy the UObject pointer to Result
		*(UObject**)Result = InterfaceValue.GetObject();
	}
}
IMPLEMENT_FUNCTION( UObject, EX_InterfaceContext, execInterfaceContext );

void UObject::execClassContext( FFrame& Stack, RESULT_DECL )
{
	// Get class expression.
	UClass* ClassContext=NULL;
	Stack.Step( this, &ClassContext );

	// Execute expression in class context.
	if( ClassContext )
	{
		Stack.Code += sizeof(WORD) + sizeof(VariableSizeType);

		UObject* DefaultObject = ClassContext->GetDefaultObject();
		check(DefaultObject!=NULL);
		Stack.Step( DefaultObject, Result );
	}
	else
	{
		if ( GProperty )
		{
			Stack.Logf( NAME_Error, TEXT("Accessed null class context '%s'"), *GProperty->GetName() );
		}
		else
		{ 
			Stack.Logf( NAME_Error, TEXT("Accessed null class context") );
		}

		if ( GDebugger )
		{
			GDebugger->NotifyAccessedNone();
		}

		WORD wSkip = Stack.ReadWord();
		VariableSizeType bSize = Stack.ReadVariableSize();
		Stack.Code += wSkip;
		GPropAddr = NULL;
		GPropObject = NULL;
		GProperty = NULL;
		if( Result )
		{
			appMemzero( Result, bSize );
		}
	}
}
IMPLEMENT_FUNCTION( UObject, EX_ClassContext, execClassContext );

void UObject::execArrayElement( FFrame& Stack, RESULT_DECL )
{
	// Get array index expression.
	INT Index=0;
	Stack.Step( Stack.Object, &Index );

	// Get base element (must be a variable!!).
	GProperty = NULL;
	Stack.Step( this, NULL );
	GPropObject = this;

	// Add scaled offset to base pointer.
	if( GProperty && GPropAddr )
	{
		// Bounds check.
		if( Index>=GProperty->ArrayDim || Index<0 )
		{
			// Display out-of-bounds warning and continue on with index clamped to valid range.
			Stack.Logf( NAME_Error, TEXT("Accessed array '%s.%s' out of bounds (%i/%i)"), *GetName(), *GProperty->GetName(), Index, GProperty->ArrayDim );
			Index = Clamp( Index, 0, GProperty->ArrayDim - 1 );
		}

		// Update address.
		GPropAddr += Index * GProperty->ElementSize;
		if( Result )
		{
			GProperty->CopySingleValue( Result, GPropAddr );
		}
	}
}
IMPLEMENT_FUNCTION( UObject, EX_ArrayElement, execArrayElement );

void UObject::execDynArrayElement( FFrame& Stack, RESULT_DECL )
{
	// Get array index expression.
	INT Index=0;
	Stack.Step( Stack.Object, &Index );

	GProperty = NULL;
	Stack.Step( this, NULL );
	GPropObject = this;

	// Add scaled offset to base pointer.
	if( GProperty && GPropAddr )
	{
		UArrayProperty* ArrayProp = (UArrayProperty*)(GProperty);
		checkSlow(ArrayProp);

		FScriptArray* Array=(FScriptArray*)GPropAddr;
		if( Index>=Array->Num() || Index<0 )
		{
			//if we are returning a value, check for out-of-bounds
			if (Result || Index < 0 || (GRuntimeUCFlags & RUC_NeverExpandDynArray))
			{
				if (ArrayProp->GetOuter()->GetClass() == UFunction::StaticClass())
				{
					Stack.Logf(NAME_Error, TEXT("Accessed array '%s' out of bounds (%i/%i)"), *ArrayProp->GetName(), Index, Array->Num());
				}
				else
				{
					Stack.Logf(NAME_Error, TEXT("Accessed array '%s.%s' out of bounds (%i/%i)"), *GetName(), *ArrayProp->GetName(), Index, Array->Num());
				}
				GPropAddr = 0;
				GPropObject = NULL;
				if ( Result )
				{
					appMemzero( Result, ArrayProp->Inner->ElementSize );
				}

				return;
			}

			//if we are setting a value, allow the array to be resized
			else
			{
				INT OrigSize = Array->Num();
				INT Count = Index - OrigSize + 1;
				Array->AddZeroed(Count, ArrayProp->Inner->ElementSize);

				// if this is an array of structs, and the struct has defaults, propagate those now
				UStructProperty* StructInner = Cast<UStructProperty>(ArrayProp->Inner,CLASS_IsAUStructProperty);
				if ( StructInner && StructInner->Struct->GetDefaultsCount() )
				{
					// no need to initialize the element at Index, since this element is being assigned a new value in the next step
					for ( INT i = OrigSize; i < Index; i++ )
					{
						BYTE* Dest = (BYTE*)Array->GetData() + i * ArrayProp->Inner->ElementSize;
						StructInner->InitializeValue( Dest );
					}
				}
			}
		}

		GPropAddr = (BYTE*)Array->GetData() + Index * ArrayProp->Inner->ElementSize;

		// Add scaled offset to base pointer.
		if( Result )
		{
			ArrayProp->Inner->CopySingleValue( Result, GPropAddr );
		}
	}
}
IMPLEMENT_FUNCTION( UObject, EX_DynArrayElement, execDynArrayElement );

void UObject::execDynArrayLength( FFrame& Stack, RESULT_DECL )
{
	GProperty = NULL;
	Stack.Step( this, NULL );
	GPropObject = this;

	if (GPropAddr)
	{
		FScriptArray* Array=(FScriptArray*)GPropAddr;
		if ( !Result )
			GRuntimeUCFlags |= RUC_ArrayLengthSet; //so that EX_Let knows that this is a length 'set'-ting
		else
			*(INT*)Result = Array->Num();
	}
}
IMPLEMENT_FUNCTION( UObject, EX_DynArrayLength, execDynArrayLength );

void UObject::execDynArrayInsert( FFrame& Stack, RESULT_DECL )
{
	GPropObject = this;
	GProperty = NULL;
	Stack.Step( this, NULL );
	UArrayProperty* ArrayProperty = Cast<UArrayProperty>(GProperty);
	FScriptArray* Array=(FScriptArray*)GPropAddr;

	P_GET_INT(Index);
	P_GET_INT(Count);
	if (Array && Count)
	{
		checkSlow(ArrayProperty);
		if ( Count < 0 )
		{
			Stack.Logf( TEXT("Attempt to insert a negative number of elements '%s'"), *ArrayProperty->GetName() );
			return;
		}
		if ( Index < 0 || Index > Array->Num() )
		{
			Stack.Logf( TEXT("Attempt to insert %i elements at %i an %i-element array '%s'"), Count, Index, Array->Num(), *ArrayProperty->GetName() );
			Index = Clamp(Index, 0,Array->Num());
		}
		Array->InsertZeroed( Index, Count, ArrayProperty->Inner->ElementSize);

		// if this is an array of structs, and the struct has defaults, propagate those now
		UStructProperty* StructInner = Cast<UStructProperty>(ArrayProperty->Inner,CLASS_IsAUStructProperty);
		if ( StructInner && StructInner->Struct->GetDefaultsCount() )
		{
			for ( INT i = Index; i < Count; i++ )
			{
				BYTE* Dest = (BYTE*)Array->GetData() + i * ArrayProperty->Inner->ElementSize;
				StructInner->InitializeValue( Dest );
			}
		}
	}
}
IMPLEMENT_FUNCTION( UObject, EX_DynArrayInsert, execDynArrayInsert );

void UObject::execDynArrayAdd( FFrame& Stack, RESULT_DECL )
{	
	GProperty = NULL;
	GPropObject = this;
	Stack.Step( this, NULL );
	UArrayProperty* ArrayProperty = Cast<UArrayProperty>(GProperty);
	FScriptArray* Array=(FScriptArray*)GPropAddr;

	P_GET_INT(Count);
	if (Array && Count)
	{
		checkSlow(ArrayProperty);
		if ( Count < 0 )
		{
			Stack.Logf( TEXT("Attempt to add a negative number of elements '%s'"), *ArrayProperty->GetName() );
			return;
		}
		INT Index = Array->AddZeroed( Count, ArrayProperty->Inner->ElementSize);

		// if this is an array of structs, and the struct has defaults, propagate those now
		UStructProperty* StructInner = Cast<UStructProperty>(ArrayProperty->Inner,CLASS_IsAUStructProperty);
		if ( StructInner && StructInner->Struct->GetDefaultsCount() )
		{
			for ( INT i = Index; i < Count; i++ )
			{
				BYTE* Dest = (BYTE*)Array->GetData() + i * ArrayProperty->Inner->ElementSize;
				StructInner->InitializeValue( Dest );
			}
		}
		// set the return as the index of the first added
		*(INT*)Result = Index;
	}
}
IMPLEMENT_FUNCTION( UObject, EX_DynArrayAdd, execDynArrayAdd );

void UObject::execDynArrayAddItem( FFrame& Stack, RESULT_DECL )
{	
	GProperty = NULL;
	GPropObject = this;
	Stack.Step( this, NULL );
	UArrayProperty* ArrayProp = Cast<UArrayProperty>(GProperty);
	FScriptArray* Array=(FScriptArray*)GPropAddr;

	// if GPropAddr is NULL, we weren't able to read a valid property address from the stream, which should mean that we evaluted a NULL
	// context expression (accessed none)
	if ( GPropAddr != NULL )
	{
		// advance past the word used to hold the skip offset - we won't need it
		//@note: assumes that the data type used by the compiler to store the skip offset is a WORD (@see FScriptCompiler.CompileExpr)
		Stack.Code += sizeof(WORD);

		// figure out what type to read off the stack
		UProperty *InnerProp = ArrayProp->Inner;
		checkSlow(InnerProp != NULL);

		// grab the item
		BYTE *Item = (BYTE*)appAlloca(InnerProp->ElementSize);
		appMemzero(Item,InnerProp->ElementSize);
		Stack.Step(Stack.Object,Item);

		// add it to the array
		INT Index = Array->AddZeroed(1,InnerProp->ElementSize);
		InnerProp->CopySingleValue((BYTE*)Array->GetData() + (Index * InnerProp->ElementSize), Item);

		if ( InnerProp->HasAnyPropertyFlags(CPF_NeedCtorLink) )
		{
			InnerProp->DestroyValue(Item);
		}

		// return the index of the added item
		*(INT*)Result = Index;
	}
	else
	{
		// accessed none while trying to evaluate the address of the array - skip over the parameter expression bytecodes
		//@note: assumes that the data type used by the compiler to store the skip offset is a WORD (@see FScriptCompiler.CompileExpr)
		WORD NumBytesToSkip = Stack.ReadWord();
		Stack.Code += NumBytesToSkip;

		// set the return value to the expected "invalid" value
		*(INT*)Result = INDEX_NONE;
	}
}
IMPLEMENT_FUNCTION( UObject, EX_DynArrayAddItem, execDynArrayAddItem );

void UObject::execDynArrayInsertItem( FFrame& Stack, RESULT_DECL )
{	
	GProperty = NULL;
	GPropObject = this;
	Stack.Step( this, NULL );
	UArrayProperty* ArrayProp = Cast<UArrayProperty>(GProperty);
	FScriptArray* Array=(FScriptArray*)GPropAddr;

	// if GPropAddr is NULL, we weren't able to read a valid property address from the stream, which should mean that we evaluted a NULL
	// context expression (accessed none)
	if ( GPropAddr != NULL )
	{
		// advance past the word used to hold the skip offset - we won't need it
		//@note: assumes that the data type used by the compiler to store the skip offset is a WORD (@see FScriptCompiler.CompileExpr)
		Stack.Code += sizeof(WORD);

		// figure out what type to read off the stack
		UProperty *InnerProp = ArrayProp->Inner;
		checkSlow(InnerProp != NULL);

		P_GET_INT(Index);

		//debugf(TEXT("insert idx: %d"),Index);

		// grab the item
		BYTE *Item = (BYTE*)appAlloca(InnerProp->ElementSize);
		appMemzero(Item,InnerProp->ElementSize);
		Stack.Step(Stack.Object,Item);

		//debugf(TEXT("insert item: %s"),**(*(FString**)Item));

		// add it to the array
		Array->InsertZeroed(Index,1,InnerProp->ElementSize);
		InnerProp->CopySingleValue((BYTE*)Array->GetData() + (Index * InnerProp->ElementSize), Item);

		if ( InnerProp->HasAnyPropertyFlags(CPF_NeedCtorLink) )
		{
			InnerProp->DestroyValue(Item);
		}

		// return the index of the added item
		*(INT*)Result = Index;
	}
	else
	{
		// accessed none while trying to evaluate the address of the array - skip over the parameter expression bytecodes
		//@note: assumes that the data type used by the compiler to store the skip offset is a WORD (@see FScriptCompiler.CompileExpr)
		WORD NumBytesToSkip = Stack.ReadWord();
		Stack.Code += NumBytesToSkip;

		// set the return value to the expected "invalid" value
		*(INT*)Result = INDEX_NONE;
	}
}
IMPLEMENT_FUNCTION( UObject, EX_DynArrayInsertItem, execDynArrayInsertItem );

void UObject::execDynArrayRemoveItem( FFrame& Stack, RESULT_DECL )
{
	GProperty = NULL;
	GPropObject = this;
	Stack.Step( this, NULL );
	UArrayProperty* ArrayProp = Cast<UArrayProperty>(GProperty);
	FScriptArray* Array=(FScriptArray*)GPropAddr;

	// if GPropAddr is NULL, we weren't able to read a valid property address from the stream, which should mean that we evaluted a NULL
	// context expression (accessed none)
	if ( GPropAddr != NULL )
	{
		// advance past the word used to hold the skip offset - we won't need it
		//@note: assumes that the data type used by the compiler to store the skip offset is a WORD (@see FScriptCompiler.CompileExpr)
		Stack.Code += sizeof(WORD);
		// figure out what type to read off the stack
		UProperty *InnerProp = ArrayProp->Inner;
		checkSlow(InnerProp != NULL);

		// grab the item
		BYTE *Item = (BYTE*)appAlloca(InnerProp->ElementSize);
		appMemzero(Item,InnerProp->ElementSize);
		Stack.Step(Stack.Object,Item);

		for (INT Idx = 0; Idx < Array->Num(); Idx++)
		{
			BYTE *CheckItem = ((BYTE*)Array->GetData() + (Idx * InnerProp->ElementSize));
			if (InnerProp->Identical(Item,CheckItem))
			{
				// remove the entry
				InnerProp->DestroyValue((BYTE*)Array->GetData() + (InnerProp->ElementSize * Idx));
				Array->Remove(Idx--,1,InnerProp->ElementSize);
			}
		}
	}
	else
	{
		// accessed none while trying to evaluate the address of the array - skip over the parameter expression bytecodes
		//@note: assumes that the data type used by the compiler to store the skip offset is a WORD (@see FScriptCompiler.CompileExpr)
		WORD NumBytesToSkip = Stack.ReadWord();
		Stack.Code += NumBytesToSkip;
	}
}
IMPLEMENT_FUNCTION( UObject, EX_DynArrayRemoveItem, execDynArrayRemoveItem );

void UObject::execDynArrayRemove( FFrame& Stack, RESULT_DECL )
{	
	GProperty = NULL;
	GPropObject = this;
	Stack.Step( this, NULL );
	UArrayProperty* ArrayProperty = Cast<UArrayProperty>(GProperty);
	FScriptArray* Array=(FScriptArray*)GPropAddr;

	P_GET_INT(Index);
	P_GET_INT(Count);
	if (Array && Count)
	{
		checkSlow(ArrayProperty);
		if ( Count < 0 )
		{
			Stack.Logf( TEXT("Attempt to remove a negative number of elements '%s'"), *ArrayProperty->GetName() );
			return;
		}
		if ( Index < 0 || Index >= Array->Num() || Index + Count > Array->Num() )
		{
			if (Count == 1)
				Stack.Logf( TEXT("Attempt to remove element %i in an %i-element array '%s'"), Index, Array->Num(), *ArrayProperty->GetName() );
			else
				Stack.Logf( TEXT("Attempt to remove elements %i through %i in an %i-element array '%s'"), Index, Index+Count-1, Array->Num(), *ArrayProperty->GetName() );
			Index = Clamp(Index, 0,Array->Num());
			if ( Index + Count > Array->Num() )
				Count = Array->Num() - Index;
		}

		for (INT i=Index+Count-1; i>=Index; i--)
		{
			ArrayProperty->Inner->DestroyValue((BYTE*)Array->GetData() + ArrayProperty->Inner->ElementSize*i);
		}
		Array->Remove( Index, Count, ArrayProperty->Inner->ElementSize);
	}
}
IMPLEMENT_FUNCTION( UObject, EX_DynArrayRemove, execDynArrayRemove );

void UObject::execDynArrayFind( FFrame& Stack, RESULT_DECL )
{
	// the script compiler doesn't allow calling Find on a dynamic array unless it's being assigned to something, so we should _always_ have a Result buffer
	checkSlow(Result);

	GProperty = NULL;
	GPropAddr = NULL;
	GPropObject = this;

	// read the array property off of the stack; if the array is being accessed through a NULL context, we'll just skip past the
	// bytecodes corresponding to the expressions for the parameters of the 'find'
	Stack.Step( this, NULL );

	// if GPropAddr is NULL, we weren't able to read a valid property address from the stream, which should mean that we evaluted a NULL
	// context expression (accessed none)
	if ( GPropAddr != NULL )
	{
		// advance past the word used to hold the skip offset - we won't need it
		//@note: assumes that the data type used by the compiler to store the skip offset is a WORD (@see FScriptCompiler.CompileExpr)
		Stack.Code += sizeof(WORD);

		// got it
		UArrayProperty* ArrayProp = Cast<UArrayProperty>(GProperty);
		checkSlow(ArrayProp);

		FScriptArray* Array=(FScriptArray*)GPropAddr;

		// figure out what type to read off the stack
		UProperty *InnerProp = ArrayProp->Inner;
		checkSlow(InnerProp != NULL);

		// evaluate the value that we should search for, store it in SearchItem
		BYTE *SearchItem = (BYTE*)appAlloca(InnerProp->ElementSize);
		appMemzero(SearchItem,InnerProp->ElementSize);
		Stack.Step(Stack.Object,SearchItem);

		// compare against each element in the array
		INT ResultIndex = INDEX_NONE;
		for (INT Idx = 0; Idx < Array->Num() && ResultIndex == INDEX_NONE; Idx++)
		{
			BYTE *CheckItem = ((BYTE*)Array->GetData() + (Idx * InnerProp->ElementSize));
			if (InnerProp->Identical(SearchItem,CheckItem))
			{
				ResultIndex = Idx;
			}
		}

		if ( InnerProp->HasAnyPropertyFlags(CPF_NeedCtorLink) )
		{
			InnerProp->DestroyValue(SearchItem);
		}

		// assign the resulting index
		*(INT*)Result = ResultIndex;
	}
	else
	{
		// accessed none while trying to evaluate the address of the array - skip over the parameter expression bytecodes
		//@note: assumes that the data type used by the compiler to store the skip offset is a WORD (@see FScriptCompiler.CompileExpr)
		WORD NumBytesToSkip = Stack.ReadWord();
		Stack.Code += NumBytesToSkip;

		// set the return value to the expected "invalid" value
		*(INT*)Result = INDEX_NONE;
	}
}
IMPLEMENT_FUNCTION( UObject, EX_DynArrayFind, execDynArrayFind );

void UObject::execDynArrayFindStruct( FFrame& Stack, RESULT_DECL )
{
	// the script compiler doesn't allow calling Find on a dynamic array unless it's being assigned to something, so we should _always_ have a Result buffer
	checkSlow(Result);

	GProperty = NULL;
	GPropAddr = NULL;
	GPropObject = this;

	// read the array property off of the stack; if the array is being accessed through a NULL context, we'll just skip past the
	// bytecodes corresponding to the expressions for the parameters of the 'find'
	Stack.Step( this, NULL );

	// if GPropAddr is NULL, we weren't able to read a valid property address from the stream, which should mean that we evaluted a NULL
	// context expression (accessed none)
	if ( GPropAddr != NULL )
	{
		// advance past the word used to hold the skip offset - we won't need it
		//@note: assumes that the data type used by the compiler to store the skip offset is a WORD (@see FScriptCompiler.CompileExpr)
		Stack.Code += sizeof(WORD);

		UArrayProperty* ArrayProp = Cast<UArrayProperty>(GProperty);
		checkSlow(ArrayProp);

		FScriptArray* Array=(FScriptArray*)GPropAddr;

		// read the name of the property to search for
		P_GET_NAME(PropName);

		// figure out what type to read off the stack
		UStructProperty* InnerProp = Cast<UStructProperty>(ArrayProp->Inner,CLASS_IsAUStructProperty);
		checkSlow(InnerProp != NULL);

		UStruct *Struct = InnerProp->Struct;

		// find the property in the struct
		UProperty *SearchProp = FindField<UProperty>(Struct, PropName);
		check(SearchProp != NULL);

		// evaluate the value that we should search for, store it in SearchItem
		BYTE *SearchItem = (BYTE*)appAlloca(SearchProp->ElementSize * SearchProp->ArrayDim);
		appMemzero(SearchItem, SearchProp->ElementSize * SearchProp->ArrayDim);
		Stack.Step(Stack.Object,SearchItem);

		// compare against each element in the array
		INT ResultIndex = INDEX_NONE;
		for (INT Idx = 0; Idx < Array->Num(); Idx++)
		{
			// read the struct out of the array
			UStruct &CheckStruct = *(UStruct*)((BYTE*)Array->GetData() + (Idx * InnerProp->ElementSize));
			// check each element in the search property (might be a static array property) and if any fail, then they do not match
			UBOOL bIsIdentical = TRUE;
			for (INT i = 0; i < SearchProp->ArrayDim; i++)
			{
				if (!SearchProp->Identical(SearchItem + (i * SearchProp->ElementSize),((BYTE*)&CheckStruct + SearchProp->Offset + (i * SearchProp->ElementSize))))
				{
					bIsIdentical = FALSE;
					break;
				}
			}
			if (bIsIdentical)
			{
				ResultIndex = Idx;
				break;
			}
		}

		if ( InnerProp->HasAnyPropertyFlags(CPF_NeedCtorLink) )
		{
			SearchProp->DestroyValue(SearchItem);
		}

		// assign the resulting index
		*(INT*)Result = ResultIndex;
	}
	else
	{
		// accessed none while trying to evaluate the address of the array - skip over the parameter expression bytecodes
		//@note: assumes that the data type used by the compiler to store the skip offset is a WORD (@see FScriptCompiler.CompileExpr)
		WORD NumBytesToSkip = Stack.ReadWord();
		Stack.Code += NumBytesToSkip;

		// set the return value to the expected "invalid" value
		*(INT*)Result = INDEX_NONE;
	}
}
IMPLEMENT_FUNCTION( UObject, EX_DynArrayFindStruct, execDynArrayFindStruct );

void UObject::execBoolVariable( FFrame& Stack, RESULT_DECL )
{ 
	// Get bool variable.
	BYTE B = *Stack.Code++;

	// we pull 32-bits of data out, which is really a UBoolProperty* in some representation (depending on platform)
	DWORD TempCode;
#if defined(REQUIRES_ALIGNED_ACCESS)
	appMemcpy(&TempCode, Stack.Code, sizeof(DWORD));
#else
	TempCode = *(DWORD*)Stack.Code;
#endif
	// turn that DWORD into a UBoolProperty pointer
	UBoolProperty* Property = (UBoolProperty*)appDWORDToPointer(TempCode);

	(this->*GNatives[B])( Stack, NULL );
	GProperty = Property;
	GPropObject = this;

	// Note that we're not returning an in-place pointer to the bool, so EX_Let 
	// must take special precautions with bools.
	if( Result )
	{
		*(BITFIELD*)Result = (GPropAddr && (*(BITFIELD*)GPropAddr & ((UBoolProperty*)GProperty)->BitMask)) ? 1 : 0;
	}
}
IMPLEMENT_FUNCTION( UObject, EX_BoolVariable, execBoolVariable );

void UObject::execStructMember( FFrame& Stack, RESULT_DECL )
{
	// Get structure element.
	UProperty* Property = (UProperty*)Stack.ReadObject();
	checkSlow(Property);

	// Read the struct we're accessing
	UStruct* Struct = (UStruct*)Stack.ReadObject();
	checkSlow(Struct);

	// read the byte that indicates whether we must make a copy of the struct in order to access its members
	BYTE bMemberAccessRequiresStructCopy = *Stack.Code++;

	BYTE* Buffer = NULL;
	if ( bMemberAccessRequiresStructCopy != 0 )
	{
		// We must use the struct's aligned size so that if Struct's aligned size is larger than its PropertiesSize, we don't overrun the buffer when
		// UStructProperty::CopyCompleteValue performs an appMemcpy using the struct property's ElementSize (which is always aligned)
		const INT BufferSize = Align(Struct->GetPropertiesSize(),Struct->GetMinAlignment());

		// allocate a buffer that will contain the copy of the struct we're accessing
		Buffer = (BYTE*)appAlloca(BufferSize);
		appMemzero( Buffer, BufferSize );
	}

	// read the byte that indicates whether the struct will be modified by the expression that's using it
	BYTE bStructWillBeModified = *Stack.Code++;

	// set flag so that if we're accessing a member of a struct inside a dynamic array,
	// execDynArrayElement() will throw an error if it is accessed out of bounds instead of expanding it
	DWORD OldUCFlags = GRuntimeUCFlags;
	if (*Stack.Code == EX_DynArrayElement)
	{
		GRuntimeUCFlags |= RUC_NeverExpandDynArray;
	}

	// now evaluate the expression corresponding to the struct value.
	// If bMemberRequiresStructCopy is 1, Buffer will be non-NULL and the value of the struct will be copied into Buffer
	// Otherwise, this will set GPropAddr to the address of the value for the struct, and GProperty to the struct variable
	GPropAddr = NULL;
	Stack.Step( this, Buffer );

	GRuntimeUCFlags = OldUCFlags;

	// if the struct property will be modified and is a variable relevant to netplay, set the object dirty
	if (bStructWillBeModified && GPropObject != NULL && GProperty != NULL && (GProperty->PropertyFlags & CPF_Net))
	{
		GPropObject->NetDirty(GProperty);
	}

	// Set result.
	GProperty = Property;
	GPropObject = this;

	// GPropAddr is non-NULL if the struct evaluated in the previous call to Step is an instance, local, or default variable
	if( GPropAddr )
	{
		GPropAddr += Property->Offset;
	}

	if ( Buffer != NULL )
	{
		// Result is non-NULL when we're accessing the struct member as an r-value;  Result is where we're copying the value of the struct member to
		if( Result )
		{
			Property->CopyCompleteValue( Result, Buffer+Property->Offset );
		}

		// if we allocated a temporary buffer, clean up any properties which may have allocated heap memory
		for( UProperty* P=Struct->ConstructorLink; P; P=P->ConstructorLinkNext )
		{
			P->DestroyValue( Buffer + P->Offset );
		}
	}
	else if ( Result != NULL && GPropAddr != NULL )
	{
		// Result is non-NULL when we're accessing the struct member as an r-value;  Result is where we're copying the value of the struct member to
		Property->CopyCompleteValue( Result, GPropAddr );
	}
}
IMPLEMENT_FUNCTION( UObject, EX_StructMember, execStructMember );

void UObject::execEndOfScript( FFrame& Stack, RESULT_DECL )
{
	appErrorf(TEXT("Execution beyond end of script in %s on %s"), *Stack.Node->GetFullName(), *Stack.Object->GetFullName());
}
IMPLEMENT_FUNCTION( UObject, EX_EndOfScript, execEndOfScript );

/////////////
// Nothing //
/////////////

void UObject::execNothing( FFrame& Stack, RESULT_DECL )
{
	// Do nothing.
}
IMPLEMENT_FUNCTION( UObject, EX_Nothing, execNothing );

/** failsafe for functions that return a value - returns the zero value for a property and logs that control reached the end of a non-void function */
void UObject::execReturnNothing(FFrame& Stack, RESULT_DECL)
{
	// send a warning to the log
	Stack.Logf(NAME_ScriptWarning, TEXT("Control reached the end of non-void function (make certain that all paths through the function 'return <value>'"));

	// copy a zero value into Result, which is the return value data
	UProperty* ReturnProperty = (UProperty*)Stack.ReadObject();
	checkSlow(ReturnProperty != NULL);
	// destroy old value if necessary
	if (ReturnProperty->PropertyFlags & CPF_NeedCtorLink)
	{
		ReturnProperty->DestroyValue(Result);
	}
	appMemzero(Result, ReturnProperty->ArrayDim * ReturnProperty->ElementSize);
}
IMPLEMENT_FUNCTION(UObject, EX_ReturnNothing, execReturnNothing);

/**
 * Only called when an iterator is performed through a null context expression.
 */
void UObject::execIteratorPop( FFrame& Stack, RESULT_DECL )
{
	// Do nothing
}
IMPLEMENT_FUNCTION( UObject, EX_IteratorPop, execIteratorPop );

void UObject::execEmptyParmValue( FFrame& Stack, RESULT_DECL )
{
	// indicates that no value was specified in the function call for this optional parameter
	GRuntimeUCFlags |= RUC_SkippedOptionalParm;
}
IMPLEMENT_FUNCTION( UObject, EX_EmptyParmValue, execEmptyParmValue );

void UObject::execNativeParm( FFrame& Stack, RESULT_DECL )
{
	UProperty* Property = (UProperty*)Stack.ReadObject();
	if( Result )
	{
		GPropObject = NULL;
		if (Property->PropertyFlags & CPF_OutParm)
		{
			// look through the out parameter infos and find the one that has the address of this property
			FOutParmRec* Out = Stack.OutParms;
			checkSlow(Out);
			while (Out->Property != Property)
			{
				Out = Out->NextOutParm;
				checkSlow(Out);
			}
			GPropAddr = Out->PropAddr;
			// no need to copy property value, since the caller is just looking for GPropAddr
		}
		else
		{
			GPropAddr = Stack.Locals + Property->Offset;
			Property->CopyCompleteValue( Result, Stack.Locals + Property->Offset );
		}
	}
}
IMPLEMENT_FUNCTION( UObject, EX_NativeParm, execNativeParm );

void UObject::execEndFunctionParms( FFrame& Stack, RESULT_DECL )
{
	// For skipping over optional function parms without values specified.
	GPropObject = NULL;  
	Stack.Code--;
}
IMPLEMENT_FUNCTION( UObject, EX_EndFunctionParms, execEndFunctionParms );

//////////////
// Commands //
//////////////

void UObject::execStop( FFrame& Stack, RESULT_DECL )
{
	Stack.Code = NULL;
}
IMPLEMENT_FUNCTION( UObject, EX_Stop, execStop );

//@warning: Does not support UProperty's fully, will break when TArray's are supported in UnrealScript!
void UObject::execSwitch( FFrame& Stack, RESULT_DECL )
{
	// Get switch size.
	VariableSizeType bSize = Stack.ReadVariableSize();

	// Get switch expression.
	MS_ALIGN(4) BYTE SwitchBuffer[1024] GCC_ALIGN(4);
	MS_ALIGN(4) BYTE Buffer[1024] GCC_ALIGN(4);
	appMemzero( Buffer,       sizeof(FString) );
	appMemzero( SwitchBuffer, sizeof(FString) );
	Stack.Step( Stack.Object, SwitchBuffer );

	// Check each case clause till we find a match.
	for( ; ; )
	{
		// Skip over case token.
		checkSlow(*Stack.Code==EX_Case);
		Stack.Code++;

		// Get address of next handler.
		INT wNext = Stack.ReadWord();
		if( wNext == MAXWORD ) // Default case or end of cases.
			break;

		// Get case expression.
		Stack.Step( Stack.Object, Buffer );

		// Compare.
		if( bSize ? (appMemcmp(SwitchBuffer,Buffer,bSize)==0) : (*(FString*)SwitchBuffer==*(FString*)Buffer) )
			break;

		// Jump to next handler.
		Stack.Code = &Stack.Node->Script(wNext);
	}
	if( !bSize )
	{
		(*(FString*)SwitchBuffer).~FString();
		(*(FString*)Buffer      ).~FString();
	}
}
IMPLEMENT_FUNCTION( UObject, EX_Switch, execSwitch );

void UObject::execCase( FFrame& Stack, RESULT_DECL )
{
	INT wNext = Stack.ReadWord();
	if( wNext != MAXWORD )
	{
		// Skip expression.
		MS_ALIGN(4) BYTE Buffer[1024] GCC_ALIGN(4);
		appMemzero( Buffer, sizeof(FString) );
		Stack.Step( Stack.Object, Buffer );
	}
}
IMPLEMENT_FUNCTION( UObject, EX_Case, execCase );

void UObject::execJump( FFrame& Stack, RESULT_DECL )
{
	CHECK_RUNAWAY;

	// Jump immediate.
	INT Offset = Stack.ReadWord();
	Stack.Code = &Stack.Node->Script(Offset);
//	Stack.Code = &Stack.Node->Script(Stack.ReadWord() );
}
IMPLEMENT_FUNCTION( UObject, EX_Jump, execJump );

void UObject::execJumpIfNot( FFrame& Stack, RESULT_DECL )
{
	CHECK_RUNAWAY;

	// Get code offset.
	INT wOffset = Stack.ReadWord();

	// Get boolean test value.
	UBOOL Value=0;
	Stack.Step( Stack.Object, &Value );

	// Jump if false.
	if( !Value )
		Stack.Code = &Stack.Node->Script( wOffset );
}
IMPLEMENT_FUNCTION( UObject, EX_JumpIfNot, execJumpIfNot );

void UObject::execConditional( FFrame& Stack, RESULT_DECL )
{
	// Get test expression
	UBOOL CondValue = 0;
	Stack.Step( Stack.Object, &CondValue );
	
	// Get first skip offset
	INT SkipOffset = Stack.ReadWord();
	
	// Skip ahead if false.
	if( !CondValue )
	{
		Stack.Code += SkipOffset+2;
	}
	
	Stack.Step( Stack.Object, Result );
	
	// Get second skip offset
	if ( CondValue )
	{
		SkipOffset = Stack.ReadWord();
	}
	
	// Skip ahead if true
	if( CondValue )
	{
		Stack.Code += SkipOffset;
	}
}
IMPLEMENT_FUNCTION( UObject, EX_Conditional, execConditional );

void UObject::execAssert( FFrame& Stack, RESULT_DECL )
{
	// Get line number.
	INT wLine = Stack.ReadWord();

	// find out whether we are in debug mode and therefore should crash on failure
	BYTE bDebug = *(BYTE*)Stack.Code++;

	// Get boolean assert value.
	DWORD Value=0;
	Stack.Step( Stack.Object, &Value );

	// Check it.
	if( !Value && (!GDebugger || !GDebugger->NotifyAssertionFailed( wLine )) )
	{
		Stack.Logf(TEXT("%s"), *Stack.GetStackTrace());
		Stack.Logf(bDebug ? NAME_Critical : NAME_ScriptWarning, TEXT("Assertion failed, line %i"), wLine);
	}
}
IMPLEMENT_FUNCTION( UObject, EX_Assert, execAssert );

void UObject::execGotoLabel( FFrame& Stack, RESULT_DECL )
{
	CHECK_RUNAWAY;

	P_GET_NAME(N);
	if( !GotoLabel( N ) )
	{
		Stack.Logf( NAME_Error, TEXT("GotoLabel (%s): Label not found"), *N.ToString() ); 
	}
}
IMPLEMENT_FUNCTION( UObject, EX_GotoLabel, execGotoLabel );

////////////////
// Assignment //
////////////////

void UObject::execLet( FFrame& Stack, RESULT_DECL )
{
	checkSlow(!IsA(UBoolProperty::StaticClass()));

	// Get variable address.
	GPropAddr = NULL;
	Stack.Step( Stack.Object, NULL ); // Evaluate variable.
	if( !GPropAddr )
	{
		Stack.Logf( NAME_ScriptWarning, TEXT("Attempt to assign variable through None") );
		static BYTE Crud[1024];//@temp
		GPropAddr = Crud;
		appMemzero( GPropAddr, sizeof(FString) );
	}
	else if ( GPropObject && GProperty && (GProperty->PropertyFlags & CPF_Net) )
	{
		GPropObject->NetDirty(GProperty); //FIXME - use object property instead for performance
	}

	if (GRuntimeUCFlags & RUC_ArrayLengthSet)
	{
		GRuntimeUCFlags &= ~RUC_ArrayLengthSet;
		FScriptArray* Array=(FScriptArray*)GPropAddr;
		UArrayProperty* ArrayProp = (UArrayProperty*)GProperty;
		INT NewSize = 0;
		Stack.Step( Stack.Object, &NewSize); // Evaluate expression into variable.
		if (NewSize > Array->Num())
		{
			INT OldSize = Array->Num();
			INT Count = NewSize - OldSize;
			Array->AddZeroed(Count, ArrayProp->Inner->ElementSize);

			// if this is an array of structs, and the struct has defaults, propagate those now
			UStructProperty* StructInner = Cast<UStructProperty>(ArrayProp->Inner,CLASS_IsAUStructProperty);
			if ( StructInner && StructInner->Struct->GetDefaultsCount() )
			{
				for ( INT i = OldSize; i < NewSize; i++ )
				{
					BYTE* Dest = (BYTE*)Array->GetData() + i * ArrayProp->Inner->ElementSize;
					StructInner->InitializeValue( Dest );
				}
			}
		}
		else if (NewSize < Array->Num())
		{
			for (INT i=Array->Num()-1; i>=NewSize; i--)
				ArrayProp->Inner->DestroyValue((BYTE*)Array->GetData() + ArrayProp->Inner->ElementSize*i);
			Array->Remove(NewSize, Array->Num()-NewSize, ArrayProp->Inner->ElementSize );
		}
	} else
		Stack.Step( Stack.Object, GPropAddr ); // Evaluate expression into variable.
}
IMPLEMENT_FUNCTION( UObject, EX_Let, execLet );

void UObject::execLetBool( FFrame& Stack, RESULT_DECL )
{
	GPropAddr = NULL;
	GProperty = NULL;
	GPropObject = NULL;

	// Get the variable and address to place the data.
	Stack.Step( Stack.Object, NULL );

	/*
		Class bool properties are packed together as bitfields, so in order 
		to set the value on the correct bool, we need to mask it against
		the bool property's BitMask.

		Local bool properties (declared inside functions) are not packed, thus
		their bitmask is always 1.

		Bool properties inside dynamic arrays and tmaps are also not packed together.
		If the bool property we're accessing is an element in a dynamic array, GProperty
		will be pointing to the dynamic array that has a UBoolProperty as its inner, so
		we'll need to check for that.
	*/
	BITFIELD*      BoolAddr     = (BITFIELD*)GPropAddr;
	UBoolProperty* BoolProperty = ExactCast<UBoolProperty>(GProperty);
	if ( BoolProperty == NULL )
	{
		UArrayProperty* ArrayProp = ExactCast<UArrayProperty>(GProperty);
		if ( ArrayProp != NULL )
		{
			BoolProperty = ExactCast<UBoolProperty>(ArrayProp->Inner);
		}
		/* @todo tmaps
		else
		{
			UMapProperty* MapProp = ExactCast<UMapProperty>(GProperty);
		}
		*/
	}

	INT NewValue=0;  // must be INT...execTrue/execFalse return 32 bits. --ryan.
	if ( GPropObject && GProperty && (GProperty->PropertyFlags & CPF_Net) )
	{
		GPropObject->NetDirty(GProperty);
	}

	// evaluate the r-value for this expression into Value
	Stack.Step( Stack.Object, &NewValue );
	if( BoolAddr )
	{
		checkSlow(BoolProperty->IsA(UBoolProperty::StaticClass()));
		if( NewValue )
		{
			*BoolAddr |=  BoolProperty->BitMask;
		}
		else
		{
			*BoolAddr &= ~BoolProperty->BitMask;
		}
	}
}
IMPLEMENT_FUNCTION( UObject, EX_LetBool, execLetBool );

/////////////////////////
// Context expressions //
/////////////////////////

void UObject::execSelf( FFrame& Stack, RESULT_DECL )
{
	// Get Self actor for this context.
	*(UObject**)Result = this;
}
IMPLEMENT_FUNCTION( UObject, EX_Self, execSelf );

void UObject::execContext( FFrame& Stack, RESULT_DECL )
{
	GProperty = NULL;
	// Get object variable.
	UObject* NewContext=NULL;
	Stack.Step( this, &NewContext );

	// Execute or skip the following expression in the object's context.
	if( NewContext != NULL )
	{
		Stack.Code += sizeof(WORD) + sizeof(VariableSizeType);
		Stack.Step( NewContext, Result );
	}
	else
	{
		if ( GProperty )
			Stack.Logf( NAME_Warning, TEXT("Accessed None '%s'"), *GProperty->GetName() );
		else
			Stack.Logf( NAME_Warning, TEXT("Accessed None"));

		// DEBUGGER
		if ( GDebugger )
			GDebugger->NotifyAccessedNone();	// jmw

		WORD wSkip = Stack.ReadWord();
		VariableSizeType bSize = Stack.ReadVariableSize();
		Stack.Code += wSkip;
		GPropAddr = NULL;
		GProperty = NULL;
		GPropObject = NULL;
		if( Result )
			appMemzero( Result, bSize );
	}
}
IMPLEMENT_FUNCTION( UObject, EX_Context, execContext );

////////////////////
// Function calls //
////////////////////

void UObject::execVirtualFunction( FFrame& Stack, RESULT_DECL )
{
	// Call the virtual function.
	CallFunction( Stack, Result, FindFunctionChecked(Stack.ReadName(),0) );
}
IMPLEMENT_FUNCTION( UObject, EX_VirtualFunction, execVirtualFunction );

void UObject::execFinalFunction( FFrame& Stack, RESULT_DECL )
{
	// Call the final function.
	CallFunction( Stack, Result, (UFunction*)Stack.ReadObject() );
}
IMPLEMENT_FUNCTION( UObject, EX_FinalFunction, execFinalFunction );

void UObject::execGlobalFunction( FFrame& Stack, RESULT_DECL )
{
	// Call global version of virtual function.
	CallFunction( Stack, Result, FindFunctionChecked(Stack.ReadName(),1) );
}
IMPLEMENT_FUNCTION( UObject, EX_GlobalFunction, execGlobalFunction );

///////////////////////
// Delegates         //
///////////////////////

void UObject::execDelegateFunction( FFrame& Stack, RESULT_DECL )
{
	BYTE bLocalProp = *(BYTE*)Stack.Code;
	Stack.Code += sizeof(BYTE);

	// Look up delegate property.
	UDelegateProperty* DelegateProperty = (UDelegateProperty*)Stack.ReadObject();
	checkSlow(DelegateProperty);
	checkSlow(DelegateProperty->IsA(UDelegateProperty::StaticClass()));

	// read the delegate info
	FScriptDelegate* Delegate = NULL;
	if (bLocalProp)
	{
		// read off the stack 
		Delegate = (FScriptDelegate*)(Stack.Locals + DelegateProperty->Offset);
	}
	else
	{
		// read off the object
		Delegate = (FScriptDelegate*)((BYTE*)this + DelegateProperty->Offset);
	}
	FName DelegateName = Stack.ReadName();
	if( Delegate->Object && Delegate->Object->IsPendingKill() )
	{
		// disallow delegates being called on deleted objects
		Delegate->Object = NULL;
		Delegate->FunctionName = NAME_None;
	}
	if (Delegate->Object != NULL)
	{
		// attempt to call the delegate in the specified object
		Delegate->Object->CallFunction( Stack, Result, Delegate->Object->FindFunctionChecked(Delegate->FunctionName) );	
	}
	else if (Delegate->FunctionName != NAME_None)
	{
		// attempt to call the delegate in this object
		CallFunction( Stack, Result, FindFunctionChecked(Delegate->FunctionName) );
	}
	// if we are calling a delegate function (not a delegate property), invoke the default implementation
	else if (DelegateProperty->SourceDelegate == NULL)
	{
		// default to the original default delegate
		CallFunction( Stack, Result, FindFunctionChecked(DelegateName) );
	}
	else
	{
		// attempt to call a function through a delegate property, but the value is NULL
		// technically, we could call the default implementation if the property is using a delegate function
		// declared in this class's heirarchy, but the difference would likely be confusing and cause unexpected bugs,
		// so we always fail instead
		Stack.Logf(NAME_ScriptWarning, TEXT("Attempt to call None through delegate property '%s'"), *DelegateProperty->GetName());
		SkipFunction(Stack, Result, DelegateProperty->SourceDelegate);
	}
}
IMPLEMENT_FUNCTION( UObject, EX_DelegateFunction, execDelegateFunction );

void UObject::execDelegateProperty( FFrame& Stack, RESULT_DECL )
{
	FName FunctionName = Stack.ReadName();
	((FScriptDelegate*)Result)->FunctionName = FunctionName;
	if( FunctionName == NAME_None )
		((FScriptDelegate*)Result)->Object = NULL;
	else
		((FScriptDelegate*)Result)->Object = this;
}
IMPLEMENT_FUNCTION( UObject, EX_DelegateProperty, execDelegateProperty );

void UObject::execLetDelegate( FFrame& Stack, RESULT_DECL )
{
	// Get variable address.
	GPropAddr = NULL;
	GProperty = NULL;
	GPropObject = NULL;
	Stack.Step( Stack.Object, NULL ); // Variable.
	FScriptDelegate* DelegateAddr = (FScriptDelegate*)GPropAddr;
	FScriptDelegate Delegate;
	Stack.Step( Stack.Object, &Delegate );
	if( DelegateAddr )
	{
		DelegateAddr->FunctionName = Delegate.FunctionName;
		DelegateAddr->Object	   = Delegate.Object;
	}
}
IMPLEMENT_FUNCTION( UObject, EX_LetDelegate, execLetDelegate );


///////////////////////
// Struct comparison //
///////////////////////

void UObject::execStructCmpEq( FFrame& Stack, RESULT_DECL )
{
	UStruct* Struct  = (UStruct*)Stack.ReadObject();
	checkSlow(Struct);

	// We must use the struct's aligned size so that if Struct's aligned size is larger than its PropertiesSize, we don't overrun the buffer when
	// UStructProperty::CopyCompleteValue performs an appMemcpy using the struct property's ElementSize (which is always aligned)
	const INT BufferSize = Align(Struct->GetPropertiesSize(),Struct->GetMinAlignment());

	BYTE*    Buffer1 = (BYTE*)appAlloca(BufferSize);
	BYTE*    Buffer2 = (BYTE*)appAlloca(BufferSize);
	appMemzero( Buffer1, BufferSize );
	appMemzero( Buffer2, BufferSize );
	Stack.Step( this, Buffer1 );
	Stack.Step( this, Buffer2 );
	*(DWORD*)Result  = Struct->StructCompare( Buffer1, Buffer2 );
	UStructProperty *StructProp = Cast<UStructProperty>(GProperty);
	if (StructProp != NULL && StructProp->HasAnyPropertyFlags(CPF_NeedCtorLink))
	{
		StructProp->DestroyValue(Buffer1);
		StructProp->DestroyValue(Buffer2);
	}
}
IMPLEMENT_FUNCTION( UObject, EX_StructCmpEq, execStructCmpEq );

void UObject::execStructCmpNe( FFrame& Stack, RESULT_DECL )
{
	UStruct* Struct = (UStruct*)Stack.ReadObject();
	checkSlow(Struct);

	// We must use the struct's aligned size so that if Struct's aligned size is larger than its PropertiesSize, we don't overrun the buffer when
	// UStructProperty::CopyCompleteValue performs an appMemcpy using the struct property's ElementSize (which is always aligned)
	const INT BufferSize = Align(Struct->GetPropertiesSize(),Struct->GetMinAlignment());

	BYTE*    Buffer1 = (BYTE*)appAlloca(BufferSize);
	BYTE*    Buffer2 = (BYTE*)appAlloca(BufferSize);
	appMemzero( Buffer1, BufferSize );
	appMemzero( Buffer2, BufferSize );
	Stack.Step( this, Buffer1 );
	Stack.Step( this, Buffer2 );
	*(DWORD*)Result = !Struct->StructCompare(Buffer1,Buffer2);
	UStructProperty *StructProp = Cast<UStructProperty>(GProperty);
	if (StructProp != NULL && StructProp->HasAnyPropertyFlags(CPF_NeedCtorLink))
	{
		StructProp->DestroyValue(Buffer1);
		StructProp->DestroyValue(Buffer2);
	}
}
IMPLEMENT_FUNCTION( UObject, EX_StructCmpNe, execStructCmpNe );


/////////////////////////
// Delegate comparison //
/////////////////////////
void UObject::execEqualEqual_DelegateDelegate( FFrame& Stack, RESULT_DECL )
{
	P_GET_DELEGATE(A);
	P_GET_DELEGATE(B);
	P_FINISH;			// required because the compiler emits a EX_EndFunctionParms token for this bytecode
	if ( A.Object == NULL && A.FunctionName != NAME_None )
	{
		A.Object = this;
	}
	if ( B.Object == NULL && B.FunctionName != NAME_None )
	{
		B.Object = this;
	}
	*(UBOOL*)Result = (A == B);
}
IMPLEMENT_FUNCTION( UObject, EX_EqualEqual_DelDel, execEqualEqual_DelegateDelegate );

void UObject::execNotEqual_DelegateDelegate( FFrame& Stack, RESULT_DECL )
{
	P_GET_DELEGATE(A);
	P_GET_DELEGATE(B);
	P_FINISH;			// required because the compiler emits a EX_EndFunctionParms token for this bytecode
	if ( A.Object == NULL && A.FunctionName != NAME_None )
	{
		A.Object = this;
	}
	if ( B.Object == NULL && B.FunctionName != NAME_None )
	{
		B.Object = this;
	}
	*(UBOOL*)Result = (A != B);
}
IMPLEMENT_FUNCTION( UObject, EX_NotEqual_DelDel, execNotEqual_DelegateDelegate );

void UObject::execEqualEqual_DelegateFunction( FFrame& Stack, RESULT_DECL )
{
	P_GET_DELEGATE(A);
	P_GET_DELEGATE(B);
	P_FINISH;			// required because the compiler emits a EX_EndFunctionParms token for this bytecode
	if ( A.Object == NULL && A.FunctionName != NAME_None )
	{
		A.Object = this;
	}
	if ( B.Object == NULL && B.FunctionName != NAME_None )
	{
		B.Object = this;
	}
	*(UBOOL*)Result = (A == B);
}
IMPLEMENT_FUNCTION( UObject, EX_EqualEqual_DelFunc, execEqualEqual_DelegateFunction );

void UObject::execNotEqual_DelegateFunction( FFrame& Stack, RESULT_DECL )
{
	P_GET_DELEGATE(A);
	P_GET_DELEGATE(B);
	P_FINISH;			// required because the compiler emits a EX_EndFunctionParms token for this bytecode
	if ( A.Object == NULL && A.FunctionName != NAME_None )
	{
		A.Object = this;
	}
	if ( B.Object == NULL && B.FunctionName != NAME_None )
	{
		B.Object = this;
	}
	*(UBOOL*)Result = (A != B);
}
IMPLEMENT_FUNCTION( UObject, EX_NotEqual_DelFunc, execNotEqual_DelegateFunction );

///////////////
// Constants //
///////////////

void UObject::execIntConst( FFrame& Stack, RESULT_DECL )
{
	*(INT*)Result = Stack.ReadInt();
}
IMPLEMENT_FUNCTION( UObject, EX_IntConst, execIntConst );

void UObject::execFloatConst( FFrame& Stack, RESULT_DECL )
{
	*(FLOAT*)Result = Stack.ReadFloat();
}
IMPLEMENT_FUNCTION( UObject, EX_FloatConst, execFloatConst );

void UObject::execStringConst( FFrame& Stack, RESULT_DECL )
{
	*(FString*)Result = (ANSICHAR*)Stack.Code;
	while( *Stack.Code )
		Stack.Code++;
	Stack.Code++;
}
IMPLEMENT_FUNCTION( UObject, EX_StringConst, execStringConst );

void UObject::execUnicodeStringConst( FFrame& Stack, RESULT_DECL )
{
#if XBOX || PLATFORM_UNIX
 	*(FString*)Result = FString((TCHAR*)Stack.Code);
#else
 	*(FString*)Result = FString((UNICHAR*)Stack.Code);
#endif

	while( *(WORD*)Stack.Code )
		Stack.Code+=sizeof(WORD);
	Stack.Code+=sizeof(WORD);
}
IMPLEMENT_FUNCTION( UObject, EX_UnicodeStringConst, execUnicodeStringConst );

void UObject::execObjectConst( FFrame& Stack, RESULT_DECL )
{
	*(UObject**)Result = (UObject*)Stack.ReadObject();
}
IMPLEMENT_FUNCTION( UObject, EX_ObjectConst, execObjectConst );

void UObject::execInstanceDelegate( FFrame& Stack, RESULT_DECL )
{
	((FScriptDelegate*)Result)->FunctionName = Stack.ReadName();
	((FScriptDelegate*)Result)->Object = this;
}
IMPLEMENT_FUNCTION( UObject, EX_InstanceDelegate, execInstanceDelegate );

void UObject::execNameConst( FFrame& Stack, RESULT_DECL )
{
	*(FName*)Result = Stack.ReadName();
}
IMPLEMENT_FUNCTION( UObject, EX_NameConst, execNameConst );

void UObject::execByteConst( FFrame& Stack, RESULT_DECL )
{
	*(BYTE*)Result = *Stack.Code++;
}
IMPLEMENT_FUNCTION( UObject, EX_ByteConst, execByteConst );

void UObject::execIntZero( FFrame& Stack, RESULT_DECL )
{
	*(INT*)Result = 0;
}
IMPLEMENT_FUNCTION( UObject, EX_IntZero, execIntZero );

void UObject::execIntOne( FFrame& Stack, RESULT_DECL )
{
	*(INT*)Result = 1;
}
IMPLEMENT_FUNCTION( UObject, EX_IntOne, execIntOne );

void UObject::execTrue( FFrame& Stack, RESULT_DECL )
{
	*(INT*)Result = 1;
}
IMPLEMENT_FUNCTION( UObject, EX_True, execTrue );

void UObject::execFalse( FFrame& Stack, RESULT_DECL )
{
	*(DWORD*)Result = 0;
}
IMPLEMENT_FUNCTION( UObject, EX_False, execFalse );

void UObject::execNoObject( FFrame& Stack, RESULT_DECL )
{
	*(UObject**)Result = NULL;
}
IMPLEMENT_FUNCTION( UObject, EX_NoObject, execNoObject );

void UObject::execEmptyDelegate( FFrame& Stack, RESULT_DECL )
{
	((FScriptDelegate*)Result)->Object = NULL;
	((FScriptDelegate*)Result)->FunctionName = NAME_None;
}
IMPLEMENT_FUNCTION( UObject, EX_EmptyDelegate, execEmptyDelegate );

void UObject::execIntConstByte( FFrame& Stack, RESULT_DECL )
{
	*(INT*)Result = *Stack.Code++;
}
IMPLEMENT_FUNCTION( UObject, EX_IntConstByte, execIntConstByte );

/////////////////
// Conversions //
/////////////////

void UObject::execDynamicCast( FFrame& Stack, RESULT_DECL )
{
	// Get "to cast to" class for the dynamic actor class
	UClass* Class = (UClass *)Stack.ReadObject();

	// Compile object expression.
	UObject* Castee = NULL;
	Stack.Step( Stack.Object, &Castee );
	//*(UObject**)Result = (Castee && Castee->IsA(Class)) ? Castee : NULL;
	*(UObject**)Result = NULL; // default value


	// if we were passed in a null value
	if( Castee == NULL )
	{
		if( Class->HasAnyClassFlags(CLASS_Interface) )
		{
			((FScriptInterface*)Result)->SetObject(NULL);
		}
		else
		{
			*(UObject**)Result = NULL;
		}
		return;
	}

	// check to see if the Castee is an implemented interface by looking up the
	// class hierarchy and seeing if any class in said hierarchy implements the interface
	if( Class->HasAnyClassFlags(CLASS_Interface) )
	{
		if ( Castee->GetClass()->ImplementsInterface(Class) )
		{
			// interface property type - convert to FScriptInterface
			((FScriptInterface*)Result)->SetObject(Castee);
			((FScriptInterface*)Result)->SetInterface(Castee->GetInterfaceAddress(Class));
		}
	}
	// check to see if the Castee is a castable class
	else if( Castee->IsA(Class) )
	{
		*(UObject**)Result = Castee;
	}
}
IMPLEMENT_FUNCTION( UObject, EX_DynamicCast, execDynamicCast );

void UObject::execMetaCast( FFrame& Stack, RESULT_DECL )
{
	UClass* MetaClass = (UClass*)Stack.ReadObject();

	// Compile actor expression.
	UObject* Castee=NULL;
	Stack.Step( Stack.Object, &Castee );
	*(UObject**)Result = (Castee && Castee->IsA(UClass::StaticClass()) && ((UClass*)Castee)->IsChildOf(MetaClass)) ? Castee : NULL;
}
IMPLEMENT_FUNCTION( UObject, EX_MetaCast, execMetaCast );

void UObject::execPrimitiveCast( FFrame& Stack, RESULT_DECL )
{
	INT B = *(Stack.Code)++;
	(Stack.Object->*GCasts[B])( Stack, Result );
}
IMPLEMENT_FUNCTION( UObject, EX_PrimitiveCast, execPrimitiveCast );

void UObject::execInterfaceCast( FFrame& Stack, RESULT_DECL )
{
	(Stack.Object->*GCasts[CST_ObjectToInterface])(Stack, Result);
}
IMPLEMENT_FUNCTION( UObject, EX_InterfaceCast, execInterfaceCast );

void UObject::execByteToInt( FFrame& Stack, RESULT_DECL )
{
	BYTE B=0;
	Stack.Step( Stack.Object, &B );
	*(INT*)Result = B;
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_ByteToInt, execByteToInt );

void UObject::execByteToBool( FFrame& Stack, RESULT_DECL )
{
	BYTE B=0;
	Stack.Step( Stack.Object, &B );
	*(DWORD*)Result = B ? 1 : 0;
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_ByteToBool, execByteToBool );

void UObject::execByteToFloat( FFrame& Stack, RESULT_DECL )
{
	BYTE B=0;
	Stack.Step( Stack.Object, &B );
	*(FLOAT*)Result = B;
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_ByteToFloat, execByteToFloat );

void UObject::execByteToString( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE(B);
	// dump the enum string if possible
	UByteProperty *ByteProp = Cast<UByteProperty>(GProperty);
	if (ByteProp != NULL &&
		ByteProp->Enum != NULL &&
		B < ByteProp->Enum->NumEnums())
	{
		ByteProp->Enum->GetEnum(B).ToString( *(FString*)Result );
	}
	else
	{
		*(FString*)Result = FString::Printf(TEXT("%i"),B);
	}
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_ByteToString, execByteToString );

void UObject::execIntToByte( FFrame& Stack, RESULT_DECL )
{
	INT I=0;
	Stack.Step( Stack.Object, &I );
	*(BYTE*)Result = I;
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_IntToByte, execIntToByte );

void UObject::execIntToBool( FFrame& Stack, RESULT_DECL )
{
	INT I=0;
	Stack.Step( Stack.Object, &I );
	*(INT*)Result = I ? 1 : 0;
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_IntToBool, execIntToBool );

void UObject::execIntToFloat( FFrame& Stack, RESULT_DECL )
{
	INT I=0;
	Stack.Step( Stack.Object, &I );
	*(FLOAT*)Result = I;
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_IntToFloat, execIntToFloat );

void UObject::execIntToString( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(I);
	*(FString*)Result = FString::Printf(TEXT("%i"),I);
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_IntToString, execIntToString );

void UObject::execBoolToByte( FFrame& Stack, RESULT_DECL )
{
	UBOOL B=0;
	Stack.Step( Stack.Object, &B );
	*(BYTE*)Result = B & 1;
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_BoolToByte, execBoolToByte );

void UObject::execBoolToInt( FFrame& Stack, RESULT_DECL )
{
	UBOOL B=0;
	Stack.Step( Stack.Object, &B );
	*(INT*)Result = B & 1;
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_BoolToInt, execBoolToInt );

void UObject::execBoolToFloat( FFrame& Stack, RESULT_DECL )
{
	UBOOL B=0;
	Stack.Step( Stack.Object, &B );
	*(FLOAT*)Result = B & 1;
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_BoolToFloat, execBoolToFloat );

void UObject::execBoolToString( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(B);
	*(FString*)Result = B ? GTrue : GFalse;
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_BoolToString, execBoolToString );

void UObject::execFloatToByte( FFrame& Stack, RESULT_DECL )
{
	FLOAT F=0.f;
	Stack.Step( Stack.Object, &F );
	*(BYTE*)Result = (BYTE)appTrunc(F);
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_FloatToByte, execFloatToByte );

void UObject::execFloatToInt( FFrame& Stack, RESULT_DECL )
{
	FLOAT F=0.f;
	Stack.Step( Stack.Object, &F );
	*(INT*)Result = appTrunc(F);
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_FloatToInt, execFloatToInt );

void UObject::execFloatToBool( FFrame& Stack, RESULT_DECL )
{
	FLOAT F=0.f;
	Stack.Step( Stack.Object, &F );
	*(DWORD*)Result = F!=0.f ? 1 : 0;
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_FloatToBool, execFloatToBool );

void UObject::execFloatToString( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(F);
	*(FString*)Result = FString::Printf(TEXT("%.4f"),F);
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_FloatToString, execFloatToString );

void UObject::execObjectToBool( FFrame& Stack, RESULT_DECL )
{
	UObject* Obj=NULL;
	Stack.Step( Stack.Object, &Obj );
	*(DWORD*)Result = Obj!=NULL;
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_ObjectToBool, execObjectToBool );

void UObject::execObjectToInterface( FFrame& Stack, RESULT_DECL )
{
	FScriptInterface* InterfaceValue = (FScriptInterface*)Result;

	// read the interface class off the stack
	UClass* InterfaceClass = Cast<UClass>(Stack.ReadObject());
	checkSlow(InterfaceClass != NULL);

	// read the object off the stack
	UObject* ObjectValue = NULL;
	Stack.Step( Stack.Object, &ObjectValue );

	if ( ObjectValue != NULL )
	{
		InterfaceValue->SetObject(ObjectValue);
		InterfaceValue->SetInterface(ObjectValue->GetInterfaceAddress(InterfaceClass));
	}
	else
	{
		InterfaceValue->SetObject(NULL);
	}
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_ObjectToInterface, execObjectToInterface );

void UObject::execObjectToString( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UObject,Obj);
	*(FString*)Result = Obj->GetName();
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_ObjectToString, execObjectToString );

void UObject::execInterfaceToString( FFrame& Stack, RESULT_DECL )
{
	P_GET_INTERFACE(InterfaceValue);
	*(FString*)Result = InterfaceValue.GetInterface() != NULL ? InterfaceValue.GetObject()->GetName() : TEXT("None");
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_InterfaceToString, execInterfaceToString );

void UObject::execInterfaceToBool( FFrame& Stack, RESULT_DECL )
{
	P_GET_INTERFACE(InterfaceValue);
	*(UBOOL*)Result = InterfaceValue.GetInterface() != NULL;
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_InterfaceToBool, execInterfaceToBool );

void UObject::execInterfaceToObject( FFrame& Stack, RESULT_DECL )
{
	P_GET_INTERFACE(InterfaceValue);
	*(UObject**)Result=InterfaceValue.GetObject();
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_InterfaceToObject, execInterfaceToObject );

void UObject::execDelegateToString( FFrame& Stack, RESULT_DECL )
{
	P_GET_DELEGATE(Del);
	*(FString*)Result = Del.ToString(this);
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_DelegateToString, execDelegateToString );

void UObject::execNameToBool( FFrame& Stack, RESULT_DECL )
{
	FName N=NAME_None;
	Stack.Step( Stack.Object, &N );
	*(DWORD*)Result = N!=NAME_None ? 1 : 0;
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_NameToBool, execNameToBool );

void UObject::execNameToString( FFrame& Stack, RESULT_DECL )
{
	P_GET_NAME(N);
	N.ToString( *(FString*)Result );
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_NameToString, execNameToString );

void UObject::execStringToName( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(String);
	*(FName*)Result = FName(*String);
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_StringToName, execStringToName );

void UObject::execStringToByte( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(Str);
	*(BYTE*)Result = appAtoi( *Str );
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_StringToByte, execStringToByte );

void UObject::execStringToInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(Str);
	*(INT*)Result = appAtoi( *Str );
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_StringToInt, execStringToInt );

void UObject::execStringToBool( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(Str);
	if (appStricmp(*Str,TEXT("True") )==0 
	||	appStricmp(*Str,GTrue)==0
	||	appStricmp(*Str,TEXT("Yes"))==0
	||	appStricmp(*Str,GYes)==0)
	{
		*(INT*)Result = 1;
	}
	else if(appStricmp(*Str,TEXT("False"))==0
		||	appStricmp(*Str,GFalse)==0
		||	appStricmp(*Str,TEXT("No"))==0
		||	appStricmp(*Str,GNo)==0)
	{
		*(INT*)Result = 0;
	}
	else
	{
		*(INT*)Result = appAtoi(*Str) ? 1 : 0;
	}
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_StringToBool, execStringToBool );

void UObject::execStringToFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(Str);
	*(FLOAT*)Result = appAtof( *Str );
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_StringToFloat, execStringToFloat );

/////////////////////////////////////////
// Native bool operators and functions //
/////////////////////////////////////////

void UObject::execNot_PreBool( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(A);
	P_FINISH;

	*(DWORD*)Result = !A;
}
IMPLEMENT_FUNCTION( UObject, 129, execNot_PreBool );

void UObject::execEqualEqual_BoolBool( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(A);
	P_GET_UBOOL(B);
	P_FINISH;

	*(DWORD*)Result = ((!A) == (!B));
}
IMPLEMENT_FUNCTION( UObject, 242, execEqualEqual_BoolBool );

void UObject::execNotEqual_BoolBool( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(A);
	P_GET_UBOOL(B);
	P_FINISH;

	*(DWORD*)Result = ((!A) != (!B));
}
IMPLEMENT_FUNCTION( UObject, 243, execNotEqual_BoolBool );

void UObject::execAndAnd_BoolBool( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(A);
	P_GET_SKIP_OFFSET(W);

	if( A )
	{
		P_GET_UBOOL(B);
		*(DWORD*)Result = A && B;
		Stack.Code++; //DEBUGGER
	}
	else
	{
		*(DWORD*)Result = 0;
		Stack.Code += W;
	}
}
IMPLEMENT_FUNCTION( UObject, 130, execAndAnd_BoolBool );

void UObject::execXorXor_BoolBool( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(A);
	P_GET_UBOOL(B);
	P_FINISH;

	*(DWORD*)Result = !A ^ !B;
}
IMPLEMENT_FUNCTION( UObject, 131, execXorXor_BoolBool );

void UObject::execOrOr_BoolBool( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(A);
	P_GET_SKIP_OFFSET(W);
	if( !A )
	{
		P_GET_UBOOL(B);
		*(DWORD*)Result = A || B;
		Stack.Code++; //DEBUGGER
	}
	else
	{
		*(DWORD*)Result = 1;
		Stack.Code += W;
	}
}
IMPLEMENT_FUNCTION( UObject, 132, execOrOr_BoolBool );

/////////////////////////////////////////
// Native byte operators and functions //
/////////////////////////////////////////

void UObject::execMultiplyEqual_ByteByte( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE_REF(A);
	P_GET_BYTE(B);
	P_FINISH;

	*(BYTE*)Result = (A *= B);
}
IMPLEMENT_FUNCTION( UObject, 133, execMultiplyEqual_ByteByte );

void UObject::execMultiplyEqual_ByteFloat(FFrame& Stack, RESULT_DECL)
{
	P_GET_BYTE_REF(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(BYTE*)Result = A = BYTE(appTrunc(A * B));
}
IMPLEMENT_FUNCTION(UObject, 198, execMultiplyEqual_ByteFloat);

void UObject::execDivideEqual_ByteByte( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE_REF(A);
	P_GET_BYTE(B);
	P_FINISH;

	*(BYTE*)Result = B ? (A /= B) : 0;
}
IMPLEMENT_FUNCTION( UObject, 134, execDivideEqual_ByteByte );

void UObject::execAddEqual_ByteByte( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE_REF(A);
	P_GET_BYTE(B);
	P_FINISH;

	*(BYTE*)Result = (A += B);
}
IMPLEMENT_FUNCTION( UObject, 135, execAddEqual_ByteByte );

void UObject::execSubtractEqual_ByteByte( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE_REF(A);
	P_GET_BYTE(B);
	P_FINISH;

	*(BYTE*)Result = (A -= B);
}
IMPLEMENT_FUNCTION( UObject, 136, execSubtractEqual_ByteByte );

void UObject::execAddAdd_PreByte( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE_REF(A);
	P_FINISH;

	*(BYTE*)Result = ++(A);
}
IMPLEMENT_FUNCTION( UObject, 137, execAddAdd_PreByte );

void UObject::execSubtractSubtract_PreByte( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE_REF(A);
	P_FINISH;

	*(BYTE*)Result = --(A);
}
IMPLEMENT_FUNCTION( UObject, 138, execSubtractSubtract_PreByte );

void UObject::execAddAdd_Byte( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE_REF(A);
	P_FINISH;

	*(BYTE*)Result = (A)++;
}
IMPLEMENT_FUNCTION( UObject, 139, execAddAdd_Byte );

void UObject::execSubtractSubtract_Byte( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE_REF(A);
	P_FINISH;

	*(BYTE*)Result = (A)--;
}
IMPLEMENT_FUNCTION( UObject, 140, execSubtractSubtract_Byte );

/////////////////////////////////
// Int operators and functions //
/////////////////////////////////

void UObject::execComplement_PreInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_FINISH;

	*(INT*)Result = ~A;
}
IMPLEMENT_FUNCTION( UObject, 141, execComplement_PreInt );

void UObject::execGreaterGreaterGreater_IntInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = ((DWORD)A) >> B;
}
IMPLEMENT_FUNCTION( UObject, 196, execGreaterGreaterGreater_IntInt );

void UObject::execSubtract_PreInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_FINISH;

	*(INT*)Result = -A;
}
IMPLEMENT_FUNCTION( UObject, 143, execSubtract_PreInt );

void UObject::execMultiply_IntInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = A * B;
}
IMPLEMENT_FUNCTION( UObject, 144, execMultiply_IntInt );

void UObject::execDivide_IntInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	if (B == 0)
	{
		Stack.Logf(NAME_ScriptWarning,TEXT("Divide by zero"));
	}

	*(INT*)Result = B ? A / B : 0;
}
IMPLEMENT_FUNCTION( UObject, 145, execDivide_IntInt );

void UObject::execAdd_IntInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = A + B;
}
IMPLEMENT_FUNCTION( UObject, 146, execAdd_IntInt );

void UObject::execSubtract_IntInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = A - B;
}
IMPLEMENT_FUNCTION( UObject, 147, execSubtract_IntInt );

void UObject::execLessLess_IntInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = A << B;
}
IMPLEMENT_FUNCTION( UObject, 148, execLessLess_IntInt );

void UObject::execGreaterGreater_IntInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = A >> B;
}
IMPLEMENT_FUNCTION( UObject, 149, execGreaterGreater_IntInt );

void UObject::execLess_IntInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(DWORD*)Result = A < B;
}
IMPLEMENT_FUNCTION( UObject, 150, execLess_IntInt );

void UObject::execGreater_IntInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(DWORD*)Result = A > B;
}
IMPLEMENT_FUNCTION( UObject, 151, execGreater_IntInt );

void UObject::execLessEqual_IntInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(DWORD*)Result = A <= B;
}
IMPLEMENT_FUNCTION( UObject, 152, execLessEqual_IntInt );

void UObject::execGreaterEqual_IntInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(DWORD*)Result = A >= B;
}
IMPLEMENT_FUNCTION( UObject, 153, execGreaterEqual_IntInt );

void UObject::execEqualEqual_IntInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(DWORD*)Result = A == B;
}
IMPLEMENT_FUNCTION( UObject, 154, execEqualEqual_IntInt );

void UObject::execNotEqual_IntInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(DWORD*)Result = A != B;
}
IMPLEMENT_FUNCTION( UObject, 155, execNotEqual_IntInt );

void UObject::execAnd_IntInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = A & B;
}
IMPLEMENT_FUNCTION( UObject, 156, execAnd_IntInt );

void UObject::execXor_IntInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = A ^ B;
}
IMPLEMENT_FUNCTION( UObject, 157, execXor_IntInt );

void UObject::execOr_IntInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = A | B;
}
IMPLEMENT_FUNCTION( UObject, 158, execOr_IntInt );

void UObject::execMultiplyEqual_IntFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_REF(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(INT*)Result = A = appTrunc(A * B);
}
IMPLEMENT_FUNCTION( UObject, 159, execMultiplyEqual_IntFloat );

void UObject::execDivideEqual_IntFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_REF(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(INT*)Result = A = appTrunc(B ? A/B : 0.f);
}
IMPLEMENT_FUNCTION( UObject, 160, execDivideEqual_IntFloat );

void UObject::execAddEqual_IntInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_REF(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = (A += B);
}
IMPLEMENT_FUNCTION( UObject, 161, execAddEqual_IntInt );

void UObject::execSubtractEqual_IntInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_REF(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = (A -= B);
}
IMPLEMENT_FUNCTION( UObject, 162, execSubtractEqual_IntInt );

void UObject::execAddAdd_PreInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_REF(A);
	P_FINISH;

	*(INT*)Result = ++(A);
}
IMPLEMENT_FUNCTION( UObject, 163, execAddAdd_PreInt );

void UObject::execSubtractSubtract_PreInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_REF(A);
	P_FINISH;

	*(INT*)Result = --(A);
}
IMPLEMENT_FUNCTION( UObject, 164, execSubtractSubtract_PreInt );

void UObject::execAddAdd_Int( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_REF(A);
	P_FINISH;

	*(INT*)Result = (A)++;
}
IMPLEMENT_FUNCTION( UObject, 165, execAddAdd_Int );

void UObject::execSubtractSubtract_Int( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_REF(A);
	P_FINISH;

	*(INT*)Result = (A)--;
}
IMPLEMENT_FUNCTION( UObject, 166, execSubtractSubtract_Int );

void UObject::execRand( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_FINISH;

	// RAND_MAX+1 give interval [0..A) with even distribution.
	*(INT*)Result = RandHelper(A);
}
IMPLEMENT_FUNCTION( UObject, 167, execRand );

void UObject::execMin( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = Min(A,B);
}
IMPLEMENT_FUNCTION( UObject, 249, execMin );

void UObject::execMax( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = Max(A,B);
}
IMPLEMENT_FUNCTION( UObject, 250, execMax );

void UObject::execClamp( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(V);
	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = Clamp(V,A,B);
}
IMPLEMENT_FUNCTION( UObject, 251, execClamp );

void UObject::execToHex( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(A);
	P_FINISH;
	*(FString*)Result = FString::Printf(TEXT("%08X"), A);
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execToHex );

///////////////////////////////////
// Float operators and functions //
///////////////////////////////////

void UObject::execSubtract_PreFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = -A;
}	
IMPLEMENT_FUNCTION( UObject, 169, execSubtract_PreFloat );

void UObject::execMultiplyMultiply_FloatFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = appPow(A,B);
}	
IMPLEMENT_FUNCTION( UObject, 170, execMultiplyMultiply_FloatFloat );

void UObject::execMultiply_FloatFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = A * B;
}	
IMPLEMENT_FUNCTION( UObject, 171, execMultiply_FloatFloat );

void UObject::execDivide_FloatFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	if (B == 0.f)
	{
		Stack.Logf(NAME_ScriptWarning,TEXT("Divide by zero"));
	}

	*(FLOAT*)Result = A / B;
}	
IMPLEMENT_FUNCTION( UObject, 172, execDivide_FloatFloat );

void UObject::execPercent_FloatFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	if (B == 0.f)
	{
		Stack.Logf(NAME_ScriptWarning,TEXT("Modulo by zero"));
	}

	*(FLOAT*)Result = appFmod(A,B);
}	
IMPLEMENT_FUNCTION( UObject, 173, execPercent_FloatFloat );

void UObject::execAdd_FloatFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = A + B;
}	
IMPLEMENT_FUNCTION( UObject, 174, execAdd_FloatFloat );

void UObject::execSubtract_FloatFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = A - B;
}	
IMPLEMENT_FUNCTION( UObject, 175, execSubtract_FloatFloat );

void UObject::execLess_FloatFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(DWORD*)Result = A < B;
}	
IMPLEMENT_FUNCTION( UObject, 176, execLess_FloatFloat );

void UObject::execGreater_FloatFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(DWORD*)Result = A > B;
}	
IMPLEMENT_FUNCTION( UObject, 177, execGreater_FloatFloat );

void UObject::execLessEqual_FloatFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(DWORD*)Result = A <= B;
}	
IMPLEMENT_FUNCTION( UObject, 178, execLessEqual_FloatFloat );

void UObject::execGreaterEqual_FloatFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(DWORD*)Result = A >= B;
}	
IMPLEMENT_FUNCTION( UObject, 179, execGreaterEqual_FloatFloat );

void UObject::execEqualEqual_FloatFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(DWORD*)Result = A == B;
}	
IMPLEMENT_FUNCTION( UObject, 180, execEqualEqual_FloatFloat );

void UObject::execNotEqual_FloatFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(DWORD*)Result = A != B;
}	
IMPLEMENT_FUNCTION( UObject, 181, execNotEqual_FloatFloat );

void UObject::execComplementEqual_FloatFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(DWORD*)Result = Abs(A - B) < (1.e-4);
}	
IMPLEMENT_FUNCTION( UObject, 210, execComplementEqual_FloatFloat );

void UObject::execMultiplyEqual_FloatFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT_REF(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = (A *= B);
}	
IMPLEMENT_FUNCTION( UObject, 182, execMultiplyEqual_FloatFloat );

void UObject::execDivideEqual_FloatFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT_REF(A);
	P_GET_FLOAT(B);
	P_FINISH;

	if (B == 0.f)
	{
		Stack.Logf(NAME_ScriptWarning,TEXT("Divide by zero"));
	}

	*(FLOAT*)Result = (A /= B);
}	
IMPLEMENT_FUNCTION( UObject, 183, execDivideEqual_FloatFloat );

void UObject::execAddEqual_FloatFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT_REF(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = (A += B);
}	
IMPLEMENT_FUNCTION( UObject, 184, execAddEqual_FloatFloat );

void UObject::execSubtractEqual_FloatFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT_REF(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = (A -= B);
}	
IMPLEMENT_FUNCTION( UObject, 185, execSubtractEqual_FloatFloat );

void UObject::execAbs( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = Abs(A);
}	
IMPLEMENT_FUNCTION( UObject, 186, execAbs );

void UObject::execSin( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = appSin(A);
}	
IMPLEMENT_FUNCTION( UObject, 187, execSin );

void UObject::execAsin( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = appAsin(A);
}	
IMPLEMENT_FUNCTION( UObject, -1, execAsin );

void UObject::execCos( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = appCos(A);
}	
IMPLEMENT_FUNCTION( UObject, 188, execCos );

void UObject::execAcos( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = appAcos(A);
}	
IMPLEMENT_FUNCTION( UObject, -1, execAcos );

void UObject::execTan( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = appTan(A);
}	
IMPLEMENT_FUNCTION( UObject, 189, execTan );

void UObject::execAtan( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
    P_GET_FLOAT(B); //amb
	P_FINISH;

    *(FLOAT*)Result = appAtan2(A,B); //amb: changed to atan2
}	
IMPLEMENT_FUNCTION( UObject, 190, execAtan );

void UObject::execExp( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = appExp(A);
}	
IMPLEMENT_FUNCTION( UObject, 191, execExp );

void UObject::execLoge( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = appLoge(A);
}	
IMPLEMENT_FUNCTION( UObject, 192, execLoge );

void UObject::execSqrt( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_FINISH;

	FLOAT Sqrt = 0.f;
	if( A > 0.f )
	{
		// Can't use appSqrt(0) as it computes it as 1 / appInvSqrt(). Not a problem as Sqrt variable defaults to 0 == sqrt(0).
		Sqrt = appSqrt( A );
	}
	else if (A < 0.f)
	{
		Stack.Logf(NAME_ScriptWarning,TEXT("Attempt to take Sqrt() of negative number - returning 0."));
	}

	*(FLOAT*)Result = Sqrt;
}	
IMPLEMENT_FUNCTION( UObject, 193, execSqrt );

void UObject::execSquare( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = Square(A);
}	
IMPLEMENT_FUNCTION( UObject, 194, execSquare );

void UObject::execRound( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_FINISH;

	*(INT*)Result = appRound(A);
}	
IMPLEMENT_FUNCTION( UObject, 199, execRound );

void UObject::execFFloor( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_FINISH;

	*(INT*)Result = appFloor(A);
}	
IMPLEMENT_FUNCTION( UObject, -1, execFFloor );

void UObject::execFCeil( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_FINISH;

	*(INT*)Result = appCeil(A);
}	
IMPLEMENT_FUNCTION( UObject, -1, execFCeil );

void UObject::execFRand( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;

	*(FLOAT*)Result = appFrand();
}	
IMPLEMENT_FUNCTION( UObject, 195, execFRand );

void UObject::execFMin( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = Min(A,B);
}	
IMPLEMENT_FUNCTION( UObject, 244, execFMin );

void UObject::execFMax( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = Max(A,B);
}	
IMPLEMENT_FUNCTION( UObject, 245, execFMax );

void UObject::execFClamp( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(V);
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = Clamp(V,A,B);
}	
IMPLEMENT_FUNCTION( UObject, 246, execFClamp );

void UObject::execLerp( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_GET_FLOAT(V);
	P_FINISH;

	*(FLOAT*)Result = A + V*(B-A);
}	
IMPLEMENT_FUNCTION( UObject, 247, execLerp );

void UObject::execFCubicInterp( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(P0);
	P_GET_FLOAT(T0);
	P_GET_FLOAT(P1);
	P_GET_FLOAT(T1);
	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = CubicInterp(P0, T0, P1, T1, A);
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execFCubicInterp );

void UObject::execFInterpEaseInOut( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_GET_FLOAT(Alpha);
	P_GET_FLOAT(Exp);
	P_FINISH;

	*(FLOAT*)Result = FInterpEaseInOut(A, B, Alpha, Exp);
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execFInterpEaseInOut );

void UObject::execFInterpTo( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(Current);
	P_GET_FLOAT(Target);
	P_GET_FLOAT(DeltaTime);
	P_GET_FLOAT(InterpSpeed);
	P_FINISH;

	*(FLOAT*)Result = FInterpTo( Current, Target, DeltaTime, InterpSpeed);
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execFInterpTo );

void UObject::execRotationConst( FFrame& Stack, RESULT_DECL )
{
	((FRotator*)Result)->Pitch = Stack.ReadInt();
	((FRotator*)Result)->Yaw   = Stack.ReadInt();
	((FRotator*)Result)->Roll  = Stack.ReadInt();
}
IMPLEMENT_FUNCTION( UObject, EX_RotationConst, execRotationConst );

void UObject::execVectorConst( FFrame& Stack, RESULT_DECL )
{
	((FVector*)Result)->X = Stack.ReadFloat();
	((FVector*)Result)->Y = Stack.ReadFloat();
	((FVector*)Result)->Z = Stack.ReadFloat();
}
IMPLEMENT_FUNCTION( UObject, EX_VectorConst, execVectorConst );


void UObject::execPointDistToLine( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(Point);
	P_GET_VECTOR(Line);
	P_GET_VECTOR(Origin);
	P_GET_VECTOR_OPTX_REF(OutClosestPoint,FVector(0,0,0));
	P_FINISH;

	*(FLOAT*)Result = PointDistToLine(Point, Line, Origin, OutClosestPoint);
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execPointDistToLine);

void UObject::execPointDistToSegment( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(Point);
	P_GET_VECTOR(StartPoint);
	P_GET_VECTOR(EndPoint);
	P_GET_VECTOR_OPTX_REF(OutClosestPoint,FVector(0,0,0));
	P_FINISH;

	*(FLOAT*)Result = PointDistToSegment(Point, StartPoint, EndPoint, OutClosestPoint);
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execPointDistToSegment);

//////////////////////////////////////
// Vector2D operators and functions //
//////////////////////////////////////

void UObject::execGetDotDistance( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR2D_REF(OutDotDist);
	P_GET_VECTOR(Direction);
	P_GET_VECTOR(AxisX);
	P_GET_VECTOR(AxisY);
	P_GET_VECTOR(AxisZ);
	P_FINISH;

	*(UBOOL*)Result = GetDotDistance( OutDotDist, Direction, AxisX, AxisY, AxisZ );
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execGetDotDistance);

void UObject::execGetAngularDistance( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR2D_REF(OutAngularDist);
	P_GET_VECTOR(Direction);
	P_GET_VECTOR(AxisX);
	P_GET_VECTOR(AxisY);
	P_GET_VECTOR(AxisZ);
	P_FINISH;

	*(UBOOL*)Result = GetAngularDistance( OutAngularDist, Direction, AxisX, AxisY, AxisZ );
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execGetAngularDistance);

void UObject::execGetAngularFromDotDist( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR2D_REF(OutAngDist);
	P_GET_VECTOR2D(DotDist);
	P_FINISH;

	GetAngularFromDotDist( OutAngDist, DotDist );
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execGetAngularFromDotDist);



/////////////////
// Conversions //
/////////////////

void UObject::execStringToVector( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(Str);

	const TCHAR* Stream = *Str;
	FVector Value(0,0,0);
	Value.X = appAtof(Stream);
	Stream = appStrstr(Stream,TEXT(","));
	if( Stream )
	{
		Value.Y = appAtof(++Stream);
		Stream = appStrstr(Stream,TEXT(","));
		if( Stream )
			Value.Z = appAtof(++Stream);
	}
	*(FVector*)Result = Value;
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_StringToVector, execStringToVector );

void UObject::execStringToRotator( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(Str);

	const TCHAR* Stream = *Str;
	FRotator Rotation(0,0,0);
	Rotation.Pitch = appAtoi(Stream);
	Stream = appStrstr(Stream,TEXT(","));
	if( Stream )
	{
		Rotation.Yaw = appAtoi(++Stream);
		Stream = appStrstr(Stream,TEXT(","));
		if( Stream )
			Rotation.Roll = appAtoi(++Stream);
	}
	*(FRotator*)Result = Rotation;
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_StringToRotator, execStringToRotator );

void UObject::execVectorToBool( FFrame& Stack, RESULT_DECL )
{
	FVector V(0,0,0);
	Stack.Step( Stack.Object, &V );
	*(DWORD*)Result = V.IsZero() ? 0 : 1;
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_VectorToBool, execVectorToBool );

void UObject::execVectorToString( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(V);
	*(FString*)Result = FString::Printf( TEXT("%.2f,%.2f,%.2f"), V.X, V.Y, V.Z );
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_VectorToString, execVectorToString );

void UObject::execVectorToRotator( FFrame& Stack, RESULT_DECL )
{
	FVector V(0,0,0);
	Stack.Step( Stack.Object, &V );
	*(FRotator*)Result = V.Rotation();
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_VectorToRotator, execVectorToRotator );

void UObject::execRotatorToBool( FFrame& Stack, RESULT_DECL )
{
	FRotator R(0,0,0);
	Stack.Step( Stack.Object, &R );
	*(DWORD*)Result = R.IsZero() ? 0 : 1;
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_RotatorToBool, execRotatorToBool );

void UObject::execRotatorToVector( FFrame& Stack, RESULT_DECL )
{
	FRotator R(0,0,0);
	Stack.Step( Stack.Object, &R );
	*(FVector*)Result = R.Vector();
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_RotatorToVector, execRotatorToVector );

void UObject::execRotatorToString( FFrame& Stack, RESULT_DECL )
{
	P_GET_ROTATOR(R);
//	*(FString*)Result = FString::Printf( TEXT("%i,%i,%i"), R.Pitch&65535, R.Yaw&65535, R.Roll&65535 );
	*(FString*)Result = FString::Printf( TEXT("%i,%i,%i"), R.Pitch, R.Yaw, R.Roll );
}
IMPLEMENT_CAST_FUNCTION( UObject, CST_RotatorToString, execRotatorToString );

////////////////////////////////////
// Vector operators and functions //
////////////////////////////////////

void UObject::execSubtract_PreVector( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_FINISH;

	*(FVector*)Result = -A;
}	
IMPLEMENT_FUNCTION( UObject, 0x80 + 83, execSubtract_PreVector );

void UObject::execMultiply_VectorFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_GET_FLOAT (B);
	P_FINISH;

	*(FVector*)Result = A*B;
}	
IMPLEMENT_FUNCTION( UObject, 0x80 + 84, execMultiply_VectorFloat );

void UObject::execMultiply_FloatVector( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT (A);
	P_GET_VECTOR(B);
	P_FINISH;

	*(FVector*)Result = A*B;
}	
IMPLEMENT_FUNCTION( UObject, 0x80 + 85, execMultiply_FloatVector );

void UObject::execMultiply_VectorVector( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_GET_VECTOR(B);
	P_FINISH;

	*(FVector*)Result = A*B;
}	
IMPLEMENT_FUNCTION( UObject, 296, execMultiply_VectorVector );

void UObject::execDivide_VectorFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_GET_FLOAT (B);
	P_FINISH;

	if (B == 0.f)
	{
		Stack.Logf(NAME_ScriptWarning,TEXT("Divide by zero"));
	}

	*(FVector*)Result = A/B;
}	
IMPLEMENT_FUNCTION( UObject, 0x80 + 86, execDivide_VectorFloat );

void UObject::execAdd_VectorVector( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_GET_VECTOR(B);
	P_FINISH;

	*(FVector*)Result = A+B;
}	
IMPLEMENT_FUNCTION( UObject, 0x80 + 87, execAdd_VectorVector );

void UObject::execSubtract_VectorVector( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_GET_VECTOR(B);
	P_FINISH;

	*(FVector*)Result = A-B;
}	
IMPLEMENT_FUNCTION( UObject, 0x80 + 88, execSubtract_VectorVector );

void UObject::execLessLess_VectorRotator( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_GET_ROTATOR(B);
	P_FINISH;

	*(FVector*)Result = FRotationMatrix(B).Transpose().TransformNormal( A );
}	
IMPLEMENT_FUNCTION( UObject, 275, execLessLess_VectorRotator );

void UObject::execGreaterGreater_VectorRotator( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_GET_ROTATOR(B);
	P_FINISH;

	*(FVector*)Result = FRotationMatrix(B).TransformNormal( A );
}	
IMPLEMENT_FUNCTION( UObject, 276, execGreaterGreater_VectorRotator );

void UObject::execEqualEqual_VectorVector( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_GET_VECTOR(B);
	P_FINISH;

	*(DWORD*)Result = A.X==B.X && A.Y==B.Y && A.Z==B.Z;
}	
IMPLEMENT_FUNCTION( UObject, 0x80 + 89, execEqualEqual_VectorVector );

void UObject::execNotEqual_VectorVector( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_GET_VECTOR(B);
	P_FINISH;

	*(DWORD*)Result = A.X!=B.X || A.Y!=B.Y || A.Z!=B.Z;
}	
IMPLEMENT_FUNCTION( UObject, 0x80 + 90, execNotEqual_VectorVector );

void UObject::execDot_VectorVector( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_GET_VECTOR(B);
	P_FINISH;

	*(FLOAT*)Result = A|B;
}	
IMPLEMENT_FUNCTION( UObject, 0x80 + 91, execDot_VectorVector );

void UObject::execCross_VectorVector( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_GET_VECTOR(B);
	P_FINISH;

	*(FVector*)Result = A^B;
}	
IMPLEMENT_FUNCTION( UObject, 0x80 + 92, execCross_VectorVector );

void UObject::execMultiplyEqual_VectorFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR_REF(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FVector*)Result = (A *= B);
}	
IMPLEMENT_FUNCTION( UObject, 0x80 + 93, execMultiplyEqual_VectorFloat );

void UObject::execMultiplyEqual_VectorVector( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR_REF(A);
	P_GET_VECTOR(B);
	P_FINISH;

	*(FVector*)Result = (A *= B);
}	
IMPLEMENT_FUNCTION( UObject, 297, execMultiplyEqual_VectorVector );

void UObject::execDivideEqual_VectorFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR_REF(A);
	P_GET_FLOAT(B);
	P_FINISH;

	if (B == 0.f)
	{
		Stack.Logf(NAME_ScriptWarning,TEXT("Divide by zero"));
	}

	*(FVector*)Result = (A /= B);
}	
IMPLEMENT_FUNCTION( UObject, 0x80 + 94, execDivideEqual_VectorFloat );

void UObject::execAddEqual_VectorVector( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR_REF(A);
	P_GET_VECTOR(B);
	P_FINISH;

	*(FVector*)Result = (A += B);
}	
IMPLEMENT_FUNCTION( UObject, 0x80 + 95, execAddEqual_VectorVector );

void UObject::execSubtractEqual_VectorVector( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR_REF(A);
	P_GET_VECTOR(B);
	P_FINISH;

	*(FVector*)Result = (A -= B);
}	
IMPLEMENT_FUNCTION( UObject, 0x80 + 96, execSubtractEqual_VectorVector );

void UObject::execVSize( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_FINISH;

	*(FLOAT*)Result = A.Size();
}	
IMPLEMENT_FUNCTION( UObject, 0x80 + 97, execVSize );

void UObject::execVSize2D( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_FINISH;

	*(FLOAT*)Result = A.Size2D();
}
IMPLEMENT_FUNCTION( UObject, -1, execVSize2D );

void UObject::execVSizeSq( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_FINISH;

	*(FLOAT*)Result = A.SizeSquared();
}
IMPLEMENT_FUNCTION( UObject, -1, execVSizeSq );

void UObject::execVSizeSq2D( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_FINISH;

	*(FLOAT*)Result = A.SizeSquared2D();
}
IMPLEMENT_FUNCTION( UObject, -1, execVSizeSq2D );

void UObject::execNormal( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_FINISH;

	*(FVector*)Result = A.SafeNormal();
}
IMPLEMENT_FUNCTION( UObject, 0x80 + 98, execNormal );

void UObject::execVLerp( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_GET_VECTOR(B);
	P_GET_FLOAT(V);
	P_FINISH;

	*(FVector*)Result = A + V*(B-A);
}	
IMPLEMENT_FUNCTION( UObject, -1, execVLerp );

void UObject::execVSmerp( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_GET_VECTOR(B);
	P_GET_FLOAT(V);
	P_FINISH;

	*(FVector*)Result = A + V*V*(3.f - 2.f*V)*(B-A);
}
IMPLEMENT_FUNCTION( UObject, -1, execVSmerp );

void UObject::execVInterpTo( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(Current);
	P_GET_VECTOR(Target);
	P_GET_FLOAT(DeltaTime);
	P_GET_FLOAT(InterpSpeed);
	P_FINISH;

	*(FVector*)Result = VInterpTo( Current, Target, DeltaTime, InterpSpeed );
}
IMPLEMENT_FUNCTION( UObject, -1, execVInterpTo );

void UObject::execClampLength( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(V);
	P_GET_FLOAT(MaxLength);
	P_FINISH;

	*(FVector*)Result = ClampLength(V, MaxLength);
}
IMPLEMENT_FUNCTION( UObject, -1, execClampLength );

void UObject::execVRand( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	*((FVector*)Result) = VRand();
}
IMPLEMENT_FUNCTION( UObject, 0x80 + 124, execVRand );

void UObject::execRotRand( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL_OPTX(bRoll, 0);
	P_FINISH;

	FRotator RRot;
	RRot.Yaw = RandHelper(65536);
	RRot.Pitch = RandHelper(65536);
	if ( bRoll )
		RRot.Roll = RandHelper(65536);
	else
		RRot.Roll = 0;
	*((FRotator*)Result) = RRot;
}
IMPLEMENT_FUNCTION( UObject, 320, execRotRand );

void UObject::execMirrorVectorByNormal( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_GET_VECTOR(B);
	P_FINISH;

	B = B.SafeNormal();
	*(FVector*)Result = A - 2.f * B * (B | A);
}
IMPLEMENT_FUNCTION( UObject, 300, execMirrorVectorByNormal );

//scion ===========================================================
//	SCION Function: execProjectOnTo
//	Author: superville
//
//	Description: Projects on vector (X) onto another (Y) and returns
//		the projected vector without modifying either of the arguments
//
//	Last Modified: 03/06/04 (superville)
// ================================================================
void UObject::execProjectOnTo(FFrame &Stack, RESULT_DECL)
{
	P_GET_VECTOR( X );
	P_GET_VECTOR( Y );
	P_FINISH;

	*(FVector*)Result = X.ProjectOnTo( Y );
}
IMPLEMENT_FUNCTION(UObject,1500,execProjectOnTo);

//scion ===========================================================
//	SCION Function: execIsZero
//	Author: superville
//
//	Description: Returns true if all components of the vector are zero
//
//	Last Modified: 03/06/04 (superville)
// ================================================================
void UObject::execIsZero(FFrame &Stack, RESULT_DECL)
{
	P_GET_VECTOR(A);
	P_FINISH;

	*(UBOOL*)Result = (A.X == 0 && A.Y == 0 && A.Z == 0);
}
IMPLEMENT_FUNCTION(UObject,1501,execIsZero);

//////////////////////////////////////
// Rotation operators and functions //
//////////////////////////////////////

void UObject::execEqualEqual_RotatorRotator( FFrame& Stack, RESULT_DECL )
{
	P_GET_ROTATOR(A);
	P_GET_ROTATOR(B);
	P_FINISH;

	*(DWORD*)Result = A.Pitch==B.Pitch && A.Yaw==B.Yaw && A.Roll==B.Roll;
}	
IMPLEMENT_FUNCTION( UObject, 0x80 + 14, execEqualEqual_RotatorRotator );

void UObject::execNotEqual_RotatorRotator( FFrame& Stack, RESULT_DECL )
{
	P_GET_ROTATOR(A);
	P_GET_ROTATOR(B);
	P_FINISH;

	*(DWORD*)Result = A.Pitch!=B.Pitch || A.Yaw!=B.Yaw || A.Roll!=B.Roll;
}	
IMPLEMENT_FUNCTION( UObject, 0x80 + 75, execNotEqual_RotatorRotator );

void UObject::execMultiply_RotatorFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_ROTATOR(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FRotator*)Result = A * B;
}	
IMPLEMENT_FUNCTION( UObject, 287, execMultiply_RotatorFloat );

void UObject::execMultiply_FloatRotator( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(A);
	P_GET_ROTATOR(B);
	P_FINISH;

	*(FRotator*)Result = B * A;
}	
IMPLEMENT_FUNCTION( UObject, 288, execMultiply_FloatRotator );

void UObject::execDivide_RotatorFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_ROTATOR(A);
	P_GET_FLOAT(B);
	P_FINISH;

	if (B == 0.f)
	{
		Stack.Logf(NAME_ScriptWarning,TEXT("Divide by zero"));
	}

	*(FRotator*)Result = A * (1.f/B);
}	
IMPLEMENT_FUNCTION( UObject, 289, execDivide_RotatorFloat );

void UObject::execMultiplyEqual_RotatorFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_ROTATOR_REF(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FRotator*)Result = (A *= B);
}	
IMPLEMENT_FUNCTION( UObject, 290, execMultiplyEqual_RotatorFloat );

void UObject::execDivideEqual_RotatorFloat( FFrame& Stack, RESULT_DECL )
{
	P_GET_ROTATOR_REF(A);
	P_GET_FLOAT(B);
	P_FINISH;

	if (B == 0.f)
	{
		Stack.Logf(NAME_ScriptWarning,TEXT("Divide by zero"));
	}

	*(FRotator*)Result = (A *= (1.f/B));
}	
IMPLEMENT_FUNCTION( UObject, 291, execDivideEqual_RotatorFloat );

void UObject::execAdd_RotatorRotator( FFrame& Stack, RESULT_DECL )
{
	P_GET_ROTATOR(A);
	P_GET_ROTATOR(B);
	P_FINISH;

	*(FRotator*)Result = A + B;
}
IMPLEMENT_FUNCTION( UObject, 316, execAdd_RotatorRotator );

void UObject::execSubtract_RotatorRotator( FFrame& Stack, RESULT_DECL )
{
	P_GET_ROTATOR(A);
	P_GET_ROTATOR(B);
	P_FINISH;

	*(FRotator*)Result = A - B;
}
IMPLEMENT_FUNCTION( UObject, 317, execSubtract_RotatorRotator );

void UObject::execAddEqual_RotatorRotator( FFrame& Stack, RESULT_DECL )
{
	P_GET_ROTATOR_REF(A);
	P_GET_ROTATOR(B);
	P_FINISH;

	*(FRotator*)Result = (A += B);
}
IMPLEMENT_FUNCTION( UObject, 318, execAddEqual_RotatorRotator );

void UObject::execSubtractEqual_RotatorRotator( FFrame& Stack, RESULT_DECL )
{
	P_GET_ROTATOR_REF(A);
	P_GET_ROTATOR(B);
	P_FINISH;

	*(FRotator*)Result = (A -= B);
}
IMPLEMENT_FUNCTION( UObject, 319, execSubtractEqual_RotatorRotator );

void UObject::execGetAxes( FFrame& Stack, RESULT_DECL )
{
	P_GET_ROTATOR(A);
	P_GET_VECTOR_REF(X);
	P_GET_VECTOR_REF(Y);
	P_GET_VECTOR_REF(Z);
	P_FINISH;

	FRotationMatrix R(A);
	X = R.GetAxis(0);
	Y = R.GetAxis(1);
	Z = R.GetAxis(2);
}
IMPLEMENT_FUNCTION( UObject, 0x80 + 101, execGetAxes );

void UObject::execGetUnAxes( FFrame& Stack, RESULT_DECL )
{
	P_GET_ROTATOR(A);
	P_GET_VECTOR_REF(X);
	P_GET_VECTOR_REF(Y);
	P_GET_VECTOR_REF(Z);
	P_FINISH;

	FMatrix R = FRotationMatrix(A).Transpose();
	X = R.GetAxis(0);
	Y = R.GetAxis(1);
	Z = R.GetAxis(2);
}
IMPLEMENT_FUNCTION( UObject, 0x80 + 102, execGetUnAxes );

void UObject::execOrthoRotation( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(X);
	P_GET_VECTOR(Y);
	P_GET_VECTOR(Z);
	P_FINISH;

	FMatrix M = FMatrix::Identity;
	M.SetAxis( 0, X );
	M.SetAxis( 1, Y );
	M.SetAxis( 2, Z );

	*(FRotator*)Result = M.Rotator();
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execOrthoRotation );

void UObject::execNormalize( FFrame& Stack, RESULT_DECL )
{
	P_GET_ROTATOR(Rot);
	P_FINISH;

	*(FRotator*)Result = Rot.GetNormalized();
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execNormalize );

void UObject::execRLerp( FFrame& Stack, RESULT_DECL )
{
	P_GET_ROTATOR(A);
	P_GET_ROTATOR(B);
	P_GET_FLOAT(Alpha);
	P_GET_UBOOL_OPTX(bShortestPath,FALSE);
	P_FINISH;

	FRotator DeltaAngle = B - A;

	if( bShortestPath )
	{
		DeltaAngle = DeltaAngle.GetNormalized();
	}

	*(FRotator*)Result = A + Alpha*DeltaAngle;
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execRLerp );

void UObject::execRSmerp( FFrame& Stack, RESULT_DECL )
{
	P_GET_ROTATOR(A);
	P_GET_ROTATOR(B);
	P_GET_FLOAT(Alpha);
	P_GET_UBOOL_OPTX(bShortestPath,FALSE);
	P_FINISH;

	FRotator DeltaAngle = B - A;

	if( bShortestPath )
	{
		DeltaAngle = DeltaAngle.GetNormalized();
	}

	*(FRotator*)Result = A + (Alpha*Alpha*(3.f - 2.f*Alpha))*DeltaAngle;
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execRSmerp );


void UObject::execRInterpTo( FFrame& Stack, RESULT_DECL )
{
	P_GET_ROTATOR(Current);
	P_GET_ROTATOR(Target);
	P_GET_FLOAT(DeltaTime);
	P_GET_FLOAT(InterpSpeed);
	P_FINISH;

	*(FRotator*)Result = RInterpTo( Current, Target, DeltaTime, InterpSpeed );
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execRInterpTo );


void UObject::execNormalizeRotAxis( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(Angle);
	P_FINISH;

	*(INT*)Result = FRotator::NormalizeAxis(Angle);
}

IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execNormalizeRotAxis );

void UObject::execRDiff( FFrame& Stack, RESULT_DECL )
{
	P_GET_ROTATOR(A);
	P_GET_ROTATOR(B);
	P_FINISH;

	FVector ADeg = A.GetNormalized().Euler();
	FVector BDeg = B.GetNormalized().Euler();
	FVector Diff = (ADeg - BDeg);
	Diff.UnwindEuler();

	*(FLOAT*)Result = Diff.Size();
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execRDiff );

void UObject::execClockwiseFrom_IntInt( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(IntA);
	P_GET_INT(IntB);
	P_FINISH;

	IntA = IntA & 0xFFFF;
	IntB = IntB & 0xFFFF;

	*(DWORD*)Result = ( Abs(IntA - IntB) > 32768 ) ? ( IntA < IntB ) : ( IntA > IntB );
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execClockwiseFrom_IntInt );

////////////////////////////////////
// Matrix functions				  //
////////////////////////////////////

void UObject::execMultiply_MatrixMatrix( FFrame& Stack, RESULT_DECL )
{
	P_GET_MATRIX(A);
	P_GET_MATRIX(B);
	P_FINISH;

	*(FMatrix*)Result = A * B;
}

void UObject::execTransformVector( FFrame& Stack, RESULT_DECL )
{
	P_GET_MATRIX(TM);
	P_GET_VECTOR(A);
	P_FINISH;

	*(FVector*)Result = TM.TransformFVector(A);
}

void UObject::execInverseTransformVector( FFrame& Stack, RESULT_DECL )
{
	P_GET_MATRIX(TM);
	P_GET_VECTOR(A);
	P_FINISH;

	*(FVector*)Result = TM.InverseTransformFVector(A);
}

void UObject::execTransformNormal( FFrame& Stack, RESULT_DECL )
{
	P_GET_MATRIX(TM);
	P_GET_VECTOR(A);
	P_FINISH;

	*(FVector*)Result = TM.TransformNormal(A);
}

void UObject::execInverseTransformNormal( FFrame& Stack, RESULT_DECL )
{
	P_GET_MATRIX(TM);
	P_GET_VECTOR(A);
	P_FINISH;

	*(FVector*)Result = TM.InverseTransformNormal(A);
}

void UObject::execMakeRotationTranslationMatrix( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(Translation);
	P_GET_ROTATOR(Rotation);
	P_FINISH;

	*(FMatrix*)Result = FRotationTranslationMatrix(Rotation, Translation);
}

void UObject::execMatrixGetRotator( FFrame& Stack, RESULT_DECL )
{
	P_GET_MATRIX(TM);
	P_FINISH;

	*(FRotator*)Result = TM.Rotator();
}

void UObject::execMatrixGetOrigin( FFrame& Stack, RESULT_DECL )
{
	P_GET_MATRIX(TM);
	P_FINISH;

	*(FVector*)Result = TM.GetOrigin();
}

void UObject::execMatrixGetAxis( FFrame& Stack, RESULT_DECL )
{
	P_GET_MATRIX(TM);
	P_GET_BYTE(Axis);
	P_FINISH;

	if(Axis == AXIS_X)
	{
		*(FVector*)Result = TM.GetAxis(0);
	}
	else if(Axis == AXIS_Y)
	{
		*(FVector*)Result = TM.GetAxis(1);
	}
	else if(Axis == AXIS_Z)
	{
		*(FVector*)Result = TM.GetAxis(2);
	}
	else
	{
		*(FVector*)Result = FVector(0,0,0);
	}
}

////////////////////////////////////
// Quaternion functions           //
////////////////////////////////////

void UObject::execQuatProduct( FFrame& Stack, RESULT_DECL )
{
	P_GET_STRUCT(FQuat, A);
	P_GET_STRUCT(FQuat, B);
	P_FINISH;

	*(FQuat*)Result = A * B;
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execQuatProduct);

void UObject::execQuatDot( FFrame& Stack, RESULT_DECL )
{
	P_GET_STRUCT(FQuat, A);
	P_GET_STRUCT(FQuat, B);
	P_FINISH;

	*(FLOAT*)Result = A | B;
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execQuatDot);

void UObject::execQuatInvert( FFrame& Stack, RESULT_DECL )
{
	P_GET_STRUCT(FQuat, A);
	P_FINISH;

	FQuat invA(-A.X, -A.Y, -A.Z, A.W);
	*(FQuat*)Result = invA;
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execQuatInvert);

void UObject::execQuatRotateVector( FFrame& Stack, RESULT_DECL )
{
	P_GET_STRUCT(FQuat, A);
	P_GET_VECTOR(B);
	P_FINISH;

	*(FVector*)Result = A.RotateVector(B);
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execQuatRotateVector);

// Generate the 'smallest' (geodesic) rotation between these two vectors.
void UObject::execQuatFindBetween( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(A);
	P_GET_VECTOR(B);
	P_FINISH;

	FVector cross = A ^ B;
	FLOAT crossMag = cross.Size();

	// If these vectors are basically parallel - just return identity quaternion (ie no rotation).
	if(crossMag < KINDA_SMALL_NUMBER)
	{
		*(FQuat*)Result = FQuat::Identity;
		return;
	}

	FLOAT angle = appAsin(crossMag);

	FLOAT dot = A | B;
	if(dot < 0.f)
		angle = PI - angle;

	FLOAT sinHalfAng = appSin(0.5f * angle);
	FLOAT cosHalfAng = appCos(0.5f * angle);
	FVector axis = cross / crossMag;

	*(FQuat*)Result = FQuat(
		sinHalfAng * axis.X,
		sinHalfAng * axis.Y,
		sinHalfAng * axis.Z,
		cosHalfAng );
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execQuatFindBetween);

void UObject::execQuatFromAxisAndAngle( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(Axis);
	P_GET_FLOAT(Angle);
	P_FINISH;

	FLOAT sinHalfAng = appSin(0.5f * Angle);
	FLOAT cosHalfAng = appCos(0.5f * Angle);
	FVector normAxis = Axis.SafeNormal();

	*(FQuat*)Result = FQuat(
		sinHalfAng * normAxis.X,
		sinHalfAng * normAxis.Y,
		sinHalfAng * normAxis.Z,
		cosHalfAng );
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execQuatFromAxisAndAngle);

void UObject::execQuatFromRotator( FFrame& Stack, RESULT_DECL )
{
	P_GET_ROTATOR(A);
	P_FINISH;

	*(FQuat*)Result = FQuat( FRotationMatrix(A) );
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execQuatFromRotator);

void UObject::execQuatToRotator( FFrame& Stack, RESULT_DECL )
{
	P_GET_STRUCT(FQuat, A);
	P_FINISH;

	*(FRotator*)Result = FQuatRotationTranslationMatrix( A, FVector(0.f) ).Rotator();
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execQuatToRotator);

void UObject::execQuatSlerp( FFrame& Stack, RESULT_DECL )
{
	P_GET_STRUCT(FQuat, A);
	P_GET_STRUCT(FQuat, B);
	P_GET_FLOAT(Alpha);
	P_GET_UBOOL_OPTX(bShortestPath, true);
	P_FINISH;

	if(bShortestPath)
	{
		*(FQuat*)Result = SlerpQuat(A, B, Alpha);
	}
	else
	{
		*(FQuat*)Result = SlerpQuatFullPath(A, B, Alpha);
	}
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execQuatSlerp);

void UObject::execAdd_QuatQuat( FFrame& Stack, RESULT_DECL )
{
	P_GET_STRUCT(FQuat, A);
	P_GET_STRUCT(FQuat, B);
	P_FINISH;

	*(FQuat*)Result = A + B;
}
IMPLEMENT_FUNCTION( UObject, 270, execAdd_QuatQuat);

void UObject::execSubtract_QuatQuat( FFrame& Stack, RESULT_DECL )
{
	P_GET_STRUCT(FQuat, A);
	P_GET_STRUCT(FQuat, B);
	P_FINISH;

	*(FQuat*)Result = A - B;
}
IMPLEMENT_FUNCTION( UObject, 271, execSubtract_QuatQuat);

void UObject::execEatReturnValue(FFrame& Stack, RESULT_DECL)
{
	// get the return value property
	UProperty* Property = (UProperty*)Stack.ReadObject();
	// allocate a temporary buffer for the return value
	INT BufferSize = Property->ArrayDim * Property->ElementSize;
	BYTE* Buffer = (BYTE*)appAlloca(BufferSize);
	appMemzero(Buffer, BufferSize);
	// call the function, storing the unused return value in our temporary buffer
	Stack.Step(Stack.Object, Buffer);
	// destroy the return value
	Property->DestroyValue(Buffer);
}
IMPLEMENT_FUNCTION(UObject, EX_EatReturnValue, execEatReturnValue);

////////////////////////////////////
// Str operators and functions //
////////////////////////////////////

void UObject::execConcat_StrStr( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(A);
	P_GET_STR(B);
	P_FINISH;

	// faster, preallocating method
	FString& StringResult= *(FString*)Result;
	StringResult.Empty(A.Len()+B.Len()+1); // adding one for the string terminator
	StringResult+= A;
	StringResult+= B;

}
IMPLEMENT_FUNCTION( UObject, 112, execConcat_StrStr );

void UObject::execAt_StrStr( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(A);
	P_GET_STR(B);
	P_FINISH;

	// faster, preallocating method
	FString& StringResult= *(FString*)Result;
	StringResult.Empty(A.Len()+B.Len()+1+1); // adding 1 for the spacer, and 1 for the string terminator
	StringResult+= A;
	StringResult+= (TCHAR)' '; // adding spacer as a single TCHAR (it's faster)
	StringResult+= B;

}
IMPLEMENT_FUNCTION( UObject, 168, execAt_StrStr );

void UObject::execLess_StrStr( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(A);
	P_GET_STR(B);
	P_FINISH;

	*(DWORD*)Result = appStrcmp(*A,*B)<0;;
}
IMPLEMENT_FUNCTION( UObject, 115, execLess_StrStr );

void UObject::execGreater_StrStr( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(A);
	P_GET_STR(B);
	P_FINISH;

	*(DWORD*)Result = appStrcmp(*A,*B)>0;
}
IMPLEMENT_FUNCTION( UObject, 116, execGreater_StrStr );

void UObject::execLessEqual_StrStr( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(A);
	P_GET_STR(B);
	P_FINISH;

	*(DWORD*)Result = appStrcmp(*A,*B)<=0;
}
IMPLEMENT_FUNCTION( UObject, 120, execLessEqual_StrStr );

void UObject::execGreaterEqual_StrStr( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(A);
	P_GET_STR(B);
	P_FINISH;

	*(DWORD*)Result = appStrcmp(*A,*B)>=0;
}
IMPLEMENT_FUNCTION( UObject, 121, execGreaterEqual_StrStr );

void UObject::execEqualEqual_StrStr( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(A);
	P_GET_STR(B);
	P_FINISH;

	*(DWORD*)Result = appStrcmp(*A,*B)==0;
}
IMPLEMENT_FUNCTION( UObject, 122, execEqualEqual_StrStr );

void UObject::execNotEqual_StrStr( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(A);
	P_GET_STR(B);
	P_FINISH;

	*(DWORD*)Result = appStrcmp(*A,*B)!=0;
}
IMPLEMENT_FUNCTION( UObject, 123, execNotEqual_StrStr );

void UObject::execComplementEqual_StrStr( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(A);
	P_GET_STR(B);
	P_FINISH;

	*(DWORD*)Result = appStricmp(*A,*B)==0;
}
IMPLEMENT_FUNCTION( UObject, 124, execComplementEqual_StrStr );

void UObject::execConcatEqual_StrStr( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR_REF(A);
	P_GET_STR(B);
	P_FINISH;

	*(FString*)Result = ( A += B );
}
IMPLEMENT_FUNCTION( UObject, 322, execConcatEqual_StrStr );

void UObject::execAtEqual_StrStr( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR_REF(A);
	P_GET_STR(B);
	P_FINISH;

	A += TEXT(" ");
	*(FString*)Result = ( A += B );
}
IMPLEMENT_FUNCTION( UObject, 323, execAtEqual_StrStr );

void UObject::execSubtractEqual_StrStr( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR_REF(A);
	P_GET_STR(B);
	P_FINISH;

	FString& ResultString = *(FString*)Result = TEXT("");
	INT pos = A.InStr(B);
	while (pos != -1)
	{
		ResultString += A.Left(pos);
		A = A.Mid(pos + B.Len());
		pos = A.InStr(B);
	}

	ResultString += A;
	A = ResultString;
}
IMPLEMENT_FUNCTION( UObject, 324, execSubtractEqual_StrStr );

void UObject::execLen( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(S);
	P_FINISH;

	*(INT*)Result = S.Len();
}
IMPLEMENT_FUNCTION( UObject, 125, execLen );

void UObject::execInStr( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(S);
	P_GET_STR(A);
	P_GET_UBOOL_OPTX(bSearchFromRight, FALSE);
	P_FINISH;
	*(INT*)Result = S.InStr(A, bSearchFromRight);
}
IMPLEMENT_FUNCTION( UObject, 126, execInStr );

void UObject::execMid( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(A);
	P_GET_INT(I);
	P_GET_INT_OPTX(C,MAXINT);
	P_FINISH;

	*(FString*)Result = A.Mid(I,C);
}
IMPLEMENT_FUNCTION( UObject, 127, execMid );

void UObject::execLeft( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(A);
	P_GET_INT(N);
	P_FINISH;

	*(FString*)Result = A.Left(N);
}
IMPLEMENT_FUNCTION( UObject, 128, execLeft );

void UObject::execRight( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(A);
	P_GET_INT(N);
	P_FINISH;

	*(FString*)Result = A.Right(N);
}
IMPLEMENT_FUNCTION( UObject, 234, execRight );

void UObject::execCaps( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(A);
	P_FINISH;

	*(FString*)Result = A.ToUpper();
}
IMPLEMENT_FUNCTION( UObject, 235, execCaps );

void UObject::execLocs(FFrame& Stack, RESULT_DECL)
{
	P_GET_STR(A);
	P_FINISH;

	*(FString*)Result = A.ToLower();
}
IMPLEMENT_FUNCTION(UObject,238,execLocs);

void UObject::execChr( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(i);
	P_FINISH;

	TCHAR Temp[2];
	Temp[0] = i;
	Temp[1] = 0;
	*(FString*)Result = Temp;
}
IMPLEMENT_FUNCTION( UObject, 236, execChr );

void UObject::execAsc( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(S);
	P_FINISH;

	*(INT*)Result = **S;	
}
IMPLEMENT_FUNCTION( UObject, 237, execAsc );

void UObject::execRepl( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(Src);
	P_GET_STR(Match);
	P_GET_STR(With);
	P_GET_UBOOL_OPTX( bCaseSensitive, 0 );
	P_FINISH;

	INT i = bCaseSensitive ? Src.InStr( Match ) : Src.ToUpper().InStr( Match.ToUpper() );

	FString& ResultString = *(FString*)Result = TEXT("");
	while (i != -1)
	{
		ResultString += (Src.Left(i) + With);
		Src = Src.Mid( i + Match.Len() );
		i = bCaseSensitive ? Src.InStr(Match) : Src.ToUpper().InStr( Match.ToUpper() );
	}

	ResultString += Src;
}
IMPLEMENT_FUNCTION( UObject, 201, execRepl );

void UObject::execParseStringIntoArray(FFrame& Stack, RESULT_DECL)
{
	P_GET_STR(BaseString);
	P_GET_TARRAY_REF(FString, Pieces);
	P_GET_STR(Delim);
	P_GET_UBOOL(bCullEmpty);
	P_FINISH;

	BaseString.ParseIntoArray(&Pieces, *Delim, bCullEmpty);
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execParseStringIntoArray );

/////////////////////////////////////////
// Native name operators and functions //
/////////////////////////////////////////

void UObject::execEqualEqual_NameName( FFrame& Stack, RESULT_DECL )
{
	P_GET_NAME(A);
	P_GET_NAME(B);
	P_FINISH;

	*(DWORD*)Result = A == B;
}
IMPLEMENT_FUNCTION( UObject, 254, execEqualEqual_NameName );

void UObject::execNotEqual_NameName( FFrame& Stack, RESULT_DECL )
{
	P_GET_NAME(A);
	P_GET_NAME(B);
	P_FINISH;

	*(DWORD*)Result = A != B;
}
IMPLEMENT_FUNCTION( UObject, 255, execNotEqual_NameName );

////////////////////////////////////
// Object operators and functions //
////////////////////////////////////

void UObject::execEqualEqual_ObjectObject( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UObject,A);
	P_GET_OBJECT(UObject,B);
	P_FINISH;

	*(DWORD*)Result = A == B;
}
IMPLEMENT_FUNCTION( UObject, 114, execEqualEqual_ObjectObject );

void UObject::execNotEqual_ObjectObject( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UObject,A);
	P_GET_OBJECT(UObject,B);
	P_FINISH;

	*(DWORD*)Result = A != B;
}
IMPLEMENT_FUNCTION( UObject, 119, execNotEqual_ObjectObject );

void UObject::execEqualEqual_InterfaceInterface( FFrame& Stack, RESULT_DECL )
{
	P_GET_INTERFACE(A);
	P_GET_INTERFACE(B);
	P_FINISH;

	*(UBOOL*)Result = (A == B);
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execEqualEqual_InterfaceInterface );

void UObject::execNotEqual_InterfaceInterface( FFrame& Stack, RESULT_DECL )
{
	P_GET_INTERFACE(A);
	P_GET_INTERFACE(B);
	P_FINISH;

	*(UBOOL*)Result = A != B;
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execNotEqual_InterfaceInterface );


/////////////////////////////
// Log and error functions //
/////////////////////////////

void UObject::execLogInternal( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(S);
	P_GET_NAME_OPTX(N,NAME_ScriptLog);
	P_FINISH;

	debugf( (EName)N.GetIndex(), TEXT("%s"), *S );
}
IMPLEMENT_FUNCTION( UObject, 231, execLogInternal );

void UObject::execWarnInternal( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(S);
	P_FINISH;

	Stack.Logf( TEXT("%s"), *S );
}
IMPLEMENT_FUNCTION( UObject, 232, execWarnInternal );

void UObject::execLocalize( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(SectionName);
	P_GET_STR(KeyName);
	P_GET_STR(PackageName);
	P_FINISH;

	*(FString*)Result = Localize( *SectionName, *KeyName, *PackageName );
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execLocalize );

//////////////////
// High natives //
//////////////////

#define HIGH_NATIVE(n) \
void UObject::execHighNative##n( FFrame& Stack, RESULT_DECL ) \
{ \
	BYTE B = *Stack.Code++; \
	(this->*GNatives[ n*0x100 + B ])( Stack, Result ); \
} \
IMPLEMENT_FUNCTION( UObject, 0x60 + n, execHighNative##n );

HIGH_NATIVE(0);
HIGH_NATIVE(1);
HIGH_NATIVE(2);
HIGH_NATIVE(3);
HIGH_NATIVE(4);
HIGH_NATIVE(5);
HIGH_NATIVE(6);
HIGH_NATIVE(7);
HIGH_NATIVE(8);
HIGH_NATIVE(9);
HIGH_NATIVE(10);
HIGH_NATIVE(11);
HIGH_NATIVE(12);
HIGH_NATIVE(13);
HIGH_NATIVE(14);
HIGH_NATIVE(15);
#undef HIGH_NATIVE

/////////////////////////
// Object construction //
/////////////////////////

void UObject::execNew( FFrame& Stack, RESULT_DECL )
{
	// Get parameters.
	P_GET_OBJECT_OPTX(UObject,Outer,NULL);
	P_GET_STR_OPTX(Name,TEXT(""));
	P_GET_INT_OPTX(Flags,0);
	P_GET_OBJECT_OPTX(UClass,Cls,NULL);
	P_GET_OBJECT_OPTX(UObject,Template,NULL);

	if (Cls == NULL)
	{
		Stack.Logf(NAME_ScriptWarning, TEXT("No class passed to 'new' operator"));
		return;
	}

	// Validate parameters.
	if( Flags & ~RF_ScriptMask )
		Stack.Logf( TEXT("new: Flags %08X not allowed"), Flags & ~RF_ScriptMask );

	//@hack: it's not valid to spawn Actor classes through 'new', but there's no hook that would allow us to check this in Engine, so we do it here
	for (UClass* TempClass = Cls; TempClass; TempClass = TempClass->GetSuperClass())
	{
		if (TempClass->GetFName() == NAME_Actor)
		{
			Stack.Logf(NAME_ScriptWarning, TEXT("Attempt to create Actor subclass '%s' through 'new'; Use 'Spawn' instead"), *Cls->GetName());
			*(UObject**)Result = NULL;
			return;
		}
	}

	// Construct new object.
	if( !Outer )
		Outer = GetTransientPackage();
	*(UObject**)Result = StaticConstructObject( Cls, Outer, Name.Len()?FName(*Name):NAME_None, Flags&RF_ScriptMask, Template, &Stack, Template != NULL ? INVALID_OBJECT : NULL );
}
IMPLEMENT_FUNCTION( UObject, EX_New, execNew );

/////////////////////////////
// Class related functions //
/////////////////////////////

void UObject::execClassIsChildOf( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UClass,K);
	P_GET_OBJECT(UClass,C);
	P_FINISH;

	*(DWORD*)Result = (C && K) ? K->IsChildOf(C) : 0;
}
IMPLEMENT_FUNCTION( UObject, 258, execClassIsChildOf );

///////////////////////////////
// State and label functions //
///////////////////////////////

void UObject::execGotoState( FFrame& Stack, RESULT_DECL )
{
	FName CurrentStateName = (StateFrame && StateFrame->StateNode!=Class) ? StateFrame->StateNode->GetFName() : FName(NAME_None);
	P_GET_NAME_OPTX( S, CurrentStateName );
	P_GET_NAME_OPTX( L, NAME_None );
	P_GET_UBOOL_OPTX( bForceEvents, FALSE );
	P_GET_UBOOL_OPTX( bKeepStack, FALSE );
	P_FINISH;

	// Go to the state.
	EGotoState ResultState = GOTOSTATE_Success;
	if( S!=CurrentStateName || bForceEvents)
	{
		ResultState = GotoState( S, bForceEvents, bKeepStack );
	}

	// Handle success.
	if( ResultState==GOTOSTATE_Success )
	{
		// Now go to the label.
		if( !GotoLabel( L==NAME_None ? FName(NAME_Begin) : L ) && L!=NAME_None )
		{
			Stack.Logf( TEXT("GotoState (%s %s): Label not found"), *S.ToString(), *L.ToString() );
		}
	}
	else if( ResultState==GOTOSTATE_NotFound )
	{
		// Warning.
		if( S!=NAME_None && S!=NAME_Auto )
		{
			Stack.Logf( TEXT("GotoState (%s %s): State not found"), *S.ToString(), *L.ToString() );
		}
	}
	else
	{
		// Safely preempted by another GotoState.
	}
}
IMPLEMENT_FUNCTION( UObject, 113, execGotoState );

void UObject::PushState(FName NewState, FName NewLabel)
{
	// if we support states
	if (StateFrame != NULL)
	{
		// find the new state
		UState *StateNode = FindState(NewState);
		if (StateNode != NULL)
		{
			UBOOL bAlreadyInStack = FALSE;
			for (INT Idx = 0; Idx < StateFrame->StateStack.Num() && !bAlreadyInStack; Idx++)
			{
				if (StateFrame->StateStack(Idx).State == StateNode)
				{
					bAlreadyInStack = TRUE;
				}
			}
			bAlreadyInStack = bAlreadyInStack || (StateNode == StateFrame->StateNode);
			if (bAlreadyInStack)
			{
				debugf(NAME_ScriptWarning, TEXT("Attempt to push state %s that is already on the state stack"), *StateNode->GetName());
			}
			else
			{
				// notify the current state
				eventPausedState();

#if !FINAL_RELEASE
				if (GDebugger != NULL)
				{
					// manually pop the debugger stack node for this state...we'll restore it later
					GDebugger->DebugInfo(this, StateFrame, DI_PrevStackState, 0, 0);
				}
#endif

				// save the current state information
				INT Idx = StateFrame->StateStack.AddZeroed();
				StateFrame->StateStack(Idx).State = StateFrame->StateNode;
				StateFrame->StateStack(Idx).Node = StateFrame->Node;
				StateFrame->StateStack(Idx).Code = StateFrame->Code;

				// push the new state
				StateFrame->StateNode = StateNode;
				StateFrame->Node = StateNode;
				StateFrame->Code = NULL;
				StateFrame->ProbeMask = (StateNode->ProbeMask | GetClass()->ProbeMask) & StateNode->IgnoreMask;
				StateFrame->LatentAction = 0;
				StateFrame->bContinuedState = FALSE;
				// and send pushed event
				eventPushedState();
				// and jump to label if specified
				GotoLabel(NewLabel != NAME_None ? NewLabel : NAME_Begin);
			}
		}
		else
		{
			debugf(NAME_Warning,TEXT("Failed to find state %s"), *NewState.ToString());
		}
	}

}

void UObject::execPushState(FFrame &Stack, RESULT_DECL)
{
	P_GET_NAME(newState);
	P_GET_NAME_OPTX(newLabel, NAME_None);
	P_FINISH;

	PushState(newState, newLabel);
}

IMPLEMENT_FUNCTION(UObject, -1, execPushState);

void UObject::PopState(FFrame& Stack, UBOOL bPopAll)
{
	// if we support states, and we have a nested state
	if (StateFrame != NULL &&
		StateFrame->StateNode != NULL &&
		StateFrame->StateStack.Num())
	{
		// if popping all then find the lowest state, otherwise go one level up
		INT PopCnt = 0;
		while (StateFrame->StateStack.Num() &&
			   (bPopAll ||
				PopCnt == 0))
		{
			// send the popped event
			eventPoppedState();

			// verify that no state transitions happened during the event
			if (StateFrame->StateStack.Num() > 0)
			{
				if ( GDebugger )
				{
					/*
					Process any pending debug information in this state prior to transitioning to a new state
					todo - the debugger stacknode for this state node must be manually popped in order to maintain
					consistency in the udebugger's callstack.  Something still isn't right here - I suspect that the
					problem has to do with where I'm pushing a new stacknode for the state node
					that we're returning to...but it doesn't seem worth fixing, at this time.
				*/
					GDebugger->DebugInfo( this, StateFrame, DI_PrevStackState, 0, 0 );
				}

				// grab the new desired state
				INT Index = StateFrame->StateStack.Num() - 1;
				UState *StateNode = StateFrame->StateStack(Index).State;
				UStruct *Node = StateFrame->StateStack(Index).Node;
				BYTE *Code = StateFrame->StateStack(Index).Code;
				// remove from the current stack
				StateFrame->StateStack.Pop();
				// and set the new state
				StateFrame->StateNode = StateNode;
				StateFrame->Node = Node;
				StateFrame->Code = Code;
				StateFrame->ProbeMask = (StateNode->ProbeMask | GetClass()->ProbeMask) & StateNode->IgnoreMask;
				StateFrame->LatentAction = 0;
				StateFrame->bContinuedState = TRUE;
				PopCnt++;

				// send continue event
				eventContinuedState();
			}
		}
	}
	else
	{
		debugf(NAME_Warning, TEXT("%s called PopState() w/o a valid state stack!"), *GetName());
	}
}

void UObject::execPopState(FFrame &Stack, RESULT_DECL)
{
	P_GET_UBOOL_OPTX(bPopAll,0);
	P_FINISH;

	PopState(Stack, bPopAll);
}

IMPLEMENT_FUNCTION(UObject, -1, execPopState);

void UObject::execDumpStateStack(FFrame &Stack,RESULT_DECL)
{
	P_FINISH;
	if ( StateFrame != NULL )
	{
		debugf(TEXT("%s current state: %s"),*GetName(),*StateFrame->StateNode->GetName());
		if ( StateFrame->StateStack.Num() > 0 )
		{
			for (INT idx = 0; idx < StateFrame->StateStack.Num(); idx++)
			{
				debugf(TEXT("%d - %s"),idx,*StateFrame->StateStack(idx).State->GetName());
			}
		}
	}
}
IMPLEMENT_FUNCTION(UObject,-1,execDumpStateStack);

void UObject::execEnable( FFrame& Stack, RESULT_DECL )
{
	P_GET_NAME(N);
	if( N.GetIndex()>=NAME_PROBEMIN && N.GetIndex()<NAME_PROBEMAX && StateFrame )
	{
		QWORD BaseProbeMask = (GetStateFrame()->StateNode->ProbeMask | GetClass()->ProbeMask) & GetStateFrame()->StateNode->IgnoreMask;
		GetStateFrame()->ProbeMask |= (BaseProbeMask & ((QWORD)1<<(N.GetIndex()-NAME_PROBEMIN)));
	}
	else
	{
		Stack.Logf( TEXT("Enable: '%s' is not a probe function"), *N.ToString() );
	}
	P_FINISH;
}
IMPLEMENT_FUNCTION( UObject, 117, execEnable );

void UObject::execDisable( FFrame& Stack, RESULT_DECL )
{
	P_GET_NAME(N);
	P_FINISH;

	if( N.GetIndex()>=NAME_PROBEMIN && N.GetIndex()<NAME_PROBEMAX && StateFrame )
	{
		GetStateFrame()->ProbeMask &= ~((QWORD)1<<(N.GetIndex()-NAME_PROBEMIN));
	}
	else
	{
		Stack.Logf( TEXT("Disable: '%s' is not a probe function"), *N.ToString() );
	}
}
IMPLEMENT_FUNCTION( UObject, 118, execDisable );

void UObject::execSaveConfig( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	SaveConfig();
}
IMPLEMENT_FUNCTION( UObject, 536, execSaveConfig);

void UObject::execStaticSaveConfig( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;

	UObject* DefaultObject = Class->GetDefaultObject();
	check(DefaultObject);
	DefaultObject->SaveConfig();
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execStaticSaveConfig);

void UObject::execGetPerObjectConfigSections( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UClass,SearchClass);
	P_GET_TARRAY_REF(FString,out_SectionNames);
	P_GET_OBJECT_OPTX(UObject,ObjectOuter,GObjTransientPkg);
	P_GET_INT_OPTX(MaxResults,1024);
	P_FINISH;

	UBOOL& bResult = *(UBOOL*)Result;
	bResult = FALSE;

	if ( SearchClass != NULL )
	{
		if ( SearchClass->HasAnyClassFlags(CLASS_PerObjectConfig) )
		{
			FString Filename = ObjectOuter != GObjTransientPkg
				? (appGameConfigDir() + FString(GGameName) + *ObjectOuter->GetName() + TEXT(".ini"))
				: SearchClass->GetConfigName();

			bResult = GConfig->GetPerObjectConfigSections(*Filename, *SearchClass->GetName(), out_SectionNames, MaxResults);
		}
		else
		{
			Stack.Logf(NAME_Warning, TEXT("GetPerObjectConfigSections: class '%s' is not a PerObjectConfig class!"), *SearchClass->GetPathName());
		}
	}
	else
	{
		Stack.Logf(NAME_Warning, TEXT("GetPerObjectConfigSections called with NULL SearchClass!"));
	}
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execGetPerObjectConfigSections );

void UObject::execGetEnum( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UObject,E);
	P_GET_INT(i);
	P_FINISH;

	*(FName*)Result = NAME_None;
	if( Cast<UEnum>(E) && i>=0 && i<Cast<UEnum>(E)->NumEnums() )
		*(FName*)Result = Cast<UEnum>(E)->GetEnum(i);
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execGetEnum);

void UObject::execDynamicLoadObject( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(Name);
	P_GET_OBJECT(UClass,Class);
	P_GET_UBOOL_OPTX(bMayFail,0);
	P_FINISH;

#if defined(LOG_DYNAMIC_LOAD_OBJECT) || LOOKING_FOR_PERF_ISSUES
	debugf( NAME_PerfWarning, TEXT( "DynamicLoadObject is occurring for: %s"), *Name ); 
#endif

	*(UObject**)Result = StaticLoadObject( Class, NULL, *Name, NULL, LOAD_NoWarn | (bMayFail?LOAD_Quiet:0), NULL );
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execDynamicLoadObject );

void UObject::execFindObject( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(Name);
	P_GET_OBJECT(UClass,Class);
	P_FINISH;

	*(UObject**)Result = StaticFindObject( Class, NULL, *Name );
}
IMPLEMENT_FUNCTION( UObject, INDEX_NONE, execFindObject );

void UObject::execIsInState( FFrame& Stack, RESULT_DECL )
{
	P_GET_NAME(StateName);
	P_GET_UBOOL_OPTX(bTestStateStack,FALSE);
	P_FINISH;
	*(DWORD*)Result = IsInState(StateName,bTestStateStack);
}
IMPLEMENT_FUNCTION( UObject, 281, execIsInState );

void UObject::execIsChildState( FFrame& Stack, RESULT_DECL )
{
	P_GET_NAME(TestStateName);
	P_GET_NAME(TestParentStateName);
	P_FINISH;
	UState *TestParentState = FindState(TestParentStateName);
	if (TestParentState != NULL)
	{
		for ( UState *TestState = FindState(TestStateName); TestState != NULL; TestState = TestState->GetSuperState() )
		{
			if (TestState == TestParentState)
			{
				*(UBOOL*)Result = 1;
				return;
			}
		}
	}
	*(UBOOL*)Result = 0;
}
IMPLEMENT_FUNCTION( UObject, -1, execIsChildState );

void UObject::execGetStateName( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	*(FName*)Result = (StateFrame && StateFrame->StateNode) ? StateFrame->StateNode->GetFName() : FName(NAME_None);
}
IMPLEMENT_FUNCTION( UObject, 284, execGetStateName );

void UObject::execGetFuncName(FFrame &Stack, RESULT_DECL)
{
	P_FINISH;
	*(FName*)Result = Stack.Node != NULL ? Stack.Node->GetFName() : NAME_None;
}
IMPLEMENT_FUNCTION(UObject,-1,execGetFuncName);

void UObject::execScriptTrace(FFrame &Stack, RESULT_DECL)
{
	P_FINISH;
	debugf(TEXT("%s"), *Stack.GetStackTrace() );
}
IMPLEMENT_FUNCTION(UObject,-1,execScriptTrace);

void UObject::execDebugBreak(FFrame &Stack, RESULT_DECL)
{
	P_FINISH;
#if _DEBUG
	appDebugBreak();
#endif
}
IMPLEMENT_FUNCTION(UObject,-1,execDebugBreak);

void UObject::execSetUTracing( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(bShouldUTrace);
	P_FINISH;

#if ENABLE_SCRIPT_TRACING
	GIsUTracing = bShouldUTrace;
#else
	Stack.Logf(NAME_ScriptWarning, TEXT("UTracing is disabled in this build"));
#endif
}
IMPLEMENT_FUNCTION(UObject,INDEX_NONE,execSetUTracing);

void UObject::execIsUTracing( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
#if ENABLE_SCRIPT_TRACING
	*(UBOOL*)Result=GIsUTracing;
#else
	Stack.Logf(NAME_ScriptWarning, TEXT("UTracing is disabled in this build"));
	*(UBOOL*)Result=FALSE;
#endif
}

void UObject::execIsA( FFrame& Stack, RESULT_DECL )
{
	P_GET_NAME(ClassName);
	P_FINISH;

	UClass* TempClass;
	for( TempClass=GetClass(); TempClass; TempClass=TempClass->GetSuperClass() )
		if( TempClass->GetFName() == ClassName )
			break;
	*(DWORD*)Result = (TempClass!=NULL);
}
IMPLEMENT_FUNCTION(UObject,197,execIsA);

void UObject::execPathName( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UObject, CheckObject);
	P_FINISH;

	*(FString*)Result = CheckObject != NULL ? CheckObject->GetPathName() : TEXT("None");
}
IMPLEMENT_FUNCTION(UObject,-1,execPathName);

void UObject::execIsPendingKill( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	*(UBOOL*)Result = IsPendingKill();
}
IMPLEMENT_FUNCTION(UObject,INDEX_NONE,execIsPendingKill);

void UObject::execTransformVectorByRotation(FFrame& Stack,RESULT_DECL)
{
	P_GET_ROTATOR(SourceRotation);
	P_GET_VECTOR(SourceVector);
	P_GET_UBOOL_OPTX(bInverse,FALSE);
	P_FINISH;
	FRotationMatrix RotMatrix(SourceRotation);
	if (bInverse)
	{
		*(FVector*)Result = RotMatrix.InverseTransformFVector(SourceVector);
	}
	else
	{
		*(FVector*)Result = RotMatrix.TransformFVector(SourceVector);
	}
}
IMPLEMENT_FUNCTION(UObject,INDEX_NONE,execTransformVectorByRotation);

/*-----------------------------------------------------------------------------
	Native iterator functions.
-----------------------------------------------------------------------------*/

void UObject::execIterator( FFrame& Stack, RESULT_DECL )
{}
IMPLEMENT_FUNCTION( UObject, EX_Iterator, execIterator );

void UObject::execDynArrayIterator( FFrame& Stack, RESULT_DECL )
{
	// grab the array
	GPropObject = this;
	GProperty = NULL;
	Stack.Step( this, NULL );
	UArrayProperty* ArrayProperty = Cast<UArrayProperty>(GProperty);
	FScriptArray* Array=(FScriptArray*)GPropAddr;
	UProperty* InnerProperty = ArrayProperty->Inner;

	// grab the out item
	Stack.Step(this, NULL);
	UProperty* ItemProperty = GProperty;
	BYTE* ItemPropAddr = GPropAddr;

	// see if an index was specified
	BYTE IndexByte = *(BYTE*)Stack.Code;
	Stack.Code += sizeof(BYTE);

	// serialize the index expression
	GProperty = NULL;
	GPropAddr = NULL;
	Stack.Step(this, NULL);
	// and grab the property, will quite possibly be null
	UProperty* IndexProperty = GProperty;
	BYTE* IndexPropAddr = GPropAddr;

	// should the iterator skip null items?  currently only for object properties 
	const UBOOL bSkipNullItems = InnerProperty->IsA(UObjectProperty::StaticClass());

	INT Index = 0;
	PRE_ITERATOR;
		if (bSkipNullItems)		
		{
			// iterate till we find a valid value
			*(UObject**)ItemPropAddr = NULL;
			while (*(UObject**)ItemPropAddr == NULL && Index < Array->Num())
			{
				InnerProperty->CopySingleValue(ItemPropAddr,((BYTE*)Array->GetData() + (Index * InnerProperty->ElementSize)));
				// if there is an index property then assign the current index
				if (IndexProperty != NULL)
				{
					IndexProperty->CopySingleValue(IndexPropAddr,&Index);
				}
				Index++;
			}
			// if we don't have a valid value then exit the iterator
			if (*(UObject**)ItemPropAddr == NULL)
			{
				Stack.Code = &Stack.Node->Script(wEndOffset + 1);
				break;
			}
		}
		else
		{
			// otherwise iterate to the end of the array
			if (Index < Array->Num())
			{
				InnerProperty->CopySingleValue(ItemPropAddr,((BYTE*)Array->GetData() + (Index * InnerProperty->ElementSize)));
				// if there is an index property then assign the current index
				if (IndexProperty != NULL)
				{
					IndexProperty->CopySingleValue(IndexPropAddr,&Index);
				}
				Index++;
			}
			else
			{
				Stack.Code = &Stack.Node->Script(wEndOffset + 1);
				break;
			}
		}
	POST_ITERATOR;
}
IMPLEMENT_FUNCTION( UObject, EX_DynArrayIterator, execDynArrayIterator );

/*-----------------------------------------------------------------------------
	Native registry.
-----------------------------------------------------------------------------*/

//
// Register a native function.
// Warning: Called at startup time, before engine initialization.
//
BYTE GRegisterNative( INT iNative, const Native& Func )
{
	static int Initialized = 0;
	if( !Initialized )
	{
		Initialized = 1;
		for( DWORD i=0; i<ARRAY_COUNT(GNatives); i++ )
			GNatives[i] = &UObject::execUndefined;
	}
	if( iNative != INDEX_NONE )
	{
		if( iNative<0 || (DWORD)iNative>ARRAY_COUNT(GNatives) || GNatives[iNative]!=&UObject::execUndefined) 
			GNativeDuplicate = iNative;
		GNatives[iNative] = Func;
	}
	return 0;
}

BYTE GRegisterCast( INT CastCode, const Native& Func )
{
	static int Initialized = 0;
	if( !Initialized )
	{
		Initialized = 1;
		for( DWORD i=0; i<ARRAY_COUNT(GCasts); i++ )
			GCasts[i] = &UObject::execUndefined;
	}
	if( CastCode != INDEX_NONE )
	{
		if( CastCode<0 || (DWORD)CastCode>ARRAY_COUNT(GCasts) || GCasts[CastCode]!=&UObject::execUndefined) 
			GCastDuplicate = CastCode;
		GCasts[CastCode] = Func;
	}
	return 0;
}


/*-----------------------------------------------------------------------------
	Call graph profiling.
-----------------------------------------------------------------------------*/

/**
 * Constructor
 *
 * @param InSoftMemoryLimit		Max number of bytes used by call graph data (soft limit).
 * @param InTime				The number of seconds to profile before stopping.
 *								If 0, then do not auto-stop.
 */
FScriptCallGraph::FScriptCallGraph( DWORD InSoftMemoryLimit, INT InTime )
{
	TimeToProfile = InTime;
	check( InSoftMemoryLimit );
	Reset( InSoftMemoryLimit );
}

/**
 * Resets data collection and memory use.
 *
 * @param InSoftMemoryLimit		Max number of bytes used by call graph data (soft limit).
 */
void FScriptCallGraph::Reset( DWORD InSoftMemoryLimit )
{
	if( InSoftMemoryLimit )
	{
		SoftMemoryLimit = InSoftMemoryLimit;
	}

	Data.Empty( SoftMemoryLimit * 11 / 10 / sizeof(PTRINT) );

	CurrProfileTime = 0.0f;
}

/**
 * These defines allow us to output intel byte order on the xenon (which is big endian)
 **/
#if XBOX || PS3
#define SCRIPT_PROF_SERIALIZE32 INTEL_ORDER32
#define SCRIPT_PROF_SERIALIZE64 INTEL_ORDER64
#else
#define SCRIPT_PROF_SERIALIZE32
#define SCRIPT_PROF_SERIALIZE64
#endif

/**
 * Returns the index [into the Data array] for the next frame end marker after the starting point specified.
 *
 * @param	StartingPoint	optional starting index for searching for the next end frame marker; the first element to be checked
 *							will be the element at this index (so for recursive searching, pass-in 1 + the result of the last call to
 *							FindNextFrameMarker)
 *
 * @return	the index [into the Data array] for the next end frame marker, or INDEX_NONE if none were found.
 */
INT FScriptCallGraph::FindNextFrameMarkerPosition( INT StartingPoint/*=0*/ ) const
{
	INT Result = INDEX_NONE;

	for ( INT SearchIndex = Max(0, StartingPoint); SearchIndex < Data.Num(); SearchIndex++ )
	{
		if ( IsFrameEndMarker(Data(SearchIndex)) )
		{
			Result = SearchIndex;
			break;
		}
	}

	return Result;
}

/**
 * Returns the location of the profile capture end marker position, or INDEX_NONE if there isn't one
 *
 * @param	StartingPoint	optional starting index to begin searching for the end marker.
 */
INT FScriptCallGraph::FindProfileEndMarkerPosition( INT StartingPoint/*=0*/ ) const
{
	INT Result = INDEX_NONE;
	
	for ( INT SearchIndex = StartingPoint; SearchIndex < Data.Num(); SearchIndex++ )
	{
		if ( IsEndMarker(Data(SearchIndex)) )
		{
			Result = SearchIndex;
			break;
		}
	}

	return Result;
}

/**
 * Serializes the profiling data to the passed in archive.	
 *
 * @param Ar	Archive to serialize data to.
 */
void FScriptCallGraph::Serialize( FArchive& Ar )
{
	if( Data.Num() )
	{
		INT StartIndex	= 0;

		// Find the end of the call graph data.
		INT EndIndex = FindProfileEndMarkerPosition(StartIndex);
		checkf(EndIndex!=INDEX_NONE, TEXT("Attempting to save script profile results while profile is still running!"));

		INT EndFrameIndex = FindNextFrameMarkerPosition();
		if ( EndFrameIndex == INDEX_NONE )
		{
			// this indicates that we didn't capture at least one full frame - manually inject an "end frame" marker just before the "end profile" marker
			// so that the GUI app can read the data correctly
			// @note: if EmitFrameEnd() is changed, this needs to be updated as well
			DWORD Value = 3;
			Data.InsertItem( Value, EndIndex++ );
		}

		// Create a list of all functions currently loaded.
		TArray<UFunction*> Functions;
		for( TObjectIterator<UFunction> It; It; ++It )
		{
			Functions.AddItem( *It );
		}


		// First value to serialize is the number of usecs per cycle. (DOUBLE)
		DOUBLE usecsPerCycle = GSecondsPerCycle * 1000.0 * 1000.0;
		QWORD Swapped = SCRIPT_PROF_SERIALIZE64((QWORD&)usecsPerCycle);
		Ar << Swapped;

		// Second value is the number of functions that are being serialized (DWORD).
		DWORD FunctionCount = SCRIPT_PROF_SERIALIZE32((DWORD)Functions.Num());
		Ar << FunctionCount;

		//@todo 64bit: need to serialize sizeof(PTRINT) and change visualizer to deal with it.

		for( INT FunctionIndex=0; FunctionIndex<Functions.Num(); FunctionIndex++ )
		{
			FString		Name					= Functions(FunctionIndex)->GetPathName();
			DWORD		Length					= Name.Len();
			DWORD		LengthToSerialize		= SCRIPT_PROF_SERIALIZE32((DWORD)Name.Len());
#if PS3 && PLATFORM_64BITS
			DWORD		Pointer					= SCRIPT_PROF_SERIALIZE32((DWORD)(PTRINT)Functions(FunctionIndex));
#elif PLATFORM_64BITS
			PTRINT		Pointer					= SCRIPT_PROF_SERIALIZE64((PTRINT) Functions(FunctionIndex));
#else
			PTRINT		Pointer					= SCRIPT_PROF_SERIALIZE32((PTRINT) Functions(FunctionIndex));
#endif

			// Serialize function pointer, length of name in characters and then serialize name without the
			// trailing '\0' as a chain of ANSI characters.
			Ar << Pointer;	// (PTRINT)
			Ar << LengthToSerialize;	// (DWORD)

			for( DWORD i=0; i<Length; i++ )
			{
				ANSICHAR ACh = ToAnsi( Name[i] );
				Ar << ACh;	// (ANSICHAR)
			}
		}


		// Serialize raw collected data as tokens of PTRINTs.
#if PS3 && PLATFORM_64BITS
		for( INT DataIdx = StartIndex; DataIdx <= EndIndex; ++DataIdx )
		{
			DWORD PointerData = SCRIPT_PROF_SERIALIZE32((DWORD)(PTRINT)Data( DataIdx ));
			Ar << PointerData;
		}
#elif PLATFORM_64BITS		
		for( INT DataIdx = StartIndex; DataIdx <= EndIndex; ++DataIdx )
		{
			PTRINT PointerData = SCRIPT_PROF_SERIALIZE64((PTRINT)Data( DataIdx ));
			Ar << (QWORD&)PointerData;
		}
#else
		for( INT DataIdx = StartIndex; DataIdx <= EndIndex; ++DataIdx )
		{
			PTRINT PointerData = SCRIPT_PROF_SERIALIZE32((PTRINT)Data( DataIdx ));
			Ar << PointerData;
		}

		// orig
		//Ar.Serialize( &Data(StartIndex), (EndIndex - StartIndex) * sizeof(PTRINT)  );
#endif

	}
}


/**
 * Emits the end of frame marker. Needs to be called when there are no script functions
 * on the stack. Also responsible for reseting call graph data if memory limit is reached.
 */
void FScriptCallGraph::Tick(FLOAT DeltaTime)
{
	EmitFrameEnd();

	CurrProfileTime += DeltaTime;
	if (TimeToProfile > 0)
	{
		if (appTrunc(CurrProfileTime) >= TimeToProfile)
		{
			// Stop the profile...
			UObject::StaticExec(TEXT("PROFILESCRIPT STOP"));
		}
	}

	if( (Data.Num()*sizeof(PTRINT)) > SoftMemoryLimit )
	{
		//@todo ronp - rather than completely emptying the array, change this so that it only removes the earliest frame
		Reset( SoftMemoryLimit );
	}
}

/** Global script call graph profiler */
FScriptCallGraph* GScriptCallGraph = NULL;

/*-----------------------------------------------------------------------------
	Script processing function.
-----------------------------------------------------------------------------*/

/** advances Stack's code past the parameters to the given Function and if the function has a return value, copies the zero value for that property to the memory for the return value
 * @param Stack the script stack frame
 * @param Result pointer to where the return value should be written
 * @param Function the function being called
 */
void UObject::SkipFunction(FFrame& Stack, RESULT_DECL, UFunction* Function)
{
	// allocate temporary memory on the stack for evaluating parameters
	BYTE* Frame = (BYTE*)appAlloca(Function->PropertiesSize);
	appMemzero(Frame, Function->PropertiesSize);
	for (UProperty* Property = (UProperty*)Function->Children; *Stack.Code != EX_EndFunctionParms; Property = (UProperty*)Property->Next)
	{
		GPropAddr = NULL;
		GPropObject = NULL;
		// evaluate the expression into our temporary memory space
		// it'd be nice to be able to skip the copy, but most native functions assume a non-NULL Result pointer
		// so we can only do that if we know the expression is an l-value (out parameter)
		Stack.Step(Stack.Object, (Property->PropertyFlags & CPF_OutParm) ? NULL : Frame + Property->Offset);
	}
	// advance the code past EX_EndFunctionParms
	Stack.Code++;
	// check for debug info
	if (*Stack.Code == EX_DebugInfo)
	{
		Stack.Step(Stack.Object, NULL);
	}
	// destruct properties requiring it for which we had to use our temporary memory 
	// @warning: conditions for skipping DestroyValue() here must match conditions for passing NULL to Stack.Step() above
	for (UProperty* Destruct = Function->ConstructorLink; Destruct; Destruct = Destruct->ConstructorLinkNext)
	{
		if (!(Destruct->PropertyFlags & CPF_OutParm))
		{
			Destruct->DestroyValue(Frame + Destruct->Offset);
		}
	}

	UProperty* ReturnProp = Function->GetReturnProperty();
	if (ReturnProp != NULL)
	{
		// destroy old value if necessary
		if (ReturnProp->PropertyFlags & CPF_NeedCtorLink)
		{
			ReturnProp->DestroyValue(Result);
		}
		// copy zero value for return property into Result
		appMemzero(Result, ReturnProp->ArrayDim * ReturnProp->ElementSize);
	}
}

//
// Call a function.
//
void UObject::CallFunction( FFrame& Stack, RESULT_DECL, UFunction* Function )
{
#if STATS_SLOW
	STAT( const DWORD ScriptStatID = GStatManager.FindStatIDForScriptFunction( Function ) );
	SCOPE_CYCLE_COUNTER( ScriptStatID );
#endif

	STAT(FScopedScriptStats ScriptStats( Function ));

#if ENABLE_SCRIPT_TRACING
	if ( GIsUTracing && !(Function->FunctionFlags&FUNC_Operator) )
	{
		debugf( NAME_UTrace, TEXT("%sBEGIN %s %s"), appSpc(UTraceIndent), *GetName(), *Function->GetPathName() );
		UTraceIndent += 2;
	}
#endif

	// Found it.
	if( Function->iNative )
	{
#if defined(SHOW_SLOW_UNREALSCRIPT_FUNCTION_CALLS) || LOOKING_FOR_PERF_ISSUES
		DWORD Time=0;
		StartMSFunctionTimer(Time);
#endif
		// Call native final function.
		(this->*Function->Func)( Stack, Result );

#if defined(SHOW_SLOW_UNREALSCRIPT_FUNCTION_CALLS) || LOOKING_FOR_PERF_ISSUES
		StopMSFunctionTimer( Time, TEXT( "Indexed Native" ), *GetClass()->GetName(), *Function->GetName(), *GetName() );
#endif
	}
	else if( Function->FunctionFlags & FUNC_Native )
	{
		// Call native networkable function.
		BYTE Buffer[1024];
		if( !ProcessRemoteFunction( Function, Buffer, &Stack ) )
		{
#if defined(SHOW_SLOW_UNREALSCRIPT_FUNCTION_CALLS) || LOOKING_FOR_PERF_ISSUES
			DWORD Time=0;
			StartMSFunctionTimer(Time);
#endif
			// Call regular native function.
			(this->*Function->Func)( Stack, Result );

#if defined(SHOW_SLOW_UNREALSCRIPT_FUNCTION_CALLS) || LOOKING_FOR_PERF_ISSUES
			StopMSFunctionTimer( Time, TEXT( "Native" ), *GetClass()->GetName(), *Function->GetName(), *GetName() );
#endif
		}
		else
		{
			// Eat up the remaining parameters in the stream.
			SkipFunction(Stack, Result, Function);
#if ENABLE_SCRIPT_TRACING
			if ( GIsUTracing && !(Function->FunctionFlags&FUNC_Operator) )
			{
				debugf(NAME_UTrace, TEXT("%sNot executing, due to RPC"), appSpc(UTraceIndent));
			}
#endif
		}
	}
	else if (!(Function->FunctionFlags & FUNC_Defined))
	{
		// function has no body
		SkipFunction(Stack, Result, Function);
	}
	else
	{
		// Make new stack frame in the current context.
		BYTE* Frame = (BYTE*)appAlloca(Function->PropertiesSize);
		appMemzero( Frame, Function->PropertiesSize );
		FFrame NewStack( this, Function, 0, Frame, &Stack );
		FOutParmRec** LastOut = &NewStack.OutParms;
		UProperty* Property;

		for (Property = (UProperty*)Function->Children; *Stack.Code != EX_EndFunctionParms; Property = (UProperty*)Property->Next)
		{
			// reset the runtime flag
			GRuntimeUCFlags &= ~RUC_SkippedOptionalParm;

			BYTE* CurrentPropAddr = NULL;
			GPropAddr = NULL;
			GPropObject = NULL;
			checkfSlow(Property, TEXT("NULL Property in Function %s"), *Function->GetPathName()); 
			if (Property->PropertyFlags & CPF_OutParm)
			{
				// evaluate the expression for this parameter, which sets GPropAddr to the address of the property accessed
				Stack.Step(Stack.Object, NULL);
				FOutParmRec* Out = (FOutParmRec*)appAlloca(sizeof(FOutParmRec));
				// set the address and property in the out param info
				// warning: GPropAddr could be NULL for optional out parameters
				// if that's the case, we use the extra memory allocated for the out param in the function's locals
				// so there's always a valid address
				Out->PropAddr = GPropAddr ? GPropAddr : Frame + Property->Offset;
				Out->Property = Property;
				CurrentPropAddr = Out->PropAddr;
				// add the new out param info to the stack frame's linked list
				if (*LastOut)
				{
					(*LastOut)->NextOutParm = Out;
					LastOut = &(*LastOut)->NextOutParm;
				}
				else
				{
					*LastOut = Out;
				}
				// if the parameter is not const, we must mark the source property dirty for net replication
				if (GPropObject && GProperty && (GProperty->PropertyFlags & CPF_Net) && !(Property->PropertyFlags & CPF_Const))
				{
					GPropObject->NetDirty(GProperty);
				}
			}
			else
			{
				// copy the result of the expression for this parameter into the appropriate part of the local variable space
				BYTE* Param = NewStack.Locals + Property->Offset;
				checkSlow(Param);

				CurrentPropAddr = Param;
				Stack.Step(Stack.Object, Param);

#if !__INTEL_BYTE_ORDER__
				if (ExactCast<UBoolProperty>(Property) != NULL && *(DWORD*)(Param))
				{
					*(DWORD*)(Param) = FIRST_BITFIELD;
				}
#endif
			}

			// if this is an optional parameter, evaluate (or skip) the default value of the parameter
			if ( (Property->PropertyFlags&CPF_OptionalParm) != 0 )
			{
				// if this is a struct property and no value was passed into the function, initialize the property value with the defaults from the struct
				if ( (GRuntimeUCFlags&RUC_SkippedOptionalParm) != 0 )
				{
					UStructProperty* StructProp = Cast<UStructProperty>(Property, CLASS_IsAUStructProperty);
					if ( StructProp != NULL )
					{
						StructProp->InitializeValue( CurrentPropAddr );
					}
				}

				NewStack.Step(this, CurrentPropAddr);
#if !__INTEL_BYTE_ORDER__
				if (ExactCast<UBoolProperty>(Property) != NULL && *(DWORD*)(CurrentPropAddr))
				{
					*(DWORD*)(CurrentPropAddr) = FIRST_BITFIELD;
				}
#endif
			}
		}
		Stack.Code++;
		// see if there were any optional out parms at the end that weren't included in the function call
		for (; Property && ((Property->PropertyFlags & CPF_OptionalParm) != 0); Property = (UProperty*)Property->Next)
		{
			//@todo verify - don't think this block is ever actually executed anymore
			GPropAddr = NULL;
			GPropObject = NULL;

			if ( (Property->PropertyFlags & CPF_OutParm) != 0 )
			{
				FOutParmRec* Out = (FOutParmRec*)appAlloca(sizeof(FOutParmRec));
				// set the out param info, using the extra memory allocated for the out param in the function's locals
				// so that there is a valid address for the property
				Out->PropAddr = Frame + Property->Offset;
				Out->Property = Property;
				// add the new out param info to the stack frame's linked list
				if (*LastOut)
				{
					(*LastOut)->NextOutParm = Out;
					LastOut = &(*LastOut)->NextOutParm;
				}
				else
				{
					*LastOut = Out;
				}
			}

			// if this is an optional parameter, evaluate (or skip) the default value of the parameter
			NewStack.Step(this, Frame + Property->Offset);
		}
#ifdef _DEBUG
		// set the next pointer of the last item to NULL so we'll properly assert if something goes wrong
		if (*LastOut)
		{
			(*LastOut)->NextOutParm = NULL;
		}
#endif

		//DEBUGGER
		// This is necessary until I find a better solution, otherwise, 
		// when the DebugInfo gets read out of the bytecode, it'll be called 
		// AFTER the function returns, which is not useful. This is one of the
		// few places I check for debug information.
		if ( *Stack.Code == EX_DebugInfo )
			Stack.Step( Stack.Object, NULL );

		// Initialize any local struct properties with defaults
		for ( UProperty* LocalProp = Function->FirstStructWithDefaults; LocalProp != NULL; LocalProp = (UProperty*)LocalProp->Next )
		{
			UStructProperty* StructProp = Cast<UStructProperty>(LocalProp,CLASS_IsAUStructProperty);
			if ( StructProp != NULL )
			{
				StructProp->InitializeValue(NewStack.Locals + StructProp->Offset);
			}
		}

#if defined(SHOW_SLOW_UNREALSCRIPT_FUNCTION_CALLS) || LOOKING_FOR_PERF_ISSUES
		DWORD Time=0;
		StartMSFunctionTimer(Time);
#endif
		// Execute the code.
		ProcessInternal( NewStack, Result );

#if defined(SHOW_SLOW_UNREALSCRIPT_FUNCTION_CALLS) || LOOKING_FOR_PERF_ISSUES
		StopMSFunctionTimer( Time, TEXT( "Script" ), *GetClass()->GetName(), *Function->GetName(), *GetName() );
#endif

		// destruct properties on the stack, except for non-optional out params since we know we didn't use that memory
		for (UProperty* Destruct = Function->ConstructorLink; Destruct; Destruct = Destruct->ConstructorLinkNext)
		{
			if (!(Destruct->PropertyFlags & CPF_OutParm) || (Destruct->PropertyFlags & CPF_OptionalParm))
			{
				Destruct->DestroyValue(NewStack.Locals + Destruct->Offset);
			}
		}
	}

	// in case this function call was passed as the value for a parameter to another function, clear the flag that indicates whether a value was specified
	// for an optional parameter
	GRuntimeUCFlags &= ~RUC_SkippedOptionalParm;

#if ENABLE_SCRIPT_TRACING
	if ( GIsUTracing && !(Function->FunctionFlags&FUNC_Operator) )
	{
		UTraceIndent -= 2;
		if ( UTraceIndent < 0 )
		{
			UTraceIndent = 0;
		}
		debugf(NAME_UTrace, TEXT("%sEND   %s %s"), appSpc(UTraceIndent), *GetName(), *Function->GetPathName());
	}
#endif
}

//
// Internal function call processing.
// @warning: might not write anything to Result if singular or proper type isn't returned.
//
void UObject::ProcessInternal( FFrame& Stack, RESULT_DECL )
{
	DWORD IsSingularFunction = ((UFunction*)Stack.Node)->FunctionFlags & FUNC_Singular;
	if
	(	!ProcessRemoteFunction( (UFunction*)Stack.Node, Stack.Locals, NULL )
	&&	IsProbing( Stack.Node->GetFName() )
	&&	(!HasAnyFlags(RF_InSingularFunc) || !IsSingularFunction)
	)
	{
		if( IsSingularFunction )
		{
			SetFlags(RF_InSingularFunc);
		}
		BYTE Buffer[MAX_SIMPLE_RETURN_VALUE_SIZE];
#if DO_GUARD
		if( ++Recurse > RECURSE_LIMIT )
		{
			if ( GDebugger && GDebugger->NotifyInfiniteLoop() )
			{
				Recurse = 0;
			}
			else
			{
				debugf(*Stack.GetStackTrace());
				Stack.Logf( NAME_Critical, TEXT("Infinite script recursion (%i calls) detected"), RECURSE_LIMIT );
			}
		}
#endif
		while( *Stack.Code != EX_Return )
			Stack.Step( Stack.Object, Buffer );
		Stack.Code++;
		Stack.Step( Stack.Object, Result );

		//DEBUGGER: Necessary for the call stack. Grab an optional 'PREVSTACK' debug info.
		if ( *Stack.Code == EX_DebugInfo )
			Stack.Step( Stack.Object, Result );
		
		if( IsSingularFunction )
		{
			ClearFlags(RF_InSingularFunc);
		}
#if DO_GUARD
		--Recurse;
#endif
	}
	else
	{
		UProperty* ReturnProp = ((UFunction*)Stack.Node)->GetReturnProperty();
		if (ReturnProp != NULL)
		{
			// destroy old value if necessary
			if (ReturnProp->PropertyFlags & CPF_NeedCtorLink)
			{
				ReturnProp->DestroyValue(Result);
			}
			// copy zero value for return property into Result
			appMemzero(Result, ReturnProp->ArrayDim * ReturnProp->ElementSize);
		}

#if ENABLE_SCRIPT_TRACING
		if ( GIsUTracing && !(((UFunction*)Stack.Node)->FunctionFlags & FUNC_Operator) )
		{
			debugf(NAME_UTrace, TEXT("%sNot executing, due to RPC or non-probed function"), appSpc(UTraceIndent));
		}
#endif
	}
}

//
// Script processing functions.
//
void UObject::ProcessEvent( UFunction* Function, void* Parms, void* UnusedResult )
{
	static INT ScriptEntryTag = 0;

	checkf(!HasAnyFlags(RF_Unreachable),TEXT("%s  Function: '%s'"), *GetFullName(), *Function->GetPathName());

	// Reject.
	if
	(	(!(Function->FunctionFlags & (FUNC_Native | FUNC_Defined)))
	||	!IsProbing( Function->GetFName() )
	||	IsPendingKill()
	||	Function->iNative
	||	((Function->FunctionFlags & FUNC_Native) && ProcessRemoteFunction( Function, Parms, NULL )) )
		return;
	checkSlow(Function->ParmsSize==0 || Parms!=NULL);

#if ENABLE_SCRIPT_TRACING
	if ( GIsUTracing && !(Function->FunctionFlags&FUNC_Operator) )
	{
		debugf( NAME_UTrace, TEXT("%sBEGIN %s %s"), appSpc(UTraceIndent), *GetName(), *Function->GetPathName() );
		UTraceIndent += 2;
	}
#endif

	DECLARE_CYCLE_COUNTER(STAT_UnrealScriptTime);
	if(++ScriptEntryTag == 1)
	{
		START_CYCLE_COUNTER(STAT_UnrealScriptTime);
	}

	// Scope required for scoped script stats.
	{
#if STATS_SLOW
		STAT( const DWORD ScriptStatID = GStatManager.FindStatIDForScriptFunction( Function ) );
		SCOPE_CYCLE_COUNTER( ScriptStatID );
#endif

		STAT(FScopedScriptStats ScriptStats( Function ));
		
		// Create a new local execution stack.
		FFrame NewStack( this, Function, 0, appAlloca(Function->PropertiesSize) );
		checkSlow(NewStack.Locals || Function->ParmsSize == 0);

		// initialize the parameter properties
		appMemcpy( NewStack.Locals, Parms, Function->ParmsSize );

		// zero the local property memory
		appMemzero( NewStack.Locals+Function->ParmsSize, Function->PropertiesSize-Function->ParmsSize );

		// if the function has out parameters, fill the stack frame's out parameter info with the info for those params 
		if ( Function->HasAnyFunctionFlags(FUNC_HasOutParms|FUNC_HasOptionalParms) )
		{
			FOutParmRec** LastOut = &NewStack.OutParms;
			for ( UProperty* Property = (UProperty*)Function->Children; Property && (Property->PropertyFlags&(CPF_Parm|CPF_ReturnParm)) == CPF_Parm; Property = (UProperty*)Property->Next )
			{
				// this is used for optional parameters - the destination address for out parameter values is the address of the calling function
				// so we'll need to know which address to use if we need to evaluate the default parm value expression located in the new function's
				// bytecode
				BYTE* CurrentPropAddr = (BYTE*)Parms + Property->Offset;
				if ( Property->HasAnyPropertyFlags(CPF_OutParm) )
				{
					FOutParmRec* Out = (FOutParmRec*)appAlloca(sizeof(FOutParmRec));
					// set the address and property in the out param info
					// note that since C++ doesn't support "optional out" we can ignore that here
					Out->PropAddr = (BYTE*)Parms + Property->Offset;
					Out->Property = Property;
					CurrentPropAddr = Out->PropAddr;
					// add the new out param info to the stack frame's linked list
					if (*LastOut)
					{
						(*LastOut)->NextOutParm = Out;
						LastOut = &(*LastOut)->NextOutParm;
					}
					else
					{
						*LastOut = Out;
					}
				}

				if ( Property->HasAnyPropertyFlags(CPF_OptionalParm) )
				{
					GRuntimeUCFlags |= RUC_SkippedOptionalParm;

					// execute the code in the new function - it should be execDefaultParmValue, which will either 
					// advance the function's code pointer to the beginning of the real code [or the next default parm value]
					// (if the user specified a value) or evaluate the default value expression (if the user did not)
					NewStack.Step(this, CurrentPropAddr);
				}
			}

#ifdef _DEBUG
			// set the next pointer of the last item to NULL so we'll properly assert if something goes wrong
			if (*LastOut)
			{
				(*LastOut)->NextOutParm = NULL;
			}
#endif
		}

		for ( UProperty* LocalProp = Function->FirstStructWithDefaults; LocalProp != NULL; LocalProp = (UProperty*)LocalProp->Next )
		{
			UStructProperty* StructProp = Cast<UStructProperty>(LocalProp,CLASS_IsAUStructProperty);
			if ( StructProp != NULL )
			{
				StructProp->InitializeValue(NewStack.Locals + StructProp->Offset);
			}
		}

		// Call native function or UObject::ProcessInternal.
		(this->*Function->Func)(NewStack, (BYTE*)Parms + Function->ReturnValueOffset);

		// Destroy local variables except function parameters.!! see also UObject::ScriptConsoleExec
		// also copy back constructed value parms here so the correct copy is destroyed when the event function returns
		for (UProperty* P = Function->ConstructorLink; P; P = P->ConstructorLinkNext)
		{
			if (P->Offset >= Function->ParmsSize)
			{
				P->DestroyValue(NewStack.Locals + P->Offset);
			}
			else if (!(P->PropertyFlags & CPF_OutParm))
			{
				appMemcpy((BYTE*)Parms + P->Offset, NewStack.Locals + P->Offset, P->ArrayDim * P->ElementSize);
			}
		}
	}

	if(--ScriptEntryTag == 0)
	{
		STOP_CYCLE_COUNTER(STAT_UnrealScriptTime);
	}

	// in case this function call was passed as the value for a parameter to another function, clear the flag that indicates whether a value was specified
	// for an optional parameter
	GRuntimeUCFlags &= ~RUC_SkippedOptionalParm;

#if ENABLE_SCRIPT_TRACING
	if ( GIsUTracing && !(Function->FunctionFlags&FUNC_Operator) )
	{
		UTraceIndent -= 2;
		if ( UTraceIndent < 0 )
		{
			UTraceIndent = 0;
		}
		debugf(NAME_UTrace, TEXT("%sEND   %s %s"), appSpc(UTraceIndent), *GetName(), *Function->GetPathName());
	}
#endif
}

void UObject::ProcessDelegate( FName DelegateName, FScriptDelegate const* Delegate, void* Parms, void* UnusedResult )
{
	UObject* ContextObject = NULL;
	if ( Delegate->FunctionName != NAME_None )
	{
		ContextObject = Delegate->Object;
		if ( ContextObject == NULL )
		{
			ContextObject = this;
		}
	}

	if( ContextObject != NULL && !ContextObject->IsPendingKill() )
	{
		ContextObject->ProcessEvent( ContextObject->FindFunctionChecked(Delegate->FunctionName), Parms, UnusedResult );
	}
	// Don't process anything for an unassigned delegate property
	else if (DelegateName != NAME_None)
	{
		// attempt to execute the delegate body
		ProcessEvent( FindFunctionChecked(DelegateName), Parms, UnusedResult );
	}
}

//
// Execute the state code of the object.
//
void UObject::ProcessState( FLOAT DeltaSeconds )
{}

//
// Process a remote function; returns 1 if remote, 0 if local.
//
UBOOL UObject::ProcessRemoteFunction( UFunction* Function, void* Parms, FFrame* Stack )
{
	return 0;
}

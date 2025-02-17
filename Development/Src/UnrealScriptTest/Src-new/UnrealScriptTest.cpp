/*=============================================================================
	ScriptTest.cpp
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Includes.
#include "UnrealScriptTest.h"
#include "PackageHelperFunctions.h"

/*-----------------------------------------------------------------------------
	The following must be done once per package.
-----------------------------------------------------------------------------*/

#define STATIC_LINKING_MOJO 1

// Register things.
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name) FName UNREALSCRIPTTEST_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name) IMPLEMENT_FUNCTION(cls,idx,name)
#include "UnrealScriptTestClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NAMES_ONLY

// Register natives.
#define NATIVES_ONLY
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name)
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#include "UnrealScriptTestClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NATIVES_ONLY
#undef NAMES_ONLY

/**
 * Initialize registrants, basically calling StaticClass() to create the class and also 
 * populating the lookup table.
 *
 * @param	Lookup	current index into lookup table
 */
void AutoInitializeRegistrantsUnrealScriptTest( INT& Lookup )
{
	AUTO_INITIALIZE_REGISTRANTS_UNREALSCRIPTTEST
}

/**
 * Auto generates names.
 */
void AutoGenerateNamesUnrealScriptTest()
{
	#define NAMES_ONLY
	#define AUTOGENERATE_FUNCTION(cls,idx,name)
	#define AUTOGENERATE_NAME(name) UNREALSCRIPTTEST_##name = FName(TEXT(#name));
	#include "UnrealScriptTestClasses.h"
	#undef AUTOGENERATE_FUNCTION
	#undef AUTOGENERATE_NAME
	#undef NAMES_ONLY
}



IMPLEMENT_CLASS(UTest0010_NativeObject);
IMPLEMENT_CLASS(UTest0011_NativeObjectBoolOrder);

IMPLEMENT_CLASS(UTest0002_InterfaceNative);

void UTest0010_NativeObject::TestNativeFunction(UBOOL bParam)
{
	InterfaceMember = this;
	eventTestInterfaceEvent(this);

	eventTest03_CallEventWithNativeInterface(this);
}
void UTest0010_NativeObject::Test02_PassNativeInterfaceToNativeFunction( const TScriptInterface<ITest0002_InterfaceNative>& InterfaceParm )
{
	debugf(TEXT("UTest0010_NativeObject::Test02_PassNativeInterfaceToNativeFunction - InterfaceParmInterface '%s'"), InterfaceParm ? *InterfaceParm.GetObject()->GetFullName() : TEXT("NULL"));
}

void UTest0010_NativeObject::VerifyConversionFromInterfaceToObjectAsNativeParm( UObject* InObject, INT DummyInt )
{
	debugf(TEXT("Results for passing a native interface variable as the value for an object parameter in a native function:"));
	debugf(TEXT("\tObj: %s"), *InObject->GetFullName());
	debugf(TEXT("\tDummyInt: %i"), DummyInt);
}

void UTest0010_NativeObject::PerformNativeTest( INT TestNumber )
{
	switch( TestNumber )
	{
	case 1:
		{
			FConstructorStructString StringFoo;
			FConstructorStructArray ArrayFoo;
			FConstructorStructCombo ComboFoo;
			FNoCtorProps NoCtor;
			appMemzero(&NoCtor, sizeof(NoCtor));
			debugf(TEXT("\r\nCalling event with stack params"));
			eventTest01_CallEventWithStruct(NoCtor,StringFoo,ArrayFoo,ComboFoo,TRUE);

			debugf(TEXT("\r\n*****\r\nCalling event with default params"));
			eventTest01_CallEventWithStruct(NoCtor,DefaultStringStruct,DefaultArrayStruct,DefaultComboStruct,TRUE);
		}
		break;

	case 2:
		TestDoubleLinkedList();
		break;

	case 3:
		{
			UClass* InterfaceClass = ITest0002_InterfaceNative::UClassType::StaticClass();
			ITest0002_InterfaceNative* InterfaceThis = static_cast<ITest0002_InterfaceNative*>(GetInterfaceAddress(InterfaceClass));
			if ( InterfaceThis != NULL )
			{
				InterfaceThis->TestNativeFunction(TRUE);
			}
			break;
		}
	case 4:
		{
			TestNativeFunction(TRUE);

			TScriptInterface<ITest0002_InterfaceNative> InterfaceVar, InterfaceVar2, InterfaceVar3(this);
			InterfaceVar2 = this;
			InterfaceVar = this;

			InterfaceVar->TestNativeFunction(TRUE);


			InterfaceVar = InterfaceVar2;
			if ( InterfaceVar == this )
			{
				debugf(TEXT("hello"));
			}
		}
		break;

	case 5:
		{
			FMyStruct* TempStruct = new(MyArray) FMyStruct(EC_EventParm);
			TempStruct->MyFirstFloat = 1.f;
			TempStruct->MyFirstInt = 3;
			TempStruct->MyFirstString = TEXT("wooyay");
			TempStruct->MyInt = 4;
			TempStruct->MyFloat = 0.2f;
			TempStruct->MyStrings[0] = TEXT("test0");
			TempStruct->MyStrings[1] = TEXT("test1");
			TempStruct->MyStrings[2] = TEXT("test2");

			eventTest05_StructInheritance();
			break;
		}

	case 6:
		{
			debugf(TEXT("****  TESTING ASSIGNMENT FROM INTERFACE TO OBJECT VAR  ****"));
			eventTest06_InterfaceToObjectConversions();
		}
		break;

	case 7:
		{
			FUniqueNetId Source_Qword_Id;
			Source_Qword_Id.Uid = DECLARE_UINT64(0xFEFEFEFE4F4F4F4F);

			debugf(TEXT("Source_Qword_Id: %016I64X"), (QWORD&)Source_Qword_Id.Uid);

			FString Dest_Qword_Id = UOnlineSubsystem::UniqueNetIdToString(Source_Qword_Id);
			debugf(TEXT("Source_Qword_Id => string: %s"), *Dest_Qword_Id);

			FUniqueNetId Final_Qword_Id;
			UOnlineSubsystem::StringToUniqueNetId(Dest_Qword_Id, Final_Qword_Id);
			debugf(TEXT("RoundTrip for QWORD source: %016I64X"), (QWORD&)Final_Qword_Id.Uid);

			debugf(TEXT(""));

			FString Source_String_Id = TEXT("FEFEFEFE4F4F4F4F");
			FUniqueNetId Dest_String_Id;

			debugf(TEXT("Source_String_Id: %s"), *Source_String_Id);
			UOnlineSubsystem::StringToUniqueNetId(Source_String_Id, Dest_String_Id);
			debugf(TEXT("Dest_String_Id => %016I64X"), (QWORD&)Dest_String_Id.Uid);
			debugf(TEXT("RoundTrip for STRING source: %s"), *UOnlineSubsystem::UniqueNetIdToString(Dest_String_Id));
			break;
		}

	case 8:
		{
			debugf(TEXT("Calling event with value for first optional parm"));
			eventTestOptionalEventStringParm(TEXT("This is the first value"),TEXT("This is the second value"));
			debugf(TEXT("Calling event without value for first optional parm"));
			eventTestOptionalEventStringParm(TEXT(""),TEXT("This is the second value"));

			debugf(TEXT("Calling event with value for second optional parm"));
			eventTestOptionalEventStringParm(TEXT("This is the first value"), TEXT("This is the second value"));
			debugf(TEXT("Calling event without value for second optional parm"));
			eventTestOptionalEventStringParm(TEXT("This is the first value"));
			break;
		}

	case 9:
		{
			eventTestMultipleOptionalParms();
			break;
		}

	case 10:
		{
			FLOAT LerpedValue = 0.f;
			FLOAT Value = 0.f;
			const FLOAT Destination = 1.f;
			FLOAT RemainingTime = 1.f;
			const FLOAT InterpAlpha = 0.2;

			FLOAT TimeElapsed = 0.f;
			debugf(TEXT("Start + (Perc * (End - Start))"));
			INT Ticks = 0;
			do 
			{
				const FLOAT DeltaTime = Max(0.3f * appFrand(), 0.05f);
				const FLOAT Start = Value;
				const FLOAT End = Destination;
				const FLOAT Percentage = DeltaTime / Max(0.00001f, RemainingTime);
				const FLOAT Difference = End - Start;

				LerpedValue = Lerp(LerpedValue, End, Percentage);
				TimeElapsed += DeltaTime;
				debugf(TEXT("Start:%f   End:%f   Percentage:%f   Difference:%f    Duration:%f   DeltaTime:%f"), Start, End, Percentage, Difference, RemainingTime, DeltaTime);
				debugf(TEXT("%i:  %f + (%f * (%f - %f)) = %f     SHOULD BE: %f     (LerpedValue:%f)"),
					Ticks, Start, Percentage, End, Start, (Start + (Percentage * (End - Start))), TimeElapsed * Destination, LerpedValue);

				Value = (Start + (Percentage * (End - Start)));

				RemainingTime -= DeltaTime;
				Ticks++;

				debugf(TEXT(""));
			} while( Destination - Value > DELTA && RemainingTime > DELTA );
			break;
		}
	}
}

void UTest0010_NativeObject::TestDoubleLinkedList()
{
	TDoubleLinkedList<FString> StringList;

	StringList.AddHead(TEXT("Base"));

	INT i = 0;
	for ( i = 0; i < 10; i++ )
	{
		StringList.AddTail(appItoa(i));
	}

	for ( i = -1; i > -10; i-- )
	{
		StringList.AddHead(appItoa(i));
	}

	TDoubleLinkedList<FString>::TDoubleLinkedListNode* BaseNode = StringList.FindNode(TEXT("Base"));
	StringList.InsertNode(TEXT("InsertedNode"), BaseNode);

	StringList.RemoveNode(TEXT("5"));

	debugf(TEXT("List has %i elements"), StringList.Num());


	debugf(TEXT("From head to tail:"));
	i = 0;
	for ( TDoubleLinkedList<FString>::TIterator It(StringList.GetHead()); It; ++It )
	{
		FString NodeValue = *It;
		debugf( TEXT("%i) %s"), i++, *NodeValue );
	}

	debugf(TEXT("\r\nFrom tail to head:"));
	i = StringList.Num() - 1;
	for ( TDoubleLinkedList<FString>::TIterator It(StringList.GetTail()); It; --It )
	{
		FString NodeValue = *It;
		debugf( TEXT("%i) %s"), i--, *NodeValue );
	}

}


void UTest0011_NativeObjectBoolOrder::PerformBoolOrderTest()
{
	eventNativeTestBoolOrderEvent(TRUE);
}


UBOOL UTest0011_NativeObjectBoolOrder::NativeTestBoolOrderFunction(UBOOL BoolParm)
{
	debugf(TEXT("Input value for native function: %i"), BoolParm);

	return BoolParm;
}


//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UTest0025_NativeIssues);
void UTest0025_NativeIssues::ShowNativeBoolAssignmentTestResults()
{
	warnf(TEXT("SHOWING NATIVE BOOL ASSIGNMENT TEST RESULTS - EnumBeforeBoolTestVar:%i   bBoolAfterEnumTestVar:%i"), EnumBeforeBoolTestVar, bBoolAfterEnumTestVar);

	UBOOL Mismatch = FALSE;
	VERIFY_CLASS_OFFSET_NODIE(UTest0025_NativeIssues,Test0025_NativeIssues,EnumBeforeBoolTestVar)
	VERIFY_CLASS_SIZE_NODIE(UTest0025_NativeIssues)
	if( Mismatch == TRUE )
	{
		warnf( NAME_FriendlyError, *LocalizeUnrealEd("Error_ScriptClassSizeMismatch") );
	}
	else
	{
		debugf(TEXT("CheckNativeClassSizes completed with no errors"));
	}
}

/**
 * Dummy archive for use by UByteCodeSerializer
 */
class FArchiveFunctionMarker : public FArchive
{
public:
	FArchiveFunctionMarker( class UByteCodeSerializer* InByteCodeSerializer, INT& iCode )
	: FArchive(), CurrentCodePosition(iCode), ByteCodeSerializer(InByteCodeSerializer)
	{}

	/**
  	 * Returns the name of the Archive.  Useful for getting the name of the package a struct or object
	 * is in when a loading error occurs.
	 *
	 * This is overridden for the specific Archive Types
	 **/
	virtual FString GetArchiveName() const { return TEXT("FArchiveFunctionMarker"); }

	virtual FArchive& operator<<( class FName& N )
	{
		// only function names should be sent to this archive
		return *this;
	}
	virtual FArchive& operator<<( class UObject*& Res )
	{
		return *this;
	}
	virtual INT Tell()
	{
		return CurrentCodePosition;
	}
	virtual INT TotalSize()
	{
		return ByteCodeSerializer->Script.Num();
	}
	virtual void Seek( INT InPos )
	{
		CurrentCodePosition = InPos;
	}

private:

	INT& CurrentCodePosition;
	class UByteCodeSerializer* ByteCodeSerializer;
};

IMPLEMENT_CLASS(UByteCodeSerializer);

/**
 * Entry point for processing a single struct's bytecode array.  Calls SerializeExpr to process the struct's bytecode.
 *
 * @param	InParentStruct	the struct containing the bytecode to be processed.
 */
void UByteCodeSerializer::ProcessStructBytecode( UStruct* InParentStruct )
{
	/*
	Unhandled references:
	- Parameter types (delegate<MyTestActor.OnWriteProfileSettingsComplete)
	- default properties
	- references through named timers
	*/
	ParentStruct = InParentStruct;
	if ( ParentStruct != NULL && ParentStruct != this )
	{
		// make ParentStruct our Outer to make searching through scopes easier
		Rename(NULL, ParentStruct, REN_ForceNoResetLoaders);
		// copy the struct's script to a local copy so we don't have to keep referencing the ParentStruct
		Script = ParentStruct->Script;
	
// 
// 		if ( ParentStruct->GetName() == TEXT("TestIt")
// 		&&	ParentStruct->GetOuter()->GetName() == TEXT("MyTestActor2") )
// 		{
// 			int i = 0;
// 		}

		INT iCode = 0;
		FArchiveFunctionMarker FunctionMarkerAr(this, iCode);
		while ( iCode < Script.Num() )
		{
			CurrentContext = NULL;
			SerializeExpr( iCode, FunctionMarkerAr );
		}
	}

	CurrentContext = NULL;
}

/**
 * Processes compiled bytecode for UStructs, marking any UFunctions encountered as referenced.
 *
 * @param	iCode	the current position in the struct's bytecode array.
 * @param	Ar		the archive used for serializing the bytecode; not really necessary for this purpose.
 *
 * @return	the token that was parsed from the bytecode stream.
 */
EExprToken UByteCodeSerializer::SerializeExpr( INT& iCode, FArchive& Ar )
{
	FName NextFunctionName;
	UFunction* NextFunction=NULL;

	#define XFER(Type)				{ iCode += sizeof(Type); }
	#define XFER_FUNC_NAME			{ NextFunctionName = *(FName*)&Script(iCode); XFER(FName) }
	#define XFER_FUNC_POINTER		{ DWORD TempCode = *(DWORD*)&Script(iCode); XFER(DWORD) NextFunction	= (UFunction*)appDWORDToPointer(TempCode); }
	#define XFER_PROP_POINTER		{ DWORD TempCode = *(DWORD*)&Script(iCode); XFER(DWORD) GProperty		= (UProperty*)appDWORDToPointer(TempCode); SetCurrentContext(); }
	#define XFER_OBJECT_POINTER(T)	{ DWORD TempCode = *(DWORD*)&Script(iCode); XFER(DWORD) GPropObject		= (UObject*)appDWORDToPointer(TempCode); CurrentContext = Cast<UClass>(GPropObject); if ( CurrentContext == NULL ) { CurrentContext = GPropObject->GetClass(); } }

	#define	XFERPTR(T) XFER(T)
	#define XFERNAME() XFER(FName)

	// include this file to define any remaining symbols
	#include "ScriptSerialization.h"

	EExprToken ExpressionToken = EX_Max;

	// Get expression token.
	XFER(BYTE);
	ExpressionToken = (EExprToken)Script(iCode-1);
	if( ExpressionToken >= EX_FirstNative )
	{
		UFunction* NextFunction = GetIndexedNative(ExpressionToken);
		check(NextFunction);
		MarkFunctionAsReferenced(NextFunction);

		// Native final function with id 1-127.
		UClass* Context = CurrentContext;
		while( SerializeExpr( iCode, Ar ) != EX_EndFunctionParms )
		{
			// these native functions are the only ones where we want to restore the original context
			CurrentContext = Context;
		}
		HANDLE_OPTIONAL_DEBUG_INFO;
	}
	else if( ExpressionToken >= EX_ExtendedNative )
	{
		// Native final function with id 256-16383.
		BYTE B = Script(iCode);
		XFER(BYTE);

		// this is the formula for calculating the function's actual function index
		INT FuncIndex = (ExpressionToken - EX_ExtendedNative) * 0x100 + B;

		// find the function with this value for iNative
		UFunction* NextFunction = GetIndexedNative(FuncIndex);
		check(NextFunction);
		MarkFunctionAsReferenced(NextFunction);

		// only keep the current context if this function is an operator
		UClass* Context = NextFunction->HasAnyFunctionFlags(FUNC_PreOperator|FUNC_Operator) ? CurrentContext : NULL;

		// the current context doesn't matter to the parameters (since each one must be evaluated starting with a context of the current class), but
		// the code can attempt to call a function through the return value of a function (i.e. SomeFuncWithObjectReturnValue().SomeOtherFunction()),
		// so we need to set the current context to the function's return value so that it will be restored when we exit (due to TGuardValue)
		SetCurrentContext(NextFunction->GetReturnProperty());
		NextFunction = NULL;

		TGuardValue<UClass*> RestoreContext(CurrentContext, NULL);
		while( SerializeExpr( iCode, Ar ) != EX_EndFunctionParms )
		{
			CurrentContext = Context;
		}
		HANDLE_OPTIONAL_DEBUG_INFO;
	}
	else switch( ExpressionToken )
	{
		case EX_MetaCast:
		{
			XFER_OBJECT_POINTER(UClass*);
			TGuardValue<UClass*> RestoreContext(CurrentContext, NULL);
			SerializeExpr( iCode, Ar );
			break;
		}
		case EX_DynamicCast:
		{
			XFER_OBJECT_POINTER(UClass*);
			TGuardValue<UClass*> RestoreContext(CurrentContext, NULL);
			SerializeExpr( iCode, Ar );
			break;
		}
		case EX_FinalFunction:
		{
			XFER_FUNC_POINTER;											// Function pointer
			check(NextFunction);

			MarkFunctionAsReferenced(NextFunction);

			// the current context doesn't matter to the parameters (since each one must be evaluated starting with a context of the current class), but
			// the code can attempt to call a function through the return value of a function (i.e. SomeFuncWithObjectReturnValue().SomeOtherFunction()),
			// so we need to set the current context to the function's return value so that it will be restored when we exit (due to TGuardValue)
			SetCurrentContext(NextFunction->GetReturnProperty());
			NextFunction = NULL;

			// the current context must be restored each time we begin processing a new parameter so that object variable parameters aren't mistakenly used as the context for the
			// the next parameter's value (which might contain a function call)
			TGuardValue<UClass*> RestoreContext(CurrentContext, NULL);
			while( SerializeExpr( iCode, Ar ) != EX_EndFunctionParms )
			{
				CurrentContext = NULL;
			}
			HANDLE_OPTIONAL_DEBUG_INFO;

			break;
		}
		case EX_VirtualFunction:
		case EX_GlobalFunction:
		{
			XFER_FUNC_NAME;												// Virtual function name.
			if ( NextFunctionName != NAME_None )
			{
				UStruct* Context = CurrentContext;
				if ( Context == NULL )
				{
					Context = ParentStruct;
				}
				NextFunction = FindFieldChecked<UFunction>(Context, NextFunctionName);
				MarkFunctionAsReferenced(NextFunction);

				// the current context doesn't matter to the parameters (since each one must be evaluated starting with a context of the current class), but
				// the code can attempt to call a function through the return value of a function (i.e. SomeFuncWithObjectReturnValue().SomeOtherFunction()),
				// so we need to set the current context to the function's return value so that it will be restored when we exit (due to TGuardValue)
				SetCurrentContext(NextFunction->GetReturnProperty());
			}

			// the current context must be restored each time we begin processing a new parameter so that object variable parameters aren't mistakenly used as the context for the
			// the next parameter's value (which might contain a function call)
			TGuardValue<UClass*> RestoreContext(CurrentContext, NULL);
			while( SerializeExpr( iCode, Ar ) != EX_EndFunctionParms )
			{
				// each time SerializeExpr returns, it means we've finished parsing one complete parameter value
				CurrentContext = NULL;
			}
			HANDLE_OPTIONAL_DEBUG_INFO;

			break;
		}
		// script call to a delegate by invoking the delegate function directly
		case EX_DelegateFunction:
		{
			// indicates whether this is a local property
			XFER(BYTE);
			// Delegate property
			XFER_PROP_POINTER;

			// Name of the delegate function (in case no functions are assigned to the delegate)
			XFER_FUNC_NAME;
			if ( NextFunctionName != NAME_None )
			{
				UStruct* Context = CurrentContext;
				if ( Context == NULL )
				{
					Context = ParentStruct;
				}
				NextFunction = FindFieldChecked<UFunction>(Context, NextFunctionName);
				MarkFunctionAsReferenced(NextFunction);

				// the current context doesn't matter to the parameters (since each one must be evaluated starting with a context of the current class), but
				// the code can attempt to call a function through the return value of a function (i.e. SomeFuncWithObjectReturnValue().SomeOtherFunction()),
				// so we need to set the current context to the function's return value so that it will be restored when we exit (due to TGuardValue)
				SetCurrentContext(NextFunction->GetReturnProperty());
			}

			// the current context must be restored each time we begin processing a new parameter so that object variable parameters aren't mistakenly used as the context for the
			// the next parameter's value (which might contain a function call)
			TGuardValue<UClass*> RestoreContext(CurrentContext, NULL);
			while( SerializeExpr( iCode, Ar ) != EX_EndFunctionParms )
			{
				CurrentContext = NULL;
			}
			HANDLE_OPTIONAL_DEBUG_INFO;

			break;
		}
		case EX_DelegateProperty:
		case EX_InstanceDelegate:
		{
			// Name of function we're assigning to the delegate.
			XFER_FUNC_NAME;
			if ( NextFunctionName != NAME_None )
			{
				UStruct* Context = CurrentContext;
				if ( Context == NULL )
				{
					Context = ParentStruct;
				}

				NextFunction = FindFieldChecked<UFunction>(Context, NextFunctionName);
				MarkFunctionAsReferenced(NextFunction);
			}
			break;
		}

		case EX_Let:
		case EX_LetBool:
		case EX_LetDelegate:
		{
			SerializeExpr( iCode, Ar ); // l-value

			// be sure to clear out the current context before evaluating the next expression
			CurrentContext = NULL;
			SerializeExpr( iCode, Ar ); // r-value
			break;
		}
		case EX_StructMember:
		{
			XFER_PROP_POINTER;			// the struct property we're accessing
			XFERPTR(UStruct*);			// the struct which contains the property (allows derived structs to be used in expressions with base structs)
			XFER(BYTE);					// byte indicating whether a local copy of the struct must be created in order to access the member property
			XFER(BYTE);					// byte indicating whether the struct member will be modified by the expression it's being used in

			// need to restore context because this call to SerializeExpr will process the part of the expression that came BEFORE the struct member property
			// i.e. SomeStruct.SomeMember =>  XFER_PROP_POINTER processed "SomeMember", this next call to SerializeExpr will process "SomeStruct."
			TGuardValue<UClass*> RestoreContext(CurrentContext, CurrentContext);
			SerializeExpr( iCode, Ar );	// expression corresponding to the struct member property.
			break;
		}
		case EX_EqualEqual_DelDel:
		case EX_NotEqual_DelDel:
		case EX_EqualEqual_DelFunc:
		case EX_NotEqual_DelFunc:
		{
			TGuardValue<UClass*> RestoreContext(CurrentContext, NULL);
			while( SerializeExpr( iCode, Ar ) != EX_EndFunctionParms )
			{
				CurrentContext = NULL;
			}
			HANDLE_OPTIONAL_DEBUG_INFO;
			break;
		}
		default:
		{
			// back up the code pointer so that we can leave the standard code as is
			iCode -= sizeof(BYTE);

#define SERIALIZEEXPR_INC
			#include "ScriptSerialization.h"
#undef SERIALIZEEXPR_INC
			
			ExpressionToken = Expr;
			break;
		}
	}

	return ExpressionToken;

#undef XFER
#undef XFERPTR
#undef XFERNAME
#undef XFER_LABELTABLE
#undef HANDLE_OPTIONAL_DEBUG_INFO
#undef XFER_FUNC_POINTER
#undef XFER_FUNC_NAME
#undef XFER_PROP_POINTER
}

/**
 * Determines the correct class context based on the specified property.
 *
 * @param	ContextProperty		a property which potentially changes the current lookup context, such as object or interface property
 *
 * @return	if ContextProperty corresponds to an object, interface, or delegate property, result is a pointer to the class associated
 *			with the property (e.g. for UObjectProperty, would be PropertyClass); NULL if no context switch will happen as a result of
 *			processing ContextProperty
 */
UClass* UByteCodeSerializer::DetermineCurrentContext( UProperty* ContextProperty ) const
{
	UClass* NewContext = NULL;

	if ( ContextProperty != NULL )
	{
		UInterfaceProperty* InterfaceProp = SmartCastProperty<UInterfaceProperty>(ContextProperty);
		if ( InterfaceProp != NULL )
		{
			NewContext = InterfaceProp->InterfaceClass;
		}
		else
		{
			UObjectProperty* ObjectProp = SmartCastProperty<UObjectProperty>(ContextProperty);
			if ( ObjectProp != NULL )
			{
				// special case for Outer variable
				if ( ObjectProp->GetFName() == NAME_Outer )
				{
					UClass* Context = CurrentContext;
					if ( Context == NULL )
					{
						Context = ParentStruct->GetOwnerClass();
					}
					if ( Context != NULL && Context->ClassWithin != NULL && Context->ClassWithin != UObject::StaticClass() )
					{
						NewContext = Context->ClassWithin;
					}
					else
					{
						NewContext = NULL;
					}
				}
				else
				{
					UClassProperty* ClassProp = Cast<UClassProperty>(ObjectProp);
					NewContext = ClassProp != NULL ? ClassProp->MetaClass : ObjectProp->PropertyClass;
				}
			}
			else
			{
				UDelegateProperty* DelProp = SmartCastProperty<UDelegateProperty>(ContextProperty);
				if ( DelProp != NULL )
				{
					if ( DelProp->SourceDelegate == NULL )
					{
						// if this delegate property doesn't have a SourceDelegate set, then this property is the compiler-generated
						// property for an actual delegate function (i.e. __SomeDelegate__Delegate)
						check(DelProp->Function);
						NewContext = DelProp->Function->GetOwnerClass();
					}
					else
					{
						NewContext = DelProp->SourceDelegate->GetOwnerClass();
					}
				}
				else
				{
					NewContext = NULL;
				}
			}
		}
	}
	else
	{
		NewContext = NULL;
	}

	return NewContext;
}

/**
 * Sets the value of CurrentContext based on the results of calling DetermineCurrentContext()
 *
 * @param	ContextProperty		a property which potentially changes the current lookup context, such as object or interface property
 */
void UByteCodeSerializer::SetCurrentContext( UProperty* ContextProperty/*=GProperty*/ )
{
	CurrentContext = DetermineCurrentContext(ContextProperty);
}

/**
 * Marks the specified function as referenced by finding the UFunction corresponding to its original declaration.
 *
 * @param	ReferencedFunction	the function that should be marked referenced
 */
void UByteCodeSerializer::MarkFunctionAsReferenced( UFunction* ReferencedFunction )
{
	FindOriginalDeclaration(ReferencedFunction)->SetFlags(RF_Marked);
}

/**
 * Wrapper for checking whether the specified function is referenced.  Uses the UFunction corresponding to
 * the function's original declaration.
 */
UBOOL UByteCodeSerializer::IsFunctionReferenced( UFunction* FunctionToCheck )
{
	return FindOriginalDeclaration(FunctionToCheck)->HasAnyFlags(RF_Marked);
}

/**
 * Searches for the UFunction corresponding to the original declaration of the specified function.
 *
 * @param	FunctionToCheck		the function to lookup
 *
 * @return	a pointer to the original declaration of the specified function.
 */
UFunction* UByteCodeSerializer::FindOriginalDeclaration( UFunction* FunctionToCheck )
{
	UFunction* Result = NULL;

	if ( FunctionToCheck != NULL )
	{
		UFunction* SuperFunc = FunctionToCheck->GetSuperFunction();
		while ( SuperFunc != NULL )
		{
			Result = SuperFunc;
			SuperFunc = SuperFunc->GetSuperFunction();
		}

		if ( Result == NULL )
		{
			Result = FunctionToCheck;
		}

		UState* OuterScope = Result->GetOuterUState();
		if ( Cast<UClass>(OuterScope) == NULL )
		{
			// this function is declared within a state; see if we have a global version
			UClass* OwnerClass = Result->GetOwnerClass();
			UFunction* GlobalFunc = FindField<UFunction>(OwnerClass, Result->GetFName());
			if ( GlobalFunc != NULL )
			{
				check(GlobalFunc != Result);
				SuperFunc = GlobalFunc->GetSuperFunction();
				while ( SuperFunc != NULL )
				{
					Result = SuperFunc;
					SuperFunc = SuperFunc->GetSuperFunction();
				}
			}
		}
	}

	return Result;
}

/**
 * Find the function associated with the native index specified.
 *
 * @param	iNative		the native function index to search for
 *
 * @return	a pointer to the UFunction using the specified native function index.
 */
UFunction* UByteCodeSerializer::GetIndexedNative( INT iNative )
{
	UFunction* Result = NativeFunctionIndexMap.FindRef(iNative);
	if ( Result == NULL )
	{
		for ( TObjectIterator<UFunction> It; It; ++It )
		{
			if ( It->iNative == iNative )
			{
				Result = *It;
				NativeFunctionIndexMap.Set(iNative, Result);
				break;
			}
		}
	}

	return Result;
}

/**
 * Find the original function declaration from an interface class implemented by FunctionOwnerClass.
 *
 * @param	FunctionOwnerClass	the class containing the function being looked up.
 * @param	Function			the function being looked up
 *
 * @return	if Function is an implementation of a function declared in an interface class implemented by FunctionOwnerClass,
 *			returns a pointer to the function from the interface class; NULL if Function isn't an implementation of an interface
 *			function
 */
UFunction* UFindUnreferencedFunctionsCommandlet::GetInterfaceFunctionDeclaration( UClass* FunctionOwnerClass, UFunction* Function )
{
	check(FunctionOwnerClass);
	check(Function);

	UFunction* Result = NULL;

	for ( UClass* CurrentClass = FunctionOwnerClass; Result == NULL && CurrentClass; CurrentClass = CurrentClass->GetSuperClass() )
	{
		for ( TMap<UClass*,UProperty*>::TIterator It(CurrentClass->Interfaces); It; ++It )
		{
			UClass* ImplementedInterfaceClass = It.Key();
			Result = FindField<UFunction>(ImplementedInterfaceClass, Function->GetFName());
			if ( Result != NULL )
			{
				break;
			}
		}
	}

	return Result;
}

/**
 * This class is used to find which objects reference any element from a list of "TargetObjects".  When invoked,
 * it will generate a mapping of each target object to an array of objects referencing that target object.
 *
 * Each key corresponds to an element of the input TargetObjects array which was referenced
 * by some other object.  The values for these keys are the objects which are referencing them.
 */
class FArchiveFunctionReferenceCollector : public FArchive
{
public:

	/**
	 * Default constructor
	 *
	 * @param	TargetObjects	the list of objects to find references to
	 * @param	PackageToCheck	if specified, only objects contained in this package will be searched
	 *							for references to 
	 */
	FArchiveFunctionReferenceCollector()
	: bSerializingDelegateProperty(FALSE), CurrentObj(NULL), CurrentContext(NULL)
	{
		AllowEliminatingReferences(FALSE);
		ArIgnoreArchetypeRef = TRUE;
		ArIgnoreClassRef = TRUE;
		ArIgnoreOuterRef = TRUE;

		ArIsObjectReferenceCollector = TRUE;
	}

	void ProcessTemplateObjects()
	{
		// find all CDOs and subobject templates
		for ( FObjectIterator It; It; ++It )
		{
			if ( It->IsTemplate(RF_ClassDefaultObject) )
			{
				TemplateObjects.AddItem(*It);
			}
		}

		// serialize each template object
		for ( INT ObjIndex = 0; ObjIndex < TemplateObjects.Num(); ObjIndex++ )
		{
			CurrentObj = TemplateObjects(ObjIndex);
			CurrentObj->Serialize(*this);
		}

		// now mark all functions we encountered as referenced
		for ( INT FuncIndex = 0; FuncIndex < ReferencedFunctions.Num(); FuncIndex++ )
		{
			UFunction* FunctionObject = ReferencedFunctions(FuncIndex);
			if ( !UByteCodeSerializer::IsFunctionReferenced(FunctionObject) )
			{
				UByteCodeSerializer::MarkFunctionAsReferenced(FunctionObject);
			}
		}
	}

	/* === FArchive interface === */
	virtual FArchive& operator<<( FName& Name )
	{
		if ( bSerializingDelegateProperty )
		{
			if ( Name != NAME_None )
			{
				UObject* ContextObj = CurrentContext != NULL ? CurrentContext : CurrentObj;
				UClass* Scope = Cast<UClass>(ContextObj);
				if ( Scope == NULL )
				{
					Scope = ContextObj->GetClass();
				}

				UFunction* FunctionObj = FindFieldChecked<UFunction>(Scope, Name);
				ReferencedFunctions.AddItem(FunctionObj);
			}

			CurrentContext = NULL;
			bSerializingDelegateProperty = FALSE;
		}
		return *this;
	}

	/**
	 * Serializes the reference to the object
	 */
	virtual FArchive& operator<<( UObject*& Obj )
	{
		UFunction* FunctionObject = Cast<UFunction>(Obj);
		if ( FunctionObject != NULL )
		{
			ReferencedFunctions.AddItem(FunctionObject);
		}
		else if ( GSerializedProperty != NULL )
		{
			UDelegateProperty* DelProp = Cast<UDelegateProperty>(GSerializedProperty);
			if ( DelProp != NULL )
			{
				bSerializingDelegateProperty = TRUE;
				CurrentContext = Obj;
			}
		}

		return *this;
	}

	/**
  	 * Returns the name of this archive.
	 **/
	virtual FString GetArchiveName() const { return TEXT("FArchiveFunctionReferenceCollector"); }

private:
	UBOOL bSerializingDelegateProperty;

	/** the object currently being serialized */
	UObject* CurrentObj;

	/** the object that contains the function reference (usually relevant for delegates) */
	UObject* CurrentContext;

	/** The list of objects which are either class default objects, or contained within class default objects */
	TArray<UObject*> TemplateObjects;

	/** the list of functions encountered while processing the template objects */
	TLookupMap<UFunction*> ReferencedFunctions;
};


/**
 * Commandlet entry point
 *
 * @param	Params	the command line parameters that were passed in.
 *
 * @return	0 if the commandlet succeeded; otherwise, an error code defined by the commandlet.
 */
INT UFindUnreferencedFunctionsCommandlet::Main( const FString& Params )
{
	const TCHAR* CmdLine = *Params;

	TArray<FString> Tokens, Switches;
	ParseCommandLine(CmdLine, Tokens, Switches);

	Serializer = ConstructObject<UByteCodeSerializer>(UByteCodeSerializer::StaticClass());

	TArray<FString> PackageNames;
	TArray<FFilename> PackageFilenames;
	if ( !NormalizePackageNames(PackageNames, PackageFilenames, TEXT("*.u"), 0) )
	{
		warnf(TEXT("No files found using wildcard *.u"));
		return 0;
	}

	warnf(TEXT("Found %i files"), PackageFilenames.Num());
	for ( INT FileIndex = 0; FileIndex < PackageFilenames.Num(); FileIndex++ )
	{
		warnf(TEXT("Loading %s...."), *PackageFilenames(FileIndex));
		LoadPackage(NULL, *PackageFilenames(FileIndex), LOAD_None);
	}

	for ( TObjectIterator<UStruct> It; It; ++It )
	{
		Serializer->ProcessStructBytecode(*It);
	}

	// now gather references from class defaults
	FArchiveFunctionReferenceCollector FunctionReferenceCollector;
	FunctionReferenceCollector.ProcessTemplateObjects();

	//@todo - add support for switches
	/*
	- shownatives (include native functions in results)
	- nativesonly (only include native functions in results)
	- showdelegates
	- delegatesonly
	- showevents
	- eventsonly
	*/

	warnf(TEXT("Unreferenced functions:"));
	INT UnreferencedFunctionCount = 0;
	INT NativeFunctionCount = 0, UnreferencedNativeFunctionCount=0;
	INT EventCount = 0, UnreferencedEventCount=0;
	INT DelegateCount = 0, UnreferencedDelegateCount=0;
	INT ExecFunctionCount = 0;
	for ( TObjectIterator<UFunction> It; It; ++It )
	{
		UFunction* Function = *It;
		UClass* FunctionOwnerClass = Function->GetOwnerClass();
		if ( !FunctionOwnerClass->HasAnyClassFlags(CLASS_Interface) )
		{
			if ( Function->HasAnyFunctionFlags(FUNC_Event|FUNC_Native|FUNC_Delegate|FUNC_Operator|FUNC_PreOperator|FUNC_Exec) )
			{
				UBOOL bIsReferencedByScript = TRUE;
				if ( !Serializer->IsFunctionReferenced(Function) )
				{
					UFunction* InterfaceParent = GetInterfaceFunctionDeclaration(FunctionOwnerClass, Function);
					if ( InterfaceParent == NULL || !Serializer->IsFunctionReferenced(InterfaceParent) )
					{
						bIsReferencedByScript = FALSE;
					}
				}

				if ( Function->HasAnyFunctionFlags(FUNC_Native) )
				{
					NativeFunctionCount++;
					if ( !bIsReferencedByScript )
					{
						UnreferencedNativeFunctionCount++;
					}
				}
				else if ( Function->HasAnyFunctionFlags(FUNC_Event) )
				{
					EventCount++;
					if ( !bIsReferencedByScript )
					{
						UnreferencedEventCount++;
					}
				}
				else if ( Function->HasAnyFunctionFlags(FUNC_Delegate) )
				{
					DelegateCount++;
					if ( !bIsReferencedByScript )
					{
						UnreferencedDelegateCount++;
					}
				}
				else if ( Function->HasAnyFunctionFlags(FUNC_Exec) )
				{
					ExecFunctionCount++;
				}
			}
			else
			{
				if ( !Serializer->IsFunctionReferenced(Function) )
				{
					UFunction* InterfaceParent = GetInterfaceFunctionDeclaration(FunctionOwnerClass, Function);
					if ( InterfaceParent == NULL || !Serializer->IsFunctionReferenced(InterfaceParent) )
					{
						UnreferencedFunctionCount++;
						warnf(TEXT("    %s"), *It->GetFullName());
					}
				}
			}
		}
	}

	warnf(TEXT("%i functions unreferenced by this game"), UnreferencedFunctionCount);
	warnf(TEXT("%i unreported native functions (%i not referenced by script)"), NativeFunctionCount, UnreferencedNativeFunctionCount);
	warnf(TEXT("%i unreported events (%i not referenced by script)"), EventCount, UnreferencedEventCount);
	warnf(TEXT("%i unreported delegates (%i not referenced by script)"), DelegateCount, UnreferencedDelegateCount);
	warnf(TEXT("%i unreported exec functions"), ExecFunctionCount);

	warnf(TEXT("%i total functions not referenced by script"), UnreferencedFunctionCount + UnreferencedNativeFunctionCount + UnreferencedEventCount + UnreferencedDelegateCount);

	return 0;
}


IMPLEMENT_CLASS(UFindUnreferencedFunctionsCommandlet);

/*=============================================================================
	UnStack.h: UnrealScript execution stack definition.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

class UStruct;

/*-----------------------------------------------------------------------------
	Constants & types.
-----------------------------------------------------------------------------*/

// Sizes.
enum {MAX_STRING_CONST_SIZE		= 1024              }; 
/** this is the size of the buffer used by the script interpreter for unused simple (not constructed) return values.
 * Larger return values use the constructed value path (EX_EatReturnValue/execEatReturnValue)
 */
enum {MAX_SIMPLE_RETURN_VALUE_SIZE = 64 };

/**
 * a typedef for the size of an UnrealScript variable; typedef'd because this value must be synchronized between the
 * script compiler and the script VM
 */
typedef WORD VariableSizeType;

/**
 * This is the largest possible size that a single variable can be; a variables size is determined by multiplying the
 * size of the type by the variables ArrayDim (always 1 unless it's a static array).
 */
enum {MAX_VARIABLE_SIZE = 0x0FFF };

//
// UnrealScript intrinsic return value declaration.
//
#define RESULT_DECL void*const Result


//
// State flags.
//
enum EStateFlags
{
	// State flags.
	STATE_Editable		= 0x00000001,	// State should be user-selectable in UnrealEd.
	STATE_Auto			= 0x00000002,	// State is automatic (the default state).
	STATE_Simulated     = 0x00000004,   // State executes on client side.
};

//
// Function flags.
//
enum EFunctionFlags
{
	// Function flags.
	FUNC_Final				= 0x00000001,	// Function is final (prebindable, non-overridable function).
	FUNC_Defined			= 0x00000002,	// Function has been defined (not just declared).
	FUNC_Iterator			= 0x00000004,	// Function is an iterator.
	FUNC_Latent				= 0x00000008,	// Function is a latent state function.
	FUNC_PreOperator		= 0x00000010,	// Unary operator is a prefix operator.
	FUNC_Singular			= 0x00000020,   // Function cannot be reentered.
	FUNC_Net				= 0x00000040,   // Function is network-replicated.
	FUNC_NetReliable		= 0x00000080,   // Function should be sent reliably on the network.
	FUNC_Simulated			= 0x00000100,	// Function executed on the client side.
	FUNC_Exec				= 0x00000200,	// Executable from command line.
	FUNC_Native				= 0x00000400,	// Native function.
	FUNC_Event				= 0x00000800,   // Event function.
	FUNC_Operator			= 0x00001000,   // Operator function.
	FUNC_Static				= 0x00002000,   // Static function.
	FUNC_HasOptionalParms	= 0x00004000,	// Function has optional parameters
	FUNC_Const				= 0x00008000,   // Function doesn't modify this object.
	//						= 0x00010000,	// unused
	FUNC_Public				= 0x00020000,	// Function is accessible in all classes (if overridden, parameters much remain unchanged).
	FUNC_Private			= 0x00040000,	// Function is accessible only in the class it is defined in (cannot be overriden, but function name may be reused in subclasses.  IOW: if overridden, parameters don't need to match, and Super.Func() cannot be accessed since it's private.)
	FUNC_Protected			= 0x00080000,	// Function is accessible only in the class it is defined in and subclasses (if overridden, parameters much remain unchanged).
	FUNC_Delegate			= 0x00100000,	// Function is actually a delegate.
	FUNC_NetServer			= 0x00200000,	// Function is executed on servers (set by replication code if passes check)
	FUNC_HasOutParms		= 0x00400000,	// function has out (pass by reference) parameters
	FUNC_HasDefaults		= 0x00800000,	// function has structs that contain defaults
	FUNC_NetClient			= 0x01000000,	// function is executed on clients

	// Combinations of flags.
	FUNC_FuncInherit        = FUNC_Exec | FUNC_Event,
	FUNC_FuncOverrideMatch	= FUNC_Exec | FUNC_Final | FUNC_Latent | FUNC_PreOperator | FUNC_Iterator | FUNC_Static | FUNC_Public | FUNC_Protected | FUNC_Const,
	FUNC_NetFuncFlags       = FUNC_Net | FUNC_NetReliable | FUNC_NetServer | FUNC_NetClient,

	FUNC_AllFlags		= 0xFFFFFFFF,
};


enum ERuntimeUCFlags
{
	RUC_ArrayLengthSet			=	0x01,	//setting the length of an array, not an element of the array
	RUC_SkippedOptionalParm		=	0x02,	// value for an optional parameter was not included in the function call
	RUC_NeverExpandDynArray		=	0x04,	// flag for execDynArrayElement() to prevent it increasing the array size for index out of bounds
};

//
// Evaluatable expression item types.
//
enum EExprToken
{
	// Variable references.
	EX_LocalVariable		= 0x00,	// A local variable.
	EX_InstanceVariable		= 0x01,	// An object variable.
	EX_DefaultVariable		= 0x02,	// Default variable for a concrete object.

	// Tokens.
	EX_Return				= 0x04,	// Return from function.
	EX_Switch				= 0x05,	// Switch.
	EX_Jump					= 0x06,	// Goto a local address in code.
	EX_JumpIfNot			= 0x07,	// Goto if not expression.
	EX_Stop					= 0x08,	// Stop executing state code.
	EX_Assert				= 0x09,	// Assertion.
	EX_Case					= 0x0A,	// Case.
	EX_Nothing				= 0x0B,	// No operation.
	EX_LabelTable			= 0x0C,	// Table of labels.
	EX_GotoLabel			= 0x0D,	// Goto a label.
	EX_EatReturnValue       = 0x0E, // destroy an unused return value
	EX_Let					= 0x0F,	// Assign an arbitrary size value to a variable.
	EX_DynArrayElement      = 0x10, // Dynamic array element.!!
	EX_New                  = 0x11, // New object allocation.
	EX_ClassContext         = 0x12, // Class default metaobject context.
	EX_MetaCast             = 0x13, // Metaclass cast.
	EX_LetBool				= 0x14, // Let boolean variable.
	EX_EndParmValue			= 0x15,	// end of default value for optional function parameter
	EX_EndFunctionParms		= 0x16,	// End of function call parameters.
	EX_Self					= 0x17,	// Self object.
	EX_Skip					= 0x18,	// Skippable expression.
	EX_Context				= 0x19,	// Call a function through an object context.
	EX_ArrayElement			= 0x1A,	// Array element.
	EX_VirtualFunction		= 0x1B,	// A function call with parameters.
	EX_FinalFunction		= 0x1C,	// A prebound function call with parameters.
	EX_IntConst				= 0x1D,	// Int constant.
	EX_FloatConst			= 0x1E,	// Floating point constant.
	EX_StringConst			= 0x1F,	// String constant.
	EX_ObjectConst		    = 0x20,	// An object constant.
	EX_NameConst			= 0x21,	// A name constant.
	EX_RotationConst		= 0x22,	// A rotation constant.
	EX_VectorConst			= 0x23,	// A vector constant.
	EX_ByteConst			= 0x24,	// A byte constant.
	EX_IntZero				= 0x25,	// Zero.
	EX_IntOne				= 0x26,	// One.
	EX_True					= 0x27,	// Bool True.
	EX_False				= 0x28,	// Bool False.
	EX_NativeParm           = 0x29, // Native function parameter offset.
	EX_NoObject				= 0x2A,	// NoObject.

	EX_IntConstByte			= 0x2C,	// Int constant that requires 1 byte.
	EX_BoolVariable			= 0x2D,	// A bool variable which requires a bitmask.
	EX_DynamicCast			= 0x2E,	// Safe dynamic class casting.
	EX_Iterator             = 0x2F, // Begin an iterator operation.
	EX_IteratorPop          = 0x30, // Pop an iterator level.
	EX_IteratorNext         = 0x31, // Go to next iteration.
	EX_StructCmpEq          = 0x32,	// Struct binary compare-for-equal.
	EX_StructCmpNe          = 0x33,	// Struct binary compare-for-unequal.
	EX_UnicodeStringConst   = 0x34, // Unicode string constant.
	EX_StructMember         = 0x35, // Struct member.
	EX_DynArrayLength		= 0x36,	// A dynamic array length for setting/getting
	EX_GlobalFunction		= 0x37, // Call non-state version of a function.
	EX_PrimitiveCast		= 0x38,	// A casting operator for primitives which reads the type as the subsequent byte
	EX_DynArrayInsert		= 0x39,	// Inserts into a dynamic array
	EX_ReturnNothing		= 0x3A, // failsafe for functions that return a value - returns the zero value for a property and logs that control reached the end of a non-void function
	EX_EqualEqual_DelDel	= 0x3B,	// delegate comparison for equality
	EX_NotEqual_DelDel		= 0x3C, // delegate comparison for inequality
	EX_EqualEqual_DelFunc	= 0x3D,	// delegate comparison for equality against a function
	EX_NotEqual_DelFunc		= 0x3E,	// delegate comparison for inequality against a function
	EX_EmptyDelegate		= 0x3F,	// delegate 'None'
	EX_DynArrayRemove		= 0x40,	// Removes from a dynamic array
	EX_DebugInfo			= 0x41,	//DEBUGGER Debug information
	EX_DelegateFunction		= 0x42, // Call to a delegate function
	EX_DelegateProperty		= 0x43, // Delegate expression
	EX_LetDelegate			= 0x44, // Assignment to a delegate
	EX_Conditional			= 0x45, // tertiary operator support
	EX_DynArrayFind			= 0x46, // dynarray search for item index
	EX_DynArrayFindStruct	= 0x47, // dynarray<struct> search for item index
	EX_LocalOutVariable		= 0x48, // local out (pass by reference) function parameter
	EX_DefaultParmValue		= 0x49,	// default value of optional function parameter
	EX_EmptyParmValue		= 0x4A,	// unspecified value for optional function parameter
	EX_InstanceDelegate		= 0x4B,	// const reference to a delegate or normal function object

#if BATMAN
	// https://github.com/EliotVU/Unreal-Library/blob/f7e45802bf69b89ce979af7c2dd8011f8ac6f4a8/src/Branch/UE3/RSS/EngineBranch.RSS.cs#L17
	EX_RSSContext			= 0x50,
#endif

	EX_InterfaceContext		= 0x51,	// Call a function through a native interface variable
	EX_InterfaceCast		= 0x52,	// Converting an object reference to native interface variable
	EX_EndOfScript			= 0x53, // Last byte in script code
	EX_DynArrayAdd			= 0x54, // Add to a dynamic array
	EX_DynArrayAddItem		= 0x55, // Add an item to a dynamic array
	EX_DynArrayRemoveItem	= 0x56, // Remove an item from a dynamic array
	EX_DynArrayInsertItem	= 0x57, // Insert an item into a dynamic array
	EX_DynArrayIterator		= 0x58, // Iterate through a dynamic array

	// Natives.
	EX_ExtendedNative		= 0x60,
	EX_FirstNative			= 0x70,
	EX_Max					= 0x1000,
};


enum ECastToken
{
	CST_InterfaceToObject	= 0x36,
	CST_InterfaceToString	= 0x37,
	CST_InterfaceToBool		= 0x38,
	CST_RotatorToVector		= 0x39,
	CST_ByteToInt			= 0x3A,
	CST_ByteToBool			= 0x3B,
	CST_ByteToFloat			= 0x3C,
	CST_IntToByte			= 0x3D,
	CST_IntToBool			= 0x3E,
	CST_IntToFloat			= 0x3F,
	CST_BoolToByte			= 0x40,
	CST_BoolToInt			= 0x41,
	CST_BoolToFloat			= 0x42,
	CST_FloatToByte			= 0x43,
	CST_FloatToInt			= 0x44,
	CST_FloatToBool			= 0x45,
	CST_ObjectToInterface	= 0x46,
	CST_ObjectToBool		= 0x47,
	CST_NameToBool			= 0x48,
	CST_StringToByte		= 0x49,
	CST_StringToInt			= 0x4A,
	CST_StringToBool		= 0x4B,
	CST_StringToFloat		= 0x4C,
	CST_StringToVector		= 0x4D,
	CST_StringToRotator		= 0x4E,
	CST_VectorToBool		= 0x4F,
	CST_VectorToRotator		= 0x50,
	CST_RotatorToBool		= 0x51,
	CST_ByteToString		= 0x52,
	CST_IntToString			= 0x53,
	CST_BoolToString		= 0x54,
	CST_FloatToString		= 0x55,
	CST_ObjectToString		= 0x56,
	CST_NameToString		= 0x57,
	CST_VectorToString		= 0x58,
	CST_RotatorToString		= 0x59,
	CST_DelegateToString	= 0x5A,
// 	CST_StringToDelegate	= 0x5B,
	CST_StringToName		= 0x60,
	CST_Max					= 0xFF,
};


//
// Latent functions.
//
enum EPollSlowFuncs
{
	EPOLL_Sleep				= 384,
	EPOLL_FinishAnim		= 385
};

/*-----------------------------------------------------------------------------
	Execution stack helpers.
-----------------------------------------------------------------------------*/

//
// Information remembered about an Out parameter.
//
struct FOutParmRec
{
	UProperty* Property;
	BYTE*      PropAddr;
	FOutParmRec* NextOutParm;
};

//
// Information about script execution at one stack level.
//
struct FFrame : public FOutputDevice
{	
	// Variables.
	UStruct*	Node;
	UObject*	Object;
	BYTE*		Code;
	BYTE*		Locals;

	/** Previous frame on the stack */
	FFrame* PreviousFrame;
	/** containts information on any out parameters */
	FOutParmRec* OutParms;

	// Constructors.
	FFrame( UObject* InObject );
	FFrame( UObject* InObject, UStruct* InNode, INT CodeOffset, void* InLocals, FFrame* InPreviousFrame = NULL );

	// Functions.
	FORCEINLINE void Step( UObject* Context, RESULT_DECL );
	void Serialize( const TCHAR* V, enum EName Event );
	INT ReadInt();
	FLOAT ReadFloat();
	FName ReadName();
	FName ReadNamePassive() const;
	UObject* ReadObject();
	UObject* ReadObjectPassive() const;
	INT ReadWord();
	VariableSizeType ReadVariableSize();

	/**
 	 * This will return the StackTrace of the current callstack from .uc land
	 **/
	FString GetStackTrace() const;
};

/**
 * Contains information about script execution at the main stack
 * level.  This part of an object's script state is saveable at
 * any time.
 */
struct FStateFrame : public FFrame
{
	/** Current state node */
	UState* StateNode;

	/** Mask for all functions the current state is probing */
	QWORD   ProbeMask;

	/** Current latent action, 0 for no active latent */
	WORD     LatentAction;

	/** set during the time between when a state is continued (via a higher state being popped via PopState) 
	 * until the object's state code is next executed
	 * used to update the script debugger's view of the script stack
	 */
	BYTE bContinuedState;

	/** Contains information needed for returning to a previous state */
	struct FPushedState
	{
	public:
		UState* State;
		UStruct* Node;
		BYTE* Code;
		
		FPushedState()
		: State(NULL)
		, Node(NULL)
		, Code(NULL)
		{
		}
	};
	friend FArchive& operator<<(FArchive& Ar,FPushedState& PushedState);

	/** List of the currently pushed states */
	TArray<FPushedState> StateStack;

	// Functions.
	FStateFrame(UObject* InObject);
	FString Describe();
};

/*-----------------------------------------------------------------------------
	Script execution helpers.
-----------------------------------------------------------------------------*/

//
// Base class for UnrealScript iterator lists.
//
struct FIteratorList
{
	FIteratorList* Next;
	FIteratorList() {}
	FIteratorList( FIteratorList* InNext ) : Next( InNext ) {}
	FIteratorList* GetNext() { return (FIteratorList*)Next; }
};

/*-----------------------------------------------------------------------------
	Call graph profiling.
-----------------------------------------------------------------------------*/

/**
 * Non- intrusive script call graph profiler, collecting function pointer and cycle count
 * per function for later analysis in a standalone tool.
 */
struct FScriptCallGraph
{
	/**
	 * Constructor
	 *
	 * @param InSoftMemoryLimit		Max number of bytes used by call graph data (soft limit).
	 * @param InTime				The number of seconds to profile before stopping.
	 *								If 0, then do not auto-stop.
	 */
	FScriptCallGraph( DWORD InSoftMemoryLimit, INT InTime = 0 );

	/**
	 * Resets data collection and memory use.
	 *
	 * @param InSoftMemoryLimit		Max number of bytes used by call graph data (soft limit).
	 */
	void Reset( DWORD InSoftMemoryLimit = 0 );
	
	/**
	 * Serializes the profiling data to the passed in archive.	
	 *
	 * @param Ar	Archive to serialize data to.
	 */
	void Serialize( FArchive& Ar );
	
	/**
	 * Emits the end of frame marker. Needs to be called when there are no script functions
	 * on the stack. Also responsible for reseting call graph data if memory limit is reached.
	 */
	void Tick(FLOAT DeltaTime);

	/**
	 * Emits function call token to stream.	
	 *
	 * @param Function	Function pointer to emit.
	 */
	inline void EmitFunction( UFunction* Function )
	{
		Data.AddItem( (PTRINT) Function );
	}

	/**
	 * Emits cycle count token.
	 *
	 * @param Cycles	Cycle count to emit.
	 */
	inline void EmitCycles( DWORD Cycles )
	{
		DWORD Value = (Cycles & ~3) | 1;
		Data.AddItem( Value );
	}

	/**
	 * Emits frame end marker.	
	 */
	inline void EmitFrameEnd()
	{
		DWORD Value = 3;
		Data.AddItem( Value );
	}

	/**
	 * Emits end of stream marker.	
	 */
	inline void EmitEnd()
	{
		DWORD Value = 0;
		Data.AddItem( Value );
	}

	/**
	 * Figures out whether the passed in value is a frame end marker.
	 *
	 * return TRUE if the passed in value is a frame end marker, FALSE otherwise
	 */
	inline UBOOL IsFrameEndMarker( DWORD Value ) const
	{
		return Value == 3;
	}

	/**
	 * Figures out whether the passed in value is the end marker.
	 * 
	 * return TRUE if the passed in value is the end marker, FALSE otherwise
	 */
	inline UBOOL IsEndMarker( DWORD Value ) const
	{
		return Value == 0;
	}

	/**
	 * Returns the index [into the Data array] for the next frame end marker after the starting point specified.
	 *
	 * @param	StartingPoint	optional starting index for searching for the next end frame marker; the first element to be checked
	 *							will be the element at this index (so for recursive searching, pass-in 1 + the result of the last call to
	 *							FindNextFrameMarker)
	 *
	 * @return	the index [into the Data array] for the next end frame marker, or INDEX_NONE if none were found.
	 */
	INT FindNextFrameMarkerPosition( INT StartingPoint=0 ) const;

	/**
	 * Returns the location of the profile capture end marker position, or INDEX_NONE if there isn't one;
	 *
	 * @param	StartingPoint	optional starting index to begin searching for the end marker.
	 */
	INT FindProfileEndMarkerPosition( INT StartingPoint=0 ) const;

	/** Array of call graph profiling tokens */
	TArray<PTRINT>		Data;
	/** Soft memory limit call graph profiling data is allowed to reach before it resets itself */
	DWORD				SoftMemoryLimit;

	/**
	 *	The amount of time (in seconds) to profile before stopping.
	 *	If 0, do not auto-stop...
	 */
	INT TimeToProfile;

	/** The current profiling total time. */
	FLOAT CurrProfileTime;
	  
};
/** Global script call graph profiler */
extern FScriptCallGraph* GScriptCallGraph;

/**
 * Helper class for call graph profiling. Scoped to correctly deal with multiple return
 * paths.
 */
struct FScopedScriptStats
{
	/** 
	 * Constructor, emiting function pointer to stream.
	 *
	 * @param InFunction	Function about to be called.
	 */
	FScopedScriptStats( UFunction* InFunction )
	: Function(NULL), StartCycles(0)
	{
		if( GScriptCallGraph )
		{
			StartCycles = appCycles();
			Function	= InFunction;
			GScriptCallGraph->EmitFunction( Function );
		}
	}

	/**
	 * Destructor, emiting cycles spent in function call.	
	 */
	~FScopedScriptStats()
	{
		// Function is NULL if we started a capture in the middle of a callstack
		if( GScriptCallGraph != NULL && Function != NULL )
		{
			DWORD Cycles = appCycles() - StartCycles;
			GScriptCallGraph->EmitCycles( Cycles );
		}
	}

private:
	/** Function associated with callss */
	UFunction*	Function;
	/** Cycle count before function was being called. */
	DWORD		StartCycles;
};


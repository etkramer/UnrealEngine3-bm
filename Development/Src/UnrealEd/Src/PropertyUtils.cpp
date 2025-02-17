/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "PropertyUtils.h"
#include "Properties.h"
#include "PropertyWindowManager.h"
#include "GenericBrowser.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Equation evaluation.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Converts a string to it's numeric equivalent, ignoring whitespace.
 * "123  45" - becomes 12,345
 *
 * @param	Value	The string to convert.
 * @return			The converted value.
 */
FLOAT Val(const FString& Value)
{
	FLOAT RetValue = 0;

	for( INT x = 0 ; x < Value.Len() ; x++ )
	{
		FString Char = Value.Mid(x, 1);

		if( Char >= TEXT("0") && Char <= TEXT("9") )
		{
			RetValue *= 10;
			RetValue += appAtoi( *Char );
		}
		else 
		{
			if( Char != TEXT(" ") )
			{
				break;
			}
		}
	}

	return RetValue;
}

FString GrabChar( FString* pStr )
{
	FString GrabChar;
	if( pStr->Len() )
	{
		do
		{		
			GrabChar = pStr->Left(1);
			*pStr = pStr->Mid(1);
		} while( GrabChar == TEXT(" ") );
	}
	else
	{
		GrabChar = TEXT("");
	}

	return GrabChar;
}

UBOOL SubEval( FString* pStr, FLOAT* pResult, INT Prec )
{
	FString c;
	FLOAT V, W, N;

	V = W = N = 0.0f;

	c = GrabChar(pStr);

	if( (c >= TEXT("0") && c <= TEXT("9")) || c == TEXT(".") )	// Number
	{
		V = 0;
		while(c >= TEXT("0") && c <= TEXT("9"))
		{
			V = V * 10 + Val(c);
			c = GrabChar(pStr);
		}

		if( c == TEXT(".") )
		{
			N = 0.1f;
			c = GrabChar(pStr);

			while(c >= TEXT("0") && c <= TEXT("9"))
			{
				V = V + N * Val(c);
				N = N / 10.0f;
				c = GrabChar(pStr);
			}
		}
	}
	else if( c == TEXT("("))									// Opening parenthesis
	{
		if( !SubEval(pStr, &V, 0) )
		{
			return 0;
		}
		c = GrabChar(pStr);
	}
	else if( c == TEXT("-") )									// Negation
	{
		if( !SubEval(pStr, &V, 1000) )
		{
			return 0;
		}
		V = -V;
		c = GrabChar(pStr);
	}
	else if( c == TEXT("+"))									// Positive
	{
		if( !SubEval(pStr, &V, 1000) )
		{
			return 0;
		}
		c = GrabChar(pStr);
	}
	else if( c == TEXT("@") )									// Square root
	{
		if( !SubEval(pStr, &V, 1000) )
		{
			return 0;
		}

		if( V < 0 )
		{
			debugf(TEXT("Expression Error : Can't take square root of negative number"));
			return 0;
		}
		else
		{
			V = appSqrt(V);
		}

		c = GrabChar(pStr);
	}
	else														// Error
	{
		debugf(TEXT("Expression Error : No value recognized"));
		return 0;
	}
PrecLoop:
	if( c == TEXT("") )
	{
		*pResult = V;
		return 1;
	}
	else if( c == TEXT(")") )
	{
		*pStr = FString(TEXT(")")) + *pStr;
		*pResult = V;
		return 1;
	}
	else if( c == TEXT("+") )
	{
		if( Prec > 1 )
		{
			*pResult = V;
			*pStr = c + *pStr;
			return 1;
		}
		else
		{
			if( SubEval(pStr, &W, 2) )
			{
				V = V + W;
				c = GrabChar(pStr);
				goto PrecLoop;
			}
		}
	}
	else if( c == TEXT("-") )
	{
		if( Prec > 1 )
		{
			*pResult = V;
			*pStr = c + *pStr;
			return 1;
		}
		else
		{
			if( SubEval(pStr, &W, 2) )
			{
				V = V - W;
				c = GrabChar(pStr);
				goto PrecLoop;
			}
		}
	}
	else if( c == TEXT("/") )
	{
		if( Prec > 2 )
		{
			*pResult = V;
			*pStr = c + *pStr;
			return 1;
		}
		else
		{
			if( SubEval(pStr, &W, 3) )
			{
				if( W == 0 )
				{
					debugf(TEXT("Expression Error : Division by zero isn't allowed"));
					return 0;
				}
				else
				{
					V = V / W;
					c = GrabChar(pStr);
					goto PrecLoop;
				}
			}
		}
	}
	else if( c == TEXT("%") )
	{
		if( Prec > 2 )
		{
			*pResult = V;
			*pStr = c + *pStr;
			return 1;
		}
		else
		{
			if( SubEval(pStr, &W, 3) )
			{
				if( W == 0 )
				{
					debugf(TEXT("Expression Error : Modulo zero isn't allowed"));
					return 0;
				}
				else
				{
					V = (INT)V % (INT)W;
					c = GrabChar(pStr);
					goto PrecLoop;
				}
			}
		}
	}
	else if( c == TEXT("*") )
	{
		if( Prec > 3 )
		{
			*pResult = V;
			*pStr = c + *pStr;
			return 1;
		}
		else
		{
			if( SubEval(pStr, &W, 4) )
			{
				V = V * W;
				c = GrabChar(pStr);
				goto PrecLoop;
			}
		}
	}
	else
	{
		debugf(TEXT("Expression Error : Unrecognized Operator"));
	}

	*pResult = V;
	return 1;
}

/**
 * Evaluates a numerical equation.
 *
 * Operators and precedence: 1:+- 2:/% 3:* 4:^ 5:&|
 * Unary: -
 * Types: Numbers (0-9.), Hex ($0-$f)
 * Grouping: ( )
 *
 * @param	Str			String containing the equation.
 * @param	pResult		Pointer to storage for the result.
 * @return				1 if successful, 0 if equation fails.
 */
UBOOL Eval( FString Str, FLOAT* pResult )
{
	FLOAT Result;

	// Check for a matching number of brackets right up front.
	INT Brackets = 0;
	for( INT x = 0 ; x < Str.Len() ; x++ )
	{
		if( Str.Mid(x,1) == TEXT("(") )
		{
			Brackets++;
		}
		if( Str.Mid(x,1) == TEXT(")") )
		{
			Brackets--;
		}
	}

	if( Brackets != 0 )
	{
		debugf(TEXT("Expression Error : Mismatched brackets"));
		Result = 0;
	}

	else
	{
		if( !SubEval( &Str, &Result, 0 ) )
		{
			debugf(TEXT("Expression Error : Error in expression"));
			Result = 0;
		}
	}

	*pResult = Result;

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Convenience definitions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

typedef WxPropertyWindow_Objects::TObjectIterator		TPropObjectIterator;
typedef WxPropertyWindow_Objects::TObjectConstIterator	TPropObjectConstIterator;

static FORCEINLINE INT GetButtonSize()
{
	return 15;
}

#ifdef IS_EDITCONST
#error IS_EDITCONST already defined
#else
#define IS_EDITCONST( x )		( (x)->PropertyFlags & CPF_EditConst )
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Property drawing
//
///////////////////////////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
	UPropertyDrawProxy
-----------------------------------------------------------------------------*/
/**
 * @return	Returns the height required by the input proxy.
 */
INT UPropertyDrawProxy::GetProxyHeight() const
{
	return PROP_DefaultItemHeight;
}

void UPropertyDrawProxy::Draw( wxDC* InDC, wxRect InRect, BYTE* InReadAddress, UProperty* InProperty, UPropertyInputProxy* InInputProxy )
{
	// Extract text from the property . . .
	FString Str;
	InProperty->ExportText( 0, Str, InReadAddress - InProperty->Offset, InReadAddress - InProperty->Offset, NULL, PPF_Localized );
	UByteProperty* EnumProperty = Cast<UByteProperty>(InProperty);
	if ( EnumProperty != NULL && EnumProperty->Enum != NULL )
	{
		// see if we have alternate text to use for displaying the value
		UMetaData* PackageMetaData = EnumProperty->GetOutermost()->GetMetaData();
		if ( PackageMetaData )
		{
			FName AltDisplayName = FName(*(Str+TEXT(".DisplayName")));
			FString ValueText = PackageMetaData->GetValue(EnumProperty->Enum, AltDisplayName);
			if ( ValueText.Len() > 0 )
			{
				// render the alternate text for this enum value
				Str = ValueText;
			}
		}
	}

	
	// To be consistent with how UPropertyInputCombo displays values, we'll check to see if this is class property,
	// and if so, we'll omit the full object path and just display the value
	UClassProperty* ClassProp = Cast<UClassProperty>( InProperty );
	if( ClassProp != NULL )
	{
		// Trim everything up to the last dot character
		INT DotPos = Str.InStr( ".", TRUE );
		if( DotPos != -1 )
		{
			Str = Str.Right( Str.Len() - DotPos - 1 );
		}

		// Eliminate trailing ' character if it has one
		while( Str.Len() > 0 && Str.GetCharArray()( Str.Len() - 1 ) == TCHAR( '\'' ) )
		{
			Str = Str.Left( Str.Len() - 1 );
		}
	}


	// . . . and draw it.
	wxCoord W, H;
	InDC->GetTextExtent( *Str, &W, &H );
	InDC->DrawText( *Str, InRect.x, InRect.y+((InRect.GetHeight() - H) / 2) );
}

void UPropertyDrawProxy::DrawUnknown( wxDC* InDC, wxRect InRect )
{
	// Draw an empty string to the property window.
	const FString Str = LocalizeUnrealEd( "MultiplePropertyValues" );

	wxCoord W, H;
	InDC->GetTextExtent( *Str, &W, &H );
	InDC->DrawText( *Str, InRect.x, InRect.y+((InRect.GetHeight() - H) / 2) );
}

/*-----------------------------------------------------------------------------
	UPropertyDrawNumeric
-----------------------------------------------------------------------------*/

UBOOL UPropertyDrawNumeric::Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const
{
	// Supported if an atomic type that is not a UStructProperty named NAME_Rotator or editconst.
	if(		( InItem->Property->IsA(UFloatProperty::StaticClass()) || InItem->Property->IsA(UIntProperty::StaticClass())
		||	( InItem->Property->IsA(UByteProperty::StaticClass()) && Cast<UByteProperty>(InItem->Property)->Enum == NULL) )
		&& !( Cast<UStructProperty>(InItem->ParentItem->Property,CLASS_IsAUStructProperty) && Cast<UStructProperty>(InItem->ParentItem->Property,CLASS_IsAUStructProperty)->Struct->GetFName() == NAME_Rotator )
		&& !IS_EDITCONST( InItem->Property ) )
	{
		return TRUE;
	}

	return FALSE;
}

void UPropertyDrawNumeric::Draw( wxDC* InDC, wxRect InRect, BYTE* InReadAddress, UProperty* InProperty, UPropertyInputProxy* InInputProxy )
{
	// By default, draw the equation.  If it's empty, draw the actual value.

	FString Str = ((UPropertyInputNumeric*)InInputProxy)->Equation;
	if( Str.Len() == 0 )
	{
		InProperty->ExportText( 0, Str, InReadAddress - InProperty->Offset, InReadAddress - InProperty->Offset, NULL, PPF_Localized );
	}

	// Draw it.
	wxCoord W, H;
	InDC->GetTextExtent( *Str, &W, &H );
	InDC->DrawText( *Str, InRect.x, InRect.y+((InRect.GetHeight() - H) / 2) );
}

/*-----------------------------------------------------------------------------
	UPropertyDrawColor
-----------------------------------------------------------------------------*/

UBOOL UPropertyDrawColor::Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const
{
	// Supported if the property is a UStructProperty named NAME_Color or NAME_LinearColor.
	return ( Cast<UStructProperty>(InItem->Property,CLASS_IsAUStructProperty) &&
			(Cast<UStructProperty>(InItem->Property,CLASS_IsAUStructProperty)->Struct->GetFName()==NAME_Color ||
			 Cast<UStructProperty>(InItem->Property,CLASS_IsAUStructProperty)->Struct->GetFName()==NAME_LinearColor) );
}

void UPropertyDrawColor::Draw( wxDC* InDC, wxRect InRect, BYTE* InReadAddress, UProperty* InProperty, UPropertyInputProxy* InInputProxy )
{
	FColor	Color;

	if( Cast<UStructProperty>(InProperty,CLASS_IsAUStructProperty)->Struct->GetFName() == NAME_Color )
	{
		Color = *(FColor*)InReadAddress;
	}
	else
	{
		check( Cast<UStructProperty>(InProperty,CLASS_IsAUStructProperty)->Struct->GetFName() == NAME_LinearColor );
		Color = FColor(*(FLinearColor*)InReadAddress);
	}

	const wxColour WkColor( Color.R, Color.G, Color.B );
	InDC->SetPen( *wxBLACK_PEN );
	InDC->SetBrush( wxBrush( WkColor, wxSOLID ) );

	check( InInputProxy );
	const INT NumButtons = InInputProxy->GetNumButtons();//InInputProxy ? InInputProxy->GetNumButtons() : 0;
	InDC->DrawRectangle( InRect.x+4,InRect.y+4, InRect.GetWidth()-(NumButtons*GetButtonSize())-12,InRect.GetHeight()-8 );
}

/*-----------------------------------------------------------------------------
	UPropertyDrawRotation
-----------------------------------------------------------------------------*/

UBOOL UPropertyDrawRotation::Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const
{
	// Supported if the property is a UStructProperty named NAME_Rotator.
	return (Cast<UStructProperty>(InItem->ParentItem->Property,CLASS_IsAUStructProperty) &&
			Cast<UStructProperty>(InItem->ParentItem->Property,CLASS_IsAUStructProperty)->Struct->GetFName() == NAME_Rotator);
}

void UPropertyDrawRotation::Draw( wxDC* InDC, wxRect InRect, BYTE* InReadAddress, UProperty* InProperty, UPropertyInputProxy* InInputProxy )
{
	FLOAT Val = *((INT*)InReadAddress);
	Val = 360.f * (Val / 65536.f);

	FString Wk;
	if( Abs(Val) > 359.f )
	{
		const INT Revolutions = Val / 360.f;
		Val -= Revolutions * 360;
		Wk = FString::Printf( TEXT("%.2f%c %s %d"), Val, 176, (Revolutions < 0)?TEXT("-"):TEXT("+"), abs(Revolutions) );
	}
	else
	{
		Wk = FString::Printf( TEXT("%.2f%c"), Val, 176 );
	}

	// note : The degree symbol is ASCII code 248 (char code 176)
	InDC->DrawText( *Wk, InRect.x,InRect.y+1 );
}

/*-----------------------------------------------------------------------------
	UPropertyDrawRotationHeader
-----------------------------------------------------------------------------*/

UBOOL UPropertyDrawRotationHeader::Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const
{
	// Supported if the property is a UStructProperty named NAME_Rotator.
	return( Cast<UStructProperty>(InItem->Property,CLASS_IsAUStructProperty) &&
			Cast<UStructProperty>(InItem->Property,CLASS_IsAUStructProperty)->Struct->GetFName() == NAME_Rotator );
}

void UPropertyDrawRotationHeader::Draw( wxDC* InDC, wxRect InRect, BYTE* InReadAddress, UProperty* InProperty, UPropertyInputProxy* InInputProxy )
{
	InDC->DrawText( TEXT("..."), InRect.x+2,InRect.y+1 );
}

/*-----------------------------------------------------------------------------
	UPropertyDrawBool
-----------------------------------------------------------------------------*/

UBOOL UPropertyDrawBool::Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const
{
	// Supported if the property is a UBoolProperty.
	return ( Cast<UBoolProperty>(InItem->Property) != NULL );
}

void UPropertyDrawBool::Draw( wxDC* InDC, wxRect InRect, BYTE* InReadAddress, UProperty* InProperty, UPropertyInputProxy* InInputProxy )
{
	const WxMaskedBitmap& bmp = (*(BITFIELD*)InReadAddress & Cast<UBoolProperty>(InProperty)->BitMask) ? GPropertyWindowManager->CheckBoxOnB : GPropertyWindowManager->CheckBoxOffB;
	InDC->DrawBitmap( bmp, InRect.x, InRect.y+((InRect.GetHeight() - 13) / 2), 1 );
}

void UPropertyDrawBool::DrawUnknown( wxDC* InDC, wxRect InRect )
{
	const WxMaskedBitmap& bmp = GPropertyWindowManager->CheckBoxUnknownB;
	InDC->DrawBitmap( bmp, InRect.x, InRect.y+((InRect.GetHeight() - 13) / 2), 1 );
}

/*-----------------------------------------------------------------------------
	UPropertyDrawArrayHeader
-----------------------------------------------------------------------------*/

UBOOL UPropertyDrawArrayHeader::Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const
{
	// Supported if the property is a UArrayProperty
	return ( ( InItem->Property->ArrayDim != 1 && InArrayIdx == -1 ) || Cast<UArrayProperty>(InItem->Property) );
}

void UPropertyDrawArrayHeader::Draw( wxDC* InDC, wxRect InRect, BYTE* InReadAddress, UProperty* InProperty, UPropertyInputProxy* InInputProxy )
{
	const FString Str = *LocalizeUnrealEd( "ArrayPropertyHeader" );
	wxCoord W, H;
	InDC->GetTextExtent( *Str, &W, &H );
	InDC->DrawText( wxString( *Str ), InRect.x, InRect.y+((InRect.GetHeight() - H) / 2) );
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Property input
//
///////////////////////////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
WxPropertyItemButton
-----------------------------------------------------------------------------*/
IMPLEMENT_DYNAMIC_CLASS(WxPropertyItemButton, wxBitmapButton);

BEGIN_EVENT_TABLE(WxPropertyItemButton, wxBitmapButton)
EVT_ERASE_BACKGROUND(WxPropertyItemButton::OnEraseBackground)
END_EVENT_TABLE()

/**
 * Constructor.
 */
WxPropertyItemButton::WxPropertyItemButton()
{

}

/**
 * Constructor.
 */
WxPropertyItemButton::WxPropertyItemButton(wxWindow *parent,
											wxWindowID id,
											const wxBitmap& bitmap,
											const wxPoint& pos,
											const wxSize& size,
											long style,
											const wxValidator& validator,
											const wxString& name)
											: wxBitmapButton(parent, id, bitmap, pos, size, style, validator, name)
{

}

/** Overloads the erase background event to prevent flickering */
void WxPropertyItemButton::OnEraseBackground(wxEraseEvent &Event)
{

}

/*-----------------------------------------------------------------------------
	UPropertyInputProxy private interface
-----------------------------------------------------------------------------*/

// Adds a button to the right side of the item window.
void UPropertyInputProxy::AddButton( WxPropertyWindow_Base* InItem, ePropButton InButton, const wxRect& InRC )
{
	if ( GUnrealEd )
	{
		WxMaskedBitmap* bmp = NULL;
		FString ToolTip;
		INT ID = 0;

		switch( InButton )
		{
			case PB_Add:
				bmp = &GPropertyWindowManager->Prop_AddNewItemB;
				ToolTip = *LocalizeUnrealEd("AddNewItem");
				ID = ID_PROP_PB_ADD;
				break;

			case PB_Empty:
				bmp = &GPropertyWindowManager->Prop_RemoveAllItemsFromArrayB;
				ToolTip = *LocalizeUnrealEd("RemoveAllItemsFromArray");
				ID = ID_PROP_PB_EMPTY;
				break;

			case PB_Insert:
				bmp = &GPropertyWindowManager->Prop_InsertNewItemHereB;
				ToolTip = *LocalizeUnrealEd("InsertNewItemHere");
				ID = ID_PROP_PB_INSERT;
				break;

			case PB_Delete:
				bmp = &GPropertyWindowManager->Prop_DeleteItemB;
				ToolTip = *LocalizeUnrealEd("DeleteItem");
				ID = ID_PROP_PB_DELETE;
				break;

			case PB_Browse:
				bmp = &GPropertyWindowManager->Prop_ShowGenericBrowserB;
				ToolTip = *LocalizeUnrealEd("ShowGenericBrowser");
				ID = ID_PROP_PB_BROWSE;
				break;

			case PB_Pick:
				bmp = &GPropertyWindowManager->Prop_UseMouseToPickColorB;
				ToolTip = *LocalizeUnrealEd("UseMouseToPickColorFromViewport");
				ID = ID_PROP_PB_PICK;
				break;

			case PB_Clear:
				bmp = &GPropertyWindowManager->Prop_ClearAllTextB;
				ToolTip = *LocalizeUnrealEd("ClearAllText");
				ID = ID_PROP_PB_CLEAR;
				break;

			case PB_Find:
				bmp = &GPropertyWindowManager->Prop_UseMouseToPickActorB;
				ToolTip = *LocalizeUnrealEd("UseMouseToPickActorFromViewport");
				ID = ID_PROP_PB_FIND;
				break;

			case PB_Use:
				bmp = &GPropertyWindowManager->Prop_UseCurrentBrowserSelectionB;
				ToolTip = *LocalizeUnrealEd("UseCurrentSelectionInBrowser");
				ID = ID_PROP_PB_USE;
				break;

			case PB_NewObject:
				bmp = &GPropertyWindowManager->Prop_NewObjectB;
				ToolTip = *LocalizeUnrealEd("CreateANewObject");
				ID = ID_PROP_PB_NEWOBJECT;
				break;

			case PB_Duplicate:
				bmp = &GPropertyWindowManager->Prop_DuplicateB;
				ToolTip = *LocalizeUnrealEd("DuplicateThisItem");
				ID = ID_PROP_PB_DUPLICATE;
				break;

			default:
				checkMsg( 0, "Unknown button type" );
				break;
		}

		// Create a new button and add it to the button array.
		WxPropertyItemButton* button = new WxPropertyItemButton( InItem, ID, *bmp );
		button->SetToolTip( *ToolTip );
		Buttons.AddItem( button );

		// Notify the parent.
		ParentResized( InItem, InRC );
	}
}

// Creates any controls that are needed to edit the property.
void UPropertyInputProxy::CreateButtons( WxPropertyWindow_Base* InItem, const wxRect& InRC )
{
	// Clear out existing buttons.
	DeleteButtons();

	// Don't create buttons in game.
	if ( !GUnrealEd )
	{
		return;
	}

	// If no property is bound, don't create any buttons.
	if( !InItem->Property )
	{
		return;
	}

	// If property is const, don't create any buttons.
	if( IS_EDITCONST( InItem->Property ) )
	{
		return;
	}

	// If the property is an item of a const array, don't create any buttons.
	const UArrayProperty* ArrayProp = Cast<UArrayProperty>( InItem->Property->GetOuter() );
	if( ArrayProp && IS_EDITCONST( ArrayProp ) )
	{
		return;
	}

	//////////////////////////////
	// Handle an array property.

	if( InItem->Property->IsA(UArrayProperty::StaticClass() ) )
	{
		if( !(InItem->Property->PropertyFlags & CPF_EditFixedSize) )
		{
			AddButton( InItem, PB_Add, InRC );
			AddButton( InItem, PB_Empty, InRC );
		}
	}

	if( ArrayProp )
	{
		if( InItem->bSingleSelectOnly && !(ArrayProp->PropertyFlags & CPF_EditFixedSize) )
		{
			AddButton( InItem, PB_Insert, InRC );
			AddButton( InItem, PB_Delete, InRC );
			AddButton( InItem, PB_Duplicate, InRC );
		}

		// If this is coming from an array property and we are using a combobox for editing,
		// we don't want any other buttons beyond these two.
		if( ComboBox != NULL )
		{
			return;
		}
	}

	//////////////////////////////
	// If the property is a color, add a color picker.

	if( Cast<UStructProperty>(InItem->Property,CLASS_IsAUStructProperty) &&
		(Cast<UStructProperty>(InItem->Property,CLASS_IsAUStructProperty)->Struct->GetFName() == NAME_Color || Cast<UStructProperty>(InItem->Property,CLASS_IsAUStructProperty)->Struct->GetFName() == NAME_LinearColor) )
	{
		AddButton( InItem, PB_Pick, InRC );
		AddButton( InItem, PB_Browse, InRC );
	}

	//////////////////////////////
	// Handle an object property.

	if( InItem->Property->IsA(UObjectProperty::StaticClass()) )
	{
		TArray<BYTE*> ReadAddresses;
		// Only add buttons if read addresses are all NULL or non-NULL.
		if ( InItem->GetReadAddress( InItem, FALSE, ReadAddresses, FALSE ) )
		{
			if( InItem->bEditInline )
			{
				if ( !InItem->bEditInlineUse )
				{
					// editinline objects can be created at runtime from within the property window
					AddButton( InItem, PB_NewObject, InRC );
				}

				// hmmm, seems like this code could be removed and the code inside the 'if <UClassProperty>' check
				// below could be moved outside the else....but is there a reason to allow class properties to have the
				// following buttons if the class property is marked 'editinline' (which is effectively what this logic is doing)
				if( !(InItem->Property->PropertyFlags & CPF_NoClear) )
				{
					AddButton( InItem, PB_Clear, InRC );
				}

				// this object could also be assigned to some resource from a content package
				if ( InItem->bEditInlineUse )
				{
					// add a button for displaying the generic browser
					AddButton( InItem, PB_Browse, InRC );

					// add button for filling the value of this item with the selected object from the GB
					AddButton( InItem, PB_Use, InRC );
				}
			}
			else
			{
				// ignore class properties
				if( Cast<UClassProperty>( InItem->Property ) == NULL )
				{
					// reference to object resource that isn't dynamically created (i.e. some content package)
					if( !(InItem->Property->PropertyFlags & CPF_NoClear) )
					{
						// add button to clear the text
						AddButton( InItem, PB_Clear, InRC );
					}

					//@fixme ?? this will always evaluate to true (!InItem->bEditInline)
					if( !InItem->bEditInline || InItem->bEditInlineUse )
					{
						// add button to display the generic browser
						AddButton( InItem, PB_Browse, InRC );

						// add button for filling the value of this item with the selected object from the GB
						AddButton( InItem, PB_Use, InRC );
					}
				}
			}
		}
	}
}

// Deletes any buttons that were created.
void UPropertyInputProxy::DeleteButtons()
{
	for( INT x = 0 ; x < Buttons.Num() ; ++x )
	{
		Buttons(x)->Destroy();
	}

	Buttons.Empty();
}

/*-----------------------------------------------------------------------------
	UPropertyInputProxy public interface
-----------------------------------------------------------------------------*/

/**
 * @return	Returns the height required by the input proxy.
 */
INT UPropertyInputProxy::GetProxyHeight() const
{
	return PROP_DefaultItemHeight;
}

// Allows the created controls to react to the parent window being resized.
void UPropertyInputProxy::ParentResized( WxPropertyWindow_Base* InItem, const wxRect& InRC )
{
	INT XPos = InRC.x + InRC.GetWidth() - GetButtonSize();
	INT YPos = InRC.height / 2 - GetButtonSize() / 2;

	for( INT x = 0 ; x < Buttons.Num() ; ++x )
	{
		Buttons(x)->SetSize(XPos, YPos, GetButtonSize(), GetButtonSize());
		XPos -= GetButtonSize();
	}
}

// Sends a text string to the selected objects.
void UPropertyInputProxy::SendTextToObjects( WxPropertyWindow_Base* InItem, const FString& InText )
{
	WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();

	// If more than one object is selected, an empty field indicates their values for this property differ.
	// Don't send it to the objects value in this case (if we did, they would all get set to None which isn't good).
	if( ObjectNode->GetNumObjects() > 1 && !InText.Len() )
	{
		return;
	}

	// Build up a list of objects to modify.
	TArray<FObjectBaseAddress> ObjectsToModify;
	for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
	{
		UObject*	Object = *Itor;
		BYTE*		Addr = InItem->GetBase( (BYTE*) Object );
		ObjectsToModify.AddItem( FObjectBaseAddress( Object, Addr ) );
	}

	const FString Value( InText );
	ImportText( InItem, ObjectsToModify, Value );
}

void UPropertyInputProxy::ImportText(WxPropertyWindow_Base* InItem,
									 const TArray<FObjectBaseAddress>& InObjects,
									 const FString& InValue)
{
	TArray<FString> Values;
	for ( INT ObjectIndex = 0 ; ObjectIndex < InObjects.Num() ; ++ObjectIndex )
	{
		Values.AddItem( InValue );
	}
	ImportText( InItem, InObjects, Values );
}

void UPropertyInputProxy::ImportText(WxPropertyWindow_Base* InItem,
									 const TArray<FObjectBaseAddress>& InObjects,
									 const TArray<FString>& InValues)
{
	////////////////
	FReloadObjectArc* OldMemoryArchive = GMemoryArchive;
	FArchetypePropagationArc* PropagationArchive;
	GMemoryArchive = PropagationArchive = new FArchetypePropagationArc();

	UWorld* OldGWorld = NULL;

	// If the object we are modifying is in the PIE world, than make the PIE world the active
	// GWorld.  Assumes all objects managed by this property window belong to the same world.
	if ( GUnrealEd && GUnrealEd->PlayWorld && InObjects(0).Object->IsIn(GUnrealEd->PlayWorld))
	{
		OldGWorld = SetPlayInEditorWorld(GUnrealEd->PlayWorld);
	}
	///////////////

	// Send the values and assemble a list of preposteditchange values.
	TArray<FString> Befores;
	UBOOL bNotifiedPreChange = FALSE;
	UObject *NotifiedObj = NULL;

	for ( INT ObjectIndex = 0 ; ObjectIndex < InObjects.Num() ; ++ObjectIndex )
	{	
		const FObjectBaseAddress& Cur = InObjects( ObjectIndex );

		// Cache the value of the property before modifying it.
		FString PreviousValue;
		InItem->Property->ExportText( 0, PreviousValue, Cur.BaseAddress - InItem->Property->Offset, Cur.BaseAddress - InItem->Property->Offset, NULL, PPF_Localized );

		// Check if we need to call PreEditChange on all objects.
		const TCHAR* NewValue = *InValues(ObjectIndex);
		if ( !bNotifiedPreChange && appStrcmp( NewValue, *PreviousValue ) )
		{
			bNotifiedPreChange = TRUE;
			NotifiedObj = Cur.Object;
			InItem->NotifyPreChange( InItem->Property, Cur.Object );
		}

		// Set the new value.
		InItem->Property->ImportText( NewValue, Cur.BaseAddress, PPF_Localized, Cur.Object );

		// Cache the value of the property after having modified it.
		FString ValueAfterImport;
		InItem->Property->ExportText( 0, ValueAfterImport, Cur.BaseAddress - InItem->Property->Offset, Cur.BaseAddress - InItem->Property->Offset, NULL, PPF_Localized );
		Befores.AddItem( ValueAfterImport );

		// If the values before and after setting the property differ, mark the object dirty.
		if ( appStrcmp( *PreviousValue, *ValueAfterImport ) != 0 )
		{
			Cur.Object->MarkPackageDirty();
		}

		// I apologize for the nasty #ifdefs, this is just to keep multiple versions of the code until we decide on one
#ifdef PROPAGATE_EXACT_PROPERTY_ONLY
		// tell the propagator to pass the change along (InData - InParent should be the byte offset into the object of the actual property)
		GObjectPropagator->OnPropertyChange(Cur.Object, InProperty, Cur.BaseAddress - InItem->Property->Offset);
#else
		// we need to go up to the highest property that we can and send the entire contents (whole struct, array, etc)
		WxPropertyWindow_Base* Item = InItem;
		while (Item->ParentItem->Property)
		{
			Item = Item->ParentItem;
		}
		GObjectPropagator->OnPropertyChange(Cur.Object, Item->Property, Item->Property->Offset);
#endif
	}

	GMemoryArchive->ActivateReader();

	// Note the current property window so that CALLBACK_ObjectPropertyChanged
	// doesn't destroy the window out from under us.
	if ( GApp )
	{
		GApp->CurrentPropertyWindow = InItem->GetPropertyWindow();
	}

	// If PreEditChange was called, so should PostEditChange.
	if ( bNotifiedPreChange )
	{
		// Call PostEditChange on all objects.
		InItem->NotifyPostChange( InItem->Property, NotifiedObj );
	}

	// Unset, effectively making this property window updatable by CALLBACK_ObjectPropertyChanged.
	if ( GApp )
	{
		GApp->CurrentPropertyWindow = NULL;
	}

	// If PreEditChange was called, so was PostEditChange.
	if ( bNotifiedPreChange )
	{
		// Look at values after PostEditChange and refresh the property window as necessary.
		for ( INT ObjectIndex = 0 ; ObjectIndex < InObjects.Num() ; ++ObjectIndex )
		{	
			const FObjectBaseAddress& Cur = InObjects( ObjectIndex );

			FString After;
			InItem->Property->ExportText( 0, After, Cur.BaseAddress - InItem->Property->Offset, Cur.BaseAddress - InItem->Property->Offset, NULL, PPF_Localized );

			if( Befores(ObjectIndex) != After || !Cast<UPropertyInputRotation>( this ) )
			{
				RefreshControlValue( InItem->Property, Cur.BaseAddress );
			}
		}
	}


	// if GMemoryArchive is still pointing to the one we created, restore it to the previous value
	if ( GMemoryArchive == PropagationArchive )
	{
		GMemoryArchive = OldMemoryArchive;
	}

	// clean up the FArchetypePropagationArc we created
	delete PropagationArchive;
	PropagationArchive = NULL;

	if (OldGWorld)
	{
		// restore the original (editor) GWorld
		RestoreEditorWorld( OldGWorld );
	}

	// Redraw
	GCallbackEvent->Send( CALLBACK_RedrawAllViewports );
}

void UPropertyInputProxy::ImportText(WxPropertyWindow_Base* InItem,
									 UProperty* InProperty,
									 const TCHAR* InBuffer,
									 BYTE* InData,
									 UObject* InParent)
{
	FReloadObjectArc* OldMemoryArchive = GMemoryArchive;
	FArchetypePropagationArc* PropagationArchive;
	GMemoryArchive = PropagationArchive = new FArchetypePropagationArc();

	UWorld* OldGWorld = NULL;
	// if the object we are modifying is in the PIE world, than make the PIE world
	// the active GWorld
	if ( GUnrealEd && GUnrealEd->PlayWorld && InParent->IsIn(GUnrealEd->PlayWorld))
	{
		OldGWorld = SetPlayInEditorWorld(GUnrealEd->PlayWorld);
	}

	InItem->NotifyPreChange( InProperty, InParent );

	GMemoryArchive->ActivateReader();

	// Cache the value of the property before modifying it.
	FString PreviousValue;
	InProperty->ExportText( 0, PreviousValue, InData - InProperty->Offset, InData - InProperty->Offset, NULL, PPF_Localized );

	// Set the new value.
	InProperty->ImportText( InBuffer, InData, PPF_Localized, InParent );

	// Cache the value of the property after having modified it.
	FString ValueAfterImport;
	InProperty->ExportText( 0, ValueAfterImport, InData - InProperty->Offset, InData - InProperty->Offset, NULL, PPF_Localized );

	// If the values before and after setting the property differ, mark the object dirty.
	if ( appStrcmp( *PreviousValue, *ValueAfterImport ) != 0 )
	{
		InParent->MarkPackageDirty();
	}

	// Note the current property window so that CALLBACK_ObjectPropertyChanged
	// doesn't destroy the window out from under us.
	if ( GApp )
	{
		GApp->CurrentPropertyWindow = InItem->GetPropertyWindow();
	}

	InItem->NotifyPostChange( InProperty, InParent );

	// Unset, effectively making this property window updatable by CALLBACK_ObjectPropertyChanged.
	if ( GApp )
	{
		GApp->CurrentPropertyWindow = NULL;
	}

	// I apologize for the nasty #ifdefs, this is just to keep multiple versions of the code until we decide on one
#ifdef PROPAGATE_EXACT_PROPERTY_ONLY
    // tell the propagator to pass the change along (InData - InParent should be the byte offset into the object of the actual property)
	GObjectPropagator->OnPropertyChange(InParent, InProperty, InData - (BYTE*)InParent);
#else
	// we need to go up to the highest property that we can and send the entire contents (whole struct, array, etc)
	WxPropertyWindow_Base* Item = InItem;
	while (Item->ParentItem->Property)
	{
		Item = Item->ParentItem;
	}
	GObjectPropagator->OnPropertyChange(InParent, Item->Property, Item->Property->Offset);
#endif

	FString After;
	InProperty->ExportText( 0, After, InData - InProperty->Offset, InData - InProperty->Offset, NULL, PPF_Localized );

	if( ValueAfterImport != After || !Cast<UPropertyInputRotation>( this ) )
	{
		RefreshControlValue( InProperty, InData );
	}

	// if GMemoryArchive is still pointing to the one we created, restore it to the previous value
	if ( GMemoryArchive == PropagationArchive )
	{
		GMemoryArchive = OldMemoryArchive;
	}

	// clean up the FArchetypePropagationArc we created
	delete PropagationArchive;
	PropagationArchive = NULL;

	if (OldGWorld)
	{
		// restore the original (editor) GWorld
		RestoreEditorWorld( OldGWorld );
	}

	GCallbackEvent->Send( CALLBACK_RedrawAllViewports );

	//@todo: need a clean way to notify archetype actor property windows to update value of property
	// when value in archetype is modified!
}

/**
 * Called when the "Use selected" property item button is clicked.
 */
void UPropertyInputProxy::OnUseSelected(WxPropertyWindow_Base* InItem)
{
	UObjectProperty* ObjProp = Cast<UObjectProperty>( InItem->Property );
	if( ObjProp )
	{
		UClass* ObjPropClass = ObjProp->PropertyClass;
		USelection* SelectedSet = GEditor->GetSelectedSet( ObjPropClass );

		// If an object of that type is selected, use it.
		UObject* SelectedObject = SelectedSet->GetTop( ObjPropClass );
		if( SelectedObject )
		{
			const FString SelectedObjectPathName = SelectedObject->GetPathName();
			const FString CompareName = FString::Printf( TEXT("%s'%s'"), *SelectedObject->GetClass()->GetName(), *SelectedObjectPathName );
			SendTextToObjects( InItem, *SelectedObjectPathName );
			InItem->Refresh();

			// Read values back and report any failures.
			TArray<FObjectBaseAddress> ObjectsThatWereModified;
			WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();
			UBOOL bAllObjectPropertyValuesMatch = TRUE;
			for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
			{
				UObject*	Object = *Itor;
				BYTE*		Addr = InItem->GetBase( (BYTE*) Object );
				FObjectBaseAddress Cur( Object, Addr );
				FString PropertyValue;
				InItem->Property->ExportText( 0, PropertyValue, Cur.BaseAddress - InItem->Property->Offset, Cur.BaseAddress - InItem->Property->Offset, NULL, PPF_Localized );
				if ( PropertyValue != CompareName )
				{
					bAllObjectPropertyValuesMatch = FALSE;
					break;
				}
			}

			// Warn that some object assignments failed.
			if ( !bAllObjectPropertyValuesMatch )
			{
				appMsgf( AMT_OK, *LocalizeUnrealEd("ObjectAssignmentsFailedCrossPackageRefs") );
			}
		}
	}
}

/*-----------------------------------------------------------------------------
	UPropertyInputBool
-----------------------------------------------------------------------------*/

/**
 * Wrapper method for determining whether a class is valid for use by this property item input proxy.
 *
 * @param	InItem			the property window item that contains this proxy.
 * @param	CheckClass		the class to verify
 * @param	bAllowAbstract	TRUE if abstract classes are allowed
 *
 * @return	TRUE if CheckClass is valid to be used by this input proxy
 */
UBOOL UPropertyInputArrayItemBase::IsClassAllowed( WxPropertyWindow_Base* InItem,  UClass* CheckClass, UBOOL bAllowAbstract ) const
{
	check(CheckClass);

	return !CheckClass->HasAnyClassFlags(CLASS_Hidden|CLASS_HideDropDown|CLASS_Deprecated)
		&&	(bAllowAbstract || !CheckClass->HasAnyClassFlags(CLASS_Abstract));
}

void UPropertyInputArrayItemBase::ButtonClick( WxPropertyWindow_Base* InItem, ePropButton InButton )
{
	switch( InButton )
	{
		case PB_Insert:
			{
				TArray<BYTE*> Addresses;
				FScriptArray* Addr = NULL;
				if ( InItem->ParentItem->GetReadAddress( InItem->ParentItem, InItem->ParentItem->bSingleSelectOnly, Addresses ) )
				{
					Addr = (FScriptArray*)Addresses(0);
				}

				if( Addr )
				{
					// Create an FArchetypePropagationArc to propagate the updated property values from archetypes to instances of that archetype
					FReloadObjectArc* OldMemoryArchive = GMemoryArchive;
					FArchetypePropagationArc* PropagationArchive;
					GMemoryArchive = PropagationArchive = new FArchetypePropagationArc();

					WxPropertyWindow_Base* SaveParentItem = InItem->ParentItem;
					SaveParentItem->NotifyPreChange( SaveParentItem->Property );

					// now change the propagation archive to read mode
					PropagationArchive->ActivateReader();

					Addr->Insert( InItem->ArrayIndex, 1, InItem->Property->GetSize() );	
					BYTE* Dest = (BYTE*)Addr->GetData() + InItem->ArrayIndex*InItem->Property->GetSize();
					appMemzero( Dest, InItem->Property->GetSize() );

					// Apply struct defaults.
					UStructProperty* StructProperty	= Cast<UStructProperty>( InItem->Property,CLASS_IsAUStructProperty);
					if( StructProperty )
					{
						UScriptStruct* InnerStruct = StructProperty->Struct;
						check(InnerStruct);

						// if this struct has defaults, copy them over
						if ( InnerStruct->GetDefaultsCount() && StructProperty->HasValue(InnerStruct->GetDefaults()) )
						{
							check( Align(InnerStruct->GetDefaultsCount(),InnerStruct->MinAlignment) == InItem->Property->GetSize() );
							StructProperty->CopySingleValue( Dest, InnerStruct->GetDefaults() );
						}
					}

					SaveParentItem->Collapse();
					SaveParentItem->Expand();
					SaveParentItem->NotifyPostChange( SaveParentItem->Property );

					// if GMemoryArchive is still pointing to the one we created, restore it to the previous value
					if ( GMemoryArchive == PropagationArchive )
					{
						GMemoryArchive = OldMemoryArchive;
					}

					// clean up the FArchetypePropagationArc we created
					delete PropagationArchive;
					PropagationArchive = NULL;
				}

			}
			break;

		case PB_Delete:
			{
				TArray<BYTE*> Addresses;
				if ( InItem->ParentItem->GetReadAddress( InItem->ParentItem, InItem->ParentItem->bSingleSelectOnly, Addresses ) )
				{
					// Create an FArchetypePropagationArc to propagate the updated property values from archetypes to instances of that archetype
					FReloadObjectArc* OldMemoryArchive = GMemoryArchive;
					FArchetypePropagationArc* PropagationArchive;
					GMemoryArchive = PropagationArchive = new FArchetypePropagationArc();

					WxPropertyWindow_Base* SaveParentItem = InItem->ParentItem;
					SaveParentItem->NotifyPreChange( SaveParentItem->Property );

					// perform the operation on the array for all selected objects
					for ( INT i = 0 ; i < Addresses.Num() ; ++i )
					{
						FScriptArray* Addr = (FScriptArray*)Addresses(i);
						Addr->Remove( InItem->ArrayIndex, 1, InItem->Property->GetSize() );
					}

					// now change the propagation archive to read mode
					PropagationArchive->ActivateReader();

					SaveParentItem->Collapse();
					SaveParentItem->Expand();
					SaveParentItem->NotifyPostChange( SaveParentItem->Property );

					// if GMemoryArchive is still pointing to the one we created, restore it to the previous value
					if ( GMemoryArchive == PropagationArchive )
					{
						GMemoryArchive = OldMemoryArchive;
					}

					// clean up the FArchetypePropagationArc we created
					delete PropagationArchive;
					PropagationArchive = NULL;
				}
			}
			break;

		case PB_Duplicate:
			{
				TArray<BYTE*> Addresses;
				FScriptArray* Addr = NULL;
				if ( InItem->ParentItem->GetReadAddress( InItem->ParentItem, InItem->ParentItem->bSingleSelectOnly, Addresses ) )
				{
					Addr = (FScriptArray*)Addresses(0);
				}

				if( Addr )
				{
					// Create an FArchetypePropagationArc to propagate the updated property values from archetypes to instances of that archetype
					FReloadObjectArc* OldMemoryArchive = GMemoryArchive;
					FArchetypePropagationArc* PropagationArchive;
					GMemoryArchive = PropagationArchive = new FArchetypePropagationArc();

					WxPropertyWindow_Base* SaveParentItem = InItem->ParentItem;
					SaveParentItem->NotifyPreChange( SaveParentItem->Property );

					// now change the propagation archive to read mode
					PropagationArchive->ActivateReader();

					Addr->Insert( InItem->ArrayIndex, 1, InItem->Property->GetSize() );	
					BYTE* Dest = (BYTE*)Addr->GetData() + InItem->ArrayIndex*InItem->Property->GetSize();
					appMemzero( Dest, InItem->Property->GetSize() );

					// Copy the selected item's value to the new item.
					// Find the object that owns the array and pass it to the SubObjectRoot and DestObject arguments
					// of CopyCompleteValue so that the instanced objects will be correctly duplicated.
					WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();
					UObject* ArrayOwner = NULL;
					for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor && !ArrayOwner ; ++Itor )
					{
						ArrayOwner = *Itor;
					}
					InItem->Property->CopyCompleteValue(Dest, (BYTE*)Addr->GetData() + (InItem->ArrayIndex + 1) * InItem->Property->GetSize(), ArrayOwner, ArrayOwner);

					SaveParentItem->Collapse();
					SaveParentItem->Expand();
					SaveParentItem->NotifyPostChange( SaveParentItem->Property );

					// if GMemoryArchive is still pointing to the one we created, restore it to the previous value
					if ( GMemoryArchive == PropagationArchive )
					{
						GMemoryArchive = OldMemoryArchive;
					}

					// clean up the FArchetypePropagationArc we created
					delete PropagationArchive;
					PropagationArchive = NULL;
				}

			}
			break;

		default:
			Super::ButtonClick( InItem, InButton );
			break;
	}
}

/*-----------------------------------------------------------------------------
	UPropertyInputBool
-----------------------------------------------------------------------------*/

// Determines if this input proxy supports the specified UProperty.
UBOOL UPropertyInputBool::Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const
{
	if( IS_EDITCONST( InItem->Property ) )
	{
		return FALSE;
	}

	return ( Cast<UBoolProperty>(InItem->Property) != NULL );
}

// Allows a customized response to a user double click on the item in the property window.
void UPropertyInputBool::DoubleClick(WxPropertyWindow_Base* InItem, const TArray<FObjectBaseAddress>& InObjects, const TCHAR* InValue, UBOOL bForceValue)
{
	if ( InObjects.Num() == 0 )
	{
		return;
	}

	if ( InItem->GetPropertyWindow() && InItem->GetPropertyWindow()->IsReadOnly() )
	{
		return;
	}

	// Assemble a list of target values.
	TArray<FString> Results;
	const FString ForceResult( InValue );

	for ( INT ObjectIndex = 0 ; ObjectIndex < InObjects.Num() ; ++ObjectIndex )
	{	
		if( bForceValue )
		{
			Results.AddItem( ForceResult );
		}
		else
		{
			const FObjectBaseAddress& Cur = InObjects( ObjectIndex );
			FString TempResult;

			InItem->Property->ExportTextItem( TempResult, Cur.BaseAddress, NULL, NULL, 0 );

			if( TempResult == TEXT("True") || TempResult == GTrue )
			{
				TempResult = TEXT("False");
			}
			else
			{
				TempResult = TEXT("True");
			}
			Results.AddItem( TempResult );
		}
	}

	ImportText( InItem, InObjects, Results );
}

// Allows special actions on left mouse clicks.  Return TRUE to stop normal processing.
UBOOL UPropertyInputBool::LeftClick( WxPropertyWindow_Base* InItem, INT InX, INT InY )
{
	if (InItem->GetPropertyWindow() && InItem->GetPropertyWindow()->IsReadOnly())
		return TRUE;

	// If clicking on top of the checkbox graphic, toggle it's value and return.
	InX -= InItem->GetPropertyWindow()->GetSplitterPos();

	if( InX > 0 && InX < GPropertyWindowManager->CheckBoxOnB.GetWidth() )
	{
		FString Value = InItem->GetPropertyText();
		UBOOL bForceValue = 0;
		if( !Value.Len() )
		{
			Value = GTrue;
			bForceValue = 1;
		}

		TArray<FObjectBaseAddress> ObjectsToModify;

		WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();
		for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
		{
			UObject*	Object = *Itor;
			BYTE*		Addr = InItem->GetBase( (BYTE*) Object );
			ObjectsToModify.AddItem( FObjectBaseAddress( Object, Addr ) );
		}

		DoubleClick( InItem, ObjectsToModify, *Value, bForceValue );
		//DoubleClick( InItem, Object, InItem->GetBase( (BYTE*) Object ), *Value, bForceValue );

		InItem->Refresh();

		return 1;
	}

	return 0;
}

/*-----------------------------------------------------------------------------
	UPropertyInputColor
-----------------------------------------------------------------------------*/

/**
* Generates a text string based from the color passed in, based on the color type of the property passed in.
*
* @param InItem	Property that we are basing our string type off of.
* @param InColor	Color to use to generate the string.
* @param OutString Output for generated string.
*/
void UPropertyInputColor::GenerateColorString( const WxPropertyWindow_Base* InItem, const FColor &InColor, FString& OutString )
{
	if( Cast<UStructProperty>(InItem->Property,CLASS_IsAUStructProperty)->Struct->GetFName() == NAME_Color )
	{
		OutString = FString::Printf( TEXT("(R=%i,G=%i,B=%i)"), (INT)InColor.R, (INT)InColor.G, (INT)InColor.B );
	}
	else
	{
		check( Cast<UStructProperty>(InItem->Property,CLASS_IsAUStructProperty)->Struct->GetFName() == NAME_LinearColor );
		const FLinearColor lc( InColor );
		OutString = FString::Printf( TEXT("(R=%f,G=%f,B=%f)"), lc.R, lc.G, lc.B );
	}
}

// Determines if this input proxy supports the specified UProperty.
UBOOL UPropertyInputColor::Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const
{
	if( IS_EDITCONST( InItem->Property ) )
	{
		return FALSE;
	}

	return ( Cast<UStructProperty>(InItem->Property,CLASS_IsAUStructProperty) && (Cast<UStructProperty>(InItem->Property,CLASS_IsAUStructProperty)->Struct->GetFName()==NAME_Color || Cast<UStructProperty>(InItem->Property,CLASS_IsAUStructProperty)->Struct->GetFName()==NAME_LinearColor) );
}

// Handles button clicks from within the item.
void UPropertyInputColor::ButtonClick( WxPropertyWindow_Base* InItem, ePropButton InButton )
{
	FColor	PrevColor;

	if(Cast<UStructProperty>(InItem->Property,CLASS_IsAUStructProperty)->Struct->GetFName() == NAME_Color)
	{
		TArray<BYTE*> Addresses;
		if ( InItem->GetReadAddress(InItem,InItem->bSingleSelectOnly,Addresses) )
		{
			PrevColor = *(FColor*)Addresses(0);
		}
	}
	else
	{
		check( Cast<UStructProperty>(InItem->Property,CLASS_IsAUStructProperty)->Struct->GetFName() == NAME_LinearColor );
		TArray<BYTE*> Addresses;
		if ( InItem->GetReadAddress(InItem,InItem->bSingleSelectOnly,Addresses) )
		{
			PrevColor = FColor(*(FLinearColor*)Addresses(0));
		}
	}

	switch( InButton )
	{
		case PB_Browse:
		{
			// Disable color pick mode if we hit the browse button.
			InItem->GetPropertyWindow()->SetColorPickModeEnabled( FALSE );
			GEditor->SetColorPickModeEnabled( FALSE );

			// Initialize the color data for the picker window.
			wxColourData ColorData;
			ColorData.SetChooseFull( true );
			ColorData.SetColour(wxColour(PrevColor.R,PrevColor.G,PrevColor.B));

			// Get the parent object before opening the picker dialog, in case the dialog deselects.
			WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();

			wxWindow* TopWindow = InItem->GetPropertyWindow()->GetParent();
			WxDlgColor ColorDialog;
			ColorDialog.Create(TopWindow, &ColorData );
			if( ColorDialog.ShowModal() == wxID_OK)
			{
				const wxColour clr = ColorDialog.GetColourData().GetColour();
				const FColor SourceColor( clr.Red(), clr.Green(), clr.Blue() );
				FString Value;

				GenerateColorString( InItem, SourceColor, Value );
				
				for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
				{
					ImportText(InItem, InItem->Property, *Value, InItem->GetBase((BYTE*)(*Itor)), *Itor );
				}
			}
		
			InItem->Refresh();
		}
		break;

		case PB_Pick:
		{
			InItem->GetPropertyWindow()->SetColorPickModeEnabled( TRUE );
			GEditor->SetColorPickModeEnabled( TRUE );
		}
		break;

		default:
			Super::ButtonClick( InItem, InButton );
			break;
	}
}

/*-----------------------------------------------------------------------------
	UPropertyInputArray
-----------------------------------------------------------------------------*/

// Determines if this input proxy supports the specified UProperty.
UBOOL UPropertyInputArray::Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const
{
	if( IS_EDITCONST( InItem->Property ) )
	{
		return FALSE;
	}

	return ( ( InItem->Property->ArrayDim != 1 && InArrayIdx == -1 ) || Cast<UArrayProperty>(InItem->Property) );
}

// Handles button clicks from within the item.
void UPropertyInputArray::ButtonClick( WxPropertyWindow_Base* InItem, ePropButton InButton )
{
	switch( InButton )
	{
		case PB_Empty:
		{
			WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();

			TArray<BYTE*> Addresses;
			const UBOOL bSuccess = InItem->GetReadAddress( InItem, InItem->bSingleSelectOnly, Addresses,
															TRUE, //bComparePropertyContents
															FALSE, //bObjectForceCompare
															TRUE ); //bArrayPropertiesCanDifferInSize
			if ( bSuccess )
			{
				// determines whether we actually changed any values (if the user clicks the "emtpy" button when the array is already empty,
				// we don't want the objects to be marked dirty)
				UBOOL bNotifiedPreChange = FALSE;

				// these archives are used to propagate property value changes from archetypes to instances of that archetype
				FReloadObjectArc* OldMemoryArchive = GMemoryArchive;
				FArchetypePropagationArc* PropagationArchive = NULL;

				UArrayProperty* Array = CastChecked<UArrayProperty>( InItem->Property );
				for ( INT i = 0 ; i < Addresses.Num() ; ++i )
				{
					BYTE* Addr = Addresses(i);
					if( Addr )
					{
						if ( !bNotifiedPreChange )
						{
							bNotifiedPreChange = TRUE;

							// Create an FArchetypePropagationArc to propagate the updated property values from archetypes to instances of that archetype
							GMemoryArchive = PropagationArchive = new FArchetypePropagationArc();

							// send the PreEditChange notification to all selected objects
							InItem->NotifyPreChange( InItem->Property );

							// collapse the array items
							InItem->Collapse();
							InItem->GetPropertyWindow()->PositionChildren();
						}

						((FScriptArray*)Addr)->Empty( 0, Array->Inner->ElementSize );
					}
				}

				if ( bNotifiedPreChange )
				{
					// re-expand the property items corresponding to the array elements
					InItem->Expand();

					// now change the propagation archive to read mode
					PropagationArchive->ActivateReader();

					// send the PostEditChange notification; it will be propagated to all selected objects
					InItem->NotifyPostChange( InItem->Property );
				}

				// if GMemoryArchive is still pointing to the one we created, restore it to the previous value
				if ( GMemoryArchive == PropagationArchive )
				{
					GMemoryArchive = OldMemoryArchive;
				}

				// clean up the FArchetypePropagationArc we created
				delete PropagationArchive;
				PropagationArchive = NULL;
			}
		}
		break;

		case PB_Add:
		{
			WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();

			TArray<BYTE*> Addresses;
			const UBOOL bSuccess = InItem->GetReadAddress( InItem, InItem->bSingleSelectOnly, Addresses, TRUE, FALSE, TRUE );
			if ( bSuccess )
			{
				// determines whether we actually changed any values (if the user clicks the "emtpy" button when the array is already empty,
				// we don't want the objects to be marked dirty)
				UBOOL bNotifiedPreChange = FALSE;

				// these archives are used to propagate property value changes from archetypes to instances of that archetype
				FReloadObjectArc* OldMemoryArchive = GMemoryArchive;
				FArchetypePropagationArc* PropagationArchive = NULL;

				UArrayProperty* Array = CastChecked<UArrayProperty>( InItem->Property );
				for ( INT i = 0 ; i < Addresses.Num() ; ++i )
				{
					BYTE* Addr = Addresses(i);
					if( Addr )
					{
						if ( !bNotifiedPreChange )
						{
							bNotifiedPreChange = TRUE;

							// Create an FArchetypePropagationArc to propagate the updated property values from archetypes to instances of that archetype
							GMemoryArchive = PropagationArchive = new FArchetypePropagationArc();
							GMemoryArchive->ActivateWriter();

							// send the PreEditChange notification to all selected objects
							InItem->NotifyPreChange( InItem->Property );

							// collapse the array items
							InItem->Collapse();
						}

						const INT ArrayIndex = ((FScriptArray*)Addr)->AddZeroed( 1, Array->Inner->ElementSize );

						// Apply struct defaults.
						UStructProperty* StructProperty = Cast<UStructProperty>(Array->Inner,CLASS_IsAUStructProperty);
						if( StructProperty && StructProperty->Struct->GetDefaultsCount() && StructProperty->HasValue(StructProperty->Struct->GetDefaults()) )
						{
							check( Align(StructProperty->Struct->GetDefaultsCount(),StructProperty->Struct->MinAlignment) == Array->Inner->ElementSize );
							BYTE* Dest = (BYTE*)((FScriptArray*)Addr)->GetData() + ArrayIndex * Array->Inner->ElementSize;
							StructProperty->CopySingleValue( Dest, StructProperty->Struct->GetDefaults() );
						}
					}
				}

				if ( bNotifiedPreChange )
				{
					// re-expand the property items corresponding to the array elements
					InItem->Expand();

					// now change the propagation archive to read mode
					PropagationArchive->ActivateReader();

					// send the PostEditChange notification; it will be propagated to all selected objects
					InItem->NotifyPostChange( InItem->Property );
				}

				// if GMemoryArchive is still pointing to the one we created, restore it to the previous value
				if ( GMemoryArchive == PropagationArchive )
				{
					GMemoryArchive = OldMemoryArchive;
				}

				// clean up the FArchetypePropagationArc we created
				delete PropagationArchive;
				PropagationArchive = NULL;
			}
		}
		break;

		default:
			Super::ButtonClick( InItem, InButton );
			break;
	}
}

/*-----------------------------------------------------------------------------
	UPropertyInputArrayItem
-----------------------------------------------------------------------------*/

// Determines if this input proxy supports the specified UProperty.
UBOOL UPropertyInputArrayItem::Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const
{
	if( IS_EDITCONST( InItem->Property ) )
	{
		return FALSE;
	}

	return !Cast<UClassProperty>( InItem->Property ) &&
			Cast<UArrayProperty>( InItem->Property->GetOuter() ) &&
			InItem->bSingleSelectOnly &&
			!IS_EDITCONST( Cast<UArrayProperty>(InItem->Property->GetOuter()) );
}

// Handles button clicks from within the item.
void UPropertyInputArrayItem::ButtonClick( WxPropertyWindow_Base* InItem, ePropButton InButton )
{
	switch( InButton )
	{
		case PB_Browse:
		{
			if ( GUnrealEd )
			{
				WxGenericBrowser* Browser = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
				if ( Browser )
				{
					GUnrealEd->GetBrowserManager()->ShowWindow(Browser->GetDockID(),TRUE);
				}
			}
			break;
		}

		case PB_Clear:
			SendTextToObjects( InItem, TEXT("None") );
			InItem->Refresh();
			break;

		case PB_Use:
			OnUseSelected( InItem );
			break;

		default:
			Super::ButtonClick( InItem, InButton );
			break;
	}
}

/*-----------------------------------------------------------------------------
 WxPropertyPanel
-----------------------------------------------------------------------------*/

/**
 * This is a overloaded wxPanel class that passes certain events back up to the wxPropertyWindow_Base parent class.
 */
class WxPropertyPanel : public wxPanel
{
public:
	WxPropertyPanel(WxPropertyWindow_Base* InParent);

protected:
	/** Keypress event handler, just routes the event back up to the parent. */
	void OnChar(wxKeyEvent &Event);

	/** Pointer to the parent property window that created this class. */
	WxPropertyWindow_Base* Parent;

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WxPropertyPanel, wxPanel)
	EVT_CHAR(WxPropertyPanel::OnChar)
END_EVENT_TABLE()

WxPropertyPanel::WxPropertyPanel(WxPropertyWindow_Base* InParent) : 
wxPanel(InParent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER),
Parent(InParent)
{
	
}

/** Keypress event handler, just routes the event back up to the parent. */
void WxPropertyPanel::OnChar(wxKeyEvent &Event)
{
	// Make sure this event gets propagated up to the parent.  We need to use the add pending event function
	// because the parent may destroy this panel in its event handler.  This way the event will be processed
	// later on, when this class can safely be deleted.
	Event.SetId(Parent->GetId());
	Parent->GetEventHandler()->AddPendingEvent(Event);
}

/*-----------------------------------------------------------------------------
	UPropertyInputText::WxPropertyItemTextCtrl
-----------------------------------------------------------------------------*/

IMPLEMENT_DYNAMIC_CLASS(UPropertyInputText::WxPropertyItemTextCtrl, WxTextCtrl);

BEGIN_EVENT_TABLE(UPropertyInputText::WxPropertyItemTextCtrl, WxTextCtrl)
EVT_SET_FOCUS(UPropertyInputText::WxPropertyItemTextCtrl::OnSetFocus)
EVT_KILL_FOCUS(UPropertyInputText::WxPropertyItemTextCtrl::OnKillFocus)
EVT_LEFT_DOWN(UPropertyInputText::WxPropertyItemTextCtrl::OnLeftMouseDown)
EVT_ENTER_WINDOW(UPropertyInputText::WxPropertyItemTextCtrl::OnMouseEnter)
END_EVENT_TABLE()

/**
 * Constructor.
 */
UPropertyInputText::WxPropertyItemTextCtrl::WxPropertyItemTextCtrl()
{

}

/**
 * Constructor.
 */
UPropertyInputText::WxPropertyItemTextCtrl::WxPropertyItemTextCtrl( wxWindow* parent,
					   wxWindowID id,
					   const wxString& value,
					   const wxPoint& pos,
					   const wxSize& size,
					   long style)
					   : WxTextCtrl(parent, id, value, pos, size, style)
{
	wxEvtHandler::Connect(wxID_ANY, ID_SELECT_ALL_TEXT, wxCommandEventHandler(UPropertyInputText::WxPropertyItemTextCtrl::OnSelectAllText));
}

/**
 * Event handler for the focus event. This function maintains proper selection state within the parent property window.
 *
 * @param	Event	Information about the event.
 */
void UPropertyInputText::WxPropertyItemTextCtrl::OnSetFocus(wxFocusEvent &Event)
{
	WxPropertyWindow_Base *ParentItem = (WxPropertyWindow_Base*)GetGrandParent();
	WxPropertyWindow *Window = ParentItem->GetPropertyWindow();

	if(GPropertyWindowManager->GetShowAllItemButtons())
	{
		Window->Freeze();

		// Update the previously focused item.
		if(Window->GetLastFocused())
		{
			Window->GetLastFocused()->InputProxy->SendToObjects(Window->GetLastFocused());
			Window->GetLastFocused()->InputProxy->OnKillFocus(Window->GetLastFocused());
		}

		SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

		// We've received focus, so make sure all text gets selected
		wxCommandEvent TxtEvent;
		TxtEvent.SetEventObject(this);
		TxtEvent.SetEventType(ID_SELECT_ALL_TEXT);
		wxEvtHandler::AddPendingEvent(TxtEvent);

		// Set this node as being in focus.
		Window->SetLastFocused(ParentItem);
		Window->Thaw();
		Window->Refresh();
	}
	else
	{
		SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

		// We've received focus, so make sure all text gets selected
		wxCommandEvent TxtEvent;
		TxtEvent.SetEventObject(this);
		TxtEvent.SetEventType(ID_SELECT_ALL_TEXT);
		wxEvtHandler::AddPendingEvent(TxtEvent);
	}
}

/**
* Event handler for the kill focus event. This function brings the cursor back to index 0 within the text control.
*
* @param	Event	Information about the event.
*/
void UPropertyInputText::WxPropertyItemTextCtrl::OnKillFocus(wxFocusEvent& Event)
{
	SetSelection(0, 0);
}

/**
 * Event handler for the left mouse button down event. This handler is needed because when the text control is clicked and it doesn't have focus it cannot
 * select all of the text in the control due to the fact that the control thinks it is in the middle of a drag/selection operation. This only applies when
 * all property item buttons are being rendered because that's the only time when all text control's are created and visible.
 */
void UPropertyInputText::WxPropertyItemTextCtrl::OnLeftMouseDown(wxMouseEvent &Event)
{
	//If all buttons are being rendered that means the text control already existed
	if(wxWindow::FindFocus() != this && GPropertyWindowManager->GetShowAllItemButtons())
	{
		//so me must not let the base class handle the event because it'll think we're in the middle of a drag/selection operation
		Event.Skip(false);

		//instead set focus ourselves and prevent any further operations
		SetFocus();
	}
	else
	{
		//otherwise just let things continue on as normal
		Event.Skip(true);
	}
}

/**
 * Event handler for the select all text event.
 *
 * @param	Event	Information about the event.
 */
void UPropertyInputText::WxPropertyItemTextCtrl::OnSelectAllText(wxCommandEvent &Event)
{
	SetSelection(-1, -1);
}

/**
 * This event handler sets the default cursor when the mouse cursor enters the text box. This prevents the resize cursor from being permanent until you move the mouse outside of the text box.
 *
 * @param	Event	Information about the event.
 */
void UPropertyInputText::WxPropertyItemTextCtrl::OnMouseEnter(wxMouseEvent &Event)
{
	SetCursor(wxCursor(wxCURSOR_DEFAULT));
}

/*-----------------------------------------------------------------------------
UPropertyInputText
-----------------------------------------------------------------------------*/

/**
* Clears any selection in the text control.
*/
void UPropertyInputText::ClearTextSelection()
{
	if ( TextCtrl )
	{
		TextCtrl->SetSelection( -1, -1 );
	}
}

// Determines if this input proxy supports the specified UProperty.
UBOOL UPropertyInputText::Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const
{
	if( IS_EDITCONST( InItem->Property ) )
	{
		return FALSE;
	}

	if(	!InItem->bEditInline
			&&	( (InItem->Property->IsA(UNameProperty::StaticClass()) && InItem->Property->GetFName() != NAME_InitialState)
			||	InItem->Property->IsA(UStrProperty::StaticClass())
			||	InItem->Property->IsA(UObjectProperty::StaticClass()) )
			&&	!InItem->Property->IsA(UComponentProperty::StaticClass()) )
	{
		return TRUE;
	}

	return FALSE;
}

// Allows the created controls to react to the parent window being resized.  Derived classes
// that implement this method should to call up to their parent.
void UPropertyInputText::CreateControls( WxPropertyWindow_Base* InItem, UClass* InBaseClass, const wxRect& InRC, const TCHAR* InValue )
{
	// Create any buttons for the control.
	UPropertyInputProxy::CreateControls( InItem, InBaseClass, InRC, InValue );

	PropertyPanel = new WxPropertyPanel(InItem);
	PropertySizer = new wxBoxSizer(wxHORIZONTAL);
	{
		// NOTE: We specify wxTE_PROCESS_ENTER and wxTE_PROCESS_TAB so that the edit control will receive
		//    WM_CHAR messages for VK_RETURN and VK_TAB.  This is so that we can use these keys for navigation
		//    between properties, etc.  Without specifying these flags, property windows embedding inside of
		//    dialog frames would not receive WM_CHAR messages for these keys.
		DWORD WindowStyle = wxNO_BORDER | wxTE_PROCESS_TAB;
		if( InItem->GetPropertyWindow() != NULL && InItem->GetPropertyWindow()->AllowEnterKeyToApplyChanges() )
		{
			WindowStyle |= wxTE_PROCESS_ENTER;
		}
		TextCtrl = new WxPropertyItemTextCtrl(PropertyPanel, ID_PROPWINDOW_EDIT, InValue, wxDefaultPosition, wxDefaultSize, WindowStyle);
		TextCtrl->SetSelection(0,0);
		TextCtrl->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
		PropertySizer->Add(TextCtrl,1);
	}
	PropertyPanel->SetSizer(PropertySizer);
	PropertyPanel->SetAutoLayout(true);

	ParentResized( InItem, InRC );
}

// Deletes any controls which were created for editing.
void UPropertyInputText::DeleteControls()
{
	// Delete any buttons on the control.
	UPropertyInputProxy::DeleteControls();

	if( PropertyPanel )
	{
		PropertyPanel->Destroy();
		PropertyPanel = NULL;
		TextCtrl = NULL;
	}
}

// Sends the current value in the input control to the selected objects.
void UPropertyInputText::SendToObjects( WxPropertyWindow_Base* InItem )
{
	if( !TextCtrl )
	{
		return;
	}

	FString Value = (const TCHAR*)TextCtrl->GetValue();

	// Strip any leading underscores and spaces from names.
	if( InItem->Property->IsA( UNameProperty::StaticClass() ) )
	{
		while ( true )
		{
			if ( Value.StartsWith( TEXT("_") ) )
			{
				appMsgf( AMT_OK, *LocalizeUnrealEd("NamesCantBeingWithUnderscore") );
				// Strip leading underscores.
				do
				{
					Value = Value.Right( Value.Len()-1 );
				} while ( Value.StartsWith( TEXT("_") ) );
			}
			else if ( Value.StartsWith( TEXT(" ") ) )
			{
				appMsgf( AMT_OK, *LocalizeUnrealEd("NamesCantBeingWithSpace") );
				// Strip leading spaces.
				do
				{
					Value = Value.Right( Value.Len()-1 );
				} while ( Value.StartsWith( TEXT(" ") ) );
			}
			else
			{
				// Starting with something valid -- break.
				break;
			}
		}

		// Ensure the name is enclosed with quotes.
		if(Value.Len())
		{
			Value = FString::Printf(TEXT("\"%s\""), *Value.TrimQuotes());
		}
	}

	// If more than one object is selected, an empty field indicates their values for this property differ.
	// Don't send it to the objects value in this case (if we did, they would all get set to None which isn't good).
	WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();

	if( ObjectNode->GetNumObjects() == 1 || Value.Len() )
	{
		// Build up a list of objects to modify.
		TArray<FObjectBaseAddress> ObjectsToModify;
		for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
		{
			UObject*	Object = *Itor;
			BYTE*		Addr = InItem->GetBase( (BYTE*) Object );
			ObjectsToModify.AddItem( FObjectBaseAddress( Object, Addr ) );
		}

		ImportText( InItem, ObjectsToModify, Value );
	}
}

// Allows the created controls to react to the parent window being resized.
void UPropertyInputText::ParentResized( WxPropertyWindow_Base* InItem, const wxRect& InRC )
{
	UPropertyInputProxy::ParentResized( InItem, InRC );

	if( PropertyPanel )
	{
		PropertyPanel->SetSize( InRC.x, 1, InRC.GetWidth() + 2 - (Buttons.Num() * GetButtonSize()), InRC.GetHeight() - 1);
		PropertyPanel->Layout();
	}
}

/**
 * Syncs the generic browser to the object specified by the given property.
 */
static void SyncGenericBrowser(WxPropertyWindow_Base* InItem)
{
	UObjectProperty* ObjectProperty = Cast<UObjectProperty>( InItem->Property );
	if( ObjectProperty )
	{
		UClass* PropertyClass = ObjectProperty->PropertyClass;

		// Get a list of addresses for objects handled by the property window.
		TArray<BYTE*> Addresses;
		const UBOOL bWasGetReadAddressSuccessful = InItem->GetReadAddress( InItem, InItem->bSingleSelectOnly, Addresses, FALSE );
		check( bWasGetReadAddressSuccessful );

		// Create a list of object names.
		TArray<FString> ObjectNames;
		ObjectNames.Empty( Addresses.Num() );

		// Copy each object's object proper tyname off into the name list.
		UProperty* Property = InItem->Property;
		for ( INT i = 0 ; i < Addresses.Num() ; ++i )
		{
			new( ObjectNames ) FString();
			Property->ExportText( 0, ObjectNames(i), Addresses(i) - Property->Offset, Addresses(i) - Property->Offset, NULL, PPF_Localized );
		}

		// Create a list of objects to sync the generic browser to.
		TArray<UObject*> Objects;
		for ( INT i = 0 ; i < ObjectNames.Num() ; ++i )
		{
			UObject* Object = UObject::StaticFindObject( PropertyClass, ANY_PACKAGE, *ObjectNames(i), FALSE );
			if ( Object )
			{
				Objects.AddItem( Object );
			}
		}

		// Sync up the generic browser.
		WxGenericBrowser* GenericBrowser = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
		if ( GenericBrowser )
		{
			// Make sure the window is visible.  The window needs to be visible *before*
			// the browser is sync'd to objects so that redraws actually happen!
			GUnrealEd->GetBrowserManager()->ShowWindow( GenericBrowser->GetDockID(), TRUE );

			// Sync.
			GenericBrowser->SyncToObjects( Objects );
		}
	}
}

// Handles button clicks from within the item.
void UPropertyInputText::ButtonClick( WxPropertyWindow_Base* InItem, ePropButton InButton )
{
	switch( InButton )
	{
		case PB_Browse:
			if ( GUnrealEd )
			{
				// Sync the generic browser to the object(s) specified by the given property.
				SyncGenericBrowser( InItem );
			}
			break;

		case PB_Clear:
			SendTextToObjects( InItem, TEXT("None") );
			InItem->Refresh();
			break;

		case PB_Use:
			OnUseSelected( InItem );
			break;

		default:
			Super::ButtonClick( InItem, InButton );
			break;
	}
}

// Refreshes the value in the controls created for this property.
void UPropertyInputText::RefreshControlValue( UProperty* InProperty, BYTE* InData )
{
	FString Wk;
	InProperty->ExportText( 0, Wk, InData - InProperty->Offset, InData - InProperty->Offset, NULL, PPF_Localized );

	UByteProperty* EnumProperty = Cast<UByteProperty>(InProperty);
	if ( EnumProperty != NULL && EnumProperty->Enum != NULL )
	{
		// see if we have alternate text to use for displaying the value
		UMetaData* PackageMetaData = EnumProperty->GetOutermost()->GetMetaData();
		if ( PackageMetaData )
		{
			FName AltValueName = FName(*(Wk+TEXT(".DisplayName")));
			FString ValueText = PackageMetaData->GetValue(EnumProperty->Enum, AltValueName);
			if ( ValueText.Len() > 0 )
			{
				// render the alternate text for this enum value
				Wk = ValueText;
			}
		}
	}

	TextCtrl->SetValue( *Wk );
}

/**
* Allows a customized response to the set focus event of the property item.
* @param	InItem			The property window node.
*/
void UPropertyInputText::OnSetFocus(WxPropertyWindow_Base* InItem)
{
	if(TextCtrl)
	{
		TextCtrl->SetFocus();
	}
}

/**
* Allows a customized response to the kill focus event of the property item.
* @param	InItem			The property window node.
*/
void UPropertyInputText::OnKillFocus(WxPropertyWindow_Base* InItem)
{
	if(TextCtrl)
	{
		TextCtrl->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
	}
}

/**
 *	This is a spinner control specific to numeric properties.  It creates the spinner buttons and
 *  emulates a 3DSMax style spinner control.
 */ 
class WxNumericPropertySpinner : public wxPanel
{
public:
	WxNumericPropertySpinner(wxWindow* InParent, class UPropertyInputNumeric* InPropertyInput, WxPropertyWindow_Base* InItem);

	/** Event handler for mouse events. */
	void OnMouseEvent(wxMouseEvent& InEvent);

	/** Event handler for the spin buttons. */
	void OnSpin(wxCommandEvent& InEvent);

	/**
	 * Sets a fixed increment amount to use instead of 1% of the current value.
	 * @param IncrementAmount	The increment amount to use instead of defaulting to 1% of the starting value.
	 */
	void SetFixedIncrementAmount(FLOAT IncrementAmount)
	{
		FixedIncrementAmount = IncrementAmount;
		bFixedIncrement = TRUE;
	}

private:
	/**
	 * Updates the numeric property text box and sends the new value to all objects.
	 * @param NewValue New value for the property.
	 */
	void UpdatePropertyValue(FLOAT NewValue);

	/**
	 * Copies to the specified output array the property value for all objects being edited.
	 */
	void CapturePropertyValues(TArray<FLOAT>& OutValues);

	/** @return Whether or not we should be operating as if there are objects with multiple values selected. */
	UBOOL ShouldSpinMultipleValues();

	/** Parent Property Input Proxy. */
	class UPropertyInputNumeric* PropertyInput;

	/** Parent Property to send value updates to. */
	WxPropertyWindow_Base* PropertyItem;

	/** Change since mouse was captured. */
	INT MouseDelta;

	wxPoint MouseStartPoint;

	/** Starting value of the property when we capture the mouse. */
	FLOAT StartValue;

	TArray<FLOAT> StartValues;

	/** Flag for whether or not we should be using a fixed increment amount. */
	UBOOL bFixedIncrement;

	/** Fixed increment amount. */
	FLOAT FixedIncrementAmount;

	/** Since Wx has no way of hiding cursors normally, we need to create a blank cursor to use to hide our cursor. */
	wxCursor BlankCursor;

	/** TRUE if spinning a property for multiple objects, in which case spinning affects a relative change independently for each object. */
	UBOOL bSpinningMultipleValues;

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WxNumericPropertySpinner, wxPanel)
	EVT_MOUSE_EVENTS(OnMouseEvent)
	EVT_BUTTON(IDB_SPIN_UP, WxNumericPropertySpinner::OnSpin)
	EVT_BUTTON(IDB_SPIN_DOWN, WxNumericPropertySpinner::OnSpin)
END_EVENT_TABLE()

WxNumericPropertySpinner::WxNumericPropertySpinner(wxWindow* InParent, class UPropertyInputNumeric* InPropertyInput, WxPropertyWindow_Base* InItem) : 
wxPanel(InParent),
PropertyInput(InPropertyInput),
PropertyItem(InItem),
bFixedIncrement(FALSE),
FixedIncrementAmount(1.0f)
{
	// Create spinner buttons
	wxSizer* Sizer = new wxBoxSizer(wxVERTICAL);
	{
		{
			WxMaskedBitmap bmp;
			bmp.Load(TEXT("Spinner_Up"));
			WxPropertyItemButton* SpinButton = new WxPropertyItemButton( this, IDB_SPIN_UP, bmp );
	
			SpinButton->SetMinSize(wxSize(12,-1));
			Sizer->Add(SpinButton, 1, wxEXPAND);
		}

		Sizer->AddSpacer(4);

		{
			WxMaskedBitmap bmp;
			bmp.Load(TEXT("Spinner_Down"));
			WxPropertyItemButton* SpinButton = new WxPropertyItemButton( this, IDB_SPIN_DOWN, bmp );

			SpinButton->SetMinSize(wxSize(12,-1));
			Sizer->Add(SpinButton, 1, wxEXPAND);
		}
	}
	SetSizer(Sizer);

	// Create a Blank Cursor
	WxMaskedBitmap bitmap(TEXT("blank"));
	wxImage BlankImage = bitmap.ConvertToImage();
	BlankImage.SetMaskColour(192,192,192);
	BlankCursor = wxCursor(BlankImage);

	// If more than one object is selected, spinning affects a relative change independently for each object.
	WxPropertyWindow_Objects* ObjectNode = PropertyItem->FindObjectItemParent();
	bSpinningMultipleValues = ObjectNode->GetNumObjects() > 1;
}

/**
 * Copies to the specified output array the property value for all objects being edited.
 */
void WxNumericPropertySpinner::CapturePropertyValues(TArray<FLOAT>& OutValues)
{
	PropertyInput->GetValues(PropertyItem, OutValues);
}

/** @return Whether or not we should be operating as if there are objects with multiple values selected. */
UBOOL WxNumericPropertySpinner::ShouldSpinMultipleValues()
{
	WxPropertyWindow_Objects* ObjectNode = PropertyItem->FindObjectItemParent();

	TArray<BYTE*> OutAddresses;
	const UBOOL bSpinMultiple = ObjectNode->GetReadAddress(PropertyItem, FALSE, OutAddresses, TRUE) == FALSE;

	return bSpinMultiple;
}

void WxNumericPropertySpinner::OnMouseEvent(wxMouseEvent& InEvent)
{
	const UBOOL bHasCapture = HasCapture();

	// See if we should be capturing the mouse.
	if(InEvent.LeftDown())
	{
		CaptureMouse();

		MouseDelta = 0;
		MouseStartPoint = InEvent.GetPosition();

		// Update whether or not we are spinning multiple values by comparing the properties values.
		bSpinningMultipleValues = ShouldSpinMultipleValues();


		if ( bSpinningMultipleValues )
		{
			// Spinning multiple values affects a relative change independently for each object.
			StartValue = 100.0f;

			// Assemble a list of starting values for objects manipulated by this property window.
			CapturePropertyValues( StartValues );
		}
		else
		{
			// Solve for the equation value and pass that value back to the property
			StartValue = PropertyInput->GetValue();
		}
		
		// Change the cursor and background color for the panel to indicate that we are dragging.
		wxSetCursor(BlankCursor);

		SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
		Refresh();
	}
	else if(InEvent.LeftUp() && bHasCapture)
	{
		WarpPointer(MouseStartPoint.x, MouseStartPoint.y);
		ReleaseMouse();

		// clear the text control if we are spinning multiple values.
		if(bSpinningMultipleValues)
		{
			PropertyInput->Equation = TEXT("");
			PropertyInput->TextCtrl->SetValue(TEXT(""));
		}

		// Change the cursor back to normal and the background color of the panel back to normal.
		wxSetCursor(wxNullCursor);

		SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
		Refresh();
	}
	else if(InEvent.Dragging() && bHasCapture)
	{
		const INT MoveDelta = InEvent.GetY() - MouseStartPoint.y;
		const INT DeltaSmoother = 3;

		// To keep the movement from being completely twitchy, we use a DeltaSmoother which means the mouse needs to move at least so many pixels before registering a change.
		if(Abs<INT>(MoveDelta) >= DeltaSmoother)
		{
			// Add contribution of delta to the total mouse delta, we need to invert it because Y is 0 at the top of the screen.
			MouseDelta += -(MoveDelta) / DeltaSmoother;

			// Solve for the equation value and pass that value back to the property, we always increment by 1% of the original value.
			FLOAT ChangeAmount;

			if(bFixedIncrement == FALSE)
			{
				ChangeAmount = Abs(StartValue) * 0.01f;
				const FLOAT SmallestChange = 0.01f;
				if(ChangeAmount < SmallestChange)
				{
					ChangeAmount = SmallestChange;
				}
			}
			else
			{
				ChangeAmount = FixedIncrementAmount;
			}

			WarpPointer(MouseStartPoint.x, MouseStartPoint.y);

			if ( bSpinningMultipleValues )
			{
				FString DeltaText;
				TArray<FLOAT> NewValues;
				if ( !bFixedIncrement )
				{
					// e.g. floats.
					const FLOAT FinalValue = ChangeAmount * MouseDelta + StartValue;
					const FLOAT FinalScale = FinalValue/100.0f;
					for ( INT i = 0 ; i < StartValues.Num() ; ++i )
					{
						NewValues.AddItem( StartValues(i) * FinalScale );
					}

					DeltaText = FString::Printf(TEXT("%.2f%c"), FinalScale*100, '%');
				}
				else
				{
					// e.g. integers or rotations.
					for ( INT i = 0 ; i < StartValues.Num() ; ++i )
					{
						NewValues.AddItem( ChangeAmount * MouseDelta + StartValues(i) );
					}

					DeltaText = FString::Printf(TEXT("%i"), appTrunc(ChangeAmount * MouseDelta) );
				}
				
				PropertyInput->TextCtrl->Freeze();
				{
					PropertyInput->SetValues( NewValues, PropertyItem );

					// Set some text to let the user know what the current is.
					PropertyInput->Equation = DeltaText;
					PropertyInput->TextCtrl->SetValue(*DeltaText);
				}
				PropertyInput->TextCtrl->Thaw();
			}
			else
			{
				// We're spinning for a single value, so just set the new value.
				const FLOAT FinalValue = ChangeAmount * MouseDelta + StartValue;
				PropertyInput->SetValue( FinalValue, PropertyItem );
			}
		}
	}
	else if(InEvent.Moving())
	{
		// Change the cursor to a NS drag cursor when the user mouses over it so that they can tell that there is a drag opportunity here.
		wxSetCursor(wxCURSOR_SIZENS);
	}
}

void WxNumericPropertySpinner::OnSpin(wxCommandEvent& InEvent)
{
	// Calculate how much to change the value by.  If there is no fixed amount set,
	// then use 1% of the current value.  Otherwise, use the fixed value that was set.

	const int WidgetId = InEvent.GetId();
	const FLOAT DirectionScale = (WidgetId == IDB_SPIN_UP) ? 1.0f : -1.0f;

	// Update whether or not we are spinning multiple values by comparing the properties values.
	bSpinningMultipleValues = ShouldSpinMultipleValues();

	if ( bFixedIncrement )
	{
		const FLOAT ChangeAmount = FixedIncrementAmount * DirectionScale;
		if ( bSpinningMultipleValues )
		{
			// Assemble a list of starting values for objects manipulated by this property window.
			CapturePropertyValues( StartValues );

			for ( INT i = 0 ; i < StartValues.Num() ; ++i )
			{
				StartValues(i) += ChangeAmount;
			}
			PropertyInput->SetValues( StartValues, PropertyItem );
			PropertyInput->Equation = TEXT("");
			PropertyInput->TextCtrl->SetValue(TEXT(""));
		}
		else
		{
			// We're spinning for a single value, so just solve for
			// the equation value and pass that value back to the property.
			const FLOAT EqResult = PropertyInput->GetValue();
			PropertyInput->TextCtrl->Freeze();
			{
				PropertyInput->SetValue( EqResult + ChangeAmount, PropertyItem );
			}
			PropertyInput->TextCtrl->Thaw();
			
		}
	}
	else
	{
		const FLOAT SmallestChange = 0.01f;

		if ( bSpinningMultipleValues )
		{
			// Assemble a list of starting values for objects manipulated by this property window.
			CapturePropertyValues( StartValues );

			for ( INT i = 0 ; i < StartValues.Num() ; ++i )
			{
				FLOAT ChangeAmount = Abs(StartValues(i)) * 0.01f;
				if( ChangeAmount < SmallestChange )
				{
					ChangeAmount = SmallestChange;
				}
				StartValues(i) += ChangeAmount * DirectionScale;
			}

			PropertyInput->SetValues( StartValues, PropertyItem );
			PropertyInput->Equation = TEXT("");
			PropertyInput->TextCtrl->SetValue(TEXT(""));
		}
		else
		{
			const FLOAT EqResult = PropertyInput->GetValue();

			FLOAT ChangeAmount = Abs(EqResult) * 0.01f;
			if( ChangeAmount < SmallestChange )
			{
				ChangeAmount = SmallestChange;
			}

			// We're spinning for a single value, so just solve for
			// the equation value and pass that value back to the property.
			PropertyInput->TextCtrl->Freeze();
			{
				PropertyInput->SetValue( EqResult + ChangeAmount*DirectionScale, PropertyItem );
			}
			PropertyInput->TextCtrl->Thaw();
		}
	}
}

/*-----------------------------------------------------------------------------
	UPropertyInputNumeric (with support for equations)
-----------------------------------------------------------------------------*/

// Determines if this input proxy supports the specified UProperty.
UBOOL UPropertyInputNumeric::Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const
{
	if( IS_EDITCONST( InItem->Property ) )
	{
		return FALSE;
	}

	if(	!InItem->bEditInline && InItem->Property->IsA(UFloatProperty::StaticClass()) )
	{
		return TRUE;
	}

	return FALSE;
}

// Allows the created controls to react to the parent window being resized.  Derived classes
// that implement this method should to call up to their parent.
void UPropertyInputNumeric::CreateControls( WxPropertyWindow_Base* InItem, UClass* InBaseClass, const wxRect& InRC, const TCHAR* InValue )
{
	// Set the equation initially to be the same as the numeric value in the property.
	if( Equation.Len() == 0 )
	{
		Equation = InValue;
	}

	// Create any buttons for the control.
	UPropertyInputText::CreateControls( InItem, InBaseClass, InRC, *Equation );

	// Add a spin button to the property sizer.
	SpinButton = new WxNumericPropertySpinner( PropertyPanel, this, InItem );
	PropertySizer->Add(SpinButton, 0, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 1);

	ParentResized(InItem, InRC);
}

// Sends the current value in the input control to the selected objects.
void UPropertyInputNumeric::SendToObjects( WxPropertyWindow_Base* InItem )
{
	if( !TextCtrl )
	{
		return;
	}

	WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();

	// If more than one object is selected, an empty field indicates their values for this property differ.
	// Don't send it to the objects value in this case (if we did, they would all get set to None which isn't good).
	if( ObjectNode->GetNumObjects() == 1 || Equation.Len() || TextCtrl->GetValue().Len() )
	{
		const FLOAT EqResult = GetValue();
		const FString Value( FString::Printf( TEXT("%f"), EqResult ) );

		// Build up a list of objects to modify.
		TArray<FObjectBaseAddress> ObjectsToModify;
		for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
		{
			UObject*	Object = *Itor;
			BYTE*		Addr = InItem->GetBase( (BYTE*) Object );
			ObjectsToModify.AddItem( FObjectBaseAddress( Object, Addr ) );
		}

		ImportText( InItem, ObjectsToModify, Value );
	}
}

/**
 * Refreshes the value in the controls created for this property.
 */
void UPropertyInputNumeric::RefreshControlValue(UProperty* InProperty, BYTE* InData)
{
	Super::RefreshControlValue( InProperty, InData );

	// Update the equation stored at this property.
	Equation = (const TCHAR*)TextCtrl->GetValue();
}

/**
 * @return Returns the numeric value of the property.
 */
FLOAT UPropertyInputNumeric::GetValue()
{
	FLOAT EqResult;

	Equation = (const TCHAR*)TextCtrl->GetValue();
	Eval( Equation, &EqResult );

	return EqResult;
}

/**
 * Gets the values for each of the objects that a property item is affecting.
 * @param	InItem		Property item to get objects from.
 * @param	Values		Array of FLOATs to store values.
 */
void UPropertyInputNumeric::GetValues(WxPropertyWindow_Base* InItem, TArray<FLOAT> &Values)
{
	WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();

	Values.Empty();

	for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
	{
		UObject*	Object = *Itor;
		BYTE*		Addr = InItem->GetBase( (BYTE*) Object );

		const FObjectBaseAddress Cur( Object, Addr );

		FString PreviousValue;
		InItem->Property->ExportText( 0, PreviousValue, Cur.BaseAddress - InItem->Property->Offset, Cur.BaseAddress - InItem->Property->Offset, NULL, PPF_Localized );

		FLOAT EqResult;
		Eval( PreviousValue, &EqResult );

		Values.AddItem( EqResult );
	}
}

/**
 * Updates the numeric property text box and sends the new value to all objects.
 * @param NewValue New value for the property.
 */
void UPropertyInputNumeric::SetValue(FLOAT NewValue, WxPropertyWindow_Base* InItem)
{
	wxString NewTextValue;
	NewTextValue.Printf(TEXT("%.4f"), NewValue);
	TextCtrl->SetValue(NewTextValue);

	SendToObjects(InItem);
}

/**
 * Sends the new values to all objects.
 * @param NewValues		New values for the property.
 */
void UPropertyInputNumeric::SetValues(const TArray<FLOAT>& NewValues, WxPropertyWindow_Base* InItem)
{
	TArray<FString> Values;
	for ( INT i = 0 ; i < NewValues.Num() ; ++i )
	{
		Values.AddItem( FString::Printf( TEXT("%f"), NewValues(i) ) );
	}

	// Build up a list of objects to modify.
	WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();
	TArray<FObjectBaseAddress> ObjectsToModify;
	for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
	{
		UObject*	Object = *Itor;
		BYTE*		Addr = InItem->GetBase( (BYTE*) Object );
		ObjectsToModify.AddItem( FObjectBaseAddress( Object, Addr ) );
	}

	ImportText( InItem, ObjectsToModify, Values );
}

/*-----------------------------------------------------------------------------
	UPropertyInputInteger (with support for equations)
-----------------------------------------------------------------------------*/

// Determines if this input proxy supports the specified UProperty.
UBOOL UPropertyInputInteger::Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const
{
	if( IS_EDITCONST( InItem->Property ) )
	{
		return FALSE;
	}

	const UBOOL bIsInteger = InItem->Property->IsA(UIntProperty::StaticClass());
	const UBOOL bIsNonEnumByte = ( InItem->Property->IsA(UByteProperty::StaticClass()) && Cast<UByteProperty>(InItem->Property)->Enum == NULL);

	if(	!InItem->bEditInline && (bIsInteger || bIsNonEnumByte) )
	{
		return TRUE;
	}

	return FALSE;
}

// Allows the created controls to react to the parent window being resized.  Derived classes
// that implement this method should to call up to their parent.
void UPropertyInputInteger::CreateControls( WxPropertyWindow_Base* InItem, UClass* InBaseClass, const wxRect& InRC, const TCHAR* InValue )
{
	// Create any buttons for the control.
	UPropertyInputNumeric::CreateControls( InItem, InBaseClass, InRC, InValue );

	SpinButton->SetFixedIncrementAmount(1.0f);
}

// Sends the current value in the input control to the selected objects.
void UPropertyInputInteger::SendToObjects( WxPropertyWindow_Base* InItem )
{
	if( !TextCtrl )
	{
		return;
	}

	WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();
	

	// If more than one object is selected, an empty field indicates their values for this property differ.
	// Don't send it to the objects value in this case (if we did, they would all get set to None which isn't good).
	if( ObjectNode->GetNumObjects() == 1 || Equation.Len() || TextCtrl->GetValue().Len() )
	{
		FLOAT EqResult = GetValue();
		const FString Value( FString::Printf( TEXT("%i"), appTrunc(EqResult) ) );

		// Build up a list of objects to modify.
		TArray<FObjectBaseAddress> ObjectsToModify;
		for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
		{
			UObject*	Object = *Itor;
			BYTE*		Addr = InItem->GetBase( (BYTE*) Object );
			ObjectsToModify.AddItem( FObjectBaseAddress( Object, Addr ) );
		}

		ImportText( InItem, ObjectsToModify, Value );
	}
}

/**
 * @return Returns the numeric value of the property.
 */
FLOAT UPropertyInputInteger::GetValue()
{
	FLOAT EqResult;

	Equation = (const TCHAR*)TextCtrl->GetValue();
	Eval(Equation, &EqResult);

	return EqResult;
}

/**
* Updates the numeric property text box and sends the new value to all objects.
* @param NewValue New value for the property.
*/
void UPropertyInputInteger::SetValue(FLOAT NewValue, WxPropertyWindow_Base* InItem)
{
	wxString NewTextValue;
	NewTextValue.Printf(TEXT("%i"), appTrunc(NewValue));
	TextCtrl->SetValue(NewTextValue);

	SendToObjects(InItem);
}

/**
 * Sends the new values to all objects.
 * @param NewValues		New values for the property.
 */
void UPropertyInputInteger::SetValues(const TArray<FLOAT>& NewValues, WxPropertyWindow_Base* InItem)
{
	TArray<FString> Values;
	for ( INT i = 0 ; i < NewValues.Num() ; ++i )
	{
		const INT NewVal = appTrunc(NewValues(i));
		Values.AddItem( FString::Printf( TEXT("%i"), NewVal ) );
	}

	// Build up a list of objects to modify.
	WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();
	TArray<FObjectBaseAddress> ObjectsToModify;
	for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
	{
		UObject*	Object = *Itor;
		BYTE*		Addr = InItem->GetBase( (BYTE*) Object );
		ObjectsToModify.AddItem( FObjectBaseAddress( Object, Addr ) );
	}

	ImportText( InItem, ObjectsToModify, Values );
}

/*-----------------------------------------------------------------------------
	UPropertyInputRotation
-----------------------------------------------------------------------------*/

// Determines if this input proxy supports the specified UProperty.
UBOOL UPropertyInputRotation::Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const
{
	if( IS_EDITCONST( InItem->Property ) )
	{
		return FALSE;
	}

	if( Cast<UStructProperty>(InItem->ParentItem->Property,CLASS_IsAUStructProperty) && Cast<UStructProperty>(InItem->ParentItem->Property,CLASS_IsAUStructProperty)->Struct->GetFName() == NAME_Rotator )
	{
		return TRUE;
	}

	return FALSE;
}

// Allows the created controls to react to the parent window being resized.  Derived classes
// that implement this method should to call up to their parent.
void UPropertyInputRotation::CreateControls( WxPropertyWindow_Base* InItem, UClass* InBaseClass, const wxRect& InRC, const TCHAR* InValue )
{
	FString Result;

	// Set the equation initially to be the same as the numeric value in the property.
	Equation = InValue;

	if(Equation.Len() > 0)
	{
		FLOAT Val = appAtof( *Equation );

		Val = 360.f * (Val / 65536.f);

		FString Wk;
		if( Abs(Val) > 359.f )
		{
			INT Revolutions = Val / 360.f;
			Val -= Revolutions * 360;
			Wk = FString::Printf( TEXT("%.2f%c %s %d"), Val, 176, (Revolutions < 0)?TEXT("-"):TEXT("+"), abs(Revolutions) );
		}
		else
		{
			Wk = FString::Printf( TEXT("%.2f%c"), Val, 176 );
		}

		Result = Wk;
	}


	// Create any buttons for the control.
	UPropertyInputNumeric::CreateControls( InItem, InBaseClass, InRC, *Result );

	TextCtrl->SetValue(*Result);
	TextCtrl->SetSelection(-1,-1);

	// Set a fixed increment amount.
	SpinButton->SetFixedIncrementAmount(1.0f);

}

// Sends the current value in the input control to the selected objects.
void UPropertyInputRotation::SendToObjects( WxPropertyWindow_Base* InItem )
{
	if( !TextCtrl )
	{
		return;
	}

	WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();

	// If more than one object is selected, an empty field indicates their values for this property differ.
	// Don't send it to the objects value in this case (if we did, they would all get set to None which isn't good).
	if( ObjectNode->GetNumObjects() == 1 || Equation.Len() || TextCtrl->GetValue().Len() )
	{
		// Parse the input back from the control (i.e. "80.0° - 2")
		FLOAT Val = GetValue();

		// Convert back to Unreal units
		Val = 65536.f * (Val / 360.f);

		
		const FString Value = *FString::Printf( TEXT("%f"), Val );

		// Build up a list of objects to modify.
		TArray<FObjectBaseAddress> ObjectsToModify;
		for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
		{
			UObject*	Object = *Itor;
			BYTE*		Addr = InItem->GetBase( (BYTE*) Object );
			ObjectsToModify.AddItem( FObjectBaseAddress( Object, Addr ) );
		}

		ImportText( InItem, ObjectsToModify, Value );
	}
}

/**
 * @return Returns the numeric value of the property in DEGREES, NOT Unreal Units!
 */
FLOAT UPropertyInputRotation::GetValue()
{
	// Parse the input back from the control (i.e. "80.0° - 2")
	Equation = (const TCHAR*)TextCtrl->GetValue();
	FLOAT Val;

	TArray<FString> Split;
	if( Equation.ParseIntoArray( &Split, TEXT(" "), 0 ) == 3 )
	{
		const INT Sign = (Split(1) == TEXT("+")) ? +1 : -1;
		const INT Revolutions = appAtof( *Split(2) );

		Val = appAtof( *Split(0) ) + (360 * (Revolutions * Sign));
	}
	else
	{
		Val = appAtof( *Equation );
	}

	return Val;
}

/**
 * Gets the values for each of the objects that a property item is affecting.
 * @param	InItem		Property item to get objects from.
 * @param	Values		Array of FLOATs to store values.
 */
void UPropertyInputRotation::GetValues(WxPropertyWindow_Base* InItem, TArray<FLOAT> &Values)
{
	UPropertyInputNumeric::GetValues(InItem, Values);

	// Convert rotation values from unreal units to degrees.
	for(INT ItemIdx=0; ItemIdx<Values.Num(); ItemIdx++)
	{
		const FLOAT Val = Values(ItemIdx);

		Values(ItemIdx) = 360.f * (Val / 65536.f);
	}
}

/**
 * Updates the numeric property text box and sends the new value to all objects, note that this takes values IN DEGREES.  NOT Unreal Units!
 * @param NewValue New value for the property.
 */
void UPropertyInputRotation::SetValue(FLOAT Val, WxPropertyWindow_Base* InItem)
{
	FString Wk;
	if( Abs(Val) > 359.f )
	{
		const INT Revolutions = Val / 360.f;
		Val -= Revolutions * 360;
		Wk = FString::Printf( TEXT("%.2f%c %s %d"), Val, 176, (Revolutions < 0)?TEXT("-"):TEXT("+"), abs(Revolutions) );
	}
	else
	{
		Wk = FString::Printf( TEXT("%.2f%c"), Val, 176 );
	}

	TextCtrl->SetValue(*Wk);

	SendToObjects(InItem);
}

/**
 * Sends the new values to all objects.
 * @param NewValues		New values for the property.
 */
void UPropertyInputRotation::SetValues(const TArray<FLOAT>& NewValues, WxPropertyWindow_Base* InItem)
{
	TArray<FString> Values;
	for ( INT i = 0 ; i < NewValues.Num() ; ++i )
	{
		FLOAT Val = NewValues(i);

		// Convert back to Unreal units
		Val = 65536.f * (Val / 360.f);

		const FString Wk = *FString::Printf( TEXT("%f"), Val );

		Values.AddItem( Wk );
	}

	// Build up a list of objects to modify.
	WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();
	TArray<FObjectBaseAddress> ObjectsToModify;
	for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
	{
		UObject*	Object = *Itor;
		BYTE*		Addr = InItem->GetBase( (BYTE*) Object );
		ObjectsToModify.AddItem( FObjectBaseAddress( Object, Addr ) );
	}

	ImportText( InItem, ObjectsToModify, Values );
}

/*-----------------------------------------------------------------------------
	UPropertyInputCombo
-----------------------------------------------------------------------------*/

// Determines if this input proxy supports the specified UProperty.
UBOOL UPropertyInputCombo::Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const
{
	if( IS_EDITCONST( InItem->Property ) )
	{
		return FALSE;
	}

	if(	((InItem->Property->IsA(UByteProperty::StaticClass()) && Cast<UByteProperty>(InItem->Property)->Enum)
			||	(InItem->Property->IsA(UNameProperty::StaticClass()) && InItem->Property->GetFName() == NAME_InitialState)
			||  (Cast<UClassProperty>(InItem->Property)))
			&&	( ( InArrayIdx == -1 && InItem->Property->ArrayDim == 1 ) || ( InArrayIdx > -1 && InItem->Property->ArrayDim > 0 ) ) )
	{
		return TRUE;
	}

	return FALSE;
}

// Allows the created controls to react to the parent window being resized.  Derived classes
// that implement this method should to call up to their parent.
void UPropertyInputCombo::CreateControls( WxPropertyWindow_Base* InItem, UClass* InBaseClass, const wxRect& InRC, const TCHAR* InValue )
{
	long style;

	FString CurrentTextValue = InValue;

	UByteProperty* ByteProperty = Cast<UByteProperty>(InItem->Property);
	if ( ByteProperty != NULL )
	{
		style = wxCB_READONLY;
		if(ByteProperty->GetOwnerProperty()->HasMetaData(TEXT("Sorted")))
		{
			style |= wxCB_SORT;
		}
	}
	else
	{
		style = wxCB_READONLY|wxCB_SORT;
	}

	ComboBox = new WxComboBox( InItem, ID_PROPWINDOW_COMBOBOX, TEXT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, style );

	// Freeze the combo box while we populate it to reduce the performance hit
	ComboBox->Freeze();

	if( ByteProperty != NULL )
	{
		check(ByteProperty->Enum);

		UEnum* Enum = ByteProperty->Enum;
		UMetaData* PackageMetaData = Enum->GetOutermost()->GetMetaData();
		checkSlow(PackageMetaData);

		UBOOL bReplacedInValue=FALSE;

		// Names.Num() - 1, because the last item in an enum is the _MAX item
		for( INT i=0; i< ByteProperty->Enum->NumEnums() - 1; i++ )
		{
			// see if we specified an alternate name for this value using metadata

			// this is the name of the enum value (i.e. ROLE_Authority)
			FString EnumValueName = ByteProperty->Enum->GetEnum(i).ToString();

			// this is the alternate name for this enum value as specified by the meta data
			FName EnumValueFName = *(EnumValueName + TEXT(".DisplayName"));
			const FString& AlternateEnumValueName = PackageMetaData->GetValue(Enum, EnumValueFName);
			if ( AlternateEnumValueName.Len() == 0 )
			{
				ComboBox->Append( *EnumValueName );
			}
			else
			{
				// CurrentTextValue will be the actual enum value name; if this enum value is the one corresponding to the current property value,
				// change CurrentTextValue to the alternate name text.
				if ( !bReplacedInValue && EnumValueName == CurrentTextValue )
				{
					bReplacedInValue = TRUE;
					CurrentTextValue = AlternateEnumValueName;
				}

				bUsesAlternateDisplayValues = TRUE;
				ComboBox->Append( *AlternateEnumValueName );
			}
		}
	}
	else if( InItem->Property->IsA(UNameProperty::StaticClass()) && InItem->Property->GetFName() == NAME_InitialState )
	{
		TArray<FName> States;

		if( InBaseClass )
		{
			for( TFieldIterator<UState> StateIt( InBaseClass ) ; StateIt ; ++StateIt )
			{
				if( StateIt->StateFlags & STATE_Editable )
				{
					States.AddUniqueItem( StateIt->GetFName() );
				}
			}
		}

		ComboBox->Append( *FName(NAME_None).ToString() );
		for( INT i = 0 ; i < States.Num() ; i++ )
		{
			ComboBox->Append( *States(i).ToString() );
		}
	}
	else if( Cast<UClassProperty>(InItem->Property) )			
	{
		UClassProperty* ClassProp = static_cast<UClassProperty*>(InItem->Property);
		ComboBox->Append( TEXT("None") );
		const UBOOL bAllowAbstract = ClassProp->GetOwnerProperty()->HasMetaData(TEXT("AllowAbstract"));
		for( TObjectIterator<UClass> It ; It ; ++It )
		{
			if ( It->IsChildOf(ClassProp->MetaClass) && IsClassAllowed(InItem, *It, bAllowAbstract) )
			{
				ComboBox->Append( *It->GetName() );
			}
		}
	}

	// This is hacky and terrible, but I don't see a way around it.

	if( CurrentTextValue.Left( 6 ) == TEXT("Class'") )
	{
		CurrentTextValue = CurrentTextValue.Mid( 6, CurrentTextValue.Len()-7 );
		CurrentTextValue = CurrentTextValue.Right( CurrentTextValue.Len() - CurrentTextValue.InStr( ".", 1 ) - 1 );
	}

	INT Index = 0;
	TArray<BYTE*> Addresses;
	if ( InItem->GetReadAddress( InItem, InItem->bSingleSelectOnly, Addresses ) )
	{
		Index = ComboBox->FindString( *CurrentTextValue );
	}

	ComboBox->Select( Index >= 0 ? Index : 0 );

	// Thaw the combo box after we've populated its contents
	ComboBox->Thaw();

	// Create any buttons for the control.
	UPropertyInputProxy::CreateControls( InItem, InBaseClass, InRC, InValue );

	ParentResized( InItem, InRC );
}

// Deletes any controls which were created for editing.
void UPropertyInputCombo::DeleteControls()
{
	// Delete any buttons on the control.
	UPropertyInputProxy::DeleteControls();

	if( ComboBox )
	{
		ComboBox->Destroy();
		ComboBox = NULL;
	}
}

// Sends the current value in the input control to the selected objects.
void UPropertyInputCombo::SendToObjects( WxPropertyWindow_Base* InItem )
{
	if( !ComboBox )
	{
		return;
	}

	FString Value;
	if ( bUsesAlternateDisplayValues )
	{
		// currently only enum properties can use alternate display values; this might change, so assert here so that if support is expanded to other property types
		// without updating this block of code, we'll catch it quickly
		UEnum* Enum = CastChecked<UByteProperty>(InItem->Property)->Enum;
		check(Enum);

		INT SelectedItem = ComboBox->GetSelection();
		check(SelectedItem<Enum->NumEnums());

		Value = Enum->GetEnum(SelectedItem).ToString();
	}
	else
	{
		Value = ComboBox->GetStringSelection().c_str();
	}

	// Build up a list of objects to modify.
	TArray<FObjectBaseAddress> ObjectsToModify;

	WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();
	for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
	{
		UObject*	Object = *Itor;
		BYTE*		Addr = InItem->GetBase( (BYTE*) Object );
		ObjectsToModify.AddItem( FObjectBaseAddress( Object, Addr ) );
	}

	ImportText( InItem, ObjectsToModify, Value );
}

// Allows the created controls to react to the parent window being resized.
void UPropertyInputCombo::ParentResized( WxPropertyWindow_Base* InItem, const wxRect& InRC )
{
	UPropertyInputProxy::ParentResized( InItem, InRC );

	if( ComboBox )
	{
		ComboBox->SetSize( InRC.x - 2, 0, InRC.GetWidth() + 2 - (Buttons.Num() * GetButtonSize()), InRC.GetHeight());
	}
}

/*-----------------------------------------------------------------------------
	UPropertyInputEditInline
-----------------------------------------------------------------------------*/

// Determines if this input proxy supports the specified UProperty.
UBOOL UPropertyInputEditInline::Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const
{
	if( IS_EDITCONST( InItem->Property ) )
	{
		return FALSE;
	}
	
	return InItem->bEditInline;
}

// Handles button clicks from within the item.
void UPropertyInputEditInline::ButtonClick( WxPropertyWindow_Base* InItem, ePropButton InButton )
{
	switch( InButton )
	{
		case PB_NewObject:
		{
			UObjectProperty* ObjProp = Cast<UObjectProperty>( InItem->Property );
			check( ObjProp );

			wxMenu Menu;

			// Create a popup menu containing the classes that can be created for this property.
			// Add all editinlinenew classes which derive from PropertyClass to the menu.
			// At the same time, build a map of enums to classes which will be used by
			// InItem->OnEditInlineNewClass(...) to identify the selected class.

			INT id = ID_PROP_CLASSBASE_START;
			InItem->ClassMap.Empty();
			for( TObjectIterator<UClass> It ; It ; ++It )
			{
				if( It->IsChildOf(ObjProp->PropertyClass) && IsClassAllowed(InItem, *It, FALSE) )
				{
					UBOOL bValidEditinlineClass = TRUE;
					for ( TPropObjectIterator Itor( InItem->FindObjectItemParent()->ObjectIterator() ); bValidEditinlineClass && Itor ; ++Itor )
					{
						UObject* OwnerObject = *Itor;
						debugf(TEXT("(%s) WithinClass for %s is: %s"), *OwnerObject->GetClass()->GetName(), *It->GetName(), *It->ClassWithin->GetName());
						bValidEditinlineClass = OwnerObject->IsA(It->ClassWithin);
					}

					if ( bValidEditinlineClass )
					{
						InItem->ClassMap.Set( id, *It );
						Menu.Append( id, *It->GetName() );
						++id;
					}
				}
			}

			// Show the menu to the user, sending messages to InItem.
			FTrackPopupMenu tpm( InItem, &Menu );
			tpm.Show();
		}
		break;

		case PB_Clear:
		{

			FString CurrValue = InItem->GetPropertyText(); // this is used to check for a Null component and if we find one we do not want to pop up the dialog

			if( CurrValue == TEXT("None") )
			{
				return;
			}
			
			// Prompt the user that this action will eliminate existing data.
			const UBOOL bDoReplace = appMsgf( AMT_YesNo, *LocalizeUnrealEd("EditInlineNewAreYouSure") );

			if( bDoReplace )
			{
				SendTextToObjects( InItem, TEXT("None") );
				InItem->Collapse();
			}
		}
		break;

		case PB_Use:
			OnUseSelected( InItem );
			break;

		default:
			Super::ButtonClick( InItem, InButton );
			break;
	}
}

/**
 * Wrapper method for determining whether a class is valid for use by this property item input proxy.
 *
 * @param	InItem			the property window item that contains this proxy.
 * @param	CheckClass		the class to verify
 * @param	bAllowAbstract	TRUE if abstract classes are allowed
 *
 * @return	TRUE if CheckClass is valid to be used by this input proxy
 */
UBOOL UPropertyInputEditInline::IsClassAllowed( WxPropertyWindow_Base* InItem,  UClass* CheckClass, UBOOL bAllowAbstract ) const
{
	return Super::IsClassAllowed(InItem, CheckClass, bAllowAbstract) && CheckClass->HasAnyClassFlags(CLASS_EditInlineNew);
}

/*-----------------------------------------------------------------------------
	Class implementations.
	
	Done here to control the order they are iterated later on.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(UPropertyDrawArrayHeader);
IMPLEMENT_CLASS(UPropertyDrawProxy);
IMPLEMENT_CLASS(UPropertyDrawNumeric);
IMPLEMENT_CLASS(UPropertyDrawColor);
IMPLEMENT_CLASS(UPropertyDrawRotation);
IMPLEMENT_CLASS(UPropertyDrawRotationHeader);
IMPLEMENT_CLASS(UPropertyDrawBool);

IMPLEMENT_CLASS(UPropertyInputProxy);
IMPLEMENT_CLASS(UPropertyInputArray);
IMPLEMENT_CLASS(UPropertyInputArrayItemBase);
IMPLEMENT_CLASS(UPropertyInputColor);
IMPLEMENT_CLASS(UPropertyInputBool);
IMPLEMENT_CLASS(UPropertyInputArrayItem);
IMPLEMENT_CLASS(UPropertyInputNumeric);
IMPLEMENT_CLASS(UPropertyInputText);
IMPLEMENT_CLASS(UPropertyInputRotation);
IMPLEMENT_CLASS(UPropertyInputInteger);
IMPLEMENT_CLASS(UPropertyInputEditInline);
IMPLEMENT_CLASS(UPropertyInputCombo);

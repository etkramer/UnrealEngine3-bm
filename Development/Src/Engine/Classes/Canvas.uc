//=============================================================================
// Canvas: A drawing canvas.
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class Canvas extends Object
	native
	noexport
	transient;

/**
 * Holds texture information with UV coordinates as well.
 */
struct CanvasIcon
{
	/** Source texture */
	var Texture Texture;
	/** UV coords */
	var float U, V, UL, VL;
};

// Modifiable properties.
var font    Font;            // Font for DrawText.
var float   OrgX, OrgY;      // Origin for drawing.
var float   ClipX, ClipY;    // Bottom right clipping region.
var float   CurX, CurY;      // Current position for drawing.
var float   CurYL;           // Largest Y size since DrawText.
var color   DrawColor;       // Color for drawing.
var bool    bCenter;         // Whether to center the text.
var bool    bNoSmooth;       // Don't bilinear filter.
var const int SizeX, SizeY;  // Zero-based actual dimensions.

// Internal.
var native const pointer Canvas{FCanvas};
var native const pointer SceneView{FSceneView};

var Plane ColorModulate;
var Texture2D DefaultTexture;

// native functions.
native final function DrawTile( Texture Tex, float XL, float YL, float U, float V, float UL, float VL );

/**
 * Draws the emissive channel of a material to an axis-aligned quad at CurX,CurY.
 *
 * @param	Mat - The material which contains the emissive expression to render.
 * @param	XL - The width of the quad in pixels.
 * @param	YL - The height of the quad in pixels.
 * @param	U - The U coordinate of the quad's upper left corner, in normalized coordinates.
 * @param	V - The V coordinate of the quad's upper left corner, in normalized coordinates.
 * @param	UL - The range of U coordinates which is mapped to the quad.
 * @param	VL - The range of V coordinates which is mapped to the quad.
 */
native final function DrawMaterialTile
(
				MaterialInterface	Mat,
				float				XL,
				float				YL,
	optional	float				U,
	optional	float				V,
	optional	float				UL,
	optional	float				VL
);

native final function DrawMaterialTileClipped
(
				MaterialInterface	Mat,
				float				XL,
				float				YL,
	optional	float				U,
	optional	float				V,
	optional	float				UL,
	optional	float				VL
);

native final function StrLen( coerce string String, out float XL, out float YL ); // Wrapped!
native final function TextSize( coerce string String, out float XL, out float YL ); // Clipped!

native final function DrawText( coerce string Text, optional bool CR, optional float XScale, optional float YScale );
native final function DrawTextClipped( coerce string Text, optional bool bCheckHotKey, optional float XScale, optional float YScale );

/**
 * Draws text right aligned from the current position.
 */
final function DrawTextRA(coerce string Text, optional bool CR)
{
	local float XL, YL;
	TextSize(Text,XL,YL);
	SetPos(CurX - XL,CurY);
	DrawText(Text,CR);
}



native(468) final function DrawTileClipped( Texture Tex, float XL, float YL, float U, float V, float UL, float VL );


native final function vector Project(vector location);			// Convert a 3D vector to a 2D screen coords.

/**
 * Pushes a translation matrix onto the canvas.
 *
 * @param TranslationVector		Translation vector to use to create the translation matrix.
 */
native final function PushTranslationMatrix(vector TranslationVector);

/** Pops the topmost matrix from the canvas transform stack. */
native final function PopTransform();

native final function DrawTileStretched(Texture Tex, float XL, float YL,  float U, float V, float UL, float VL, LinearColor LColor, optional bool bStretchHorizontally=true, optional bool bStretchVertically=true,optional float ScalingFactor=1.0);
native final function DrawColorizedTile( Texture Tex, float XL, float YL, float U, float V, float UL, float VL, LinearColor LColor );

/* rjp - unused
native final function WrapStringToArray(string Text, out array<string> OutArray, float dx, optional string EOL);
*/

/* rjp - unused
// jmw - These are two helper functions.  The use the whole texture only.  If you need better support, use DrawTile
native final function DrawTileJustified(Texture2D Tex, byte Justification, float XL, float YL);
native final function DrawTileScaled(Texture2D Tex, float XScale, float YScale);
native final function DrawTextJustified(coerce string String, byte Justification, float x1, float y1, float x2, float y2);
*/
// UnrealScript functions.
event Reset(optional bool bKeepOrigin)
{
	Font        = Default.Font;
	if ( !bKeepOrigin )
	{
		OrgX        = Default.OrgX;
		OrgY        = Default.OrgY;
	}
	CurX        = Default.CurX;
	CurY        = Default.CurY;
	DrawColor   = Default.DrawColor;
	CurYL       = Default.CurYL;
	bCenter     = false;
	bNoSmooth   = false;
}

native final function SetPos( float PosX, float PosY );
final function SetOrigin( float X, float Y )
{
	OrgX = X;
	OrgY = Y;
}
final function SetClip( float X, float Y )
{
	ClipX = X;
	ClipY = Y;
}
/* rjp - unused
final function DrawPattern( Texture2D Tex, float XL, float YL, float Scale )
{
	DrawTile( Tex, XL, YL, (CurX-OrgX)*Scale, (CurY-OrgY)*Scale, XL*Scale, YL*Scale );
}
*/
final function DrawTexture(Texture Tex, float Scale)
{
	if (Tex != None)
	{
		DrawTile(Tex, Tex.GetSurfaceWidth()*Scale, Tex.GetSurfaceHeight()*Scale, 0, 0, Tex.GetSurfaceWidth(), Tex.GetSurfaceHeight());
	}
}

/**
 * Fake CanvasIcon constructor.
 */
final function CanvasIcon MakeIcon(Texture Texture, optional float U, optional float V, optional float UL, optional float VL)
{
	local CanvasIcon Icon;
	if (Texture != None)
	{
		Icon.Texture = Texture;
		Icon.U = U;
		Icon.V = V;
		Icon.UL = (UL != 0.f ? UL : Texture.GetSurfaceWidth());
		Icon.VL = (VL != 0.f ? VL : Texture.GetSurfaceHeight());
	}
	return Icon;
}

/**
 * Draw a CanvasIcon at the desired canvas position.
 */
final function DrawIcon(CanvasIcon Icon, float X, float Y, optional float Scale)
{
	if (Icon.Texture != None)
	{
		// verify properties are valid
		if (Scale <= 0.f)
		{
			Scale = 1.f;
		}
		if (Icon.UL == 0.f)
		{
			Icon.UL = Icon.Texture.GetSurfaceWidth();
		}
		if (Icon.VL == 0.f)
		{
			Icon.VL = Icon.Texture.GetSurfaceHeight();
		}
		// set the canvas position
		CurX = X;
		CurY = Y;
		// and draw the texture
		DrawTile(Icon.Texture, Icon.UL*Scale, Icon.VL*Scale, Icon.U, Icon.V, Icon.UL, Icon.VL);
	}
}

/**
 * Draw a subsection of a CanvasIcon at the desired canvas position.
 */
final function DrawIconSection(CanvasIcon Icon, float X, float Y, float UStartPct, float VStartPct, float UEndPct, float VEndPct, optional float Scale)
{
	if (Icon.Texture != None)
	{
		// verify properties are valid
		if (Scale <= 0.f)
		{
			Scale = 1.f;
		}
		if (Icon.UL == 0.f)
		{
			Icon.UL = Icon.Texture.GetSurfaceWidth();
		}
		if (Icon.VL == 0.f)
		{
			Icon.VL = Icon.Texture.GetSurfaceHeight();
		}
		// set the canvas position
		CurX = X + (UStartPct * Icon.UL * Scale);
		CurY = Y + (VStartPct * Icon.VL * Scale);
		// and draw the texture
		DrawTile(Icon.Texture, Icon.UL * UEndPct * Scale, Icon.VL * VEndPct * Scale, Icon.U + (UStartPct * Icon.UL), Icon.V + (VStartPct * Icon.VL), Icon.UL * UEndPct, Icon.VL * VEndPct);
	}
}

final function DrawRect(float RectX, float RectY, optional Texture Tex = DefaultTexture )
{
	DrawTile( Tex, RectX, RectY, 0, 0, Tex.GetSurfaceWidth(), Tex.GetSurfaceHeight() );
}

final simulated function DrawBox(float width, float height)
{
	local int X, Y;

	X = CurX;
	Y = CurY;

	// normalize CurX, CurY (eliminate float precision errors)
	SetPos(X, Y);

	// draw the left side
	DrawRect(2, height);
	// then move cursor to top-right
	SetPos(X + width - 2, Y);

	// draw the right face
	DrawRect(2, height);
	// then move the cursor to the top-left (+2 to account for the line we already drew for the left-face)
	SetPos(X + 2, Y);

	// draw the top face
	DrawRect(width - 4, 2);
	// then move the cursor to the bottom-left (+2 to account for the line we already drew for the left-face)
	SetPos(X + 2, Y + height - 2);

	// draw the bottom face
	DrawRect(width - 4, 2);
	// move the cursor back to its original position
	SetPos(X, Y);
}

native final function SetDrawColor(byte R, byte G, byte B, optional byte A /*= 255*/);

/* rjp - unused
// Draw a vertical line
final function DrawVertical(float X, float height)
{
    SetPos( X, CurY);
    DrawRect(2, height);
}

// Draw a horizontal line
final function DrawHorizontal(float Y, float width)
{
    SetPos(CurX, Y);
    DrawRect(width, 2);
}

// Draw Line is special as it saves it's original position

final function DrawLine(int direction, float size)
{
    local float X, Y;

    // Save current position
    X = CurX;
    Y = CurY;

    switch (direction)
    {
      case 0:
		  SetPos(X, Y - size);
		  DrawRect(2, size);
		  break;

      case 1:
		  DrawRect(2, size);
		  break;

      case 2:
		  SetPos(X - size, Y);
		  DrawRect(size, 2);
		  break;

	  case 3:
		  DrawRect(size, 2);
		  break;
    }
    // Restore position
    SetPos(X, Y);
}

final simulated function DrawBracket(float width, float height, float bracket_size)
{
    local float X, Y;
    X = CurX;
    Y = CurY;

	Width  = max(width,5);
	Height = max(height,5);

    DrawLine(3, bracket_size);
    DrawLine(1, bracket_size);
    SetPos(X + width, Y);
    DrawLine(2, bracket_size);
    DrawLine(1, bracket_size);
    SetPos(X + width, Y + height);
    DrawLine(0, bracket_size);
    DrawLine(2, bracket_size);
    SetPos(X, Y + height);
    DrawLine(3, bracket_size);
    DrawLine( 0, bracket_size);

    SetPos(X, Y);
}
*/

native final function DrawRotatedTile( Texture Tex, rotator Rotation, float XL, float YL,  float U, float V,float UL, float VL,
											optional float AnchorX, optional float AnchorY);

native final function DrawRotatedMaterialTile( MaterialInterface Mat, rotator Rotation, float XL, float YL,  optional float U, optional float V,
													optional float UL, optional float VL, optional float AnchorX, optional float AnchorY);


native final function Draw2DLine(float X1, float Y1, float X2, float Y2, color LineColor);

native final function DrawTextureLine(vector StartPoint, vector EndPoint, float Perc, float Width, color LineColor, Texture LineTexture, float U, float V, float UL, float VL );
native final function DrawTextureDoubleLine(vector StartPoint, vector EndPoint, float Perc, float Spacing, float Width,	color LineColor, color AltLineColor, Texture Tex, float U, float V, float UL, float VL);


defaultproperties
{
     DrawColor=(R=127,G=127,B=127,A=255)
	 Font="EngineFonts.SmallFont"
     ColorModulate=(X=1,Y=1,Z=1,W=1)
     DefaultTexture="EngineResources.WhiteSquareTexture"
}

/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * This is an abstract base class that is used to define the interface that
 * UnrealEd will use when rendering a given object's thumbnail labels. This
 * is declared as a separate object so that label rendering can be customized
 * without having to support any other interfaces
 */
class ThumbnailLabelRenderer extends Object
	abstract
	native;

cpptext
{
public:

	/** Thumbnail options */
	struct ThumbnailOptions
	{
		/** True if content tags should be shown, where available. */
		UBOOL bShowContentTags;
		
		/** Constructor */
		ThumbnailOptions()
			: bShowContentTags( FALSE )
		{
		}
	};

protected:
	/**
	 * Calculates the size the thumbnail labels will be for the specified font.
	 * Note: that this is a common method for handling lists of strings. The
	 * child class is resposible for building this list of strings.
	 *
	 * @param Labels the list of strings to write out as the labels
	 * @param Font the font object to render with
	 * @param Viewport the viewport that will be rendered into
	 * @param RI the render interface to use for getting the size
	 * @param OutWidth the var that gets the width of the labels
	 * @param OutHeight the var that gets the height
	 */
	void GetSizeFromLabels(const TArray<FString>& Labels,UFont* Font,
		FViewport* Viewport,FCanvas* Canvas,DWORD& OutWidth,
		DWORD& OutHeight);

	/**
	 * Renders the thumbnail labels for the specified object with the specified
	 * font and text color
	 * Note: that this is a common method for handling lists of strings. The
	 * child class is resposible for building this list of strings.
	 *
	 * @param Labels the list of strings to write out as the labels
	 * @param Font the font to draw with
	 * @param X the X location to start drawing at
	 * @param Y the Y location to start drawing at
	 * @param Viewport the viewport to draw into
	 * @param RI the render interface to draw with
	 * @param TextColor the color to draw the text with
	 */
	void DrawLabels(const TArray<FString>& Labels,UFont* Font,INT X,INT Y,
		FViewport* Viewport,FCanvas* Canvas,const FColor& TextColor);

public:
	/**
	 * Subclasses should implement this function to add to the list of labels
	 * for a given object.
	 *
	 * @param Object the object to build the labels for
	 * @param OutLabels the array that is added to
	 */
	virtual void BuildLabelList(UObject*, const ThumbnailOptions&, TArray<FString>&) PURE_VIRTUAL(UThumbnailLabelRenderer::BuildLabelList,);

	/**
	 * Calculates the size the thumbnail labels will be for the specified font
	 *
	 * @param Object the object the thumbnail is of
	 * @param Font the font object to render with
	 * @param Viewport the viewport that will be rendered into
	 * @param RI the render interface to use for getting the size
	 * @param OutWidth the var that gets the width of the labels
	 * @param OutHeight the var that gets the height
	 */
	virtual void GetThumbnailLabelSize(UObject* Object,UFont* Font,
		FViewport* Viewport,FCanvas* Canvas, const ThumbnailOptions& InOptions, DWORD& OutWidth,
		DWORD& OutHeight);

	/**
	 * Renders the thumbnail labels for the specified object with the specified
	 * font and text color
	 *
	 * @param Object the object to render labels for
	 * @param Font the font to draw with
	 * @param X the X location to start drawing at
	 * @param Y the Y location to start drawing at
	 * @param Viewport the viewport to draw into
	 * @param RI the render interface to draw with
	 * @param TextColor the color to draw the text with
	 */
	virtual void DrawThumbnailLabels(UObject* Object,UFont* Font,INT X,INT Y,
		FViewport* Viewport,FCanvas* Canvas, const ThumbnailOptions& InOptions,
		const FColor& TextColor = FColor(255,255,255,255));
}

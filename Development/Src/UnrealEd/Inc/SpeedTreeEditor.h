/*=============================================================================
	SpeedTreeEditor.h: SpeedTree editor definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if WITH_SPEEDTREE

class WxSpeedTreeEditor;

struct FSpeedTreeEditorViewportClient : public FEditorLevelViewportClient, private FPreviewScene
{
public:
	FSpeedTreeEditorViewportClient(class WxSpeedTreeEditor* pwxEditor);

	virtual FSceneInterface* GetScene()
	{ 
		return FPreviewScene::GetScene( ); 
	}
	
	virtual FLinearColor GetBackgroundColor()
	{ 
		return FColor(64, 64, 64); 
	}
	
	virtual UBOOL InputAxis( FViewport* Viewport, INT ControllerId, FName Key, FLOAT Delta, FLOAT DeltaTime );
	
	virtual void Serialize(FArchive& Ar)
	{
		Ar << Input << (FPreviewScene&)*this; 
	}

protected:
	WxSpeedTreeEditor*			SpeedTreeEditor;
	class USpeedTreeComponent*	SpeedTreeComponent;
	UEditorComponent*			EditorComponent;
};

class WxSpeedTreeEditor : public wxFrame, public FSerializableObject
{
	DECLARE_DYNAMIC_CLASS(WxSpeedTreeEditor)
public:
	
	WxSpeedTreeEditor() { }
	WxSpeedTreeEditor(wxWindow* Parent, wxWindowID wxID, class USpeedTree* SpeedTree);

	virtual	~WxSpeedTreeEditor();

	virtual void Serialize(FArchive& Ar);

	void OnSize(wxSizeEvent& wxIn);
	void OnPaint(wxPaintEvent& wxIn);
	
	DECLARE_EVENT_TABLE( )

public:
	class USpeedTree*				SpeedTree;
	WxViewportHolder* 				ViewportHolder;
	wxSplitterWindow* 				SplitterWnd;
	WxPropertyWindow* 				PropertyWindow;
	FSpeedTreeEditorViewportClient*	ViewportClient;
};

#endif


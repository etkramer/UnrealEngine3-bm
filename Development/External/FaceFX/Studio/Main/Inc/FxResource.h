//------------------------------------------------------------------------------
// All of FaceFX Studio's resource IDs.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxResource_H__
#define FxResource_H__

#include "wx/defs.h"

namespace OC3Ent
{

namespace Face
{

enum FaceFXResourceID
{

	FaceFXResourceID_First = wxID_HIGHEST + 1,

//------------------------------------------------------------------------------
// Main Window
//------------------------------------------------------------------------------
	MenuID_MainWindowFileNewActor = FaceFXResourceID_First,
	MenuID_MainWindowFileLoadActor,
	MenuID_MainWindowFileCloseActor,
	MenuID_MainWindowFileSaveActorProxy,
	MenuID_MainWindowFileSaveActor,
	MenuID_MainWindowFileSaveActorAs,
	MenuID_MainWindowEditSearchAndReplaceName,
	MenuID_MainWindowEditUndoProxy,
	MenuID_MainWindowEditUndo,
	MenuID_MainWindowEditRedoProxy,
	MenuID_MainWindowEditRedo,
	MenuID_MainWindowTearoff,
	MenuID_MainWindowViewFullscreen,
	MenuID_MainWindowActorRenameActor,
	MenuID_MainWindowActorFaceGraphNodeGroupManager,
	MenuID_MainWindowActorAnimationManager,
	MenuID_MainWindowActorCurveManager,
	MenuID_MainWindowActorExportTemplate,
	MenuID_MainWindowActorSyncTemplate,
	MenuID_MainWindowActorMountAnimset,
	MenuID_MainWindowActorUnmountAnimset,
	MenuID_MainWindowActorImportAnimset,
	MenuID_MainWindowActorExportAnimset,
	MenuID_MainWindowActorEditDefaultBoneWeights,
	MenuID_MainWindowActorEditAnimBoneWeights,
	MenuID_MainWindowActorPrevFrame,
	MenuID_MainWindowActorNextFrame,
	MenuID_MainWindowOptionsAppOptions,
	MenuID_MainWindowRecentFiles,
	MenuID_MainWindowHelpAbout,
	MenuID_MainWindowHelpHelp,

	MenuID_MainWindowForwardStart,
	MenuID_MainWindowForwardViewResetView = MenuID_MainWindowForwardStart,
	MenuID_MainWindowForwardToolsOptionsPhonemebar,
	MenuID_MainWindowForwardToolsOptionsAudioView,
	MenuID_MainWindowForwardToolsOptionsFaceGraph,
	MenuID_MainWindowForwardToolsOptionsGlobal,
	MenuID_MainWindowForwardEnd = MenuID_MainWindowForwardToolsOptionsGlobal,

	ControlID_MainWindowNotebook,

//------------------------------------------------------------------------------
// Actor Widget
//------------------------------------------------------------------------------
	ControlID_ActorWidgetBoneViewListBox = MenuID_MainWindowForwardEnd + 1,
	ControlID_ActorWidgetFaceGraphViewNodeFilterCombo,
	ControlID_ActorWidgetFaceGraphViewNodeListBox,
	ControlID_ActorWidgetFaceGraphViewQuickPreviewSlider,
	ControlID_ActorWidgetAnimationViewAnimGroupCombo,
	ControlID_ActorWidgetAnimationViewAnimCombo,
	ControlID_ActorWidgetAnimationViewAnimCurveListBox,

	MenuID_ActorWidgetAnimationViewCurveManager,
	MenuID_ActorWidgetAnimationViewCurveProperties,
	
//------------------------------------------------------------------------------
// Audio Widget
//------------------------------------------------------------------------------
	MenuID_AudioWidgetResetView,
	MenuID_AudioWidgetTogglePlotType,
	MenuID_AudioWidgetEditOptions,
	MenuID_AudioWidgetPlayAudio,
	MenuID_AudioWidgetToggleStress,
	MenuID_AudioWidgetSpectralPlot,
	MenuID_AudioWidgetWaveformPlot,

	ControlID_AudioWidgetOptionsDlgOKButton,
	ControlID_AudioWidgetOptionsDlgCancelButton,
	ControlID_AudioWidgetOptionsDlgApplyButton,
	ControlID_AudioWidgetOptionsDlgRestoreDefaultsButton,
	ControlID_AudioWidgetOptionsDlgChangeGeneralColorButton,
	ControlID_AudioWidgetOptionsDlgChangeWaveformColorButton,
	ControlID_AudioWidgetOptionsDlgChangeSpectralColorButton,
	ControlID_AudioWidgetOptionsDlgGeneralColorsListBox,
	ControlID_AudioWidgetOptionsDlgWaveformColorsListBox,
	ControlID_AudioWidgetOptionsDlgSpectralColorsListBox,
	ControlID_AudioWidgetOptionsDlgReverseSpectralCheck,
	ControlID_AudioWidgetOptionsDlgMouseWheelZoomFactorSlider,
	ControlID_AudioWidgetOptionsDlgMousePanningSpeedSlider,
	ControlID_AudioWidgetOptionsDlgColorGammaSlider,
	ControlID_AudioWidgetOptionsDlgWindowLengthChoice,
	ControlID_AudioWidgetOptionsDlgWindowFunctionChoice,
	ControlID_AudioWidgetOptionsDlgTimeAxisOnTopCheck,

//------------------------------------------------------------------------------
// Coarticulation Widget
//------------------------------------------------------------------------------
	ControlID_CoarticulationWidgetCustomCoarticulationCheck,
	ControlID_CoarticulationWidgetParameterGrid,
	ControlID_CoarticulationWidgetSuppressionWindowMaxText,
	ControlID_CoarticulationWidgetSuppressionWindowMaxSlopeText,
	ControlID_CoarticulationWidgetSuppressionWindowMinText,
	ControlID_CoarticulationWidgetSuppressionWindowMinSlopeText,
	ControlID_CoarticulationWidgetShortSilenceText,
	ControlID_CoarticulationWidgetLeadInTimeText,
	ControlID_CoarticulationWidgetLeadOutTimeText,
	ControlID_CoarticulationWidgetSplitDiphthongCheck,
	ControlID_CoarticulationWidgetDiphthongBoundaryText,
	ControlID_CoarticulationWidgetConvertToFlapsCheck,
	ControlID_CoarticulationWidgetCoarticulateButton,
	ControlID_CoarticulationWidgetConfigNameText,

//------------------------------------------------------------------------------
// Curve Widget
//------------------------------------------------------------------------------
	MenuID_CurveWidgetResetView,
	MenuID_CurveWidgetSelectAll,
	MenuID_CurveWidgetLockTangent,
	MenuID_CurveWidgetUnlockTangent,
	MenuID_CurveWidgetDeleteSelection,
	MenuID_CurveWidgetEditKey,
	MenuID_CurveWidgetResetDerivatives,
	MenuID_CurveWidgetAddKey,
	MenuID_CurveWidgetCopy,
	MenuID_CurveWidgetCut,
	MenuID_CurveWidgetPaste,

	ControlID_CurveWidgetToolbarCut,
	ControlID_CurveWidgetToolbarCopy,
	ControlID_CurveWidgetToolbarPaste,
	ControlID_CurveWidgetToolbarLockTangent,
	ControlID_CurveWidgetToolbarUnlockTangent,
	ControlID_CurveWidgetToolbarSnapValue,
	ControlID_CurveWidgetToolbarSnapTime,
	ControlID_CurveWidgetToolbarFitTime,
	ControlID_CurveWidgetToolbarFitValue,
	ControlID_CurveWidgetToolbarViewAllActive,
	ControlID_CurveWidgetToolbarPrevKey,
	ControlID_CurveWidgetToolbarNextKey,
	// Insert all curve widget toolbar *button* ID's before this point.
	ControlID_CurveWidgetToolbarTimeDisplay,
	ControlID_CurveWidgetToolbarValueDisplay,
	ControlID_CurveWidgetToolbarIncDerivDisplay,
	ControlID_CurveWidgetToolbarOutDerivDisplay,
	ControlID_CurveWidgetToolbarDelete,

	ControlID_CurveWidgetToolbarStart = ControlID_CurveWidgetToolbarCut,
	ControlID_CurveWidgetToolbarEnd   = ControlID_CurveWidgetToolbarDelete,

//------------------------------------------------------------------------------
// Face Graph Node Properties Widget
//------------------------------------------------------------------------------
	MenuID_FaceGraphNodePropertiesWidgetCopy = ControlID_CurveWidgetToolbarEnd + 1,
	MenuID_FaceGraphNodePropertiesWidgetPaste,
	MenuID_FaceGraphNodePropertiesWidgetDelete,

//------------------------------------------------------------------------------
// Face Graph Widget
//------------------------------------------------------------------------------
	MenuID_FaceGraphWidgetResetView,
	MenuID_FaceGraphWidgetDeleteNode,
	MenuID_FaceGraphWidgetDeleteLink,
	MenuID_FaceGraphWidgetLinkProperties,
	MenuID_FaceGraphWidgetToggleValues,
	MenuID_FaceGraphWidgetToggleCurves, 
	MenuID_FaceGraphWidgetToggleTransitions,
	MenuID_FaceGraphWidgetToggleLinks,
	MenuID_FaceGraphWidgetToggleAutopan,
	MenuID_FaceGraphWidgetRelayout,
	MenuID_FaceGraphWidgetOptions,
	MenuID_FaceGraphWidgetCreateSpeechGesturesFragment,

	// Reminder: There is a menu range starting at ADDNODEMENU_MODIFIER in 
	// FxFaceGraphWidget.cpp.  It may need to be updated if it conflicts.

	ControlID_FaceGraphWidgetToolbarAddNodeButton,
	ControlID_FaceGraphWidgetToolbarNodeValueVizButton,
	ControlID_FaceGraphWidgetToolbarRelayoutButton,
	ControlID_FaceGraphWidgetToolbarResetViewButton,
	ControlID_FaceGraphWidgetToolbarFindSelectionButton,
	ControlID_FaceGraphWidgetToolbarMakeSGSetupButton,

	ControlID_FaceGraphWidgetOptionsDlgColourListBox,
	ControlID_FaceGraphWidgetOptionsDlgColourPreviewStaticBmp,
	ControlID_FaceGraphWidgetOptionsDlgColourChangeButton,
	ControlID_FaceGraphWidgetOptionsDlgValueVisualizationCheck,
	ControlID_FaceGraphWidgetOptionsDlgBezierLinksCheck,
	ControlID_FaceGraphWidgetOptionsDlgAnimateTransitionsCheck,
	ControlID_FaceGraphWidgetOptionsDlgDrawOnlySelectedCheck,

//------------------------------------------------------------------------------
// Gesture Widget
//------------------------------------------------------------------------------
	ControlID_GestureWidgetProcessButton,
	ControlID_GestureWidgetEAAnimChoice,
	ControlID_GestureWidgetEATracksGrid,
	ControlID_GestureWidgetEAProbScaleGrid,
	ControlID_GestureWidgetODPropGrid,
	ControlID_GestureWidgetUpdateEAAnimButton,
	ControlID_GestureWidgetLoadConfigButton,
	ControlID_GestureWidgetSaveConfigButton,
	ControlID_GestureWidgetResetDefaultConfigButton,
	ControlID_GestureWidgetReinitializeGesturesButton,
	ControlID_GestureWidgetConfigNameText,

//------------------------------------------------------------------------------
// Mapping Widget
//------------------------------------------------------------------------------
	MenuID_MappingWidgetRemoveTarget,
	MenuID_MappingWidgetMakeDefault,
	MenuID_MappingWidgetAddTarget,
	MenuID_MappingWidgetClearMapping,
	MenuID_MappingWidgetSyncAnim,

	ControlID_MappingWidgetToolbarMakeDefaultMappingButton,
	ControlID_MappingWidgetToolbarClearMappingButton,
	ControlID_MappingWidgetToolbarAddTargetButton,
	ControlID_MappingWidgetToolbarSyncAnimButton,

	ControlID_MappingWidgetTargetSelectionDlgNodeGroupChoice,
	ControlID_MappingWidgetTargetSelectionDlgTargetNameCombo,

//------------------------------------------------------------------------------
// Phoneme Widget
//------------------------------------------------------------------------------
	MenuID_PhonemeWidgetResetView,
	MenuID_PhonemeWidgetPlaySelection,
	MenuID_PhonemeWidgetDeleteSelection,
	MenuID_PhonemeWidgetGroupSelection,
	MenuID_PhonemeWidgetUngroupSelection,
	MenuID_PhonemeWidgetInsertPhoneme,
	MenuID_PhonemeWidgetAppendPhoneme,
	MenuID_PhonemeWidgetOptions,
	MenuID_PhonemeWidgetHelp,
	MenuID_PhonemeWidgetCut,
	MenuID_PhonemeWidgetCopy,
	MenuID_PhonemeWidgetPaste,
	MenuID_PhonemeWidgetUndo,
	MenuID_PhonemeWidgetRedo,
	MenuID_PhonemeWidgetZoomSelection,
	MenuID_PhonemeWidgetReanalyzeSelection,
	MenuID_PhonemeWidgetQuickChangePhoneme,

	// Reminder: There is a menu range starting at PHONMENU_MODIFIER in 
	// FxPhonemeWidget.cpp.  It may need to be updated if it conflicts.

	ControlID_PhonemeWidgetOptionsDlgOkButton,
	ControlID_PhonemeWidgetCancelButton,
	ControlID_PhonemeWidgetApplyButton,
	ControlID_PhonemeWidgetRestoreDefaultButton,
	ControlID_PhonemeWidgetPhoneticAlphabetRadio,
	ControlID_PhonemeWidgetColorsListBox,
	ControlID_PhonemeWidgetColorSampleStaticBmp,
	ControlID_PhonemeWidgetChangeColorButton,
	ControlID_PhonemeWidgetPhonemesFillBarCheck,

	ControlID_PhonemeWidgetQuickChangePhonDlgPhonemeListBox,
	ControlID_PhonemeWidgetQuickChangePhonDlgOKButton,

//------------------------------------------------------------------------------
// Render Widget (OC3)
//------------------------------------------------------------------------------
	ControlID_RenderWidgetOC3ToolbarResetViewButton,
	ControlID_RenderWidgetOC3ToolbarToggleWireframeRenderingButton,
	ControlID_RenderWidgetOC3ToolbarToggleLightingButton,
	ControlID_RenderWidgetOC3ToolbarToggleMeshRenderingButton,
	ControlID_RenderWidgetOC3ToolbarToggleNormalsRenderingButton,
	ControlID_RenderWidgetOC3ToolbarToggleInfluencesRenderingButton,
	ControlID_RenderWidgetOC3ToolbarToggleSkeletonRenderingButton,
	ControlID_RenderWidgetOC3ToolbarCreateCameraButton,
	ControlID_RenderWidgetOC3ToolbarDeleteCameraButton,
	ControlID_RenderWidgetOC3ToolbarCameraChoice,

//------------------------------------------------------------------------------
// Timeline Widget
//------------------------------------------------------------------------------
	MenuID_TimelineWidgetResetView,
	MenuID_TimelineWidgetPlayAudio,

	ControlID_TimelineWidgetTopPanelMinTimeExtent,
	ControlID_TimelineWidgetTopPanelMinTimeWindow,
	ControlID_TimelineWidgetTopPanelMaxTimeWindow,
	ControlID_TimelineWidgetTopPanelMaxTimeExtent,

	MenuID_TimelineWidgetBottomPanelVerySlow,
	MenuID_TimelineWidgetBottomPanelSlow,
	MenuID_TimelineWidgetBottomPanelNormal,
	MenuID_TimelineWidgetBottomPanelFast,
	MenuID_TimelineWidgetBottomPanelVeryFast,

	ControlID_TimelineWidgetBottomPanelLoopToggle,
	ControlID_TimelineWidgetBottomPanelResetButton,
	ControlID_TimelineWidgetBottomPanelPlayButton,
	ControlID_TimelineWidgetBottomPanelCurrTimeText,

	ControlID_TimelineWidgetMainSliderWindow,
	ControlID_TimelineWidgetMainTimelineWindow,

//------------------------------------------------------------------------------
// New Workspace Wizard
//------------------------------------------------------------------------------
	ControlID_NewWorkspaceWizardSelectNodePageNodeGroupChoice,
	ControlID_NewWorkspaceWizardSelectNodePageNodeNameList,
	ControlID_NewWorkspaceWizardSelectNodePageSliderNameList,
	ControlID_NewWorkspaceWizardSelectNodePageAddButton,
	ControlID_NewWorkspaceWizardSelectNodePageRemoveButton,

	ControlID_NewWorkspaceWizardSelectLayoutPageSingleHorizontalButton,
	ControlID_NewWorkspaceWizardSelectLayoutPageSingleVerticalButton,
	ControlID_NewWorkspaceWizardSelectLayoutPageDoubleHorizontalButton,
	ControlID_NewWorkspaceWizardSelectLayoutPageDoubleVerticalButton,

//------------------------------------------------------------------------------
// Workspace Edit Widget
//------------------------------------------------------------------------------
	ControlID_WorkspaceEditWidgetSidebarNodeGroupChoice,
	ControlID_WorkspaceEditWidgetSidebarNodeNameList,
	ControlID_WorkspaceEditWidgetSidebarXAxisRemoveButton,
	ControlID_WorkspaceEditWidgetSidebarYAxisRemoveButton,
	ControlID_WorkspaceEditWidgetToolbarWorkspaceChoice,
	ControlID_WorkspaceEditWidgetToolbarNewWorkspaceButton,
	ControlID_WorkspaceEditWidgetToolbarNewWorkspaceWizardButton,
	ControlID_WorkspaceEditWidgetToolbarSaveWorkspaceButton,
	ControlID_WorkspaceEditWidgetToolbarRenameWorkspaceButton,
	ControlID_WorkspaceEditWidgetToolbarRemoveWorkspaceButton,
	ControlID_WorkspaceEditWidgetToolbarAddBackgroundButton,
	ControlID_WorkspaceEditWidgetToolbarAddViewportButton,
	ControlID_WorkspaceEditWidgetToolbarRemoveSelectedButton,
	ControlID_WorkspaceEditWidgetToolbarCloseButton,
	ControlID_WorkspaceEditWidgetToolbarChangeTextColourButton,
	ControlID_WorkspaceEditWidgetToolbarSetTabOrderButton,

	ControlID_WorkspaceEditWidgetViewportCameraChooserDlgCameraListBox,

//------------------------------------------------------------------------------
// Workspace Widget
//------------------------------------------------------------------------------
	ControlID_WorkspaceWidgetToolbarWorkspaceChoice,
	ControlID_WorkspaceWidgetToolbarEditWorkspaceButton,
	ControlID_WorkspaceWidgetToolbarInstanceWorkspaceButton,
	ControlID_WorkspaceWidgetToolbarModeChoice,
	ControlID_WorkspaceWidgetToolbarAutoselectCurvesToggle,
	ControlID_WorkspaceWidgetToolbarDisplayNamesToggle,
	ControlID_WorkspaceWidgetToolbarAnimModeAutokeyToggle,
	ControlID_WorkspaceWidgetToolbarAnimModeKeyAllButton,
	ControlID_WorkspaceWidgetToolbarAnimModePrevKeyButton,
	ControlID_WorkspaceWidgetToolbarAnimModeNextKeyButton,
	ControlID_WorkspaceWidgetToolbarSharedModeClearButton,
	ControlID_WorkspaceWidgetToolbarSharedModeLoadCurrentFrameButton,
	ControlID_WorkspaceWidgetToolbarCombinerModeCreateButton,
	ControlID_WorkspaceWidgetToolbarMappingModePhonemeChoice,
	ControlID_WorkspaceWidgetToolbarMappingModeApplyButton,
	
	MenuID_WorkspaceInstanceWidgetUndo,
	MenuID_WorkspaceInstanceWidgetRedo,
	MenuID_WorkspaceInstanceWidgetPrevFrame,
	MenuID_WorkspaceInstanceWidgetNextFrame,

//------------------------------------------------------------------------------
// Anim Curve Name Selection Dialog
//------------------------------------------------------------------------------

	ControlID_AnimCurveNameSelDlgNodeGroupChoice,
	ControlID_AnimCruveNameSelDlgTargetNameCombo,

//------------------------------------------------------------------------------
// Anim Manager Dialog
//------------------------------------------------------------------------------
	ControlID_AnimManagerDlgPrimaryAnimGroupChoice,
	ControlID_AnimManagerDlgNewGroupButton,
	ControlID_AnimManagerDlgDeleteGroupButton,
	ControlID_AnimManagerDlgPrimaryAnimListCtrl,
	ControlID_AnimManagerDlgDeleteAnimButton,
	ControlID_AnimManagerDlgCreateAnimButton,
	ControlID_AnimManagerDlgCurveManagerButton,
	ControlID_AnimManagerDlgMoveFromSecondaryToPrimaryButton,
	ControlID_AnimManagerDlgMoveFromPrimaryToSecondaryButton,
	ControlID_AnimManagerDlgCopyFromSecondaryToPrimaryButton,
	ControlID_AnimManagerDlgCopyFromPrimaryToSecondaryButton,
	ControlID_AnimManagerDlgSecondaryAnimGroupChoice,
	ControlID_AnimManagerDlgSecondaryAnimListCtrl,
	ControlID_AnimManagerDlgOKButton,

//------------------------------------------------------------------------------
// Bone Weight Dialog
//------------------------------------------------------------------------------
	ControlID_BoneWeightDlgWeightGrid,
	ControlID_BoneWeightDlgOKButton,

//------------------------------------------------------------------------------
// Command Panel
//------------------------------------------------------------------------------
	ControlID_CommandPanelCommandLineText,
	ControlID_CommandPanelCommandResultText,

//------------------------------------------------------------------------------
// Create Anim Wizard
//------------------------------------------------------------------------------
	ControlID_CreateAnimWizardSelectTypePageAnimTypeRadio,
	ControlID_CreateAnimWizardSelectTypePageImportAudioCheck,
	ControlID_CreateAnimWizardSelectDirectoryPageBrowseButton,
	ControlID_CreateAnimWizardSelectDirectoryPageGroupMethodRadio,
	ControlID_CreateAnimWizardSelectDirectoryPageGroupChoice,
	ControlID_CreateAnimWizardSelectAudioPageBrowseButton,
	ControlID_CreateAnimWizardSelectSoundNodeWavePageSoundNodeWaveListBox,
	ControlID_CreateAnimWizardSelectOptionsPageLanguageChoice,
	ControlID_CreateAnimWizardSelectOptionsPageSelectGestureConfigButton,
	ControlID_CreateAnimWizardSelectOptionsPageSelectCoarticulationConfigButton,
	ControlID_CreateAnimWizardSelectOptionsPageSpeechGesturesCheck,

//------------------------------------------------------------------------------
// Create Camera Dialog
//------------------------------------------------------------------------------
	ControlID_CreateCameraDlgBindToBoneCheck,
	ControlID_CreateCameraDlgBoneListBox,
	ControlID_CreateCameraDlgCreateButton,
	ControlID_CreateCameraDlgCancelButton,

//------------------------------------------------------------------------------
// Create Face Graph Node Dialog
//------------------------------------------------------------------------------
	ControlID_CreateFaceGraphNodeDlgCreateButton,
	ControlID_CreateFaceGraphNodeDlgCancelButton,

//------------------------------------------------------------------------------
// Curve Manager Dialog
//------------------------------------------------------------------------------
	ControlID_CurveManagerAnimGroupChoice,
	ControlID_CurveManagerAnimationChoice,
	ControlID_CurveManagerTopFilterChoice,
	ControlID_CurveManagerBottomFilterChoice,
	ControlID_CurveManagerTargetNamesListCtrl,
	ControlID_CurveManagerSourceNamesListCtrl,
	ControlID_CurveManagerRemoveCurvesButton,
	ControlID_CurveManagerAddCurrAnimButon,
	ControlID_CurveManagerAddCurrGroupButton,
	ControlID_CurveManagerAddAllAnimsButton,
	ControlID_CurveManagerOKButton,

//------------------------------------------------------------------------------
// Curve Properties Dialog
//------------------------------------------------------------------------------
	ControlID_CurvePropertiesDlgColourPreviewStaticBmp,
	ControlID_CurvePropertiesDlgChangeColourButton,
	ControlID_CurvePropertiesDlgOwnedByAnalysisCheck,

//------------------------------------------------------------------------------
// Face Graph Node Group Manager Dialog
//------------------------------------------------------------------------------
	ControlID_FaceGraphNodeGroupManagerDlgNodeTypeFilterChoice,
	ControlID_FaceGraphNodeGroupManagerDlgFaceGraphNodesListCtrl,
	ControlID_FaceGraphNodeGroupManagerDlgAddToGroupButton,
	ControlID_FaceGraphNodeGroupManagerDlgGroupChoice,
	ControlID_FaceGraphNodeGroupManagerDlgNewGroupButton,
	ControlID_FaceGraphNodeGroupManagerDlgDeleteGroupButton,
	ControlID_FaceGraphNodeGroupManagerDlgGroupNodesListCtrl,
	ControlID_FaceGraphNodeGroupManagerDlgRemoveFromGroupButton,
	ControlID_FaceGraphNodeGroupManagerDlgOKButton,

//------------------------------------------------------------------------------
// Free-Floating Frame
//------------------------------------------------------------------------------
	MenuID_FreeFloatingFrameFullscreen,
	MenuID_FreeFloatingFrameDock,
	MenuID_FreeFloatingFrameNewActor,
	MenuID_FreeFloatingFrameLoadActor,
	MenuID_FreeFloatingFrameSaveActor,
	MenuID_FreeFloatingFrameUndo,
	MenuID_FreeFloatingFrameRedo,
	MenuID_FreeFloatingFrameAlwaysOnTop,

//------------------------------------------------------------------------------
// Function Editor
//------------------------------------------------------------------------------
	MenuID_FunctionEditorResetView,
	MenuID_FunctionEditorSelectAll,
	MenuID_FunctionEditorDeleteSelection,
	MenuID_FunctionEditorAddKey,

//------------------------------------------------------------------------------
// Link Function Edit Dialog
//------------------------------------------------------------------------------
	ControlID_LinkFunctionEditDlgLinkFunctionChoice,
	ControlID_LinkFunctionEditDlgParameterGrid,
	ControlID_LinkFunctionEditDlgOKButton,
	ControlID_LinkFunctionEditDlgCancelButton,
	ControlID_LinkFunctionEditDlgApplyButton,
	ControlID_LinkFunctionEditDlgLabelStaticText,

//------------------------------------------------------------------------------
// Reanalyze Selection Dialog
//------------------------------------------------------------------------------
	ControlID_ReanalyzeSelectionDlgAnimTextEditBox,
	ControlID_ReanalyzeSelectionDlgBrowseForTextButton,
	ControlID_ReanalyzeSelectionDlgPlayAudioButton,
	ControlID_ReanalyzeSelectionDlgStopAudioButton,
	ControlID_ReanalyzeSelectionDlgReanalyzeButton,
	ControlID_ReanalyzeSelectionDlgCancelButton,

//------------------------------------------------------------------------------
// Coarticulation and Gesture Config Chooser Dlg
//------------------------------------------------------------------------------
	ControlID_CGConfigChooserDlgConfigNameChoice,
	ControlID_CGConfigChooserDlgDescriptionText,

//------------------------------------------------------------------------------
// Studio Options Dialog
//------------------------------------------------------------------------------
	ControlID_StudioOptionsDlgChangeColourMinButton,
	ControlID_StudioOptionsDlgChangeColourZeroButton,
	ControlID_StudioOptionsDlgChangeColourMaxButton,
	ControlID_StudioOptionsDlgChangeColourTextButton,
	ControlID_StudioOptionsDlgChangeColourFocusButton,
	ControlID_StudioOptionsDlgPhoneticAlphabetRadio,
	ControlID_StudioOptionsDlgRestoreDefaultButton,
	ControlID_StudioOptionsDlgOKButton,
	ControlID_StudioOptionsDlgApplyButton,
	ControlID_StudioOptionsDlgTimeGridFormatChoice,
	ControlID_StudioOptionsDlgTimelineGridFormatChoice,
	ControlID_StudioOptionsDlgChooseFontButton,

//------------------------------------------------------------------------------
// Bookkeeping
//------------------------------------------------------------------------------
	FaceFXResourceID_Last = ControlID_StudioOptionsDlgChooseFontButton,
	FaceFXResourceID_NumIDs = FaceFXResourceID_Last - FaceFXResourceID_First + 1
};

} // namespace Face

} // namespace OC3Ent

#endif

//------------------------------------------------------------------------------
// Base class for widgets.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxWidget.h"

namespace OC3Ent
{

namespace Face
{

FxWidget::FxWidget( FxWidgetMediator* mediator )
	: _mediator(mediator)
{
	if( _mediator )
	{
		_mediator->AddWidget(this);
	}
}

FxWidget::~FxWidget()
{
	if( _mediator )
	{
		_mediator->RemoveWidget(this);
	}
}

void FxWidget::OnAppStartup( FxWidget* FxUnused(sender) )
{
}

void FxWidget::OnAppShutdown( FxWidget* FxUnused(sender) )
{
}

void FxWidget::OnCreateActor( FxWidget* FxUnused(sender) )
{
}

void FxWidget::OnLoadActor( FxWidget* FxUnused(sender), const FxString& FxUnused(actorPath) )
{
}

void FxWidget::OnCloseActor( FxWidget* FxUnused(sender) )
{
}

void FxWidget::OnSaveActor( FxWidget* FxUnused(sender), const FxString& FxUnused(actorPath) )
{
}

void FxWidget::OnActorChanged( FxWidget* FxUnused(sender), FxActor* FxUnused(actor) )
{
}

void FxWidget::OnActorInternalDataChanged( FxWidget* FxUnused(sender), FxActorDataChangedFlag FxUnused(flags) )
{
}

void FxWidget::OnActorRenamed( FxWidget* FxUnused(sender) )
{
}

void FxWidget::OnQueryLanguageInfo( FxWidget* FxUnused(sender), FxArray<FxLanguageInfo>& FxUnused(languages) )
{
}

void FxWidget::
OnCreateAnim( FxWidget* FxUnused(sender), const FxName& FxUnused(animGroupName), FxAnim* FxUnused(anim) )
{
}

void FxWidget::
OnAudioMinMaxChanged( FxWidget* FxUnused(sender), FxReal FxUnused(audioMin), FxReal FxUnused(audioMax) )
{
}

void FxWidget::
OnReanalyzeRange( FxWidget* FxUnused(sender), FxAnim* FxUnused(anim), FxReal FxUnused(rangeStart), 
				  FxReal FxUnused(rangeEnd), const FxWString& FxUnused(analysisText) )
{
}

void FxWidget::
OnDeleteAnim( FxWidget* FxUnused(sender), const FxName& FxUnused(animGroupName), const FxName& FxUnused(animName) )
{
}

void FxWidget::OnDeleteAnimGroup( FxWidget* FxUnused(sender), const FxName& FxUnused(animGroupName) )
{
}

void FxWidget::
OnAnimChanged( FxWidget* FxUnused(sender), const FxName& FxUnused(animGroupName), FxAnim* FxUnused(anim) )
{
}

void FxWidget::
OnAnimPhonemeListChanged( FxWidget* FxUnused(sender), FxAnim* FxUnused(anim), FxPhonWordList* FxUnused(phonemeList) )
{
}

void FxWidget::OnPhonemeMappingChanged( FxWidget* FxUnused(sender), FxPhonemeMap* FxUnused(phonemeMap) )
{
}

void FxWidget::
OnAnimCurveSelChanged( FxWidget* FxUnused(sender), const FxArray<FxName>& FxUnused(selAnimCurves) )
{
}

void FxWidget::OnLayoutFaceGraph( FxWidget* FxUnused(sender) )
{
}

void FxWidget::OnAddFaceGraphNode( FxWidget* FxUnused(sender), FxFaceGraphNode* FxUnused(node) )
{
}

void FxWidget::OnRemoveFaceGraphNode( FxWidget* FxUnused(sender), FxFaceGraphNode* FxUnused(node) )
{
}

void FxWidget::
OnFaceGraphNodeSelChanged( FxWidget* FxUnused(sender), const FxArray<FxName>& FxUnused(selFaceGraphNodeNames), FxBool FxUnused(zoomToSelection) )
{
}

void FxWidget::OnFaceGraphNodeGroupSelChanged( FxWidget* FxUnused(sender), const FxName& FxUnused(selGroup) )
{
}

void FxWidget::
OnLinkSelChanged( FxWidget* FxUnused(sender), const FxName& FxUnused(inputNode), const FxName& FxUnused(outputNode) )
{
}

void FxWidget::OnRefresh( FxWidget* FxUnused(sender) )
{
}

void FxWidget::OnTimeChanged( FxWidget* FxUnused(sender), FxReal FxUnused(newTime) )
{
}

void FxWidget::OnPlayAnimation( FxWidget* FxUnused(sender), FxReal FxUnused(startTime),
								FxReal FxUnused(endTime), FxBool FxUnused(loop) )
{
}

void FxWidget::OnStopAnimation( FxWidget* FxUnused(sender) )
{
}

void FxWidget::OnAnimationStopped( FxWidget* FxUnused(sender) )
{
}

void FxWidget::OnSerializeOptions( FxWidget* FxUnused(sender), wxFileConfig* FxUnused(config), FxBool FxUnused(isLoading) )
{
}

void FxWidget::OnSerializeFaceGraphSettings( FxWidget* FxUnused(sender), FxBool FxUnused(isGetting), 
										     FxArray<FxNodePositionInfo>& FxUnused(nodeInfos), 
											 FxInt32& FxUnused(viewLeft), FxInt32& FxUnused(viewTop),
										     FxInt32& FxUnused(viewRight), FxInt32& FxUnused(viewBottom), 
											 FxReal& FxUnused(zoomLevel) )
{
}

void FxWidget::OnRecalculateGrid( FxWidget* FxUnused(sender) )
{
}

void FxWidget::OnQueryRenderWidgetCaps( FxWidget* FxUnused(sender), FxRenderWidgetCaps& FxUnused(renderWidgetCaps) )
{
}

void FxWidget::OnInteractEditWidget( FxWidget* FxUnused(sender), FxBool FxUnused(isQuery), FxBool& FxUnused(shouldSave) )
{
}

void FxWidget::OnGenericCoarticulationConfigChanged( FxWidget* FxUnused(sender), FxCoarticulationConfig* FxUnused(config) )
{
}

void FxWidget::OnGestureConfigChanged( FxWidget* FxUnused(sender), FxGestureConfig* FxUnused(config) )
{
}

FxWidgetMediator::FxWidgetMediator()
{
}

FxWidgetMediator::~FxWidgetMediator()
{
}

void FxWidgetMediator::AddWidget( FxWidget* widget )
{
	_widgets.PushBack(widget);
}

void FxWidgetMediator::RemoveWidget( FxWidget* widget )
{
	FxSize widgetIndex = _widgets.Find(widget);
	if( FxInvalidIndex != widgetIndex )
	{
		_widgets.Remove(widgetIndex);
	}
}

void FxWidgetMediator::OnAppStartup( FxWidget* sender )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnAppStartup(sender);
			}
		}
	}
}

void FxWidgetMediator::OnAppShutdown( FxWidget* sender )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnAppShutdown(sender);
			}
		}
	}
}

void FxWidgetMediator::OnCreateActor( FxWidget* sender )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnCreateActor(sender);
			}
		}
	}
}

void FxWidgetMediator::OnLoadActor( FxWidget* sender, const FxString& actorPath )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnLoadActor(sender, actorPath);
			}
		}
	}
}

void FxWidgetMediator::OnCloseActor( FxWidget* sender )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnCloseActor(sender);
			}
		}
	}
}

void FxWidgetMediator::OnSaveActor( FxWidget* sender, const FxString& actorPath )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnSaveActor(sender, actorPath);
			}
		}
	}
}

void FxWidgetMediator::OnActorChanged( FxWidget* sender, FxActor* actor )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnActorChanged(sender, actor);
			}
		}
	}
}

void FxWidgetMediator::OnActorInternalDataChanged( FxWidget* sender, FxActorDataChangedFlag flags )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnActorInternalDataChanged(sender, flags);
			}
		}
	}
}

void FxWidgetMediator::OnActorRenamed( FxWidget* sender )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnActorRenamed(sender);
			}
		}
	}
}

void FxWidgetMediator::OnQueryLanguageInfo( FxWidget* sender, FxArray<FxLanguageInfo>& languages )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnQueryLanguageInfo(sender, languages);
			}
		}
	}
}

void FxWidgetMediator::
OnCreateAnim( FxWidget* sender, const FxName& animGroupName, FxAnim* anim )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnCreateAnim(sender, animGroupName, anim);
			}
		}
	}
}

void FxWidgetMediator::
OnAudioMinMaxChanged( FxWidget* sender, FxReal audioMin, FxReal audioMax )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnAudioMinMaxChanged(sender, audioMin, audioMax);
			}
		}
	}
}

void FxWidgetMediator::
OnReanalyzeRange( FxWidget* sender, FxAnim* anim, FxReal rangeStart, 
				  FxReal rangeEnd, const FxWString& analysisText )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnReanalyzeRange(sender, anim, rangeStart, rangeEnd, analysisText);
			}
		}
	}
}

void FxWidgetMediator::
OnDeleteAnim( FxWidget* sender, const FxName& animGroupName, const FxName& animName )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnDeleteAnim(sender, animGroupName, animName);
			}
		}
	}
}

void FxWidgetMediator::OnDeleteAnimGroup( FxWidget* sender, const FxName& animGroupName )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnDeleteAnimGroup(sender, animGroupName);
			}
		}
	}
}

void FxWidgetMediator::
OnAnimChanged( FxWidget* sender, const FxName& animGroupName, FxAnim* anim )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnAnimChanged(sender, animGroupName,anim);
			}
		}
	}
}

void FxWidgetMediator::
OnAnimPhonemeListChanged( FxWidget* sender, FxAnim* anim, FxPhonWordList* phonemeList )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnAnimPhonemeListChanged(sender, anim, phonemeList);
			}
		}
	}
}

void FxWidgetMediator::
OnPhonemeMappingChanged( FxWidget* sender, FxPhonemeMap* phonemeMap )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnPhonemeMappingChanged(sender, phonemeMap);
			}
		}
	}
}

void FxWidgetMediator::
OnAnimCurveSelChanged( FxWidget* sender, const FxArray<FxName>& selAnimCurveNames )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnAnimCurveSelChanged(sender, selAnimCurveNames);
			}
		}
	}
}

void FxWidgetMediator::OnLayoutFaceGraph( FxWidget* sender )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnLayoutFaceGraph(sender);
			}
		}
	}
}

void FxWidgetMediator::OnAddFaceGraphNode( FxWidget* sender, FxFaceGraphNode* node )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnAddFaceGraphNode(sender, node);
			}
		}
	}
}

void FxWidgetMediator::OnRemoveFaceGraphNode( FxWidget* sender, FxFaceGraphNode* node )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnRemoveFaceGraphNode(sender, node);
			}
		}
	}
}

void FxWidgetMediator::
OnFaceGraphNodeSelChanged( FxWidget* sender, const FxArray<FxName>& selFaceGraphNodeNames, FxBool zoomToSelection )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnFaceGraphNodeSelChanged(sender, selFaceGraphNodeNames, zoomToSelection);
			}
		}
	}
}

void FxWidgetMediator::OnFaceGraphNodeGroupSelChanged( FxWidget* sender, const FxName& selGroup )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnFaceGraphNodeGroupSelChanged(sender, selGroup);
			}
		}
	}
}

void FxWidgetMediator::
OnLinkSelChanged( FxWidget* sender, const FxName& inputNode, const FxName& outputNode )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnLinkSelChanged(sender, inputNode, outputNode);
			}
		}
	}
}

void FxWidgetMediator::OnRefresh( FxWidget* sender )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnRefresh(sender);
			}
		}
	}
}

void FxWidgetMediator::OnTimeChanged( FxWidget* sender, FxReal newTime )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] )
		{
			if( _widgets[i] )
			{
				_widgets[i]->OnTimeChanged(sender, newTime);
			}
		}
	}
}

void FxWidgetMediator::OnPlayAnimation( FxWidget* sender, FxReal startTime,
										FxReal endTime, FxBool loop )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( _widgets[i] && sender != _widgets[i] )
		{
			_widgets[i]->OnPlayAnimation(sender, startTime, endTime, loop);
		}
	}
}

void FxWidgetMediator::OnStopAnimation( FxWidget* sender )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( _widgets[i] && sender != _widgets[i] )
		{
			_widgets[i]->OnStopAnimation(sender);
		}
	}
}

void FxWidgetMediator::OnAnimationStopped( FxWidget* sender )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( _widgets[i] && sender != _widgets[i] )
		{
			_widgets[i]->OnAnimationStopped(sender);
		}
	}
}

void FxWidgetMediator::OnSerializeOptions( FxWidget* sender, wxFileConfig* config, FxBool isLoading )
{
	// Propagate.
	if( config )
	{
		for( FxSize i = 0; i < _widgets.Length(); ++i )
		{
			if( sender != _widgets[i] )
			{
				if( _widgets[i] )
				{
					_widgets[i]->OnSerializeOptions(sender, config, isLoading);
				}
			}
		}
	}
}

// Called to get/set options in the face graph.
void FxWidgetMediator::OnSerializeFaceGraphSettings( FxWidget* sender, FxBool isGetting, 
											FxArray<FxNodePositionInfo>& nodeInfos, 
											FxInt32& viewLeft, FxInt32& viewTop,
											FxInt32& viewRight, FxInt32& viewBottom, 
											FxReal& zoomLevel )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] && _widgets[i] )
		{
			_widgets[i]->OnSerializeFaceGraphSettings(sender, isGetting, nodeInfos,
				viewLeft, viewTop, viewRight, viewBottom, zoomLevel);
		}
	}
}

// Called when the widgets should recalculate their griding, if the have any.
void FxWidgetMediator::OnRecalculateGrid( FxWidget* sender )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] && _widgets[i] )
		{
			_widgets[i]->OnRecalculateGrid(sender);
		}
	}
}

// Called to query the current render widget's capabilities.
void FxWidgetMediator::OnQueryRenderWidgetCaps( FxWidget* sender, FxRenderWidgetCaps& renderWidgetCaps )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] && _widgets[i] )
		{
			_widgets[i]->OnQueryRenderWidgetCaps(sender, renderWidgetCaps);
		}
	}
}

// Called to interact with the workspace edit widget.
void FxWidgetMediator::OnInteractEditWidget( FxWidget* sender, FxBool isQuery, FxBool& shouldSave )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] && _widgets[i] )
		{
			_widgets[i]->OnInteractEditWidget(sender, isQuery, shouldSave);
		}
	}
}

// Called when the generic coarticulation configuration has changed.
void FxWidgetMediator::OnGenericCoarticulationConfigChanged( FxWidget* sender, FxCoarticulationConfig* config )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] && _widgets[i] )
		{
			_widgets[i]->OnGenericCoarticulationConfigChanged(sender, config);
		}
	}
}

// Called when the gestures configuration has changed.
void FxWidgetMediator::OnGestureConfigChanged( FxWidget* sender, FxGestureConfig* config )
{
	// Propagate.
	for( FxSize i = 0; i < _widgets.Length(); ++i )
	{
		if( sender != _widgets[i] && _widgets[i] )
		{
			_widgets[i]->OnGestureConfigChanged(sender, config);
		}
	}
}

} // namespace Face

} // namespace OC3Ent

class ActorComponent extends Component
    abstract
    native
    noexport;

var native const transient Pointer Scene{FSceneInterface};
var const transient Actor Owner;
var native const transient bool bAttached;
var const bool bTickInEditor;
var const transient bool bNeedsReattach;
var const transient bool bNeedsUpdateTransform;
var native transient bool bDisableTick;
var native transient bool bEnableUpdateChildComponents;
var const Object.ETickingGroup TickGroup;

// Export UActorComponent::execSetTickGroup(FFrame&, void* const)
native final function SetTickGroup(Object.ETickingGroup NewTickGroup);

// Export UActorComponent::execSetComponentRBFixed(FFrame&, void* const)
native final function SetComponentRBFixed(bool bFixed);

// Export UActorComponent::execForceUpdate(FFrame&, void* const)
native final function ForceUpdate(bool bTransformOnly);

// Export UActorComponent::execQueueReattach(FFrame&, void* const)
native final function QueueReattach(bool bTransformOnly);

// Export UActorComponent::execDetachFromAny(FFrame&, void* const)
native final function DetachFromAny();

defaultproperties
{
    TickGroup=TG_DuringAsyncWork
}

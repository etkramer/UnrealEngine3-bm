//------------------------------------------------------------------------------
// FaceFx Doxygen header.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxDoxygen_H__
#define FxDoxygen_H__

/// \mainpage FaceFX Documentation
///
/// \section whatsNew What's New?
/// - \subpage whatsNew172 "What's new in FaceFX 1.7.2"
/// - \subpage whatsNew17 "What's new in FaceFX 1.7"
/// - \subpage whatsNew161 "What's new in FaceFX 1.6.1"
/// 
/// \section understanding Understanding FaceFX
/// - \subpage understandingFaceFX "Understanding FaceFX"
/// - \subpage startSDK "Starting the SDK"
/// - \subpage memory "Using the memory management system"
/// - \subpage loadActor "Loading an actor"
///		- \ref object "Understanding the serialization system"
/// - \subpage facegraphEval "Evaluating the Face Graph"
///		- \ref faceGraph "Understanding the Face Graph"
///		- \ref registers "Procedural animation using Face Graph registers"
///		- \ref creatingCustomNodeTypes "Creating custom Face Graph nodes"
///		- \ref usingMorphTargets "Using morph targets"
/// - \ref blending "Requesting Blended Bones"
///		- \subpage coordinateSystem "Notes on the FaceFX coordinate system"
/// - \subpage actorInstanceSpecifics "Understanding actor instances"
/// - \subpage multithreading "Multithreading issues with FaceFX"
///
/// \section classes Important Functions and Classes
/// - \ref OC3Ent::Face::FxSDKStartup() "FxSDKStartup()"
/// - \ref OC3Ent::Face::FxActor "FxActor"
/// - \ref OC3Ent::Face::FxActorInstance "FxActorInstance"
/// - \ref OC3Ent::Face::FxLoadActorFromFile() "FxLoadActorFromFile()"
/// - \ref OC3Ent::Face::FxLoadActorFromMemory() "FxLoadActorFromMemory()"
/// - \ref OC3Ent::Face::FxLoadAnimSetFromFile() "FxLoadAnimSetFromFile()"
/// - \ref OC3Ent::Face::FxLoadAnimSetFromMemory() "FxLoadAnimSetFromMemory()"
/// - \ref OC3Ent::Face::FxName "FxName"
/// - \ref OC3Ent::Face::FxSDKShutdown() "FxSDKShutdown()"

/// \page whatsNew161 What's New in FaceFX 1.61
/// 
/// \section newInSDK What's new in the FaceFX SDK
/// FaceFX 1.61 is a bug fix release for FaceFX 1.6.
/// FaceFX 1.6 is mostly a performance optimization release for the FaceFX SDK.
/// Because of this, it was necessary to change some of the interfaces licensees
/// use.  While we have tried to limit the amount of licensee code changes that
/// would be required when updating to FaceFX 1.6, some were unavoidable, and 
/// we hope that the performance improvements will far outweigh the inconvenience.
///
/// \subsection newInSDKArchives FxArchives and Serialization Improvements
/// The serialization system was revamped to minimize the number of dynamic 
/// memory allocations, and thus the amount of fragmentation, during loading.
/// To accomplish this, an archive directory containing the archive's name table
/// and various other information is now at the beginning of every archive.
/// However, the addition of this directory has necessitated some changes to the
/// loading and saving code.
/// \note All current loading and saving code will compile, but <b><i>will 
/// not</i></b> run.  It must be changed.
///
/// As of FaceFX 1.6, we provide high-level loading and saving functions.
/// Please replace all direct usage of \ref OC3Ent::Face::FxArchive "FxArchive"
/// with the appropriate high-level loading functions.
/// - \ref OC3Ent::Face::FxLoadActorFromFile() "FxLoadActorFromFile()"
/// - \ref OC3Ent::Face::FxLoadActorFromMemory() "FxLoadActorFromMemory()"
/// - \ref OC3Ent::Face::FxLoadAnimSetFromFile() "FxLoadAnimSetFromFile()"
/// - \ref OC3Ent::Face::FxLoadAnimSetFromMemory() "FxLoadAnimSetFromMemory()"
///
/// If you have a tool that saves actors or anim sets, it will need to be 
/// updated as well.  Check the high-level saving functions.
/// - \ref OC3Ent::Face::FxSaveActorToFile() "FxSaveActorToFile()"
/// - \ref OC3Ent::Face::FxSaveActorToMemory() "FxSaveActorToMemory()"
/// - \ref OC3Ent::Face::FxSaveAnimSetToFile() "FxSaveAnimSetToFile()"
/// - \ref OC3Ent::Face::FxSaveAnimSetToMemory() "FxSaveAnimSetToMemory()"
/// 
/// For more information, please see \ref loadActor "Loading an actor"
///
/// \subsection newInSDKNames FxNames
/// The name table was significantly optimized in this release.  
/// \ref OC3Ent::Face::FxName "FxNames" are now hashed to determine if and where
/// they exist in the name table, making creating names a much less intensive
/// process.
/// \note The constructor of \ref OC3Ent::Face::FxName "FxName" no longer takes 
/// an \ref OC3Ent::Face::FxString "FxString."  It now takes a \p const \p FxChar*.
///
/// Thus, all places where you constructed a name that took a string will need
/// to be changed to pass in a \p const \p FxChar*.  If constructing from an
/// FxString, call \ref OC3Ent::Face::FxString::GetData() "FxString::GetData()" 
/// to retrieve the underlying \p const \p FxChar*.  Otherwise, determine how to
/// get a pointer to a C-style ASCII string.
///
/// \subsection newInSDKMemory Memory System
/// The memory system has been rewritten.  The allocators have been replaced by
/// an \ref OC3Ent::Face::FxMemoryAllocationPolicy "FxMemoryAllocationPolicy",
/// which is a wrapper for a couple simple options and function pointers for 
/// changing the behavior of memory allocation and release.
/// \note Any allocators you have written will need to be converted to the new
/// function pointer style, and the call to \ref OC3Ent::Face::FxSDKStartup() "FxSDKStartup()"
/// might need to be updated.
/// 
/// This change was introduced to provide a more robust method of overriding
/// FaceFX's memory allocations, and to remove unnecessary virtual function 
/// calls.  For more information, please see 
/// \ref memory "Using the memory management system"
///
/// \subsection newInSDKBoneBlending Face Graph Update and Bone Blending
/// To provide better performance, two important changes were made in the actor
/// instance update module.
/// \note You must now call \ref OC3Ent::Face::FxActorInstance::UpdateBonePoses() "FxActorInstance::UpdateBonePoses()"
/// after the call to \ref OC3Ent::Face::FxActorInstance::Tick() "FxActorInstance::Tick()"
/// 
/// This new call evaluates all the bone pose nodes and caches their values,
/// providing a nice performance boost for the actual bone blending.
///
/// The call to retrieve a blended bone has been changed so it no longer returns 
/// an FxBoneDelta.
/// \note Update all calls to \ref OC3Ent::Face::FxActorInstance::GetBone() "FxActorInstance::GetBone()".
///
/// \ref OC3Ent::Face::FxActorInstance::GetBone() "FxActorInstance::GetBone()" now
/// fills out a position, rotation, scale and bone weight, eliminating the 
/// construction of the FxBoneDelta.  For more information, please see
/// \ref facegraphEval "Evaluating the Face Graph"
///
/// \subsection newInSDKConclusion Conclusion
/// These are the changes in FaceFX 1.6 that will require a licensee to update 
/// their integration code.  For an exhaustive list of all the changes made in 
/// FaceFX 1.6, please see the release notes in $SourceRoot/Documentation/ReleaseNotes

/// \page whatsNew17 What's New in FaceFX 1.7
/// 
/// FaceFX 1.7 is a major optimization release. The %Face Graph has been completely
/// overhauled to significantly improve its performance on next-gen consoles by
/// eliminating the graph structure in memory, thereby greatly improving its
/// cache coherency. This new structure is called the Compiled %Face Graph.
///
/// Any calls to FxActor::GetFaceGraph() will fail to compile. Licensees should 
/// switch to using FxActor::GetCompiledFaceGraph() in all game code.
///
/// Unfortunately, this task is likely to affect licensee code. If you were using
/// any old-style %Face Graph iteration,
/// \code
/// FxFaceGraph::Iterator curr = actor->GetFaceGraph().Begin(FxFaceGraphNode::StaticClass());
/// FxFaceGraph::Iterator end  = actor->GetFaceGraph().End(FxFaceGraphNode::StaticClass());
/// while( curr != end )
/// {
///		//...
///		++curr;
/// }
/// \endcode
///
/// It will need to be replaced with the new style of %Face Graph iteration.
/// \code
/// FxCompiledFaceGraph& compiledGraph = actor->GetCompiledFaceGraph();
/// FxSize numNodes = compiledGraph.nodes.Length();
/// for( FxSize i = 0; i < numNodes; ++i )
/// {
///		//...
/// }
/// \endcode
///
/// Iteration over a certain type of node is easy. In the loop above, just check
/// if the node is the type you're looking for.
/// \code 
/// for( FxSize i = 0; i < numNodes; ++i )
/// {
///		if( compiledGraph.nodes[i].nodeType == NT_MorphTarget )
///		{
///			//...
///		}
/// }
/// \endcode
/// 
/// Check the \ref OC3Ent::Face::FxCompiledFaceGraph "Compiled Face Graph"
/// documentation for more details. Customers with source code access can see
/// our implementation of this in FxRenderWidgetOC3.cpp.
///
/// All of the FxGenericTargetProxy functionality is gone, and is replaced
/// with a user data system. You are now free to define your own user data 
/// structs and attach it to a compiled face graph node via the void* pUserData.
/// Note if you attach user data to a compiled face graph node, you are 
/// responsible for cleaning up after yourself. Since the compiled face graph 
/// might be recompiled (very rare!), you will need to use the callbacks
/// provided, the most useful being the pre-compilation callback. The other 
/// three may be set to NULL if you don't want to use their functionality.
///
/// Note that UpdateBonePoses() has been removed, and will break your
/// compilation.
/// 
/// Finally, FxAnimPlayer has been removed, and its functionality has been
/// incorporated directly into FxActorInstance. Where before, to play an 
/// animation you had to call
/// \code
/// _actorInstance->GetAnimPlayer().Play(animName, groupName);
/// \endcode
/// now, you interact directly with the actor instance.
/// \code
/// _actorInstance->PlayAnim(animName, groupName);
/// \endcode
///
/// Likewise for checking if it is playing and stopping an animation. Formerly,
/// you would write
/// \code
/// if( _actorInstance->GetAnimPlayer().IsPlaying() )
/// {
///     _actorInstance->GetAnimPlayer().Stop();
/// }
/// \endcode
/// and now you would write
/// \code
/// if( _actorInstance->IsPlayingAnim() )
/// {
///     _actorInstance->StopAnim();
/// }
/// \endcode

/// \page whatsNew172 What's New in FaceFX 1.7.2
/// 
/// \section decoupling SDK/File Version Decoupling
/// The most significant change for licensees in FaceFX 1.7.2 is the decoupling
/// of the file version and the SDK version. With this change, the SDK checks 
/// the file format version stored in the archive, rather than the SDK version
/// stored in the archive. This change will allow OC3 Entertainment to release
/// new versions of the SDK and Studio that don't impact serialization at all,
/// and files saved by the newer version can still be loaded in an older 
/// version with the same file version.
///
/// \section phonemes Extended Phoneme List
/// Also added to 1.7.2 are more phonemes, about 20 in all, to have a greater
/// coverage of the phonemes that aren't necessarily in English, but are in
/// our other supported languages. The order of the phoneme list was also 
/// revamped to something a little more standards-compliant, though this is only
/// useful to know if you've hard-coded phoneme enum values anywhere. The 
/// phoneme enum value names were changed to break compilation, to ensure that
/// no subtle bugs go unnoticed, from PHONEME_* to PHON_*.
///
/// \section mapping Default Mapping
/// The default mapping in 1.7.2 was changed to a hybrid target/component-based
/// model. This was done to telegraph the direction we're planning on taking
/// FaceFX. Your old mappings will still work just fine with 1.7.2, and an
/// automatic process will update your old mappings so the new phonemes are
/// intellegently mapped based on your old mapping. We strongly suggest going
/// with the new hybrid mapping, however. It has fewer targets, but looks almost
/// identical, making it more efficient for your artists to get up to speed on.
///
/// \section other Other Changes
/// Several bugs were fixed, including an important fix to registers correct
/// their functionality. For a more complete list of changes, features, and
/// additions, please check the main FaceFX documentation, which includes 
/// changes to Studio and the tools. Be sure to check out the anim playback
/// sample, which now has an interface for learning/testing/playing with
/// the register system.

/// \page understandingFaceFX Understanding FaceFX - SDK Overview
/// 
/// \section designBasics Design Basics
/// During in-game usage, the FaceFX SDK works on data stored in a FaceFX actor file. 
/// These files have the extension “fxa”. Contained in an fxa file are all of the facial 
/// animations for a character, as well as the character’s %Face Graph and master bone list. 
/// Together, these provide all the data necessary for playing back the canned animations, 
/// and also procedurally generating realistic, expressive facial animations in 
/// response to game play events. An artist will create a self-contained actor 
/// archive for each character in the game using FaceFX Studio, the facial 
/// animation editor provided with the FaceFX SDK, and the provided tools for 
/// exporting poses from 3dsmax or Maya.
/// 
/// The \ref faceGraph "Face Graph" is the foundation of the FaceFX system. The 
/// %Face Graph is a directed acyclic graph created in FaceFX Studio.  
/// It builds complex relationships between various targets that the artist 
/// has defined for a character’s face. 
/// The %Face Graph is evaluated once per actor instance per frame. During the 
/// evaluation, if there is an animation playing for the actor, the
/// \ref animation "animation is evaluated" and the curve values are placed into 
/// the appropriate nodes in the %Face Graph. 
/// By using the register system, dynamically-generated values may be 
/// placed into the %Face Graph to have the character change its facial expression 
/// in response to events in the game.  
/// After the values propagate through the graph, the app 
/// \ref blending "requests the final position of each bone" in the face and 
/// sets the transforms in the skeleton.
///
/// The SDK comes with platform-independent \ref support "support code" and an
/// \ref object "object serialization system", used to create the fxa files.
/// 
/// \section integrationBasics Integration Basics
/// There are a couple of functions to call to \ref startSDK "start up and shut down the SDK".  
/// You will also need to \ref loadActor "load the actors" for the in-game characters. 
/// The majority of the SDK’s integration, however, will go into your per-frame 
/// character updates.  This is where the \ref facegraphEval "Face Graph is evaluated" and bone 
/// transformations calculated.  Check any of the links for some sample code.

/// \defgroup animation Animation Playback
/// \brief The animation playback system.
///
/// \par Keys
/// The \ref OC3Ent::Face::FxAnimKey "FxAnimKey" is a time, value, incoming slope
/// and outgoing slope.  It is the fundamental building block of animations in
/// FaceFX.
///
/// \par Curves
/// The \ref OC3Ent::Face::FxAnimCurve "FxAnimCurve" is a series of FxAnimKeys.  It supports
/// two interpolation methods, linear and a modified Hermite.  When using linear
/// interpolation, the FxAnimKey's incoming and outgoing slopes are unused.
/// However, they are necessary during the Hermite interpolation.
/// 
/// \par Curve Evaluation
/// An FxAnimCurve is linked to \ref OC3Ent::Face::FxFaceGraphNode "FxFaceGraphNode" with 
/// the same name during \ref OC3Ent::Face::FxActor::Link() "FxActor::Link()" via
/// a cached pointer in the curve.  This is done so that each curve can 
/// quickly access the corresponding node in the %Face Graph and set the track 
/// value each time the curve is evaluated.
/// 
/// \par Animations
/// An \ref OC3Ent::Face::FxAnim "FxAnim" groups several FxAnimCurves into a single animation,
/// which allows all related curves to be evaluated as a single group.  When 
/// \ref OC3Ent::Face::FxAnim::Tick() "FxAnim::Tick()"
/// is called, each of the FxAnimCurves in the FxAnim is evaluated for that specific
/// time, and their values are automatically placed in the correct nodes in the
/// %Face Graph.  This is the first thing that happens during an 
/// \ref OC3Ent::Face::FxActorInstance::Tick() "FxActorInstance::Tick()" call.
/// 
/// \par Animation Groups
/// FxAnims belong to an \ref OC3Ent::Face::FxAnimGroup "FxAnimGroup" which allows 
/// related animations to be grouped together for organizational purposes.  

/// \defgroup faceGraph The FaceFX Face Graph 
/// \brief A directed acyclic graph for modeling complex interactions
///
/// \par Overview
/// The %Face Graph is the brains of the FaceFX SDK.  It allows you to model the 
/// complex interactions that are necessary for realistic facial animations, but
/// still have simple controls for your character so creating those animations is
/// as quick as possible.  The %Face Graph is a directed acyclic graph, meaning that
/// each edge has a direction and cycles are not allowed to exist in the graph.  
/// Also, the %Face Graph does not allow duplicate edges between two nodes.  The graph
/// is composed of nodes and links between those nodes.  In addition, each node has
/// a value, a single floating-point number.  This value is some combination of 
/// \li The value of the corresponding \ref OC3Ent::Face::FxAnimCurve "curve"
/// \li The value of the node's input links
/// \li The value set dynamically into the node each frame
///
/// After all the values have been combined together in some way, the resulting value
/// is clamped between the node's minimum and maximum values, to prevent a situation
/// where the node might be improperly under- or over-driven.
///
/// \par Nodes
/// The fundamental element in the %Face Graph is a 
/// \ref OC3Ent::Face::FxFaceGraphNode "FxFaceGraphNode".  In fact, in the simplest of face
/// graphs, links are not necessary.  If you only wanted to build the 15 speech
/// targets and have your character do some basic lip-synched animations, for example,
/// your %Face Graph would have 15 \ref OC3Ent::Face::FxBonePoseNode "FxBonePoseNodes" with
/// no links between them.  These nodes would be driven directly with the curves
/// generated by the analysis.
///
/// \par
/// While that provides satisfactory facial animations for some applications, it
/// belies the real power in the %Face Graph.  In fact, you can create a node type
/// for just about anything you want.  In our samples, you can see we've created a
/// morph target proxy using the FxGenericTargetNode, 
/// which allows vertex-based animation rather than bones-based
/// animations.  If your engine supports it, you can have both bones-based and 
/// vertex-based deformations applied to the same mesh each frame.  In addition to 
/// the geometry-based nodes, you could create a material property proxy for the 
/// FxGenericTargetNode, which would drive a parameter in a material.  
/// In short, if your engine supports it, you can drive it with the %Face Graph.
/// 
/// \par Combining Target Nodes
/// While you could animate each of those nodes directly, it is generally both
/// useful and logical to "combine" several nodes into a single target.  For example,
/// you might have a lip corners out bone pose, a lip corners up bone pose, 
/// a jaw open bone pose, an eyelid squint bone pose, and a normal map for the 
/// crinkles at the eye corners.  It would be useful to be able to have a single 
/// control for the logical grouping of those into a "smile".  By creating a 
/// \ref OC3Ent::Face::FxCombinerNode "FxCombinerNode" named "Smile" and linking with
/// the appropriate weighting to each of those targets, you would then have a 
/// way of \e indirectly controlling those multiple independent targets with a 
/// single manipulator, be it in animations created with FaceFX Studio or 
/// procedural animations as the character responds to in-game events.  An FxCombinerNode
/// is different from the "target nodes" in the sense that it carries no additional
/// data, like the bone pose node.  Its sole purpose is to "combine" other nodes
/// according to the way they are linked together.
///
/// \par Linking Nodes
/// When a node requests the value of its inputs, it does so using the array of 
/// \ref OC3Ent::Face::FxFaceGraphNodeLink "FxFaceGraphNodeLinks" it contains.  
/// \ref OC3Ent::Face::FxFaceGraphNodeLink::GetTransformedValue() "GetTransformedValue()" 
/// takes the value from the output node, transforms it by some function f(x), known as
/// the \ref OC3Ent::Face::FxLinkFn "link function".  This value is then either summed or 
/// multiplied with the other inputs to the node - this is a property of most nodes
/// and can be changed programmatically or in FaceFX Studio.
/// 
/// \par Link Functions
/// A link function defines what is done with the value of an output node as it
/// flows to the input node.  The default behavior is a simple 
/// \ref OC3Ent::Face::FxLinearLinkFn "linear link function", where the value of the output
/// node has a weighting applied to it - the old equation f(x) = m*x + b.  However,
/// you can do much more complicated links.  If you wanted a target to ramp up slower
/// in relation to its linear-linked counterparts, you could use the \ref OC3Ent::Face::FxCubicLinkFn
/// "cubic link function", f(x) = a*x^3.  If there is some behavior that can't be
/// adequately captured by these simple (but fast) mathematical functions, you can 
/// use the \ref OC3Ent::Face::FxCustomLinkFn "custom link function" to define practically
/// any relationship between the two nodes.
///
/// \par %Face Graph Evaluation
/// <small>Please note that this section assumes you have not added custom node types to the graph.</small>
/// \par
/// During each frame in game, the relevant portions of the %Face Graph will be evaluated.
/// Only the nodes pertaining to the information you need will be calculated, thanks
/// to the graph traversal method.  At the start of the frame, during
/// \ref OC3Ent::Face::FxActorInstance::Tick() "FxActorInstance::Tick()",
/// the curves are evaluated and their values placed in the corresponding nodes.
/// Next, during \ref OC3Ent::Face::FxActorInstance::BeginFrame() "FxActorInstance::BeginFrame()"
/// the registers are updated and their values placed into the nodes they drive.
/// 
/// \par
/// Once the pre-frame setup is complete, the client begins requesting the output of
/// the %Face Graph, either fully-blended bones or floating-point values 
/// directly from the nodes.  First, the bone pose nodes need to be updated.
/// This is done by calling 
/// \ref OC3Ent::Face::FxActorInstance::UpdateBonePoses() "FxActorInstance::UpdateBonePoses()"
/// directly after \ref OC3Ent::Face::FxActorInstance::BeginFrame() "FxActorInstance::BeginFrame()".
/// When the client requests a bone transformation via
/// \ref OC3Ent::Face::FxActorInstance::GetBone() "FxActorInstance::GetBone()", the application
/// determines which FxBonePoseNodes the bone is a part of by consulting the
/// \ref OC3Ent::Face::FxMasterBoneList "FxMasterBoneList", and requests the value of each
/// of those nodes.  Those values are used to calculate the final transformation.
/// More details of the bone blending step can be found in the \ref blending "Bone Blending"
/// section of the documentation.
///
/// \par
/// If you have added your own FxGenericTargetNode types, this would be the time to iterate
/// through them and \ref OC3Ent::Face::FxGenericTargetNode::UpdateTarget() "update the targets".
/// Also keep in mind the \ref actorInstanceSpecifics "interactions between FxActorInstance, FxActor, and FxFaceGraph" because
/// you might need to switch what data the Face Graph nodes deal with on a per actor instance 
/// basis.
///
/// \par
/// Finally, during the \ref OC3Ent::Face::FxActorInstance::EndFrame() "FxActorInstance::EndFrame()" call,
/// the actor instance caches the values of the nodes which have an associated register, and the 
/// %Face Graph evaluation is completed

/// \defgroup blending Bone Blending
/// \brief Blends between bone poses based on results evaluated in the %Face Graph.
///
/// \par Requesting a Blend Operation
/// Requests for blended bones must be made after the instance is ticked and the frame begun.
/// Then, for each bone in actor instance, you can request a blended bone.
/// \dontinclude SampleCode.h
/// \skip Set the correct animation values into the face graph.
/// \until GetBone
/// The method fills out a position, rotation, and scale component, along with 
/// a bone weight which was set by the artist in FaceFX Studio for blending FaceFX
/// animations with underlying skeletal animations.
/// 
/// \par Associating FaceFX Bones With Your Bones
/// FaceFX provides a client index system to help you associate your bones with FaceFX bones in the
/// master bone list contained in the actor.  This client index is saved to the actor archive with each
/// reference bone for convenience.  To set the client indices, go through the master bone list.  
/// To get the client indices, go through the actor instance:
/// \dontinclude SampleCode.h
/// \skip Sets reference bone 0's client index to 25.
/// \until GetBoneClientIndex(0);
/// If you choose to use this system, it lets you write your bone transformation update loop in a more
/// straightforward way:
/// \code
/// // Pseudo code:
/// foreach bone in the actor instance
/// {
///     trigger a blend operation for this bone
///     get this bone's client index in your skeletal system
///     set this bone's transform in your skeletal system directly using the client index
/// }
/// \endcode
/// For a more detailed example of an actor update loop, please see \ref facegraphEval "Face Graph evaluation".
///
/// See also:
///	- \ref coordinateSystem "Notes on the FaceFX coordinate system"


/// \defgroup support Supporting Code
/// \brief Data structures and platform abstractions.
///
/// The supporting code are simple utility classes and functions that aren't 
/// directly involved with facial animation.

/// \defgroup object Object Serialization
/// \brief Serializes objects and pointers to objects in a cross-platform manner.
///
/// \par Archive System Overview
/// FaceFX stores data in a binary file called an \ref OC3Ent::Face::FxArchive "archive".  It is important
/// to note that there is no set format for an archive in FaceFX.  An archive is simply a binary 
/// repository that can store primitive types as well as any \ref OC3Ent::Face::FxObject "FxObject"
/// derived objects.  FaceFX archives also have the ability to store pointers to \ref OC3Ent::Face::FxObject "FxObject"
/// derived objects.  An archive must always be associated with a valid \ref OC3Ent::Face::FxArchiveStore "archive store"
/// such as an \ref OC3Ent::Face::FxArchiveStoreFile "FxArchiveStoreFile" or an \ref OC3Ent::Face::FxArchiveStoreMemory "FxArchiveStoreMemory".
/// In theory it is even possible to create a network archive store class and serialize complex object-oriented
/// data over a network connection if that ability is ever required.  For an example of loading from an archive file,
/// see the \ref loadActor "loading actors" example.  Note that the "patriarch object" (the object triggering the
/// serialization - the \ref OC3Ent::Face::FxActor "FxActor" in the \ref loadActor "loading actors" example) must match the
/// "patriarch object" in the archive.  For example, you can not save an archive by triggering serialization on an
/// \ref OC3Ent::Face::FxActor "FxActor" object and then expect to load that archive by triggering serialization on an
/// \ref OC3Ent::Face::FxFaceGraph "FxFaceGraph" object.
/// 
/// \par Building FaceFX Archives Into Your Game Engine
/// Typically, FaceFX users will simply opt to keep FaceFX archives on disc as separate files.  More advanced
/// integrations may want to couple the game engine more tightly with FaceFX.  One reason is to support linear
/// loading on game consoles to prevent DVD seek time when loading FaceFX actors.  To support such integrations,
/// FaceFX provides a \ref OC3Ent::Face::FxArchiveStoreMemory "memory archive store".  Using such an archive store,
/// you can write out FaceFX data as a chunk of bytes in-place in your own files and read them back at load time
/// through the FaceFX archive system.  Typically in linear loading situations, the game engine will store everything 
/// needed for a level in one big file, with all unneeded data stripped out via a pre-baking process.  You
/// can also use FaceFX in this way by using memory archives, not using lazy loading on the archive, and pre-baking
/// your FaceFX data in the same way, then embedding the FaceFX raw bytes directly into your file format.  See
/// Console Information below for further console-specific optimizations.
///
/// \par Recommended Usage
/// As of FaceFX 1.6, several functions are provided to encapsulate the process of loading and saving actors and 
/// animation sets.  We recommend you replace your old loading code with these new function calls.
///
/// For actors:
/// \li \ref OC3Ent::Face::FxLoadActorFromFile() "FxLoadActorFromFile()"
/// \li \ref OC3Ent::Face::FxSaveActorToFile() "FxSaveActorToFile()"
/// \li \ref OC3Ent::Face::FxLoadActorFromMemory() "FxLoadActorFromMemory()"
/// \li \ref OC3Ent::Face::FxSaveActorToMemory() "FxSaveActorToMemory()"
///
/// For anim sets:
/// \li \ref OC3Ent::Face::FxLoadAnimSetFromFile() "FxLoadAnimSetFromFile()"
/// \li \ref OC3Ent::Face::FxSaveAnimSetToFile() "FxSaveAnimSetToFile()"
/// \li \ref OC3Ent::Face::FxLoadAnimSetFromMemory() "FxLoadAnimSetFromMemory()"
/// \li \ref OC3Ent::Face::FxSaveAnimSetToMemory() "FxSaveAnimSetToMemory()"
///
/// \par Console Information
/// Interested console developers might find it useful to know that all FaceFX archives are stored in Little Endian
/// byte order.  \ref OC3Ent::Face::FxArchive "FxArchive" performs the task of swapping the byte order when loading
/// archives on Big Endian architectures.  However, this might not be optimal on Big Endian console machines.
/// In that case, it will be ideal to pre-bake the archive into a more console-friendly format with the byte order
/// already swapped by using the FxCooker command-line tool.
///
/// \par Further Reading
/// The FaceFX object serialization subsystem is based in part on NASA's PObject serialization
/// system.  For more information, NASA has 
/// <a href="http://www.grc.nasa.gov/WWW/price000/pfc/htm/pobject_serial.html">detailed documentation</a>
/// on the PObject serialization system.

/// \page startSDK FaceFX SDK Startup/Shutdown
/// Before you can use any other functions in the FaceFX SDK, you should
/// initialize the SDK.  This is a simple operation, just one call to 
/// \ref OC3Ent::Face::FxSDKStartup() "FxSDKStartup()".
/// \dontinclude Tests.cpp
/// \skip Start up the FaceFX SDK with its default allocation policy.
/// \until FxSDKStartup
/// The SDK initialization ensures that the \ref OC3Ent::Face::FxName "name table"
/// is created and sets the allocation policy that the memory system will use for the 
/// duration of execution.  For more information about allocation policies, please read
/// the page about \ref memory "the FaceFX memory system".
/// 
/// Likewise, when you are finished using functions from the FaceFX SDK, you should
/// shut down the SDK.  This is equally simple.  Just call 
/// \ref OC3Ent::Face::FxSDKShutdown() "FxSDKShutdown()".
/// \dontinclude Tests.cpp
/// \skip Shut down the FaceFX SDK.
/// \until FxSDKShutdown
///
/// The termination cleans up the name table and makes sure that 
/// all managed \ref OC3Ent::Face::FxArchive "archives" are 
/// \ref OC3Ent::Face::FxArchive::UnmountAllArchives() "unmounted and destroyed".
/// 
/// You must call \ref OC3Ent::Face::FxSDKStartup() "FxSDKStartup()" and 
/// \ref OC3Ent::Face::FxSDKShutdown() "FxSDKShutdown()" exactly once per 
/// process execution.

/// \page loadActor Loading an Actor
/// To load an actor from a file on disk, call FxLoadActorFromFile().
///
/// \b Sample: Linear load an actor from a file on disk.
/// \dontinclude SampleCode.h
/// \skip Loads an actor from the file
/// \until FxLoadActorFromFile
///
/// Passing \p FxTrue as the third parameter to the function tells the function
/// to use \ref OC3Ent::Face::FxArchiveStoreFileFast "FxArchiveStoreFileFast"
/// and the archive in \p AM_LinearLoad mode, ensuring the fastest possible load
/// times from disk.
///
/// If you have your own platform-specific file reading code, or want to
/// embed FaceFX assets into your own file formats, you can use 
/// FxLoadActorFromMemory() to serialize from a memory chunk.
///
/// \b Sample: Linear load an actor from a memory region.
/// \dontinclude SampleCode.h
/// \skip Loads an actor from a chunk in memory
/// \until }
///
/// FxLoadActorFromMemory() serializes from a chunk of memory without making
/// an internal copy of the memory.  This is the fastest way to serialize from 
/// memory.
///
/// Linear loading ensures that all data is read sequentially.  The other archive
/// loading mode is lazy loading.  As of version 1.5, this has been superseded by
/// the anim sets.  Now the animation groups, which were formerly lazy loaded from
/// the main actor's file, are separated into their own archives on disk, and
/// can be loaded and attached to any actor.  For more information on 
/// anim sets, please see \ref OC3Ent::Face::FxAnimSet "FxAnimSet" documentation.
///
/// It is important to note that all \ref OC3Ent::Face::FxActorInstance "actor instances" of a particular 
/// \ref OC3Ent::Face::FxActor "actor" should refer to the same actor in memory.  Otherwise the point of
/// the actor instance system is defeated.  Therefore, when loading actors in an in-game situation, FaceFX 
/// provides some convenient functionality for making sure that the same actor data is not loaded multiple times.  
/// If you have given your actors descriptive names in FaceFX Studio and have access to this name when loading
/// the in-game asset that the actor belongs to, this process is painless.  First, before
/// loading the actor, make sure that the actor is not already loaded by calling
/// \ref OC3Ent::Face::FxActor::FindActor() "FxActor::FindActor()".  If the actor is found, there is no need to load the actor
/// file from disc and you can simply create a new actor instance and set its actor pointer to the pointer returned
/// from \ref OC3Ent::Face::FxActor::FindActor() "FxActor::FindActor()".  If NULL is returned, the actor is not
/// in the actor management system and the actor archive should be loaded from disc.  After loading the actor,
/// add it to the actor management system by calling \ref OC3Ent::Face::FxActor::AddActor() "FxActor::AddActor()".
/// When changing levels in your game, you might want to call \ref OC3Ent::Face::FxActor::FlushActors() "FxActor::FlushActors()"
/// to flush out the actor management system completely and reclaim memory for actors that may not exist in the next level,
/// or you can call \ref OC3Ent::Face::FxActor::RemoveActor() "FxActor::RemoveActor()" to remove specific actors directly.
/// Take a look at an example loading scheme for your game:
///
/// \dontinclude SampleCode.h
/// \skip Loads FaceFX actors for GameX
/// \until } // GameX_
/// 
/// For a detailed explanation of the archive and object serialization system and how
/// you can build it into your game engine, please see \ref object "object serialization".

/// \page facegraphEval Evaluating the Face Graph
/// Once per frame, if an animation is playing or if you want to use the system
/// to procedurally generate facial animation, you will need to update the face
/// graph, tick the system and get the results.
/// 
/// For each actor instance to be updated, the code is as follows.
/// \dontinclude SampleCode.h
/// \skip Evaluating the Face Graph.
/// \until End Face Graph eval example
/// 
/// This is obviously a very simplistic example. In your implementation, you probably
/// will have the actor instance embedded as a member of the actual "actor"
/// in your game. However, there are a few things to note. The 
/// \ref OC3Ent::Face::FxName "name" is created once, rather than each frame. 
/// Name creation is a somewhat expensive task.  It must hash the name and then
/// resolve any potential collisions.  It may give better performance to cache
/// the names, depending on the situation.
/// Also, note that the audio time is initialized to -1.0f. This ensures that if
/// there is no audio playing for whatever reason, the animation will still be 
/// updated correctly, using the app time as a fallback.
/// 
/// There is a special FaceFX mode of operation worth mentioning.  The %Face Graph is very powerful
/// and has lot of different applications, some of which are not really facial animation related.  It
/// is also useful to be able to occasionally control a character's face even when there is no facial
/// animation playing on the character.  To support such operations, there are two functions
/// in \ref OC3Ent::Face::FxActorInstance "FxActorInstance": \ref OC3Ent::Face::FxActorInstance::SetAllowNonAnimTick()
/// "FxActorInstance::SetAllowNonAnimTick()" and \ref OC3Ent::Face::FxActorInstance::GetAllowNonAnimTick()
/// "FxActorInstance::GetAllowNonAnimTick()".  When you call SetAllowNonAnimTick(FxTrue), it allows you to call Tick() 
/// on the actor instance even without a currently playing facial animation.  To support this feature, the actor instance 
/// update loop above should be changed to look like so:
///
/// \dontinclude SampleCode.h
/// \skip Do work only if the actor instance is playing a facial animation or non anim ticks are allowed
/// \until }
/// 
/// This allows the programmer to control the %Face Graph via the %Face Graph register system.  This is a powerful
/// tool and allows characters to respond emotionally to in-game stimuli.  See \ref OC3Ent::Face::FxActorInstance::SetRegister()
/// "FxActorInstance::SetRegister()" and \ref OC3Ent::Face::FxActorInstance::GetRegister() "FxActorInstance::GetRegister()".
///
/// See also:
///	- \subpage registers "Procedural animation using Face Graph registers"
///	- \subpage creatingCustomNodeTypes "Creating custom Face Graph nodes"
///	- \subpage usingMorphTargets "Using morph targets"


/// \page coordinateSystem FaceFX Coordinate System Notes
/// The FaceFX Sample Rendering Engine defines its own coordinate system, which the FaceFX .fxa exporters from 3dsmax and Maya
/// use by default.  For most licensees, it is ok to keep the FaceFX actor's bone poses in this coordinate system.  Some engines
/// however use 3dsmax' or Maya's coordinate system for the skeletons in the game.  For this reason, FaceFX provides special
/// "native coordinates" versions of the .fxa exporters as well as the FxUpdateActor command line tool.  Documentation on those
/// can be found in the FaceFX .chm documentation file installed along with FaceFX Studio.
///
/// It is important to note that it is impossible to tell from the .fxa file alone which coordinate system the actor is in.  It is
/// up to the developer to use the correct tools and know this information.  If you are using the "native coordinates" plug-ins, the 
/// coordinates vary from .fxa to .fxa depending on what animation package it was exported from.  That is, files from Maya are in Maya's 
/// coordinate system and files from 3dsmax are in 3dsmax' coordinate system.  If you are not using "native coordinates" then the .fxa 
/// files are in FaceFX' Sample Rendering Engine coordinates: +X to the right, +Y up, +Z in.  For example, to go from Maya's coordinate 
/// system to FaceFX' Sample Rendering Engine coordinates, you would negate the x component of the bone's position vector and rotation 
/// quaternion.  To go from 3dsmax' coordinate system to FaceFX' Sample Rendering Engine coordinates, you would swap the y and z 
/// components of the bone's position vector and rotation quaternion.
///
/// .fxa files exported in "native coordinates" cannot be loaded into FaceFX Studio correctly unless of course you are a full licensee with
/// source code access and have replaced the FaceFX Sample Rendering Engine window with your own game engine.  You can use the FxUpdateActor 
/// command-line tool to convert the .fxa files back and forth.
///
/// It is also worth noting here that the \ref OC3Ent::Face::FxBoneDelta "FxBoneDelta" that pops out from 
/// \ref OC3Ent::Face::FxMasterBoneList::GetBlendedBone() "FxMasterBoneList::GetBlendedBone()" and 
/// \ref OC3Ent::Face::FxActorInstance::GetBone() "FxActorInstance::GetBone()" is in 'parent space' -- that is 
/// each bone's coordinate frame is relative to its parent's transformation.  If you are exporting with the true "native coordinates" version 
/// of the plug-ins, then you will need to adjust your coordinate remapping to account for whether the character was exported from Maya 
/// or 3dsmax.

/// \page actorInstanceSpecifics FxActorInstance, FxActor, and FxFaceGraph Interaction
/// An actor “instance” (\ref OC3Ent::Face::FxActorInstance "FxActorInstance") is just that: an instance of a particular actor 
/// (\ref OC3Ent::Face::FxActor "FxActor") in the system that can cache its current state.  All actor “instances” of a particular actor share 
/// the same underlying actor structure such that only one copy of the actual actor is in memory at any given time.  The actor instances share 
/// this actor resource and evaluate the %Face Graph in the actor independently and sequentially (one after the other, never simultaneously).  
/// The %Face Graph is evaluated once for each actor instance because each instance could be playing completely different 
/// animations -- i.e., the instances share the %Face Graph resource via the shared actor, but they do not share the same %Face 
/// Graph state.  The \ref OC3Ent::Face::FxActorInstance::Tick() "FxActorInstance::Tick()" call grabs control of the %Face Graph in the shared 
/// actor resource for that particular instance, so in the update loop for each instance it is safe to query the face graph itself (you can get a 
/// pointer to the shared \ref OC3Ent::Face::FxActor "FxActor" through the \ref OC3Ent::Face::FxActorInstance "FxActorInstance" where you can 
/// then access the %Face Graph for that actor). Be sure to access the %Face Graph through \ref OC3Ent::Face::FxActor::GetCompiledFaceGraph "FxActor::GetCompiledFaceGraph()", 
/// since the decompiled %Face Graph is only used in tools, not in game.

/// \page creatingCustomNodeTypes Creating Custom Face Graph Node Types
/// To create a custom Face Graph node type, you must first derive your new class from \ref OC3Ent::Face::FxGenericTargetNode "FxGenericTargetNode" 
/// and at a minimum implement \ref OC3Ent::Face::FxGenericTargetNode::Clone() "Clone()", \ref OC3Ent::Face::FxGenericTargetNode::CopyData() "CopyData()", 
/// and \ref OC3Ent::Face::FxGenericTargetNode::Serialize() "Serialize()".  You will also need to edit FxSDK.cpp and add a line in the 
/// \ref OC3Ent::Face::FxSDKStartup() "FxSDKStartup()" function to initialize your new class.  For example, you may #include "FxMyNewNodeType.h" 
/// at the top of FxSDK.cpp and add FxMyNewNodeType::StaticClass(); in FxSDKStartup() if your new node class is named FxMyNewNodeType.  If you 
/// want users to be able to directly add nodes of the new type to the Face Graph in FaceFX Studio, you must set 
/// \ref OC3Ent::Face::FxFaceGraphNode::_isPlaceable "_isPlaceable" to FxTrue in the constructor of the new node type.  
///
/// Next you must decide if your node needs custom properties that are editable in FaceFX Studio.  
/// If so, you will need to use the Node User Property mechanism.  To add a \ref OC3Ent::Face::FxFaceGraphNodeUserProperty "user property" 
/// to your custom node type, simply create the property with the \ref OC3Ent::Face::FxFaceGraphNodeUserPropertyType "proper type", name, 
/// and default value and add it in the constructor of the new node type by calling 
/// \ref OC3Ent::Face::FxFaceGraphNode::AddUserProperty() "AddUserProperty()".
/// It is always a good idea to cache the index of specific properties by using a #define in the code to avoid searching for
/// the property by name in-game each time you need to retrieve it.  It is worth noting that for this to work you must be a
/// full licensee of FaceFX with access to the FaceFX SDK and Studio source code as well as the 3dsmax and Maya plug-in source 
/// code because those things need to be recompiled with a new version of the SDK containing the new node types.
///
/// Now that you have a functioning Face Graph node complete with user properties, you must create a custom proxy class that
/// will provide the glue between the Face Graph node and the component of your game that the node should control.  To do this
/// simply derive a new class from \ref OC3Ent::Face::FxGenericTargetProxy "FxGenericTargetProxy".  Note that you must implement 
/// \ref OC3Ent::Face::FxGenericTargetProxy::Copy "Copy()", \ref OC3Ent::Face::FxGenericTargetProxy::Update() "Update()", and 
/// \ref OC3Ent::Face::FxGenericTargetProxy::Destroy() "Destroy()" in your new class.  Update() is what is called from your new node type's 
/// \ref OC3Ent::Face::FxGenericTargetNode::UpdateTarget() "UpdateTarget()" function and in it you will want to take
/// the passed in value and plug it directly into the component of your game that the node is controlling.  Also, this proxy class
/// should not be at the FaceFX SDK level.  Instead, it should be a class in your game code to keep FaceFX from knowing any specifics
/// about your game engine.  Due to this, its class initialization (FxMyNewTargetProxy::StaticClass()) should be called somewhere during
/// the initialization step of your game engine.  This is so that the 3dsmax and Maya plug-ins can still load FaceFX actor files that 
/// contain your new node type, without having to drag all of your game engine code into their compile process.  Think of these proxy 
/// objects as "plug-ins" to the custom node types that are only present when running inside of your game engine.  At all other times, the
/// custom node types' "plug-in" slots remain empty.
///
/// There is a built-in mechanism to alert your new nodes that user properties have been changed in FaceFX Studio.  If you 
/// implement \ref \ref OC3Ent::Face::FxFaceGraphNode::ReplaceUserProperty() "ReplaceUserProperty()" in your new node type, it will be called 
/// whenever the user changes a user property in FaceFX Studio which will allow you to "re-link" your proxy if the new user property would 
/// require it.  This "re-linking" logic is also provided for you at the FxGenericTargetNode level via 
/// \ref OC3Ent::Face::FxGenericTargetNode::ShouldLink() "ShouldLink()" and \ref OC3Ent::Face::FxGenericTargetNode::SetShouldLink() "SetShouldLink()", 
/// though if you implement ReplaceUserProperty() in your new node class you will want to be sure to call Super::ReplaceUserProperty() first 
/// thing and also set _shouldLink to FxTrue.
///
/// The proxy objects you create are never serialized into archives.  They must be created and linked up to the Face Graph nodes
/// during initialization.  To do this, simply use the Face Graph node iteration mechanism to iterate over all nodes of your new
/// type and add a proxy object to each one.  By default, FxGenericTargetNode sets _shouldLink to FxTrue in its constructor so
/// all custom node types are requesting a "re-link" during initialization.  This creation and "re-linking" step has been traditionally
/// implemented in the Face Graph evaluation loop for each actor instance in your game.  
///
/// Please see \ref usingMorphTargets "how morph targets are implemented" for a detailed example of this with code samples.

/// \page usingMorphTargets Using Morph Targets
/// FaceFX does not store any morph target specific data.  For example, there is no way to retrieve morph target vertex positions
/// from FaceFX.  The actual morphing constructs and systems should be implemented in your game engine before you attempt to use
/// FaceFX to drive morph targets.
///
/// Before continuing with this section it is a good idea to review the material covered in 
/// \ref creatingCustomNodeTypes "Creating Custom Face Graph Node Types" to familiarize yourself with the FaceFX SDK constructs
/// presented below.
///
/// FaceFX provides a high-level morphing construct via the \ref OC3Ent::Face::FxMorphTargetNode "FxMorphTargetNode" %Face Graph node type.  
/// This node type is derived from \ref OC3Ent::Face::FxGenericTargetNode "FxGenericTargetNode" and is simply a container for the morph target 
/// name user property.  When FaceFX Studio loads an actor file with the sample rendering engine, and that actor file's corresponding rendering 
/// archive file (.fxr) contains morph targets, it will automatically add FxMorphTargetNodes to the %Face Graph of the actor.  The FaceFX 
/// sample rendering engine has its own morph target system so it implements a special FxMorphTargetProxy derived from 
/// \ref OC3Ent::Face::FxGenericTargetProxy "FxGenericTargetProxy".  However, you can use these high-level FxMorphTargetNodes in the 
/// %Face Graph to drive your own game engine's morph target system by simply providing your own FxGenericTargetProxy derived class to hook 
/// into your game engine's morph target system.
///
/// FxMorphTargetNode (and the corresponding sample rendering engine specific FxMorphTargetProxy) provide a good example of creating
/// custom %Face Graph node types complete with node user properties, linking, and notification when a user property is changed
/// in FaceFX Studio.
///
/// Take a look at how FxMorphTargetNode is defined:
/// \include FxMorphTargetNode.h
///
/// Now take a look at the FxMorphTargetNode implementation from the FaceFX SDK source code:
/// \include FxMorphTargetNode.cpp
///
/// Pretty simple stuff.  This implements a new node type derived from FxGenericTargetNode, adds some 
/// \ref OC3Ent::Face::FxFaceGraphNodeUserProperty "user properties" that are automatically editable in FaceFX Studio, and implements 
/// \ref OC3Ent::Face::FxFaceGraphNode::ReplaceUserProperty() "ReplaceUserProperty()" to receive notification that the user has
/// changed a property from within FaceFX Studio.
/// 
/// All that is needed now is an engine-specific proxy class derived from FxGenericTargetProxy to glue our new %Face Graph node type
/// to our engine-specific morph target structures.  FxMorphTargetProxy is from the FaceFX Studio source code and glues the high-level
/// FxMorphTargetNodes to the FaceFX Sample Rendering Engine's morphing system.
/// \dontinclude FxRenderWidgetOC3.cpp
/// \skip class FxMorphTargetProxy : 
/// \until FX_IMPLEMENT_CLASS(FxMorphTargetProxy, 
///
/// Once the new node type and proxy have been implemented, they must be hooked up and linked to the appropriate in-engine 
/// objects.  The traditional place for all of this to happen is when updating actor instances and evaluating the Face Graph.
/// Take a look at this code example direct from the FaceFX Studio source code.  This is how morph targets work in FaceFX Studio
/// with the FaceFX Sample Rendering Engine.  Keep in mind that FaceFX Studio does not need to use \ref OC3Ent::Face::FxActorInstance "FxActorInstances"
/// because at the present time only one \ref OC3Ent::Face::FxActor "FxActor" can be loaded at any given time.  The same techniques apply
/// for in-game uses with actor instances; just grab the actor pointer from the actor instance 
/// (see \ref OC3Ent::Face::FxActorInstance::GetActor() "FxActorInstance::GetActor()") and use it instead of _actor in the below code.
/// \dontinclude FxRenderWidgetOC3.cpp
/// \skip // Update any morph target nodes that may be in the Face Graph.
/// \until // Continue with any other Face Graph processing and bone blending.
/// 

/// \page memory FaceFX Memory Management
/// FaceFX handles all its dynamic memory allocation requests through function
/// pointers.  This allows a programmer to specify an alternate method of 
/// memory allocation and release.
///
/// FaceFX requires that an 
/// \ref OC3Ent::Face::FxMemoryAllocationPolicy "FxMemoryAllocationPolicy" be
/// passed to \ref OC3Ent::Face::FxSDKStartup() "FxSDKStartup()".  In the
/// memory allocation policy are all the memory allocation options that can be
/// set, including whether to use new/delete, malloc/free, or custom function
/// pointers to allocate/release memory.
///
/// If going through global operator new/delete or malloc/free is sufficient for
/// your game, no function pointers need be filled out in the memory allocation
/// policy.  Simply set the allocation type to \p MAT_FreeStore for new/delete
/// or \p MAT_Heap for malloc/free.
///
/// To use custom allocation routines, set the allocation type to \p MAT_Custom,
/// and provide three function pointers, one for allocation, another for 
/// allocation with a custom description tag (for debugging only), and one for
/// freeing.  Their signatures, along with more information, can be found in
/// the \ref OC3Ent::Face::FxMemoryAllocationPolicy "FxMemoryAllocationPolicy"
/// documentation.
///
/// Also provided in the memory allocation policy is an \p FxBool parameter for
/// whether or not to allow FaceFX to do its own small-object memory pooling.
/// This is by default off.  It is off by default because in order to safely 
/// allow FaceFX to do memory pooling, your integration code must be written
/// in such a way so that it is guaranteed that no FaceFX objects are created
/// before \ref OC3Ent::Face::FxSDKStartup() "FxSDKStartup()", and that no
/// FaceFX objects are destroyed after 
/// \ref OC3Ent::Face::FxSDKStartup() "FxSDKShutdown()".  If that is the case, 
/// using the built-in small-object memory pool can result in a nice little
/// performance boost, particularly during loading actors and anim sets.
///
/// The FaceFX memory system can do some rudimentary memory leak detection, 
/// bounds checking, and stats tracking, implemented by overallocating during 
/// allocation requests and prepending some allocation information.  This is 
/// mainly useful in debug builds.  If you would like to turn this feature on
/// modify FxPlatform.h and uncomment the #define FX_TRACK_MEMORY_STATS and 
/// then rebuild the FaceFX SDK.

/// \page registers Procedural Face Graph Animation with Registers
/// FaceFX 1.5 introduced a much-improved register system for actor instances.
/// Prior to FaceFX 1.5, registers needed to be set every frame to continue 
/// influencing the instance's %Face Graph.  Now, once a register's value is
/// set, it will remain at that value until it is set to a new value, or 
/// "unset".  This change makes it much easier to use the register system to
/// procedurally drive the %Face Graph.
///
/// \b Sample: Making an actor instance happy.
/// \dontinclude SampleCode.h
/// \skip Making a character happy
/// \until SetRegister(registerName, 0.0f, RO_None)
///
/// The above sample will pop if the actor is in view when the register is set,
/// since it will immediately start adding 1 to the current value of Happy
/// the very next time the character is rendered. However, also introduced in 
/// FaceFX 1.5 is the ability for registers to internally 
/// handle interpolation between their current value and a destination value,
/// allowing for a "fire-and-forget" method of procedural animation.  This can
/// be used in several different ways, such as adding in an effect, or removing
/// an effect from a precanned animation. 
/// 
/// \b Sample: Making an actor instance happy, smoothly.
/// \dontinclude SampleCode.h
/// \skip Making a character happy, smoothly
/// \until SetRegister("Happy", 0.0f, RO_Add, 1.0f);
/// 
/// \b Sample: Removing an effect from a precanned animation.
/// \dontinclude SampleCode.h
/// \skip Removing an effect from a canned animation
/// \until SetRegister("Godfather", 0.0f, RO_Multiply);
/// Multiplying the node value by 0 will remove the effect from the animation.
/// 
/// Also added is a new function call, 
/// \ref OC3Ent::Face::FxActorInstance::SetRegisterEx() "FxActorInstance::SetRegisterEx()", which allows even more
/// possibilities for procedural facial animation.  SetRegisterEx() takes the
/// desired register value and interpolation time just like 
/// \ref OC3Ent::Face::FxActorInstance::SetRegister() "FxActorInstance::SetRegister()", but
/// also the "next" register value and interpolation time, allowing for two
/// segment animation (effectively, three keys, with the first (time, value)
/// pair being the current value and time.
/// 
/// \b Sample: Triggering a dynamic blink.
/// \dontinclude SampleCode.h
/// \skip Triggering a dynamic blink
/// \until SetRegisterEx("Blink", RO_Add, 1.0f, 0.125f, RO_Add, 0.0f, 0.083f);
///
/// Also, registers should be set before 
/// \ref OC3Ent::Face::FxActorInstance::Tick() "FxActorInstance::Tick()" is called
/// for a given frame, because FxActorInstance::Tick() is where the interpolation
/// is performed and the values loaded into the %Face Graph.

/// \page multithreading FaceFX and Multithreading
/// FaceFX is not threadsafe and is not multithreaded internally.  If you are
/// using FaceFX in a multithreaded environment, ensure you use FaceFX in a 
/// single threaded fashion.

#endif

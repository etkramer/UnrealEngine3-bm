/*=============================================================================
	UnNames.h: Header file registering global hardcoded Unreal names.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*-----------------------------------------------------------------------------
	Macros.
-----------------------------------------------------------------------------*/

// Define a message as an enumeration.
#ifndef REGISTER_NAME
	#define REGISTER_NAME(num,name) NAME_##name = num,
	#define REGISTERING_ENUM
	enum EName {
#endif

/*-----------------------------------------------------------------------------
	Hardcoded names which are not messages.
-----------------------------------------------------------------------------*/

// Special zero value, meaning no name.
REGISTER_NAME(   0, None             )

// Class property types
REGISTER_NAME(   1, ByteProperty     )
REGISTER_NAME(   2, IntProperty      )
REGISTER_NAME(   3, BoolProperty     )
REGISTER_NAME(   4, FloatProperty    )
REGISTER_NAME(   5, ObjectProperty   )
REGISTER_NAME(   6, NameProperty     )
REGISTER_NAME(   7, DelegateProperty )
REGISTER_NAME(   8, ClassProperty    )
REGISTER_NAME(   9, ArrayProperty    )
REGISTER_NAME(  10, StructProperty   )
REGISTER_NAME(  11, VectorProperty   )
REGISTER_NAME(  12, RotatorProperty  )
REGISTER_NAME(  13, StrProperty      )
REGISTER_NAME(  14, MapProperty      )
REGISTER_NAME(	15,	InterfaceProperty)

// Packages.
REGISTER_NAME(  20, Core			 )
REGISTER_NAME(  21, Engine			 )
REGISTER_NAME(  22, Editor           )
REGISTER_NAME(  23, Gameplay         )

// UnrealScript types.
REGISTER_NAME(  80, Byte		     )
REGISTER_NAME(  81, Int			     )
REGISTER_NAME(  82, Bool		     )
REGISTER_NAME(  83, Float		     )
REGISTER_NAME(  84, Name		     )
REGISTER_NAME(  85, String		     )
REGISTER_NAME(  86, Struct			 )
REGISTER_NAME(  87, Vector		     )
REGISTER_NAME(  88, Rotator	         )
REGISTER_NAME(  89, SHVector         )
REGISTER_NAME(  90, Color            )
REGISTER_NAME(  91, Plane            )
REGISTER_NAME(  92, Button           )
REGISTER_NAME(  94, Matrix			 )
REGISTER_NAME(	95,	LinearColor		 )
REGISTER_NAME(	96, QWord			 )
REGISTER_NAME(	97, Pointer			 )
REGISTER_NAME(	98, Double			 )
REGISTER_NAME(	99, Quat			 )

// Keywords.
REGISTER_NAME( 100, Begin			 )
REGISTER_NAME( 102, State            )
REGISTER_NAME( 103, Function         )
REGISTER_NAME( 104, Self             )
REGISTER_NAME( 105, TRUE             )
REGISTER_NAME( 106, FALSE            )
REGISTER_NAME( 107, Transient        )
REGISTER_NAME( 117, Enum			 )
REGISTER_NAME( 119, Replication      )
REGISTER_NAME( 120, Reliable         )
REGISTER_NAME( 121, Unreliable       )
REGISTER_NAME( 122, Always           )
REGISTER_NAME( 123, Server			 )
REGISTER_NAME( 124, Client			 )

// Object class names.
REGISTER_NAME( 150, Field            )
REGISTER_NAME( 151, Object           )
REGISTER_NAME( 152, TextBuffer       )
REGISTER_NAME( 153, Linker           )
REGISTER_NAME( 154, LinkerLoad       )
REGISTER_NAME( 155, LinkerSave       )
REGISTER_NAME( 156, Subsystem        )
REGISTER_NAME( 157, Factory          )
REGISTER_NAME( 158, TextBufferFactory)
REGISTER_NAME( 159, Exporter         )
REGISTER_NAME( 160, StackNode        )
REGISTER_NAME( 161, Property         )
REGISTER_NAME( 162, Camera           )
REGISTER_NAME( 163, PlayerInput      )
REGISTER_NAME( 164, Actor			 )
REGISTER_NAME( 165, ObjectRedirector )
REGISTER_NAME( 166, ObjectArchetype  )

// Constants.
REGISTER_NAME( 600, Vect)
REGISTER_NAME( 601, Rot)
REGISTER_NAME( 605, ArrayCount)
REGISTER_NAME( 606, EnumCount)
REGISTER_NAME( 607, Rng)
REGISTER_NAME( 608, NameOf)

// Flow control.
REGISTER_NAME( 620, Else)
REGISTER_NAME( 621, If)
REGISTER_NAME( 622, Goto)
REGISTER_NAME( 623, Stop)
REGISTER_NAME( 625, Until)
REGISTER_NAME( 626, While)
REGISTER_NAME( 627, Do)
REGISTER_NAME( 628, Break)
REGISTER_NAME( 629, For)
REGISTER_NAME( 630, ForEach)
REGISTER_NAME( 631, Assert)
REGISTER_NAME( 632, Switch)
REGISTER_NAME( 633, Case)
REGISTER_NAME( 634, Default)
REGISTER_NAME( 635, Continue)

// Variable overrides.
REGISTER_NAME( 640, Private)
REGISTER_NAME( 641, Const)
REGISTER_NAME( 642, Out)
REGISTER_NAME( 643, Export)
REGISTER_NAME( 644, DuplicateTransient )
REGISTER_NAME( 645, NoImport )
REGISTER_NAME( 646, Skip)
REGISTER_NAME( 647, Coerce)
REGISTER_NAME( 648, Optional)
REGISTER_NAME( 649, Input)
REGISTER_NAME( 650, Config)
REGISTER_NAME( 651, EditorOnly)
REGISTER_NAME( 652, NotForConsole)
REGISTER_NAME( 653, EditConst)
REGISTER_NAME( 654, Localized)
REGISTER_NAME( 655, GlobalConfig)
REGISTER_NAME( 656, SafeReplace)
REGISTER_NAME( 657, New)
REGISTER_NAME( 658, Protected)
REGISTER_NAME( 659, Public)
REGISTER_NAME( 660, EditInline)
REGISTER_NAME( 661, EditInlineUse)
REGISTER_NAME( 662, Deprecated)
REGISTER_NAME( 663, Atomic)
REGISTER_NAME( 664, Immutable)
REGISTER_NAME( 665, Automated )
REGISTER_NAME( 666, RepNotify )
REGISTER_NAME( 667, Interp )
REGISTER_NAME( 668, NoClear )
REGISTER_NAME( 669, NonTransactional )
REGISTER_NAME( 670, EditFixedSize)
REGISTER_NAME( 940, ImmutableWhenCooked )
REGISTER_NAME( 941, RepRetry )

// Class overrides.
REGISTER_NAME( 671, Intrinsic)
REGISTER_NAME( 672, Within)
REGISTER_NAME( 673, Abstract)
REGISTER_NAME( 674, Package)
REGISTER_NAME( 675, Guid)
REGISTER_NAME( 676, Parent)
REGISTER_NAME( 677, Class)
REGISTER_NAME( 678, Extends)
REGISTER_NAME( 679, NoExport)
REGISTER_NAME( 680, Placeable)
REGISTER_NAME( 681, PerObjectConfig)
REGISTER_NAME( 682, NativeReplication)
REGISTER_NAME( 683, NotPlaceable)
REGISTER_NAME( 684,	EditInlineNew)
REGISTER_NAME( 685,	NotEditInlineNew)
REGISTER_NAME( 686,	HideCategories)
REGISTER_NAME( 687,	ShowCategories)
REGISTER_NAME( 688,	CollapseCategories)
REGISTER_NAME( 689,	DontCollapseCategories)
REGISTER_NAME( 698, DependsOn)
REGISTER_NAME( 699, HideDropDown)

REGISTER_NAME( 950, Implements )
REGISTER_NAME( 951, Interface )
REGISTER_NAME( 952, Inherits)
REGISTER_NAME( 953, PerObjectLocalized)
REGISTER_NAME( 954, NonTransient)
REGISTER_NAME( 955, Archetype)

REGISTER_NAME( 956, StrictConfig )
REGISTER_NAME( 957, UnusedStructKeyword1 )
REGISTER_NAME( 958, UnusedStructKeyword2 )

// State overrides.
REGISTER_NAME( 690, Auto)
REGISTER_NAME( 691, Ignores)

// Auto-instanced subobjects
REGISTER_NAME( 692, Instanced)

// Components.
REGISTER_NAME( 693, Component)
REGISTER_NAME( 694, Components)

// Calling overrides.
REGISTER_NAME( 695, Global)
REGISTER_NAME( 696, Super)
REGISTER_NAME( 697, Outer)

// Function overrides.
REGISTER_NAME( 700, Operator)
REGISTER_NAME( 701, PreOperator)
REGISTER_NAME( 702, PostOperator)
REGISTER_NAME( 703, Final)
REGISTER_NAME( 704, Iterator)
REGISTER_NAME( 705, Latent)
REGISTER_NAME( 706, Return)
REGISTER_NAME( 707, Singular)
REGISTER_NAME( 708, Simulated)
REGISTER_NAME( 709, Exec)
REGISTER_NAME( 710, Event)
REGISTER_NAME( 711, Static)
REGISTER_NAME( 712, Native)
REGISTER_NAME( 713, Invariant)
REGISTER_NAME( 714, Delegate)
REGISTER_NAME( 715, Virtual)
REGISTER_NAME( 716, NoExportHeader)
REGISTER_NAME( 717, UnusedFunctionKeyword1)
REGISTER_NAME( 718, UnusedFunctionKeyword2)
REGISTER_NAME( 719, UnusedFunctionKeyword3)

// Variable declaration.
REGISTER_NAME( 720, Var)
REGISTER_NAME( 721, Local)
REGISTER_NAME( 722, Import)
REGISTER_NAME( 723, From)

// Special commands.
REGISTER_NAME( 730, Spawn)
REGISTER_NAME( 731, Array)
REGISTER_NAME( 732, Map)
REGISTER_NAME( 733,	AutoExpandCategories)
REGISTER_NAME( 734, AutoCollapseCategories)
REGISTER_NAME( 735, DontAutoCollapseCategories)

// Misc.
REGISTER_NAME( 740, Tag)
REGISTER_NAME( 742, Role)
REGISTER_NAME( 743, RemoteRole)
REGISTER_NAME( 744, System)
REGISTER_NAME( 745, User)
REGISTER_NAME( 746, PersistentLevel)
REGISTER_NAME( 747, TheWorld)

// Platforms
REGISTER_NAME( 750, Windows)
REGISTER_NAME( 751, XBox)
REGISTER_NAME( 752, PlayStation)
REGISTER_NAME( 753, Linux)
REGISTER_NAME( 754, MacOSX)
REGISTER_NAME( 755, PC)

// Log messages.
REGISTER_NAME( 756, DevDecals)
REGISTER_NAME( 757, PerfWarning)
REGISTER_NAME( 758, DevStreaming)
REGISTER_NAME( 759, DevLive)
REGISTER_NAME( 760, Log)
REGISTER_NAME( 761, Critical)
REGISTER_NAME( 762, Init)
REGISTER_NAME( 763, Exit)
REGISTER_NAME( 764, Cmd)
REGISTER_NAME( 765, Play)
REGISTER_NAME( 766, Console)
REGISTER_NAME( 767, Warning)
REGISTER_NAME( 768, ExecWarning)
REGISTER_NAME( 769, ScriptWarning)
REGISTER_NAME( 770, ScriptLog)
REGISTER_NAME( 771, Dev)
REGISTER_NAME( 772, DevNet)
REGISTER_NAME( 773, DevPath)
REGISTER_NAME( 774, DevNetTraffic)
REGISTER_NAME( 775, DevAudio)
REGISTER_NAME( 776, DevLoad)
REGISTER_NAME( 777, DevSave)
REGISTER_NAME( 778, DevGarbage)
REGISTER_NAME( 779, DevKill)
REGISTER_NAME( 780, DevReplace)
REGISTER_NAME( 781, DevUI)
REGISTER_NAME( 782, DevSound)
REGISTER_NAME( 783, DevCompile)
REGISTER_NAME( 784, DevBind)
REGISTER_NAME( 785, Localization)
REGISTER_NAME( 786, Compatibility)
REGISTER_NAME( 787, NetComeGo)
REGISTER_NAME( 788, Title)
REGISTER_NAME( 789, Error)
REGISTER_NAME( 790, Heading)
REGISTER_NAME( 791, SubHeading)
REGISTER_NAME( 792, FriendlyError)
REGISTER_NAME( 793, Progress)
REGISTER_NAME( 794, UserPrompt)
REGISTER_NAME( 795, SourceControl)
REGISTER_NAME( 796, DevPhysics)
REGISTER_NAME( 797, DevTick)
REGISTER_NAME( 798, DevStats)
REGISTER_NAME( 799, DevComponents)
REGISTER_NAME( 809, DevMemory)
REGISTER_NAME( 810, XMA)
REGISTER_NAME( 811, WAV)
REGISTER_NAME( 812, AILog)
REGISTER_NAME( 813, DevParticle)
REGISTER_NAME( 814, PerfEvent )
REGISTER_NAME( 815, LocalizationWarning )
REGISTER_NAME( 816, DevUIStyles )
REGISTER_NAME( 817, DevUIStates )
REGISTER_NAME( 818, DevUIFocus )
REGISTER_NAME( 819, ParticleWarn )
REGISTER_NAME( 854, UTrace )
REGISTER_NAME( 855, DevCollision )
REGISTER_NAME( 856, DevSHA )
REGISTER_NAME( 857, DevSpawn )
REGISTER_NAME( 858, DevAnim )
REGISTER_NAME( 859, Hack ) // useful for putting this where we have have hacks so we can see often they are spamming out
REGISTER_NAME( 1118, DevShaders )
REGISTER_NAME( 1119, DevDataBase )
REGISTER_NAME( 1120, DevDataStore )
REGISTER_NAME( 1121, DevAudioVerbose )
REGISTER_NAME( 1125, DevUIAnimation )
REGISTER_NAME( 1126, DevHDDCaching )
REGISTER_NAME( 1127, DevMovie )
REGISTER_NAME( 1128, DevShadersDetailed )
REGISTER_NAME( 1129, PlayerManagement )		// log messages related to creating and removing local players

// Console text colors.
REGISTER_NAME( 800, White)
REGISTER_NAME( 801, Black)
REGISTER_NAME( 802, Red)
REGISTER_NAME( 803, Green)
REGISTER_NAME( 804, Blue)
REGISTER_NAME( 805, Cyan)
REGISTER_NAME( 806, Magenta)
REGISTER_NAME( 807, Yellow)
REGISTER_NAME( 808, DefaultColor)

// Misc.
REGISTER_NAME( 820, KeyType)
REGISTER_NAME( 821, KeyEvent)
REGISTER_NAME( 822, Write)
REGISTER_NAME( 823, Message)
REGISTER_NAME( 824, InitialState)
REGISTER_NAME( 825, Texture)
REGISTER_NAME( 826, Sound)
REGISTER_NAME( 827, FireTexture)
REGISTER_NAME( 828, IceTexture)
REGISTER_NAME( 829, WaterTexture)
REGISTER_NAME( 830, WaveTexture)
REGISTER_NAME( 831, WetTexture)
REGISTER_NAME( 832, Main)
REGISTER_NAME( 834, VideoChange)
REGISTER_NAME( 835, SendText)
REGISTER_NAME( 836, SendBinary)
REGISTER_NAME( 837, ConnectFailure)
REGISTER_NAME( 838, Length)
REGISTER_NAME( 839, Insert)
REGISTER_NAME( 840, Remove)
REGISTER_NAME( 1200, Add)
REGISTER_NAME( 1201, AddItem)
REGISTER_NAME( 1202, RemoveItem)
REGISTER_NAME( 1203, InsertItem)
REGISTER_NAME( 841, Game)
REGISTER_NAME( 842, SequenceObjects)
REGISTER_NAME( 843, PausedState)
REGISTER_NAME( 844, ContinuedState)
REGISTER_NAME( 845, SelectionColor)
REGISTER_NAME( 846, Find)
REGISTER_NAME( 847, UI)
REGISTER_NAME( 848, DevCooking)
REGISTER_NAME( 849, DevOnline)
REGISTER_NAME( 850, DataBinding )
REGISTER_NAME( 851, OptionMusic)
REGISTER_NAME( 852, OptionSFX)
REGISTER_NAME( 853, OptionVoice)
REGISTER_NAME( 1122, Linear )
REGISTER_NAME( 1123, Point )
REGISTER_NAME( 1124, Aniso )

/** Virtual data store names */
REGISTER_NAME( 865, Attributes)	// virtual data store for specifying text attributes (italic, bold, underline, etc.)
REGISTER_NAME( 866, Strings )	// virtual data store for looking up localized string values
REGISTER_NAME( 867, Images)		// virtual data store for specifying direct object references
REGISTER_NAME( 868, SceneData)	// virtual data store for per-scene data stores

// Script Preprocessor
REGISTER_NAME( 870, EndIf)
REGISTER_NAME( 871, Include)
REGISTER_NAME( 872, Define)
REGISTER_NAME( 873, Undefine)
REGISTER_NAME( 874, IsDefined)
REGISTER_NAME( 875, NotDefined)
REGISTER_NAME( 876, Debug)
REGISTER_NAME( 877, Counter)
REGISTER_NAME( 878, SetCounter)
REGISTER_NAME( 879, GetCounter)

//@compatibility - class names that have property remapping (reserve 900 - 999)
REGISTER_NAME( 900, FontCharacter )
REGISTER_NAME( 901,	SourceState )
REGISTER_NAME( 902, InitChild2StartBone )
REGISTER_NAME( 903, SourceStyle )
REGISTER_NAME( 904, SoundCueLocalized )
REGISTER_NAME( 905, SoundCue )
REGISTER_NAME( 906, RawDistributionFloat )
REGISTER_NAME( 907, RawDistributionVector )
REGISTER_NAME( 908, UIDockingSet )
REGISTER_NAME( 909, DockPadding )
REGISTER_NAME( 910, UIScreenValue )
REGISTER_NAME( 911, UIScreenValue_Extent )
REGISTER_NAME( 912, ScaleType )
REGISTER_NAME( 913, EvalType)
REGISTER_NAME( 914, AutoSizePadding )

// Game specific Logging Categories
REGISTER_NAME( 1000, Gear_General )

REGISTER_NAME( 1001, Gear_ActiveReload )
REGISTER_NAME( 1002, Gear_MiniGames )
REGISTER_NAME( 1003, Gear_ResurrectionSystem )
REGISTER_NAME( 1004, Gear_VehicleSystem )
REGISTER_NAME( 1005, Gear_CheckpointSystem )
REGISTER_NAME( 1006, Cover )
REGISTER_NAME( 1007, AICommand )
REGISTER_NAME( 1008, Success )
REGISTER_NAME( 1009, Failure )
REGISTER_NAME( 1010, Aborted )

REGISTER_NAME( 1080, PlayerID )
REGISTER_NAME( 1081, TeamID )
REGISTER_NAME( 1082, HearSoundFinished )

// Needed for post time/locale deletion to export with proper case
REGISTER_NAME( 1100, Time )
// Post processing volume support.
REGISTER_NAME( 1101, PPVolume_BloomEffect )
REGISTER_NAME( 1102, PPVolume_DOFEffect )
REGISTER_NAME( 1103, PPVolume_MotionBlurEffect )
REGISTER_NAME( 1104, PPVolume_SceneEffect )
REGISTER_NAME( 1105, PPVolume_DOFAndBloomEffect )
REGISTER_NAME( 1106, Desaturation )
REGISTER_NAME( 1107, HighLights )
REGISTER_NAME( 1108, MidTones )
REGISTER_NAME( 1109, Shadows )
REGISTER_NAME( 1110, PPVolume_UberPostProcessEffect )

// Script uses both cases which causes *Classes.h problems
REGISTER_NAME( 1111, DeviceID )

// Needed for interpolation curve fixes.
REGISTER_NAME( 1112, InterpCurveFloat )
REGISTER_NAME( 1113, InterpCurveVector2D )
REGISTER_NAME( 1114, InterpCurveVector )
REGISTER_NAME( 1115, InterpCurveTwoVectors )
REGISTER_NAME( 1116, InterpCurveQuat )

/*-----------------------------------------------------------------------------
	Special engine-generated probe messages.
-----------------------------------------------------------------------------*/

//
//warning: All probe entries must be filled in, otherwise non-probe names might be mapped
// to probe name indices.
//
#define NAME_PROBEMIN ((EName)300)
#define NAME_PROBEMAX ((EName)364)

// Creation and destruction.
REGISTER_NAME( 300, UnusedProbe0	 ) // Sent to actor immediately after spawning.
REGISTER_NAME( 301, Destroyed        ) // Called immediately before actor is removed from actor list.

// Gaining/losing actors.
REGISTER_NAME( 302, GainedChild		 ) // Sent to a parent actor when another actor attaches to it.
REGISTER_NAME( 303, LostChild		 ) // Sent to a parent actor when another actor detaches from it.
REGISTER_NAME( 304, UnusedProbe4	 )
REGISTER_NAME( 305, UnusedProbe5	 )

// Triggers.
REGISTER_NAME( 306, Trigger			 ) // Message sent by Trigger actors.
REGISTER_NAME( 307, UnTrigger		 ) // Message sent by Trigger actors.

// Physics & world interaction.
REGISTER_NAME( 308, Timer			 ) // The per-actor timer has fired.
REGISTER_NAME( 309, HitWall			 ) // Ran into a wall.
REGISTER_NAME( 310, Falling			 ) // Actor is falling.
REGISTER_NAME( 311, Landed			 ) // Actor has landed.
REGISTER_NAME( 312, PhysicsVolumeChange) // Actor has changed into a new physicsvolume.
REGISTER_NAME( 313, Touch			 ) // Actor was just touched by another actor.
REGISTER_NAME( 314, UnTouch			 ) // Actor touch just ended, always sent sometime after Touch.
REGISTER_NAME( 315, Bump			 ) // Actor was just touched and blocked. No interpenetration. No UnBump.
REGISTER_NAME( 316, BeginState		 ) // Just entered a new state.
REGISTER_NAME( 317, EndState		 ) // About to leave the current state.
REGISTER_NAME( 318, BaseChange		 ) // Sent to actor when its floor changes.
REGISTER_NAME( 319, Attach			 ) // Sent to actor when it's stepped on by another actor.
REGISTER_NAME( 320, Detach			 ) // Sent to actor when another actor steps off it.
REGISTER_NAME( 321, UnusedProbe21	 ) 
REGISTER_NAME( 322, UnusedProbe22	 ) 
REGISTER_NAME( 323, UnusedProbe23	 ) 
REGISTER_NAME( 324, UnusedProbe24	 ) 
REGISTER_NAME( 325, UnusedProbe25	 ) 
REGISTER_NAME( 326, UnusedProbe26    ) 
REGISTER_NAME( 327, EncroachingOn    ) // Encroaching on another actor.
REGISTER_NAME( 328, EncroachedBy     ) // Being encroached by another actor.
REGISTER_NAME( 329, PoppedState)
REGISTER_NAME( 330, HeadVolumeChange ) // Pawn's head changed zones
REGISTER_NAME( 331, PostTouch        ) // Touch reported after completion of physics
REGISTER_NAME( 332, PawnEnteredVolume) // sent to PhysicsVolume when pawn enters it
REGISTER_NAME( 333, MayFall 		 )
REGISTER_NAME( 334, PushedState)
REGISTER_NAME( 335, PawnLeavingVolume) // sent to PhysicsVolume when pawn leaves it

// Updates.
REGISTER_NAME( 336, Tick			 ) // Clock tick update for nonplayer.
REGISTER_NAME( 337, PlayerTick		 ) // Clock tick update for player.
REGISTER_NAME( 338, ModifyVelocity   )
REGISTER_NAME( 339, UnusedProbe39	 )

// AI.
REGISTER_NAME( 340, SeePlayer        ) // Can see player.
REGISTER_NAME( 341, EnemyNotVisible  ) // Current Enemy is not visible.
REGISTER_NAME( 342, HearNoise        ) // Noise nearby.
REGISTER_NAME( 343, UpdateEyeHeight  ) // Update eye level (after physics).
REGISTER_NAME( 344, SeeMonster       ) // Can see non-player.
REGISTER_NAME( 346, SpecialHandling	 ) // Navigation point requests special handling.
REGISTER_NAME( 347, BotDesireability ) // Value of this inventory to bot.

// Controller notifications
REGISTER_NAME( 348, NotifyBump		 )
REGISTER_NAME( 349, NotifyPhysicsVolumeChange )
REGISTER_NAME( 350, UnusedProbe50	 )
REGISTER_NAME( 351, NotifyHeadVolumeChange)
REGISTER_NAME( 352, NotifyLanded	 )
REGISTER_NAME( 353, NotifyHitWall	 )
REGISTER_NAME( 354, UnusedProbe54	 )
REGISTER_NAME( 355, PreBeginPlay	 )
REGISTER_NAME( 356, UnusedProbe56	 )
REGISTER_NAME( 357, PostBeginPlay	 )
REGISTER_NAME( 358, UnusedProbe58	 )

// More Physics & world interaction.
REGISTER_NAME( 359, PhysicsChangedFor)
REGISTER_NAME( 360, ActorEnteredVolume)
REGISTER_NAME( 361, ActorLeavingVolume)
REGISTER_NAME( 362, UnusedProbe62   )

// Special tag meaning 'All probes'.
REGISTER_NAME( 363, All				 ) // Special meaning, not a message.

// Misc. Make sure this starts after NAME_ProbeMax
REGISTER_NAME( 400, MeshEmitterVertexColor )
REGISTER_NAME( 401, TextureOffsetParameter )
REGISTER_NAME( 402, TextureScaleParameter )
REGISTER_NAME( 403, ImpactVel )
REGISTER_NAME( 404, SlideVel )
 
/*-----------------------------------------------------------------------------
	Closing.
-----------------------------------------------------------------------------*/

#ifdef REGISTERING_ENUM
	};
	#undef REGISTER_NAME
	#undef REGISTERING_ENUM
#endif


/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

interface AIObjectAvoidanceInterface;



function bool ShouldAvoid(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent);
function bool ShouldEvade(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent);
function bool ShouldRoadieRun(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent);




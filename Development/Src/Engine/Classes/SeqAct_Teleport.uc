class SeqAct_Teleport extends SequenceAction;

var() bool bUpdateRotation;
var() bool bDontResetCamera;
var() bool bDontResetState;
var() bool bSnapPlayerAnim;
var() bool bStopAllMovement;

function Actor GetActor(Object Obj)
{
    local Actor DestActor;
    local Controller C;

    DestActor = Actor(Obj);
    C = Controller(DestActor);
    // End:0x57
    if((C != none) && C.Pawn != none)
    {
        DestActor = C.Pawn;
    }
    return DestActor;
    //return ReturnValue;    
}

function bool GetDestination(out Vector Loc, out Rotator Rot)
{
    local array<Object> ObjVars;
    local int Idx;
    local bool UpdateRot;
    local Actor DestActor, TargetActor;
    local array<Vector> vecVars;
    local SeqVar_Vector VectorVar;

    GetVectorVars(vecVars, "Offset");
    // End:0x2F
    if(vecVars.Length > 0)
    {
        Loc = vecVars[0];        
    }
    else
    {
        Loc = vect(0.0000000, 0.0000000, 0.0000000);
    }
    GetObjectVars(ObjVars, "Destination");
    // End:0x11B
    if(ObjVars.Length > 0)
    {
        DestActor = GetActor(ObjVars[Rand(ObjVars.Length)]);
        // End:0xD8
        if(DestActor == none)
        {
            Idx = 0;
            J0x96:

            // End:0xD8 [Loop If]
            if((Idx < ObjVars.Length) && DestActor == none)
            {
                DestActor = GetActor(ObjVars[Idx]);
                Idx++;
                // [Loop Continue]
                goto J0x96;
            }
        }
        // End:0x11B
        if(DestActor != none)
        {
            Loc += DestActor.Location;
            Rot = DestActor.Rotation;
            UpdateRot = bUpdateRotation;
        }
    }
    TargetActor = Actor(Targets[0]);
    // End:0x1BE
    if(TargetActor != none)
    {
        // End:0x179
        foreach LinkedVariables(Class'SeqVar_Vector', VectorVar, "Old Position")
        {
            VectorVar.VectValue = TargetActor.Location;            
        }        
        // End:0x1BD
        foreach LinkedVariables(Class'SeqVar_Vector', VectorVar, "Old Rotation")
        {
            VectorVar.VectValue = Vector(TargetActor.Rotation);            
        }        
    }
    return UpdateRot;
    //return ReturnValue;    
}

defaultproperties
{
    bUpdateRotation=true
    bStopAllMovement=true
    VariableLinks[0]=(ExpectedType=Class'SeqVar_Object',LinkedVariables=none,LinkDesc="Target",LinkVar="None",PropertyName="Targets",bWriteable=false,bModifiesLinkedObject=false,bHidden=false,MinVars=1,MaxVars=255,DrawX=0,CachedProperty=none)
    VariableLinks[1]=(ExpectedType=Class'SeqVar_Object',LinkedVariables=none,LinkDesc="Destination",LinkVar="None",PropertyName="None",bWriteable=false,bModifiesLinkedObject=false,bHidden=false,MinVars=1,MaxVars=255,DrawX=0,CachedProperty=none)
    VariableLinks[2]=(ExpectedType=Class'SeqVar_Vector',LinkedVariables=none,LinkDesc="Offset",LinkVar="None",PropertyName="Offset",bWriteable=false,bModifiesLinkedObject=false,bHidden=false,MinVars=1,MaxVars=255,DrawX=0,CachedProperty=none)
    VariableLinks[3]=(ExpectedType=Class'SeqVar_Vector',LinkedVariables=none,LinkDesc="Old Position",LinkVar="None",PropertyName="None",bWriteable=true,bModifiesLinkedObject=false,bHidden=false,MinVars=1,MaxVars=255,DrawX=0,CachedProperty=none)
    VariableLinks[4]=(ExpectedType=Class'SeqVar_Vector',LinkedVariables=none,LinkDesc="Old Rotation",LinkVar="None",PropertyName="None",bWriteable=true,bModifiesLinkedObject=false,bHidden=false,MinVars=1,MaxVars=255,DrawX=0,CachedProperty=none)
}

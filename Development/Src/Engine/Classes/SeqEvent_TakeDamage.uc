class SeqEvent_TakeDamage extends SequenceEvent
    native(Sequence);

var() float MinDamageAmount;
var() float DamageThreshold;
var() array< class<DamageType> > DamageTypes<AllowAbstract>;
var() array< class<DamageType> > IgnoreDamageTypes<AllowAbstract>;
var float CurrentDamage;
var() bool bResetDamageOnToggle;

final function bool IsValidDamageType(class<DamageType> inDamageType)
{
    local int Idx;
    local bool bValidDamageType;

    // End:0x63
    if(DamageTypes.Length > 0)
    {
        bValidDamageType = false;
        Idx = 0;
        J0x1B:

        // End:0x56 [Loop If]
        if(Idx < DamageTypes.Length)
        {
            // End:0x4C
            if(ClassIsChildOf(inDamageType, DamageTypes[Idx]))
            {
                bValidDamageType = true;
                // [Explicit Break]
                goto J0x56;
            }
            Idx++;
            // [Loop Continue]
            goto J0x1B;
        }
        J0x56:

        // End:0x63
        if(!bValidDamageType)
        {
            return false;
        }
    }
    // End:0xA8
    if(IgnoreDamageTypes.Length > 0)
    {
        Idx = 0;
        J0x76:

        // End:0xA8 [Loop If]
        if(Idx < IgnoreDamageTypes.Length)
        {
            // End:0x9E
            if(ClassIsChildOf(inDamageType, IgnoreDamageTypes[Idx]))
            {
                return false;
            }
            Idx++;
            // [Loop Continue]
            goto J0x76;
        }
    }
    return true;
    //return ReturnValue;    
}

function HandleDamage(Actor InOriginator, Actor InInstigator, class<DamageType> inDamageType, int inAmount, Vector HitLocation)
{
    local SeqVar_Float FloatVar;
    local SeqVar_Vector VectorVar;
    local SeqVar_Object ObjectVar;
    local bool bAlreadyActivatedThisTick;

    // End:0x1C0
    if(((((InOriginator != none) && bEnabled) && float(inAmount) >= MinDamageAmount) && IsValidDamageType(inDamageType)) && !bPlayerOnly || (InInstigator != none) && InInstigator.IsPlayerOwned())
    {
        CurrentDamage += float(inAmount);
        // End:0x1C0
        if(CurrentDamage >= DamageThreshold)
        {
            bAlreadyActivatedThisTick = bActive && ActivationTime ~= GetWorldInfo().TimeSeconds;
            // End:0x1C0
            if(CheckActivate(InOriginator, InInstigator, false))
            {
                // End:0x121
                foreach LinkedVariables(Class'SeqVar_Float', FloatVar, "Damage Taken")
                {
                    // End:0x10B
                    if(bAlreadyActivatedThisTick)
                    {
                        FloatVar.FloatValue += CurrentDamage;
                        // End:0x120
                        continue;
                    }
                    FloatVar.FloatValue = CurrentDamage;                    
                }                
                // End:0x15C
                foreach LinkedVariables(Class'SeqVar_Vector', VectorVar, "Damage Location")
                {
                    VectorVar.VectValue = HitLocation;                    
                }                
                // End:0x196
                foreach LinkedVariables(Class'SeqVar_Object', ObjectVar, "Originator")
                {
                    ObjectVar.SetObjectValue(InOriginator);                    
                }                
                // End:0x1B4
                if(DamageThreshold <= 0.0000000)
                {
                    CurrentDamage = 0.0000000;                    
                }
                else
                {
                    CurrentDamage -= DamageThreshold;
                }
            }
        }
    }
    //return;    
}

function Reset()
{
    super.Reset();
    CurrentDamage = 0.0000000;
    //return;    
}

static event int GetObjClassVersion()
{
    return super(SequenceObject).GetObjClassVersion() + 2;
    //return ReturnValue;    
}

event Toggled()
{
    // End:0x14
    if(bResetDamageOnToggle)
    {
        CurrentDamage = 0.0000000;
    }
    super.Toggled();
    //return;    
}

defaultproperties
{
    DamageThreshold=100.0000000
    bResetDamageOnToggle=true
    VariableLinks[0]=(ExpectedType=Class'SeqVar_Object',LinkedVariables=none,LinkDesc="Instigator",LinkVar="None",PropertyName="None",bWriteable=true,bModifiesLinkedObject=false,bHidden=false,MinVars=1,MaxVars=255,DrawX=0,CachedProperty=none)
    VariableLinks[1]=(ExpectedType=Class'SeqVar_Float',LinkedVariables=none,LinkDesc="Damage Taken",LinkVar="None",PropertyName="None",bWriteable=true,bModifiesLinkedObject=false,bHidden=false,MinVars=1,MaxVars=255,DrawX=0,CachedProperty=none)
    VariableLinks[2]=(ExpectedType=Class'SeqVar_Vector',LinkedVariables=none,LinkDesc="Damage Location",LinkVar="None",PropertyName="None",bWriteable=true,bModifiesLinkedObject=false,bHidden=false,MinVars=1,MaxVars=255,DrawX=0,CachedProperty=none)
    VariableLinks[3]=(ExpectedType=Class'SeqVar_Object',LinkedVariables=none,LinkDesc="Originator",LinkVar="None",PropertyName="None",bWriteable=true,bModifiesLinkedObject=false,bHidden=false,MinVars=1,MaxVars=255,DrawX=0,CachedProperty=none)
}

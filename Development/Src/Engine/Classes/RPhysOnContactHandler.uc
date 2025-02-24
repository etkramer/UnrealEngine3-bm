class RPhysOnContactHandler extends Object
    native;

var Object OwnerObject;
var bool bEnableCapeSpam;

native function OnContact(RB_BodyInstance BodyInst0, RB_BodyInstance BodyInst1, Vector SumNormalForce, Vector SumFrictionForce);
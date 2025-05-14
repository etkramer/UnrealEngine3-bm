class AccessControl extends Info
    config(Game)
    notplaceable;

var globalconfig array<config string> IPPolicies;
var globalconfig array<config UniqueNetId> BannedIDs;
var const localized string IPBanned;
var const localized string WrongPassword;
var const localized string NeedPassword;
var const localized string SessionBanned;
var const localized string KickedMsg;
var const localized string DefaultKickReason;
var const localized string IdleKickReason;
var class<Admin> AdminClass;
var private globalconfig string AdminPassword;
var private globalconfig string GamePassword;
var const localized string ACDisplayText[3];
var const localized string ACDescText[3];
var bool bDontAddDefaultAdmin;

function bool IsAdmin(PlayerController P)
{
    // End:0x53
    if(P != none)
    {
        // End:0x1D
        if(Admin(P) != none)
        {
            return true;
        }
        // End:0x53
        if((P.PlayerReplicationInfo != none) && P.PlayerReplicationInfo.bAdmin)
        {
            return true;
        }
    }
    return false;
    //return ReturnValue;    
}

function bool SetAdminPassword(string P)
{
    AdminPassword = P;
    return true;
    //return ReturnValue;    
}

function SetGamePassword(string P)
{
    GamePassword = P;
    WorldInfo.Game.UpdateGameSettings();
    //return;    
}

function bool RequiresPassword()
{
    return GamePassword != "";
    //return ReturnValue;    
}

function Controller GetControllerFromString(string Target)
{
    local Controller C, FinalC;
    local int I;

    FinalC = none;
    // End:0x97
    foreach WorldInfo.AllControllers(Class'Controller', C)
    {
        // End:0x96
        if((C.PlayerReplicationInfo != none) && (C.PlayerReplicationInfo.PlayerName ~= Target) || C.PlayerReplicationInfo.GetPlayerAlias() ~= Target)
        {
            FinalC = C;
            // End:0x97
            break;
        }        
    }    
    // End:0x168
    if(((C == none) && WorldInfo != none) && WorldInfo.GRI != none)
    {
        I = 0;
        J0xCE:

        // End:0x168 [Loop If]
        if(I < WorldInfo.GRI.PRIArray.Length)
        {
            // End:0x15E
            if(string(WorldInfo.GRI.PRIArray[I].PlayerID) == Target)
            {
                FinalC = Controller(WorldInfo.GRI.PRIArray[I].Owner);
                // [Explicit Break]
                goto J0x168;
            }
            I++;
            // [Loop Continue]
            goto J0xCE;
        }
    }
    J0x168:

    return FinalC;
    //return ReturnValue;    
}

function Kick(string Target)
{
    local Controller C;

    C = GetControllerFromString(Target);
    // End:0xC4
    if((C != none) && C.PlayerReplicationInfo != none)
    {
        // End:0x63
        if(PlayerController(C) != none)
        {
            KickPlayer(PlayerController(C), DefaultKickReason);            
        }
        else
        {
            // End:0xC4
            if(C.PlayerReplicationInfo.bBot)
            {
                // End:0xAC
                if(C.Pawn != none)
                {
                    C.Pawn.Destroy();
                }
                // End:0xC4
                if(C != none)
                {
                    C.Destroy();
                }
            }
        }
    }
    //return;    
}

function KickBan(string Target)
{
    local PlayerController P;
    local string IP;

    P = PlayerController(GetControllerFromString(Target));
    // End:0x165
    if(NetConnection(P.Player) != none)
    {
        // End:0xC7
        if(!WorldInfo.IsConsoleBuild())
        {
            IP = P.GetPlayerNetworkAddress();
            // End:0xC7
            if(CheckIPPolicy(IP))
            {
                IP = Left(IP, InStr(IP, ":"));
                LogInternal("Adding IP Ban for: " $ IP);
                IPPolicies[IPPolicies.Length] = "DENY," $ IP;
                SaveConfig();
            }
        }
        // End:0x14F
        if(P.PlayerReplicationInfo.UniqueId != P.PlayerReplicationInfo.default.UniqueId && !IsIDBanned(P.PlayerReplicationInfo.UniqueId))
        {
            BannedIDs.AddItem(P.PlayerReplicationInfo.UniqueId);
            SaveConfig();
        }
        KickPlayer(P, DefaultKickReason);
        return;
    }
    //return;    
}

function bool KickPlayer(PlayerController C, string KickReason)
{
    // End:0x9E
    if(((C != none) && !IsAdmin(C)) && NetConnection(C.Player) != none)
    {
        // End:0x70
        if(C.Pawn != none)
        {
            C.Pawn.Suicide();
        }
        C.ClientWasKicked();
        // End:0x9C
        if(C != none)
        {
            C.Destroy();
        }
        return true;
    }
    return false;
    //return ReturnValue;    
}

function bool AdminLogin(PlayerController P, string Password)
{
    // End:0x0E
    if(AdminPassword == "")
    {
        return false;
    }
    // End:0x3B
    if(Password == AdminPassword)
    {
        P.PlayerReplicationInfo.bAdmin = true;
        return true;
    }
    return false;
    //return ReturnValue;    
}

function bool AdminLogout(PlayerController P)
{
    // End:0x61
    if(P.PlayerReplicationInfo.bAdmin)
    {
        P.PlayerReplicationInfo.bAdmin = false;
        P.bGodMode = false;
        P.Suicide();
        return true;
    }
    return false;
    //return ReturnValue;    
}

function AdminEntered(PlayerController P)
{
    local string LoginString;

    LoginString = P.PlayerReplicationInfo.GetPlayerAlias() @ "logged in as a server administrator.";
    LogInternal(LoginString);
    WorldInfo.Game.Broadcast(P, LoginString);
    //return;    
}

function AdminExited(PlayerController P)
{
    local string LogoutString;

    LogoutString = P.PlayerReplicationInfo.GetPlayerAlias() $ "is no longer logged in as a server administrator.";
    LogInternal(LogoutString);
    WorldInfo.Game.Broadcast(P, LogoutString);
    //return;    
}

function bool ParseAdminOptions(string Options)
{
    local string InAdminName, InPassword;

    InPassword = Class'GameInfo'.static.ParseOption(Options, "Password");
    InAdminName = Class'GameInfo'.static.ParseOption(Options, "AdminName");
    return ValidLogin(InAdminName, InPassword);
    //return ReturnValue;    
}

function bool ValidLogin(string UserName, string Password)
{
    return (AdminPassword != "") && Password == AdminPassword;
    //return ReturnValue;    
}

event PreLogin(string Options, string Address, out string OutError, bool bSpectator)
{
    local string InPassword;

    OutError = "";
    InPassword = WorldInfo.Game.ParseOption(Options, "Password");
    // End:0xBA
    if((WorldInfo.NetMode != NM_Standalone) && WorldInfo.Game.AtCapacity(bSpectator))
    {
        OutError = PathName(WorldInfo.Game.GameMessageClass) $ ".MaxedOutMessage";        
    }
    else
    {
        // End:0x159
        if(((GamePassword != "") && Caps(InPassword) != Caps(GamePassword)) && (AdminPassword == "") || Caps(InPassword) != Caps(AdminPassword))
        {
            OutError = ((InPassword == "") ? "Engine.AccessControl.NeedPassword" : "Engine.AccessControl.WrongPassword");
        }
    }
    // End:0x192
    if(!CheckIPPolicy(Address))
    {
        OutError = "Engine.AccessControl.IPBanned";
    }
    //return;    
}

function bool CheckIPPolicy(string Address)
{
    local int I, J, LastMatchingPolicy;
    local string Policy, Mask;
    local bool bAcceptAddress, bAcceptPolicy;

    J = InStr(Address, ":");
    // End:0x32
    if(J != -1)
    {
        Address = Left(Address, J);
    }
    bAcceptAddress = true;
    I = 0;
    J0x41:

    // End:0x172 [Loop If]
    if(I < IPPolicies.Length)
    {
        J = InStr(IPPolicies[I], ",");
        // End:0x7A
        if(J == -1)
        {
            // [Explicit Continue]
            goto J0x168;
        }
        Policy = Left(IPPolicies[I], J);
        Mask = Mid(IPPolicies[I], J + 1);
        // End:0xCB
        if(Policy ~= "ACCEPT")
        {
            bAcceptPolicy = true;            
        }
        else
        {
            // End:0xE6
            if(Policy ~= "DENY")
            {
                bAcceptPolicy = false;                
            }
            else
            {
                // [Explicit Continue]
                goto J0x168;
            }
        }
        J = InStr(Mask, "*");
        // End:0x141
        if(J != -1)
        {
            // End:0x13E
            if(Left(Mask, J) == Left(Address, J))
            {
                bAcceptAddress = bAcceptPolicy;
                LastMatchingPolicy = I;
            }
            // [Explicit Continue]
            goto J0x168;
        }
        // End:0x168
        if(Mask == Address)
        {
            bAcceptAddress = bAcceptPolicy;
            LastMatchingPolicy = I;
        }
        J0x168:

        I++;
        // [Loop Continue]
        goto J0x41;
    }
    // End:0x1C0
    if(!bAcceptAddress)
    {
        LogInternal((("Denied connection for " $ Address) $ " with IP policy ") $ IPPolicies[LastMatchingPolicy]);
    }
    return bAcceptAddress;
    //return ReturnValue;    
}

function bool IsIDBanned(const out UniqueNetId NetId)
{
    local int I;

    I = 0;
    J0x07:

    // End:0x3B [Loop If]
    if(I < BannedIDs.Length)
    {
        // End:0x31
        if(BannedIDs[I] == NetId)
        {
            return true;
        }
        I++;
        // [Loop Continue]
        goto J0x07;
    }
    return false;
    //return ReturnValue;    
}

defaultproperties
{
    IPPolicies[0]="ACCEPT;*"
    IPBanned="Your IP address has been banned on this server."
    WrongPassword="La contrase?a no es v?lida."
    NeedPassword="You need to enter a password to join this game."
    SessionBanned="Tu direcci?n IP ha sido rechazada de la sesi?n de juego act."
    KickedMsg="You have been forcibly removed from the game."
    DefaultKickReason="Ninguno especificado"
    IdleKickReason="Kicked for idling."
    AdminClass=Class'Admin'
    ACDisplayText[0]="Contrase?a de juego"
    ACDisplayText[1]="Access Policies"
    ACDisplayText[2]="Contrase?a de administrador"
    ACDescText[0]="If this password is set, players will have to enter it to join this server."
    ACDescText[1]="Especifica las direcciones o el rango de direcciones IP que son rechazadas."
    ACDescText[2]="Password required to login with administrator privileges on this server."
    Components[0]=none
}

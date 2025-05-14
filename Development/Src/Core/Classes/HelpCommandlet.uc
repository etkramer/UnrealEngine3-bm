class HelpCommandlet extends Commandlet
    transient
    native;

// Export UHelpCommandlet::execMain(FFrame&, void* const)
native event int Main(string Params);

defaultproperties
{
    HelpDescription="This commandlet displays help information on other commandlets"
    HelpUsage="gamename.exe help <list | commandletname | webhelp commandletname>"
    HelpWebLink="https://udn.epicgames.com/bin/view/Three/HelpCommandlet"
    HelpParamNames[0]="list"
    HelpParamNames[1]="commandlet name"
    HelpParamNames[2]="webhelp"
    HelpParamDescriptions[0]="Lists all commandlets that are available"
    HelpParamDescriptions[1]="Displays help information for the specified commandlet"
    HelpParamDescriptions[2]="Launches a browser with the URL of the web page that documents the commandlet"
}
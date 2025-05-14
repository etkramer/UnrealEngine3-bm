class Commandlet extends Object
    abstract
    transient
    native;

var const localized string HelpDescription;
var const localized string HelpUsage;
var const localized string HelpWebLink;
var const localized array<localized string> HelpParamNames;
var const localized array<localized string> HelpParamDescriptions;
var bool IsServer;
var bool IsClient;
var bool IsEditor;
var bool LogToConsole;
var bool ShowErrorCount;

cpptext
{
	/**
	 * Parses a string into tokens, separating switches (beginning with - or /) from
	 * other parameters
	 *
	 * @param	CmdLine		the string to parse
	 * @param	Tokens		[out] filled with all parameters found in the string
	 * @param	Switches	[out] filled with all switches found in the string
	 *
	 * @return	@todo
	 */
	static void ParseCommandLine( const TCHAR* CmdLine, TArray<FString>& Tokens, TArray<FString>& Switches )
	{
		FString NextToken;
		while ( ParseToken(CmdLine, NextToken, FALSE) )
		{
			if ( **NextToken == TCHAR('-') || **NextToken == TCHAR('/') )
			{
				new(Switches) FString(NextToken.Mid(1));
			}
			else
			{
				new(Tokens) FString(NextToken);
			}
		}
	}

	/**
	 * This is where you put any custom code that needs to be executed from InitializeIntrinsicPropertyValues() in
	 * your commandlet
	 */
	void StaticInitialize() {}

	/**
	 * Allows commandlets to override the default behavior and create a custom engine class for the commandlet. If
	 * the commandlet implements this function, it should fully initialize the UEngine object as well.  Commandlets
	 * should indicate that they have implemented this function by assigning the custom UEngine to GEngine.
	 */
	virtual void CreateCustomEngine() {}
}

// Export UCommandlet::execMain(FFrame&, void* const)
native event int Main(string Params);

defaultproperties
{
    IsServer=true
    IsClient=true
    IsEditor=true
    ShowErrorCount=true
}

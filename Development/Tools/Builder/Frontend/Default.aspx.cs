using System;
using System.Data;
using System.Configuration;
using System.Collections;
using System.Data.SqlClient;
using System.Drawing;
using System.Web;
using System.Web.Security;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Web.UI.WebControls.WebParts;
using System.Web.UI.HtmlControls;

public partial class _Default : BasePage
{
    private int ActiveBuilderCount = -1;

    protected void Page_Load( object sender, EventArgs e )
    {
        string LoggedOnUser = Context.User.Identity.Name;
        string MachineName = Context.Request.UserHostName;

        Label_Welcome.Text = "Welcome \"" + LoggedOnUser + "\" running on \"" + MachineName + "\"";

        ScriptTimer_Tick( sender, e );
        VerifyTimer_Tick( sender, e );

        CheckSystemDown();
    }

    protected void CheckSystemDown()
    {
        string CommandString;

        SqlConnection Connection = OpenConnection();

        CommandString = "SELECT Value FROM [Variables] WHERE ( Variable = 'StatusMessage' )";
        string Message = ReadString( Connection, CommandString );

        CloseConnection( Connection );

        if( Message.Length > 0 )
        {
            Response.Redirect( "Offline.aspx" );
        }
    }

    protected void Button_TriggerBuild_Click( object sender, EventArgs e )
    {
        Button Pressed = ( Button )sender;
        if( Pressed.ID == "Button_TriggerBuild" )
        {
            Response.Redirect( "Builder.aspx" );
        }
        else if( Pressed.ID == "Button_TriggerPCS" )
        {
            Response.Redirect( "PCS.aspx" );
        }
        else if( Pressed.ID == "Button_LegacyUTPC" )
        {
            Response.Redirect( "UTPC.aspx" );
        }
        else if( Pressed.ID == "Button_LegacyUTPS3" )
        {
            Response.Redirect( "UTPS3.aspx" );
        }
        else if( Pressed.ID == "Button_LegacyUTX360" )
        {
            Response.Redirect( "UTX360.aspx" );
        }
        else if( Pressed.ID == "Button_LegacyDelta" )
        {
            Response.Redirect( "Delta.aspx" );
        }
        else if( Pressed.ID == "Button_TriggerCook" )
        {
            Response.Redirect( "Cooker.aspx" );
        }
        else if( Pressed.ID == "Button_PromoteBuild" )
        {
            Response.Redirect( "Promote.aspx" );
        }
        else if( Pressed.ID == "Button_Verification" )
        {
            Response.Redirect( "CIS.aspx" );
        }
    }

    protected void Button_RestartControllers_Click( object sender, EventArgs e )
    {
        string CommandString;

        SqlConnection Connection = OpenConnection();

        CommandString = "UPDATE Builders SET Restart = 1 WHERE ( State != 'Dead' AND State != 'Zombied' )";
        Update( Connection, CommandString );

        CloseConnection( Connection );
    }

    protected void Repeater_BuildLog_ItemCommand( object source, RepeaterCommandEventArgs e )
    {
        if( e.Item != null )
        {
            string CommandString;

            SqlConnection Connection = OpenConnection();

            // Find the command id that matches the description
            LinkButton Pressed = ( LinkButton )e.CommandSource;
            string Build = Pressed.Text.Substring( "Stop ".Length );
            CommandString = "SELECT [ID] FROM [Commands] WHERE ( Description = '" + Build + "' )";
            int CommandID = ReadInt( Connection, CommandString );

            if( CommandID != 0 )
            {
                string User = Context.User.Identity.Name;
                int Offset = User.LastIndexOf( '\\' );
                if( Offset >= 0 )
                {
                    User = User.Substring( Offset + 1 );
                }

                CommandString = "UPDATE Commands SET Killing = 1, Killer = '" + User + "' WHERE ( ID = " + CommandID.ToString() + " )";
                Update( Connection, CommandString );
            }

            CloseConnection( Connection );
        }
    }
    
    protected string DateDiff( object Start )
    {
        TimeSpan Taken = DateTime.Now - ( DateTime )Start;

        string TimeTaken = "Time taken :" + Environment.NewLine;
        TimeTaken += Taken.Hours.ToString( "00" ) + ":" + Taken.Minutes.ToString( "00" ) + ":" + Taken.Seconds.ToString( "00" );

        return ( TimeTaken );
    }

    protected string DateDiff2( object Start )
    {
        TimeSpan Taken = DateTime.Now - ( DateTime )Start;

        string TimeTaken = "( " + Taken.Hours.ToString( "00" ) + ":" + Taken.Minutes.ToString( "00" ) + ":" + Taken.Seconds.ToString( "00" ) + " )";

        return ( TimeTaken );
    }

    protected Color CheckConnected( object LastPing )
    {
        if( LastPing.GetType() == DateTime.Now.GetType() )
        {
            TimeSpan Taken = DateTime.Now - ( DateTime )LastPing;

            // Check for no ping in 900 seconds
            if( Taken.TotalSeconds > 900 )
            {
                return ( Color.Red );
            }
        }

        return ( Color.DarkGreen );
    }

    protected string GetAvailability( object Machine, object LastPing )
    {
        if( LastPing.GetType() == DateTime.Now.GetType() )
        {
            TimeSpan Taken = DateTime.Now - ( DateTime )LastPing;

            if( Taken.TotalSeconds > 900 )
            {
                return ( "Controller '" + ( string )Machine + "' is NOT responding!" );
            }
        }

        return ( "Controller '" + ( string )Machine + "' is available" );
    }

    protected void ScriptTimer_Tick( object sender, EventArgs e )
    {
        SqlConnection Connection = OpenConnection();

        DataSet StatusData = new DataSet();
        SqlDataAdapter StatusCommand = new SqlDataAdapter( "EXEC SelectBuildStatus", Connection );
        StatusCommand.Fill( StatusData, "ActiveBuilds" );
        Repeater_BuildLog.DataSource = StatusData;
        Repeater_BuildLog.DataBind();

        DataSet JobsData = new DataSet();
        SqlDataAdapter JobsCommand = new SqlDataAdapter( "EXEC SelectJobStatus", Connection );
        JobsCommand.Fill( JobsData, "ActiveJobs" );
        Repeater_JobLog.DataSource = JobsData;
        Repeater_JobLog.DataBind();

        int BuilderCount = ReadIntSP( Connection, "GetActiveBuilderCount" );

        if( BuilderCount != ActiveBuilderCount )
        {
            DataSet MainBranchData = new DataSet();
            SqlDataAdapter MainBranchCommand = new SqlDataAdapter( "EXEC SelectBuilds 100,0", Connection );
            MainBranchCommand.Fill( MainBranchData, "MainBranch" );
            Repeater_MainBranch.DataSource = MainBranchData;
            Repeater_MainBranch.DataBind();
            
            DataSet UT3BranchesData = new DataSet();
            SqlDataAdapter UT3BranchesCommand = new SqlDataAdapter( "EXEC SelectBuilds 300,0", Connection );
            UT3BranchesCommand.Fill( UT3BranchesData, "UT3Branches" );
            Repeater_UT3Branches.DataSource = UT3BranchesData;
            Repeater_UT3Branches.DataBind();

            DataSet BuildersData = new DataSet();
            SqlDataAdapter BuildersCommand = new SqlDataAdapter( "EXEC SelectActiveBuilders", Connection );
            BuildersCommand.Fill( BuildersData, "ActiveBuilders" );
            Repeater_Builders.DataSource = BuildersData;
            Repeater_Builders.DataBind();

            ActiveBuilderCount = BuilderCount;
        }

        CloseConnection( Connection );
    }

    protected void VerifyTimer_Tick( object sender, EventArgs e )
    {
        SqlConnection Connection = OpenConnection();

        DataSet StatusData = new DataSet();
        SqlDataAdapter StatusCommand = new SqlDataAdapter( "EXEC SelectVerifyStatus", Connection );
        StatusCommand.Fill( StatusData, "ActiveBuilds" );
        Repeater_Verify.DataSource = StatusData;
        Repeater_Verify.DataBind();

        CloseConnection( Connection );
    }
}

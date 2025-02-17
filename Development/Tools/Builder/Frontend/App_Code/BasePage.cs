using System;
using System.Data;
using System.Diagnostics;
using System.Configuration;
using System.Data.SqlClient;
using System.Web;
using System.Web.Security;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Web.UI.WebControls.WebParts;
using System.Web.UI.HtmlControls;

public class BasePage : System.Web.UI.Page
{
    protected SqlConnection OpenConnection()
    {
        SqlConnection Connection = new SqlConnection( "Data Source=DB-02;Initial Catalog=Perf_Build;Integrated Security=True" );
        Connection.Open();
        return ( Connection );
    }

    protected void CloseConnection( SqlConnection Connection )
    {
        Connection.Close();
    }

    protected void Update( SqlConnection Connection, string CommandString )
    {
        try
        {
            SqlCommand Command = new SqlCommand( CommandString, Connection );
            Command.ExecuteNonQuery();
        }
        catch
        {
            System.Diagnostics.Debug.WriteLine( "Exception in Update" );
        }
    }

    protected int ReadInt( SqlConnection Connection, string CommandString )
    {
        int Result = 0;

        try
        {
            SqlCommand Command = new SqlCommand( CommandString, Connection );
            SqlDataReader DataReader = Command.ExecuteReader();
            if( DataReader.Read() )
            {
                Result = DataReader.GetInt32( 0 );
            }
            DataReader.Close();
        }
        catch
        {
            System.Diagnostics.Debug.WriteLine( "Exception in ReadInt" );
        }

        return ( Result );
    }

    protected int ReadIntSP( SqlConnection Connection, string StoredProcedure )
    {
        int Result = 0;

        try
        {
            SqlCommand Command = new SqlCommand( StoredProcedure, Connection );
            Command.CommandType = CommandType.StoredProcedure;

            SqlDataReader DataReader = Command.ExecuteReader();
            if( DataReader.Read() )
            {
                Result = DataReader.GetInt32( 0 );
            }
            DataReader.Close();
        }
        catch
        {
            System.Diagnostics.Debug.WriteLine( "Exception in ReadIntSP" );
        }

        return ( Result );
    }

    protected string ReadString( SqlConnection Connection, string CommandString )
    {
        string Result = "";

        try
        {
            SqlCommand Command = new SqlCommand( CommandString, Connection );
            SqlDataReader DataReader = Command.ExecuteReader();
            if( DataReader.Read() )
            {
                Result = DataReader.GetString( 0 );
            }
            DataReader.Close();
        }   
        catch
        {
            System.Diagnostics.Debug.WriteLine( "Exception in ReadString" );
        }

        return( Result );
    }

    protected DateTime ReadDateTime( SqlConnection Connection, string CommandString )
    {
        DateTime Result = DateTime.Now;

        try
        {
            SqlCommand Command = new SqlCommand( CommandString, Connection );
            SqlDataReader DataReader = Command.ExecuteReader();
            if( DataReader.Read() )
            {
                Result = DataReader.GetDateTime( 0 );
            }
            DataReader.Close();
        }
        catch
        {
            System.Diagnostics.Debug.WriteLine( "Exception in ReadDateTime" );
        }

        return ( Result );
    }

    protected void BuilderDBRepeater_ItemCommand( object source, RepeaterCommandEventArgs e )
    {
        if( e.Item != null )
        {
            string CommandString;

            SqlConnection Connection = OpenConnection();

            // Find the command id that matches the description
            int CommandID = Int32.Parse( ( string )e.CommandName );

            if( CommandID != 0 )
            {   
                string User = Context.User.Identity.Name;
                int Offset = User.LastIndexOf( '\\' );
                if( Offset >= 0 )
                {
                    User = User.Substring( Offset + 1 );
                }

                CommandString = "UPDATE Commands SET Pending = 1, Operator = '" + User + "' WHERE ( ID = " + CommandID.ToString() + " )";
                Update( Connection, CommandString );
            }

            CloseConnection( Connection );
            Response.Redirect( "Default.aspx" );
        }
    }
}

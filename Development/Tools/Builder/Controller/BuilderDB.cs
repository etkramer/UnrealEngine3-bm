using System;
using System.Collections.Generic;
using System.Data;
using System.Data.SqlClient;
using System.Drawing;
using System.Text;

namespace Controller
{
    public class ProcedureParameter
    {
        public string Name;
        public object Value;
        public SqlDbType Type;
        public int Size;

        public ProcedureParameter( string InName, object InValue, SqlDbType InType, int InSize )
        {
            Name = InName; 
            Value = InValue;
            Type = InType;
            Size = InSize;
        }
    }

    public class BuilderDB
    {
        private Main Parent = null;
        private SqlConnection Connection = null;

        public BuilderDB( Main InParent )
        {
            Parent = InParent;

            try
            {
                string ConnectionString = Properties.Settings.Default.DBConnection;
                Connection = new SqlConnection( ConnectionString );
                Connection.Open();

                Parent.Log( "Connected to database OK", Color.DarkGreen );
            }
            catch
            {
                Parent.Log( "Database connection FAILED", Color.Red );
                Parent.Ticking = false;
                Connection = null;
            }
        }

        public void Destroy()
        {
            Parent.Log( "Closing database connection", Color.DarkGreen );
            if( Connection != null )
            {
                Connection.Close();
            }
        }

        public void Update( string CommandString )
        {
            try
            {
                SqlCommand Command = new SqlCommand( CommandString, Connection );
                Command.ExecuteNonQuery();
            }
            catch
            {
            }
        }

        public int ReadInt( string Procedure, ProcedureParameter Parm0, ProcedureParameter Parm1, ProcedureParameter Parm2 )
        {
            int Result = 0;

            try
            {
                SqlCommand Command = new SqlCommand( Procedure, Connection );
                Command.CommandType = CommandType.StoredProcedure;

                if( Parm0 != null )
                {
                    Command.Parameters.Add( Parm0.Name, Parm0.Type );
                    Command.Parameters[Parm0.Name].Value = Parm0.Value;

                    if( Parm1 != null )
                    {
                        Command.Parameters.Add( Parm1.Name, Parm1.Type );
                        Command.Parameters[Parm1.Name].Value = Parm1.Value;

                        if( Parm2 != null )
                        {
                            Command.Parameters.Add( Parm2.Name, Parm2.Type );
                            Command.Parameters[Parm2.Name].Value = Parm2.Value;
                        }
                    }
                }

                SqlDataReader DataReader = Command.ExecuteReader();
                try
                {
                    if( DataReader.Read() )
                    {
                        Result = DataReader.GetInt32( 0 );
                    }
                }
                catch
                {
                }
                DataReader.Close();
            }
            catch
            {
                Parent.Log( "ERROR: DB reading INT in procedure '" + Procedure + "'", Color.Red );
            }

            return ( Result );
        }

        public void Update( string Procedure, ProcedureParameter[] Parms )
        {
            try
            {
                SqlCommand Command = new SqlCommand( Procedure, Connection );
                Command.CommandType = CommandType.StoredProcedure; 
                
                foreach( ProcedureParameter Parm in Parms )
                {
                    Command.Parameters.Add( Parm.Name, Parm.Type );
                    Command.Parameters[Parm.Name].Value = Parm.Value;
                }

                Command.ExecuteNonQuery();
            }
            catch
            {
                Parent.Log( "ERROR: Update in procedure '" + Procedure + "'", Color.Red );
            }
        }

        public int ReadInt( string CommandString )
        {
            int Result = 0;

            try
            {
                SqlCommand Command = new SqlCommand( CommandString, Connection );
                SqlDataReader DataReader = Command.ExecuteReader();
                try
                {
                    if( DataReader.Read() )
                    {
                        Result = DataReader.GetInt32( 0 );
                    }
                }
                catch
                {
                }
                DataReader.Close();
            }
            catch
            {
                Parent.Log( "ERROR: DB reading INT [" + CommandString + "]", Color.Red );
            }

            return ( Result );
        }

        public int ReadDecimal( string CommandString )
        {
            int Result = 0;

            try
            {
                SqlCommand Command = new SqlCommand( CommandString, Connection );
                SqlDataReader DataReader = Command.ExecuteReader();
                try
                {
                    if( DataReader.Read() )
                    {
                        Result = ( int )DataReader.GetDecimal( 0 );
                    }
                }
                catch
                {
                }
                DataReader.Close();
            }
            catch
            {
                Parent.Log( "ERROR: DB reading Decimal [" + CommandString + "]", Color.Red );
            }

            return ( Result );
        }

        public bool ReadBool( string CommandString )
        {
            bool Result = false;

            try
            {
                SqlCommand Command = new SqlCommand( CommandString, Connection );
                SqlDataReader DataReader = Command.ExecuteReader();
                try
                {
                    if( DataReader.Read() )
                    {
                        Result = DataReader.GetBoolean( 0 );
                    }
                }
                catch
                {
                }
                DataReader.Close();
            }
            catch
            {
                Parent.Log( "ERROR: DB reading BOOL [" + CommandString + "]", Color.Red );
            }

            return ( Result );
        }

        public string ReadString( string CommandString )
        {
            string Result = "";

            try
            {
                SqlCommand Command = new SqlCommand( CommandString, Connection );
                SqlDataReader DataReader = Command.ExecuteReader();
                try
                {
                    if( DataReader.Read() )
                    {
                        Result = DataReader.GetString( 0 );
                    }
                }
                catch
                {
                }
                DataReader.Close();
            }
            catch
            {
                Parent.Log( "ERROR: DB reading STRING [" + CommandString + "]", Color.Red );
            }

            return ( Result );
        }

        public DateTime ReadDateTime( string CommandString )
        {
            DateTime Result = DateTime.Now;

            try
            {
                SqlCommand Command = new SqlCommand( CommandString, Connection );
                SqlDataReader DataReader = Command.ExecuteReader();
                try
                {
                    if( DataReader.Read() )
                    {
                        Result = DataReader.GetDateTime( 0 );
                    }
                }
                catch
                {
                }
                DataReader.Close();
            }
            catch
            {
                Parent.Log( "ERROR: DB reading DATETIME [" + CommandString + "]", Color.Red );
            }

            return( Result );
        }

        public string GetVariable( string Branch, string Var )
        {
            string Query = "SELECT [Value] FROM [Variables] WHERE ( Variable = '" + Var + "' AND Branch = '" + Branch + "' )";
            string Result = ReadString( Query );
            return ( Result );
        }

        public void SetLastGoodBuild( int CommandID, int ChangeList, DateTime Time )
        {
            if( CommandID != 0 )
            {
                SetInt( "Commands", CommandID, "LastGoodChangeList", ChangeList );
                SetDateTime( "Commands", CommandID, "LastGoodDateTime", Time );
            }
        }

        public bool Trigger( int CurrentCommandID, string BuildDescription )
        {
            string Query = "SELECT [ID] FROM [Commands] WHERE ( Description = '" + BuildDescription + "' )";
            int CommandID = ReadInt( Query );

            if( CommandID != 0 )
            {
                Query = "SELECT [Machine] FROM [Commands] WHERE ( ID = " + CommandID.ToString() + " )";
                string Machine = ReadString( Query );

                if( Machine.Length == 0 )
                {
                    Query = "SELECT [Operator] FROM [Commands] WHERE ( ID = " + CurrentCommandID.ToString() + " )";
                    string User = ReadString( Query );

                    Query = "UPDATE Commands SET Pending = '1', Operator = '" + User + "' WHERE ( ID = " + CommandID.ToString() + " )";
                    Update( Query );

                    return ( true );
                }
            }

            return ( false );
        }

        public void Trigger( int CommandID )
        {
            string Query = "SELECT [Operator] FROM [Commands] WHERE ( ID = " + CommandID.ToString() + " )";
            string User = ReadString( Query );

            Query = "UPDATE Commands SET Pending = '1', Operator = '" + User + "' WHERE ( ID = " + CommandID.ToString() + " )";
            Update( Query );
        }

        public int PollForKillBuild()
        {
            string QueryString = "SELECT [ID] FROM [Commands] WHERE ( Machine = '" + Parent.MachineName + "' AND Killing = 1 )";
            int ID = ReadInt( QueryString );
            return ( ID );
        }

        public int PollForKillJob()
        {
            string QueryString = "SELECT [ID] FROM [Jobs] WHERE ( Machine = '" + Parent.MachineName + "' AND Killing = 1 AND Complete = 0 )";
            int ID = ReadInt( QueryString );
            return ( ID );
        }

        public int PollForBuild( string MachineName )
        {
            int ID = 0;

            // Check for machine locked timed build
            ProcedureParameter ParmMachine = new ProcedureParameter( "MachineName", MachineName, SqlDbType.VarChar, 32 );
            ProcedureParameter ParmLocked = new ProcedureParameter( "AnyMachine", false, SqlDbType.Bit, 1 );
            ProcedureParameter ParmOS64Bit = new ProcedureParameter( "OS64Bit", Parent.MachineIs64Bit, SqlDbType.Bit, 1 );
            ID = ReadInt( "CheckTimedBuild", ParmMachine, ParmLocked, ParmOS64Bit );
            if( ID != 0 )
            {
                SetString( "Commands", ID, "Operator", "AutoTimer" );
                return ( ID );
            }

            // Check for a machine locked triggered build
            ID = ReadInt( "CheckTriggeredBuild", ParmMachine, ParmLocked, ParmOS64Bit );
            if( ID != 0 )
            {
                return ( ID );
            }

            // If this machine locked to a build then don't allow it to grab normal ones
            if( Parent.MachineLock != 0 )
            {
                return ( 0 );
            }

            // Poll for a timed build to be grabbed by anyone
            ParmLocked = new ProcedureParameter( "AnyMachine", true, SqlDbType.Bit, 1 );
            ID = ReadInt( "CheckTimedBuild", ParmMachine, ParmLocked, ParmOS64Bit );
            if( ID != 0 )
            {
                SetString( "Commands", ID, "Operator", "AutoTimer" );
                return ( ID );
            }

            // Poll for a triggered build
            ID = ReadInt( "CheckTriggeredBuild", ParmMachine, ParmLocked, ParmOS64Bit );

            return ( ID );
        }

        public int PollForJob( string MachineName )
        {
            int ID;

            // Poll for a job
            ProcedureParameter ParmMachine = new ProcedureParameter( "MachineName", MachineName, SqlDbType.VarChar, 32 );
            ProcedureParameter ParmOS64Bit = new ProcedureParameter( "OS64Bit", Parent.MachineIs64Bit, SqlDbType.Bit, 1 );
            ID = ReadInt( "CheckJob", ParmMachine, ParmOS64Bit, null );
            return ( ID );
        }

        // The passed in CommandID wants to copy to the network - can it?
        public bool AvailableBandwidth( int CommandID )
        {
            StringBuilder Query = new StringBuilder();

            string Check = "SELECT [ID] FROM [Commands] WHERE ( ConchHolder is not NULL )";
            int ID = ReadInt( Check );
            if( ID != 0 )
            {
                Check = "SELECT [ConchHolder] FROM [Commands] WHERE ( ID = " + ID.ToString() + " )";
                DateTime Start = ReadDateTime( Check );
                // 60 minute timeout
                if( DateTime.Now - Start > new TimeSpan( 0, 60, 0 ) )
                {
                    Delete( "Commands", ID, "ConchHolder" );
                }

                // No free slots
                return ( false );
            }

            // Atomically set the ConchHolder time
            if( CommandID != 0 )
            {
                ProcedureParameter Parm = new ProcedureParameter( "CommandID", CommandID, SqlDbType.Int, 4 );
                return ( ReadInt( "SetConchTime", Parm, null, null ) == 0 );
            }

            return ( false );
        }

        public int GetInt( string TableName, int ID, string Command )
        {
            int Result = 0;

            if( ID != 0 )
            {
                string Query = "SELECT [" + Command + "] FROM [" + TableName + "] WHERE ( ID = " + ID.ToString() + " )";
                Result = ReadInt( Query );
            }

            return ( Result );
        }

        public bool GetBool( string TableName, int ID, string Command )
        {
            bool Result = false;

            if( ID != 0 )
            {
                string Query = "SELECT [" + Command + "] FROM [" + TableName + "] WHERE ( ID = " + ID.ToString() + " )";
                Result = ReadBool( Query );
            }

            return ( Result );
        }

        public string GetString( string TableName, int ID, string Command )
        {
            string Result = "";

            if( ID != 0 )
            {
                string Query = "SELECT [" + Command + "] FROM [" + TableName + "] WHERE ( ID = " + ID.ToString() + " )";
                Result = ReadString( Query );
            }

            return ( Result );
        }

        public DateTime GetDateTime( string TableName, int ID, string Command )
        {
            DateTime Result = DateTime.Now;

            if( ID != 0 )
            {
                string Query = "SELECT [" + Command + "] FROM [" + TableName + "] WHERE ( ID = " + ID.ToString() + " )";
                Result = ReadDateTime( Query );
            }

            return ( Result );
        }

        public void SetString( string TableName, int ID, string Field, string Info )
        {
            if( ID != 0 )
            {
                Info = Info.Replace( "'", "" );

                string Command = "UPDATE " + TableName + " SET " + Field + " = '" + Info + "' WHERE ( ID = " + ID.ToString() + " )";
                Update( Command );
            }
        }

        public void SetInt( string TableName, int ID, string Field, int Info )
        {
            if( ID != 0 )
            {
                string Command = "UPDATE " + TableName + " SET " + Field + " = " + Info.ToString() + " WHERE ( ID = " + ID.ToString() + " )";
                Update( Command );
            }
        }

        public void SetBool( string TableName, int ID, string Field, bool BoolInfo )
        {
            if( ID != 0 )
            {
                int Info = 0;
                if( BoolInfo )
                {
                    Info = 1;
                }
                string Command = "UPDATE " + TableName + " SET " + Field + " = " + Info.ToString() + " WHERE ( ID = " + ID.ToString() + " )";
                Update( Command );
            }
        }

        public void SetDateTime( string TableName, int ID, string Field, DateTime Time )
        {
            if( ID != 0 )
            {
                string Command = "UPDATE " + TableName + " SET " + Field + " = '" + Time.ToString() + "' WHERE ( ID = " + ID.ToString() + " )";
                Update( Command );
            }
        }

        public void Delete( string TableName, int ID, string Field )
        {
            if( ID != 0 )
            {
                string Command = "UPDATE " + TableName + " SET " + Field + " = null WHERE ( ID = " + ID.ToString() + " )";
                Update( Command );
            }
        }

        public void WritePerformanceData( string MachineName, string KeyName, int Value )
        {
            ProcedureParameter[] Parms = 
            {
                new ProcedureParameter( "CounterName", KeyName, SqlDbType.Char, 64 ),
                new ProcedureParameter( "MachineName", MachineName, SqlDbType.Char, 64 ),
                new ProcedureParameter( "AppName", "Controller", SqlDbType.Char, 64 ),
                new ProcedureParameter( "IntValue", Value, SqlDbType.BigInt, 8 ),
                new ProcedureParameter( "DateTimeStamp", DateTime.Now, SqlDbType.DateTime, 0 )
            };
            
            Update( "CreatePerformanceData", Parms );
        }
    }
}
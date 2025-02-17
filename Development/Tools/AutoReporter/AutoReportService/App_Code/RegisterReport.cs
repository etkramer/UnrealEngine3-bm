using System;
using System.Data;
using System.Web;
using System.Collections;
using System.Collections.Specialized;
using System.Configuration;
using System.Web.Services;
using System.Web.Services.Protocols;
using System.Data.SqlClient;
using System.IO;


/// <summary>
/// Summary description for RegisterReport
/// </summary>
[WebService(Namespace = "http://tempuri.org/")]
[WebServiceBinding(ConformsTo = WsiProfiles.BasicProfile1_1)]
public class RegisterReport : System.Web.Services.WebService 
{
    public RegisterReport ()
	{
    }

    private DateTime ConvertDumpTimeToDateTime(string timeString)
    {
        //"2006.10.11-13.50.53"

        DateTime newDateTime;

        try
        {
            int yearEndIndex = timeString.IndexOf(".");
            int monthEndIndex = timeString.IndexOf(".", yearEndIndex + 1);
            int dayEndIndex = timeString.IndexOf("-", monthEndIndex + 1);
            int hourEndIndex = timeString.IndexOf(".", dayEndIndex + 1);
            int minuteEndIndex = timeString.IndexOf(".", hourEndIndex + 1);

            string year = timeString.Substring(0, yearEndIndex);
            string month = timeString.Substring(yearEndIndex + 1, monthEndIndex - yearEndIndex - 1);
            string day = timeString.Substring(monthEndIndex + 1, dayEndIndex - monthEndIndex - 1);
            string hour = timeString.Substring(dayEndIndex + 1, hourEndIndex - dayEndIndex - 1);
            string minute = timeString.Substring(hourEndIndex + 1, minuteEndIndex - hourEndIndex - 1);
            string second = timeString.Substring(minuteEndIndex + 1);

            newDateTime = new DateTime(int.Parse(year), int.Parse(month), int.Parse(day), int.Parse(hour), int.Parse(minute), int.Parse(second));
        }
        catch (Exception)
        {
            newDateTime = new DateTime();
        }
        return newDateTime;
    }

    /**
     * CreateNewReport - creates a new record in the appropriate table from the parameters.
     * 
     * @return int - the unique identifier of the new record.  This will be -1 on failure. 
     */
    [WebMethod]
    public int CreateNewReport(string ComputerName, string UserName, string GameName, string PlatformName, string LanguageExt,
        string TimeOfCrash, string BuildVer, string ChangelistVer, string CommandLine, string BaseDir, 
        string CallStack, string EngineMode)
    {
        string LogFileName = ConfigurationManager.AppSettings["LogFileName"];
		int newID = -1;

		using(StreamWriter LogFile = new StreamWriter(LogFileName, true))
		{
			ConnectionStringSettings connSettings = ConfigurationManager.ConnectionStrings["DatabaseConnection"];

			using(SqlConnection connection = new SqlConnection(connSettings.ConnectionString))
			{

				LogFile.WriteLine("");
				DateTime currentDate = DateTime.Now;
				LogFile.WriteLine("Creating new report..." + currentDate.ToString("G"));

				try
				{
					connection.Open();
				}
				catch(Exception e)
				{
					LogFile.WriteLine("Failed to open connection to database!");
					LogFile.WriteLine("ConnectionString");
					LogFile.WriteLine(e.Message);
					LogFile.Close();
					return -1;
				}

				try
				{
					using(SqlCommand command = new SqlCommand(null, connection))
					{
						command.CommandText =
							"INSERT INTO ReportData (ComputerName, UserName, GameName, PlatformName, LanguageExt, TimeOfCrash, BuildVer, ChangelistVer, CommandLine, BaseDir, CallStack, EngineMode, TTP, Status, FixedChangelist) " +
							"VALUES (@ComputerName, @UserName, @GameName, @PlatformName, @LanguageExt, @TimeOfCrash, @BuildVer, @ChangelistVer, @CommandLine, @BaseDir, @CallStack, @EngineMode, @TTP, @Status, '0'); " +
							"SELECT CAST(scope_identity() AS int)";

						SqlParameter computerNameParam = new SqlParameter("@ComputerName", SqlDbType.VarChar, 50);
						computerNameParam.Value = ComputerName;
						command.Parameters.Add(computerNameParam);

						SqlParameter UserNameParam = new SqlParameter("@UserName", SqlDbType.VarChar, 50);
						UserNameParam.Value = UserName;
						command.Parameters.Add(UserNameParam);

						SqlParameter GameNameParam = new SqlParameter("@GameName", SqlDbType.VarChar, 50);
						GameNameParam.Value = GameName;
						command.Parameters.Add(GameNameParam);

						SqlParameter PlatformNameParam = new SqlParameter("@PlatformName", SqlDbType.VarChar, 50);
						PlatformNameParam.Value = PlatformName;
						command.Parameters.Add(PlatformNameParam);

						SqlParameter LanguageExtParam = new SqlParameter("@LanguageExt", SqlDbType.VarChar, 50);
						LanguageExtParam.Value = LanguageExt;
						command.Parameters.Add(LanguageExtParam);

						SqlParameter TimeOfCrashParam = new SqlParameter("@TimeOfCrash", SqlDbType.DateTime);
						TimeOfCrashParam.Value = ConvertDumpTimeToDateTime(TimeOfCrash);
						command.Parameters.Add(TimeOfCrashParam);

						SqlParameter BuildVerParam = new SqlParameter("@BuildVer", SqlDbType.VarChar, 50);
						BuildVerParam.Value = BuildVer;
						command.Parameters.Add(BuildVerParam);

						SqlParameter ChangelistVerParam = new SqlParameter("@ChangelistVer", SqlDbType.VarChar, 50);
						ChangelistVerParam.Value = ChangelistVer;
						command.Parameters.Add(ChangelistVerParam);

						SqlParameter CommandLineParam = new SqlParameter("@CommandLine", SqlDbType.VarChar, 512);
						CommandLineParam.Value = CommandLine;
						command.Parameters.Add(CommandLineParam);

						SqlParameter BaseDirParam = new SqlParameter("@BaseDir", SqlDbType.VarChar, 260);
						BaseDirParam.Value = BaseDir;
						command.Parameters.Add(BaseDirParam);

						SqlParameter CallStackParam = new SqlParameter("@CallStack", SqlDbType.VarChar, 3000);
						CallStackParam.Value = CallStack;
						command.Parameters.Add(CallStackParam);

						SqlParameter EngineModeParam = new SqlParameter("@EngineMode", SqlDbType.VarChar, 50);
						EngineModeParam.Value = EngineMode;
						command.Parameters.Add(EngineModeParam);

						SqlParameter TTPParam = new SqlParameter("@TTP", SqlDbType.VarChar, 50);
						TTPParam.Value = "0";
						command.Parameters.Add(TTPParam);

						SqlParameter StatusParam = new SqlParameter("@Status", SqlDbType.VarChar, 50);
						//todo: want to add function here to set coders and testers to "coder" and "tester" Status automatically when they are added.
						StatusParam.Value = "New";
						command.Parameters.Add(StatusParam);

						// Call Prepare after setting the Commandtext and Parameters.
						command.Prepare();
						newID = (Int32)command.ExecuteScalar();
						connection.Close();
					}
				}
				catch(Exception e)
				{
					LogFile.WriteLine("Exception caught!");
					LogFile.WriteLine(e.Message);
					LogFile.Close();
					connection.Close();
					return -1;
				}
			}

			LogFile.WriteLine("Successfully created new record with id " + newID.ToString());
			LogFile.Close();
		}

        return newID;
    }

    /**
     * AddCrashDescription - updates the record identified by rowID with a crash description
     * 
     * @param rowID - the id of the row to update
     * @param CrashDescription - the new crash description
     * 
     * @return bool - true if successful
     */
    [WebMethod]
    public bool AddCrashDescription(int rowID, string CrashDescription, string Summary)
    {
        ConnectionStringSettings connSettings = ConfigurationManager.ConnectionStrings["DatabaseConnection"];
		using(SqlConnection connection = new SqlConnection(connSettings.ConnectionString))
		{
			string LogFileName = ConfigurationManager.AppSettings["LogFileName"];
			using(StreamWriter LogFile = new StreamWriter(LogFileName, true))
			{
				try
				{
					connection.Open();
				}
				catch(Exception e)
				{
					LogFile.WriteLine("Failed to open connection to database!");
					LogFile.WriteLine("ConnectionString");
					LogFile.WriteLine(e.Message);
					LogFile.Close();
					return false;
				}

				int rowsAffected = 0;

				try
				{

					using(SqlCommand command = new SqlCommand(null, connection))
					{
						command.CommandText =
							"UPDATE ReportData SET CrashDescription = @CrashDescription, Summary = @Summary WHERE ID = @RowID";

						SqlParameter CrashDescriptionParam = new SqlParameter("@CrashDescription", SqlDbType.VarChar, 1024);
						CrashDescriptionParam.Value = CrashDescription;
						command.Parameters.Add(CrashDescriptionParam);

						SqlParameter SummaryParam = new SqlParameter("@Summary", SqlDbType.VarChar, 1024);
						SummaryParam.Value = Summary;
						command.Parameters.Add(SummaryParam);

						SqlParameter rowIDParam = new SqlParameter("@RowID", SqlDbType.Int);
						rowIDParam.Value = rowID;
						command.Parameters.Add(rowIDParam);

						// Call Prepare after setting the Commandtext and Parameters.
						command.Prepare();
						rowsAffected = command.ExecuteNonQuery();

						connection.Close();
					}
				}
				catch(Exception e)
				{
					LogFile.WriteLine("Exception caught!");
					LogFile.WriteLine(e.Message);
					LogFile.Close();
					connection.Close();
					return false;
				}

				LogFile.WriteLine("Successfully updated Crash Description, " + rowsAffected.ToString() + " Rows updated");
				LogFile.Close();
			}
		}

        return true;
    }
    
}


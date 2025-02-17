using System;
using System.Net;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Text;
using System.IO;

namespace AutoReporter
{
    class UploadReportFiles
    {

        string StripPath(string PathName)
        {
            int LastBackSlashIndex = PathName.LastIndexOf("\\");
            int LastForwardSlashIndex = PathName.LastIndexOf("/");

            if (LastBackSlashIndex > LastForwardSlashIndex && LastBackSlashIndex >= 0)
            {
                return PathName.Substring(LastBackSlashIndex + 1);
            }
            else if (LastForwardSlashIndex >= 0)
            {
                return PathName.Substring(LastForwardSlashIndex + 1);
            }
            return PathName;
        }

        /** 
         * UploadFiles - uploads the two files using HTTP POST
         * 
         * @param LogFilename - the local log file
         * @param IniFilename - the local ini file
         * @param uniqueID - the id of the crash report associated with these files. 
         * @return bool - true if successful
         */
        public bool UploadFiles(string LogFilename, string IniFilename, string MiniDumpFilename, int uniqueID, OutputLogFile logFile)
        {
            logFile.WriteLine("Uploading files " + LogFilename + ", " + IniFilename + " and " + MiniDumpFilename);

            try 
            {
                WebClient client = new WebClient();
                string UploadReportURL = "http://crashreport/UploadReportFiles.aspx";
                //string UploadReportURL = "http://localhost/AutoReportService/UploadReportFiles.aspx";

                client.Headers.Add("NewFolderName", uniqueID.ToString());

                //copy the log before trying to upload it, since the engine may still have it open for writing
                //in which case a direct upload would fail
                string TempLogFilename = LogFilename + "_AutoReportTemp.log";

                try
                {
                    File.Copy(LogFilename, TempLogFilename, true);
                }
                catch (Exception e)
                {
                    logFile.WriteLine(e.Message);
                    logFile.WriteLine("Couldn't copy log to " + TempLogFilename + ", continuing...");
                }

                client.Headers.Add("LogName", StripPath(TempLogFilename));

                byte[] responseArray;

                try
                {
                    responseArray = client.UploadFile(UploadReportURL, "POST", TempLogFilename);
                }
                catch (WebException webEx)
                {
                    logFile.WriteLine(webEx.Message);
                    logFile.WriteLine(webEx.InnerException.Message);
                    logFile.WriteLine("Couldn't upload log, continuing...");
                }

                try
                {
                    File.Delete(TempLogFilename);
                }
                catch (Exception)
                {
                }

                try
                {
                    responseArray = client.UploadFile(UploadReportURL, "POST", IniFilename);
                }
                catch (WebException webEx)
                {
                    logFile.WriteLine(webEx.Message);
                    logFile.WriteLine(webEx.InnerException.Message);
                    logFile.WriteLine("Couldn't upload ini, continuing...");
                }

                client.Headers.Add("SaveFileName", "MiniDump.dmp");

                try
                {
                    responseArray = client.UploadFile(UploadReportURL, "POST", MiniDumpFilename);
                }
                catch (WebException webEx)
                {
                    logFile.WriteLine(webEx.Message);
                    logFile.WriteLine(webEx.InnerException.Message);
                    logFile.WriteLine("Couldn't upload mini dump, continuing...");
                }

            } catch (WebException webEx) {
                logFile.WriteLine(webEx.Message);
                logFile.WriteLine(webEx.InnerException.Message);
                return false;
            }

            return true;
        }
    }
}

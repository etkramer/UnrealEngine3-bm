using System;
using System.Text;
using System.Data;
using System.Configuration;
using System.Collections;
using System.Collections.Specialized;
using System.Web;
using System.Web.Security;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Web.UI.WebControls.WebParts;
using System.Web.UI.HtmlControls;
using System.IO;

public partial class UploadReportFiles : System.Web.UI.Page
{
    /**
     * Uploads files and places them in the folder specified by the NewFolderName key
     * in the HTTP header.
     */
    protected void Page_Load(object sender, EventArgs e)
    {
        string NewFolderName = "";
        string SaveFileName = "";
        string LogFileName = "Launch.log";

        foreach (string headerString in Request.Headers.AllKeys)
        {
            if (headerString.Equals("NewFolderName"))
            {
                NewFolderName = Request.Headers[headerString];
            }
            else if (headerString.Equals("SaveFileName"))
            {
                SaveFileName = Request.Headers[headerString];
            }
            else if (headerString.Equals("LogName"))
            {
                //get the log filename from the header
                LogFileName = Request.Headers[headerString];
            }
        }

        if (NewFolderName.Length > 0)
        {
            
            string SaveFilesPath = ConfigurationManager.AppSettings["SaveFilesPath"];
            string NewPath = SaveFilesPath + NewFolderName;

            foreach (string fileString in Request.Files.AllKeys)
            {
                HttpPostedFile file = Request.Files[fileString];
                if (SaveFileName.Length == 0)
                {
                    SaveFileName = file.FileName;
                }
                //if this is the log file, rename it so that it can be identified by the web site
                if (file.FileName.Contains(LogFileName))
                {
                    SaveFileName = "Launch.log";
                }

                file.SaveAs(NewPath + "_" + SaveFileName);

            }
        }
    }
}

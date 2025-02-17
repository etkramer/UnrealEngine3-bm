using System;
using System.Configuration;
using System.Collections;
using System.Data;
using System.Drawing;
using System.Web;
using System.Web.Security;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Web.UI.WebControls.WebParts;
using System.Web.UI.HtmlControls;
using RemotableType;
using UnrealProp;

public partial class Web_User_UserActions : System.Web.UI.Page
{
    protected void Page_Load( object sender, EventArgs e )
    {
        if( !Global.IsUser( Context.User.Identity.Name ) )
        {
            Response.Redirect( "~/Default.aspx" );
        }

        float GBPropped, TBPropped;
        float DataRate = 0.0f;
        long NumProps = 0;
        long BytesPropped = 0;

        NumProps = Global.IUPMS.Utils_GetStats( "", "", DateTime.MinValue, ref BytesPropped, ref DataRate );
        TBPropped = BytesPropped / ( 1024.0f * 1024.0f * 1024.0f * 1024.0f );

        ProppedAmountLabel.Text = TBPropped.ToString( "0.000" ) + " TB propped in " + NumProps.ToString() + " props! ( " + DataRate.ToString( "f" ) + " MB/s )";

        Global.IUPMS.Utils_GetStats( "Gear", "Xenon", DateTime.MinValue, ref BytesPropped, ref DataRate );
        GBPropped = BytesPropped / ( 1024.0f * 1024.0f * 1024.0f );
        ProppedAmountLabelGearXenon.Text = GBPropped.ToString( "0.00" ) + " GB of Gears2 Xenon propped! ( " + DataRate.ToString( "f" ) + " MB/s )";

        Global.IUPMS.Utils_GetStats( "Gear", "PC", DateTime.MinValue, ref BytesPropped, ref DataRate );
        GBPropped = BytesPropped / ( 1024.0f * 1024.0f * 1024.0f );
        ProppedAmountLabelGearPC.Text = GBPropped.ToString( "0.00" ) + " GB of Gears2 PC propped ( " + DataRate.ToString( "f" ) + " MB/s )";

        Global.IUPMS.Utils_GetStats( "UT", "Xenon", DateTime.MinValue, ref BytesPropped, ref DataRate );
        GBPropped = BytesPropped / ( 1024.0f * 1024.0f * 1024.0f );
        ProppedAmountLabelUTXenon.Text = GBPropped.ToString( "0.00" ) + " GB of UT Xenon propped ( " + DataRate.ToString( "f" ) + " MB/s )";

        Global.IUPMS.Utils_GetStats( "UT", "PC", DateTime.MinValue, ref BytesPropped, ref DataRate );
        GBPropped = BytesPropped / ( 1024.0f * 1024.0f * 1024.0f );
        ProppedAmountLabelUTPC.Text = GBPropped.ToString( "0.00" ) + " GB of UT PC propped ( " + DataRate.ToString( "f" ) + " MB/s )";
    }
}

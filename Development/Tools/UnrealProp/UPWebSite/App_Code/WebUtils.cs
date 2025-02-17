using System;
using System.Data;
using System.Configuration;
using System.Web;
using System.Web.Security;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Web.UI.WebControls.WebParts;
using System.Web.UI.HtmlControls;
using RemotableType;
using UnrealProp;

static public class WebUtils
{
    static public string GetParamFromCCD( string ListItem, bool value )
    {
        string Parameter = "-1";

        if( ListItem != null )
        {
            int i = ListItem.IndexOf( ":::" );
            if( i >= 0 )
            {
                if( !value )
                {
                    Parameter = ListItem.Substring( i + 3 );
                }
                else
                {
                    Parameter = ListItem.Substring( 0, i );
                }

                if( Parameter == "" )
                {
                    Parameter = "-1";
                }
            }
        }

        return( Parameter.Trim() );
    }

}


<%@ Page Language="C#" MasterPageFile="~/MasterPage.master" CodeFile="UserActions.aspx.cs" Inherits="Web_User_UserActions" Title="UnrealProp - User Actions" %>
<asp:Content ID="Content1" ContentPlaceHolderID="ContentPlaceHolder1" Runat="Server">
<center>
    <asp:Label ID="ProppedAmountLabel" runat="server" Font-Size="X-Large" ForeColor="DarkGreen" >
    </asp:Label>
    <br />
    <br />
    <asp:Label ID="ProppedAmountLabelGearXenon" runat="server" Font-Size="Large" ForeColor="DarkGreen" >
    </asp:Label>
    <br />
    <asp:Label ID="ProppedAmountLabelGearPC" runat="server" Font-Size="Large" ForeColor="DarkGreen" >
    </asp:Label>
    <br />
    <asp:Label ID="ProppedAmountLabelUTXenon" runat="server" Font-Size="Large" ForeColor="DarkGreen" >
    </asp:Label>
    <br />
    <asp:Label ID="ProppedAmountLabelUTPC" runat="server" Font-Size="Large" ForeColor="DarkGreen" >
    </asp:Label>
    <br />
</center>
This is the main page for user actions.<br />
<br />
Click on "Task Scheduler" to select a build to propagate.<br />
Click on "My Tasks" to view the status of your current and past propagations.<br />
<br />
If you wish to add a target machine, ask an UnrealProp Admin. John Scott, Scott Bigwood, Preston Thorne and Ray Davis are admins.<br />

</asp:Content>


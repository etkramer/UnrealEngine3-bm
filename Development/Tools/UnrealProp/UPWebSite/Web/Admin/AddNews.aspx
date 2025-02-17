<%@ Page Language="C#" MasterPageFile="~/MasterPage.master" Title="UnrealProp - Manage News" CodeFile="AddNews.aspx.cs" Inherits="Web_Admin_AddNews" ValidateRequest="false" %>
<asp:Content ID="Content1" ContentPlaceHolderID="ContentPlaceHolder1" Runat="Server">
Enter your news items here; HTML tags are allowed.
<br />
<br />
<asp:TextBox ID="NewsTextBox" runat="server" Width="95%" Rows="30" TextMode="MultiLine" OnPreRender="NewsTextBox_PreRender" >
</asp:TextBox>
<br />
<br />
<asp:Button ID="SaveNewsButton" runat="server" Text="Save News" Font-Size="Small" Width="15%" OnClick="SaveNewsButton_Click" />
</asp:Content>


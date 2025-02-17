<%@ Page Language="C#" AutoEventWireup="true" CodeFile="Default.aspx.cs" Inherits="_Default" %>

<%@ Register Assembly="AjaxControlToolkit" Namespace="AjaxControlToolkit" TagPrefix="ajaxToolkit" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" >
<head runat="server">
<title>Epic Build System - Main Page</title>
</head>
<body>
<center>

<form id="Form1" runat="server">
<asp:ScriptManager ID="BuilderScriptManager" runat="server" />

<asp:Label ID="Label_Title" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="XX-Large" ForeColor="Blue" Height="48px" Text="Epic Build System" Width="320px"></asp:Label>
<br />
<asp:Label ID="Label_Welcome" runat="server" Height="32px" Width="384px" Font-Bold="True" Font-Names="Arial" Font-Size="Small" ForeColor="Blue"></asp:Label>

<br />
<asp:Button ID="Button_TriggerBuild" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="Large" Text="Trigger Build" OnClick="Button_TriggerBuild_Click" />
<br />
<br />
<asp:Button ID="Button_LegacyUTPC" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="Large" Text="UT3 (PC Branch)" OnClick="Button_TriggerBuild_Click" />
<br />
<br />
<asp:Button ID="Button_LegacyUTPS3" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="Large" Text="UT3 (PS3 Branch)" OnClick="Button_TriggerBuild_Click" />
<br />
<br />
<asp:Button ID="Button_LegacyUTX360" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="Large" Text="UT3 (X360 Branch)" OnClick="Button_TriggerBuild_Click" />
<br />
<br />
<asp:Button ID="Button_TriggerPCS" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="Large" Text="Trigger PCS" OnClick="Button_TriggerBuild_Click" />
<br />
<br />
<asp:Button ID="Button_TriggerCook" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="Large" Text="Trigger Cook" OnClick="Button_TriggerBuild_Click" />
<br />
<br />
<asp:Button ID="Button_PromoteBuild" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="Large" Text="Promote Build" OnClick="Button_TriggerBuild_Click" />
<br />
<br />
<asp:Button ID="Button_Verification" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="Large" Text="Trigger Verification Build" OnClick="Button_TriggerBuild_Click" />
<br />
<br />
<asp:Button ID="Button_RestartControllers" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="Large" Text="Restart Controllers" OnClick="Button_RestartControllers_Click" />
<br />
<br />

</center>
<center>
<asp:UpdatePanel ID="ScriptUpdatePanel" runat="Server" UpdateMode="Conditional">
<ContentTemplate>
<fieldset>
<Legend style="color:BlueViolet"><b>Builds in Progress</b></Legend>
<asp:Repeater ID="Repeater_BuildLog" runat="server" OnItemCommand="Repeater_BuildLog_ItemCommand">
<ItemTemplate>

<table width="80%">
<tr><td align="center">
<asp:Label ID="Label_BuildLog1" runat="server" Font-Bold="True" ForeColor=<%# CheckConnected( DataBinder.Eval(Container.DataItem, "[\"CurrentTime\"]") ) %> Text=<%# DataBinder.Eval(Container.DataItem, "[\"Machine\"]") %> />
is building from ChangeList :
<asp:Label ID="Label_BuilderLog2" runat="server" Font-Bold="True" ForeColor=<%# CheckConnected( DataBinder.Eval(Container.DataItem, "[\"CurrentTime\"]") ) %> Text=<%# DataBinder.Eval(Container.DataItem, "[\"ChangeList\"]") %> />
<br />
<asp:Label ID="Label_BuilderLog3" runat="server" Font-Bold="True" ForeColor=<%# CheckConnected( DataBinder.Eval(Container.DataItem, "[\"CurrentTime\"]") ) %> Text=<%# DataBinder.Eval(Container.DataItem, "[\"CurrentStatus\"]") %> />
</td><td>
<asp:Label ID="Label_BuilderLog4" runat="server" Font-Bold="True" ForeColor="DarkGreen" Text=<%# DateDiff( DataBinder.Eval(Container.DataItem, "[\"BuildStarted\"]") ) %> />
</td><td>
Triggered by : 
<asp:Label ID="Label_BuilderLog5" runat="server" Font-Bold="True" ForeColor="DarkGreen" Text=<%# DataBinder.Eval(Container.DataItem, "[\"Operator\"]") %> />
</td><td align="center" Width="40%">
<asp:LinkButton ID="Button_StopBuild" runat="server" Font-Bold="True" Width="384" ForeColor="Red" Font-Size="Large" Text=<%# "Stop " + DataBinder.Eval(Container.DataItem, "[\"Description\"]") %> />
<ajaxToolkit:ConfirmButtonExtender ID="ConfirmButtonExtender1" runat="server" TargetControlID="Button_StopBuild" ConfirmText="Are you sure you want to stop the build?" /> 
</td></tr>
</table>

</ItemTemplate>
</asp:Repeater>

<asp:Timer ID="ScriptTimer" runat="server" Interval="2000" OnTick="ScriptTimer_Tick" />  
</fieldset>
</ContentTemplate>
</asp:UpdatePanel>

        &nbsp; &nbsp;

<asp:UpdatePanel ID="JobUpdatePanel" runat="Server" UpdateMode="Always">
<ContentTemplate>
<fieldset>
<Legend style="color:BlueViolet"><b>Jobs in Progress</b></Legend>
<asp:Repeater ID="Repeater_JobLog" runat="server">
<ItemTemplate>

<table width="80%">
<tr><td align="center">
<asp:Label ID="Label_JobLog1" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"Machine\"]") %> />
<asp:Label ID="Label_JobLog2" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DateDiff2( DataBinder.Eval(Container.DataItem, "[\"BuildStarted\"]") ) %> />
:
<asp:Label ID="Label_JobLog3" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"CurrentStatus\"]") %> />
</td>
</tr>
</table>

</ItemTemplate>
</asp:Repeater>
</fieldset>
</ContentTemplate>
</asp:UpdatePanel>

        &nbsp; &nbsp;
        
<asp:UpdatePanel ID="VerifyUpdatePanel" runat="Server" UpdateMode="Conditional">
<ContentTemplate>
<fieldset>
<Legend style="color:BlueViolet"><b>Verification Builds in Progress</b></Legend>
<asp:Repeater ID="Repeater_Verify" runat="server" OnItemCommand="Repeater_BuildLog_ItemCommand">
<ItemTemplate>

<table width="80%">
<tr><td align="center">
<asp:Label ID="Label_BuildLog1" runat="server" Font-Bold="True" ForeColor="DarkGreen" Text=<%# DataBinder.Eval(Container.DataItem, "[\"Machine\"]") %> />
is building from ChangeList :
<asp:Label ID="Label_BuilderLog2" runat="server" Font-Bold="True" ForeColor="DarkGreen" Text=<%# DataBinder.Eval(Container.DataItem, "[\"ChangeList\"]") %> />
<br />
<asp:Label ID="Label_BuilderLog3" runat="server" Font-Bold="True" ForeColor="DarkGreen" Text=<%# DataBinder.Eval(Container.DataItem, "[\"CurrentStatus\"]") %> />
</td><td>
<asp:Label ID="Label_BuilderLog4" runat="server" Font-Bold="True" ForeColor="DarkGreen" Text=<%# DateDiff( DataBinder.Eval(Container.DataItem, "[\"BuildStarted\"]") ) %> />
</td><td>
Triggered by : 
<asp:Label ID="Label_BuilderLog5" runat="server" Font-Bold="True" ForeColor="DarkGreen" Text=<%# DataBinder.Eval(Container.DataItem, "[\"Operator\"]") %> />
</td><td align="center" Width="40%">
<asp:LinkButton ID="Button_StopBuild" runat="server" Font-Bold="True" Width="384" ForeColor="Red" Font-Size="Large" Text=<%# "Stop " + DataBinder.Eval(Container.DataItem, "[\"Description\"]") %> />
<ajaxToolkit:ConfirmButtonExtender ID="ConfirmButtonExtender1" runat="server" TargetControlID="Button_StopBuild" ConfirmText="Are you sure you want to stop the build?" /> 
</td></tr>
</table>

</ItemTemplate>
</asp:Repeater>

<asp:Timer ID="VerifyTimer" runat="server" Interval="2000" OnTick="VerifyTimer_Tick" />  
</fieldset>
</ContentTemplate>
</asp:UpdatePanel>

        &nbsp; &nbsp;

<br />
    </center>
    <center>
   <asp:UpdatePanel ID="MainBranchUpdatePanel" runat="Server" UpdateMode="Always">
<ContentTemplate>
 
                <asp:Label ID="Label_Main" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="X-Large"
            Height="36px" Text="Main UnrealEngine3 Branch" Width="640px" ForeColor="BlueViolet"></asp:Label><br />
            
<asp:Repeater ID="Repeater_MainBranch" runat="server">
<ItemTemplate>
Last good build of
<asp:Label ID="Label_Status1" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"Description\"]") %> />
was from ChangeList 
<asp:Label ID="Label_Status2" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"LastGoodChangeList\"]") %> />
on 
<asp:Label ID="Label_Status3" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"LastGoodDateTime\"]") %> />
<asp:Label ID="Label_Status5" runat="server" Font-Bold="True" ForeColor="Blue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"DisplayLabel\"]") %> />
<asp:Label ID="Label_Status4" runat="server" Font-Bold="True" ForeColor="Green" Text=<%# DataBinder.Eval(Container.DataItem, "[\"Status\"]") %> />
<br />
</ItemTemplate>
</asp:Repeater>
</ContentTemplate>
</asp:UpdatePanel>

        &nbsp; &nbsp;

<br />

<asp:UpdatePanel ID="UT3BranchesUpdatePanel" runat="Server" UpdateMode="Always">
<ContentTemplate>

<asp:Label ID="Label_UT3" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="X-Large"
Height="36px" Text="UT3 PC/PS3/Xenon Branches" Width="640px" ForeColor="BlueViolet"></asp:Label><br />

<asp:Repeater ID="Repeater_UT3Branches" runat="server">
<ItemTemplate>
Last good build of
<asp:Label ID="Label_Status1" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"Description\"]") %> />
was from ChangeList 
<asp:Label ID="Label_Status2" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"LastGoodChangeList\"]") %> />
on 
<asp:Label ID="Label_Status3" runat="server" Font-Bold="True" ForeColor="DarkBlue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"LastGoodDateTime\"]") %> />
<asp:Label ID="Label_Status5" runat="server" Font-Bold="True" ForeColor="Blue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"DisplayLabel\"]") %> />
<asp:Label ID="Label_Status4" runat="server" Font-Bold="True" ForeColor="Green" Text=<%# DataBinder.Eval(Container.DataItem, "[\"Status\"]") %> />
<br />
</ItemTemplate>
</asp:Repeater>
</ContentTemplate>
</asp:UpdatePanel>

        &nbsp; &nbsp;

<asp:UpdatePanel ID="BuildersUpdatePanel" runat="Server" UpdateMode="Always">
<ContentTemplate>
<fieldset>
<Legend style="color:BlueViolet"><b>Available Builders</b></Legend>

<asp:Repeater ID="Repeater_Builders" runat="server">
<ItemTemplate>
<asp:Label ID="Label_Builder1" runat="server" Font-Bold="True" ForeColor=<%# CheckConnected( DataBinder.Eval(Container.DataItem, "[\"CurrentTime\"]") ) %> Text=<%# GetAvailability( DataBinder.Eval(Container.DataItem, "[\"Machine\"]"), DataBinder.Eval(Container.DataItem, "[\"CurrentTime\"]") ) %> />
<br />
</ItemTemplate>
</asp:Repeater>
</fieldset>
</ContentTemplate>
</asp:UpdatePanel>
</form>
</center>
   
</body>
</html>

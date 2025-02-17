<%@ Page Language="C#" AutoEventWireup="true" CodeFile="Promote.aspx.cs" Inherits="Promote" Debug="true" EnableEventValidation="false" %>
<%@ Import Namespace="System.Data" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" >
<head runat="server">
    <title>Promote a Build</title>
</head>
<body>

<center>
  <form id="form1" runat="server">
    <div>
        <asp:Label ID="Label_Title" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="XX-Large"
            Height="56px" Text="Epic Build System - Promote Build" Width="640px" ForeColor="Blue">
        </asp:Label>
        <br />
        <br />
        <asp:Label ID="Label_Welcome" runat="server" Height="32px" Width="384px" Font-Bold="True" Font-Names="Arial" Font-Size="Small" ForeColor="Blue">
        </asp:Label>
        <br />
        <br />
        <asp:Label ID="Label_Subtitle1" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="X-Large"
            Height="36px" Text="Promote Build from UnrealEngine3 branch" Width="640px" ForeColor="BlueViolet">
        </asp:Label>
        <table>
          <asp:Repeater ID="UnrealEngine3Repeater" runat="server">
            <ItemTemplate>
              <tr>
                <td>
                  <asp:Label ID="Label" runat="server" Font-Bold="True" Height="26px" ForeColor="Blue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"BuildLabel\"]") %> Width="384px" />
                </td>
                <td>
                  <asp:Repeater ID="ChildRepeater" runat="server" DataSource=<%# ( ( DataRowView )Container.DataItem ).Row.GetChildRows( "PseudoRelation" ) %> OnItemCommand="BuilderDBRepeater_PromoteItemCommand_UE3" >
			        <itemtemplate>
                      <asp:Button ID="Button_Promote" runat="server" Font-Bold="True" Height="26px" ForeColor="DarkGreen" Width="224px" 
                                CommandName=<%# DataBinder.Eval(Container.DataItem, "[\"CommandID\"]") %>
                                CommandArgument=<%# DataBinder.Eval(Container.DataItem, "[\"BuildLabel\"]") %>
                                Text=<%# DataBinder.Eval(Container.DataItem, "[\"Description\"]") %> />
			        </itemtemplate>
		          </asp:Repeater> 
                </td>
              </tr>
            </ItemTemplate>
          </asp:Repeater>
        </table>
      </div>
      
      <br />
      
      <div>
        <asp:Label ID="LabelQA" runat="server" Font-Bold="True" Font-Names="Arial" Font-Size="X-Large"
            Height="36px" Text="Promote Build to QA Candidate" Width="640px" ForeColor="BlueViolet">
        </asp:Label>
        <table>
          <asp:Repeater ID="UnrealEngine3QARepeater" runat="server">
            <ItemTemplate>
              <tr>
                <td>
                  <asp:Label ID="Label" runat="server" Font-Bold="True" Height="26px" ForeColor="Blue" Text=<%# DataBinder.Eval(Container.DataItem, "[\"BuildLabel\"]") %> Width="384px" />
                </td>
                <td>
                  <asp:Repeater ID="ChildRepeater" runat="server" DataSource=<%# ( ( DataRowView )Container.DataItem ).Row.GetChildRows( "PseudoRelation" ) %> OnItemCommand="BuilderDBRepeater_PromoteItemCommand_QA" >
                    <itemtemplate>
                      <asp:Button ID="Button_Promote" runat="server" Font-Bold="True" Height="26px" ForeColor="DarkGreen" Width="224px" 
                                CommandName=<%# DataBinder.Eval(Container.DataItem, "[\"CommandID\"]") %>
                                CommandArgument=<%# DataBinder.Eval(Container.DataItem, "[\"BuildLabel\"]") %>
                                Text=<%# DataBinder.Eval(Container.DataItem, "[\"Description\"]") %> />
                    </itemtemplate>
                  </asp:Repeater> 
                </td>
              </tr>
            </ItemTemplate>
          </asp:Repeater>
        </table>
      </div>

      <br />        
      <br />
        
      <div>
        <asp:Label ID="Label_ChangesRange" runat="server" Font-Bold="True" ForeColor="Teal" Font-Names="Arial" Font-Size="X-Large" />
        <br />
        <asp:Button ID="Button_ChangesRange" runat="server" Font-Bold="True" Text="Send QA Build Changes" OnClick="Button_QAChanges_Click" />
        <br />
      </div>        
        
      <br />

<asp:SqlDataSource ID="BuilderDBSource_Commands" runat="server" ConnectionString="<%$ ConnectionStrings:Perf_BuildConnectionString %>"
                SelectCommandType="StoredProcedure" SelectCommand="SelectBuilds">
  <SelectParameters>
    <asp:QueryStringParameter Name="DisplayID" Type="Int32" DefaultValue="0" />
    <asp:QueryStringParameter Name="DisplayDetailID" Type="Int32" DefaultValue="400" />
  </SelectParameters>
</asp:SqlDataSource>
 
<asp:Repeater ID="Repeater_Commands" runat="server" DataSourceID="BuilderDBSource_Commands">
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
        
    </form>

</center>    
</body>
</html>


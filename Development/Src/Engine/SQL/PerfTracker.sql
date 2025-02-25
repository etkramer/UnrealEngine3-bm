/****** Object:  Table [dbo].[Tags]    Script Date: 03/19/2008 23:40:08 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[Tags]') AND type in (N'U'))
BEGIN
CREATE TABLE [Tags](
	[TagID] [int] IDENTITY(1,1) NOT NULL,
	[Tag] [varchar](64) NOT NULL,
 CONSTRAINT [PK_Tags] PRIMARY KEY CLUSTERED 
(
	[TagID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
/****** Object:  Table [dbo].[StatGroups]    Script Date: 03/19/2008 23:40:06 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[StatGroups]') AND type in (N'U'))
BEGIN
CREATE TABLE [StatGroups](
	[StatGroupID] [int] IDENTITY(1,1) NOT NULL,
	[StatGroupName] [varchar](255) NOT NULL,
 CONSTRAINT [PK_StatGroups] PRIMARY KEY CLUSTERED 
(
	[StatGroupID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
/****** Object:  Table [dbo].[TaskParameters]    Script Date: 03/19/2008 23:40:09 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[TaskParameters]') AND type in (N'U'))
BEGIN
CREATE TABLE [TaskParameters](
	[TaskParameterID] [int] IDENTITY(1,1) NOT NULL,
	[TaskParameter] [varchar](255) NOT NULL,
 CONSTRAINT [PK_TaskParameters] PRIMARY KEY CLUSTERED 
(
	[TaskParameterID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
/****** Object:  Table [dbo].[Games]    Script Date: 03/19/2008 23:39:52 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[Games]') AND type in (N'U'))
BEGIN
CREATE TABLE [Games](
	[GameID] [int] IDENTITY(1,1) NOT NULL,
	[GameName] [char](32) NOT NULL,
 CONSTRAINT [PK_Games] PRIMARY KEY CLUSTERED 
(
	[GameID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
/****** Object:  Table [dbo].[Levels]    Script Date: 03/19/2008 23:39:53 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[Levels]') AND type in (N'U'))
BEGIN
CREATE TABLE [Levels](
	[LevelID] [int] IDENTITY(1,1) NOT NULL,
	[LevelName] [varchar](64) NOT NULL,
 CONSTRAINT [PK_Levels] PRIMARY KEY CLUSTERED 
(
	[LevelID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
/****** Object:  Table [dbo].[Platforms]    Script Date: 03/19/2008 23:39:57 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[Platforms]') AND type in (N'U'))
BEGIN
CREATE TABLE [Platforms](
	[PlatformID] [int] IDENTITY(1,1) NOT NULL,
	[PlatformName] [varchar](64) NOT NULL,
 CONSTRAINT [PK_Platforms] PRIMARY KEY CLUSTERED 
(
	[PlatformID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
/****** Object:  Table [dbo].[Tasks]    Script Date: 03/19/2008 23:40:10 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[Tasks]') AND type in (N'U'))
BEGIN
CREATE TABLE [Tasks](
	[TaskID] [int] IDENTITY(1,1) NOT NULL,
	[TaskDescription] [varchar](255) NOT NULL,
 CONSTRAINT [PK_Tasks] PRIMARY KEY CLUSTERED 
(
	[TaskID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
/****** Object:  Table [dbo].[Configs]    Script Date: 03/19/2008 23:39:51 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[Configs]') AND type in (N'U'))
BEGIN
CREATE TABLE [Configs](
	[ConfigID] [int] IDENTITY(1,1) NOT NULL,
	[ConfigName] [varchar](64) NOT NULL,
 CONSTRAINT [PK_Configs] PRIMARY KEY CLUSTERED 
(
	[ConfigID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
/****** Object:  Table [dbo].[Results]    Script Date: 03/19/2008 23:39:58 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[Results]') AND type in (N'U'))
BEGIN
CREATE TABLE [Results](
	[ResultID] [int] IDENTITY(1,1) NOT NULL,
	[ResultDescription] [varchar](255) NOT NULL,
 CONSTRAINT [PK_Results] PRIMARY KEY CLUSTERED 
(
	[ResultID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
/****** Object:  Table [dbo].[Locations]    Script Date: 03/19/2008 23:39:55 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[Locations]') AND type in (N'U'))
BEGIN
CREATE TABLE [Locations](
	[LocationID] [int] IDENTITY(1,1) NOT NULL,
	[LocX] [int] NOT NULL,
	[LocY] [int] NOT NULL,
	[LocZ] [int] NOT NULL,
	[RotYaw] [int] NOT NULL,
	[RotPitch] [int] NOT NULL,
	[RotRoll] [int] NOT NULL,
 CONSTRAINT [PK_Locations] PRIMARY KEY CLUSTERED 
(
	[LocationID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
/****** Object:  Table [dbo].[CmdLines]    Script Date: 03/19/2008 23:39:50 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[CmdLines]') AND type in (N'U'))
BEGIN
CREATE TABLE [CmdLines](
	[CmdLineID] [int] IDENTITY(1,1) NOT NULL,
	[CmdLine] [varchar](max) NOT NULL,
 CONSTRAINT [PK_CmdLines] PRIMARY KEY CLUSTERED 
(
	[CmdLineID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
/****** Object:  Table [dbo].[GameTypes]    Script Date: 03/19/2008 23:39:53 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[GameTypes]') AND type in (N'U'))
BEGIN
CREATE TABLE [GameTypes](
	[GameTypeID] [int] IDENTITY(1,1) NOT NULL,
	[GameType] [varchar](64) NOT NULL,
 CONSTRAINT [PK_GameTypes] PRIMARY KEY CLUSTERED 
(
	[GameTypeID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
/****** Object:  Table [dbo].[Machines]    Script Date: 03/19/2008 23:39:56 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[Machines]') AND type in (N'U'))
BEGIN
CREATE TABLE [Machines](
	[MachineID] [int] IDENTITY(1,1) NOT NULL,
	[MachineName] [char](32) NOT NULL,
 CONSTRAINT [PK_Machines] PRIMARY KEY CLUSTERED 
(
	[MachineID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
/****** Object:  Table [dbo].[Users]    Script Date: 03/19/2008 23:40:11 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[Users]') AND type in (N'U'))
BEGIN
CREATE TABLE [Users](
	[UserID] [int] IDENTITY(1,1) NOT NULL,
	[UserName] [varchar](64) NOT NULL,
 CONSTRAINT [PK_Users] PRIMARY KEY CLUSTERED 
(
	[UserID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
/****** Object:  Table [dbo].[Runs]    Script Date: 03/19/2008 23:40:04 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[Runs]') AND type in (N'U'))
BEGIN
CREATE TABLE [Runs](
	[RunID] [int] IDENTITY(1,1) NOT NULL,
	[ResultID] [int] NOT NULL,
	[PlatformID] [int] NOT NULL,
	[ConfigID] [int] NOT NULL,
	[Changelist] [int] NOT NULL,
	[MachineID] [int] NOT NULL,
	[UserID] [int] NOT NULL,
	[GameID] [int] NOT NULL,
	[GameTypeID] [int] NOT NULL,
	[TaskID] [int] NOT NULL,
	[TaskParameterID] [int] NOT NULL,
	[LevelID] [int] NOT NULL,
	[TagID] [int] NOT NULL,
	[CmdLineID] [int] NOT NULL,
	[Date] [datetime] NOT NULL CONSTRAINT [DF_Runs_Date]  DEFAULT (getdate()),
 CONSTRAINT [PK_Runs] PRIMARY KEY CLUSTERED 
(
	[RunID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
/****** Object:  Table [dbo].[Stats]    Script Date: 03/19/2008 23:40:07 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[Stats]') AND type in (N'U'))
BEGIN
CREATE TABLE [Stats](
	[StatID] [int] IDENTITY(1,1) NOT NULL,
	[StatGroupID] [int] NOT NULL,
	[StatName] [varchar](255) NOT NULL,
 CONSTRAINT [PK_Stats] PRIMARY KEY CLUSTERED 
(
	[StatID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
/****** Object:  Table [dbo].[RunData]    Script Date: 03/19/2008 23:39:59 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[RunData]') AND type in (N'U'))
BEGIN
CREATE TABLE [RunData](
	[RunDataID] [bigint] IDENTITY(1,1) NOT NULL,
	[RunID] [int] NOT NULL,
	[LocationID] [int] NOT NULL,
	[StatID] [int] NOT NULL,
	[StatValue] [float] NOT NULL,
 CONSTRAINT [PK_RunData] PRIMARY KEY CLUSTERED 
(
	[RunDataID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
/****** Object:  View [dbo].[RunDataView]    Script Date: 03/19/2008 23:40:11 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.views WHERE object_id = OBJECT_ID(N'[RunDataView]'))
EXEC dbo.sp_executesql @statement = N'CREATE VIEW [RunDataView]
AS
SELECT     dbo.Users.UserName, dbo.StatGroups.StatGroupName, dbo.Locations.LocX, dbo.Locations.LocY, dbo.Locations.LocZ, dbo.Locations.RotYaw, dbo.Locations.RotPitch, 
                      dbo.Locations.RotRoll, dbo.Levels.LevelName, dbo.GameTypes.GameType, dbo.Games.GameName, dbo.Configs.ConfigName, dbo.CmdLines.CmdLine, 
                      dbo.Machines.MachineName, dbo.Stats.StatName, dbo.Platforms.PlatformName, dbo.Results.ResultDescription, dbo.TaskParameters.TaskParameter, dbo.Tags.Tag, 
                      dbo.RunData.StatValue, dbo.Runs.Date, dbo.Runs.Changelist
FROM         dbo.Levels INNER JOIN
                      dbo.CmdLines INNER JOIN
                      dbo.RunData INNER JOIN
                      dbo.Locations ON dbo.RunData.LocationID = dbo.Locations.LocationID INNER JOIN
                      dbo.Runs ON dbo.RunData.RunID = dbo.Runs.RunID INNER JOIN
                      dbo.Machines ON dbo.Runs.MachineID = dbo.Machines.MachineID INNER JOIN
                      dbo.Platforms ON dbo.Runs.PlatformID = dbo.Platforms.PlatformID INNER JOIN
                      dbo.Results ON dbo.Runs.ResultID = dbo.Results.ResultID ON dbo.CmdLines.CmdLineID = dbo.Runs.CmdLineID INNER JOIN
                      dbo.Configs ON dbo.Runs.ConfigID = dbo.Configs.ConfigID INNER JOIN
                      dbo.Games ON dbo.Runs.GameID = dbo.Games.GameID INNER JOIN
                      dbo.GameTypes ON dbo.Runs.GameTypeID = dbo.GameTypes.GameTypeID ON dbo.Levels.LevelID = dbo.Runs.LevelID INNER JOIN
                      dbo.Stats ON dbo.RunData.StatID = dbo.Stats.StatID INNER JOIN
                      dbo.StatGroups ON dbo.Stats.StatGroupID = dbo.StatGroups.StatGroupID INNER JOIN
                      dbo.Tags ON dbo.Runs.TagID = dbo.Tags.TagID INNER JOIN
                      dbo.TaskParameters ON dbo.Runs.TaskParameterID = dbo.TaskParameters.TaskParameterID INNER JOIN
                      dbo.Tasks ON dbo.Runs.TaskID = dbo.Tasks.TaskID INNER JOIN
                      dbo.Users ON dbo.Runs.UserID = dbo.Users.UserID
'
GO
IF NOT EXISTS (SELECT * FROM ::fn_listextendedproperty(N'MS_DiagramPane1' , N'SCHEMA',N'dbo', N'VIEW',N'RunDataView', NULL,NULL))
EXEC sys.sp_addextendedproperty @name=N'MS_DiagramPane1', @value=N'[0E232FF0-B466-11cf-A24F-00AA00A3EFFF, 1.00]
Begin DesignProperties = 
   Begin PaneConfigurations = 
      Begin PaneConfiguration = 0
         NumPanes = 4
         Configuration = "(H (1[49] 4[20] 2[22] 3) )"
      End
      Begin PaneConfiguration = 1
         NumPanes = 3
         Configuration = "(H (1 [50] 4 [25] 3))"
      End
      Begin PaneConfiguration = 2
         NumPanes = 3
         Configuration = "(H (1 [50] 2 [25] 3))"
      End
      Begin PaneConfiguration = 3
         NumPanes = 3
         Configuration = "(H (4 [30] 2 [40] 3))"
      End
      Begin PaneConfiguration = 4
         NumPanes = 2
         Configuration = "(H (1 [56] 3))"
      End
      Begin PaneConfiguration = 5
         NumPanes = 2
         Configuration = "(H (2 [66] 3))"
      End
      Begin PaneConfiguration = 6
         NumPanes = 2
         Configuration = "(H (4 [50] 3))"
      End
      Begin PaneConfiguration = 7
         NumPanes = 1
         Configuration = "(V (3))"
      End
      Begin PaneConfiguration = 8
         NumPanes = 3
         Configuration = "(H (1[56] 4[18] 2) )"
      End
      Begin PaneConfiguration = 9
         NumPanes = 2
         Configuration = "(H (1 [75] 4))"
      End
      Begin PaneConfiguration = 10
         NumPanes = 2
         Configuration = "(H (1[66] 2) )"
      End
      Begin PaneConfiguration = 11
         NumPanes = 2
         Configuration = "(H (4 [60] 2))"
      End
      Begin PaneConfiguration = 12
         NumPanes = 1
         Configuration = "(H (1) )"
      End
      Begin PaneConfiguration = 13
         NumPanes = 1
         Configuration = "(V (4))"
      End
      Begin PaneConfiguration = 14
         NumPanes = 1
         Configuration = "(V (2))"
      End
      ActivePaneConfig = 0
   End
   Begin DiagramPane = 
      Begin Origin = 
         Top = 0
         Left = 0
      End
      Begin Tables = 
         Begin Table = "CmdLines"
            Begin Extent = 
               Top = 6
               Left = 38
               Bottom = 93
               Right = 198
            End
            DisplayFlags = 280
            TopColumn = 0
         End
         Begin Table = "Configs"
            Begin Extent = 
               Top = 6
               Left = 236
               Bottom = 93
               Right = 396
            End
            DisplayFlags = 280
            TopColumn = 0
         End
         Begin Table = "Games"
            Begin Extent = 
               Top = 6
               Left = 434
               Bottom = 93
               Right = 594
            End
            DisplayFlags = 280
            TopColumn = 0
         End
         Begin Table = "GameTypes"
            Begin Extent = 
               Top = 6
               Left = 632
               Bottom = 93
               Right = 792
            End
            DisplayFlags = 280
            TopColumn = 0
         End
         Begin Table = "Levels"
            Begin Extent = 
               Top = 6
               Left = 830
               Bottom = 93
               Right = 990
            End
            DisplayFlags = 280
            TopColumn = 0
         End
         Begin Table = "Locations"
            Begin Extent = 
               Top = 6
               Left = 1028
               Bottom = 123
               Right = 1188
            End
            DisplayFlags = 280
            TopColumn = 3
         End
         Begin Table = "Machines"
            Begin Extent = 
               Top = 96
               Left = 38
               Bottom = 183
               Right = 198
            End
            DisplayFlags = 280
       ' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'VIEW',@level1name=N'RunDataView'
GO
IF NOT EXISTS (SELECT * FROM ::fn_listextendedproperty(N'MS_DiagramPane2' , N'SCHEMA',N'dbo', N'VIEW',N'RunDataView', NULL,NULL))
EXEC sys.sp_addextendedproperty @name=N'MS_DiagramPane2', @value=N'     TopColumn = 0
         End
         Begin Table = "Platforms"
            Begin Extent = 
               Top = 96
               Left = 236
               Bottom = 183
               Right = 396
            End
            DisplayFlags = 280
            TopColumn = 0
         End
         Begin Table = "Results"
            Begin Extent = 
               Top = 96
               Left = 434
               Bottom = 183
               Right = 606
            End
            DisplayFlags = 280
            TopColumn = 0
         End
         Begin Table = "RunData"
            Begin Extent = 
               Top = 96
               Left = 644
               Bottom = 213
               Right = 804
            End
            DisplayFlags = 280
            TopColumn = 1
         End
         Begin Table = "Runs"
            Begin Extent = 
               Top = 238
               Left = 847
               Bottom = 467
               Right = 1019
            End
            DisplayFlags = 280
            TopColumn = 4
         End
         Begin Table = "StatGroups"
            Begin Extent = 
               Top = 126
               Left = 1052
               Bottom = 213
               Right = 1217
            End
            DisplayFlags = 280
            TopColumn = 0
         End
         Begin Table = "Stats"
            Begin Extent = 
               Top = 186
               Left = 38
               Bottom = 288
               Right = 198
            End
            DisplayFlags = 280
            TopColumn = 0
         End
         Begin Table = "Tags"
            Begin Extent = 
               Top = 354
               Left = 541
               Bottom = 441
               Right = 701
            End
            DisplayFlags = 280
            TopColumn = 0
         End
         Begin Table = "TaskParameters"
            Begin Extent = 
               Top = 211
               Left = 440
               Bottom = 298
               Right = 612
            End
            DisplayFlags = 280
            TopColumn = 0
         End
         Begin Table = "Tasks"
            Begin Extent = 
               Top = 216
               Left = 644
               Bottom = 303
               Right = 808
            End
            DisplayFlags = 280
            TopColumn = 0
         End
         Begin Table = "Users"
            Begin Extent = 
               Top = 256
               Left = 1097
               Bottom = 343
               Right = 1257
            End
            DisplayFlags = 280
            TopColumn = 0
         End
      End
   End
   Begin SQLPane = 
   End
   Begin DataPane = 
      Begin ParameterDefaults = ""
      End
   End
   Begin CriteriaPane = 
      Begin ColumnWidths = 11
         Column = 1440
         Alias = 900
         Table = 1170
         Output = 720
         Append = 1400
         NewValue = 1170
         SortType = 1350
         SortOrder = 1410
         GroupBy = 1350
         Filter = 1350
         Or = 1350
         Or = 1350
         Or = 1350
      End
   End
End
' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'VIEW',@level1name=N'RunDataView'
GO
IF NOT EXISTS (SELECT * FROM ::fn_listextendedproperty(N'MS_DiagramPaneCount' , N'SCHEMA',N'dbo', N'VIEW',N'RunDataView', NULL,NULL))
EXEC sys.sp_addextendedproperty @name=N'MS_DiagramPaneCount', @value=2 , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'VIEW',@level1name=N'RunDataView'
GO
/****** Object:  StoredProcedure [dbo].[BeginRun]    Script Date: 03/19/2008 23:39:49 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[BeginRun]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'
CREATE PROCEDURE [BeginRun]
	@PlatformName varchar(64) = ''Unknown'',
	@ConfigName varchar(64) = ''Unknown'',
	@Changelist int = -1,
	@MachineName varchar(32) = ''Unknown'',
	@UserName varchar(64) = ''Unknown'',
	@GameName varchar(32) = ''Unknown'',
	@GameType varchar(64) = ''Unknown'',
	@TaskDescription varchar(255) = ''Unknown'',
	@TaskParameter varchar(255) = ''Unknown'',
	@LevelName varchar(64) = ''Unknown'',
	@Tag varchar(64) = '''',
	@CmdLine varchar(MAX) = '''',
	@Date datetime = NULL
AS
BEGIN
	BEGIN TRANSACTION

		-- SET NOCOUNT ON added to prevent extra result sets from
		-- interfering with SELECT statements.
		SET NOCOUNT ON;

		-- Get ResultID, add if it doesn''t exist yet.
		DECLARE @ResultID int
		SET @ResultID = (SELECT ResultID FROM dbo.Results WHERE ResultDescription = ''Unknown'')
		IF (@ResultID IS NULL)
		BEGIN
			INSERT INTO dbo.Results (ResultDescription) VALUES (''Unknown'')
			SET @ResultID = @@IDENTITY
		END

		-- Get PlatformID, add if it doesn''t exist yet.
		DECLARE @PlatformID int
		SET @PlatformID = (SELECT PlatformID FROM dbo.Platforms WHERE PlatformName = @PlatformName)
		IF (@PlatformID IS NULL)
		BEGIN
			INSERT INTO dbo.Platforms (PlatformName) VALUES (@PlatformName)
			SET @PlatformID = @@IDENTITY
		END

		-- Get ConfigID, add if it doesn''t exist yet.
		DECLARE @ConfigID int
		SET @ConfigID = (SELECT ConfigID FROM dbo.Configs WHERE ConfigName = @ConfigName)
		IF (@ConfigID IS NULL)
		BEGIN
			INSERT INTO dbo.Configs (ConfigName) VALUES (@ConfigName)
			SET @ConfigID = @@IDENTITY
		END

		-- Get MachineID, add if it doesn''t exist yet.
		DECLARE @MachineID int
		SET @MachineID = (SELECT MachineID FROM dbo.Machines WHERE MachineName = @MachineName)
		IF (@MachineID IS NULL)
		BEGIN
			INSERT INTO dbo.Machines (MachineName) VALUES (@MachineName)
			SET @MachineID = @@IDENTITY
		END

		-- Get UserID, add if it doesn''t exist yet.
		DECLARE @UserID int
		SET @UserID = (SELECT UserID FROM dbo.Users WHERE UserName = @UserName)
		IF (@UserID IS NULL)
		BEGIN
			INSERT INTO dbo.Users (UserName) VALUES (@UserName)
			SET @UserID = @@IDENTITY
		END

		-- Get GameID, add if it doesn''t exist yet.
		DECLARE @GameID int
		SET @GameID = (SELECT GameID FROM dbo.Games WHERE GameName = @GameName)
		IF (@GameID IS NULL)
		BEGIN
			INSERT INTO dbo.Games (GameName) VALUES (@GameName)
			SET @GameID = @@IDENTITY
		END

		-- Get GameTypeID, add if it doesn''t exist yet.
		DECLARE @GameTypeID int
		SET @GameTypeID = (SELECT GameTypeID FROM dbo.GameTypes WHERE GameType = @GameType)
		IF (@GameTypeID IS NULL)
		BEGIN
			INSERT INTO dbo.GameTypes (GameType) VALUES (@GameType)
			SET @GameTypeID = @@IDENTITY
		END

		-- Get TaskID, add if it doesn''t exist yet.
		DECLARE @TaskID int
		SET @TaskID = (SELECT TaskID FROM dbo.Tasks WHERE TaskDescription = @TaskDescription)
		IF (@TaskID IS NULL)
		BEGIN
			INSERT INTO dbo.Tasks (TaskDescription) VALUES (@TaskDescription)
			SET @TaskID = @@IDENTITY
		END

		-- Get TaskParameterID, add if it doesn''t exist yet.
		DECLARE @TaskParameterID int
		SET @TaskParameterID = (SELECT TaskParameterID FROM dbo.TaskParameters WHERE TaskParameter = @TaskParameter)
		IF (@TaskParameterID IS NULL)
		BEGIN
			INSERT INTO dbo.TaskParameters (TaskParameter) VALUES (@TaskParameter)
			SET @TaskParameterID = @@IDENTITY
		END
		
		-- Get LevelID, add if it doesn''t exist yet.
		DECLARE @LevelID int
		SET @LevelID = (SELECT LevelID FROM dbo.Levels WHERE LevelName = @LevelName)
		IF (@LevelID IS NULL)
		BEGIN
			INSERT INTO dbo.Levels (LevelName) VALUES (@LevelName)
			SET @LevelID = @@IDENTITY
		END

		-- Get TagID, add if it doesn''t exist yet.
		DECLARE @TagID int
		SET @TagID = (SELECT TagID FROM dbo.Tags WHERE Tag = @Tag)
		IF (@TagID IS NULL)
		BEGIN
			INSERT INTO dbo.Tags (Tag) VALUES (@Tag)
			SET @TagID = @@IDENTITY
		END

		-- Get CmdLineID, add if it doesn''t exist yet.
		DECLARE @CmdLineID int
		SET @CmdLineID = (SELECT CmdLineID FROM dbo.CmdLines WHERE CmdLine = @CmdLine)
		IF (@CmdLineID IS NULL)
		BEGIN
			INSERT INTO dbo.CmdLines (CmdLine) VALUES (@CmdLine)
			SET @CmdLineID= @@IDENTITY
		END

		-- Set date if it wasn''t passed in.
		IF (@Date IS NULL)
		BEGIN
			SET @Date = GETDATE()
		END

		-- Add run to DB and return its ID.
		DECLARE @RunID int
		INSERT INTO dbo.Runs		(ResultID,PlatformID,ConfigID,Changelist,MachineID,UserID,GameID,GameTypeID,TaskID,TaskParameterID,LevelID,TagID,CmdLineID,Date)
							VALUES	(@ResultID,@PlatformID,@ConfigID,@Changelist,@MachineID,@UserID,@GameID,@GameTypeID,@TaskID,@TaskParameterID,@LevelID,@TagID,@CmdLineID,@Date)
		SET @RunID = @@IDENTITY

	COMMIT TRANSACTION
	
	SELECT @RunID AS [Return Value]
	RETURN @RunID
END















' 
END
GO
/****** Object:  StoredProcedure [dbo].[AddStat]    Script Date: 03/19/2008 23:39:48 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[AddStat]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'
CREATE PROCEDURE [AddStat]
	@StatGroupName varchar(255),
	@StatName varchar(255)

AS
BEGIN
	BEGIN TRANSACTION

		-- SET NOCOUNT ON added to prevent extra result sets from
		-- interfering with SELECT statements.
		SET NOCOUNT ON;

		-- Get StatGroupID, add if it doesn''t exist yet
		DECLARE @StatGroupID int
		SET @StatGroupID = (SELECT StatGroupID FROM dbo.StatGroups WHERE StatGroupName = @StatGroupName)
		IF (@StatGroupID IS NULL)
		BEGIN
			INSERT INTO dbo.StatGroups (StatGroupName) VALUES (@StatGroupName)
			SET @StatGroupID = @@IDENTITY
		END

		-- Get StatID, add if it doesn''t exist yet
		DECLARE @StatID int
		SET @StatID = (SELECT StatID FROM dbo.Stats WHERE StatName = @StatName)
		IF (@StatID IS NULL)
		BEGIN
			INSERT INTO dbo.Stats (StatName,StatGroupID) VALUES (@StatName,@StatGroupID)
			SET @StatID = @@IDENTITY
		END
	
	COMMIT TRANSACTION

	SELECT @StatID AS [Return Value]
	RETURN @StatID
END














' 
END
GO
/****** Object:  StoredProcedure [dbo].[EndRun]    Script Date: 03/19/2008 23:39:49 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[EndRun]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'
CREATE PROCEDURE [EndRun]
	@RunID int,
	@ResultDescription varchar(255)
AS
BEGIN
	BEGIN TRANSACTION

		-- SET NOCOUNT ON added to prevent extra result sets from
		-- interfering with SELECT statements.
		SET NOCOUNT ON;

		-- Get ResultID, add if it doesn''t exist yet
		DECLARE @ResultID int
		SET @ResultID = (SELECT ResultID FROM dbo.Results WHERE ResultDescription = @ResultDescription)
		IF (@ResultID IS NULL)
		BEGIN
			INSERT INTO dbo.Results (ResultDescription) VALUES (@ResultDescription)
			SET @ResultID = @@IDENTITY
		END

		-- Update run with result.
		UPDATE dbo.Runs
		SET ResultID = @ResultID
		WHERE RunID = @RunID

	COMMIT TRANSACTION
END













' 
END
GO
/****** Object:  StoredProcedure [dbo].[AddRunData]    Script Date: 03/19/2008 23:39:48 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[AddRunData]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'
CREATE PROCEDURE [AddRunData]
	@RunID int,
	@StatID int = NULL,
	@StatName varchar(255) = ''Unknown'',
	@StatGroupName varchar(255) = ''Unknown'',
	@StatValue float,
	@LocX int = 0,
	@LocY int = 0,
	@LocZ int = 0,
	@RotYaw int = 0,
	@RotPitch int = 0,
	@RotRoll int = 0
AS
BEGIN
	BEGIN TRANSACTION

		-- SET NOCOUNT ON added to prevent extra result sets from
		-- interfering with SELECT statements.
		SET NOCOUNT ON;

		-- Get LocationID, add if it doesn''t exist yet
		DECLARE @LocationID int
		SET @LocationID = (SELECT LocationID FROM dbo.Locations WHERE LocX = @LocX AND LocY = @LocY AND LocZ = @LocZ AND RotYaw = @RotYaw AND RotPitch = @RotPitch AND RotRoll = @RotRoll)
		IF (@LocationID IS NULL)
		BEGIN
			INSERT INTO dbo.Locations (LocX,LocY,LocZ,RotYaw,RotPitch,RotRoll) VALUES (@LocX,@LocY,@LocZ,@RotYaw,@RotPitch,@RotRoll)
			SET @LocationID = @@IDENTITY
		END

		-- If StatID is not specified, create it.	
		IF (@StatID IS NULL)
		BEGIN
			EXEC @StatID = [dbo].[AddStat] @StatName = @StatName, @StatGroupName = @StatGroupName
		END

		-- Insert run data into table.
		INSERT INTO dbo.RunData (RunID,LocationID,StatID,StatValue) VALUES (@RunID,@LocationID,@StatID,@StatValue)

	COMMIT TRANSACTION
END













' 
END
GO
/****** Object:  ForeignKey [FK_RunData_Locations1]    Script Date: 03/19/2008 23:39:59 ******/
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[FK_RunData_Locations1]') AND parent_object_id = OBJECT_ID(N'[RunData]'))
ALTER TABLE [RunData]  WITH CHECK ADD  CONSTRAINT [FK_RunData_Locations1] FOREIGN KEY([LocationID])
REFERENCES [Locations] ([LocationID])
GO
ALTER TABLE [RunData] CHECK CONSTRAINT [FK_RunData_Locations1]
GO
/****** Object:  ForeignKey [FK_RunData_Runs]    Script Date: 03/19/2008 23:40:00 ******/
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[FK_RunData_Runs]') AND parent_object_id = OBJECT_ID(N'[RunData]'))
ALTER TABLE [RunData]  WITH CHECK ADD  CONSTRAINT [FK_RunData_Runs] FOREIGN KEY([RunID])
REFERENCES [Runs] ([RunID])
GO
ALTER TABLE [RunData] CHECK CONSTRAINT [FK_RunData_Runs]
GO
/****** Object:  ForeignKey [FK_RunData_Stats1]    Script Date: 03/19/2008 23:40:00 ******/
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[FK_RunData_Stats1]') AND parent_object_id = OBJECT_ID(N'[RunData]'))
ALTER TABLE [RunData]  WITH CHECK ADD  CONSTRAINT [FK_RunData_Stats1] FOREIGN KEY([StatID])
REFERENCES [Stats] ([StatID])
GO
ALTER TABLE [RunData] CHECK CONSTRAINT [FK_RunData_Stats1]
GO
/****** Object:  ForeignKey [FK_Runs_CmdLines]    Script Date: 03/19/2008 23:40:04 ******/
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[FK_Runs_CmdLines]') AND parent_object_id = OBJECT_ID(N'[Runs]'))
ALTER TABLE [Runs]  WITH CHECK ADD  CONSTRAINT [FK_Runs_CmdLines] FOREIGN KEY([CmdLineID])
REFERENCES [CmdLines] ([CmdLineID])
GO
ALTER TABLE [Runs] CHECK CONSTRAINT [FK_Runs_CmdLines]
GO
/****** Object:  ForeignKey [FK_Runs_Configs1]    Script Date: 03/19/2008 23:40:04 ******/
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[FK_Runs_Configs1]') AND parent_object_id = OBJECT_ID(N'[Runs]'))
ALTER TABLE [Runs]  WITH CHECK ADD  CONSTRAINT [FK_Runs_Configs1] FOREIGN KEY([ConfigID])
REFERENCES [Configs] ([ConfigID])
GO
ALTER TABLE [Runs] CHECK CONSTRAINT [FK_Runs_Configs1]
GO
/****** Object:  ForeignKey [FK_Runs_Games1]    Script Date: 03/19/2008 23:40:04 ******/
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[FK_Runs_Games1]') AND parent_object_id = OBJECT_ID(N'[Runs]'))
ALTER TABLE [Runs]  WITH CHECK ADD  CONSTRAINT [FK_Runs_Games1] FOREIGN KEY([GameID])
REFERENCES [Games] ([GameID])
GO
ALTER TABLE [Runs] CHECK CONSTRAINT [FK_Runs_Games1]
GO
/****** Object:  ForeignKey [FK_Runs_GameTypes1]    Script Date: 03/19/2008 23:40:04 ******/
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[FK_Runs_GameTypes1]') AND parent_object_id = OBJECT_ID(N'[Runs]'))
ALTER TABLE [Runs]  WITH CHECK ADD  CONSTRAINT [FK_Runs_GameTypes1] FOREIGN KEY([GameTypeID])
REFERENCES [GameTypes] ([GameTypeID])
GO
ALTER TABLE [Runs] CHECK CONSTRAINT [FK_Runs_GameTypes1]
GO
/****** Object:  ForeignKey [FK_Runs_Levels1]    Script Date: 03/19/2008 23:40:04 ******/
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[FK_Runs_Levels1]') AND parent_object_id = OBJECT_ID(N'[Runs]'))
ALTER TABLE [Runs]  WITH CHECK ADD  CONSTRAINT [FK_Runs_Levels1] FOREIGN KEY([LevelID])
REFERENCES [Levels] ([LevelID])
GO
ALTER TABLE [Runs] CHECK CONSTRAINT [FK_Runs_Levels1]
GO
/****** Object:  ForeignKey [FK_Runs_Machines1]    Script Date: 03/19/2008 23:40:04 ******/
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[FK_Runs_Machines1]') AND parent_object_id = OBJECT_ID(N'[Runs]'))
ALTER TABLE [Runs]  WITH CHECK ADD  CONSTRAINT [FK_Runs_Machines1] FOREIGN KEY([MachineID])
REFERENCES [Machines] ([MachineID])
GO
ALTER TABLE [Runs] CHECK CONSTRAINT [FK_Runs_Machines1]
GO
/****** Object:  ForeignKey [FK_Runs_Platforms1]    Script Date: 03/19/2008 23:40:05 ******/
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[FK_Runs_Platforms1]') AND parent_object_id = OBJECT_ID(N'[Runs]'))
ALTER TABLE [Runs]  WITH CHECK ADD  CONSTRAINT [FK_Runs_Platforms1] FOREIGN KEY([PlatformID])
REFERENCES [Platforms] ([PlatformID])
GO
ALTER TABLE [Runs] CHECK CONSTRAINT [FK_Runs_Platforms1]
GO
/****** Object:  ForeignKey [FK_Runs_Results1]    Script Date: 03/19/2008 23:40:05 ******/
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[FK_Runs_Results1]') AND parent_object_id = OBJECT_ID(N'[Runs]'))
ALTER TABLE [Runs]  WITH CHECK ADD  CONSTRAINT [FK_Runs_Results1] FOREIGN KEY([ResultID])
REFERENCES [Results] ([ResultID])
GO
ALTER TABLE [Runs] CHECK CONSTRAINT [FK_Runs_Results1]
GO
/****** Object:  ForeignKey [FK_Runs_Tags1]    Script Date: 03/19/2008 23:40:05 ******/
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[FK_Runs_Tags1]') AND parent_object_id = OBJECT_ID(N'[Runs]'))
ALTER TABLE [Runs]  WITH CHECK ADD  CONSTRAINT [FK_Runs_Tags1] FOREIGN KEY([TagID])
REFERENCES [Tags] ([TagID])
GO
ALTER TABLE [Runs] CHECK CONSTRAINT [FK_Runs_Tags1]
GO
/****** Object:  ForeignKey [FK_Runs_TaskParameters]    Script Date: 03/19/2008 23:40:05 ******/
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[FK_Runs_TaskParameters]') AND parent_object_id = OBJECT_ID(N'[Runs]'))
ALTER TABLE [Runs]  WITH CHECK ADD  CONSTRAINT [FK_Runs_TaskParameters] FOREIGN KEY([TaskParameterID])
REFERENCES [TaskParameters] ([TaskParameterID])
GO
ALTER TABLE [Runs] CHECK CONSTRAINT [FK_Runs_TaskParameters]
GO
/****** Object:  ForeignKey [FK_Runs_Tasks1]    Script Date: 03/19/2008 23:40:05 ******/
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[FK_Runs_Tasks1]') AND parent_object_id = OBJECT_ID(N'[Runs]'))
ALTER TABLE [Runs]  WITH CHECK ADD  CONSTRAINT [FK_Runs_Tasks1] FOREIGN KEY([TaskID])
REFERENCES [Tasks] ([TaskID])
GO
ALTER TABLE [Runs] CHECK CONSTRAINT [FK_Runs_Tasks1]
GO
/****** Object:  ForeignKey [FK_Runs_Users1]    Script Date: 03/19/2008 23:40:05 ******/
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[FK_Runs_Users1]') AND parent_object_id = OBJECT_ID(N'[Runs]'))
ALTER TABLE [Runs]  WITH CHECK ADD  CONSTRAINT [FK_Runs_Users1] FOREIGN KEY([UserID])
REFERENCES [Users] ([UserID])
GO
ALTER TABLE [Runs] CHECK CONSTRAINT [FK_Runs_Users1]
GO
/****** Object:  ForeignKey [FK_Stats_StatGroups1]    Script Date: 03/19/2008 23:40:07 ******/
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[FK_Stats_StatGroups1]') AND parent_object_id = OBJECT_ID(N'[Stats]'))
ALTER TABLE [Stats]  WITH CHECK ADD  CONSTRAINT [FK_Stats_StatGroups1] FOREIGN KEY([StatGroupID])
REFERENCES [StatGroups] ([StatGroupID])
GO
ALTER TABLE [Stats] CHECK CONSTRAINT [FK_Stats_StatGroups1]
GO

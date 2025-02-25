SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Platforms]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[Platforms](
	[ID] [smallint] IDENTITY(1,1) NOT NULL,
	[Name] [varchar](16) NOT NULL,
 CONSTRAINT [PK_Platforms] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[PlatformBuildStatuses]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[PlatformBuildStatuses](
	[ID] [smallint] IDENTITY(1,1) NOT NULL,
	[Description] [varchar](64) NOT NULL,
 CONSTRAINT [PK_BuildStatus] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Paths]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[Paths](
	[ID] [bigint] IDENTITY(1,1) NOT NULL,
	[Path] [varchar](256) NOT NULL,
 CONSTRAINT [PK_Paths] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[TaskStatuses]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[TaskStatuses](
	[ID] [smallint] IDENTITY(1,1) NOT NULL,
	[Description] [varchar](16) NOT NULL,
 CONSTRAINT [PK_JobStatus] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[CachedFileInfo]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[CachedFileInfo](
	[ID] [bigint] IDENTITY(1,1) NOT NULL,
	[Size] [bigint] NOT NULL,
	[DateAndTime] [datetime] NOT NULL,
	[Hash] [varchar](48) NOT NULL,
 CONSTRAINT [PK_CachedFileInfo] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Projects]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[Projects](
	[ID] [smallint] IDENTITY(1,1) NOT NULL,
	[Title] [varchar](32) NOT NULL,
 CONSTRAINT [PK_Projects] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[ClientGroups]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[ClientGroups](
	[ID] [bigint] IDENTITY(1,1) NOT NULL,
	[GroupName] [varchar](32) NOT NULL,
 CONSTRAINT [PK_ClientGroups] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Tasks]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[Tasks](
	[ID] [bigint] IDENTITY(1,1) NOT NULL,
	[AssignedUPDS] [varchar](32) NULL,
	[ClientMachineID] [bigint] NOT NULL,
	[SubmissionTime] [datetime] NOT NULL,
	[ScheduleTime] [datetime] NOT NULL,
	[Recurring] [bit] NOT NULL CONSTRAINT [DF_Tasks_Recurring]  DEFAULT ((0)),
	[CompletionTime] [datetime] NULL,
	[RunAfterProp] [bit] NOT NULL,
	[BuildConfigID] [smallint] NOT NULL,
	[CommandLine] [varchar](1024) NOT NULL,
	[StatusID] [smallint] NOT NULL,
	[PlatformBuildID] [bigint] NOT NULL,
	[SubmittedByUser] [varchar](64) NOT NULL,
	[ErrorDescription] [varchar](1024) NULL,
	[Progress] [smallint] NOT NULL CONSTRAINT [DF_Tasks_Progress]  DEFAULT ((0)),
	[Priority] [smallint] NOT NULL CONSTRAINT [DF_Tasks_Priority]  DEFAULT ((0)),
 CONSTRAINT [PK_Tasks] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[ClientMachines]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[ClientMachines](
	[ID] [bigint] IDENTITY(1,1) NOT NULL,
	[PlatformID] [smallint] NOT NULL,
	[Path] [varchar](1024) NOT NULL,
	[Name] [varchar](32) NOT NULL,
	[ClientGroupID] [bigint] NOT NULL,
 CONSTRAINT [PK_ClientMachines] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[PlatformBuilds]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[PlatformBuilds](
	[ID] [bigint] IDENTITY(1,1) NOT NULL,
	[Special] [bit] NOT NULL CONSTRAINT [DF_PlatformBuilds_Special]  DEFAULT ((0)),
	[PlatformID] [smallint] NOT NULL,
	[ProjectID] [smallint] NOT NULL,
	[Title] [varchar](40) NOT NULL,
	[Path] [varchar](256) NOT NULL,
	[Size] [bigint] NOT NULL,
	[StatusID] [smallint] NOT NULL,
	[DiscoveryTime] [datetime] NOT NULL CONSTRAINT [DF_PlatformBuilds_Date]  DEFAULT ('2007-01-01 10:00:00'),
	[BuildTime] [datetime] NULL,
 CONSTRAINT [PK_BuildForPlatform] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[BuildConfigs]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[BuildConfigs](
	[ID] [smallint] IDENTITY(1,1) NOT NULL,
	[PlatformID] [smallint] NOT NULL,
	[BuildConfig] [varchar](32) NOT NULL,
 CONSTRAINT [PK_BuildConfigs] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[PlatformBuildFiles]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[PlatformBuildFiles](
	[ID] [bigint] IDENTITY(1,1) NOT NULL,
	[PlatformBuildID] [bigint] NOT NULL,
	[PathID] [bigint] NOT NULL CONSTRAINT [DF_PlatformBuildFiles_PathID]  DEFAULT ((0)),
	[CachedFileInfoID] [bigint] NOT NULL CONSTRAINT [DF_PlatformBuildFiles_CachedFileInfoID]  DEFAULT ((0)),
 CONSTRAINT [PK_BuildFiles] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Paths_Paths]') AND parent_object_id = OBJECT_ID(N'[dbo].[Paths]'))
ALTER TABLE [dbo].[Paths]  WITH CHECK ADD  CONSTRAINT [FK_Paths_Paths] FOREIGN KEY([ID])
REFERENCES [dbo].[Paths] ([ID])
GO
ALTER TABLE [dbo].[Paths] CHECK CONSTRAINT [FK_Paths_Paths]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Jobs_ClientMachines]') AND parent_object_id = OBJECT_ID(N'[dbo].[Tasks]'))
ALTER TABLE [dbo].[Tasks]  WITH CHECK ADD  CONSTRAINT [FK_Jobs_ClientMachines] FOREIGN KEY([ClientMachineID])
REFERENCES [dbo].[ClientMachines] ([ID])
GO
ALTER TABLE [dbo].[Tasks] CHECK CONSTRAINT [FK_Jobs_ClientMachines]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Jobs_JobStatus]') AND parent_object_id = OBJECT_ID(N'[dbo].[Tasks]'))
ALTER TABLE [dbo].[Tasks]  WITH CHECK ADD  CONSTRAINT [FK_Jobs_JobStatus] FOREIGN KEY([StatusID])
REFERENCES [dbo].[TaskStatuses] ([ID])
GO
ALTER TABLE [dbo].[Tasks] CHECK CONSTRAINT [FK_Jobs_JobStatus]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Tasks_BuildConfigs]') AND parent_object_id = OBJECT_ID(N'[dbo].[Tasks]'))
ALTER TABLE [dbo].[Tasks]  WITH CHECK ADD  CONSTRAINT [FK_Tasks_BuildConfigs] FOREIGN KEY([BuildConfigID])
REFERENCES [dbo].[BuildConfigs] ([ID])
GO
ALTER TABLE [dbo].[Tasks] CHECK CONSTRAINT [FK_Tasks_BuildConfigs]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Tasks_PlatformBuilds]') AND parent_object_id = OBJECT_ID(N'[dbo].[Tasks]'))
ALTER TABLE [dbo].[Tasks]  WITH CHECK ADD  CONSTRAINT [FK_Tasks_PlatformBuilds] FOREIGN KEY([PlatformBuildID])
REFERENCES [dbo].[PlatformBuilds] ([ID])
GO
ALTER TABLE [dbo].[Tasks] CHECK CONSTRAINT [FK_Tasks_PlatformBuilds]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_ClientMachines_ClientGroups]') AND parent_object_id = OBJECT_ID(N'[dbo].[ClientMachines]'))
ALTER TABLE [dbo].[ClientMachines]  WITH CHECK ADD  CONSTRAINT [FK_ClientMachines_ClientGroups] FOREIGN KEY([ClientGroupID])
REFERENCES [dbo].[ClientGroups] ([ID])
GO
ALTER TABLE [dbo].[ClientMachines] CHECK CONSTRAINT [FK_ClientMachines_ClientGroups]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_ClientMachines_Platforms]') AND parent_object_id = OBJECT_ID(N'[dbo].[ClientMachines]'))
ALTER TABLE [dbo].[ClientMachines]  WITH CHECK ADD  CONSTRAINT [FK_ClientMachines_Platforms] FOREIGN KEY([PlatformID])
REFERENCES [dbo].[Platforms] ([ID])
GO
ALTER TABLE [dbo].[ClientMachines] CHECK CONSTRAINT [FK_ClientMachines_Platforms]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_PlatformBuilds_PlatformBuildStatuses]') AND parent_object_id = OBJECT_ID(N'[dbo].[PlatformBuilds]'))
ALTER TABLE [dbo].[PlatformBuilds]  WITH CHECK ADD  CONSTRAINT [FK_PlatformBuilds_PlatformBuildStatuses] FOREIGN KEY([StatusID])
REFERENCES [dbo].[PlatformBuildStatuses] ([ID])
GO
ALTER TABLE [dbo].[PlatformBuilds] CHECK CONSTRAINT [FK_PlatformBuilds_PlatformBuildStatuses]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_PlatformBuilds_Platforms]') AND parent_object_id = OBJECT_ID(N'[dbo].[PlatformBuilds]'))
ALTER TABLE [dbo].[PlatformBuilds]  WITH CHECK ADD  CONSTRAINT [FK_PlatformBuilds_Platforms] FOREIGN KEY([PlatformID])
REFERENCES [dbo].[Platforms] ([ID])
GO
ALTER TABLE [dbo].[PlatformBuilds] CHECK CONSTRAINT [FK_PlatformBuilds_Platforms]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_PlatformBuilds_Projects]') AND parent_object_id = OBJECT_ID(N'[dbo].[PlatformBuilds]'))
ALTER TABLE [dbo].[PlatformBuilds]  WITH CHECK ADD  CONSTRAINT [FK_PlatformBuilds_Projects] FOREIGN KEY([ProjectID])
REFERENCES [dbo].[Projects] ([ID])
GO
ALTER TABLE [dbo].[PlatformBuilds] CHECK CONSTRAINT [FK_PlatformBuilds_Projects]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_BuildConfigs_Platforms]') AND parent_object_id = OBJECT_ID(N'[dbo].[BuildConfigs]'))
ALTER TABLE [dbo].[BuildConfigs]  WITH CHECK ADD  CONSTRAINT [FK_BuildConfigs_Platforms] FOREIGN KEY([PlatformID])
REFERENCES [dbo].[Platforms] ([ID])
GO
ALTER TABLE [dbo].[BuildConfigs] CHECK CONSTRAINT [FK_BuildConfigs_Platforms]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_PlatformBuildFiles_CachedFileInfo]') AND parent_object_id = OBJECT_ID(N'[dbo].[PlatformBuildFiles]'))
ALTER TABLE [dbo].[PlatformBuildFiles]  WITH CHECK ADD  CONSTRAINT [FK_PlatformBuildFiles_CachedFileInfo] FOREIGN KEY([CachedFileInfoID])
REFERENCES [dbo].[CachedFileInfo] ([ID])
GO
ALTER TABLE [dbo].[PlatformBuildFiles] CHECK CONSTRAINT [FK_PlatformBuildFiles_CachedFileInfo]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_PlatformBuildFiles_Paths]') AND parent_object_id = OBJECT_ID(N'[dbo].[PlatformBuildFiles]'))
ALTER TABLE [dbo].[PlatformBuildFiles]  WITH CHECK ADD  CONSTRAINT [FK_PlatformBuildFiles_Paths] FOREIGN KEY([PathID])
REFERENCES [dbo].[Paths] ([ID])
GO
ALTER TABLE [dbo].[PlatformBuildFiles] CHECK CONSTRAINT [FK_PlatformBuildFiles_Paths]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_PlatformBuildFiles_PlatformBuilds]') AND parent_object_id = OBJECT_ID(N'[dbo].[PlatformBuildFiles]'))
ALTER TABLE [dbo].[PlatformBuildFiles]  WITH CHECK ADD  CONSTRAINT [FK_PlatformBuildFiles_PlatformBuilds] FOREIGN KEY([PlatformBuildID])
REFERENCES [dbo].[PlatformBuilds] ([ID])
GO
ALTER TABLE [dbo].[PlatformBuildFiles] CHECK CONSTRAINT [FK_PlatformBuildFiles_PlatformBuilds]

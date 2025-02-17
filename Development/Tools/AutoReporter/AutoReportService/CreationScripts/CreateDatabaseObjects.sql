SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[ReportData]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[ReportData](
	[ID] [int] IDENTITY(1,1) NOT NULL,
	[ComputerName] [varchar](50) NOT NULL,
	[UserName] [varchar](50) NOT NULL,
	[GameName] [varchar](50) NOT NULL,
	[PlatformName] [varchar](50) NOT NULL,
	[LanguageExt] [varchar](50) NOT NULL,
	[TimeOfCrash] [datetime] NOT NULL,
	[BuildVer] [varchar](50) NOT NULL,
	[ChangelistVer] [varchar](50) NOT NULL,
	[CommandLine] [varchar](512) NULL,
	[BaseDir] [varchar](260) NOT NULL,
	[EngineMode] [varchar](50) NULL,
	[TTP] [varchar](50) NULL,
	[Status] [varchar](50) NULL,
	[Summary] [varchar](256) NULL,
	[CrashDescription] [varchar](1024) NULL,
	[CallStack] [varchar](3000) NULL,
	[Selected] [bit] NULL CONSTRAINT [DF_ReportData_Selected]  DEFAULT ((0)),
	[FixedChangelist] [varchar](50) NULL,
 CONSTRAINT [PK_ReportData] PRIMARY KEY CLUSTERED 
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
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[CoderUsers]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[CoderUsers](
	[UserName] [varchar](50) NOT NULL,
 CONSTRAINT [PK_CoderUsers] PRIMARY KEY CLUSTERED 
(
	[UserName] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END

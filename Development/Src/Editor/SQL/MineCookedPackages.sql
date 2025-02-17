/****** Object:  Table [dbo].[PackageDependencies]    Script Date: 03/07/2008 13:07:29 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[PackageDependencies]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[PackageDependencies](
	[ID] [int] IDENTITY(1,1) NOT NULL,
	[PackageID] [int] NOT NULL,
	[DependencyID] [int] NOT NULL,
 CONSTRAINT [PK_Sublevels] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO
/****** Object:  Table [dbo].[Packages]    Script Date: 03/07/2008 13:07:29 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Packages]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[Packages](
	[PackageID] [int] IDENTITY(1,1) NOT NULL,
	[PackageName] [char](64) NOT NULL,
	[bIsMapPackage] [bit] NOT NULL,
	[UncompressedSize] [int] NOT NULL,
	[CompressedSize] [int] NOT NULL,
	[NameCount] [int] NOT NULL,
	[ExportCount] [int] NOT NULL,
	[ImportCount] [int] NOT NULL,
	[NameTableSize] [int] NOT NULL,
	[ExportTableSize] [int] NOT NULL,
	[ImportTableSize] [int] NOT NULL,
	[TotalHeaderSize] [int] NOT NULL,
 CONSTRAINT [PK_Packages] PRIMARY KEY CLUSTERED 
(
	[PackageID] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO

/****** Object:  Index [IX_Packages]    Script Date: 03/07/2008 13:07:29 ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[Packages]') AND name = N'IX_Packages')
CREATE NONCLUSTERED INDEX [IX_Packages] ON [dbo].[Packages] 
(
	[PackageName] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[Classes]    Script Date: 03/07/2008 13:07:29 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Classes]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[Classes](
	[ClassID] [int] IDENTITY(1,1) NOT NULL,
	[ClassName] [char](64) NOT NULL,
	[SuperID] [int] NOT NULL,
	[SuperDepth] [smallint] NOT NULL,
 CONSTRAINT [PK_Classes] PRIMARY KEY CLUSTERED 
(
	[ClassID] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO

/****** Object:  Index [IX_Classes]    Script Date: 03/07/2008 13:07:29 ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[Classes]') AND name = N'IX_Classes')
CREATE NONCLUSTERED INDEX [IX_Classes] ON [dbo].[Classes] 
(
	[ClassName] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[Exports]    Script Date: 03/07/2008 13:07:29 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Exports]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[Exports](
	[ExportID] [int] IDENTITY(1,1) NOT NULL,
	[PackageID] [int] NOT NULL,
	[ObjectID] [int] NOT NULL,
	[Size] [int] NOT NULL,
 CONSTRAINT [PK_Exports] PRIMARY KEY CLUSTERED 
(
	[ExportID] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO
/****** Object:  Table [dbo].[Objects]    Script Date: 03/07/2008 13:07:29 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Objects]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[Objects](
	[ObjectID] [int] IDENTITY(1,1) NOT NULL,
	[ObjectName] [char](255) NOT NULL,
	[ClassID] [int] NOT NULL,
 CONSTRAINT [PK_Objects] PRIMARY KEY CLUSTERED 
(
	[ObjectID] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
END
GO

/****** Object:  Index [IX_Objects]    Script Date: 03/07/2008 13:07:29 ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[Objects]') AND name = N'IX_Objects')
CREATE NONCLUSTERED INDEX [IX_Objects] ON [dbo].[Objects] 
(
	[ObjectName] ASC
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]
GO
/****** Object:  StoredProcedure [dbo].[AddExport]    Script Date: 03/07/2008 13:07:29 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[AddExport]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'
CREATE PROCEDURE [dbo].[AddExport] 
	@PackageName char(64),
	@ClassName char(64), 
	@ObjectName char(255),	
	@Size int
AS
BEGIN
	-- SET NOCOUNT ON added to prevent extra result sets from
	-- interfering with SELECT statements.
	SET NOCOUNT ON;

	-- Finds unique ID for package. At this point it is guaranteed to exist.
	DECLARE @PackageID int	
	SET @PackageID = (SELECT PackageID FROM dbo.Packages WHERE PackageName = @PackageName)
	
	-- Finds unique ID for object and creates one if not found.
	DECLARE @ObjectID int	
	SET @ObjectID = (SELECT ObjectID FROM dbo.Objects WHERE ObjectName = @ObjectName)
	IF (@ObjectID IS NULL)
	BEGIN
		-- Finds unique ID for class. At this point it is guaranteed to exist.
		DECLARE @ClassID int	
		SET @ClassID = (SELECT ClassID FROM dbo.Classes WHERE ClassName = @ClassName AND SuperDepth = 0)
		-- Insert objectname and class into objects table.
		INSERT INTO dbo.Objects (ObjectName,ClassID) VALUES (@ObjectName,@ClassID)
		SET @ObjectID = @@IDENTITY
	END

	-- Finally insert export into table.
	INSERT INTO dbo.Exports (PackageID,ObjectID,Size) VALUES (@PackageID,@ObjectID,@Size)	

	RETURN 0
END








' 
END
GO
/****** Object:  StoredProcedure [dbo].[AddPackageDependency]    Script Date: 03/07/2008 13:07:29 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[AddPackageDependency]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'
CREATE PROCEDURE [dbo].[AddPackageDependency] 
	@PackageName char(64),
	@DependencyName char(64)
AS
BEGIN
	-- SET NOCOUNT ON added to prevent extra result sets from
	-- interfering with SELECT statements.
	SET NOCOUNT ON;

	-- We rely on AddPackage to have been called on both master and sub level before adding sublevels.
	
	DECLARE @PackageID int	
	SET @PackageID = (SELECT PackageID FROM dbo.Packages WHERE PackageName = @PackageName)
	
	DECLARE @DependencyID int	
	SET @DependencyID = (SELECT PackageID FROM dbo.Packages WHERE PackageName = @DependencyName)

	-- Add without avoiding duplicates as we don''t avoid duplicates for exports either.
	INSERT INTO dbo.PackageDependencies (PackageID,DependencyID) VALUES (@PackageID,@DependencyID)

	RETURN 0
END










' 
END
GO
/****** Object:  StoredProcedure [dbo].[AddPackage]    Script Date: 03/07/2008 13:07:29 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[AddPackage]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'
CREATE PROCEDURE [dbo].[AddPackage] 
	@PackageName char(64),
    @bIsMapPackage bit,
	@UncompressedSize int,
	@CompressedSize int,
	@TotalHeaderSize int,
	@NameTableSize int,
	@ImportTableSize int,
	@ExportTableSize int,
	@NameCount int,
	@ImportCount int,
	@ExportCount int
AS
BEGIN
	-- SET NOCOUNT ON added to prevent extra result sets from
	-- interfering with SELECT statements.
	SET NOCOUNT ON;

	-- Adds package if it doesn''t already exist.
	DECLARE @PackageID int	
	SET @PackageID = (SELECT PackageID FROM dbo.Packages WHERE PackageName = @PackageName)
	IF (@PackageID IS NULL)
	BEGIN
		INSERT INTO dbo.Packages (PackageName,bIsMapPackage,UncompressedSize,CompressedSize,NameCount,ImportCount,ExportCount,NameTableSize,ExportTableSize,ImportTableSize,TotalHeaderSize) VALUES (@PackageName,@bIsMapPackage,@UncompressedSize,@CompressedSize,@NameCount,@ImportCount,@ExportCount,@NameTableSize,@ExportTableSize,@ImportTableSize,@TotalHeaderSize)
		SET @PackageID = @@IDENTITY
	END

	RETURN 0
END








' 
END
GO
/****** Object:  StoredProcedure [dbo].[AddClass]    Script Date: 03/07/2008 13:07:29 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[AddClass]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'
CREATE PROCEDURE [dbo].[AddClass] 
	@ClassName char(64),
	@SuperName char(64), 
	@SuperDepth int
AS
BEGIN
	-- SET NOCOUNT ON added to prevent extra result sets from
	-- interfering with SELECT statements.
	SET NOCOUNT ON;

	-- We rely on AddClass to be called on base class first so super will already have an id... unless we''re very very first add.
	DECLARE @SuperID int	
	SET @SuperID = (SELECT ClassID FROM dbo.Classes WHERE ClassName = @SuperName AND SuperDepth = 0)
	IF (@SuperID IS NULL)
	BEGIN
		INSERT INTO dbo.Classes (ClassName,SuperID,SuperDepth) VALUES (@SuperName,0,0)
		SET @SuperID = @@IDENTITY
	END

	-- Add unique ClassName/ SuperDepth pair if one doesn''t already exist
	DECLARE @ClassID int	
	SET @ClassID = (SELECT ClassID FROM dbo.Classes WHERE ClassName = @ClassName AND SuperDepth = @SuperDepth)
	IF (@ClassID IS NULL)
	BEGIN
		INSERT INTO dbo.Classes (ClassName,SuperID,SuperDepth) VALUES (@ClassName,@SuperID,@SuperDepth)
	END

	RETURN 0
END








' 
END
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Exports_Objects]') AND parent_object_id = OBJECT_ID(N'[dbo].[Exports]'))
ALTER TABLE [dbo].[Exports]  WITH CHECK ADD  CONSTRAINT [FK_Exports_Objects] FOREIGN KEY([ObjectID])
REFERENCES [dbo].[Objects] ([ObjectID])
GO
ALTER TABLE [dbo].[Exports] CHECK CONSTRAINT [FK_Exports_Objects]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Exports_Packages]') AND parent_object_id = OBJECT_ID(N'[dbo].[Exports]'))
ALTER TABLE [dbo].[Exports]  WITH CHECK ADD  CONSTRAINT [FK_Exports_Packages] FOREIGN KEY([PackageID])
REFERENCES [dbo].[Packages] ([PackageID])
GO
ALTER TABLE [dbo].[Exports] CHECK CONSTRAINT [FK_Exports_Packages]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Objects_Classes]') AND parent_object_id = OBJECT_ID(N'[dbo].[Objects]'))
ALTER TABLE [dbo].[Objects]  WITH CHECK ADD  CONSTRAINT [FK_Objects_Classes] FOREIGN KEY([ClassID])
REFERENCES [dbo].[Classes] ([ClassID])
GO
ALTER TABLE [dbo].[Objects] CHECK CONSTRAINT [FK_Objects_Classes]

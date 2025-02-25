IF NOT EXISTS (SELECT * FROM sys.types st JOIN sys.schemas ss ON st.schema_id = ss.schema_id WHERE st.name = N'KeyID' AND ss.name = N'dbo')਍䌀刀䔀䄀吀䔀 吀夀倀䔀 嬀搀戀漀崀⸀嬀䬀攀礀䤀䐀崀 䘀刀伀䴀 嬀椀渀琀崀 一伀吀 一唀䰀䰀ഀഀ
GO਍䤀䘀 一伀吀 䔀堀䤀匀吀匀 ⠀匀䔀䰀䔀䌀吀 ⨀ 䘀刀伀䴀 猀礀猀⸀琀礀瀀攀猀 猀琀 䨀伀䤀一 猀礀猀⸀猀挀栀攀洀愀猀 猀猀 伀一 猀琀⸀猀挀栀攀洀愀开椀搀 㴀 猀猀⸀猀挀栀攀洀愀开椀搀 圀䠀䔀刀䔀 猀琀⸀渀愀洀攀 㴀 一✀䘀椀砀攀搀匀琀爀椀渀最✀ 䄀一䐀 猀猀⸀渀愀洀攀 㴀 一✀搀戀漀✀⤀ഀഀ
CREATE TYPE [dbo].[FixedString] FROM [char](64) NOT NULL਍䜀伀ഀഀ
IF NOT EXISTS (SELECT * FROM sys.types st JOIN sys.schemas ss ON st.schema_id = ss.schema_id WHERE st.name = N'String' AND ss.name = N'dbo')਍䌀刀䔀䄀吀䔀 吀夀倀䔀 嬀搀戀漀崀⸀嬀匀琀爀椀渀最崀 䘀刀伀䴀 嬀渀瘀愀爀挀栀愀爀崀⠀㐀　　　⤀ 一伀吀 一唀䰀䰀ഀഀ
GO਍匀䔀吀 䄀一匀䤀开一唀䰀䰀匀 伀一ഀഀ
GO਍匀䔀吀 儀唀伀吀䔀䐀开䤀䐀䔀一吀䤀䘀䤀䔀刀 伀一ഀഀ
GO਍䤀䘀 一伀吀 䔀堀䤀匀吀匀 ⠀匀䔀䰀䔀䌀吀 ⨀ 䘀刀伀䴀 猀礀猀⸀漀戀樀攀挀琀猀 圀䠀䔀刀䔀 漀戀樀攀挀琀开椀搀 㴀 伀䈀䨀䔀䌀吀开䤀䐀⠀一✀嬀搀戀漀崀⸀嬀匀攀琀䌀漀渀挀栀吀椀洀攀崀✀⤀ 䄀一䐀 琀礀瀀攀 椀渀 ⠀一✀倀✀Ⰰ 一✀倀䌀✀⤀⤀ഀഀ
BEGIN਍䔀堀䔀䌀 搀戀漀⸀猀瀀开攀砀攀挀甀琀攀猀焀氀 䀀猀琀愀琀攀洀攀渀琀 㴀 一✀  䌀刀䔀䄀吀䔀 倀刀伀䌀䔀䐀唀刀䔀 嬀搀戀漀崀⸀嬀匀攀琀䌀漀渀挀栀吀椀洀攀崀ഀഀ
਍ऀ䀀䌀漀洀洀愀渀搀䤀䐀 䤀一吀ഀഀ
	਍䄀匀ഀഀ
਍ऀ䈀䔀䜀䤀一 吀刀䄀一ഀഀ
	DECLARE @Count INT਍ऀ匀䔀䰀䔀䌀吀 䀀䌀漀甀渀琀 㴀 䌀伀唀一吀⠀ 䤀䐀 ⤀ 䘀刀伀䴀 䌀漀洀洀愀渀搀猀 圀䠀䔀刀䔀 ⠀ 䌀漀渀挀栀䠀漀氀搀攀爀 椀猀 渀漀琀 一唀䰀䰀 ⤀ഀഀ
	if @Count = 0 UPDATE Commands SET ConchHolder = GETDATE() WHERE ( ID = @CommandID )਍ऀ匀䔀䰀䔀䌀吀 䀀䌀漀甀渀琀ഀഀ
	COMMIT਍ऀഀഀ
RETURN਍✀ ഀഀ
END਍䜀伀ഀഀ
SET ANSI_NULLS ON਍䜀伀ഀഀ
SET QUOTED_IDENTIFIER ON਍䜀伀ഀഀ
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Builders]') AND type in (N'U'))਍䈀䔀䜀䤀一ഀഀ
CREATE TABLE [dbo].[Builders](਍ऀ嬀䤀䐀崀 嬀椀渀琀崀 䤀䐀䔀一吀䤀吀夀⠀㄀Ⰰ㄀⤀ 一伀吀 一唀䰀䰀Ⰰഀഀ
	[Machine] [varchar](32) NOT NULL,਍ऀ嬀匀琀愀琀攀崀 嬀瘀愀爀挀栀愀爀崀⠀㄀㘀⤀ 一伀吀 一唀䰀䰀Ⰰഀഀ
	[StartTime] [datetime] NOT NULL,਍ऀ嬀䌀甀爀爀攀渀琀吀椀洀攀崀 嬀搀愀琀攀琀椀洀攀崀 一唀䰀䰀Ⰰഀഀ
	[EndTime] [datetime] NULL,਍ऀ嬀伀匀㘀㐀䈀椀琀崀 嬀戀椀琀崀 一唀䰀䰀 䌀伀一匀吀刀䄀䤀一吀 嬀䐀䘀开䈀甀椀氀搀攀爀猀开䤀猀䘀愀猀琀崀  䐀䔀䘀䄀唀䰀吀 ⠀⠀　⤀⤀Ⰰഀഀ
	[Restart] [bit] NULL਍⤀ 伀一 嬀倀刀䤀䴀䄀刀夀崀ഀഀ
END਍䜀伀ഀഀ
SET ANSI_NULLS ON਍䜀伀ഀഀ
SET QUOTED_IDENTIFIER ON਍䜀伀ഀഀ
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Jobs]') AND type in (N'U'))਍䈀䔀䜀䤀一ഀഀ
CREATE TABLE [dbo].[Jobs](਍ऀ嬀䤀䐀崀 嬀椀渀琀崀 䤀䐀䔀一吀䤀吀夀⠀㄀Ⰰ㄀⤀ 一伀吀 一唀䰀䰀Ⰰഀഀ
	[Name] [varchar](64) NULL,਍ऀ嬀䌀漀洀洀愀渀搀崀 嬀瘀愀爀挀栀愀爀崀⠀㌀㈀⤀ 一唀䰀䰀Ⰰഀഀ
	[Platform] [varchar](16) NULL,਍ऀ嬀䜀愀洀攀崀 嬀瘀愀爀挀栀愀爀崀⠀㄀㘀⤀ 一唀䰀䰀Ⰰഀഀ
	[Parameter] [varchar](1024) NULL,਍ऀ嬀䈀爀愀渀挀栀崀 嬀瘀愀爀挀栀愀爀崀⠀㌀㈀⤀ 一唀䰀䰀Ⰰഀഀ
	[Label] [varchar](96) NULL,਍ऀ嬀䤀渀猀琀椀最愀琀漀爀崀 嬀瘀愀爀挀栀愀爀崀⠀㄀㘀⤀ 一唀䰀䰀Ⰰഀഀ
	[Machine] [varchar](16) NULL,਍ऀ嬀䈀甀椀氀搀䰀漀最䤀䐀崀 嬀椀渀琀崀 一唀䰀䰀Ⰰഀഀ
	[Compatible64Bit] [bit] NOT NULL CONSTRAINT [DF_Jobs_Compatible64Bit]  DEFAULT ((0)),਍ऀ嬀䄀挀琀椀瘀攀崀 嬀戀椀琀崀 一伀吀 一唀䰀䰀 䌀伀一匀吀刀䄀䤀一吀 嬀䐀䘀开䨀漀戀猀开䄀挀琀椀瘀攀崀  䐀䔀䘀䄀唀䰀吀 ⠀⠀　⤀⤀Ⰰഀഀ
	[Complete] [bit] NOT NULL CONSTRAINT [DF_Jobs_Complete]  DEFAULT ((0)),਍ऀ嬀匀甀挀挀攀攀搀攀搀崀 嬀戀椀琀崀 一伀吀 一唀䰀䰀 䌀伀一匀吀刀䄀䤀一吀 嬀䐀䘀开䨀漀戀猀开匀甀挀挀攀攀搀攀搀崀  䐀䔀䘀䄀唀䰀吀 ⠀⠀　⤀⤀Ⰰഀഀ
	[Copied] [bit] NULL CONSTRAINT [DF_Jobs_Copied]  DEFAULT ((0)),਍ऀ嬀䬀椀氀氀椀渀最崀 嬀戀椀琀崀 一唀䰀䰀Ⰰഀഀ
	[SpawnTime] [bigint] NULL਍⤀ 伀一 嬀倀刀䤀䴀䄀刀夀崀ഀഀ
END਍䜀伀ഀഀ
SET ANSI_NULLS ON਍䜀伀ഀഀ
SET QUOTED_IDENTIFIER ON਍䜀伀ഀഀ
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Commands]') AND type in (N'U'))਍䈀䔀䜀䤀一ഀഀ
CREATE TABLE [dbo].[Commands](਍ऀ嬀䤀䐀崀 嬀椀渀琀崀 䤀䐀䔀一吀䤀吀夀⠀㄀Ⰰ㄀⤀ 一伀吀 一唀䰀䰀Ⰰഀഀ
	[Description] [varchar](40) NOT NULL,਍ऀ嬀䌀漀洀洀愀渀搀崀 嬀瘀愀爀挀栀愀爀崀⠀㌀㈀⤀ 一伀吀 一唀䰀䰀Ⰰഀഀ
	[Parameter] [varchar](64) NOT NULL,਍ऀ嬀伀瀀攀爀愀琀漀爀崀 嬀瘀愀爀挀栀愀爀崀⠀㌀㈀⤀ 一伀吀 一唀䰀䰀Ⰰഀഀ
	[Killer] [varchar](32) NOT NULL,਍ऀ嬀䈀爀愀渀挀栀崀 嬀瘀愀爀挀栀愀爀崀⠀㌀㈀⤀ 一伀吀 一唀䰀䰀Ⰰഀഀ
	[Pending] [bit] NULL,਍ऀ嬀䬀椀氀氀椀渀最崀 嬀戀椀琀崀 一唀䰀䰀Ⰰഀഀ
	[Looping] [bit] NOT NULL,਍ऀ嬀倀爀漀洀漀琀愀戀氀攀崀 嬀椀渀琀崀 一唀䰀䰀Ⰰഀഀ
	[Access] [int] NULL,਍ऀ嬀䐀椀猀瀀氀愀礀崀 嬀椀渀琀崀 一唀䰀䰀Ⰰഀഀ
	[DisplayDetail] [int] NULL,਍ऀ嬀䌀漀洀瀀愀琀椀戀氀攀㘀㐀䈀椀琀崀 嬀戀椀琀崀 一伀吀 一唀䰀䰀 䌀伀一匀吀刀䄀䤀一吀 嬀䐀䘀开䌀漀洀洀愀渀搀猀开䄀氀氀漀眀㘀㐀䈀椀琀崀  䐀䔀䘀䄀唀䰀吀 ⠀⠀　⤀⤀Ⰰഀഀ
	[PrimaryBuild] [bit] NOT NULL CONSTRAINT [DF_Commands_PrimaryBuild]  DEFAULT ((0)),਍ऀ嬀䴀愀挀栀椀渀攀崀 嬀瘀愀爀挀栀愀爀崀⠀㄀㘀⤀ 一唀䰀䰀Ⰰഀഀ
	[MachineLock] [varchar](16) NULL,਍ऀ嬀䈀甀椀氀搀䰀漀最䤀䐀崀 嬀椀渀琀崀 一唀䰀䰀Ⰰഀഀ
	[LastGoodChangeList] [int] NULL,਍ऀ嬀䰀愀猀琀䜀漀漀搀䰀愀戀攀氀崀 嬀瘀愀爀挀栀愀爀崀⠀㤀㘀⤀ 一唀䰀䰀Ⰰഀഀ
	[LastGoodDateTime] [datetime] NULL,਍ऀ嬀一攀砀琀吀爀椀最最攀爀䐀攀氀愀礀崀 嬀椀渀琀崀 一唀䰀䰀Ⰰഀഀ
	[NextTrigger] [datetime] NULL,਍ऀ嬀䌀漀渀挀栀䠀漀氀搀攀爀崀 嬀搀愀琀攀琀椀洀攀崀 一唀䰀䰀Ⰰഀഀ
 CONSTRAINT [PK_Commands] PRIMARY KEY CLUSTERED ਍⠀ഀഀ
	[ID] ASC਍⤀圀䤀吀䠀 ⠀倀䄀䐀开䤀一䐀䔀堀  㴀 伀䘀䘀Ⰰ 䤀䜀一伀刀䔀开䐀唀倀开䬀䔀夀 㴀 伀䘀䘀⤀ 伀一 嬀倀刀䤀䴀䄀刀夀崀ഀഀ
) ON [PRIMARY]਍䔀一䐀ഀഀ
GO਍匀䔀吀 䄀一匀䤀开一唀䰀䰀匀 伀一ഀഀ
GO਍匀䔀吀 儀唀伀吀䔀䐀开䤀䐀䔀一吀䤀䘀䤀䔀刀 伀一ഀഀ
GO਍䤀䘀 一伀吀 䔀堀䤀匀吀匀 ⠀匀䔀䰀䔀䌀吀 ⨀ 䘀刀伀䴀 猀礀猀⸀漀戀樀攀挀琀猀 圀䠀䔀刀䔀 漀戀樀攀挀琀开椀搀 㴀 伀䈀䨀䔀䌀吀开䤀䐀⠀一✀嬀搀戀漀崀⸀嬀䈀爀愀渀挀栀攀猀崀✀⤀ 䄀一䐀 琀礀瀀攀 椀渀 ⠀一✀唀✀⤀⤀ഀഀ
BEGIN਍䌀刀䔀䄀吀䔀 吀䄀䈀䰀䔀 嬀搀戀漀崀⸀嬀䈀爀愀渀挀栀攀猀崀⠀ഀഀ
	[BuilderID] [int] NULL,਍ऀ嬀䈀爀愀渀挀栀崀 嬀瘀愀爀挀栀愀爀崀⠀㌀㈀⤀ 一唀䰀䰀ഀഀ
) ON [PRIMARY]਍䔀一䐀ഀഀ
GO਍匀䔀吀 䄀一匀䤀开一唀䰀䰀匀 伀一ഀഀ
GO਍匀䔀吀 儀唀伀吀䔀䐀开䤀䐀䔀一吀䤀䘀䤀䔀刀 伀一ഀഀ
GO਍䤀䘀 一伀吀 䔀堀䤀匀吀匀 ⠀匀䔀䰀䔀䌀吀 ⨀ 䘀刀伀䴀 猀礀猀⸀漀戀樀攀挀琀猀 圀䠀䔀刀䔀 漀戀樀攀挀琀开椀搀 㴀 伀䈀䨀䔀䌀吀开䤀䐀⠀一✀嬀搀戀漀崀⸀嬀匀攀氀攀挀琀䄀挀琀椀瘀攀䈀甀椀氀搀攀爀猀崀✀⤀ 䄀一䐀 琀礀瀀攀 椀渀 ⠀一✀倀✀Ⰰ 一✀倀䌀✀⤀⤀ഀഀ
BEGIN਍䔀堀䔀䌀 搀戀漀⸀猀瀀开攀砀攀挀甀琀攀猀焀氀 䀀猀琀愀琀攀洀攀渀琀 㴀 一✀䌀刀䔀䄀吀䔀 倀刀伀䌀䔀䐀唀刀䔀 嬀搀戀漀崀⸀嬀匀攀氀攀挀琀䄀挀琀椀瘀攀䈀甀椀氀搀攀爀猀崀 ഀഀ
਍䄀匀ഀഀ
਍ऀ匀䔀䰀䔀䌀吀 䌀甀爀爀攀渀琀吀椀洀攀Ⰰ 䴀愀挀栀椀渀攀 䘀刀伀䴀 嬀䈀甀椀氀搀攀爀猀崀 圀䠀䔀刀䔀 ഀഀ
		( State = ''Connected'' ) ORDER BY Machine਍ഀഀ
RETURN਍✀ ഀഀ
END਍䜀伀ഀഀ
SET ANSI_NULLS ON਍䜀伀ഀഀ
SET QUOTED_IDENTIFIER ON਍䜀伀ഀഀ
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[SelectBuilds]') AND type in (N'P', N'PC'))਍䈀䔀䜀䤀一ഀഀ
EXEC dbo.sp_executesql @statement = N'  CREATE PROCEDURE [dbo].[SelectBuilds] ਍ഀഀ
	@DisplayID int,਍ऀ䀀䐀椀猀瀀氀愀礀䐀攀琀愀椀氀䤀䐀 椀渀琀ഀഀ
਍䄀匀ഀഀ
਍ऀ匀䔀䰀䔀䌀吀 䐀攀猀挀爀椀瀀琀椀漀渀Ⰰ 䰀愀猀琀䜀漀漀搀䌀栀愀渀最攀䰀椀猀琀Ⰰ 䰀愀猀琀䜀漀漀搀䐀愀琀攀吀椀洀攀Ⰰ ഀഀ
		CASE Pending WHEN 0 THEN '''' WHEN 1 THEN ''(Build Pending)'' END AS Status,਍ऀऀ䌀䄀匀䔀 圀䠀䔀一 䰀愀猀琀䜀漀漀搀䰀愀戀攀氀 椀猀 渀甀氀氀 吀䠀䔀一 ✀✀✀✀ 圀䠀䔀一 䰀愀猀琀䜀漀漀搀䰀愀戀攀氀 椀猀 渀漀琀 渀甀氀氀 吀䠀䔀一 ✀✀笀✀✀ ⬀ 䰀愀猀琀䜀漀漀搀䰀愀戀攀氀 ⬀ ✀✀紀✀✀ 䔀一䐀 䄀匀 䐀椀猀瀀氀愀礀䰀愀戀攀氀 ഀഀ
		FROM [Commands] WHERE ਍ऀऀऀ⠀ 䰀愀猀琀䜀漀漀搀䐀愀琀攀吀椀洀攀 椀猀 渀漀琀 渀甀氀氀 ⤀ഀഀ
			AND ( ( @DisplayID != 0 AND ( Display = @DisplayID ) )਍ऀऀऀ伀刀 ⠀ 䀀䐀椀猀瀀氀愀礀䐀攀琀愀椀氀䤀䐀 ℀㴀 　 䄀一䐀 ⠀ 䐀椀猀瀀氀愀礀䐀攀琀愀椀氀 㴀 䀀䐀椀猀瀀氀愀礀䐀攀琀愀椀氀䤀䐀 ⤀ ⤀ ⤀ഀഀ
				ORDER BY LastGoodChangeList DESC, LastGoodDateTime DESC਍ഀഀ
RETURN਍✀ ഀഀ
END਍䜀伀ഀഀ
SET ANSI_NULLS ON਍䜀伀ഀഀ
SET QUOTED_IDENTIFIER ON਍䜀伀ഀഀ
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Variables]') AND type in (N'U'))਍䈀䔀䜀䤀一ഀഀ
CREATE TABLE [dbo].[Variables](਍ऀ嬀䤀䐀崀 嬀椀渀琀崀 䤀䐀䔀一吀䤀吀夀⠀㄀Ⰰ㄀⤀ 一伀吀 一唀䰀䰀Ⰰഀഀ
	[Variable] [varchar](32) NULL,਍ऀ嬀䈀爀愀渀挀栀崀 嬀瘀愀爀挀栀愀爀崀⠀㌀㈀⤀ 一唀䰀䰀Ⰰഀഀ
	[Value] [varchar](max) NULL਍⤀ 伀一 嬀倀刀䤀䴀䄀刀夀崀ഀഀ
END਍䜀伀ഀഀ
SET ANSI_NULLS ON਍䜀伀ഀഀ
SET QUOTED_IDENTIFIER ON਍䜀伀ഀഀ
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[GetTriggerable]') AND type in (N'P', N'PC'))਍䈀䔀䜀䤀一ഀഀ
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[GetTriggerable]਍ഀഀ
	@MinAccess int,਍ऀ䀀䴀愀砀䄀挀挀攀猀猀 椀渀琀ഀഀ
਍䄀匀ഀഀ
਍ऀ匀䔀䰀䔀䌀吀 嬀䤀䐀崀Ⰰ 嬀䐀攀猀挀爀椀瀀琀椀漀渀崀Ⰰ 嬀䌀漀洀洀愀渀搀崀 䘀刀伀䴀 嬀䌀漀洀洀愀渀搀猀崀 圀䠀䔀刀䔀 ഀഀ
		( Access >= @MinAccess AND Access < @MaxAccess ) ਍ऀऀऀ伀刀䐀䔀刀 䈀夀 䄀挀挀攀猀猀ഀഀ
਍刀䔀吀唀刀一ഀഀ
' ਍䔀一䐀ഀഀ
GO਍匀䔀吀 䄀一匀䤀开一唀䰀䰀匀 伀一ഀഀ
GO਍匀䔀吀 儀唀伀吀䔀䐀开䤀䐀䔀一吀䤀䘀䤀䔀刀 伀一ഀഀ
GO਍䤀䘀 一伀吀 䔀堀䤀匀吀匀 ⠀匀䔀䰀䔀䌀吀 ⨀ 䘀刀伀䴀 猀礀猀⸀漀戀樀攀挀琀猀 圀䠀䔀刀䔀 漀戀樀攀挀琀开椀搀 㴀 伀䈀䨀䔀䌀吀开䤀䐀⠀一✀嬀搀戀漀崀⸀嬀䜀攀琀一攀眀䤀䐀崀✀⤀ 䄀一䐀 琀礀瀀攀 椀渀 ⠀一✀倀✀Ⰰ 一✀倀䌀✀⤀⤀ഀഀ
BEGIN਍䔀堀䔀䌀 搀戀漀⸀猀瀀开攀砀攀挀甀琀攀猀焀氀 䀀猀琀愀琀攀洀攀渀琀 㴀 一✀䌀刀䔀䄀吀䔀 倀刀伀䌀䔀䐀唀刀䔀 嬀搀戀漀崀⸀嬀䜀攀琀一攀眀䤀䐀崀 ഀഀ
਍ऀ䀀吀愀戀氀攀一愀洀攀 吀攀砀琀 ഀഀ
਍䄀匀ഀഀ
	DECLARE @Count int਍ऀഀഀ
	SET @Count = IDENT_CURRENT( @TableName )਍ऀ匀䔀䰀䔀䌀吀 䀀䌀漀甀渀琀ഀഀ
	਍刀䔀吀唀刀一ഀഀ
' ਍䔀一䐀ഀഀ
GO਍匀䔀吀 䄀一匀䤀开一唀䰀䰀匀 伀一ഀഀ
GO਍匀䔀吀 儀唀伀吀䔀䐀开䤀䐀䔀一吀䤀䘀䤀䔀刀 伀一ഀഀ
GO਍䤀䘀 一伀吀 䔀堀䤀匀吀匀 ⠀匀䔀䰀䔀䌀吀 ⨀ 䘀刀伀䴀 猀礀猀⸀漀戀樀攀挀琀猀 圀䠀䔀刀䔀 漀戀樀攀挀琀开椀搀 㴀 伀䈀䨀䔀䌀吀开䤀䐀⠀一✀嬀搀戀漀崀⸀嬀䈀甀椀氀搀䰀漀最崀✀⤀ 䄀一䐀 琀礀瀀攀 椀渀 ⠀一✀唀✀⤀⤀ഀഀ
BEGIN਍䌀刀䔀䄀吀䔀 吀䄀䈀䰀䔀 嬀搀戀漀崀⸀嬀䈀甀椀氀搀䰀漀最崀⠀ഀഀ
	[ID] [int] IDENTITY(1,1) NOT NULL,਍ऀ嬀䌀漀洀洀愀渀搀崀 嬀瘀愀爀挀栀愀爀崀⠀㌀㈀⤀ 一唀䰀䰀Ⰰഀഀ
	[Machine] [varchar](16) NULL,਍ऀ嬀䌀栀愀渀最攀䰀椀猀琀崀 嬀椀渀琀崀 一唀䰀䰀Ⰰഀഀ
	[Promotable] [int] NULL,਍ऀ嬀䈀甀椀氀搀匀琀愀爀琀攀搀崀 嬀搀愀琀攀琀椀洀攀崀 一唀䰀䰀Ⰰഀഀ
	[BuildEnded] [datetime] NULL,਍ऀ嬀䌀甀爀爀攀渀琀匀琀愀琀甀猀崀 嬀瘀愀爀挀栀愀爀崀⠀㄀㈀㠀⤀ 一唀䰀䰀Ⰰഀഀ
	[BuildLabel] [varchar](48) NULL਍⤀ 伀一 嬀倀刀䤀䴀䄀刀夀崀ഀഀ
END਍䜀伀ഀഀ
SET ANSI_NULLS ON਍䜀伀ഀഀ
SET QUOTED_IDENTIFIER ON਍䜀伀ഀഀ
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[PerformanceData]') AND type in (N'U'))਍䈀䔀䜀䤀一ഀഀ
CREATE TABLE [dbo].[PerformanceData](਍ऀ嬀䄀瀀瀀䤀䐀崀 嬀搀戀漀崀⸀嬀䬀攀礀䤀䐀崀 一伀吀 一唀䰀䰀Ⰰഀഀ
	[MachineID] [dbo].[KeyID] NOT NULL,਍ऀ嬀䌀漀甀渀琀攀爀䤀䐀崀 嬀搀戀漀崀⸀嬀䬀攀礀䤀䐀崀 一伀吀 一唀䰀䰀Ⰰഀഀ
	[IntValue] [bigint] NOT NULL CONSTRAINT [DF_PerformanceData_IntValue]  DEFAULT ((0)),਍ऀ嬀䐀愀琀攀吀椀洀攀匀琀愀洀瀀崀 嬀搀愀琀攀琀椀洀攀崀 一伀吀 一唀䰀䰀ഀഀ
) ON [PRIMARY]਍䔀一䐀ഀഀ
GO਍匀䔀吀 䄀一匀䤀开一唀䰀䰀匀 伀一ഀഀ
GO਍匀䔀吀 儀唀伀吀䔀䐀开䤀䐀䔀一吀䤀䘀䤀䔀刀 伀一ഀഀ
GO਍䤀䘀 一伀吀 䔀堀䤀匀吀匀 ⠀匀䔀䰀䔀䌀吀 ⨀ 䘀刀伀䴀 猀礀猀⸀漀戀樀攀挀琀猀 圀䠀䔀刀䔀 漀戀樀攀挀琀开椀搀 㴀 伀䈀䨀䔀䌀吀开䤀䐀⠀一✀嬀搀戀漀崀⸀嬀䴀愀挀栀椀渀攀猀崀✀⤀ 䄀一䐀 琀礀瀀攀 椀渀 ⠀一✀唀✀⤀⤀ഀഀ
BEGIN਍䌀刀䔀䄀吀䔀 吀䄀䈀䰀䔀 嬀搀戀漀崀⸀嬀䴀愀挀栀椀渀攀猀崀⠀ഀഀ
	[ID] [dbo].[KeyID] IDENTITY(1,1) NOT NULL,਍ऀ嬀䴀愀挀栀椀渀攀一愀洀攀崀 嬀搀戀漀崀⸀嬀䘀椀砀攀搀匀琀爀椀渀最崀 一伀吀 一唀䰀䰀Ⰰഀഀ
	[Notes] [dbo].[String] NULL,਍ 䌀伀一匀吀刀䄀䤀一吀 嬀倀䬀开䴀愀挀栀椀渀攀崀 倀刀䤀䴀䄀刀夀 䬀䔀夀 䌀䰀唀匀吀䔀刀䔀䐀 ഀഀ
(਍ऀ嬀䤀䐀崀 䄀匀䌀ഀഀ
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY],਍ 䌀伀一匀吀刀䄀䤀一吀 嬀唀渀椀焀甀攀开䴀愀挀栀椀渀攀一愀洀攀崀 唀一䤀儀唀䔀 一伀一䌀䰀唀匀吀䔀刀䔀䐀 ഀഀ
(਍ऀ嬀䴀愀挀栀椀渀攀一愀洀攀崀 䄀匀䌀ഀഀ
)WITH (PAD_INDEX  = OFF, IGNORE_DUP_KEY = OFF) ON [PRIMARY]਍⤀ 伀一 嬀倀刀䤀䴀䄀刀夀崀ഀഀ
END਍䜀伀ഀഀ
SET ANSI_NULLS ON਍䜀伀ഀഀ
SET QUOTED_IDENTIFIER ON਍䜀伀ഀഀ
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Applications]') AND type in (N'U'))਍䈀䔀䜀䤀一ഀഀ
CREATE TABLE [dbo].[Applications](਍ऀ嬀䤀䐀崀 嬀搀戀漀崀⸀嬀䬀攀礀䤀䐀崀 䤀䐀䔀一吀䤀吀夀⠀㄀Ⰰ㄀⤀ 一伀吀 一唀䰀䰀Ⰰഀഀ
	[AppName] [dbo].[FixedString] NOT NULL,਍ऀ嬀一漀琀攀猀崀 嬀搀戀漀崀⸀嬀匀琀爀椀渀最崀 一唀䰀䰀Ⰰഀഀ
 CONSTRAINT [PK_Application] PRIMARY KEY CLUSTERED ਍⠀ഀഀ
	[ID] ASC਍⤀圀䤀吀䠀 ⠀倀䄀䐀开䤀一䐀䔀堀  㴀 伀䘀䘀Ⰰ 䤀䜀一伀刀䔀开䐀唀倀开䬀䔀夀 㴀 伀䘀䘀⤀ 伀一 嬀倀刀䤀䴀䄀刀夀崀Ⰰഀഀ
 CONSTRAINT [Unique_Application] UNIQUE NONCLUSTERED ਍⠀ഀഀ
	[AppName] ASC਍⤀圀䤀吀䠀 ⠀倀䄀䐀开䤀一䐀䔀堀  㴀 伀䘀䘀Ⰰ 䤀䜀一伀刀䔀开䐀唀倀开䬀䔀夀 㴀 伀䘀䘀⤀ 伀一 嬀倀刀䤀䴀䄀刀夀崀ഀഀ
) ON [PRIMARY]਍䔀一䐀ഀഀ
GO਍匀䔀吀 䄀一匀䤀开一唀䰀䰀匀 伀一ഀഀ
GO਍匀䔀吀 儀唀伀吀䔀䐀开䤀䐀䔀一吀䤀䘀䤀䔀刀 伀一ഀഀ
GO਍䤀䘀 一伀吀 䔀堀䤀匀吀匀 ⠀匀䔀䰀䔀䌀吀 ⨀ 䘀刀伀䴀 猀礀猀⸀漀戀樀攀挀琀猀 圀䠀䔀刀䔀 漀戀樀攀挀琀开椀搀 㴀 伀䈀䨀䔀䌀吀开䤀䐀⠀一✀嬀搀戀漀崀⸀嬀倀攀爀昀漀爀洀愀渀挀攀䌀漀甀渀琀攀爀猀崀✀⤀ 䄀一䐀 琀礀瀀攀 椀渀 ⠀一✀唀✀⤀⤀ഀഀ
BEGIN਍䌀刀䔀䄀吀䔀 吀䄀䈀䰀䔀 嬀搀戀漀崀⸀嬀倀攀爀昀漀爀洀愀渀挀攀䌀漀甀渀琀攀爀猀崀⠀ഀഀ
	[ID] [dbo].[KeyID] IDENTITY(1,1) NOT NULL,਍ऀ嬀䌀漀甀渀琀攀爀一愀洀攀崀 嬀搀戀漀崀⸀嬀䘀椀砀攀搀匀琀爀椀渀最崀 一伀吀 一唀䰀䰀Ⰰഀഀ
 CONSTRAINT [PK_PerformanceCounters] PRIMARY KEY CLUSTERED ਍⠀ഀഀ
	[ID] ASC਍⤀圀䤀吀䠀 ⠀倀䄀䐀开䤀一䐀䔀堀  㴀 伀䘀䘀Ⰰ 䤀䜀一伀刀䔀开䐀唀倀开䬀䔀夀 㴀 伀䘀䘀⤀ 伀一 嬀倀刀䤀䴀䄀刀夀崀ഀഀ
) ON [PRIMARY]਍䔀一䐀ഀഀ
GO਍匀䔀吀 䄀一匀䤀开一唀䰀䰀匀 伀一ഀഀ
GO਍匀䔀吀 儀唀伀吀䔀䐀开䤀䐀䔀一吀䤀䘀䤀䔀刀 伀一ഀഀ
GO਍䤀䘀 一伀吀 䔀堀䤀匀吀匀 ⠀匀䔀䰀䔀䌀吀 ⨀ 䘀刀伀䴀 猀礀猀⸀漀戀樀攀挀琀猀 圀䠀䔀刀䔀 漀戀樀攀挀琀开椀搀 㴀 伀䈀䨀䔀䌀吀开䤀䐀⠀一✀嬀搀戀漀崀⸀嬀䜀攀琀䄀挀琀椀瘀攀䘀愀猀琀䈀甀椀氀搀攀爀䌀漀甀渀琀崀✀⤀ 䄀一䐀 琀礀瀀攀 椀渀 ⠀一✀倀✀Ⰰ 一✀倀䌀✀⤀⤀ഀഀ
BEGIN਍䔀堀䔀䌀 搀戀漀⸀猀瀀开攀砀攀挀甀琀攀猀焀氀 䀀猀琀愀琀攀洀攀渀琀 㴀 一✀䌀刀䔀䄀吀䔀 倀刀伀䌀䔀䐀唀刀䔀 嬀搀戀漀崀⸀嬀䜀攀琀䄀挀琀椀瘀攀䘀愀猀琀䈀甀椀氀搀攀爀䌀漀甀渀琀崀 ഀഀ
਍䄀匀ഀഀ
	BEGIN TRAN਍   ഀഀ
	SELECT COUNT( ID ) FROM [Builders]਍ऀ圀䠀䔀刀䔀 ⠀ 伀匀㘀㐀䈀椀琀 㴀 ㄀ ⤀ 䄀一䐀 ⠀ 匀琀愀琀攀 㴀 ✀✀䌀漀渀渀攀挀琀攀搀✀✀ ⤀ 䄀一䐀 ⠀ 䐀䄀吀䔀䐀䤀䘀䘀⠀ 洀椀渀甀琀攀Ⰰ 䌀甀爀爀攀渀琀吀椀洀攀Ⰰ 䜀䔀吀䐀䄀吀䔀⠀⤀ ⤀ 㰀 ㄀　　 ⤀ഀഀ
	      AND ( NOT EXISTS ( SELECT MachineLock FROM [Commands]਍ऀऀऀऀ圀䠀䔀刀䔀 ⠀ 䴀愀挀栀椀渀攀䰀漀挀欀 㴀 䈀甀椀氀搀攀爀猀⸀䴀愀挀栀椀渀攀 ⤀ ⤀ ⤀ഀഀ
 ਍ऀ䌀伀䴀䴀䤀吀ഀഀ
 ਍ऀ刀䔀吀唀刀一ഀഀ
' ਍䔀一䐀ഀഀ
GO਍匀䔀吀 䄀一匀䤀开一唀䰀䰀匀 伀一ഀഀ
GO਍匀䔀吀 儀唀伀吀䔀䐀开䤀䐀䔀一吀䤀䘀䤀䔀刀 伀一ഀഀ
GO਍䤀䘀 一伀吀 䔀堀䤀匀吀匀 ⠀匀䔀䰀䔀䌀吀 ⨀ 䘀刀伀䴀 猀礀猀⸀漀戀樀攀挀琀猀 圀䠀䔀刀䔀 漀戀樀攀挀琀开椀搀 㴀 伀䈀䨀䔀䌀吀开䤀䐀⠀一✀嬀搀戀漀崀⸀嬀䌀栀攀挀欀䨀漀戀崀✀⤀ 䄀一䐀 琀礀瀀攀 椀渀 ⠀一✀倀✀Ⰰ 一✀倀䌀✀⤀⤀ഀഀ
BEGIN਍䔀堀䔀䌀 搀戀漀⸀猀瀀开攀砀攀挀甀琀攀猀焀氀 䀀猀琀愀琀攀洀攀渀琀 㴀 一✀  䌀刀䔀䄀吀䔀 倀刀伀䌀䔀䐀唀刀䔀 嬀搀戀漀崀⸀嬀䌀栀攀挀欀䨀漀戀崀ഀഀ
਍ऀ䀀䴀愀挀栀椀渀攀一愀洀攀 瘀愀爀挀栀愀爀⠀㌀㈀⤀Ⰰഀഀ
	@OS64Bit bit਍ഀഀ
AS਍ഀഀ
   BEGIN TRAN਍   䐀䔀䌀䰀䄀刀䔀 䀀吀爀椀最最攀爀 䤀一吀ഀഀ
   DECLARE @BuilderID INT਍   匀䔀吀 䀀吀爀椀最最攀爀 㴀 　ഀഀ
   SET @BuilderID = -1਍   ഀഀ
   	SELECT @BuilderID = ID FROM Builders WHERE ਍ऀऀ ⠀ 䴀愀挀栀椀渀攀 㴀 䀀䴀愀挀栀椀渀攀一愀洀攀 䄀一䐀 匀琀愀琀攀 ℀㴀 ✀✀䐀攀愀搀✀✀ 䄀一䐀 匀琀愀琀攀 ℀㴀 ✀✀娀漀洀戀椀攀搀✀✀ ⤀ 䄀一䐀 ⠀ 䐀䄀吀䔀䐀䤀䘀䘀⠀ 洀椀渀甀琀攀Ⰰ 䌀甀爀爀攀渀琀吀椀洀攀Ⰰ 䜀䔀吀䐀䄀吀䔀⠀⤀ ⤀ 㰀 ㄀　　 ⤀ഀഀ
   ਍   匀䔀䰀䔀䌀吀 䀀吀爀椀最最攀爀 㴀 䨀漀戀猀⸀䤀䐀 䘀刀伀䴀 䨀漀戀猀Ⰰ 䈀爀愀渀挀栀攀猀 圀䠀䔀刀䔀 ഀഀ
		( ( Jobs.Active = 0 AND Jobs.Complete = 0 )਍ऀऀ䄀一䐀 ⠀ 䀀伀匀㘀㐀䈀椀琀 㴀 　 伀刀 䨀漀戀猀⸀䌀漀洀瀀愀琀椀戀氀攀㘀㐀䈀椀琀 㴀 䀀伀匀㘀㐀䈀椀琀 ⤀ഀഀ
		AND ( Branches.Branch = Jobs.Branch )਍ऀऀ䄀一䐀 ⠀ 䈀爀愀渀挀栀攀猀⸀䈀甀椀氀搀攀爀䤀䐀 㴀 䀀䈀甀椀氀搀攀爀䤀䐀 ⤀ ⤀ഀഀ
		਍   椀昀 䀀吀爀椀最最攀爀 ℀㴀 　 唀倀䐀䄀吀䔀 䨀漀戀猀 匀䔀吀 䄀挀琀椀瘀攀 㴀 ㄀ 圀䠀䔀刀䔀 ഀഀ
		( ID = @Trigger )਍ऀऀഀഀ
   SELECT @Trigger਍   ഀഀ
   COMMIT਍ഀഀ
RETURN਍✀ ഀഀ
END਍䜀伀ഀഀ
SET ANSI_NULLS ON਍䜀伀ഀഀ
SET QUOTED_IDENTIFIER ON਍䜀伀ഀഀ
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[GetActiveBuilderCount]') AND type in (N'P', N'PC'))਍䈀䔀䜀䤀一ഀഀ
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[GetActiveBuilderCount] ਍ഀഀ
AS਍   䈀䔀䜀䤀一 吀刀䄀一ഀഀ
   ਍   匀䔀䰀䔀䌀吀 䌀伀唀一吀⠀ 䤀䐀 ⤀ 䘀刀伀䴀 嬀䈀甀椀氀搀攀爀猀崀 圀䠀䔀刀䔀 ⠀ 匀琀愀琀攀 㴀 ✀✀䌀漀渀渀攀挀琀攀搀✀✀ ⤀ 䄀一䐀 ⠀ 䐀䄀吀䔀䐀䤀䘀䘀⠀ 洀椀渀甀琀攀Ⰰ 䌀甀爀爀攀渀琀吀椀洀攀Ⰰ 䜀䔀吀䐀䄀吀䔀⠀⤀ ⤀ 㰀 ㄀　　 ⤀ഀഀ
 ਍   䌀伀䴀䴀䤀吀ഀഀ
 ਍   刀䔀吀唀刀一ഀഀ
' ਍䔀一䐀ഀഀ
GO਍匀䔀吀 䄀一匀䤀开一唀䰀䰀匀 伀一ഀഀ
GO਍匀䔀吀 儀唀伀吀䔀䐀开䤀䐀䔀一吀䤀䘀䤀䔀刀 伀一ഀഀ
GO਍䤀䘀 一伀吀 䔀堀䤀匀吀匀 ⠀匀䔀䰀䔀䌀吀 ⨀ 䘀刀伀䴀 猀礀猀⸀漀戀樀攀挀琀猀 圀䠀䔀刀䔀 漀戀樀攀挀琀开椀搀 㴀 伀䈀䨀䔀䌀吀开䤀䐀⠀一✀嬀搀戀漀崀⸀嬀䌀栀攀挀欀吀椀洀攀搀䈀甀椀氀搀崀✀⤀ 䄀一䐀 琀礀瀀攀 椀渀 ⠀一✀倀✀Ⰰ 一✀倀䌀✀⤀⤀ഀഀ
BEGIN਍䔀堀䔀䌀 搀戀漀⸀猀瀀开攀砀攀挀甀琀攀猀焀氀 䀀猀琀愀琀攀洀攀渀琀 㴀 一✀  䌀刀䔀䄀吀䔀 倀刀伀䌀䔀䐀唀刀䔀 嬀搀戀漀崀⸀嬀䌀栀攀挀欀吀椀洀攀搀䈀甀椀氀搀崀ഀഀ
਍ऀ䀀䴀愀挀栀椀渀攀一愀洀攀 瘀愀爀挀栀愀爀⠀㌀㈀⤀Ⰰ ഀഀ
	@AnyMachine bit,਍ऀ䀀伀匀㘀㐀䈀椀琀 戀椀琀ഀഀ
਍䄀匀ഀഀ
਍   䈀䔀䜀䤀一 吀刀䄀一ഀഀ
   DECLARE @Trigger INT਍   䐀䔀䌀䰀䄀刀䔀 䀀䈀甀椀氀搀攀爀䤀䐀 䤀一吀ഀഀ
   SET @Trigger = 0਍   匀䔀吀 䀀䈀甀椀氀搀攀爀䤀䐀 㴀 ⴀ㄀ഀഀ
   ਍ऀ匀䔀䰀䔀䌀吀 䀀䈀甀椀氀搀攀爀䤀䐀 㴀 䤀䐀 䘀刀伀䴀 嬀䈀甀椀氀搀攀爀猀崀 圀䠀䔀刀䔀 ഀഀ
		 ( Machine = @MachineName AND State != ''Dead'' AND State != ''Zombied'' )਍   ഀഀ
   SELECT @Trigger = Commands.ID FROM [Commands], [Branches] WHERE ਍ऀऀ⠀ ⠀ 䜀䔀吀䐀䄀吀䔀⠀⤀ 㸀 䌀漀洀洀愀渀搀猀⸀一攀砀琀吀爀椀最最攀爀 ⤀ഀഀ
		AND ( @OS64Bit = 0 OR Commands.Compatible64Bit = @OS64Bit )਍ऀऀ䄀一䐀 ⠀ 䈀爀愀渀挀栀攀猀⸀䈀爀愀渀挀栀 㴀 䌀漀洀洀愀渀搀猀⸀䈀爀愀渀挀栀 ⤀ഀഀ
		AND ( Branches.BuilderID = @BuilderID )਍ऀऀ䄀一䐀 ⠀ ⠀ 䌀漀洀洀愀渀搀猀⸀䴀愀挀栀椀渀攀䰀漀挀欀 㴀 匀唀䈀匀吀刀䤀一䜀⠀ 䀀䴀愀挀栀椀渀攀一愀洀攀Ⰰ ㄀Ⰰ 䰀䔀一⠀ 䌀漀洀洀愀渀搀猀⸀䴀愀挀栀椀渀攀䰀漀挀欀 ⤀ ⤀ 伀刀 ⠀ 䌀漀洀洀愀渀搀猀⸀䴀愀挀栀椀渀攀䰀漀挀欀 㴀 ✀✀一漀渀攀✀✀ 䄀一䐀 䀀䄀渀礀䴀愀挀栀椀渀攀 㴀 ㄀ ⤀ ⤀ ⤀ ⤀ഀഀ
		਍   椀昀 䀀吀爀椀最最攀爀 ℀㴀 　 唀倀䐀䄀吀䔀 嬀䌀漀洀洀愀渀搀猀崀 匀䔀吀 䌀漀洀洀愀渀搀猀⸀一攀砀琀吀爀椀最最攀爀 㴀 䐀䄀吀䔀䄀䐀䐀⠀ 洀椀渀甀琀攀Ⰰ 䌀漀洀洀愀渀搀猀⸀一攀砀琀吀爀椀最最攀爀䐀攀氀愀礀Ⰰ 䌀漀洀洀愀渀搀猀⸀一攀砀琀吀爀椀最最攀爀 ⤀ 圀䠀䔀刀䔀ഀഀ
		( ID = @Trigger )਍ऀऀഀഀ
   SELECT @Trigger਍   䌀伀䴀䴀䤀吀ഀഀ
਍刀䔀吀唀刀一ഀഀ
' ਍䔀一䐀ഀഀ
GO਍匀䔀吀 䄀一匀䤀开一唀䰀䰀匀 伀一ഀഀ
GO਍匀䔀吀 儀唀伀吀䔀䐀开䤀䐀䔀一吀䤀䘀䤀䔀刀 伀一ഀഀ
GO਍䤀䘀 一伀吀 䔀堀䤀匀吀匀 ⠀匀䔀䰀䔀䌀吀 ⨀ 䘀刀伀䴀 猀礀猀⸀漀戀樀攀挀琀猀 圀䠀䔀刀䔀 漀戀樀攀挀琀开椀搀 㴀 伀䈀䨀䔀䌀吀开䤀䐀⠀一✀嬀搀戀漀崀⸀嬀䌀栀攀挀欀吀爀椀最最攀爀攀搀䈀甀椀氀搀崀✀⤀ 䄀一䐀 琀礀瀀攀 椀渀 ⠀一✀倀✀Ⰰ 一✀倀䌀✀⤀⤀ഀഀ
BEGIN਍䔀堀䔀䌀 搀戀漀⸀猀瀀开攀砀攀挀甀琀攀猀焀氀 䀀猀琀愀琀攀洀攀渀琀 㴀 一✀      䌀刀䔀䄀吀䔀 倀刀伀䌀䔀䐀唀刀䔀 嬀搀戀漀崀⸀嬀䌀栀攀挀欀吀爀椀最最攀爀攀搀䈀甀椀氀搀崀ഀഀ
਍ऀ䀀䴀愀挀栀椀渀攀一愀洀攀 瘀愀爀挀栀愀爀⠀㌀㈀⤀Ⰰ ഀഀ
	@AnyMachine bit,਍ऀ䀀伀匀㘀㐀䈀椀琀 戀椀琀ഀഀ
਍䄀匀ഀഀ
਍   䈀䔀䜀䤀一 吀刀䄀一ഀഀ
   DECLARE @Trigger INT਍   䐀䔀䌀䰀䄀刀䔀 䀀䤀猀嘀愀氀椀搀 䤀一吀ഀഀ
   DECLARE @BuilderID INT਍   匀䔀吀 䀀吀爀椀最最攀爀 㴀 　ഀഀ
   ਍ऀ匀䔀䰀䔀䌀吀 䀀䈀甀椀氀搀攀爀䤀䐀 㴀 䤀䐀 䘀刀伀䴀 嬀䈀甀椀氀搀攀爀猀崀 圀䠀䔀刀䔀 ഀഀ
		 ( Machine = @MachineName AND State != ''Dead'' AND State != ''Zombied'' )਍   ഀഀ
   SELECT @Trigger = Commands.ID FROM [Commands], [Branches] WHERE ਍ऀऀ⠀ ⠀ 䌀漀洀洀愀渀搀猀⸀倀攀渀搀椀渀最 㴀 ㄀ ⤀ഀഀ
		AND ( @OS64Bit = 0 OR Commands.Compatible64Bit = @OS64Bit )਍ऀऀ䄀一䐀 ⠀ 䈀爀愀渀挀栀攀猀⸀䈀爀愀渀挀栀 㴀 䌀漀洀洀愀渀搀猀⸀䈀爀愀渀挀栀 ⤀ഀഀ
		AND ( Branches.BuilderID = @BuilderID )਍ऀऀ䄀一䐀 ⠀ ⠀ 䌀漀洀洀愀渀搀猀⸀䴀愀挀栀椀渀攀䰀漀挀欀 㴀 匀唀䈀匀吀刀䤀一䜀⠀ 䀀䴀愀挀栀椀渀攀一愀洀攀Ⰰ ㄀Ⰰ 䰀䔀一⠀ 䌀漀洀洀愀渀搀猀⸀䴀愀挀栀椀渀攀䰀漀挀欀 ⤀ ⤀ 伀刀 ⠀ 䌀漀洀洀愀渀搀猀⸀䴀愀挀栀椀渀攀䰀漀挀欀 㴀 ✀✀一漀渀攀✀✀ 䄀一䐀 䀀䄀渀礀䴀愀挀栀椀渀攀 㴀 ㄀ ⤀ ⤀ ⤀ ⤀ഀഀ
		਍   椀昀 䀀吀爀椀最最攀爀 ℀㴀 　 唀倀䐀䄀吀䔀 嬀䌀漀洀洀愀渀搀猀崀 匀䔀吀 倀攀渀搀椀渀最 㴀 　 圀䠀䔀刀䔀ഀഀ
		( ID = @Trigger )਍ऀऀഀഀ
   SELECT @Trigger਍   䌀伀䴀䴀䤀吀ഀഀ
਍刀䔀吀唀刀一ഀഀ
' ਍䔀一䐀ഀഀ
GO਍匀䔀吀 䄀一匀䤀开一唀䰀䰀匀 伀一ഀഀ
GO਍匀䔀吀 儀唀伀吀䔀䐀开䤀䐀䔀一吀䤀䘀䤀䔀刀 伀一ഀഀ
GO਍䤀䘀 一伀吀 䔀堀䤀匀吀匀 ⠀匀䔀䰀䔀䌀吀 ⨀ 䘀刀伀䴀 猀礀猀⸀漀戀樀攀挀琀猀 圀䠀䔀刀䔀 漀戀樀攀挀琀开椀搀 㴀 伀䈀䨀䔀䌀吀开䤀䐀⠀一✀嬀搀戀漀崀⸀嬀䜀攀琀㘀㐀䈀椀琀䈀甀椀氀搀猀崀✀⤀ 䄀一䐀 琀礀瀀攀 椀渀 ⠀一✀倀✀Ⰰ 一✀倀䌀✀⤀⤀ഀഀ
BEGIN਍䔀堀䔀䌀 搀戀漀⸀猀瀀开攀砀攀挀甀琀攀猀焀氀 䀀猀琀愀琀攀洀攀渀琀 㴀 一✀䌀刀䔀䄀吀䔀 倀刀伀䌀䔀䐀唀刀䔀 嬀搀戀漀崀⸀嬀䜀攀琀㘀㐀䈀椀琀䈀甀椀氀搀猀崀ഀഀ
਍䄀匀ഀഀ
਍ऀ䈀䔀䜀䤀一 吀刀䄀一ഀഀ
਍ऀ䐀䔀䌀䰀䄀刀䔀 䀀䌀漀洀洀愀渀搀䌀漀甀渀琀 䤀一吀ഀഀ
	DECLARE @JobCount INT਍ഀഀ
	SELECT @CommandCount = COUNT( ID ) FROM [Commands] WHERE ( Pending = 1 AND Compatible64Bit = 1 )਍ऀ匀䔀䰀䔀䌀吀 䀀䨀漀戀䌀漀甀渀琀 㴀 䌀伀唀一吀⠀ 䤀䐀 ⤀ 䘀刀伀䴀 嬀䨀漀戀猀崀 圀䠀䔀刀䔀 ⠀ 䄀挀琀椀瘀攀 㴀 　 䄀一䐀 䌀漀洀瀀氀攀琀攀 㴀 　 䄀一䐀 䌀漀洀瀀愀琀椀戀氀攀㘀㐀䈀椀琀 㴀 ㄀ ⤀ഀഀ
਍ऀ匀䔀吀 䀀䌀漀洀洀愀渀搀䌀漀甀渀琀 㴀 䀀䌀漀洀洀愀渀搀䌀漀甀渀琀 ⬀ 䀀䨀漀戀䌀漀甀渀琀ഀഀ
	SELECT @CommandCount਍ഀഀ
	COMMIT਍ഀഀ
RETURN਍ഀഀ
' ਍䔀一䐀ഀഀ
GO਍匀䔀吀 䄀一匀䤀开一唀䰀䰀匀 伀一ഀഀ
GO਍匀䔀吀 儀唀伀吀䔀䐀开䤀䐀䔀一吀䤀䘀䤀䔀刀 伀一ഀഀ
GO਍䤀䘀 一伀吀 䔀堀䤀匀吀匀 ⠀匀䔀䰀䔀䌀吀 ⨀ 䘀刀伀䴀 猀礀猀⸀漀戀樀攀挀琀猀 圀䠀䔀刀䔀 漀戀樀攀挀琀开椀搀 㴀 伀䈀䨀䔀䌀吀开䤀䐀⠀一✀嬀搀戀漀崀⸀嬀匀攀氀攀挀琀䈀甀椀氀搀匀琀愀琀甀猀崀✀⤀ 䄀一䐀 琀礀瀀攀 椀渀 ⠀一✀倀✀Ⰰ 一✀倀䌀✀⤀⤀ഀഀ
BEGIN਍䔀堀䔀䌀 搀戀漀⸀猀瀀开攀砀攀挀甀琀攀猀焀氀 䀀猀琀愀琀攀洀攀渀琀 㴀 一✀䌀刀䔀䄀吀䔀 倀刀伀䌀䔀䐀唀刀䔀 嬀搀戀漀崀⸀嬀匀攀氀攀挀琀䈀甀椀氀搀匀琀愀琀甀猀崀 ഀഀ
਍䄀匀ഀഀ
਍匀䔀䰀䔀䌀吀 䌀⸀䈀甀椀氀搀䰀漀最䤀䐀Ⰰ 䈀⸀䤀䐀Ⰰ 䌀⸀䴀愀挀栀椀渀攀Ⰰ 䈀⸀䌀栀愀渀最攀䰀椀猀琀Ⰰ 䈀⸀䌀甀爀爀攀渀琀匀琀愀琀甀猀Ⰰ 䈀⸀䈀甀椀氀搀匀琀愀爀琀攀搀Ⰰ 䌀⸀伀瀀攀爀愀琀漀爀Ⰰ 䌀⸀䐀攀猀挀爀椀瀀琀椀漀渀ഀഀ
FROM Commands C, BuildLog B ਍圀䠀䔀刀䔀 ⠀ 䌀⸀䈀甀椀氀搀䰀漀最䤀䐀 㴀 䈀⸀䤀䐀 ⤀ഀഀ
਍ऀ刀䔀吀唀刀一ഀഀ
' ਍䔀一䐀ഀഀ
GO਍匀䔀吀 䄀一匀䤀开一唀䰀䰀匀 伀一ഀഀ
GO਍匀䔀吀 儀唀伀吀䔀䐀开䤀䐀䔀一吀䤀䘀䤀䔀刀 伀一ഀഀ
GO਍䤀䘀 一伀吀 䔀堀䤀匀吀匀 ⠀匀䔀䰀䔀䌀吀 ⨀ 䘀刀伀䴀 猀礀猀⸀漀戀樀攀挀琀猀 圀䠀䔀刀䔀 漀戀樀攀挀琀开椀搀 㴀 伀䈀䨀䔀䌀吀开䤀䐀⠀一✀嬀搀戀漀崀⸀嬀匀攀氀攀挀琀䨀漀戀匀琀愀琀甀猀崀✀⤀ 䄀一䐀 琀礀瀀攀 椀渀 ⠀一✀倀✀Ⰰ 一✀倀䌀✀⤀⤀ഀഀ
BEGIN਍䔀堀䔀䌀 搀戀漀⸀猀瀀开攀砀攀挀甀琀攀猀焀氀 䀀猀琀愀琀攀洀攀渀琀 㴀 一✀䌀刀䔀䄀吀䔀 倀刀伀䌀䔀䐀唀刀䔀 嬀搀戀漀崀⸀嬀匀攀氀攀挀琀䨀漀戀匀琀愀琀甀猀崀 ഀഀ
਍䄀匀ഀഀ
਍ऀ匀䔀䰀䔀䌀吀 䨀⸀䈀甀椀氀搀䰀漀最䤀䐀Ⰰ 䈀⸀䤀䐀Ⰰ 䨀⸀䴀愀挀栀椀渀攀Ⰰ 䈀⸀䈀甀椀氀搀匀琀愀爀琀攀搀Ⰰ 䈀⸀䌀甀爀爀攀渀琀匀琀愀琀甀猀 䘀刀伀䴀 䨀漀戀猀 䨀Ⰰ 䈀甀椀氀搀䰀漀最 䈀 圀䠀䔀刀䔀ഀഀ
		( J.BuildLogID = B.ID ) ਍ऀऀ䄀一䐀 ⠀ 䄀挀琀椀瘀攀 㴀 ㄀ ⤀ഀഀ
		AND ( Complete = 0 )਍ഀഀ
RETURN਍✀ ഀഀ
END਍䜀伀ഀഀ
SET ANSI_NULLS ON਍䜀伀ഀഀ
SET QUOTED_IDENTIFIER ON਍䜀伀ഀഀ
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[GetMachineID]') AND type in (N'P', N'PC'))਍䈀䔀䜀䤀一ഀഀ
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[GetMachineID]਍ऀ⠀ഀഀ
	@MachineName FixedString਍ऀ⤀ഀഀ
AS਍ऀ匀䔀吀 一伀䌀伀唀一吀 伀一ഀഀ
	਍ऀ䐀䔀䌀䰀䄀刀䔀 䀀䴀愀挀栀椀渀攀䤀䐀 䬀攀礀䤀䐀ഀഀ
	਍ऀ匀䔀吀 䀀䴀愀挀栀椀渀攀䤀䐀 㴀 ⠀匀䔀䰀䔀䌀吀 䤀䐀 䘀刀伀䴀 搀戀漀⸀䴀愀挀栀椀渀攀猀 圀䠀䔀刀䔀 䴀愀挀栀椀渀攀一愀洀攀 㴀 䀀䴀愀挀栀椀渀攀一愀洀攀⤀ഀഀ
	਍ऀ䤀䘀 ⠀䀀䴀愀挀栀椀渀攀䤀䐀 䤀匀 一唀䰀䰀⤀ഀഀ
	BEGIN਍ऀऀ䤀一匀䔀刀吀 䤀一吀伀 搀戀漀⸀䴀愀挀栀椀渀攀猀 ⠀䴀愀挀栀椀渀攀一愀洀攀⤀ 嘀䄀䰀唀䔀匀 ⠀䀀䴀愀挀栀椀渀攀一愀洀攀⤀ഀഀ
		SET @MachineID = @@IDENTITY਍ऀ䔀一䐀ഀഀ
	਍ऀ刀䔀吀唀刀一 䀀䴀愀挀栀椀渀攀䤀䐀ഀഀ
' ਍䔀一䐀ഀഀ
GO਍匀䔀吀 䄀一匀䤀开一唀䰀䰀匀 伀一ഀഀ
GO਍匀䔀吀 儀唀伀吀䔀䐀开䤀䐀䔀一吀䤀䘀䤀䔀刀 伀一ഀഀ
GO਍䤀䘀 一伀吀 䔀堀䤀匀吀匀 ⠀匀䔀䰀䔀䌀吀 ⨀ 䘀刀伀䴀 猀礀猀⸀漀戀樀攀挀琀猀 圀䠀䔀刀䔀 漀戀樀攀挀琀开椀搀 㴀 伀䈀䨀䔀䌀吀开䤀䐀⠀一✀嬀搀戀漀崀⸀嬀䜀攀琀䄀瀀瀀䤀䐀崀✀⤀ 䄀一䐀 琀礀瀀攀 椀渀 ⠀一✀倀✀Ⰰ 一✀倀䌀✀⤀⤀ഀഀ
BEGIN਍䔀堀䔀䌀 搀戀漀⸀猀瀀开攀砀攀挀甀琀攀猀焀氀 䀀猀琀愀琀攀洀攀渀琀 㴀 一✀  䌀刀䔀䄀吀䔀 倀刀伀䌀䔀䐀唀刀䔀 嬀搀戀漀崀⸀嬀䜀攀琀䄀瀀瀀䤀䐀崀ഀഀ
	(਍ऀ䀀䄀瀀瀀一愀洀攀 䘀椀砀攀搀匀琀爀椀渀最ഀഀ
	)਍䄀匀ഀഀ
	SET NOCOUNT ON਍ऀഀഀ
	DECLARE @AppID KeyID਍ऀഀഀ
	SET @AppID = (SELECT ID FROM dbo.Applications WHERE AppName = @AppName)਍ऀഀഀ
	IF (@AppID IS NULL)਍ऀ䈀䔀䜀䤀一ഀഀ
		INSERT INTO dbo.Applications (AppName) VALUES (@AppName)਍ऀऀ匀䔀吀 䀀䄀瀀瀀䤀䐀 㴀 䀀䀀䤀䐀䔀一吀䤀吀夀ഀഀ
	END਍ऀഀഀ
	RETURN @AppID਍✀ ഀഀ
END਍䜀伀ഀഀ
SET ANSI_NULLS ON਍䜀伀ഀഀ
SET QUOTED_IDENTIFIER ON਍䜀伀ഀഀ
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[GetCounterID]') AND type in (N'P', N'PC'))਍䈀䔀䜀䤀一ഀഀ
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[GetCounterID]਍ऀ⠀ഀഀ
	@CounterName FixedString਍ऀ⤀ഀഀ
AS਍ऀ匀䔀吀 一伀䌀伀唀一吀 伀一ഀഀ
	਍ऀ䐀䔀䌀䰀䄀刀䔀 䀀䌀漀甀渀琀攀爀䤀䐀 䬀攀礀䤀䐀ഀഀ
	਍ऀ匀䔀吀 䀀䌀漀甀渀琀攀爀䤀䐀 㴀 ⠀匀䔀䰀䔀䌀吀 䤀䐀 䘀刀伀䴀 搀戀漀⸀倀攀爀昀漀爀洀愀渀挀攀䌀漀甀渀琀攀爀猀 圀䠀䔀刀䔀 䌀漀甀渀琀攀爀一愀洀攀 㴀 䀀䌀漀甀渀琀攀爀一愀洀攀⤀ഀഀ
	਍ऀ䤀䘀 ⠀䀀䌀漀甀渀琀攀爀䤀䐀 䤀匀 一唀䰀䰀⤀ഀഀ
	BEGIN਍ऀऀ䤀一匀䔀刀吀 䤀一吀伀 搀戀漀⸀倀攀爀昀漀爀洀愀渀挀攀䌀漀甀渀琀攀爀猀 ⠀䌀漀甀渀琀攀爀一愀洀攀⤀ 嘀䄀䰀唀䔀匀 ⠀䀀䌀漀甀渀琀攀爀一愀洀攀⤀ഀഀ
		SET @CounterID = @@IDENTITY਍ऀ䔀一䐀ഀഀ
	਍ऀ刀䔀吀唀刀一 䀀䌀漀甀渀琀攀爀䤀䐀ഀഀ
' ਍䔀一䐀ഀഀ
GO਍匀䔀吀 䄀一匀䤀开一唀䰀䰀匀 伀一ഀഀ
GO਍匀䔀吀 儀唀伀吀䔀䐀开䤀䐀䔀一吀䤀䘀䤀䔀刀 伀一ഀഀ
GO਍䤀䘀 一伀吀 䔀堀䤀匀吀匀 ⠀匀䔀䰀䔀䌀吀 ⨀ 䘀刀伀䴀 猀礀猀⸀漀戀樀攀挀琀猀 圀䠀䔀刀䔀 漀戀樀攀挀琀开椀搀 㴀 伀䈀䨀䔀䌀吀开䤀䐀⠀一✀嬀搀戀漀崀⸀嬀䌀爀攀愀琀攀倀攀爀昀漀爀洀愀渀挀攀䐀愀琀愀崀✀⤀ 䄀一䐀 琀礀瀀攀 椀渀 ⠀一✀倀✀Ⰰ 一✀倀䌀✀⤀⤀ഀഀ
BEGIN਍䔀堀䔀䌀 搀戀漀⸀猀瀀开攀砀攀挀甀琀攀猀焀氀 䀀猀琀愀琀攀洀攀渀琀 㴀 一✀    䌀刀䔀䄀吀䔀 倀刀伀䌀䔀䐀唀刀䔀 嬀搀戀漀崀⸀嬀䌀爀攀愀琀攀倀攀爀昀漀爀洀愀渀挀攀䐀愀琀愀崀ഀഀ
	(਍ऀऀ䀀䌀漀甀渀琀攀爀一愀洀攀 䘀椀砀攀搀匀琀爀椀渀最Ⰰഀഀ
		@MachineName FixedString,਍ऀऀ䀀䄀瀀瀀一愀洀攀 䘀椀砀攀搀匀琀爀椀渀最Ⰰഀഀ
		@IntValue bigint,਍ऀऀ䀀䐀愀琀攀吀椀洀攀匀琀愀洀瀀 搀愀琀攀琀椀洀攀ഀഀ
	)਍䄀匀ഀഀ
਍ऀ䐀䔀䌀䰀䄀刀䔀 䀀䄀瀀瀀䤀䐀 䬀攀礀䤀䐀ഀഀ
	DECLARE @MachineID KeyID਍ऀ䐀䔀䌀䰀䄀刀䔀 䀀䌀漀甀渀琀攀爀䤀䐀 䬀攀礀䤀䐀ഀഀ
਍ऀ䔀堀䔀䌀 䀀䄀瀀瀀䤀䐀 㴀 搀戀漀⸀䜀攀琀䄀瀀瀀䤀䐀 䀀䄀瀀瀀一愀洀攀ഀഀ
	EXEC @MachineID = dbo.GetMachineID @MachineName਍ऀ䔀堀䔀䌀 䀀䌀漀甀渀琀攀爀䤀䐀 㴀 搀戀漀⸀䜀攀琀䌀漀甀渀琀攀爀䤀䐀 䀀䌀漀甀渀琀攀爀一愀洀攀ഀഀ
਍ऀ䤀一匀䔀刀吀 䤀一吀伀 搀戀漀⸀倀攀爀昀漀爀洀愀渀挀攀䐀愀琀愀 ⠀ 䄀瀀瀀䤀䐀Ⰰ 䴀愀挀栀椀渀攀䤀䐀Ⰰ 䌀漀甀渀琀攀爀䤀䐀Ⰰ 䤀渀琀嘀愀氀甀攀Ⰰ 䐀愀琀攀吀椀洀攀匀琀愀洀瀀 ⤀ 嘀䄀䰀唀䔀匀 ⠀ 䀀䄀瀀瀀䤀䐀Ⰰ 䀀䴀愀挀栀椀渀攀䤀䐀Ⰰ 䀀䌀漀甀渀琀攀爀䤀䐀Ⰰ 䀀䤀渀琀嘀愀氀甀攀Ⰰ 䀀䐀愀琀攀吀椀洀攀匀琀愀洀瀀 ⤀ഀഀ
	਍ऀ刀䔀吀唀刀一ഀഀ
' ਍䔀一䐀ഀഀ
GO਍䤀䘀 一伀吀 䔀堀䤀匀吀匀 ⠀匀䔀䰀䔀䌀吀 ⨀ 䘀刀伀䴀 猀礀猀⸀昀漀爀攀椀最渀开欀攀礀猀 圀䠀䔀刀䔀 漀戀樀攀挀琀开椀搀 㴀 伀䈀䨀䔀䌀吀开䤀䐀⠀一✀嬀搀戀漀崀⸀嬀䘀䬀开䌀漀洀洀愀渀搀猀开䌀漀洀洀愀渀搀猀崀✀⤀ 䄀一䐀 瀀愀爀攀渀琀开漀戀樀攀挀琀开椀搀 㴀 伀䈀䨀䔀䌀吀开䤀䐀⠀一✀嬀搀戀漀崀⸀嬀䌀漀洀洀愀渀搀猀崀✀⤀⤀ഀഀ
ALTER TABLE [dbo].[Commands]  WITH CHECK ADD  CONSTRAINT [FK_Commands_Commands] FOREIGN KEY([ID])਍刀䔀䘀䔀刀䔀一䌀䔀匀 嬀搀戀漀崀⸀嬀䌀漀洀洀愀渀搀猀崀 ⠀嬀䤀䐀崀⤀ഀഀ
GO਍䄀䰀吀䔀刀 吀䄀䈀䰀䔀 嬀搀戀漀崀⸀嬀䌀漀洀洀愀渀搀猀崀 䌀䠀䔀䌀䬀 䌀伀一匀吀刀䄀䤀一吀 嬀䘀䬀开䌀漀洀洀愀渀搀猀开䌀漀洀洀愀渀搀猀崀ഀഀ

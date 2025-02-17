using System;
using System.Collections;
using System.IO;
using System.Windows;
using System.Windows.Forms;

namespace DVDLogParser
{
	public class DVDOpSummary
	{
		public DVDOpSummary( DVDAccessOp Op )
		{
			Count = 1;
			Filename = Op.GetFiles();
			SeekTime = Op.GetSeekTime();
			ReadTime = Op.GetReadTime();
			ActualTotalTime = Op.GetActualTotalTime();
			BlocksRead = Op.GetBlockCount();
		}

		public void Update( DVDAccessOp Op )
		{
			if( Op.GetSeekTime() != 0 )
			{
				Count++;
				SeekTime += Op.GetSeekTime();
			}
			ReadTime += Op.GetReadTime();
			ActualTotalTime += Op.GetActualTotalTime();		
			BlocksRead += Op.GetBlockCount();
		}

		public string GetFilename() { return( Filename ); }
		public long GetTotalTime() { return( ActualTotalTime / 1000 ); }
		public long GetAverageTotalTime() { return( ActualTotalTime / ( Count * 1000 ) ); }
		public long GetSeekTime() { return( SeekTime / 1000 ); }
		public long GetAverageSeekTime() { return( SeekTime / ( Count * 1000 ) ); }
		public long GetReadTime() { return( ReadTime / 1000 ); }
		public long GetAverageReadTime() { return( ReadTime / ( Count * 1000 ) ); }
		public long GetBlocksRead() { return( BlocksRead ); }

		int		Count;
		string	Filename;
		long	SeekTime;
		long	ReadTime;
		long	ActualTotalTime;
		long	BlocksRead;
	}

	public class DVDOpTracker : DictionaryBase
	{
		public DVDOpSummary this[ string key ]  
		{
			get  
			{
				return( ( DVDOpSummary )Dictionary[key] );
			}
			set  
			{
				Dictionary[key] = value;
			}
		}

		public ICollection Keys  
		{
			get  
			{
				return( Dictionary.Keys );
			}
		}

		public ICollection Values  
		{
			get  
			{
				return( Dictionary.Values );
			}
		}

		public void Add( string key, DVDOpSummary value )  
		{
			Dictionary.Add( key, value );
		}

		public bool Contains( string key )  
		{
			return( Dictionary.Contains( key ) );
		}

		public void Remove( string key )  
		{
			Dictionary.Remove( key );
		}
	}

	public class DVDAccessOp
	{
		// 0         1         2         3         4         5         6         7         8
		// 000007674   REQUEST   0036930   0000818   0   1712254         2   00037614     \WarGame\Config\Xenon\Cooked, \WarGame\Config\Xenon\Cooked\Coalesced.ini
		// 000114104   REQUEST   0000000   0007144   1   1852758        64   00007065     \WarGame\Movies\AttractMode.sfd
		public DVDAccessOp( string Line )
		{
			BlockCount = -1;
			if( Line == null )
			{
				return;
			}

			if( Line.Length < 80 )
			{
				return;
			}

			Time = int.Parse( Line.Substring( 0, 9 ) );
			EmulatedSeekTime = int.Parse( Line.Substring( 22, 7 ) );
			EmulatedReadTime = int.Parse( Line.Substring( 32, 7 ) );
			Layer = int.Parse( Line.Substring( 42, 1 ) );
			StartLBA = int.Parse( Line.Substring( 46, 7 ) );
			BlockCount = int.Parse( Line.Substring( 61, 2 ) );		// Seems to be accessed in max chunks of 64 blocks (128k)
			ActualTime = int.Parse( Line.Substring( 66, 8 ) );
			Files = Line.Substring( 79, Line.Length - 79 );
		}

		public bool IsValid() { return( BlockCount > 0 ); }
		public int GetTime() { return( Time ); }
		public int GetSeekTime() { return( EmulatedSeekTime ); }
		public int GetReadTime() { return( EmulatedReadTime ); }
		public int GetActualTotalTime() { return( ActualTime ); }
		public int GetBlockCount() { return( BlockCount ); }
		public string GetFiles() { return( Files ); }

		int			Time;				// Time of op in ms
		int			StartLBA;			// Starting logical block address
		int			BlockCount;			// Number of blocks read
		int			EmulatedSeekTime;	// Time spent seeking in us
		int			EmulatedReadTime;	// Time spent reading in us
		int			ActualTime;			// Actual seek and read time in us
		int			Layer;				// 0 or 1
		string		Files;				// Files accessed
	}

	/// <summary>
	/// Summary description for LogParser.
	/// </summary>
	public class LogParser
	{
		private	LogParserDisplay Display;
		private StreamReader LogFile = null;
		private System.Collections.Queue DVDOps;
		private DVDOpTracker OpTracker;
		private string LogFilename = "";
		private int MostRecentOp = 0;

		public LogParser( LogParserDisplay Parent )
		{
			Display = Parent;
		}
	
		public bool Construct( string Filename )
		{
			DVDOps = new Queue();
			OpTracker = new DVDOpTracker();

			try
			{
				LogFilename = Filename;
				LogFile = new StreamReader( LogFilename );
			}
			catch( System.Exception e )
			{
				Display.Log( e.Message );
			}
			return( LogFile != null );
		}

		public void Destroy()
		{
			if( LogFile != null )
			{
				LogFile.Close();
				LogFile = null;
			}
		}

		public void Reset()
		{
			Destroy();
			Construct( LogFilename );
		}

		// Parse all lines in the log file upto Time ms
		public void ParseUpto( int Time )
		{
			try
			{
				do 
				{
					DVDAccessOp Op = new DVDAccessOp( LogFile.ReadLine() );
					if( Op.IsValid() )
					{
						DVDOps.Enqueue( Op );

						MostRecentOp = Op.GetTime();
					}
				} while( MostRecentOp < Time );
			}
			catch( System.Exception e )
			{
				Display.Log( e.Message );
			}
		}

		public void ParseEntireFile()
		{
			string Line;

			ProgressBar Bar = new ProgressBar( LogFilename, ( int )LogFile.BaseStream.Length );

			do
			{
				Bar.SetValue( ( int )LogFile.BaseStream.Position );
				Line = LogFile.ReadLine();
				try
				{
					DVDAccessOp Op = new DVDAccessOp( Line );
					if( Op.IsValid() )
					{
						DVDOps.Enqueue( Op );
					}
				}
				catch( System.Exception )
				{
				}
			} while( Line != null );

			Bar.Close();
		}

		public void SummateOps()
		{
			// Sum up total access time by filename
			IEnumerator Enumerator = DVDOps.GetEnumerator();
			while( Enumerator.MoveNext() )
			{
				DVDAccessOp Op = ( DVDAccessOp )Enumerator.Current;
				if( OpTracker.Contains( Op.GetFiles() ) )
				{
					OpTracker[Op.GetFiles()].Update( Op );
				}
				else
				{
					DVDOpSummary OpSummary = new DVDOpSummary( Op );
					OpTracker.Add( Op.GetFiles(), OpSummary );
				}
			}
		}

		public ListViewItem GetListViewItem( DVDOpSummary Op )
		{
			string[] Items = 
				{
					Op.GetFilename().ToString(),
					Op.GetBlocksRead().ToString(),
					Op.GetTotalTime().ToString(),
					Op.GetSeekTime().ToString(),
					Op.GetReadTime().ToString(),
					Op.GetAverageTotalTime().ToString(),
					Op.GetAverageSeekTime().ToString(),
					Op.GetAverageReadTime().ToString()
			};

			return( new ListViewItem( Items ) );
		}

		public void CreateListView()
		{
			SummateOps();

			ICollection Files = OpTracker.Keys;
			ListViewItem[] Items = new ListViewItem[Files.Count];
			int i = 0;
			foreach( string Filename in Files )
			{
				DVDOpSummary OpSummary = OpTracker[Filename];
				Items[i] = GetListViewItem( OpSummary );
				i++;
			}

			Display.DVDLogParser_AddToListView( Items );
		}

		public void Update( int Seconds )
		{
			// Trim to only last n seconds of access - n = 20
			while( DVDOps.Count > 0 )
			{
				DVDAccessOp Op = ( DVDAccessOp )DVDOps.Dequeue();
				if( MostRecentOp - Op.GetTime() < Seconds * 1000 )
				{
					break;
				}
			}

			SummateOps();

			CreateListView();

			// Clear out temp arrays
			OpTracker.Clear();
		}
	}
}

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace UnrealSync
{
	public class UnrealSyncLogger : IDisposable
	{
		StreamWriter fileStream;
		object syncObj = new object();
		string fileName = string.Empty;

		public string FileName
		{
			get { return fileName; }
		}

		public Stream BaseStream
		{
			get { return fileStream.BaseStream; }
		}

		public object SyncObject
		{
			get { return syncObj; }
		}

		public UnrealSyncLogger(string fileName)
		{
			if(fileName == null)
			{
				throw new ArgumentNullException("fileName");
			}

			this.fileName = fileName;
			fileStream = new StreamWriter(File.Open(fileName, FileMode.Create, FileAccess.ReadWrite, FileShare.Read));
		}

		public void WriteLine(string message, params object[] parms)
		{
			lock(syncObj)
			{
				fileStream.WriteLine(message, parms);
			}
		}

		public void Flush()
		{
			lock(syncObj)
			{
				fileStream.Flush();
			}
		}
	
		#region IDisposable Members

		public void  Dispose()
		{
			lock(syncObj)
			{
				fileStream.Dispose();
			}
		}

		#endregion
}
}

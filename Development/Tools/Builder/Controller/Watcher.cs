using System;
using System.Collections;
using System.Collections.Generic;
using System.Drawing;
using System.Diagnostics;
using System.IO;

namespace Controller
{
    class Watcher
    {
        private Main Parent;

        private class WatchEntry
        {
            public WatchEntry( string InName, DateTime InTimeStamp )
            {
                Name = InName;
                TimeStamp = InTimeStamp;
            }

            public string Name;
            public DateTime TimeStamp;
        }

        private Stack<WatchEntry> WatchEntries = new Stack<WatchEntry>();

        public Watcher( Main InParent )
        {
            Parent = InParent;
        }

        public void WatchStart( string KeyName )
        {
            WatchEntry Entry = new WatchEntry( KeyName, DateTime.Now );
            WatchEntries.Push( Entry );
        }

        public int WatchStop( ref string KeyName )
        {
            int Value = -1;

            if( WatchEntries.Count > 0 )
            {
                WatchEntry Start = WatchEntries.Pop();

                KeyName = Start.Name;
                TimeSpan Duration = DateTime.Now - Start.TimeStamp;
                Value = ( int )( Duration.Ticks / 10000 );
            }

            return ( Value );
        }
    }
}


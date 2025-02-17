/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.Text;
using UnrealControls;

namespace UnrealFrontend
{
	[Serializable]
	public class PlatformGameMapCollection
	{
		SerializableDictionary<string, SerializableDictionary<string, string>> mInternalDictionary = new SerializableDictionary<string,SerializableDictionary<string,string>>();

		public SerializableDictionary<string, SerializableDictionary<string, string>> Maps
		{
			get { return mInternalDictionary; }
			set
			{
				if(value != null)
				{
					mInternalDictionary = value;
				}
			}
		}

		public bool TryGetMap(string Platform, string Game, out string Map)
		{
			if(Platform == null)
			{
				throw new ArgumentNullException("Platform");
			}

			if(Game == null)
			{
				throw new ArgumentNullException("Game");
			}

			Map = string.Empty;

			SerializableDictionary<string, string> Games;
			if(mInternalDictionary.TryGetValue(Platform, out Games))
			{
				if(Games.TryGetValue(Game, out Map))
				{
					return true;
				}
			}
			else
			{
				mInternalDictionary[Platform] = new SerializableDictionary<string, string>();
			}

			return false;
		}

		public void SetMap(string Platform, string Game, string Map)
		{
			if(Platform == null)
			{
				throw new ArgumentNullException("Platform");
			}

			if(Game == null)
			{
				throw new ArgumentNullException("Game");
			}

			if(Map == null)
			{
				throw new ArgumentNullException("Map");
			}

			SerializableDictionary<string, string> Games;
			if(!mInternalDictionary.TryGetValue(Platform, out Games))
			{
				Games = new SerializableDictionary<string, string>();
				mInternalDictionary[Platform] = Games;
			}

			Games[Game] = Map;
		}
	}
}

using System;
using System.Collections.Generic;
using System.Text;
using UnrealControls;

namespace UnrealFrontend
{
	[Serializable]
	public class PlatformGameBoolCollection
	{
		SerializableDictionary<string, SerializableDictionary<string, bool>> mInternalDictionary = new SerializableDictionary<string, SerializableDictionary<string, bool>>();

		public SerializableDictionary<string, SerializableDictionary<string, bool>> Values
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

		public bool TryGetValue(string Platform, string Game, out bool Value)
		{
			if(Platform == null)
			{
				throw new ArgumentNullException("Platform");
			}

			if(Game == null)
			{
				throw new ArgumentNullException("Game");
			}

			Value = false;

			SerializableDictionary<string, bool> Games;
			if(mInternalDictionary.TryGetValue(Platform, out Games))
			{
				if(Games.TryGetValue(Game, out Value))
				{
					return true;
				}
			}
			else
			{
				mInternalDictionary[Platform] = new SerializableDictionary<string, bool>();
			}

			return false;
		}

		public void SetValue(string Platform, string Game, bool Value)
		{
			if(Platform == null)
			{
				throw new ArgumentNullException("Platform");
			}

			if(Game == null)
			{
				throw new ArgumentNullException("Game");
			}

			SerializableDictionary<string, bool> Games;
			if(!mInternalDictionary.TryGetValue(Platform, out Games))
			{
				Games = new SerializableDictionary<string, bool>();
				mInternalDictionary[Platform] = Games;
			}

			Games[Game] = Value;
		}
	}
}

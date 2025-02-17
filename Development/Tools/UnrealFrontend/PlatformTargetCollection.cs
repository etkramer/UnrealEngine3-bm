using System;
using System.Collections.Generic;
using System.Text;
using UnrealControls;

namespace UnrealFrontend
{
	/// <summary>
	/// This class exists just so that we can store a serializable dictionary in the app settings.
	/// </summary>
	[Serializable]
	public class PlatformTargetCollection
	{
		SerializableDictionary<string, List<string>> mInternalDictionary = new SerializableDictionary<string, List<string>>();

		/// <summary>
		/// Needed for serialization.
		/// </summary>
		public SerializableDictionary<string, List<string>> Targets
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

		/// <summary>
		/// Indexer for accessing the target list.
		/// </summary>
		/// <param name="Platform">The name of the platform the targets belong to.</param>
		/// <returns>The list of targets for the specified <paramref name="Platform"/>.</returns>
		public List<string> this[string Platform]
		{
			get { return mInternalDictionary[Platform]; }
			set { mInternalDictionary[Platform] = value; }
		}

		/// <summary>
		/// Retrieves a list of targets.
		/// </summary>
		/// <param name="Platform">The owner of the targets.</param>
		/// <param name="Targets">The list of targets that belong to the specified <paramref name="Platform"/>.</param>
		/// <returns>True if the function succeeds.</returns>
		public bool TryGetTargets(string Platform, out List<string> Targets)
		{
			return mInternalDictionary.TryGetValue(Platform, out Targets);
		}
	}
}

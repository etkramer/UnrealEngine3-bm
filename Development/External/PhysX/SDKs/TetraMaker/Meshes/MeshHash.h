#ifndef MESH_HASH_H
#define MESH_HASH_H

#include <vector>
#include "math3d.h"

// ------------------------------------------------------------------------------

struct MeshHashRoot {
	int first;
	int timeStamp;
};

// ------------------------------------------------------------------------------
struct MeshHashEntry {
	int itemIndex;
	int next;
};

// ------------------------------------------------------------------------------

class MeshHash {
public: 
	MeshHash();
	~MeshHash();

	void   setGridSpacing(m3Real spacing);
	m3Real getGridSpacing() { return 1.0f / mInvSpacing; }
	void reset();
	void add(const m3Bounds &bounds, int itemIndex);
	void add(const m3Vector &pos, int itemIndex);

	void query(const m3Bounds &bounds, std::vector<int> &itemIndices, int maxIndices = -1);
	void queryUnique(const m3Bounds &bounds, std::vector<int> &itemIndices, int maxIndices = -1);

	void query(const m3Vector &pos, std::vector<int> &itemIndices, int maxIndices = -1);
	void queryUnique(const m3Vector &pos, std::vector<int> &itemIndices, int maxIndices = -1);

private:
	void compressIndices(std::vector<int> &itemIndices);
	static const int mHashIndexSize = 17011;
	m3Real mSpacing;
	m3Real mInvSpacing;
	int mTime;

	inline int hashFunction(int xi, int yi, int zi) { 
	    unsigned int h = (xi * 92837111)^(yi * 689287499)^(zi * 283923481);
	    return h % mHashIndexSize;
    }

	inline void cellCoordOf(const m3Vector &v, int &xi, int &yi, int &zi) {
		xi = (int)(v.x * mInvSpacing); if (v.x < 0.0f) xi--;
		yi = (int)(v.y * mInvSpacing); if (v.y < 0.0f) yi--;
		zi = (int)(v.z * mInvSpacing); if (v.z < 0.0f) zi--;
	}

	void quickSort(std::vector<int> &itemIndices, int l, int r);

	MeshHashRoot mHashIndex[mHashIndexSize];
	std::vector<MeshHashEntry> mEntries;
};


#endif

#ifndef WAVEFRONT_OBJ_H


#define WAVEFRONT_OBJ_H


class WavefrontObj
{
public:

  WavefrontObj(void);
  ~WavefrontObj(void);

	unsigned int loadObj(const char *fname); // load a wavefront obj returns number of triangles that were loaded.  Data is persists until the class is destructed.

  int	          mVertexCount;
  int	          mTriCount;
  int          *mIndices;
  float        *mVertices;
};

#endif

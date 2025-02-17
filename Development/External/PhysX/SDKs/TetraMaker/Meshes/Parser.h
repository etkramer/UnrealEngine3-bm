#ifndef PARSER_H
#define PARSER_H

#include "FileScanner.h"
#include "math3d.h"

typedef char ParserSymbol[FileScanner::maxLineLen];

// -------------------------------------------------------------------------
class ParserReader{
public: 
	ParserReader() {};
	~ParserReader() {};

	bool open(char *filename);
	void close();
	bool checkSymbol(ParserSymbol sym);
	void readSymbol(ParserSymbol sym);
	bool equalSymbol(ParserSymbol sym, char *expectedSym);
	bool endReached();

	bool readString(char *s, int len);
	bool readBool(bool &b);
	bool readInt(int &i);
	bool readReal(m3Real &r);
	bool readIntVector(int *v, int size);
	bool readRealVector(m3Real *v, int size);

	bool readVector(m3Vector &v);
	bool readMatrix(m3Matrix &m);
	bool readBounds(m3Bounds &b);
	bool readAffine(m3Affine &a);

	void getErrorMessage(char *msg, int len);

private:
	FileScanner scanner;
};


// -------------------------------------------------------------------------
class ParserWriter{
public: 
	ParserWriter() { f = NULL; };
	~ParserWriter() { if (f) close(); };

	bool open(char *filename);
	void close();

	void writeLn();
	void write(const char *s);
	void writeSeparator();
	void writeIndent(int indent);

	void writeString(const char *s);
	void writeBool(bool b);
	void writeInt(int i);
	void writeReal(m3Real r);
	void writeIntVector(const int *v, int size);
	void writeRealVector(const m3Real *v, int size);

	void writeVector(const m3Vector &v);
	void writeMatrix(const m3Matrix &m);
	void writeBounds(const m3Bounds &b);
	void writeAffine(const m3Affine &a);

	void writeStringVar(char *var, const char *s, int indent);
	void writeBoolVar(char *var, bool b, int indent);
	void writeIntVar(char *var, int i, int indent);
	void writeRealVar(char *var, m3Real r, int indent);
	void writeVectorVar(char *var, const m3Vector &v, int indent);
	void writeIntVectorVar(char *var, const int *v, int size, int indent);
	void writeRealVectorVar(char *var, const m3Real *v, int size, int indent);
	void writeMatrixVar(char *var, const m3Matrix &m, int indent);
	void writeBoundsVar(char *var, const m3Bounds &b, int indent);
	void writeAffineVar(char *var, const m3Affine &a, int indent);

private:
	FILE *f;
};



#endif

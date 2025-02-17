#include <string.h>
#include "Parser.h"

// ========================== Writer ==================================

//---------------------------------------------------------------------
bool ParserReader::open(char *filename)
//---------------------------------------------------------------------
{
	return scanner.open(filename);
}


//---------------------------------------------------------------------
void ParserReader::close()
//---------------------------------------------------------------------
{
	scanner.close();
}


//---------------------------------------------------------------------
void ParserReader::getErrorMessage(char *msg, int len)
//---------------------------------------------------------------------
{
	strncpy(msg, scanner.errorMessage, len);
}

//---------------------------------------------------------------------
bool ParserReader::checkSymbol(ParserSymbol sym)
//---------------------------------------------------------------------
{
	return scanner.checkSymbol(sym);
}

//---------------------------------------------------------------------
void ParserReader::readSymbol(ParserSymbol sym)
//---------------------------------------------------------------------
{
	scanner.getSymbol(sym);
}

//---------------------------------------------------------------------
bool ParserReader::endReached()
//---------------------------------------------------------------------
{
	return scanner.endReached();
}

//---------------------------------------------------------------------
bool ParserReader::equalSymbol(ParserSymbol sym, char *expectedSym)
//---------------------------------------------------------------------
{
	return FileScanner::equalSymbol(sym, expectedSym);
}

//---------------------------------------------------------------------
bool ParserReader::readString(char *s, int len) 
//---------------------------------------------------------------------
{
	char sym[FileScanner::maxLineLen];
	scanner.getSymbol(sym);
	strncpy(s, sym, len);
	return true;
}

//---------------------------------------------------------------------
bool ParserReader::readBool(bool &b) 
//---------------------------------------------------------------------
{
	char sym[FileScanner::maxLineLen];
	scanner.getSymbol(sym);
	if (FileScanner::equalSymbol(sym, "true")) {
		b = true;
		return true;
	}
	if (FileScanner::equalSymbol(sym, "false")) {
		b = false;
		return true;
	}
	scanner.setErrorMessage("true or false expected");
	return false;
}

//---------------------------------------------------------------------
bool ParserReader::readInt(int &i) 
//---------------------------------------------------------------------
{
	if (!scanner.getIntSymbol(i)) return false;
	return true;
}

//---------------------------------------------------------------------
bool ParserReader::readReal(m3Real &r) 
//---------------------------------------------------------------------
{
	if (!scanner.getFloatSymbol(r)) return false;
	return true;
}

//---------------------------------------------------------------------
bool ParserReader::readIntVector(int *v, int size)
//---------------------------------------------------------------------
{
	if (!scanner.checkSymbol("<")) return false;
	for (int i = 0; i < size; i++) {
		if (!scanner.getIntSymbol(v[i])) return false;
	}
	if (!scanner.checkSymbol(">")) return false;
	return true;
}

//---------------------------------------------------------------------
bool ParserReader::readRealVector(m3Real *v, int size)
//---------------------------------------------------------------------
{
	if (!scanner.checkSymbol("<")) return false;
	for (int i = 0; i < size; i++) {
		if (!scanner.getFloatSymbol(v[i])) return false;
	}
	if (!scanner.checkSymbol(">")) return false;
	return true;
}

//---------------------------------------------------------------------
bool ParserReader::readVector(m3Vector &v) 
//---------------------------------------------------------------------
{
	if (!scanner.checkSymbol("<")) return false;
	if (!scanner.getFloatSymbol(v.x)) return false;
	if (!scanner.getFloatSymbol(v.y)) return false;
	if (!scanner.getFloatSymbol(v.z)) return false;
	if (!scanner.checkSymbol(">")) return false;
	return true;
}

//---------------------------------------------------------------------
bool ParserReader::readMatrix(m3Matrix &m) 
//---------------------------------------------------------------------
{
	if (!scanner.checkSymbol("<")) return false;
	if (!scanner.getFloatSymbol(m.r00)) return false;
	if (!scanner.getFloatSymbol(m.r01)) return false;
	if (!scanner.getFloatSymbol(m.r02)) return false;
	if (!scanner.getFloatSymbol(m.r10)) return false;
	if (!scanner.getFloatSymbol(m.r11)) return false;
	if (!scanner.getFloatSymbol(m.r12)) return false;
	if (!scanner.getFloatSymbol(m.r20)) return false;
	if (!scanner.getFloatSymbol(m.r21)) return false;
	if (!scanner.getFloatSymbol(m.r22)) return false;
	if (!scanner.checkSymbol(">")) return false;
	return true;
}

//---------------------------------------------------------------------
bool ParserReader::readBounds(m3Bounds &b) 
//---------------------------------------------------------------------
{
	if (!scanner.checkSymbol("<")) return false;
	if (!readVector(b.min)) return false;
	if (!readVector(b.max)) return false;
	if (!scanner.checkSymbol(">")) return false;
	return true;
}

//---------------------------------------------------------------------
bool ParserReader::readAffine(m3Affine &a) 
//---------------------------------------------------------------------
{
	if (!scanner.checkSymbol("<")) return false;
	if (!readVector(a.pos)) return false;
	if (!readMatrix(a.mat)) return false;
	if (!scanner.checkSymbol(">")) return false;
	return true;
}

// ========================== Reader ==================================

//---------------------------------------------------------------------
bool ParserWriter::open(char *filename)
//---------------------------------------------------------------------
{
	if (f) close();
	f = fopen(filename, "w");
	if (!f) return false;
	return true;
}


//---------------------------------------------------------------------
void ParserWriter::close()
//---------------------------------------------------------------------
{
	if (f) fclose(f);
	f = NULL;
}

//---------------------------------------------------------------------
void ParserWriter::writeLn()
//---------------------------------------------------------------------
{
	fprintf(f, "\n");
}

//---------------------------------------------------------------------
void ParserWriter::write(const char *s)
//---------------------------------------------------------------------
{
	fprintf(f, s);
}


//---------------------------------------------------------------------
void ParserWriter::writeSeparator()
//---------------------------------------------------------------------
{
	fprintf(f, ",");
}

//---------------------------------------------------------------------
void ParserWriter::writeIndent(int indent)
//---------------------------------------------------------------------
{
	for (int i = 0; i < indent; i++) 
		fprintf(f, "\t");
}

//---------------------------------------------------------------------
void ParserWriter::writeString(const char *s)
//---------------------------------------------------------------------
{
	fprintf(f, "\"%s\"", s);
}

//---------------------------------------------------------------------
void ParserWriter::writeBool(bool b)
//---------------------------------------------------------------------
{
	if (b)
		fprintf(f, "true");
	else
		fprintf(f, "false");
}

//---------------------------------------------------------------------
void ParserWriter::writeInt(int i)
//---------------------------------------------------------------------
{
	fprintf(f, "%i", i);
}

//---------------------------------------------------------------------
void ParserWriter::writeReal(m3Real r)
//---------------------------------------------------------------------
{
	fprintf(f, "%f", r);
}

//---------------------------------------------------------------------
void ParserWriter::writeIntVector(const int *v, int size)
//---------------------------------------------------------------------
{
	fprintf(f,"<");
	for (int i = 0; i < size; i++) {
		writeInt(v[i]); 
		if (i < size-1) writeSeparator();
	}
	fprintf(f,">");
}

//---------------------------------------------------------------------
void ParserWriter::writeRealVector(const m3Real *v, int size)
//---------------------------------------------------------------------
{
	fprintf(f,"<");
	for (int i = 0; i < size; i++) {
		writeReal(v[i]); 
		if (i < size-1) writeSeparator();
	}
	fprintf(f,">");
}

//---------------------------------------------------------------------
void ParserWriter::writeVector(const m3Vector &v) 
//---------------------------------------------------------------------
{
	fprintf(f,"<");
	writeReal(v.x); writeSeparator();
	writeReal(v.y); writeSeparator();
	writeReal(v.z);
	fprintf(f,">");
}

//---------------------------------------------------------------------
void ParserWriter::writeMatrix(const m3Matrix &m) 
//---------------------------------------------------------------------
{
	fprintf(f,"<");
	writeReal(m.r00); writeSeparator();
	writeReal(m.r01); writeSeparator();
	writeReal(m.r02); writeSeparator();
	writeReal(m.r10); writeSeparator();
	writeReal(m.r11); writeSeparator();
	writeReal(m.r12); writeSeparator();
	writeReal(m.r20); writeSeparator();
	writeReal(m.r21); writeSeparator();
	writeReal(m.r22);
	fprintf(f,">");
}

//---------------------------------------------------------------------
void ParserWriter::writeBounds(const m3Bounds &b) 
//---------------------------------------------------------------------
{
	fprintf(f,"<");
	writeVector(b.min); writeSeparator();
	writeVector(b.max);
	fprintf(f,">");
}

//---------------------------------------------------------------------
void ParserWriter::writeAffine(const m3Affine &a) 
//---------------------------------------------------------------------
{
	fprintf(f,"<");
	writeVector(a.pos); writeSeparator();
	writeMatrix(a.mat);
	fprintf(f,">");
}

//---------------------------------------------------------------------
void ParserWriter::writeStringVar(char *var, const char *s, int indent)
//---------------------------------------------------------------------
{
	writeIndent(indent); fprintf(f, "%s ", var); writeString(s); writeLn();
}

//---------------------------------------------------------------------
void ParserWriter::writeBoolVar(char *var, bool b, int indent)
//---------------------------------------------------------------------
{
	writeIndent(indent); fprintf(f, "%s ", var); writeBool(b); writeLn();
}

//---------------------------------------------------------------------
void ParserWriter::writeIntVar(char *var, int i, int indent)
//---------------------------------------------------------------------
{
	writeIndent(indent); fprintf(f, "%s ", var); writeInt(i); writeLn();
}

//---------------------------------------------------------------------
void ParserWriter::writeRealVar(char *var, m3Real r, int indent)
//---------------------------------------------------------------------
{
	writeIndent(indent); fprintf(f, "%s ", var); writeReal(r); writeLn();
}


//---------------------------------------------------------------------
void ParserWriter::writeIntVectorVar(char *var, const int *v, int size, int indent)
//---------------------------------------------------------------------
{
	writeIndent(indent); fprintf(f, "%s ", var); writeIntVector(v, size); writeLn();
}

//---------------------------------------------------------------------
void ParserWriter::writeRealVectorVar(char *var, const m3Real *v, int size, int indent)
//---------------------------------------------------------------------
{
	writeIndent(indent); fprintf(f, "%s ", var); writeRealVector(v, size); writeLn();
}

//---------------------------------------------------------------------
void ParserWriter::writeVectorVar(char *var, const m3Vector &v, int indent)
//---------------------------------------------------------------------
{
	writeIndent(indent); fprintf(f, "%s ", var); writeVector(v); writeLn();
}

//---------------------------------------------------------------------
void ParserWriter::writeMatrixVar(char *var, const m3Matrix &m, int indent)
//---------------------------------------------------------------------
{
	writeIndent(indent); fprintf(f, "%s ", var); writeMatrix(m); writeLn();	
}

//---------------------------------------------------------------------
void ParserWriter::writeBoundsVar(char *var, const m3Bounds &b, int indent)
//---------------------------------------------------------------------
{
	writeIndent(indent); fprintf(f, "%s ", var); writeBounds(b); writeLn();
}

//---------------------------------------------------------------------
void ParserWriter::writeAffineVar(char *var, const m3Affine &a, int indent)
//---------------------------------------------------------------------
{
	writeIndent(indent); fprintf(f, "%s ", var); writeAffine(a); writeLn();
}



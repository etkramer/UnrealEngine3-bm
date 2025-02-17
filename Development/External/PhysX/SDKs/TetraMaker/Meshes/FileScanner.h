#ifndef FILESCANNER_H
#define FILESCANNER_H

#include <stdio.h>

#define EOL 10
#define MAX_BONDS 5

// ------------------------------------------------------------------------------------
class FileScanner {
// ------------------------------------------------------------------------------------
public:
	FileScanner(); 
	~FileScanner();

	bool open(char *filename);
	void close();

	void getSymbol(char *sym);
	bool checkSymbol(char *expectedSym, bool caseSensitive = false);
	bool getIntSymbol(int &i);
	bool getFloatSymbol(float &r);

	int  getLineNr() const { return lineNr; }
	int  getPositionNr() const { return (int)(pos - line); }

	bool endReached() const { if (feof(f)) return true; else return false; }

	void setErrorMessage(char *msg);

	char errorMessage[256];

	// ------------------------------------------------------
	static bool equalSymbol(char *sym, char *expectedSym, bool caseSensitive = false);
	static const int maxLineLen = 256;

private:
	void getNextLine();
	FILE *f;
	int  lineNr;
	char line[maxLineLen+1];
	char *pos;
};

#endif

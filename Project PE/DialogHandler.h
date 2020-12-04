#pragma once
#pragma warning(disable : 4996)
#include <iostream>
#include "framework.h"
#include "shobjidl.h"

using namespace std;

void CharToWChar(const char*, wchar_t[]);
void WCharToChar(const wchar_t*, char[]);

// ===================== FileDialog =====================

class FileDialog {
private:
	IFileOpenDialog *pFileOpen;
	PWSTR pszFilePath;
	HRESULT hr;
	unsigned char *data;
public:
	~FileDialog();
	bool open();
	unsigned char* read();
};

// ===================== TreeDialog =====================

class TreeDialog {
private:
	HWND hTree;
	TVINSERTSTRUCT TI;
	HTREEITEM tParent;
public:
	TreeDialog(HWND hWnd, HINSTANCE hInst);
	void Insert(const char* title, bool is_sub);
};


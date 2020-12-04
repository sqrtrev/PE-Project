#pragma once
#include <iostream>
#include "framework.h"
#include "shobjidl.h"
#include <map>

using namespace std;

void CharToWChar(const char*, wchar_t[]);
void WCharToChar(const wchar_t*, char[]);
class FileDialog;
class ListView;
class ListView;
class PEHandler;

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

	void Insert(const char* title, bool is_sub, bool cond = 0);
public:
	TreeDialog(HWND hWnd, HINSTANCE hInst);
	HWND getHandle();
	const char* getSelectedItem();
};

// ===================== List-View =====================

class ListView {
private:
	HWND lList;
	LV_COLUMN COL;
	LV_ITEM Li;
	unsigned short idx;
	map<string, unsigned short> li_idx;
	string before;
	map<string, string[30]> dataset;

	void Insert(const char* title, unsigned int cx);
	void setDataset();
public:
	ListView(HWND hWnd, HINSTANCE hInst);
	void Add(const string = "main");
	void Set(const string content, unsigned short row, unsigned short col);
	void Show(const string key, PEHandler* pHandler);
};

// ===================== PE Handler =====================

class PEHandler {
private:
	PIMAGE_DOS_HEADER dosHeader;
	PIMAGE_NT_HEADERS imageNTHeaders;
	PIMAGE_SECTION_HEADER sectionHeader;
	PIMAGE_SECTION_HEADER importSection;
	IMAGE_IMPORT_DESCRIPTOR* importDescriptor;
	PIMAGE_THUNK_DATA thunkData;
	DWORD thunk;
	DWORD rawOffset;
	unsigned char* data;
	
	void setHeader();
	void showDosHeader(ListView* lList);
	void showSignature(ListView* lList);
	void showImageHeader(ListView* lList);
	void showOptionalHeader(ListView* lList);
	DWORD RVAtoRAW(DWORD RVA);
public:
	PEHandler();
	~PEHandler();
	void setData(unsigned char* data);
	void readPE();
	void Show(const string key, ListView* lList);
};
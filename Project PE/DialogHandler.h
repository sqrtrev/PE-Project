#pragma once
#include <iostream>
#include "framework.h"
#include "shobjidl.h"
#include <map>

using namespace std;

void CharToWChar(const char*, wchar_t[]);
void WCharToChar(const wchar_t*, char[]);
class FileDialog;
class TreeDialog;
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
public:
	TreeDialog(HWND hWnd, HINSTANCE hInst);
	void Insert(const string title, bool is_sub, bool cond = 0);
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
	map<string, string[100]> dataset;

	void Insert(const char* title, unsigned int cx);
	void setDataset();
public:
	ListView(HWND hWnd, HINSTANCE hInst);
	void Add(const string = "main");
	void Set(const string content, unsigned short row, unsigned short col);
	void Show(const string key, PEHandler* pHandler);
	void setSectionDataset(const string key);
};

// ===================== PE Handler =====================

class PEHandler {
private:
	PIMAGE_DOS_HEADER dosHeader;
	PIMAGE_NT_HEADERS imageNTHeaders;
	PIMAGE_SECTION_HEADER sectionHeader;
	PIMAGE_SECTION_HEADER importSection;
	unsigned char* data;
	string machine;
	
	void setHeader();
	void showDosHeader(ListView* lList);
	void showSignature(ListView* lList);
	void showImageHeader(ListView* lList);
	void showOptionalHeader(ListView* lList);
	void showSectionHeader(ListView* lList, const string key);
public:
	PEHandler();
	~PEHandler();
	void setData(unsigned char* data);
	void readPE(ListView* lList, TreeDialog* tDialog);
	void Show(const string key, ListView* lList);
};
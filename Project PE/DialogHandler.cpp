#include "DialogHandler.h"
#include <sstream>

// #define NAMEOF(variable) ((decltype(&variable))nullptr, #variable)
static bool isLoad = FALSE;

void CharToWChar(const char* pstrSrc, wchar_t pwstrDest[]) {
	int nLen = (int)strlen(pstrSrc) + 1;
	mbstowcs(pwstrDest, pstrSrc, nLen);
}

void WCharToChar(const wchar_t* pwstrSrc, char pstrDest[]) {
	int nLen = (int)wcslen(pwstrSrc);
	wcstombs(pstrDest, pwstrSrc, nLen + 1);
}

const string toHex(DWORD s) {
	stringstream stream;

	stream << hex << s;

	return stream.str();
}

FileDialog::~FileDialog() {
	if (data != NULL)
		delete data;
	CoUninitialize();
}

bool FileDialog::open() {
	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(hr))
		return FALSE;
	hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
	if (FAILED(hr))
		return FALSE;
	hr = pFileOpen->Show(NULL);
	if (SUCCEEDED(hr)) {
		IShellItem *pItem;
		hr = pFileOpen->GetResult(&pItem);
		if (SUCCEEDED(hr)) {
			hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
		}
		pItem->Release();
	}
	pFileOpen->Release();
	if (FAILED(hr)) {
		return FALSE;
	}
	return TRUE;
}

unsigned char* FileDialog::read() {
	FILE* stream = _wfopen(pszFilePath, L"rb");
	unsigned long size;
	unsigned int totnum = 0, curnum = 0;

	fseek(stream, 0, SEEK_END);
	size = ftell(stream);
	fseek(stream, 0, SEEK_SET);

	if (data != NULL) free(data);

	data = (unsigned char *)malloc(size);

	while ((curnum = (unsigned int)fread(&data[totnum], 1, size - totnum, stream))) {
		totnum += curnum;
	}

	fclose(stream);

	return data;
}

void TreeDialog::Insert(string title, bool is_sub, bool cond ) {
	wchar_t tmp[256];
	CharToWChar(title.c_str(), tmp);

	if (is_sub) {
		TI.hParent = tParent;
	} else {
		TI.hInsertAfter = TVI_LAST;
		TI.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		TI.hParent = 0;
	}
	TI.item.pszText = tmp;
	if (cond) {
		SendMessage(hTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)&TI);
		return;
	}
	tParent = (HTREEITEM)SendMessage(hTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)&TI);
}

TreeDialog::TreeDialog(HWND hWnd, HINSTANCE hInst) {
	hTree = CreateWindow(WC_TREEVIEW, L"", WS_CHILD | WS_VISIBLE | WS_BORDER |
		TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | WS_HSCROLL | WS_VSCROLL,
		10, 10, 250, 680, hWnd, NULL, hInst, NULL);

	/*
	this->Insert("IMAGE_SECTION_HEADER .text", 0);
	this->Insert("IMAGE_SECTION_HEADER .data", 0);
	this->Insert("IMAGE_SECTION_HEADER .rsrc", 0);
	this->Insert("BOUND IMPORT Directory Table", 0);
	this->Insert("BOUND IMPORT DLL Names", 0);
	this->Insert("SECTION .text", 0);
	this->Insert("IMPORT Address Table", 1, 1);
	this->Insert("IMAGE_DEBUG_DIRECTORY", 1, 1);
	this->Insert("IMAGE_LOAD_CONFIG_DIRECTORY", 1, 1);
	this->Insert("IMAGE_DEBUG_TYPE_CODEVERSION", 1, 1);
	this->Insert("IMPORT Directory Table", 1, 1);
	this->Insert("IMPORT Name Table", 1, 1);
	this->Insert("IMPORT Hints/Names & DLL Name", 1);
	this->Insert("SECTION .data", 0);
	*/
}

HWND TreeDialog::getHandle() {
	return hTree;
}

const char* TreeDialog::getSelectedItem() {
	static char res[256];
	wchar_t tmp[256];
	HTREEITEM hSelectedItem = TreeView_GetSelection(hTree);
	if (hSelectedItem == NULL)
		return NULL;
	TVITEM item;
	item.hItem = hSelectedItem;
	item.mask = TVIF_TEXT;
	item.cchTextMax = 128;
	item.pszText = tmp;
	if (TreeView_GetItem(hTree, &item)) {
		// MessageBox(NULL, item.pszText, L"Selected", MB_OK);
		WCharToChar(item.pszText, res);
		return res;
	}
	return NULL;
}


ListView::ListView(HWND hWnd, HINSTANCE hInst) {
	idx = 0;
	lList = CreateWindow(WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | WS_BORDER |
		LVS_REPORT | LVS_SHOWSELALWAYS | WS_HSCROLL | WS_VSCROLL,
		280, 10, 900, 680, hWnd, NULL, hInst, NULL);

	COL.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	COL.fmt = LVCFMT_LEFT;

	wchar_t tmp[256];
	CharToWChar("test", tmp);

	setDataset();
	
	this->Insert("Description", 650);
	this->Insert("Data", 250);
	//this->Insert("Offset", 150);

	// Default Li Setting
	Li.mask = LVIF_TEXT;
	Li.state = 0;
	Li.stateMask = 0;
	Li.iImage = 0;
	Li.iSubItem = 0;
	Li.iItem = 0;

	before = "main";

	this->Add("main");
	/*
	this->Set("sqrtrev", 0, 0);
	this->Set("sqrtrev2", 0, 1);
	this->Set("sqrtrev3", 0, 2);
	*/
}

void ListView::Add(const string key) {
	for(int t = 0; t < li_idx[key]; t++)
		ListView_InsertItem(lList, &Li);
}

void ListView::Set(const string content, unsigned short row, unsigned short col) {
	wchar_t tmp[256];
	CharToWChar(content.c_str(), tmp);

	ListView_SetItemText(lList, row, col, tmp);
}

void ListView::Show(const string key, PEHandler* pHandler) {
	ListView_DeleteAllItems(lList);
	Add(key);

	for (int i = 0; i < li_idx[key]; i++) {
		Set(dataset[key][i], i, 1);
	}
	pHandler->Show(key, this);

	before = key;
}

void ListView::Insert(const char* title, unsigned int cx) {
	wchar_t tmp[256];
	CharToWChar(title, tmp);

	COL.cx = cx;
	COL.pszText = tmp;
	COL.iSubItem = idx++;
	ListView_InsertColumn(lList, 0, &COL);
}

void ListView::setDataset() {
	unsigned short idx = 0;

	dataset["IMAGE_DOS_HEADER"][idx++] = "Magic Number";
	dataset["IMAGE_DOS_HEADER"][idx++] = "Bytes on last page of file";
	dataset["IMAGE_DOS_HEADER"][idx++] = "Pages in file";
	dataset["IMAGE_DOS_HEADER"][idx++] = "Relocartions";
	dataset["IMAGE_DOS_HEADER"][idx++] = "Size of header in paragraphs";
	dataset["IMAGE_DOS_HEADER"][idx++] = "Minimum extra paragraphs needed";
	dataset["IMAGE_DOS_HEADER"][idx++] = "Maximum extra paragraphs needed";
	dataset["IMAGE_DOS_HEADER"][idx++] = "Initial (relative) SS value";
	dataset["IMAGE_DOS_HEADER"][idx++] = "Initial SP value";
	dataset["IMAGE_DOS_HEADER"][idx++] = "Checksum";
	dataset["IMAGE_DOS_HEADER"][idx++] = "Initial IP value";
	dataset["IMAGE_DOS_HEADER"][idx++] = "Initial (relative) CS value";
	dataset["IMAGE_DOS_HEADER"][idx++] = "File address of relocation table";
	dataset["IMAGE_DOS_HEADER"][idx++] = "Overlay number";
	dataset["IMAGE_DOS_HEADER"][idx++] = "OEM identifier (for e_oeminfo)";
	dataset["IMAGE_DOS_HEADER"][idx++] = "OEM information; e_oemid specific";
	dataset["IMAGE_DOS_HEADER"][idx++] = "File address of new exe header";
	li_idx["IMAGE_DOS_HEADER"] = idx;
	idx = 0;

	dataset["Signature"][idx++] = "Signature";
	li_idx["Signature"] = idx;
	idx = 0;


	dataset["IMAGE_FILE_HEADER"][idx++] = "Machine";
	dataset["IMAGE_FILE_HEADER"][idx++] = "Number of Sections";
	dataset["IMAGE_FILE_HEADER"][idx++] = "Time Stamp";
	dataset["IMAGE_FILE_HEADER"][idx++] = "Pointer to Symbol Table";
	dataset["IMAGE_FILE_HEADER"][idx++] = "Number of Symbols";
	dataset["IMAGE_FILE_HEADER"][idx++] = "Size of Optional Header";
	dataset["IMAGE_FILE_HEADER"][idx++] = "Characteristics";
	li_idx["IMAGE_FILE_HEADER"] = idx;
	idx = 0;

	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Magic";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Magor Linker Version ";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Minor Linker Version ";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Size Of Code";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Size Of InitializedData";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Size Of Uninitialized Data";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Address Of EntryPoint";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Base Of Code";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "ImageBase";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Section Alignment";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "FileAlignment";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Major Operating System Version";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Minor Operating System Version";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Major Image Version";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Minor Image Version";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Major Subsystem Version";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Minor Subsystem Version";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Win32 Version Value";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Size Of Image";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Size Of Headers";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "CheckSum";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Subsystem ";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Dll Characteristics ";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Size Of Stack Reserve ";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Size Of Stack Commit ";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Size Of Heap Reserve ";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Size Of Heap Commit";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Loader Flags ";
	dataset["IMAGE_OPTIONAL_HEADER"][idx++] = "Number Of Rva And Sizes ";
	li_idx["IMAGE_OPTIONAL_HEADER"] = idx;
	idx = 0;

}

void ListView::setSectionDataset(const string key) {
	unsigned short idx = 0;

	dataset[key][idx++] = "Virtual Size";
	dataset[key][idx++] = "Virtual Address";
	dataset[key][idx++] = "Size Of Raw Data";
	dataset[key][idx++] = "Pointer To Raw Data";
	dataset[key][idx++] = "Pointer To Relocations";
	dataset[key][idx++] = "Pointer To Line Numbers";
	dataset[key][idx++] = "Number Of Relocations";
	dataset[key][idx++] = "Number Of Line Numbers";
	dataset[key][idx++] = "Characteristics";
	li_idx[key] = idx;
}

PEHandler::PEHandler() {
	dosHeader = {};
	imageNTHeaders = {};
	sectionHeader = {};
	importSection = {};
}

PEHandler::~PEHandler() {
	data = NULL;
}

void PEHandler::setData(unsigned char* data) {
	this->data = data;
}

void PEHandler::readPE(ListView* lList, TreeDialog* tDialog) {
	TreeView_DeleteAllItems(tDialog->getHandle());
	this->setHeader();
	if (::isLoad) {
		tDialog->Insert("IMAGE_DOS_HEADER", 0);
		tDialog->Insert("IMAGE_NT_HEADERS", 0);
		tDialog->Insert("Signature", 1, 1);
		tDialog->Insert("IMAGE_FILE_HEADER", 1, 1);
		tDialog->Insert("IMAGE_OPTIONAL_HEADER", 1);
		for (int i = 0; i < imageNTHeaders->FileHeader.NumberOfSections; i++) {
			string tmp = (char*)sectionHeader[i].Name;
			tDialog->Insert("Section "+tmp, 0);
			lList->setSectionDataset("Section " + tmp);
			lList->Add("Section " + tmp);
		}
	}
}

void PEHandler::Show(const string key, ListView* lList) {
	if (!::isLoad) return;

	if (key == "IMAGE_DOS_HEADER") {
		showDosHeader(lList);
	}
	else if(key == "Signature"){
		showSignature(lList);
	}
	else if (key == "IMAGE_FILE_HEADER") {
		showImageHeader(lList);
	}
	else if (key == "IMAGE_OPTIONAL_HEADER") {
		showOptionalHeader(lList);
	}
	else if (key.find("Section") != string::npos) {
		showSectionHeader(lList, key);
	}
}

void PEHandler::showDosHeader(ListView* lList) {
	unsigned short idx = 0;

	lList->Set(toHex(dosHeader->e_magic), idx++, 0);
	lList->Set(toHex(dosHeader->e_cblp), idx++, 0);
	lList->Set(toHex(dosHeader->e_cp), idx++, 0);
	lList->Set(toHex(dosHeader->e_crlc), idx++, 0);
	lList->Set(toHex(dosHeader->e_cparhdr), idx++, 0);
	lList->Set(toHex(dosHeader->e_minalloc), idx++, 0);
	lList->Set(toHex(dosHeader->e_maxalloc), idx++, 0);
	lList->Set(toHex(dosHeader->e_ss), idx++, 0);
	lList->Set(toHex(dosHeader->e_sp), idx++, 0);
	lList->Set(toHex(dosHeader->e_csum), idx++, 0);
	lList->Set(toHex(dosHeader->e_ip), idx++, 0);
	lList->Set(toHex(dosHeader->e_cs), idx++, 0);
	lList->Set(toHex(dosHeader->e_lfarlc), idx++, 0);
	lList->Set(toHex(dosHeader->e_ovno), idx++, 0);
	lList->Set(toHex(dosHeader->e_oemid), idx++, 0);
	lList->Set(toHex(dosHeader->e_oeminfo), idx++, 0);
	lList->Set(toHex(dosHeader->e_lfanew), idx++, 0);
}

void PEHandler::showSignature(ListView * lList) {
	lList->Set(toHex(imageNTHeaders->Signature), 0, 0);
}

void PEHandler::showImageHeader(ListView * lList) {
	unsigned short idx = 0;
	const string machine = toHex(imageNTHeaders->FileHeader.Machine);

	lList->Set(this->machine, idx++, 0);
	lList->Set(toHex(imageNTHeaders->FileHeader.NumberOfSections), idx++, 0);
	lList->Set(toHex(imageNTHeaders->FileHeader.TimeDateStamp), idx++, 0);
	lList->Set(toHex(imageNTHeaders->FileHeader.PointerToSymbolTable), idx++, 0);
	lList->Set(toHex(imageNTHeaders->FileHeader.NumberOfSymbols), idx++, 0);
	lList->Set(toHex(imageNTHeaders->FileHeader.SizeOfOptionalHeader), idx++, 0);
	lList->Set(toHex(imageNTHeaders->FileHeader.Characteristics), idx, 0);
}

void PEHandler::showOptionalHeader(ListView * lList) {
	unsigned short idx = 0;

	lList->Set(toHex(imageNTHeaders->OptionalHeader.Magic), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.MajorLinkerVersion), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.MinorLinkerVersion), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.SizeOfCode), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.SizeOfInitializedData), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.SizeOfUninitializedData), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.AddressOfEntryPoint), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.BaseOfCode), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.ImageBase), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.SectionAlignment), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.FileAlignment), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.MajorOperatingSystemVersion), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.MinorOperatingSystemVersion), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.MajorImageVersion), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.MinorImageVersion), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.MajorSubsystemVersion), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.MinorSubsystemVersion), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.Win32VersionValue), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.SizeOfImage), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.SizeOfHeaders), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.CheckSum), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.Subsystem), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.DllCharacteristics), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.SizeOfStackReserve), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.SizeOfStackCommit), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.SizeOfHeapReserve), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.SizeOfHeapCommit), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.LoaderFlags), idx++, 0);
	lList->Set(toHex(imageNTHeaders->OptionalHeader.NumberOfRvaAndSizes), idx, 0);
}

void PEHandler::showSectionHeader(ListView * lList, const string key) {
	unsigned short idx = 0;

	for (int i = 0; i < imageNTHeaders->FileHeader.NumberOfSections; i++) {
		if (key.find((char*)sectionHeader[i].Name) != string::npos) {
			lList->Set(toHex(sectionHeader[i].Misc.VirtualSize), idx++, 0);
			lList->Set(toHex(sectionHeader[i].VirtualAddress), idx++, 0);
			lList->Set(toHex(sectionHeader[i].SizeOfRawData), idx++, 0);
			lList->Set(toHex(sectionHeader[i].PointerToRawData), idx++, 0);
			lList->Set(toHex(sectionHeader[i].PointerToRelocations), idx++, 0);
			lList->Set(toHex(sectionHeader[i].PointerToLinenumbers), idx++, 0);
			lList->Set(toHex(sectionHeader[i].NumberOfRelocations), idx++, 0);
			lList->Set(toHex(sectionHeader[i].NumberOfLinenumbers), idx++, 0);
			lList->Set(toHex(sectionHeader[i].Characteristics), idx++, 0);
			break;
		}
	}
}

void PEHandler::setHeader() {
	wchar_t tmp[3];
	char tmp2[3];

	dosHeader = (PIMAGE_DOS_HEADER)data;
	memcpy_s(tmp2, sizeof(tmp2), &dosHeader->e_magic, sizeof(dosHeader->e_magic));
	tmp2[2] = NULL;
	CharToWChar(tmp2, tmp);

	if (strcmp("MZ", tmp2)) {
		MessageBox(NULL, L"Not a PE File or Error Occured", L"DOS Header", MB_OK);
		dosHeader = {};
		return;
	}

	imageNTHeaders = (PIMAGE_NT_HEADERS)(data + dosHeader->e_lfanew);
	sectionHeader = (PIMAGE_SECTION_HEADER)malloc(sizeof(IMAGE_SECTION_HEADER) * imageNTHeaders->FileHeader.NumberOfSections);
	DWORD sectionLocation = (DWORD)imageNTHeaders + sizeof(DWORD) + (DWORD)(sizeof(IMAGE_FILE_HEADER)) + (DWORD)imageNTHeaders->FileHeader.SizeOfOptionalHeader;
	DWORD importDirectoryRVA = imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

	for (int i = 0; i < imageNTHeaders->FileHeader.NumberOfSections; i++) {
		sectionHeader[i] = *(PIMAGE_SECTION_HEADER)sectionLocation;
		if (importDirectoryRVA >= sectionHeader[i].VirtualAddress && importDirectoryRVA < sectionHeader[i].VirtualAddress + sectionHeader[i].Misc.VirtualSize) {
			importSection = &sectionHeader[i];
		}
		sectionLocation += sizeof(IMAGE_SECTION_HEADER);
	}

	const string machine = toHex(imageNTHeaders->FileHeader.Machine);
	if (machine == "14c")
		this->machine = "x86";
	else if (machine == "2000" or machine == "8664")
		this->machine = "x64";
	else
		this->machine = "Unknown";

	::isLoad = TRUE;
}

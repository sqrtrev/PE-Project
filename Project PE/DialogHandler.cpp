#include "DialogHandler.h"

void CharToWChar(const char* pstrSrc, wchar_t pwstrDest[]) {
	int nLen = (int)strlen(pstrSrc) + 1;
	mbstowcs(pwstrDest, pstrSrc, nLen);
}

void WCharToChar(const wchar_t* pwstrSrc, char pstrDest[]) {
	int nLen = (int)wcslen(pwstrSrc);
	wcstombs(pstrDest, pwstrSrc, nLen + 1);
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

void TreeDialog::Insert(const char* title, bool is_sub) {
	wchar_t tmp[256];
	CharToWChar(title, tmp);

	if (is_sub) {
		TI.hParent = tParent;
	} else {
		TI.hInsertAfter = TVI_LAST;
		TI.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		TI.hParent = 0;
	}
	TI.item.pszText = tmp;
	tParent = (HTREEITEM)SendMessage(hTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)&TI);
}

TreeDialog::TreeDialog(HWND hWnd, HINSTANCE hInst) {
	hTree = CreateWindow(WC_TREEVIEW, L"", WS_CHILD | WS_VISIBLE | WS_BORDER |
		TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | WS_HSCROLL | WS_VSCROLL,
		10, 10, 250, 680, hWnd, NULL, hInst, NULL);
}

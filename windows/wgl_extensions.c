#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "wgl_extensions.h"

#if defined(_WIN32)

#ifdef _MSC_VER
#pragma warning(disable: 4055)
#pragma warning(disable: 4054)
#endif

static int TestPointer(const PROC pTest)
{
	ptrdiff_t iTest;
	if(!pTest) return 0;
	iTest = (ptrdiff_t)pTest;
	
	if(iTest == 1 || iTest == 2 || iTest == 3 || iTest == -1) return 0;
	
	return 1;
}

static PROC WinGetProcAddress(const char *name)
{
	HMODULE glMod = NULL;
	PROC pFunc = wglGetProcAddress((LPCSTR)name);
	if(TestPointer(pFunc))
	{
		return pFunc;
	}
	glMod = GetModuleHandleA("OpenGL32.dll");
	return (PROC)GetProcAddress(glMod, (LPCSTR)name);
}
	
#define IntGetProcAddress(name) WinGetProcAddress(name)
#endif // defined(_WIN32)

int wgl_ext_ARB_create_context = wgl_LOAD_FAILED;
int wgl_ext_EXT_swap_control = wgl_LOAD_FAILED;

HGLRC (CODEGEN_FUNCPTR *_ptrc_wglCreateContextAttribsARB)(HDC, HGLRC, const int *) = NULL;

static int Load_ARB_create_context()
{
	int numFailed = 0;
	_ptrc_wglCreateContextAttribsARB = (HGLRC (CODEGEN_FUNCPTR *)(HDC, HGLRC, const int *))IntGetProcAddress("wglCreateContextAttribsARB");
	if(!_ptrc_wglCreateContextAttribsARB) numFailed++;
	return numFailed;
}

int (CODEGEN_FUNCPTR *_ptrc_wglGetSwapIntervalEXT)() = NULL;
BOOL (CODEGEN_FUNCPTR *_ptrc_wglSwapIntervalEXT)(int) = NULL;

static int Load_EXT_swap_control()
{
	int numFailed = 0;
	_ptrc_wglGetSwapIntervalEXT = (int (CODEGEN_FUNCPTR *)())IntGetProcAddress("wglGetSwapIntervalEXT");
	if(!_ptrc_wglGetSwapIntervalEXT) numFailed++;
	_ptrc_wglSwapIntervalEXT = (BOOL (CODEGEN_FUNCPTR *)(int))IntGetProcAddress("wglSwapIntervalEXT");
	if(!_ptrc_wglSwapIntervalEXT) numFailed++;
	return numFailed;
}


static const char * (CODEGEN_FUNCPTR *_ptrc_wglGetExtensionsStringARB)(HDC) = NULL;

typedef int (*PFN_LOADFUNCPOINTERS)();
typedef struct wgl_StrToExtMap_s
{
	char *extensionName;
	int *extensionVariable;
	PFN_LOADFUNCPOINTERS LoadExtension;
} wgl_StrToExtMap;

static wgl_StrToExtMap ExtensionMap[2] = {
	{"WGL_ARB_create_context", &wgl_ext_ARB_create_context, Load_ARB_create_context},
	{"WGL_EXT_swap_control", &wgl_ext_EXT_swap_control, Load_EXT_swap_control},
};

static int g_extensionMapSize = 2;

static wgl_StrToExtMap *FindExtEntry(const char *extensionName)
{
	int loop;
	wgl_StrToExtMap *currLoc = ExtensionMap;
	for(loop = 0; loop < g_extensionMapSize; ++loop, ++currLoc)
	{
		if(strcmp(extensionName, currLoc->extensionName) == 0)
			return currLoc;
	}
	
	return NULL;
}

static void ClearExtensionVars()
{
	wgl_ext_ARB_create_context = wgl_LOAD_FAILED;
	wgl_ext_EXT_swap_control = wgl_LOAD_FAILED;
}


static void LoadExtByName(const char *extensionName)
{
	wgl_StrToExtMap *entry = NULL;
	entry = FindExtEntry(extensionName);
	if(entry)
	{
		if(entry->LoadExtension)
		{
			int numFailed = entry->LoadExtension();
			if(numFailed == 0)
			{
				*(entry->extensionVariable) = wgl_LOAD_SUCCEEDED;
			}
			else
			{
				*(entry->extensionVariable) = wgl_LOAD_SUCCEEDED + numFailed;
			}
		}
		else
		{
			*(entry->extensionVariable) = wgl_LOAD_SUCCEEDED;
		}
	}
}


static void ProcExtsFromExtString(const char *strExtList)
{
	size_t iExtListLen = strlen(strExtList);
	const char *strExtListEnd = strExtList + iExtListLen;
	const char *strCurrPos = strExtList;
	char strWorkBuff[256];

	while(*strCurrPos)
	{
		/*Get the extension at our position.*/
		int iStrLen = 0;
		const char *strEndStr = strchr(strCurrPos, ' ');
		int iStop = 0;
		if(strEndStr == NULL)
		{
			strEndStr = strExtListEnd;
			iStop = 1;
		}

		iStrLen = (int)((ptrdiff_t)strEndStr - (ptrdiff_t)strCurrPos);

		if(iStrLen > 255)
			return;

		strncpy(strWorkBuff, strCurrPos, iStrLen);
		strWorkBuff[iStrLen] = '\0';

		LoadExtByName(strWorkBuff);

		strCurrPos = strEndStr + 1;
		if(iStop) break;
	}
}

int wgl_LoadFunctions(HDC hdc)
{
	ClearExtensionVars();
	
	_ptrc_wglGetExtensionsStringARB = (const char * (CODEGEN_FUNCPTR *)(HDC))IntGetProcAddress("wglGetExtensionsStringARB");
	if(!_ptrc_wglGetExtensionsStringARB) return wgl_LOAD_FAILED;
	
	ProcExtsFromExtString((const char *)_ptrc_wglGetExtensionsStringARB(hdc));
	return wgl_LOAD_SUCCEEDED;
}


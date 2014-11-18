#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "glx_extensions.h"

#include <GL/glx.h>

#define IntGetProcAddress(name) (*glXGetProcAddressARB)((const GLubyte*)name)

int glx_ext_ARB_create_context = glx_LOAD_FAILED;
int glx_ext_EXT_swap_control = glx_LOAD_FAILED;

GLXContext (CODEGEN_FUNCPTR *_ptrc_glXCreateContextAttribsARB)(Display *, GLXFBConfig, GLXContext, Bool, const int *) = NULL;

static int Load_ARB_create_context()
{
	int numFailed = 0;
	_ptrc_glXCreateContextAttribsARB = (GLXContext (CODEGEN_FUNCPTR *)(Display *, GLXFBConfig, GLXContext, Bool, const int *))IntGetProcAddress("glXCreateContextAttribsARB");
	if(!_ptrc_glXCreateContextAttribsARB) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *_ptrc_glXSwapIntervalEXT)(Display *, GLXDrawable, int) = NULL;

static int Load_EXT_swap_control()
{
	int numFailed = 0;
	_ptrc_glXSwapIntervalEXT = (void (CODEGEN_FUNCPTR *)(Display *, GLXDrawable, int))IntGetProcAddress("glXSwapIntervalEXT");
	if(!_ptrc_glXSwapIntervalEXT) numFailed++;
	return numFailed;
}

typedef int (*PFN_LOADFUNCPOINTERS)();
typedef struct glx_StrToExtMap_s
{
	char *extensionName;
	int *extensionVariable;
	PFN_LOADFUNCPOINTERS LoadExtension;
} glx_StrToExtMap;

static glx_StrToExtMap ExtensionMap[2] = {
	{"GLX_ARB_create_context", &glx_ext_ARB_create_context, Load_ARB_create_context},
	{"GLX_EXT_swap_control", &glx_ext_EXT_swap_control, Load_EXT_swap_control},
};

static int g_extensionMapSize = 2;

static glx_StrToExtMap *FindExtEntry(const char *extensionName)
{
	int loop;
	glx_StrToExtMap *currLoc = ExtensionMap;
	for(loop = 0; loop < g_extensionMapSize; ++loop, ++currLoc)
	{
		if(strcmp(extensionName, currLoc->extensionName) == 0)
			return currLoc;
	}
	
	return NULL;
}

static void ClearExtensionVars()
{
	glx_ext_ARB_create_context = glx_LOAD_FAILED;
	glx_ext_EXT_swap_control = glx_LOAD_FAILED;
}


static void LoadExtByName(const char *extensionName)
{
	glx_StrToExtMap *entry = NULL;
	entry = FindExtEntry(extensionName);
	if(entry)
	{
		if(entry->LoadExtension)
		{
			int numFailed = entry->LoadExtension();
			if(numFailed == 0)
			{
				*(entry->extensionVariable) = glx_LOAD_SUCCEEDED;
			}
			else
			{
				*(entry->extensionVariable) = glx_LOAD_SUCCEEDED + numFailed;
			}
		}
		else
		{
			*(entry->extensionVariable) = glx_LOAD_SUCCEEDED;
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

int glx_LoadFunctions(Display *display, int screen)
{
	ClearExtensionVars();
	
	
	ProcExtsFromExtString((const char *)glXQueryExtensionsString(display, screen));
	return glx_LOAD_SUCCEEDED;
}


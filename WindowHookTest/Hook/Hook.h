// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the HOOK_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// HOOK_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef HOOK_EXPORTS
#define HOOK_API __declspec(dllexport)
#else
#define HOOK_API __declspec(dllimport)
#endif

// This class is exported from the Hook.dll
class HOOK_API CHook {
public:
	CHook(void);
	// TODO: add your methods here.
};

//extern HOOK_API int nHook;
extern "C"
{
HOOK_API LRESULT HookProc(int nCode, WPARAM wParam, LPARAM lParam);
HOOK_API LRESULT KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
}

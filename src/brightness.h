#pragma once


#include <iostream>
#include <string>
#include <windows.h>
#include <objbase.h>
#include <wbemidl.h>
#include <comdef.h>
#include <mutex>

using namespace std;

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "comsuppw.lib")

LPCWSTR stringToLPCWSTR(std::string orig);

int Init();
int GetBrightness();
int SetBrightness(int brightness);
void Cleanup();


void IncreaseBrightness();
void ReduceBrightness();

LRESULT __stdcall WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

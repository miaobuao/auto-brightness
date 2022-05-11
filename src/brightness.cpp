#include "brightness.h"

LPCWSTR stringToLPCWSTR(std::string orig)
{
    size_t origsize = orig.length() + 1;
    const size_t newsize = 100;
    size_t convertedChars = 0;
    wchar_t* wcstring = (wchar_t*)malloc(sizeof(wchar_t) * (orig.length() - 1));
    mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);

    return wcstring;
}

// wmi namespace
IWbemLocator* wmiLocator = NULL;
IWbemServices* wmiNamespace = 0;
BSTR wmiPath = SysAllocString(L"root\\wmi");
HRESULT hr = S_OK;

// brightnessAdjustmentClass
IWbemClassObject* brightnessAdjustmentClass = NULL;
IWbemClassObject* brightnessAdjustmentClassInstant = NULL;
IWbemClassObject* brightnessAdjustmentClassObject = NULL;
IWbemClassObject* brightnessAdjustmentMethodInstant = NULL;
IEnumWbemClassObject* brightnessAdjustmentClassEnum = NULL;

BSTR brightnessAdjustmentClassPath = SysAllocString(L"WmiMonitorBrightnessMethods");
BSTR brightnessAdjustmentMethodName = SysAllocString(L"WmiSetBrightness");
BSTR brightnessAdjustmentMethodArgName1 = SysAllocString(L"Timeout");
BSTR brightnessAdjustmentMethodArgName2 = SysAllocString(L"Brightness");
BSTR brightnessAdjustmentClassQuery = SysAllocString(L"Select * from WmiMonitorBrightnessMethods");


// brightnessClass
IEnumWbemClassObject* brightnessClassEnum = NULL;
//IWbemClassObject *brightnessClass = NULL;
IWbemClassObject* brightnessClassObject = NULL;

BSTR brightnessClassPath = SysAllocString(L"WmiMonitorBrightness");
BSTR brightnessVariableName = SysAllocString(L"CurrentBrightness");
BSTR brightnessQuery = SysAllocString(L"Select * from WmiMonitorBrightness");

bool init_flag = false;

int Init()
{
    init_flag = true;
    // Initialize COM and connect up to CIMOM  
    hr = CoInitialize(0);
    if (FAILED(hr))
        return -1;
    hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
    if (FAILED(hr))
        return -1;
    hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&wmiLocator);
    if (FAILED(hr))
        return -1;
    hr = wmiLocator->ConnectServer(wmiPath, NULL, NULL, NULL, 0, NULL, NULL, &wmiNamespace);
    if (hr != WBEM_S_NO_ERROR)
        return -1;
    hr = CoSetProxyBlanket(wmiNamespace, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
    if (hr != WBEM_S_NO_ERROR)
        return -1;

    // query variable enum
    hr = wmiNamespace->ExecQuery(_bstr_t(L"WQL"), brightnessAdjustmentClassQuery, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &brightnessAdjustmentClassEnum);
    if (hr != WBEM_S_NO_ERROR)
        return -1;

    ULONG ret;
    hr = brightnessAdjustmentClassEnum->Next(WBEM_INFINITE, 1, &brightnessAdjustmentClassObject, &ret);
    if (hr != WBEM_S_NO_ERROR)
        return -1;

    hr = wmiNamespace->GetObject(brightnessAdjustmentClassPath, 0, NULL, &brightnessAdjustmentClass, NULL);
    if (hr != WBEM_S_NO_ERROR)
        return -1;

    // get brightness adjust method instant
    hr = brightnessAdjustmentClass->GetMethod(brightnessAdjustmentMethodName, 0, &brightnessAdjustmentClassInstant, NULL);
    if (hr != WBEM_S_NO_ERROR)
        return -1;
    hr = brightnessAdjustmentClassInstant->SpawnInstance(0, &brightnessAdjustmentMethodInstant);
    if (hr != WBEM_S_NO_ERROR)
        return -1;

    return 0;
}

int GetBrightness()
{
    // 每次都需要查询
    if (brightnessClassEnum) brightnessClassEnum->Release();
    if (brightnessClassObject) brightnessClassObject->Release();

    HRESULT hr = wmiNamespace->ExecQuery(_bstr_t(L"WQL"), brightnessQuery, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &brightnessClassEnum);
    if (hr != WBEM_S_NO_ERROR)
        return -1;

    DWORD ret;
    hr = brightnessClassEnum->Next(WBEM_INFINITE, 1, &brightnessClassObject, &ret);
    if (hr != WBEM_S_NO_ERROR)
        return -1;

    // 获得亮度值
    VARIANT brightnessValue;
    VariantInit(&brightnessValue);
    hr = brightnessClassObject->Get(brightnessVariableName, 0, &brightnessValue, 0, 0);
    if (hr != WBEM_S_NO_ERROR)
        return -1;
    return brightnessValue.uintVal;
}

int SetBrightness(int brightness)
{
    if (!init_flag) {
        Init();
    }
    // 亮度范围是 [0, 100]
    brightness = min(max(brightness, 0), 100);
    // write brightness adjust params
    VARIANT var;
    VariantInit(&var);
    V_VT(&var) = VT_BSTR;

    // write timeout param
    V_BSTR(&var) = SysAllocString(L"0");
    hr = brightnessAdjustmentMethodInstant->Put(brightnessAdjustmentMethodArgName1, 0, &var, CIM_UINT32);
    if (hr != WBEM_S_NO_ERROR)
        return -1;

    // write brightness param
    V_BSTR(&var) = SysAllocString(to_wstring(brightness).c_str());
    hr = brightnessAdjustmentMethodInstant->Put(brightnessAdjustmentMethodArgName2, 0, &var, CIM_UINT8);
    VariantClear(&var);
    if (hr != WBEM_S_NO_ERROR)
        return -1;

    // 获取 method instant path 和 execute
    VARIANT pathVariable;
    VariantInit(&pathVariable);
    hr = brightnessAdjustmentClassObject->Get(_bstr_t(L"__PATH"), 0, &pathVariable, NULL, NULL);
    if (hr != WBEM_S_NO_ERROR)
        return -1;
    hr = wmiNamespace->ExecMethod(pathVariable.bstrVal, brightnessAdjustmentMethodName, 0, NULL, brightnessAdjustmentMethodInstant, NULL, NULL);
    VariantClear(&pathVariable);
    if (hr != WBEM_S_NO_ERROR)
        return -1;
    return 0;
}

void Cleanup()
{
    // 清理
    SysFreeString(wmiPath);
    SysFreeString(brightnessAdjustmentClassPath);
    SysFreeString(brightnessAdjustmentMethodName);
    SysFreeString(brightnessAdjustmentMethodArgName1);
    SysFreeString(brightnessAdjustmentMethodArgName2);
    SysFreeString(brightnessAdjustmentClassQuery);

    SysFreeString(brightnessClassPath);
    SysFreeString(brightnessVariableName);
    SysFreeString(brightnessQuery);

    brightnessAdjustmentClass->Release();
    brightnessAdjustmentMethodInstant->Release();
    brightnessAdjustmentClassInstant->Release();
    brightnessAdjustmentClassObject->Release();
    brightnessAdjustmentClassEnum->Release();

    //if (brightnessClass) brightnessClass->Release();
    brightnessClassObject->Release();
    brightnessClassEnum->Release();

    wmiNamespace->Release();
    wmiLocator->Release();

    CoUninitialize();
}

// 亮度进阶值
const int incrementValue = 5;

void IncreaseBrightness()
{
    // 提升一阶亮度
    int i = GetBrightness();
    SetBrightness(i + incrementValue);
}
void ReduceBrightness()
{
    // 降低一阶亮度
    int i = GetBrightness();
    SetBrightness(i - incrementValue);
}

ATOM hotKeyId1, hotKeyId2;

LRESULT __stdcall WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_HOTKEY:
        if (wParam == hotKeyId1)
            PostMessageA(0, WM_USER + 1, 0, 0);
        else if (wParam == hotKeyId2)
            PostMessageA(0, WM_USER + 2, 0, 0);
        break;
    }
    return DefWindowProcA(hWnd, Msg, wParam, lParam);
}
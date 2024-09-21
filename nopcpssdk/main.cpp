#include "pch.h"

#include "utils.h"
#include "proxy.h"

using namespace Utils;

static decltype(LoadLibraryExA) * o_LoadLibraryExA = nullptr;

static HMODULE HookLoadLibraryExA(
    _In_ LPCSTR lpLibFileName,
    _Reserved_ HANDLE hFile,
    _In_ DWORD dwFlags
)
{
    if (lpLibFileName)
    {
        const std::string file( lpLibFileName );

        if (EndsWith( file, "PsPcSdk.dll" ))
        {
            return CURRENT_MODULE;
        }
    }

    return o_LoadLibraryExA( lpLibFileName, hFile, dwFlags );
}


PSPCSDK_API HRESULT sceCommonDialogInitialize()
{
    return S_OK;
};


PSPCSDK_API HRESULT sceNpCheckCallback()
{
    return S_OK;
};


PSPCSDK_API HRESULT sceNpRegisterStateCallbackA(
    std::size_t ptr, std::size_t cb
)
{
    // Since this function registers two 2 callbacks in a row,
    // we only need the first one (main state).
    static std::once_flag flag;

    std::call_once( flag, [cb]()
    {
        if (cb)
        {
            auto flag = reinterpret_cast<std::uint8_t *>(cb + 0x16C);
            *flag = '\05'; // '5' is what needed (optional register flag?)
        }
    } );

    return S_OK;
}


PSPCSDK_API HRESULT sceNpUniversalDataSystemGetMemoryStat( 
    std::size_t ptr
)
{
    return S_OK;
};


PSPCSDK_API HRESULT sceNpUniversalDataSystemInitialize( 
    std::size_t ptr
)
{
    return S_OK;
};


PSPCSDK_API HRESULT sceNpUniversalDataSystemTerminate()
{
    return S_OK;
};


PSPCSDK_API HRESULT sceNpUnregisterStateCallbackA( 
    const std::size_t id
)
{
    return S_OK;
};


PSPCSDK_API HRESULT scePsPcHookApiCall( 
    char * func
)
{
    return S_OK;
};


PSPCSDK_API HRESULT scePsPcInitializeInternal( 
    std::size_t ptr
)
{
    return S_OK;
};


PSPCSDK_API bool scePsPcIsSupportedPlatform()
{
    return true;
};


PSPCSDK_API HRESULT scePsPcSetDeviceDataCollectionSetting(int flag)
{
    return S_OK;
};


PSPCSDK_API HRESULT scePsPcTerminate()
{
    return S_OK;
};


PSPCSDK_API HRESULT sceSigninDialogGetResult(
    std::size_t ptr
)
{
    // Throw an internal error.
    return 0x8A80010E;
};


PSPCSDK_API HRESULT sceSigninDialogInitialize()
{
    return S_OK;
}


PSPCSDK_API HRESULT sceSigninDialogOpen( 
    void * ptr
)
{
    return S_OK;
};


PSPCSDK_API HRESULT sceSigninDialogTerminate()
{
    return S_OK;
};


PSPCSDK_API int sceSigninDialogUpdateStatus()
{
    // Looks like '3' is what exactly needed.
    return 3;
};


BOOL APIENTRY DllMain( 
    HMODULE hModule,
    DWORD ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            CURRENT_MODULE = hModule;

            char system_path[MAX_PATH] {};

            if (SHGetFolderPathA( nullptr, CSIDL_SYSTEM, nullptr, SHGFP_TYPE_CURRENT, system_path ) == S_OK)
            {
                const HMODULE mod = LoadLibraryW( (fs::path( system_path ) / "version.dll").c_str() );

                if (mod)
                {
                    for (std::uint32_t i { 0 }; i < (sizeof( exports_version ) / sizeof( exports_version[0] )); i++)
                    {
                        version_dll[i] = GetProcAddress( mod, exports_version[i] );
                    }

                    o_LoadLibraryExA = &LoadLibraryExA;
                    Detour( &o_LoadLibraryExA, HookLoadLibraryExA );
                }
            }

            break;
        }

        case DLL_PROCESS_DETACH:
        {
            Detour( &o_LoadLibraryExA, HookLoadLibraryExA, true );
            break;
        }
    }

    return TRUE;
}
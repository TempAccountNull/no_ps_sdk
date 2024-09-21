#pragma once

namespace Utils
{
    // Current module handle ('version.dll')
    extern HMODULE CURRENT_MODULE { nullptr };

    // Case-insensitive 'ends_with'
    static bool EndsWith(
        const std::string & str,
        const std::string & suffix
    )
    {
        // Check if the string is long enough to contain the suffix
        if (suffix.size() > str.size())
        {
            return false;
        }

        // Compare the suffix with the end of the string in a case-insensitive manner
        return std::equal( suffix.rbegin(), suffix.rend(), str.rbegin(),
            []( char c1, char c2 )
        {
            return std::tolower( c1 ) == std::tolower( c2 ); // Compare characters case-insensitively
        } );
    }


    // Create/remove detour
    template <typename T>
    void Detour( T ** org, T * hook, bool detach = false )
    {
        if ((org && hook))
        {
            LONG err {};

            err = DetourTransactionBegin();

            if (err != NO_ERROR)
                return;

            err = DetourUpdateThread( GetCurrentThread() );

            if (err != NO_ERROR)
            {
                DetourTransactionAbort();
                return;
            }

            if (detach)
                err = DetourDetach( reinterpret_cast<PVOID *>(org), reinterpret_cast<PVOID>(hook) );
            else
                err = DetourAttach( reinterpret_cast<PVOID *>(org), reinterpret_cast<PVOID>(hook) );

            if (err != NO_ERROR)
            {
                DetourTransactionAbort();
                return;
            }

            err = DetourTransactionCommit();

            if (err != NO_ERROR)
            {
                DetourTransactionAbort();
                return;
            }
        }
    }
}
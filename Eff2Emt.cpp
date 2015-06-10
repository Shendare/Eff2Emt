// Eff2Emt.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Eff2Emt.h"

int _tmain(int argc, _TCHAR* argv[])
{
    if (argc != 2)
    {
        printf("Usage:\n");
        printf("\n");
        printf("  %s Zones\n", argv[0]);
        printf("\n");
        printf("Zones can be a zone nick, a .eff filename for a zone, or a wildcard for zones to find and convert.\n");
        printf("\n");
        
        return 0;
    }

    std::wstring _path = argv[1];
    std::wstring _zones = _path;

    int _sep = _path.find_last_of(L"/\\:");

    if (_sep == std::wstring::npos)
    {
        _path = L"";
    }
    else
    {
        _path.erase(_sep);
        _zones.erase(0, _sep + 1);
    }

    _sep = _zones.find_first_of(L"_.");

    if (_sep != std::wstring::npos)
    {
        _zones.erase(_sep);
    }

    _zones += L"_sounds.eff";

    if ((_path.length() + _zones.length() + 1) >= MAX_PATH)
    {
        printf("Error: Specified directory name is too long.\n");

        return 1;
    }

    WIN32_FIND_DATA _fileFound;
    wchar_t _fullPath[MAX_PATH];

    std::wstring _search;

    if (_path == L"")
    {
        GetCurrentDirectory(MAX_PATH, _fullPath);
        _search = _zones;
    }
    else
    {
        _wfullpath(_fullPath, _path.c_str(), MAX_PATH);
        _search = _path + L"\\" + _zones;
    }

    HANDLE _fileList = FindFirstFile(_search.c_str(), &_fileFound);

    if (_fileList == INVALID_HANDLE_VALUE)
    {
        wprintf(L"Error: No %s file%s found in the %s.\n", _zones.c_str(), ((_zones.find_first_of(L"*?") == std::wstring::npos) ? L" was" : L"s were"), ((_path == L"") ? L"current directory" : L"specified location"));

        return 1;
    }

    do
    {
        wchar_t _filename[MAX_PATH];

        wcsncpy(_filename, _fileFound.cFileName, MAX_PATH);

        wmemchr(_filename, '_', MAX_PATH)[0] = NULL;

        std::string _filenameAscii = String::WideToASCII(_filename);

        switch (Eff2EmtConverter::ConvertZone(_fullPath, _filename, _filenameAscii))
        {
            case DialogResult::OK:
                printf("Converted Zone Sounds For: %s\n", _filenameAscii.c_str());
                break;
        }
    }
    while (FindNextFile(_fileList, &_fileFound) != NULL);

    FindClose(_fileList);

    return 0;
}

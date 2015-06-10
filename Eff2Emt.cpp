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

    std::wstring _path = GetFullPathFrom(argv[1]);
    std::wstring _find = GetFileSpecFrom(argv[1]);

    // Trim off any _sounds or _sndbnk and any file extention from the filespec
    int _sep = _find.find_first_of(L"_.");
    if (_sep != std::wstring::npos)
    {
        _find.erase(_sep);
    }

    // We're looking for *_sounds.eff
    _find += L"_sounds.eff";

    if ((_path.length() + _find.length() + 1) >= MAX_PATH)
    {
        printf("Error: Specified directory name is too long.\n"); // I suppose it could happen in a Send To situation...

        return 1;
    }

    WIN32_FIND_DATA _fileFound;
    _find = _path + L"\\" + _find;

    // Find our ZoneNick_sounds.eff file(s)
    HANDLE _fileList = FindFirstFile(_find.c_str(), &_fileFound);

    if (_fileList == INVALID_HANDLE_VALUE)
    {
        wprintf(L"Error: No %s file%s found in the %s.\n", _find.c_str(), ((_find.find_first_of(L"*?") == std::wstring::npos) ? L" was" : L"s were"), ((_path == L"") ? L"current directory" : L"specified location"));

        return 1;
    }

    const wchar_t* _fullPath = _path.c_str();
    wchar_t _filename[MAX_PATH];

    do
    {
        // Strip down to just the zone nick
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

std::wstring GetFullPathFrom(std::wstring Path)
{
    wchar_t _fullPath[MAX_PATH];

    int _sep = Path.find_last_of(L"/\\:");

    if (_sep == std::wstring::npos)
    {
        GetCurrentDirectory(MAX_PATH, _fullPath);
    }
    else
    {
        // Resolve relative path to absolute

        Path.erase(_sep);

        _wfullpath(_fullPath, Path.c_str(), MAX_PATH);
    }

    return _fullPath;
}

std::wstring GetFileSpecFrom(std::wstring Path)
{
    int _sep = Path.find_last_of(L"/\\:");

    if (_sep == std::wstring::npos)
    {
        return L"";
    }
    else
    {
        return Path.erase(0, _sep + 1);
    }
}

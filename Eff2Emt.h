// Eff2Emt.h

#include "Eff2EmtConverter.h"

std::wstring GetFullPathFrom(std::wstring Argument);
std::wstring GetFileSpecFrom(std::wstring Argument);

class String
{
public:
    static std::string WideToASCII(wchar_t* Text)
    {
        int _len = wcslen(Text);
        char _text[256];
        memset(_text, 0, sizeof(_text));
        wcstombs(_text, Text, Math::Min(_len, 255));
        std::string _string = _text;
        return _string;
    }
};

/*
*  Eff2Emt.h - Header file with global string class to convert UCS16 to UTF8.
*
*  By Shendare (Jon D. Jackson)
*
*  Portions of this code not covered by another author's or entity's copyright are released under
*  the Creative Commons Zero (CC0) public domain license.
*
*  To the extent possible under law, Shendare (Jon D. Jackson) has waived all copyright and
*  related or neighboring rights to this EQIconExtractor application.
*  This work is published from: The United States.
*
*  You may copy, modify, and distribute the work, even for commercial purposes, without asking permission.
*
*  For more information, read the CC0 summary and full legal text here:
*
*  https://creativecommons.org/publicdomain/zero/1.0/
*
*/

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

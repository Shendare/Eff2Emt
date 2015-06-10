// Eff2EmtConverter Classes

#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <sys/stat.h>
#include <unordered_map>
#include <vector>

#pragma warning(disable: 4996) // Visual C++ doesn't like strncpy() since buffer overruns can happen if you don't check bounds.

#pragma once

// For ease of copy/paste from C#
typedef signed long Int32;
typedef unsigned long UInt32;
typedef unsigned char Byte;

class Math
{
public:
    static Int32 Min(Int32 value1, Int32 value2) { return (value2 < value1) ? value2 : value1; }
    static Int32 Max(Int32 value1, Int32 value2) { return (value2 > value1) ? value2 : value1; }
};

class File
{
public:
    static bool Exists(const wchar_t* Path) { struct _stat buffer; return (_wstat(Path, &buffer) == 0); }
    static size_t GetSize(const wchar_t* Path) { struct _stat buffer; return (_wstat(Path, &buffer) == 0 ? buffer.st_size : 0); }
};

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

enum DialogResult
{
    None = 0,
    OK = 1,
    Cancel = 2,
    Abort = 3,
    Retry = 4,
    Ignore = 5,
    Yes = 6,
    No = 7
};

struct EffSoundEntry
{
public:
    Int32 UnkRef00;
    Int32 UnkRef04;
    Int32 Reserved;
    Int32 Sequence;
    float X;
    float Y;
    float Z;
    float Radius;
    Int32 Cooldown1;
    Int32 Cooldown2;
    Int32 RandomDelay;
    Int32 Unk44;
    Int32 SoundID1;
    Int32 SoundID2;
    Byte SoundType;
    Byte UnkPad57;
    Byte UnkPad58;
    Byte UnkPad59;
    Int32 AsDistance;
    Int32 UnkRange64;
    Int32 FadeOutMS;
    Int32 UnkRange72;
    Int32 FullVolRange;
    Int32 UnkRange80;
};

class EmtSoundEntry
{
public:
    static const int MAXLEN_ENTRYTYPE = 64;
    static const int MAXLEN_SOUNDFILE = 64;

    char EntryType[MAXLEN_ENTRYTYPE];
    char SoundFile[MAXLEN_SOUNDFILE];
    Int32 Reserved1 = 0;
    Int32 WhenActive = 0;
    float Volume = 1.0f;
    Int32 FadeInMS = 500;
    Int32 FadeOutMS = 1000;
    Int32 WavLoopType = 0;
    float X, Y, Z;
    float WavFullVolRadius = 50;
    float WavMaxAudibleDist = 50;
    bool RandomizeLocation = false;
    Int32 ActivationRange = 50;
    Int32 MinRepeatDelay = 0;
    Int32 MaxRepeatDelay = 0;
    Int32 xmiIndex = 0;
    Int32 EchoLevel = 0;
    bool IsEnvSound = true;

    EmtSoundEntry()
    {
        EntryType[0] = '2';
        EntryType[1] = NULL;
        SoundFile[0] = NULL;
    }

    std::string ToString();

    bool _IsValid() { return ((SoundFile != NULL) && (SoundFile[0] != NULL)); }

    EmtSoundEntry* _Clone();
    void CloneTo(EmtSoundEntry &SoundEntry);
};

class Eff2EmtConverter
{
public:
    static const char* EMTLineFormat;

    static const int MAXLEN_SOUNDBNK = 768; // Largest is actually 560 bytes, so this is plenty.
    static const int MAXLEN_MP3INDEX = 512; // Actual file is 350 bytes.

    static std::vector<char*> HardCodedSoundFiles;
    static std::vector<char*> DefaultMusicFiles;

    static char _SoundBankBuffer[MAXLEN_SOUNDBNK];
    static char _MP3IndexBuffer[MAXLEN_MP3INDEX];

    static DialogResult ConvertZone(const wchar_t* EQFolder, wchar_t* ZoneNick, std::string &ZoneNickASCII);

protected:
    static std::vector<char*> _SoundBank_Emit;
    static std::vector<char*> _SoundBank_Loop;

    static std::vector<char*> _MP3IndexFiles;

    // Return the entry from mp3index.txt on line number abs(Index)
    static char* MP3IndexFile(int Index);

    static bool LoadTextFile(const wchar_t* Path, char* Buffer, int BufferSize, std::vector<std::string> &LineStrings);
    static bool LoadTextFile(const wchar_t* Path, char* Buffer, int BufferSize, std::vector<char*> &Lines);

    static bool LoadMP3Index(wchar_t* Folder);

    // Convert SoundID from ZoneNick_sounds.eff into a sound file name
    static char* SoundFileNumber(int SoundID);

    static void CheckDefaults();
};
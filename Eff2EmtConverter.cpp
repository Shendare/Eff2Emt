/*
*  Eff2EmtConverter.cpp - Class for handling conversion of old EverQuest ZoneNick_sounds.eff and ZoneNick_sndbnk.eff to editable ZoneNick.emt text files
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

const char* Eff2EmtConverter::EMTLineFormat = ";?,SoundFile (wav=sound mp3/xmi=music),Unknown (0=OK 1=OK),WhenActive (0=Always 1=Daytime 2=Nighttime),Volume (1.0 = 100%),FadeInMS,FadeOutMS,WavLoopType (0=Constant 1=Delayed Repeat),X,Y,Z,WavFullVolRadius,WavMaxAudibleDist,NonZero = RandomizeLocation,ActivationRange,MinRepeatDelay,MaxRepeatDelay,xmiIndex,EchoLevel (50 = Max),IsEnvSound (for option toggle)";
std::vector<char*> Eff2EmtConverter::HardCodedSoundFiles;
std::vector<char*> Eff2EmtConverter::DefaultMusicFiles;
std::vector<char*> Eff2EmtConverter::_SoundBank_Emit;
std::vector<char*> Eff2EmtConverter::_SoundBank_Loop;
std::vector<char*> Eff2EmtConverter::_MP3IndexFiles;
char Eff2EmtConverter::_SoundBankBuffer[] = { 0 };
char Eff2EmtConverter::_MP3IndexBuffer[] = { 0 };

void EmtSoundEntry::CloneTo(EmtSoundEntry &SoundEntry)
{
    memmove(SoundEntry.EntryType, this->EntryType, sizeof(this->EntryType));
    memmove(SoundEntry.SoundFile, this->SoundFile, sizeof(this->SoundFile));
    SoundEntry.Reserved1 = this->Reserved1;
    SoundEntry.WhenActive = this->WhenActive;
    SoundEntry.Volume = this->Volume;
    SoundEntry.FadeInMS = this->FadeInMS;
    SoundEntry.FadeOutMS = this->FadeOutMS;
    SoundEntry.WavLoopType = this->WavLoopType;
    SoundEntry.X = this->X;
    SoundEntry.Y = this->Y;
    SoundEntry.Z = this->Z;
    SoundEntry.WavFullVolRadius = this->WavFullVolRadius;
    SoundEntry.WavMaxAudibleDist = this->WavMaxAudibleDist;
    SoundEntry.RandomizeLocation = this->RandomizeLocation;
    SoundEntry.ActivationRange = this->ActivationRange;
    SoundEntry.MinRepeatDelay = this->MinRepeatDelay;
    SoundEntry.MaxRepeatDelay = this->MaxRepeatDelay;
    SoundEntry.xmiIndex = this->xmiIndex;
    SoundEntry.EchoLevel = this->EchoLevel;
    SoundEntry.IsEnvSound = this->IsEnvSound;
}

EmtSoundEntry* EmtSoundEntry::_Clone()
{
    EmtSoundEntry* _new = new EmtSoundEntry();

    if (_new != NULL)
    {
        this->CloneTo(*_new);
    }

    return _new;
}

std::string EmtSoundEntry::ToString()
{
    if (!_IsValid())
    {
        return "";
    }
    else
    {
        std::stringstream _stringBuilder;

        _stringBuilder.precision(1);
        _stringBuilder << std::fixed;

        _stringBuilder << EntryType;
        _stringBuilder << ',';
        _stringBuilder << SoundFile;
        _stringBuilder << ',';
        _stringBuilder << Reserved1;
        _stringBuilder << ',';
        _stringBuilder << WhenActive;
        _stringBuilder << ',';
        _stringBuilder << Volume;
        _stringBuilder << ',';
        _stringBuilder << FadeInMS;
        _stringBuilder << ',';
        _stringBuilder << FadeOutMS;
        _stringBuilder << ',';
        _stringBuilder << WavLoopType;
        _stringBuilder << ',';
        _stringBuilder << X;
        _stringBuilder << ',';
        _stringBuilder << Y;
        _stringBuilder << ',';
        _stringBuilder << Z;
        _stringBuilder << ',';
        _stringBuilder << WavFullVolRadius;
        _stringBuilder << ',';
        _stringBuilder << WavMaxAudibleDist;
        _stringBuilder << ',';
        _stringBuilder << RandomizeLocation ? '1' : '0';
        _stringBuilder << ',';
        _stringBuilder << ActivationRange;
        _stringBuilder << ',';
        _stringBuilder << MinRepeatDelay;
        _stringBuilder << ',';
        _stringBuilder << MaxRepeatDelay;
        _stringBuilder << ',';
        _stringBuilder << xmiIndex;
        _stringBuilder << ',';
        _stringBuilder << EchoLevel;
        _stringBuilder << ',';
        _stringBuilder << IsEnvSound ? '1' : '0';

        return _stringBuilder.str();
    }
}

char* Eff2EmtConverter::MP3IndexFile(int ID)
{
    if ((ID == 0) || (_MP3IndexFiles.size() == 0))
    {
        return "";
    }

    unsigned long _id = (unsigned long)(ID < 0 ? -ID : ID);

    if (_id >= _MP3IndexFiles.size())
    {
        return "";
    }

    return _MP3IndexFiles[ID];
}

// Convert SoundID from ZoneNick_sounds.eff into a sound file name
char* Eff2EmtConverter::SoundFileNumber(int SoundID)
{
    // 0 == None
    if (SoundID == 0)
    {
        return "";
    }

    // < 0 == Music File Reference in mp3index.txt
    if (SoundID < 0)
    {
        return MP3IndexFile(-SoundID);
    }

    unsigned long _soundID = (unsigned long)SoundID;

    // 1 - 31 == Sound File Reference in ZoneNick_sndbnk.eff (EMIT section)
    if (SoundID < 32)
    {
        if (_soundID > _SoundBank_Emit.size())
        {
            return "";
        }
        else
        {
            return _SoundBank_Emit[_soundID - 1];
        }
    }

    // 162-169 == Sound File Reference in ZoneNick_sndbnk.eff (LOOP section)
    if (_soundID > 161)
    {
        _soundID -= 161;

        if (_soundID > _SoundBank_Loop.size())
        {
            return "";
        }
        else
        {
            return _SoundBank_Loop[_soundID - 1];
        }
    }

    // 32 - 161 == Hard-Coded Sound Files
    if (_soundID >= HardCodedSoundFiles.size())
    {
        return "";
    }

    return HardCodedSoundFiles[_soundID];
}

void Eff2EmtConverter::CheckDefaults()
{
    // Initialize defaults if they haven't been already.

    if (HardCodedSoundFiles.size() == 0)
    {
        HardCodedSoundFiles.reserve(162);

        for (int _i = 0; _i < 162; _i++)
        {
            HardCodedSoundFiles.push_back("");
        }

        HardCodedSoundFiles[39] = "death_me";
        HardCodedSoundFiles[143] = "thunder1";
        HardCodedSoundFiles[144] = "thunder2";
        HardCodedSoundFiles[158] = "wind_lp1";
        HardCodedSoundFiles[159] = "rainloop";
        HardCodedSoundFiles[160] = "torch_lp";
        HardCodedSoundFiles[161] = "watundlp";
    }

    if (DefaultMusicFiles.size() == 0)
    {
        DefaultMusicFiles.reserve(25);

        for (int _i = 0; _i < 25; _i++)
        {
            DefaultMusicFiles.push_back("");
        }

        DefaultMusicFiles[1] = "bothunder.mp3";
        DefaultMusicFiles[2] = "codecay.mp3";
        DefaultMusicFiles[3] = "combattheme1.mp3";
        DefaultMusicFiles[4] = "combattheme2.mp3";
        DefaultMusicFiles[5] = "deaththeme.mp3";
        DefaultMusicFiles[6] = "eqtheme.mp3";
        DefaultMusicFiles[7] = "hohonor.mp3";
        DefaultMusicFiles[8] = "poair.mp3";
        DefaultMusicFiles[9] = "podisease.mp3";
        DefaultMusicFiles[10] = "poearth.mp3";
        DefaultMusicFiles[11] = "pofire.mp3";
        DefaultMusicFiles[12] = "poinnovation.mp3";
        DefaultMusicFiles[13] = "pojustice.mp3";
        DefaultMusicFiles[14] = "poknowledge.mp3";
        DefaultMusicFiles[15] = "ponightmare.mp3";
        DefaultMusicFiles[16] = "postorms.mp3";
        DefaultMusicFiles[17] = "potactics.mp3";
        DefaultMusicFiles[18] = "potime.mp3";
        DefaultMusicFiles[19] = "potorment.mp3";
        DefaultMusicFiles[20] = "potranquility.mp3";
        DefaultMusicFiles[21] = "povalor.mp3";
        DefaultMusicFiles[22] = "powar.mp3";
        DefaultMusicFiles[23] = "powater.mp3";
        DefaultMusicFiles[24] = "solrotower.mp3";
    }
}

bool Eff2EmtConverter::LoadTextFile(const wchar_t* Path, char* Buffer, int BufferSize, std::vector<std::string> &LineStrings)
{
    std::vector<char*> Lines;

    if (!LoadTextFile(Path, Buffer, BufferSize, Lines))
    {
        return false;
    }

    bool _emptyFirstLine = true;

    LineStrings.clear();
    for (std::vector<char*>::iterator _line = Lines.begin(); _line != Lines.end(); _line++)
    {
        if (_emptyFirstLine)
        {
            _emptyFirstLine = false;
        }
        else
        {
            LineStrings.push_back(*_line);
        }
    }

    return true;
}
bool Eff2EmtConverter::LoadTextFile(const wchar_t* Path, char* Buffer, int BufferSize, std::vector<char*> &Lines)
{
    std::ifstream _in(Path, std::ios::binary);

    if (!_in.is_open())
    {
        return false;
    }

    Lines.clear();
    Lines.reserve(32); // Decent pre-allocation for most text files.
    Lines.push_back(""); // Lines[0] shouldn't be referenced, but juuuust in case.

    memset(Buffer, 0, BufferSize);

    _in.read(Buffer, BufferSize - 1);
    int _fileSize = (int)_in.gcount();
    _in.close();

    char* _pos = Buffer;
    char* _end = Buffer + _fileSize + 1; // Parsing up to extra null at the end of the buffer in case the file ends without a line break.
    char* _lineStart = _pos;
    bool _lineEnding = true;

    while (_pos < _end)
    {
        switch (*_pos)
        {
            case '\0':
            case '\r':
            case '\n':
                *_pos = NULL;

                if (!_lineEnding)
                {
                    Lines.push_back(_lineStart);
                    _lineEnding = true;
                }
                break;
            case ' ':
                // Whitespace is completely ignored.
                break;
            default:
                if (_lineEnding)
                {
                    _lineStart = _pos;
                    _lineEnding = false;
                }
                break;
        }

        _pos++;
    }

    return true;
}

DialogResult Eff2EmtConverter::ConvertZone(const wchar_t* EQFolder, wchar_t* ZoneNick, std::string &ZoneNickAscii)
{
    if ((EQFolder == NULL) || (ZoneNick == NULL) || (EQFolder[0] == NULL) || (ZoneNick[0] == NULL))
    {
        return DialogResult::Abort;
    }

    std::wstring _mp3IndexFilename = std::wstring(EQFolder) + L"\\mp3index.txt";
    std::wstring _zoneSoundEntriesFilename = std::wstring(EQFolder) + L"\\" + ZoneNick + L"_sounds.eff";
    std::wstring _zoneSoundBankFilename = std::wstring(EQFolder) + L"\\" + ZoneNick + L"_sndbnk.eff";
    std::wstring _zoneSoundEmitterFilename = std::wstring(EQFolder) + L"\\" + ZoneNick + L".emt";

    std::string _line;

    CheckDefaults();

    // Preliminary: Read the mp3index.txt file's entries, in case they've been changed from the defaults.

    if (_MP3IndexFiles.size() == 0)
    {
        if (!LoadTextFile(_mp3IndexFilename.c_str(), &_MP3IndexBuffer[0], sizeof(_MP3IndexBuffer), _MP3IndexFiles))
        {
            printf(" Note: mp3index.txt not found. Using defaults.\n");

            _MP3IndexFiles = DefaultMusicFiles;
        }
    }

    // Step 1 - Open ZoneNick_sounds.eff (Required)

    std::ifstream _effFile(_zoneSoundEntriesFilename, std::ios::binary);

    if (!_effFile.is_open())
    {
        printf("Error: Could not open Sound Entries file: %s_sounds.eff. Zone skipped.\n", ZoneNick);

        return DialogResult::Abort;
    }

    // Step 2 - Open ZoneNick.emt if it exists, and read in the current contents for merging our new entries into

    static std::unordered_map<std::string, bool> _emtEntries;
    static std::vector<std::string> _emtExistingEntries;

    size_t _emtFileSize = File::GetSize(_zoneSoundEmitterFilename.c_str());
    char* _emtFileBuffer = NULL;

    if (_emtFileSize > 0)
    {
        _emtFileBuffer = new char[_emtFileSize + 1];

        LoadTextFile(_zoneSoundEmitterFilename.c_str(), _emtFileBuffer, _emtFileSize + 1, _emtExistingEntries);
    }

    // Step 3 - Create or replace ZoneNick.emt

    std::ofstream _emtFile(_zoneSoundEmitterFilename.c_str(), std::ios::beg);

    if (!_emtFile.is_open())
    {
        printf("Error: Could not write to Sound Emitter file: %s_sndbnk.eff. Zone skipped.\n", ZoneNick);

        _effFile.close();

        return DialogResult::Abort;
    }

    // Step 4 - Read sound file references from ZoneNick_sndbnk.eff (Required unless only background music entries are in ZoneNick_sounds.eff)

    std::vector<char*> _bnkLines;
    _SoundBank_Emit.clear();
    _SoundBank_Loop.clear();

    if (LoadTextFile(_zoneSoundBankFilename.c_str(), &_SoundBankBuffer[0], MAXLEN_SOUNDBNK, _bnkLines))
    {
        bool _inEmitSection = true;

        for (std::vector<char*>::iterator _bnkEntry = _bnkLines.begin(); _bnkEntry != _bnkLines.end(); ++_bnkEntry)
        {
            if (strcmp(*_bnkEntry, "EMIT") == 0)
            {
                _inEmitSection = true;
            }
            else if ((strcmp(*_bnkEntry, "LOOP") == 0) || (strcmp(*_bnkEntry, "RAND") == 0))
            {
                _inEmitSection = false;
            }
            else if (*_bnkEntry[0] != NULL)
            {
                if (_inEmitSection)
                {
                    _SoundBank_Emit.push_back(*_bnkEntry);
                }
                else
                {
                    _SoundBank_Loop.push_back(*_bnkEntry);
                }
            }
        }
    }

    // Step 5 - Initialize ZoneNick.emt

    if ((_emtExistingEntries.size() < 1) || ((*_emtExistingEntries.begin())[0] != ';'))
    {
        _emtEntries[EMTLineFormat] = true;
        _emtFile << EMTLineFormat;  // Add our format line for human-readable reference, if it isn't already there.
        _emtFile << "\n";
    }

    for (std::vector<std::string>::iterator _emtEntry = _emtExistingEntries.begin(); _emtEntry != _emtExistingEntries.end(); ++_emtEntry)
    {
        _emtEntries[*_emtEntry] = true;
        _emtFile << *_emtEntry;  // Write the existing entries to the file. We'll append any new ones to the end.
        _emtFile << "\n";
    }

    _emtExistingEntries.clear();
    if (_emtFileBuffer != NULL)
    {
        delete _emtFileBuffer;
    }

    // Step 6 - Read binary entries from ZoneNick_sounds.eff and write text entries to ZoneNick.emt if they aren't already there.

    while (!_effFile.eof())
    {
        EffSoundEntry _effEntry;

        _effFile.read((char*)&_effEntry, sizeof(_effEntry));
        if (_effFile.gcount() < sizeof(_effEntry))
        {
            break; // Ran out of file before we had a full new sound entry
        }

        EmtSoundEntry _sound1;
        EmtSoundEntry _sound2;

        std::string _soundFile1;
        std::string _soundFile2;

        switch (_effEntry.SoundType)
        {
            case 0: // Day/Night Sound Effect, Constant Volume
                _soundFile1 = SoundFileNumber(_effEntry.SoundID1);
                _soundFile2 = SoundFileNumber(_effEntry.SoundID2);
                _sound1.WhenActive = 1;
                break;
            case 1: // Background Music
                _soundFile1 = (_effEntry.SoundID1 < 0) ? SoundFileNumber(_effEntry.SoundID1) : (_effEntry.SoundID1 == 0) ? "" : std::string(ZoneNickAscii) + ".xmi";
                if (_effEntry.SoundID1 == _effEntry.SoundID2)
                {
                    _soundFile2 = "";
                    _sound1.WhenActive = 0; // Same music and location for day and night. No need to add two entries to .emt file.
                }
                else
                {
                    _soundFile2 = (_effEntry.SoundID2 < 0) ? SoundFileNumber(_effEntry.SoundID2) : (_effEntry.SoundID2 == 0) ? "" : std::string(ZoneNickAscii) + ".xmi";
                    _sound1.WhenActive = 1;
                }

                _sound1.xmiIndex = ((_effEntry.SoundID1 > 0) && (_effEntry.SoundID1 < 32)) ? _effEntry.SoundID1 : 0;
                break;
            case 2: // Static Sound Effect
                _soundFile1 = SoundFileNumber(_effEntry.SoundID1);
                _soundFile2 = "";

                _sound1.Volume = (_effEntry.AsDistance < 0) ? 0.0f : (_effEntry.AsDistance > 3000) ? 0.0f : (3000.0f - (float)_effEntry.AsDistance) / 3000.0f;
                break;
            case 3: // Day/Night Sound Effect, Volume by Distance
                _soundFile1 = SoundFileNumber(_effEntry.SoundID1);
                _soundFile2 = SoundFileNumber(_effEntry.SoundID2);

                _sound1.Volume = (_effEntry.AsDistance < 0) ? 0.0f : (_effEntry.AsDistance > 3000) ? 0.0f : (3000.0f - (float)_effEntry.AsDistance) / 3000.0f;
                _sound1.WhenActive = 1;
                break;
            default: // Unsupported
                continue;
        }

        if ((_soundFile1.length() > 0) && (_soundFile1.find('.') == std::string::npos))
        {
            _soundFile1 += ".wav";
        }

        strncpy(_sound1.SoundFile, _soundFile1.c_str(), sizeof(_sound1.SoundFile) - 1);

        _sound1.FadeOutMS = (_effEntry.FadeOutMS <= 0) ? 0 : (_effEntry.FadeOutMS < 100) ? 100 : _effEntry.FadeOutMS; // Sanity check: make sure FadeOutMS is either 0 or 100+ milliseconds

        // Fade in does not appear to be utilized in ZoneNick_sounds.eff, even if one of the Unk fields is supposed to map to it. Make sounds fade in at twice the speed of fading out, 
        // so a running player doesn't feel like a sound effect finished fading in after they already passed by.
        _sound1.FadeInMS = Math::Min(_sound1.FadeOutMS / 2, 5000); // Cap at 5 second FadeIn. Some music entries have a looooong FadeOut.

        _sound1.X = _effEntry.X;
        _sound1.Y = _effEntry.Y;
        _sound1.Z = _effEntry.Z;
        if (_effEntry.SoundType != 1) // Music files ignore cooldowns in ZoneNick.emt.
        {
            _sound1.WavLoopType = (_effEntry.Cooldown1 <= 0) && (_effEntry.RandomDelay <= 0) ? 0 : 1;
            _sound1.MinRepeatDelay = (_sound1.WavLoopType == 0) ? 0 : Math::Max(_effEntry.Cooldown1, 0);
            _sound1.MaxRepeatDelay = (_sound1.WavLoopType == 0) ? 0 : Math::Max(_effEntry.Cooldown1, 0) + Math::Max(_effEntry.RandomDelay, 0);
        }
        _sound1.WavFullVolRadius = (_effEntry.SoundType == 0) ? _effEntry.Radius : Math::Max(_effEntry.FullVolRange, 0);
        _sound1.ActivationRange = (Int32)_effEntry.Radius;
        _sound1.WavMaxAudibleDist = (float)_sound1.ActivationRange; // No fields in ZoneNick_sounds.eff appear to map to this, so use the activation range.
        _sound1.IsEnvSound = (_effEntry.SoundType != 1);

        if (_soundFile2 == "")
        {
            _sound2.SoundFile[0] = NULL;
        }
        else
        {
            _sound1.CloneTo(_sound2);

            if (_soundFile2.find('.') == std::string::npos)
            {
                _soundFile2 += ".wav";
            }

            strncpy(_sound2.SoundFile, _soundFile2.c_str(), sizeof(_sound2.SoundFile) - 1);

            if (_effEntry.SoundType != 1) // Music files ignore cooldowns in ZoneNick.emt.
            {
                _sound2.WavLoopType = (_effEntry.Cooldown2 <= 0) && (_effEntry.RandomDelay <= 0) ? 0 : 1;
                _sound2.MinRepeatDelay = Math::Max(_effEntry.Cooldown2, 0);
                _sound2.MaxRepeatDelay = Math::Max(_effEntry.Cooldown2, 0) + Math::Max(_effEntry.RandomDelay, 0);
            }

            switch (_effEntry.SoundType)
            {
                case 0: // Day/Night Sound Effect, Constant Volume
                    _sound2.WhenActive = 2;
                    break;
                case 1: // Background Music
                    _sound2.WhenActive = 2;
                    _sound2.xmiIndex = ((_effEntry.SoundID2 > 0) && (_effEntry.SoundID2 < 32)) ? _effEntry.SoundID2 : 0;
                    break;
                case 3: // Day/Night Sound Effect, Volume by Distance
                    _sound2.WhenActive = 2;
                    break;
                default: // Unsupported
                    continue;
            }
        }

        _soundFile1 = _sound1.ToString();
        _soundFile2 = _sound2.ToString();

        if ((_soundFile1 != "") && (_emtEntries.count(_soundFile1) == 0))
        {
            _emtEntries[_soundFile1] = true;
            _emtFile << _soundFile1;
            _emtFile << "\n";
        }

        if ((_soundFile2 != "") && (_emtEntries.count(_soundFile2) == 0))
        {
            _emtEntries[_soundFile2] = true;
            _emtFile << _soundFile2;
            _emtFile << "\n";
        }
    }

    _emtFile.close();

    return DialogResult::OK;
}

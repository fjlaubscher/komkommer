#include <cstdio>
#include <cstring>
#include <string>

#include "ini.h"
#include "xpipe.h"
#include "xstraw.h"

static int failures = 0;

static void Check(bool condition, char const* message)
{
    if (!condition) {
        std::fprintf(stderr, "%s\n", message);
        ++failures;
    }
}

static bool Load_Text(INIClass& ini, char const* text)
{
    BufferStraw straw(text, static_cast<int>(std::strlen(text)));
    return ini.Load(straw);
}

int main()
{
    {
        INIClass ini;
        Check(!Load_Text(ini, "\n\n"), "whitespace alone is not an INI file");
        Check(!ini.Is_Loaded(), "a failed load stores nothing");
        Check(!Load_Text(ini, "this is not a section\nkey=value\n; comment\n"), "text before any section is ignored");
        Check(!ini.Section_Present("key"), "a key line before the first section is not a section");
    }

    char const* text = "preamble stays out\r\n"
                       "\n"
                       "; a comment line\n"
                       "  [  House  ]  \r\n"
                       "  Credits =  1000  \n"
                       "Name=GDI ; trailing comment\n"
                       "My Key=kept\n"
                       "Blank=\n"
                       "=nokey\n"
                       "novalue\n"
                       "Spaces=   \n"
                       "Flag = yes\n"
                       "Hex = $1A\n"
                       "HexH = 10h\n"
                       "HexWord = 0x10\n"
                       "Neg = -3\n"
                       "Junk = abc\n"
                       "Maybe = maybe\n"
                       "Zero = 0\n"
                       "TrueWord = true\n"
                       "FalseWord = false\n"
                       "One = 1\n"
                       "Two = 2\n"
                       "Y = Y\n"
                       "N = n\n"
                       "T = T\n"
                       "F = f\n"
                       "Percent = 50%\n"
                       "Fixed = 1.5\n"
                       "Dot = .5\n"
                       "PlainHex = FF\n"
                       "Dup = first\n"
                       "Dup = second\n"
                       "Path=a;b\n"
                       "[Empty]\n"
                       "\n"
                       "[house]\n"
                       "Credits=1\n"
                       "Extra=nope\n"
                       "[Other]\n"
                       "Key=value\n";

    INIClass ini;
    Check(Load_Text(ini, text), "a file with a section loads");
    Check(ini.Section_Count() == 2, "empty and duplicate sections are not stored");
    Check(ini.Section_Present("house") && ini.Section_Present("OTHER"), "section lookup is case insensitive");
    Check(!ini.Section_Present("Empty"), "a section with no entries is dropped");
    Check(!ini.Is_Present("House", "Extra"), "a repeated section does not replace the first");
    Check(ini.Entry_Count("House") == 25, "blank, comment, and broken lines are not entries");
    Check(std::strcmp(ini.Get_Entry("House", 0), "Credits") == 0, "entries stay in file order");

    Check(ini.Get_Int("HOUSE", "credits", 7) == 1000, "decimal integers ignore surrounding space and case");
    Check(ini.Get_Int("House", "Hex", 0) == 0x1A, "a leading $ is hexadecimal");
    Check(ini.Get_Int("House", "HexH", 0) == 0x10, "a trailing h is hexadecimal");
    Check(ini.Get_Int("House", "Neg", 0) == -3, "a leading minus stays negative");
    Check(ini.Get_Int("House", "Junk", 9) == 0, "a present non-number becomes zero, not the default");
    Check(ini.Get_Int("House", "HexWord", 9) == 0, "a 0x prefix is not hexadecimal");
    Check(ini.Get_Int("House", "Missing", 9) == 9, "a missing integer returns the default");
    Check(ini.Get_Int(NULL, "Credits", 9) == 9, "a null section returns the integer default");
    Check(ini.Get_Int("House", NULL, 4) == 4, "a null key returns the integer default");
    Check(ini.Get_Hex("House", "PlainHex", 0) == 0xFF, "Get_Hex reads a bare hexadecimal value");
    Check(ini.Get_Hex("House", "Missing", 3) == 3, "a missing hex value returns the default");

    Check(ini.Get_Bool("House", "Flag", false), "yes is true");
    Check(ini.Get_Bool("House", "TrueWord", false) && ini.Get_Bool("House", "Y", false), "true and Y are true");
    Check(ini.Get_Bool("House", "T", false) && ini.Get_Bool("House", "One", false), "T and 1 are true");
    Check(!ini.Get_Bool("House", "FalseWord", true) && !ini.Get_Bool("House", "N", true), "false and n are false");
    Check(!ini.Get_Bool("House", "F", true) && !ini.Get_Bool("House", "Zero", true), "f and 0 are false");
    Check(ini.Get_Bool("House", "Maybe", true), "an unrecognised boolean returns the default");
    Check(ini.Get_Bool("House", "Two", true), "a boolean that is not 0 or 1 returns the default");
    Check(!ini.Get_Bool("House", "Missing", false), "a missing boolean returns the default");

    Check(ini.Get_String("House", "Name", std::string("nope")) == "GDI", "a trailing semicolon is a comment");
    Check(ini.Get_String("House", "My Key", std::string()) == "kept", "spaces are part of a key");
    Check(ini.Get_String("House", "Path", std::string()) == "a", "a semicolon inside a value ends the value");
    Check(ini.Get_String("House", "Dup", std::string()) == "first", "the first of two identical keys wins");
    Check(ini.Get_String("Other", "Key", std::string()) == "value", "a later section is kept");
    Check(ini.Get_String("House", "Missing", std::string("fallback")) == "fallback",
          "a missing string returns the default");
    char small[8];
    Check(ini.Get_String("House", "Name", "zzzz", small, 8) == 3, "a short value reports its length");
    Check(std::strcmp(small, "GDI") == 0, "a short value fits in the buffer");
    char cut[4];
    Check(ini.Get_String("House", "TrueWord", "", cut, 4) == 3, "a long value reports the truncated length");
    Check(std::strcmp(cut, "tru") == 0, "a long value is cut to the buffer");
    char none[8] = "stale";
    Check(ini.Get_String(NULL, "Name", "def", none, 8) == 0 && none[0] == '\0', "a null section clears the buffer");

    Check(ini.Get_Fixed("House", "Percent", fixed(0)) == fixed(1, 2), "a trailing percent is a fraction of 100");
    Check(ini.Get_Fixed("House", "Fixed", fixed(0)) == fixed(3, 2), "a decimal point splits whole and fraction");
    Check(ini.Get_Fixed("House", "Dot", fixed(0)) == fixed(1, 2), "a leading dot is a fraction");
    Check(ini.Get_Fixed("House", "Missing", fixed(7)) == fixed(7), "a missing fixed value returns the default");

    Check(!ini.Is_Present("House", "Blank") && !ini.Is_Present("House", "Spaces"), "an empty value is not stored");
    ini.Clear("Other", "Key");
    Check(!ini.Is_Present("Other", "Key") && ini.Section_Present("Other"), "clearing one key leaves the section");
    ini.Clear("Other");
    Check(!ini.Section_Present("Other") && ini.Section_Present("House"), "clearing a section leaves the others");

    {
        INIClass dropped;
        Check(Load_Text(dropped, "[House]\nCredits=5"), "a section header is enough for load to succeed");
        Check(!dropped.Is_Present("House", "Credits"), "the last line is dropped when it has no newline");
    }

    INIClass round;
    Check(round.Put_String("Options", "Name", "Nod"), "Put_String creates a section");
    Check(round.Put_Int("Options", "Credits", 500), "Put_Int stores a decimal");
    Check(round.Put_Bool("Options", "Bases", false), "Put_Bool stores no");
    Check(round.Put_Hex("Options", "Seed", 0x2A), "Put_Hex stores hexadecimal");
    Check(round.Put_String("Options", "Name", ""), "an empty Put_String removes the key");
    Check(!round.Is_Present("Options", "Name"), "the removed key is gone");
    char saved[512];
    std::memset(saved, 0, sizeof(saved));
    BufferPipe pipe(saved, static_cast<int>(sizeof(saved)) - 1);
    int written = round.Save(pipe);
    Check(written > 0, "Save writes the database");
    INIClass loaded;
    BufferStraw straw(saved, written);
    Check(loaded.Load(straw), "saved text loads again");
    Check(!loaded.Is_Present("Options", "Name"), "a removed key stays removed across a save");
    Check(loaded.Get_Int("Options", "Credits", 0) == 500, "an integer survives a save and load");
    Check(!loaded.Get_Bool("Options", "Bases", true), "a boolean survives a save and load");
    Check(loaded.Get_Hex("Options", "Seed", 0) == 0x2A, "hex survives a save and load");

    return failures == 0 ? 0 : 1;
}

#ifndef GCODE_PARSER_H
#define GCODE_PARSER_H

// GCodeParser.h
// Simple single-line G-code parser supporting: M3 (with S 0-255), G28, G0, G1 (with optional F).
// Supports X and Y axes. Intended to be used with serial input lines.

#include <Arduino.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

class GCodeParser {
public:
    enum Type {
        TYPE_UNKNOWN = 0,
        TYPE_G0,
        TYPE_G1,
        TYPE_G28,
        TYPE_M3
    };

    struct Command {
        bool valid = false;
        Type type = TYPE_UNKNOWN;
        // Axis/parameters
        bool hasX = false;
        double x = 0.0;
        bool hasY = false;
        double y = 0.0;
        bool hasF = false;
        double f = 0.0;
        bool hasS = false;
        int s = 0; // 0..255 for M3
        String error; // non-empty when valid==false
    };

    // Parse a single line of gcode. Trims comments (starting with ';' or '(').
    // Returns a Command struct describing the parsed command or an error.
    static Command parseLine(const String& rawLine) {
        Command cmd;
        String line = stripComments(rawLine);
        line.trim();
        if (line.length() == 0) {
            cmd.valid = false;
            cmd.error = F("Empty line");
            return cmd;
        }

        // Tokenize: sequence of letter followed by optional signed/float number.
        // We'll collect numeric values for letters of interest.
        int gNumber = -1;
        int mNumber = -1;

        for (uint16_t i = 0; i < line.length(); ) {
            // skip whitespace
            char ci = line.charAt(i);
            if (isspace((unsigned char)ci)) { ++i; continue; }
            char letter = toupper((unsigned char)ci);
            if (!isalpha((unsigned char)letter)) {
                cmd.valid = false;
                cmd.error = F("Unexpected character in input");
                return cmd;
            }
            ++i;
            // collect number start
            uint16_t numStart = i;
            // allow sign, digits, decimal point, exponent
            if (i < line.length() && (line.charAt(i) == '+' || line.charAt(i) == '-')) ++i;
            bool sawDigits = false;
            while (i < line.length() && isdigit((unsigned char)line.charAt(i))) { ++i; sawDigits = true; }
            if (i < line.length() && line.charAt(i) == '.') {
                ++i;
                while (i < line.length() && isdigit((unsigned char)line.charAt(i))) { ++i; sawDigits = true; }
            }
            if (i < line.length() && (line.charAt(i) == 'e' || line.charAt(i) == 'E')) {
                uint16_t epos = i;
                ++i;
                if (i < line.length() && (line.charAt(i) == '+' || line.charAt(i) == '-')) ++i;
                bool expDigits = false;
                while (i < line.length() && isdigit((unsigned char)line.charAt(i))) { ++i; expDigits = true; }
                if (!expDigits) {
                    // malformed exponent: rollback to before 'e' (treat as end of token)
                    i = epos;
                }
            }

            String numberText;
            if (numStart < i) numberText = line.substring(numStart, i);

            // dispatch
            if (letter == 'G') {
                if (numberText.length() == 0) { cmd.valid = false; cmd.error = F("G with no number"); return cmd; }
                int val = (int)parseDouble(numberText, cmd.error);
                if (cmd.error.length() > 0) { cmd.valid = false; return cmd; }
                gNumber = val;
            } else if (letter == 'M') {
                if (numberText.length() == 0) { cmd.valid = false; cmd.error = F("M with no number"); return cmd; }
                int val = (int)parseDouble(numberText, cmd.error);
                if (cmd.error.length() > 0) { cmd.valid = false; return cmd; }
                mNumber = val;
            } else if (letter == 'X') {
                if (numberText.length() == 0) { cmd.valid = false; cmd.error = F("X with no value"); return cmd; }
                double v = parseDouble(numberText, cmd.error);
                if (cmd.error.length() > 0) { cmd.valid = false; return cmd; }
                cmd.hasX = true;
                cmd.x = v;
            } else if (letter == 'Y') {
                if (numberText.length() == 0) { cmd.valid = false; cmd.error = F("Y with no value"); return cmd; }
                double v = parseDouble(numberText, cmd.error);
                if (cmd.error.length() > 0) { cmd.valid = false; return cmd; }
                cmd.hasY = true;
                cmd.y = v;
            } else if (letter == 'F') {
                if (numberText.length() == 0) { cmd.valid = false; cmd.error = F("F with no value"); return cmd; }
                double v = parseDouble(numberText, cmd.error);
                if (cmd.error.length() > 0) { cmd.valid = false; return cmd; }
                if (v < 0.0) { cmd.valid = false; cmd.error = F("Feed rate F must be non-negative"); return cmd; }
                cmd.hasF = true;
                cmd.f = v;
            } else if (letter == 'S') {
                if (numberText.length() == 0) { cmd.valid = false; cmd.error = F("S with no value"); return cmd; }
                double v = parseDouble(numberText, cmd.error);
                if (cmd.error.length() > 0) { cmd.valid = false; return cmd; }
                int iv = (int)v;
                if (iv < 0 || iv > 255) { cmd.valid = false; cmd.error = F("S value out of range 0-255"); return cmd; }
                cmd.hasS = true;
                cmd.s = iv;
            } else {
                // ignore other letters gracefully (could be comments, tool number, etc.)
                // but if they had a number and we don't recognize it, just skip.
            }
        }

        // Determine command type priority: M takes precedence if M3, otherwise G settings.
        if (mNumber == 3) {
            cmd.type = TYPE_M3;
            // Accept M3 with or without S (S may be provided separately)
            cmd.valid = true;
            return cmd;
        }

        if (gNumber >= 0) {
            if (gNumber == 0) {
                cmd.type = TYPE_G0;
                cmd.valid = true;
                return cmd;
            } else if (gNumber == 1) {
                cmd.type = TYPE_G1;
                // F is optional
                cmd.valid = true;
                return cmd;
            } else if (gNumber == 28) {
                cmd.type = TYPE_G28;
                cmd.valid = true;
                return cmd;
            } else {
                cmd.valid = false;
                cmd.error = F("Unsupported G-code number");
                return cmd;
            }
        }

        cmd.valid = false;
        cmd.error = F("No supported G/M command found");
        return cmd;
    }

    // Convenience overloads for Arduino C-string inputs
    static Command parseLine(const char* rawLine) {
        return parseLine(String(rawLine ? rawLine : ""));
    }

private:
    // Helper: strip comments started by ';' or '(' (parenthesis-style comments)
    static String stripComments(const String& s) {
        // Copy up to ';'
        int pos = s.indexOf(';');
        String out = (pos >= 0) ? s.substring(0, pos) : s;
        // remove parentheses and anything inside them
        String tmp;
        tmp.reserve(out.length());
        bool inParen = false;
        for (uint16_t i = 0; i < out.length(); ++i) {
            char c = out.charAt(i);
            if (c == '(') { inParen = true; continue; }
            if (c == ')') { inParen = false; continue; }
            if (!inParen) tmp += c;
        }
        return tmp;
    }

    // Parse a floating point number from text. On error, sets err and returns 0.
    static double parseDouble(const String& text, String& err) {
        err = "";
        if (text.length() == 0) { err = F("Empty numeric field"); return 0.0; }
        const char* cstr = text.c_str();
        char* endptr = NULL;
        errno = 0;
        double val = strtod(cstr, &endptr);
        if (endptr == cstr) {
            err = F("Invalid number format");
            return 0.0;
        }
        if (errno == ERANGE) {
            err = F("Numeric out of range");
            return 0.0;
        }
        return val;
    }
};

#endif // GCODE_PARSER_H
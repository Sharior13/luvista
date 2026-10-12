#pragma once
#include <string>

// Passwords for the local profiles. Luvista never stores a password, only a "fingerprint" of it:
// the password is mixed with a random salt and run through PBKDF2-HMAC-SHA256 many times.
// This keeps a profile's password private on a shared computer. It is not meant to protect
// against someone who can read the whole disk (the music and the favorites are plain files anyway).
namespace Credentials {

    std::string makeFingerprint(const std::string& password);                      // "pbkdf2$<rounds>$<salt>$<hash>"
    bool matches(const std::string& password, const std::string& fingerprint);     // is it the same password?

    std::string sha256Hex(const std::string& text);                                // exposed for the tests
    std::string pbkdf2Hex(const std::string& password, const std::string& salt, int rounds, int bytes);

}

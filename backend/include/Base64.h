#ifndef BASE64_H
#define BASE64_H

#include <string>
#include <vector>

// Utility for encoding/decoding base64 strings used by the JSON API
// (photo upload) and binary export responses (PDF).
class Base64 {
public:
    static std::string encode(const std::string& binary);
    static std::string encode(const std::vector<unsigned char>& binary);
    static std::vector<unsigned char> decode(const std::string& encoded);
    static bool isValid(const std::string& encoded);
};

#endif // BASE64_H

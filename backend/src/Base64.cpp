#include "Base64.h"

namespace {

const char kAlphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int decodeChar(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

} // namespace

std::string Base64::encode(const std::string& binary) {
    return encode(std::vector<unsigned char>(binary.begin(), binary.end()));
}

std::string Base64::encode(const std::vector<unsigned char>& binary) {
    if (binary.empty()) {
        return "";
    }

    std::string out;
    out.reserve(((binary.size() + 2) / 3) * 4);

    std::size_t i = 0;
    while (i + 2 < binary.size()) {
        unsigned int n = (binary[i] << 16) | (binary[i + 1] << 8) | binary[i + 2];
        out.push_back(kAlphabet[(n >> 18) & 63]);
        out.push_back(kAlphabet[(n >> 12) & 63]);
        out.push_back(kAlphabet[(n >> 6) & 63]);
        out.push_back(kAlphabet[n & 63]);
        i += 3;
    }

    if (i < binary.size()) {
        unsigned int n = binary[i] << 16;
        out.push_back(kAlphabet[(n >> 18) & 63]);
        if (i + 1 < binary.size()) {
            n |= binary[i + 1] << 8;
            out.push_back(kAlphabet[(n >> 12) & 63]);
            out.push_back(kAlphabet[(n >> 6) & 63]);
            out.push_back('=');
        } else {
            out.push_back(kAlphabet[(n >> 12) & 63]);
            out.push_back('=');
            out.push_back('=');
        }
    }

    return out;
}

std::vector<unsigned char> Base64::decode(const std::string& encoded) {
    std::vector<unsigned char> out;
    if (encoded.empty()) {
        return out;
    }

    std::vector<int> vals;
    vals.reserve(encoded.size());
    for (char c : encoded) {
        if (c == '=' || c == '\r' || c == '\n' || c == ' ') {
            continue;
        }
        int v = decodeChar(c);
        if (v < 0) {
            return {};
        }
        vals.push_back(v);
    }

    for (std::size_t i = 0; i + 3 < vals.size(); i += 4) {
        unsigned int n = (vals[i] << 18) | (vals[i + 1] << 12) |
                         (vals[i + 2] << 6) | vals[i + 3];
        out.push_back(static_cast<unsigned char>((n >> 16) & 0xFF));
        out.push_back(static_cast<unsigned char>((n >> 8) & 0xFF));
        out.push_back(static_cast<unsigned char>(n & 0xFF));
    }

    std::size_t rem = vals.size() % 4;
    if (rem >= 2) {
        unsigned int n = (vals[vals.size() - rem] << 18) |
                         (vals[vals.size() - rem + 1] << 12);
        out.push_back(static_cast<unsigned char>((n >> 16) & 0xFF));
        if (rem == 3) {
            n |= vals[vals.size() - 1] << 6;
            out.push_back(static_cast<unsigned char>((n >> 8) & 0xFF));
        }
    }

    return out;
}

bool Base64::isValid(const std::string& encoded) {
    if (encoded.empty()) {
        return true;
    }
    for (char c : encoded) {
        if (c == '=' || c == '\r' || c == '\n' || c == ' ') {
            continue;
        }
        if (decodeChar(c) < 0) {
            return false;
        }
    }
    return true;
}

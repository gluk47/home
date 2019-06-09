#pragma once

String operator"" _str(const char* s, size_t) {
    return String(s);
}

template <typename T>
String ToString(T&& v) {
    String ret;
    ret += v;
    return ret;
}

inline const char* YesNo(bool condition) {
    return condition ? "yes" : "no";
}

inline const char* OnOff(bool condition) {
    return condition ? "on" : "off";
}


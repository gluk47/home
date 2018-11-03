#pragma once

String operator"" _str(const char* s, size_t) {
    return String(s);
}

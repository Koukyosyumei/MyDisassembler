#include <iostream>
#include <string>

int utf8ToDecimal(const std::string& utf8_str) {
    int result = 0;
    for (unsigned char c : utf8_str) {
        result = (result << 8) + c;
    }
    return result;
}

int main() {
    std::string utf8_str = "\x81";
    int decimal_value = utf8ToDecimal(utf8_str);
    std::cout << "Decimal value: " << decimal_value << std::endl;
    return 0;
}

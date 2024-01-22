#include <ncurses.h>
#include <string>
void Print(const wchar_t *ptr,short length) {
  for (int i = 0;i < length;i++) {
    printw("%lc",ptr[i]);
  }
  refresh();
}
constexpr short CharToShort(char c) {
  return -48 + c;
}
short StringToShort(std::string text) {
  int value = CharToShort(text[0]);
  for (int i = 1;i < text.size();i++) {
    value *= 10;
    value += CharToShort(text[i]);
  }
  return value;
}
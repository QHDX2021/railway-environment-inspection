#pragma once
#include <iostream>
#include <stdexcept>
#define CHECK(expr) do { if (!(expr)) throw std::runtime_error(std::string("CHECK failed: ") + #expr); } while (false)
#define CHECK_EQ(a,b) CHECK((a) == (b))
inline int test_main(const char* name, void (*fn)()) { try { fn(); std::cout << name << " PASS\n"; return 0; } catch (const std::exception& e) { std::cerr << name << " FAIL: " << e.what() << "\n"; return 1; } }

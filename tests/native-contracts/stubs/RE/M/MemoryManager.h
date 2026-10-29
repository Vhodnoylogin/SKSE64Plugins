#pragma once
#include <new>
// Test boundary only: never resolve an engine heap or load a game/plugin.
#define TES_HEAP_REDEFINE_NEW() \
    static void* operator new(std::size_t count) { return ::operator new(count); } \
    static void operator delete(void* memory) { ::operator delete(memory); }

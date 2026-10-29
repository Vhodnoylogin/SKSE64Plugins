#include <Windows.h>
#include <cstdint>
#include <iostream>

// Separate-module runtime view: exact global names, inheritance and virtual slot
// order from ianpatt/skse64 PapyrusObjects.h/PapyrusDelayFunctors.h. The opaque
// serialization/VM value parameter types are never called by this RTTI test.
struct SKSESerializationInterface;
class VMValue;
class ISKSEObject {
public:
    virtual ~ISKSEObject() = default;
    virtual const char* ClassName() const = 0;
    virtual std::uint32_t ClassVersion() const = 0;
    virtual bool Save(SKSESerializationInterface*) = 0;
    virtual bool Load(SKSESerializationInterface*, std::uint32_t) = 0;
};
class ISKSEDelayFunctor : public ISKSEObject {
public:
    virtual ~ISKSEDelayFunctor() = default;
    virtual void Run(VMValue&) = 0;
    virtual bool ShouldReschedule(std::int32_t&) = 0;
    virtual bool ShouldResumeStack(std::uint32_t&) = 0;
};
int main(int argc, char** argv) {
    if (argc != 2) return 2;
    const auto module = LoadLibraryA(argv[1]);
    if (!module) return 3;
    const auto create = reinterpret_cast<void* (*)()>(GetProcAddress(module, "create_functor"));
    const auto release = reinterpret_cast<void (*)(void*)>(GetProcAddress(module, "release_functor"));
    if (!create || !release) return 4;
    auto* object = static_cast<ISKSEObject*>(create());
    auto* functor = dynamic_cast<ISKSEDelayFunctor*>(object);
    std::uint32_t stack = 0;
    const bool passed = functor && functor->ShouldResumeStack(stack) && stack == 47;
    release(object);
    FreeLibrary(module);
    if (!passed) return 1;
    std::cout << "cross-DLL global RTTI and resume-slot contract passed\n";
}

#include "SKEEDelayFunctors.h"

class ProbeFunctor : public SKSE::LatentSKSEDelayFunctor {
public:
    ProbeFunctor() : LatentSKSEDelayFunctor(47) {}
    const char* ClassName() const override { return "ProbeFunctor"; }
    std::uint32_t ClassVersion() const override { return 1; }
    void Run(RE::BSScript::Variable&) override {}
};
extern "C" __declspec(dllexport) void* create_functor() {
    return static_cast<ISKSEObject*>(new ProbeFunctor());
}
extern "C" __declspec(dllexport) void release_functor(void* object) {
    delete static_cast<ISKSEObject*>(object);
}

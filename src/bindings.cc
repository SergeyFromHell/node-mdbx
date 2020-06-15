#include <napi.h>

#include "cpp_mdbx.h"

using namespace Napi;

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    Napi::String name = Napi::String::New(env, "CppMdbx");
    exports.Set(name, CppMdbx::GetClass(env));
    return exports;
}

NODE_API_MODULE(addon, Init)

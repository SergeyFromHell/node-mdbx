#include "cpp_mdbx.h"

CppMdbx::CppMdbx(const Napi::CallbackInfo& info) : ObjectWrap(info) {
    Napi::Env env = info.Env();

    Napi::Object options = info[0].As<Napi::Object>();

    // Napi::Array keys = options.GetPropertyNames();
    // printf("Passed keys: %u\n", keys.Length());

    // for (uint32_t i = 0; i < keys.Length(); ++i) {
    //     auto key = keys.Get(i);
    //     auto value = options.Get(key);
    //     std::string keyStr = key.ToString();
    //     std::string valueStr = value.ToString();
    //     printf("%s => %s\n", keyStr.c_str(), valueStr.c_str());
    // }
}

Napi::Value CppMdbx::Open(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    //return Napi::String::New(env, this->_dbPath);
}

Napi::Value CppMdbx::Close(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
}

Napi::Function CppMdbx::GetClass(Napi::Env env) {
    return DefineClass(env, "CppMdbx", {
        CppMdbx::InstanceMethod("open", &CppMdbx::Open),
        CppMdbx::InstanceMethod("close", &CppMdbx::Close),
    });
}
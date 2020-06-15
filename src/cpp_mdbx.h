#pragma once

#include <napi.h>
#include "mdbx.h"

class CppMdbx : public Napi::ObjectWrap<CppMdbx>
{
public:
    CppMdbx(const Napi::CallbackInfo&);
    Napi::Value Open(const Napi::CallbackInfo&);
    Napi::Value Close(const Napi::CallbackInfo&);

    static Napi::Function GetClass(Napi::Env);

private:
    std::string _dbPath;
};

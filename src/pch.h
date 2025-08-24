#pragma once

#pragma warning(push)
#include <RE/Skyrim.h>
#include <REL/Relocation.h>
#include <SKSE/SKSE.h>

#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <atomic>
#include "HudManager.h"
#include "PrismaUI_API.h"
#include "Utils.h"


#ifdef NDEBUG
#    include <spdlog/sinks/basic_file_sink.h>
#else
#    include <spdlog/sinks/msvc_sink.h>
#endif
#pragma warning(pop)

using namespace std::literals;
using namespace std;

namespace logger = SKSE::log;

namespace util
{
    using SKSE::stl::report_and_fail;
}

#define DLLEXPORT __declspec(dllexport)

#define RELOCATION_OFFSET(SE, AE) REL::VariantOffset(SE, AE, 0).offset()


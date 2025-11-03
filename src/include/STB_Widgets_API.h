/*
* For modders: Copy this file into your own project if you wish to use this API.
*/
#pragma once

#include <functional>
#include <queue>
#include <stdint.h>
#include <iostream>
#include <PCH.h>

typedef uint64_t PrismaView;

namespace STB_UI_API
{
	constexpr const auto STBUIPluginName = "STB_Widgets";

	using PluginHandle = SKSE::PluginHandle;
	using ActorHandle = RE::ActorHandle;

	enum class InterfaceVersion : uint8_t
	{
		V1
	};

	// PrismaUI modder interface v1
	class IVPrismaUI1
	{
	public:

		// Get view order.
		virtual void GetView(uint64_t view) noexcept = 0;
	};

	typedef void* (*_RequestPluginAPI)(const InterfaceVersion interfaceVersion);

	/// <summary>
	/// Request the PrismaUI API interface.
	/// Recommended: Send your request during or after SKSEMessagingInterface::kMessage_PostLoad to make sure the dll has already been loaded
	/// </summary>
	/// <param name="a_interfaceVersion">The interface version to request</param>
	/// <returns>The pointer to the API singleton, or nullptr if request failed</returns>
	[[nodiscard]] inline void* RequestPluginAPI(const InterfaceVersion a_interfaceVersion = InterfaceVersion::V1)
	{
		auto pluginHandle = GetModuleHandle("STB_Widgets.dll");
		_RequestPluginAPI requestAPIFunction = (_RequestPluginAPI)GetProcAddress(pluginHandle, "RequestPluginAPI");

		if (requestAPIFunction) {
			return requestAPIFunction(a_interfaceVersion);
		}

		return nullptr;
	}
}

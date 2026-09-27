#include "ModConfigUI.h"
#include "Serialization.h"
#include "Settings.h"

#include "Hooks_FavoritesHandler.h"
#include "Hooks_FavoritesMenu.h"

#include "Version.h"

static void MessageHandler(SKSE::MessagingInterface::Message* a_message)
{
	if (a_message->type == SKSE::MessagingInterface::kPostLoad)
	{
		EHKS::InstallModConfigUI();
	}
}

extern "C"
{
	DLLEXPORT bool SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
	{
		a_info->infoVersion = SKSE::PluginInfo::kVersion;
		a_info->name = Version::NAME.data();
		a_info->version = REL::Version{ Version::MAJOR, Version::MINOR, Version::PATCH, 0 }.pack();

		if (a_skse->IsEditor())
		{
			logger::critical("Loaded in editor, marking as incompatible"sv);
			return false;
		}

		const auto ver = a_skse->RuntimeVersion();
		if (ver < SKSE::RUNTIME_SSE_1_5_39)
		{
			logger::critical("Unsupported runtime version {}", ver.string());
			return false;
		}

		return true;
	}

	DLLEXPORT bool SKSEPlugin_Load(SKSE::LoadInterface* a_skse)
	{
		SKSE::InitInfo initInfo{};
		initInfo.logLevel = REX::ELogLevel::Trace;
		initInfo.logPattern = "%s(%#): [%^%l%$] %v";
		initInfo.trampoline = true;
		initInfo.trampolineSize = 1 << 5;
		SKSE::Init(a_skse, initInfo);

		logger::info("{} v{} -({})", Version::FORMATTED_NAME, Version::STRING, __TIMESTAMP__);

		auto serialization = SKSE::GetSerializationInterface();
		if (!serialization)
		{
			return false;
		}
		serialization->SetUniqueID('EHKS');
		serialization->SetSaveCallback(EHKS::SaveCallback);
		serialization->SetLoadCallback(EHKS::LoadCallback);

		EHKS::LoadSettings();
		logger::info("Settings loaded.");

		EHKS::LoadSettings();

		const SKSE::MessagingInterface* messaging = SKSE::GetMessagingInterface();
		if (!messaging->RegisterListener("SKSE", MessageHandler))
		{
			logger::critical("Messaging interface registration failed.");
			return false;
		}

		EHKS::FavoritesHandlerEx::InstallHook();
		EHKS::FavoritesMenuEx::InstallHook();

		logger::info("Hooks installed.");

		return true;
	}
};

#include "logger.h"
#include "Manager.h"

namespace
{
	void OnMessage(SKSE::MessagingInterface::Message* a_message)
	{
		switch (a_message->type) {
		case SKSE::MessagingInterface::kNewGame:
			ReloadCleanup::OnNewGame();
			break;
		case SKSE::MessagingInterface::kPreLoadGame:
			ReloadCleanup::OnPreLoadGame(static_cast<const char*>(a_message->data));
			break;
		case SKSE::MessagingInterface::kPostLoadGame:
			ReloadCleanup::OnPostLoadGame(a_message->data != nullptr);
			break;
		default:
			break;
		}
	}
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {

	SetupLog();
	logger::info("Plugin loaded");
    SKSE::Init(skse);
	SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
	return true;
}

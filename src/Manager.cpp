#include "Manager.h"

namespace
{
	bool          g_hasLoadedGame{ false };
	std::uint64_t g_reloadNumber{ 0 };

	struct RunningScriptState
	{
		std::size_t   runningStacks{ 0 };
		std::size_t   waitingLatentReturns{ 0 };
		std::uint32_t frozenStacks{ 0 };
	};

	[[nodiscard]] RunningScriptState GetRunningScriptState(RE::BSScript::Internal::VirtualMachine* a_vm)
	{
		RunningScriptState result;
		{
			RE::BSSpinLockGuard lock{ a_vm->runningStacksLock };
			result.runningStacks = a_vm->allRunningStacks.size();
			result.waitingLatentReturns = a_vm->waitingLatentReturns.size();
		}
		{
			RE::BSSpinLockGuard lock{ a_vm->frozenStacksLock };
			result.frozenStacks = a_vm->frozenStacksCount;
		}
		return result;
	}
}

namespace ReloadCleanup
{
	void OnNewGame()
	{
		g_hasLoadedGame = true;
	}

	void OnPreLoadGame(const char* a_savePath)
	{
		const auto savePath = a_savePath ? std::string_view{ a_savePath } : std::string_view{ "<unknown>" };
		if (!g_hasLoadedGame) {
			return;
		}

		auto* virtualMachine = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		if (!virtualMachine) {
			logger::error(
				"Cannot clean before reload '{}': Papyrus VirtualMachine is unavailable",
				savePath);
			return;
		}

		const auto reloadNumber = ++g_reloadNumber;
		const auto before = GetRunningScriptState(virtualMachine);
		const auto started = std::chrono::steady_clock::now();
		virtualMachine->CleanupSave();
		virtualMachine->DropAllRunningData();
		const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - started);
		const auto after = GetRunningScriptState(virtualMachine);

		logger::info(
			"Reload cleanup #{} completed for '{}': CleanupSave and DropAllRunningData executed in {} ms; "
			"Papyrus running {}->{}, latent {}->{}, frozen {}->{}",
			reloadNumber,
			savePath,
			elapsed.count(),
			before.runningStacks,
			after.runningStacks,
			before.waitingLatentReturns,
			after.waitingLatentReturns,
			before.frozenStacks,
			after.frozenStacks);
	}

	void OnPostLoadGame(bool a_succeeded)
	{
		if (a_succeeded) {
			g_hasLoadedGame = true;
		}
	}
}

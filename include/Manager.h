#pragma once

namespace ReloadCleanup
{
	void OnNewGame();
	void OnPreLoadGame(const char* a_savePath);
	void OnPostLoadGame(bool a_succeeded);
}

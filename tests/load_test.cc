// Loads SDTaxi.asi into a process that isn't the game: it must not crash, must write its default ini and
// must report the game functions missing. argv[1] = path to the .asi (build.ps1 passes a sandbox copy).

#include <Windows.h>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

int main(int argc, char** argv)
{
	if (argc < 2) {
		std::printf("usage: load_test <SDTaxi.asi>\n");
		return 2;
	}

	std::string dir = argv[1];
	dir = dir.substr(0, dir.find_last_of("\\/") + 1);

	// The sandbox outlives a run: start without an earlier run's files, so the defaults are what's tested.
	DeleteFileA((dir + "SDTaxi.ini").c_str());
	DeleteFileA((dir + "SDTaxi.log").c_str());
	DeleteFileA((dir + "SDTaxi-console.sk").c_str());

	HMODULE module = LoadLibraryA(argv[1]);
	if (!module) {
		std::printf("FAIL: LoadLibrary error %lu\n", GetLastError());
		return 1;
	}

	std::ifstream ini(dir + "SDTaxi.ini");
	if (!ini) {
		std::printf("FAIL: no default SDTaxi.ini written\n");
		return 1;
	}

	std::ifstream log(dir + "SDTaxi.log");
	std::stringstream text;
	text << log.rdbuf();
	const std::string contents = text.str();
	std::printf("%s", contents.c_str());

	const char* expected[] = {
		"SDTaxi loaded (Contact=1 Name=\"Taxi\" Vehicle=object-physical-vehicle-625MHCTaxi01 Driver=object-physical-character-TaxiDriverMale ScriptPrints=1 RideLog=1 Console=0 ConsoleKey=0x7A)",
		"UFG::ScriptCache::GetScript: 0 matches",
		"skookum: game functions MISSING or not as expected, no scripts",
		"taxi: no scripts, no taxis",
		"phone: contact list functions MISSING or not as expected (or no taxi), no taxi contact",
		"console: off",
		"ride: transit functions MISSING (or no script tick), no ride log",
	};
	for (const char* line : expected) {
		if (contents.find(line) == std::string::npos) {
			std::printf("FAIL: log lacks \"%s\"\n", line);
			return 1;
		}
	}

	std::printf("PASS\n");
	return 0;
}

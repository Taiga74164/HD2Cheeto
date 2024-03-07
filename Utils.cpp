#include "Utils.h"
#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include <codecvt>
#include <cstdarg>
#include <format>
#include <psapi.h>

std::mutex _mutex;

namespace Utils
{
    void AttachConsole()
    {
		AllocConsole();
		freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
		freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
		freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
		SetConsoleOutputCP(CP_UTF8);
    }

    void DetachConsole()
    {
		fclose(stdin);
		fclose(stdout);
		fclose(stderr);
		FreeConsole();
    }

	void ConsolePrint(const char* filepath, int line, const char* fmt, ...)
	{
		char buf[4096];

		va_list va;
		va_start(va, fmt);
		vsprintf_s(buf, fmt, va);
		va_end(va);

		const std::lock_guard<std::mutex> lock(_mutex);

		auto filename = std::filesystem::path(filepath).filename().string();
		auto logLineConsole = string_format("[%s:%d] %s", filename.c_str(), line, buf);
		auto str = (logLineConsole + std::string(fmt)).c_str();

		std::cout << logLineConsole << std::endl;
	}

	void ConsolePrint(const char* filepath, int line, const wchar_t* fmt, ...)
	{
		wchar_t buf[4096];

		va_list va;
		va_start(va, fmt);
		vswprintf_s(buf, fmt, va);
		va_end(va);

		const std::lock_guard<std::mutex> lock(_mutex);

		auto filename = std::filesystem::path(filepath).filename().string();
		auto logLineConsole = string_format("[%s:%d] %s", filename.c_str(), line, buf);
		auto str = (logLineConsole + to_string(std::wstring(fmt))).c_str();

		std::cout << logLineConsole << std::endl;
	}

    void ClearConsole()
    {
        DWORD n;                         /* Number of characters written */
        DWORD size;                      /* number of visible characters */
        COORD coord = { 0 };               /* Top left screen position */
        CONSOLE_SCREEN_BUFFER_INFO csbi;

        /* Get a handle to the console */
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

        GetConsoleScreenBufferInfo(h, &csbi);

        /* Find the number of characters to overwrite */
        size = csbi.dwSize.X * csbi.dwSize.Y;

        /* Overwrite the screen buffer with whitespace */
        FillConsoleOutputCharacter(h, TEXT(' '), size, coord, &n);
        GetConsoleScreenBufferInfo(h, &csbi);
        FillConsoleOutputAttribute(h, csbi.wAttributes, size, coord, &n);

        /* Reset the cursor to the top left position */
        SetConsoleCursorPosition(h, coord);
    }

    char ConsoleReadKey()
    {
		auto key = char{ 0 };
		auto keysread = DWORD{ 0 };

		//ReadConsoleA(_in, &key, 1, &keysread, nullptr);
		return std::cin.get();
    }

    std::string GetAddressModuleName(uintptr_t address)
    {
        std::vector<MODULEENTRY32> Modules{};

        static DWORD pid = GetCurrentProcessId();
        MODULEENTRY32 mod{};
        mod.dwSize = sizeof(mod);
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
        for (Module32First(snap, &mod); Module32Next(snap, &mod);)
        {
            if (mod.th32ProcessID != pid)
                continue;

            Modules.emplace_back(mod);
        }
        CloseHandle(snap);

        for (const auto& it : Modules)
        {
            if (address >= (uintptr_t)it.modBaseAddr && address <= (uintptr_t)it.modBaseAddr + it.modBaseSize)
                return it.szModule;
        }

        return "unknown";
    }

	std::wstring GetCurrentProcessNameW()
	{
		DWORD processID = GetCurrentProcessId();
		HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

		if (processHandle != NULL)
		{
			WCHAR processName[MAX_PATH] = L"<unknown>";
			GetModuleBaseNameW(processHandle, NULL, processName, MAX_PATH);
			CloseHandle(processHandle);
			return std::wstring(processName);
		}

		// In case the handle could not be opened, return an error message.
		return L"Unable to retrieve process name.";
	}

	std::string GetCurrentProcessNameA()
	{
		return to_string(GetCurrentProcessNameW());
	}
	
	std::string to_string(std::wstring wstr)
    {
    	std::wstring_convert<std::codecvt_utf8<wchar_t>> strconverter;
    	return strconverter.to_bytes(wstr);
    }

	std::wstring to_wstring(std::string str)
    {
    	std::wstring_convert<std::codecvt_utf8<wchar_t>> strconverter;
    	return strconverter.from_bytes(str);
    }
}
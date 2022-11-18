#include <string>
#include <iostream>

#include <Windows.h>
#include <TlHelp32.h>

int main(int* argc, char* argv[]) {
	const char* dll_path = argv[1];
	HANDLE snapshot = 0;
	PROCESSENTRY32 pe32 = { 0 };

	std::cout << "[*] Using DLL @ \"" << dll_path << "\"" << std::endl;

	pe32.dwSize = sizeof(PROCESSENTRY32);

	snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	Process32First(snapshot, &pe32);

	do {
		if (strcmp(pe32.szExeFile, "Trove.exe") == 0) {
			HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, true, pe32.th32ProcessID);
			void* base_address = VirtualAllocEx(process, NULL, strlen(dll_path) + 1, MEM_COMMIT, PAGE_READWRITE);

			WriteProcessMemory(process, base_address, dll_path, strlen(dll_path) + 1, NULL);

			HMODULE module = GetModuleHandle("kernel32.dll");
			HANDLE thread = CreateRemoteThread(process, NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(GetProcAddress(module, "LoadLibraryA")), base_address, 0, NULL);

			WaitForSingleObject(thread, INFINITE);
			GetExitCodeThread(thread, 0);

			VirtualFreeEx(process, base_address, 0, MEM_RELEASE);
			CloseHandle(thread);
			CloseHandle(process);
			return 0;
		}
	} while (Process32Next(snapshot, &pe32));

	return -1;
}

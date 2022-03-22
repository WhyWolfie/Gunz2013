#include <stdio.h>
#include <string.h>

#include <vector>
using namespace std;

#include <windows.h>

#define _GUNZ_VER 	10213

#define MAX_CODE_LENGTH	64

struct MemCode
{
	unsigned long addr;
	
	unsigned char before[MAX_CODE_LENGTH];
	unsigned char after[MAX_CODE_LENGTH];
	int size;
	
	bool done;
};

void InitMemCodeArray(vector<MemCode> *pOut)
{
	#define PUSH_MEMCODE(_addr, _old, _new, _size)			\
	{														\
		MemCode node = {_addr, _old, _new, _size, false};	\
		pOut->push_back(node);								\
	}
	
	#if (_GUNZ_VER == 10213)	// GunZ Ultra.
	#ifdef _DEBUG
	// Allow multiple-run.
	PUSH_MEMCODE(0x0050412d, "\x75", "\xEB", 1);
	PUSH_MEMCODE(0x0050415a, "\x75", "\xEB", 1);
	#endif
	// Skip XTrap launching.
	PUSH_MEMCODE(0x0050409a, "\xe8\x81\x64\x12\x00", "\xeb\x03\x90\x90\x90", 5);
	// Disable WPE PRO hacking preventation.
	PUSH_MEMCODE(0x00452f7c, "\x75", "\xeb", 1);
	PUSH_MEMCODE(0x00453716, "\x75", "\xeb", 1);
	PUSH_MEMCODE(0x00453f20, "\x06", "\x00", 1);
	PUSH_MEMCODE(0x00453f25, "\x75", "\xeb", 1);
	// Ignore DLL hacking message.
	PUSH_MEMCODE(0x0050442c, "\x01\x02\x00\x00", "\x00\x00\x00\x00", 4);
	PUSH_MEMCODE(0x00504445, "\xe8\x01\x00\x00", "\x00\x00\x00\x00", 4);
	// Don't use MAIET Encrypted File System.
	PUSH_MEMCODE(0x00707ced, "\x2e", "\x00", 1);
	PUSH_MEMCODE(0x00707cd3, "\x2e", "\x00", 1);
	PUSH_MEMCODE(0x00707cbc, "\x2e", "\x00", 1);
	PUSH_MEMCODE(0x00707ca7, "\x2e", "\x00", 1);
	PUSH_MEMCODE(0x00707ee7, "\x2e", "\x00", 1);
	PUSH_MEMCODE(0x0070a0c8, "\x2e", "\x00", 1);
	PUSH_MEMCODE(0x005974fd, "\x74", "\xeb", 1);
	PUSH_MEMCODE(0x00597c5c, "\x0f\x84\x91\x00\x00\x00", "\xe9\x92\x00\x00\x00\x90", 6);
	// I'm NOT hacker.
	PUSH_MEMCODE(0x00504466, "\x75", "\xeb", 1);
	// Don't force using launcher to game run.
	PUSH_MEMCODE(0x0050550e, "\x7f", "\x00", 1);
	// Fix respawn bug.
	PUSH_MEMCODE(0x0052e31b, "\xae\x01\x00\x00", "\x00\x00\x00\x00", 4);
	PUSH_MEMCODE(0x0052edb2, "\x8d\x02\x00\x00", "\x00\x00\x00\x00", 4);
	// Continue even invalid abuse.txt file.
	PUSH_MEMCODE(0x00506655, "\x8a\x01\x00\x00", "\x00\x00\x00\x00", 4);
	PUSH_MEMCODE(0x00506669, "\x76\x01\x00\x00", "\x00\x00\x00\x00", 4);
	// Don't check file CRCs with filelist.xml.
	PUSH_MEMCODE(0x00597dc0, "\x46\xfe\xff\xff", "\x00\x00\x00\x00", 4);
	// Fix terminating game.
	PUSH_MEMCODE(0x00699b0e, "\x75", "\xeb", 1);
	PUSH_MEMCODE(0x0062a975, "\xe8\x66\xf0\x06\x00", "\xeb\x03\x90\x90\x90", 5);
	// Remove some hacking protection.
	PUSH_MEMCODE(0x00454c96, "\x0f\x84\x98\x00\x00\x00", "\xe9\x99\x00\x00\x00\x90", 6);
	PUSH_MEMCODE(0x00498bfc, "\x74", "\xeb", 1);
	// Skip XTrap engine test.
	PUSH_MEMCODE(0x005045ba, "\xe8\x23\x63\x12\x00", "\xeb\x03\x90\x90\x90", 5);
	// Enable LoginID & LoginPassword type box. (Removed due they're doesn't send ID/PW to the server.)
	/*
	PUSH_MEMCODE(0x00453e0b, "\x57\x57", "\xeb\x0c", 2);
	PUSH_MEMCODE(0x00453e27, "\x57\x57", "\xeb\x0c", 2);
	PUSH_MEMCODE(0x00453e5f, 
		"\x83\xec\x1c\x8b\xcc\x89\xa5\xcc\xfe\xff\xff\x6a\x07\xbb\x0f\x00\x00\x00\x89\x59\x18\x89\x79\x14\x68\x9c\x52\x70\x00\xc6\x41\x04\x00",
		"\x57\x6a\x01\x68\x9c\x52\x70\x00\x8b\xcb\xe8\x72\x70\xff\xff\x57\x6a\x01\x68\x6c\x3d\x70\x00\x8b\xcb\xe8\x63\x70\xff\xff\xeb\x4d\x90", 
	33);
	*/
	#else
	#ifdef _DEBUG
	// Allow multiple-run.
	PUSH_MEMCODE(0x0057ABCD, "\x75", "\xEB", 1);
	PUSH_MEMCODE(0x0057AC15, "\x75", "\xEB", 1);
	#endif
	// Skip XTrap launching.
	PUSH_MEMCODE(0x0057AB37, "\xE8\xB4\x43\x18\x00", "\xeb\x03\x90\x90\x90", 5);
	// Disable WPE PRO hacking preventation.
	PUSH_MEMCODE(0x00492610, "\x75", "\xeb", 1);
	PUSH_MEMCODE(0x0049359C, "\x75", "\xeb", 1);
	PUSH_MEMCODE(0x00495A16, "\x0c", "\x00", 1);
	PUSH_MEMCODE(0x00495A21, "\x75", "\xeb", 1);
	// Ignore DLL hacking message.
	PUSH_MEMCODE(0x0057AFDE, "\x1b", "\x00", 1);
	PUSH_MEMCODE(0x0057AFF8, "\x75", "\xeb", 1);
	// Don't use MAIET Encrypted File System.
	PUSH_MEMCODE(0x7db867, "\x2e", "\x00", 1);
	PUSH_MEMCODE(0x7db87c, "\x2e", "\x00", 1);
	PUSH_MEMCODE(0x7db893, "\x2e", "\x00", 1);
	PUSH_MEMCODE(0x7db8ad, "\x2e", "\x00", 1);
	PUSH_MEMCODE(0x7dbb2f, "\x2e", "\x00", 1);
	PUSH_MEMCODE(0x7de41c, "\x2e", "\x00", 1);
	PUSH_MEMCODE(0x00675FB8, "\xB0\x01", "\x32\xc0", 2);
	// I'm NOT hacker.
	PUSH_MEMCODE(0x0057B037, "\x75", "\xeb", 1);
	// Don't force using launcher to game run.
	PUSH_MEMCODE(0x00615451, "\x2b", "\x00", 1);
	// Fix respawn bug.
		// PUSH_MEMCODE(0x0052e31b, "\xae\x01\x00\x00", "\x00\x00\x00\x00", 4);
		// PUSH_MEMCODE(0x0052edb2, "\x8d\x02\x00\x00", "\x00\x00\x00\x00", 4);
	// Continue even invalid abuse.txt file.
	PUSH_MEMCODE(0x0057E723, "\x14", "\x00", 1);
	PUSH_MEMCODE(0x0057E732, "\x0F\x85\xE1\x00\x00\x00", "\xe9\xe2\x00\x00\x00\x90", 6);
	// Don't check file CRCs with filelist.xml.
	PUSH_MEMCODE(0x0067634F, "\x74", "\xeb", 1);
	// Fix terminating game.
	PUSH_MEMCODE(0x007965DE, "\x75", "\xeb", 1);
	PUSH_MEMCODE(0x006FF345, "\xE8\x66\x71\x09\x00", "\xeb\x03\x90\x90\x90", 5);
	// Remove some hacking protection.
	PUSH_MEMCODE(0x004A4210, "\x0F\x84\xDF\x00\x00\x00", "\xE9\xE0\x00\x00\x00\x90", 6);
	PUSH_MEMCODE(0x004F6AAB, "\x74", "\xeb", 1);
	// Skip XTrap engine test.
	PUSH_MEMCODE(0x0057B1F6, "\xE8\xB7\x40\x18\x00", "\xeb\x03\x90\x90\x90", 5);
	// Enable LoginID & LoginPassword type box. (Removed due they're doesn't send ID/PW to the server.)
		/*
		PUSH_MEMCODE(0x00453e0b, "\x57\x57", "\xeb\x0c", 2);
		PUSH_MEMCODE(0x00453e27, "\x57\x57", "\xeb\x0c", 2);
		PUSH_MEMCODE(0x00453e5f, 
			"\x83\xec\x1c\x8b\xcc\x89\xa5\xcc\xfe\xff\xff\x6a\x07\xbb\x0f\x00\x00\x00\x89\x59\x18\x89\x79\x14\x68\x9c\x52\x70\x00\xc6\x41\x04\x00",
			"\x57\x6a\x01\x68\x9c\x52\x70\x00\x8b\xcb\xe8\x72\x70\xff\xff\x57\x6a\x01\x68\x6c\x3d\x70\x00\x8b\xcb\xe8\x63\x70\xff\xff\xeb\x4d\x90", 
		33);
		*/
	#endif
}

#define CLIENT_FILENAME	".\\GunzRun.exe"

int main(int argc, char **argv)
{
	printf("Initializing...\n");
	
	// user id/password.
	char szUserID[64] = "", szPassword[64] = "";
	
	if(argc == 3)
	{
		// check string length, 1 is ID and 2 is PW.
		if(strlen(argv[1]) >= 64 || strlen(argv[2]) >= 64)
		{
			printf("User ID or Password string is too long.");
			return -1;
		}
		
		// save id/pw.
		strcpy(szUserID, argv[1]);
		strcpy(szPassword, argv[2]);
		
		#ifdef _DEBUG
		printf("User ID : %s - Password : %s\n", szUserID, szPassword);
		#endif
	}
	else
	{
		printf("No user ID/Password is given.\n");
	}
	
	// memory code.
	vector<MemCode> vtCode;
	InitMemCodeArray(&vtCode);
	
	printf("Launching %s...\n", CLIENT_FILENAME);
	
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	memset(&si, 0, sizeof(STARTUPINFO));
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	
	// char szCommandLine[] = CLIENT_FILENAME;
	
	char szCommandLine[256];
	sprintf(szCommandLine, "%s %s|%s|0", CLIENT_FILENAME, szUserID, szPassword);
	
	if(CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi) != 0)
	{
		printf("Modifying process memory...\n");
		
		while(true)
		{
			bool bDone = true;
			
			for(vector<MemCode>::iterator i = vtCode.begin(); i != vtCode.end(); i++)
			{
				MemCode *pCode = &(*i);
				if(pCode->done == true) continue;
				
				unsigned char nRead[MAX_CODE_LENGTH];
				if(ReadProcessMemory(pi.hProcess, (LPVOID)pCode->addr, &nRead, pCode->size, NULL) == 0)
				{
					printf("fatal error.\n");
					break;
				}
				else
				{
					if(memcmp(pCode->before, nRead, pCode->size) == 0)
					{
						WriteProcessMemory(pi.hProcess, (LPVOID)pCode->addr, pCode->after, pCode->size, NULL);
						printf("%x : applied new memory code.\n", (unsigned int)pCode->addr);
						pCode->done = true;
					}
					else
					{
						bDone = false;
					}
				}
			}
			
			if(bDone == true) break;
			
			Sleep(1);
		}
	}
	else
	{
		printf("Couldn't launch the game.\n");
		return -1;
	}
	
	printf("finished.\n");
	
	return 0;
}

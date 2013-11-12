/*
================================================================================
 PS ISO Tool (Supports PS1/PS2/PS3/PSP disc images) (CaptainCPS-X, 2013)
================================================================================

 This tool will parse any of the supported PlayStation ISO / BIN. The module will 
 also check for valid (ISO9660 / MODE1 / 2048) or (MODE2 / 2352).

 When used as library for an application this can:

 - [PS1 / PS2] Get game Title ID from SYSTEM.CNF and obtain Title from a text database.
 - [PS3 / PSP] Get game Title and ID from the PARAM.SFO inside the ISO (no need for text database).
 - Titles gets automatically converted from UTF-8 to ASCII.
 - Provide a function to patch PS3 ISOs created with different applications than 
   GenPS3iso (Ex. ImgBurn, PowerISO).

 When used as application:

 - Users can generate a ISO that will instantly be compatible with the PS3
 running on Cobra CFW v7.00 (mixed w/Rogero CFW 4.46 v1.00).
 
 - Users can quickly patch any specified PS3 ISO created with different application than 
   GenPS3iso (Ex. ImgBurn, PowerISO).

 Note: Patching will make the ISO valid for the PS3 system, if you try to mount 
 it without patching, the system will not detect it.

 Additional Note: Only PS3 ISO need to be patched, so if you specify "--patch" with other
 kind of ISO nothing will be done to them. If you are curious and fool "PS ISO Tool" to 
 patch a non PS3 ISO, you will corrupt the ISO with data that shouldn't be there, so
 avoid doing that xD.
 
================================================================================

 Usage:
	
	ps_isotool [opt] [in]

--------------------------------------------------------------------------------

 Example 1 - Patching a PS3 game ISO:

	ps_isotool --ps3 --verbose --patch "C:\PS3ISO\MyPS3ISO.iso"

--------------------------------------------------------------------------------	

 Example 2 - Displaying info from ISOs:

	ps_isotool --ps1 --verbose "C:\PSXISO\MyPS1ISO.bin"
	ps_isotool --ps2 --verbose "C:\PS2ISO\MyPS2ISO.iso"
	ps_isotool --ps3 --verbose "C:\PS3ISO\MyPS3ISO.iso"
	ps_isotool --psp --verbose "C:\PSPISO\MyPSPISO.iso"
	
Note: If you don't specify "--verbose" then only the Title ID and Title will be displayed.

--------------------------------------------------------------------------------

 Example 3 - Creating a PS3 ISO in compliance with the PS3 system standard disc format:

	psiso_tool --mkps3iso "C:\GAMES\BCUS98174-[The Last of Us]" "C:\DESTINATION_DIR"
	psiso_tool --mkps3iso "C:\GAMES\BCUS98174-[The Last of Us]"
	
Note: You don't have to specify the ISO file name, it will be generated automatically,
you just need to specify "Source Directory" and "Destination Directory".

If you do not specify "Destination Directory" the ISO will be created on the root 
directory of "PS ISO Tool".

Important: There is no HDD space verification implemented yet so, if you plan to
make a batch for a big list of games, make sure you check your destination HDD 
available free space, at least until it gets implemented.

================================================================================
*/
#include "psiso_tool.h"

#define APP_VER "1.03"

void print_usage()
{
	printf(
		"Usage:  ps_isotool [opt] [in]\n"
		SEP_LINE_2
		"\n"
		"Example 1 - Patching a PS3 game ISO to comply with the PS3 system standard disc format: \n"
		"\n"
		"ps_isotool --ps3 --patch \"C:\\PS3ISO\\MyPS3ISO.iso\" \n"
		"\n"
		"Note: Only PS3 games will be patched, even if you specify \"--patch\" for other ISOs, they will not be modified.\n"
		"\n"
		"Example 2 - Displaying info from ISOs: \n"
		"\n"
		"ps_isotool --ps1 --verbose \"C:\\PSXISO\\MyPS1ISO.bin\" \n"
		"ps_isotool --ps2 --verbose \"C:\\PS2ISO\\MyPS2ISO.iso\" \n"
		"ps_isotool --ps3 --verbose \"C:\\PS3ISO\\MyPS3ISO.iso\" \n"
		"ps_isotool --psp --verbose \"C:\\PSPISO\\MyPSPISO.iso\" \n"
		"\n"
		"Note: If you don't specify \"--verbose\" then only the Title ID and Title will be displayed.\n"
		"\n"
		"Example 3 - Creating a PS3 ISO in compliance with the PS3 system standard disc format:\n"
		"\n"
		"psiso_tool --mkps3iso \"C:\\GAMES\\BCUS98174-[The Last of Us]\" \"C:\\DESTINATION_DIR\" \n"
		"psiso_tool --mkps3iso \"C:\\GAMES\\BCUS98174-[The Last of Us]\" \n"
		"\n"
		"Note: You don't have to specify the ISO file name, it will be generated automatically,"
		"you just need to specify \"Source Directory\" and \"Destination Directory\". \n"
		"\n"
		SEP_LINE_2
		"\n"
	);
}

#ifdef WIN

#define _WIN32_WINNT 0x0501 // this is for XP
#include <windows.h>
#include <shellapi.h>
#include <process.h>

int upd_progress_bar(char* szPct, char* szProgressBar)
{
	// get percent from txt string as number
	int nPct = 0;
	for(nPct = 0; nPct <= 100; nPct++) 
	{
		char _szPct[256];
		ZERO(_szPct);
		sprintf(_szPct, "%d%%", nPct);
		if(strncmp(szPct, _szPct, strlen(szPct))==0) {
			break;
		}
	}
	char szBar1[] = "[ ";
	char szBar2[55] = "||||||||||||||||||||||||||||||||||||||||||||||||||";
	char szBar3[] = " ]";
	char szProgress[256];
	ZERO(szProgress);
	
	int nBars = nPct/2;
	if(nBars < 0) nBars = 0;
	for(; nBars <= 50; nBars++) {
		szBar2[nBars] = '-';
	}
	szBar2[51] = 0;

	printf("\r                                                                   ");
	printf("\r");
	sprintf(szProgress, "%d%% - %s%s%s", nPct, szBar1, szBar2, szBar3);

	strcpy(szProgressBar, szProgress);

	return nPct;
}
#endif

int main(int argc, const char* argv[])
{
#ifdef WIN
	HWND hAppWnd = GetConsoleWindow();
#endif

	char _argv[15][512];

	for(int i = 0; i < argc; i++) {
		memset(&_argv[i], 0, strlen(argv[i])+1);
		strcpy(_argv[i], argv[i]);
	}

	printf(
		SEP_LINE_1
		"PS ISO Tool v"APP_VER" (supports PS1/PS2/PS3/PSP) (CaptainCPS-X, 2013) \n"
		SEP_LINE_1
	);

#ifdef WIN
	SetWindowText(hAppWnd, "PS ISO Tool v"APP_VER" (supports PS1/PS2/PS3/PSP) (CaptainCPS-X, 2013)");
#endif

	bool bPatch = false;

	// prog [opt] [file]
	char szISO[1024];
	ZERO(szISO);

	int nSystem = -1;

	// paramater 1 - 3 :	(system)(vervose)(patch) any order
	// parameter 4:			(path) no exception

	int nSystemParam = -1;
	int nVerboseParam = -1;
	int nPatchParam = -1;

	if(argc > 1 && argc <= 5 ) 
	{
#ifdef WIN
		// Create ISO with ImgBurn (WINDOWS ONLY)
		if( ((argc == 3) && (strncmp(_argv[1], "--mkps3iso", strlen("--mkps3iso"))==0)) ||	// No destination dir specified
			((argc == 4) && (strncmp(_argv[1], "--mkps3iso", strlen("--mkps3iso"))==0)) )	// destination dir specified
		{	
			printf("Preparing to create ISO (using ImgBurn)... \n");
			
			char szCurrentPath[512];
			ZERO(szCurrentPath);
			
			if (!GetCurrentDir(szCurrentPath, sizeof(szCurrentPath))) {
				// error...
				printf("Error: Problem acquiring current working directory. Please report this. \n");
				return 0;
			}
			szCurrentPath[sizeof(szCurrentPath) - 1] = '\0';

			printf(">> Current Working Path: %s \n", szCurrentPath);

			if(argc==3) 
			{ 
				// check source path only...
				if(!strchr(_argv[2], '\\') && !strchr(_argv[2], '/')) 
				{
					// directory appears to be on root of application...
					char szTemp[256];
					ZERO(szTemp);
					sprintf(szTemp, "%s\\%s", szCurrentPath, _argv[2]);
					strcpy(_argv[2], szTemp);
				}
			} else {
				// check valid source and destination paths...
				if( !strchr(_argv[2], '\\') && !strchr(_argv[2], '/') )
				{
					// directory appears to be on root of application...
					char szTemp[256];
					ZERO(szTemp);
					sprintf(szTemp, "%s\\%s", szCurrentPath, _argv[2]);
					strcpy(_argv[2], szTemp);
				}
				if(	!strchr(_argv[3], '\\') && !strchr(_argv[3], '/') ) 
				{
					// directory appears to be on root of application...
					char szTemp[256];
					ZERO(szTemp);
					sprintf(szTemp, "%s\\%s", szCurrentPath, _argv[3]);
					strcpy(_argv[3], szTemp);
				}
			}

			char szSource[512];
			char szDest[512];
			char szParamSfo[512];
			char szCommand[1024];
			ZERO(szDest);
			ZERO(szSource);
			ZERO(szParamSfo);
			ZERO(szCommand);

			strcpy(szSource, _argv[2]);
	
			// check if directories have ending slash and remove them...
			if( (szSource[strlen(szSource)-1] == '\\' || szSource[strlen(szSource)-1] == '/')) 
			{
				szSource[strlen(szSource)-1] = 0; // remove ending slash
			}
			if( (szSource[strlen(szSource)-1] == ' ' && szSource[strlen(szSource)-2] == '"')) 
			{	
				// in case windows convert the last slash into a --> "
				szSource[strlen(szSource)-2] = 0;				
			}

			if(argc == 4) {
				// force proper naming of iso when user pass a destination file instead of directory...
				if(strstr(_argv[3], ".iso") || strstr(_argv[3], ".ISO"))
				{
					char* ch = NULL;					
					ch = strrchr(_argv[3], '\\');

					if(!ch) {
						ch = strrchr(_argv[3], '/');
					}

					if(ch) {
						_argv[3][ch-_argv[3]] = 0;
						strcpy(szDest, _argv[3]);
					}
				} else {
					strcpy(szDest, _argv[3]);
				}

				// check if directories have ending slash and remove them...
				if( (szDest[strlen(szDest)-1] == '\\' || szDest[strlen(szDest)-1] == '/')) 
				{
					szDest[strlen(szDest)-1] = 0; // remove ending slash
				}
				if( (szDest[strlen(szDest)-1] == ' ' && szDest[strlen(szDest)-2] == '"')) 
				{	
					// in case windows convert the last slash into a --> "
					szDest[strlen(szDest)-2] = 0;			
				}
			}

			printf(">> Source directory: %s \n"		, szSource);
			printf(">> Destination directory: %s \n"	, szDest);

			sprintf(szParamSfo, "%s\\PS3_GAME\\PARAM.SFO", szSource);

			printf("Checking PARAM.SFO... \n");

			FILE* fp = fopen(szParamSfo, "rb");

			if(fp) 
			{
				fseek(fp, 0, SEEK_END);
				size_t nLen = ftell(fp);
				fseek(fp, 0, SEEK_SET);

				char szTitleID[32];
				char szTitle[128];
				ZERO(szTitleID);
				ZERO(szTitle);

				ParseSFO(fp, 0, nLen, (char*)"TITLE_ID", (char*)szTitleID);
				fseek(fp, 0, SEEK_SET);
				ParseSFO(fp, 0, nLen, (char*)"TITLE", (char*)szTitle);

				if(szTitleID[0] && szTitle[0]) {
					printf("Successfully acquired TITLE_ID and TITLE from PARAM.SFO! \n");
					printf(">> Title ID: %s \n", szTitleID);
					printf(">> Title: %s \n", szTitle);
					if(argc == 3) {
						// create iso on program directory...
						sprintf(szDest, "%s\\%s-[%s].iso", szCurrentPath, szTitleID, szTitle);
					} else {
						char szTemp[512];
						ZERO(szTemp);
						// create iso on directory specified by user...
						sprintf(szTemp, "\\%s-[%s].iso", szTitleID, szTitle);
						strcat(szDest, szTemp);
					}
				} else {
					printf("Warning: Couldn't acquire TITLE_ID and TITLE from PARAM.SFO, probably is corrupted. \n");
					if(argc == 3) {
						// create iso on program directory...
						sprintf(szDest, "%s\\%s.iso", szCurrentPath, szSource); 
					} else {
						char szTemp[512];
						ZERO(szTemp);
						// create iso on directory specified by user...
						sprintf(szTemp, "\\%s.iso", szSource);
						strcat(szDest, szTemp);
					}
				}
				SAFE_FCLOSE(fp);
			} else {
				printf("Error: Cannot locate PARAM.SFO, please verify that the path contain a valid PS3 game directory. \n");
				return 0;
			}
			
			printf(">> Output ISO file: %s \n", szDest);

			printf("Booting ImgBurn for PS3 ISO creation, please wait... \n");

			sprintf(szCommand, 				
				"/MODE BUILD "
				"/BUILDMODE IMAGEFILE "
				"/SRC \"%s\" "
				"/DEST \"%s\" "
				"/FILESYSTEM \"ISO9660 + Joliet\" "
				"/VOLUMELABEL \"PS3VOLUME\" "
				"/OVERWRITE \"YES\" "
				"/CLOSE "
				"/NOIMAGEDETAILS "
				"/ROOTFOLDER \"YES\" "
				"/START "
				"/SETTINGS \"%s\\imgburn\\ImgBurn.ini\" "
				"/PORTABLE",
				szSource, 
				szDest,
				szCurrentPath
			);
			char szImgBurn[512];
			ZERO(szImgBurn);
			sprintf(szImgBurn, "\"%s\\imgburn\\ImgBurn.exe\"", szCurrentPath);
			ShellExecute(NULL, "open", szImgBurn, szCommand, NULL, SW_SHOWMINIMIZED);
			
			HMENU hmenu = GetSystemMenu(hAppWnd, FALSE);
		    EnableMenuItem(hmenu, SC_CLOSE, MF_GRAYED);
			
			bool bFound = false;
			bool bDone = false;
			HWND hWnd = NULL;
			while(1) 
			{
				if(!bFound) 
				{ 
					for(int i =0; i <= 100; i++) {
						char szSearch[64];
						ZERO(szSearch);
						sprintf(szSearch, "%d%% - ImgBurn", i);
						hWnd = (HWND)FindWindow(NULL, szSearch);
						if(hWnd) {
							bFound = true;
							break;
						}
					}
				}
				if(!hWnd) continue;

				char szBuf[2048];
				ZERO(szBuf);
				LONG lResult;
				lResult = SendMessage(hWnd, WM_GETTEXT, sizeof( szBuf ) / sizeof( szBuf[0] ), (LPARAM)szBuf );
				
				if(lResult && !bDone) {
					char* ch = strrchr(szBuf, '-');
					if(ch) {
						szBuf[(ch-szBuf)-1]=0;
						char szDisplayProgress[256];
						ZERO(szDisplayProgress);
						upd_progress_bar(szBuf, szDisplayProgress);
						
						printf("%s", szDisplayProgress);
						SetWindowText(hAppWnd, szDisplayProgress);
					} else {
						bDone = true;				
					}
				}
				
				// Finished ? (todo: do more checks, to detect errors)
				if(bDone)
				{
					printf("\r100%% - [ |||||||||||||||||||||||||||||||||||||||||||||||||| ]    \n");
					printf(SEP_LINE_2);
					break;
				}
				Sleep((unsigned long)((1.0/60.0)*1000.0));
			}			

			EnableMenuItem(hmenu, SC_CLOSE, MF_ENABLED);

			printf("Preparing to patch the created PS3 ISO... \n");
			printf(SEP_LINE_2);

			SetWindowText(hAppWnd, "PS ISO Tool v"APP_VER" (supports PS1/PS2/PS3/PSP) (CaptainCPS-X, 2013) \n");

			memset(_argv[1], 0, 512);
			memset(_argv[2], 0, 512);
			memset(_argv[3], 0, 512);
			memset(_argv[4], 0, 512);

			strcpy(_argv[1], "--ps3");
			strcpy(_argv[2], "--verbose");
			strcpy(_argv[3], "--patch");
			strcpy(_argv[4], szDest);
			*_argv[5] = 0;
			argc = 5;
		}
#endif
		for(int nParam = 1; nParam < 4; nParam++)
		{
			// System
			if(( strncmp(_argv[nParam], "--ps1", strlen("--ps1"))==0 || 
				 strncmp(_argv[nParam], "--ps2", strlen("--ps2"))==0 || 
				 strncmp(_argv[nParam], "--ps3", strlen("--ps3"))==0 || 
				 strncmp(_argv[nParam], "--psp", strlen("--psp"))==0)  && nSystemParam == -1) 
			{
				if(strncmp(_argv[nParam], "--ps1", strlen("--ps1"))==0) nSystem = ISO_SYSTEM_PS1; 
				if(strncmp(_argv[nParam], "--ps2", strlen("--ps2"))==0) nSystem = ISO_SYSTEM_PS2;
				if(strncmp(_argv[nParam], "--ps3", strlen("--ps3"))==0) nSystem = ISO_SYSTEM_PS3;
				if(strncmp(_argv[nParam], "--psp", strlen("--psp"))==0) nSystem = ISO_SYSTEM_PSP;

				nSystemParam = nParam;
			}

			if(nSystemParam != -1)
			{
				if(nSystemParam == nVerboseParam || nSystemParam == nPatchParam) {
					// wth, someone just passed the same parameter twice lol!
					print_usage(); return 1;
				}
			}

			// Verbose
			if( (strncmp(_argv[nParam], "--verbose", strlen("--verbose"))==0 || strncmp(_argv[nParam], "--v", strlen("--v"))==0) && 
				(nVerboseParam == -1) )
			{
				bPSISOTool_verbose = true;
				nVerboseParam = nParam;
			}

			if(nVerboseParam != -1)
			{
				if(nVerboseParam == nSystemParam || nVerboseParam == nPatchParam) {
					// wth, someone just passed the same parameter twice lol!
					print_usage(); return 1;
				}
			}

			// Patch
			if( (nSystem == ISO_SYSTEM_PS3) && 
				(strncmp(_argv[nParam], "--patch", strlen("--patch"))==0) && 
				(nPatchParam == -1) )
			{
				bPatch = true;
				nPatchParam = nParam;
			}

			if(nPatchParam != -1)
			{
				if(nPatchParam == nSystemParam || nPatchParam == nVerboseParam) {
					// wth, someone just passed the same parameter twice lol!
					print_usage(); return 1;
				}
			}
		}

		// system parameter must always be present
		if(nSystemParam == -1) {
			print_usage(); return 1;
		}

		// parameters specified but path was not...
		// ex. psiso_tool --ps3
		if(argc == 2) {
			print_usage(); return 1;
		}

		// parameters specified but path was not...
		// ex1. psiso_tool --ps3 --verbose
		// ex2. psiso_tool --ps3 --patch
		if( (argc == 3 && nVerboseParam != -1) || 
			(argc == 3 && nPatchParam != -1 && nSystem == ISO_SYSTEM_PS3)) 
		{
			print_usage(); return 1;
		}

		// parameters specified but path was not...
		// ex. psiso_tool --ps3 --verbose --patch
		if(argc == 4 && nVerboseParam != -1 && nPatchParam != -1 && nSystem == ISO_SYSTEM_PS3) 
		{
			print_usage(); return 1;
		}

		// get the path argument...
		strcpy(szISO, _argv[argc-1]);

	} else {
		print_usage();
		return 1;
	}

	char szTitleID[32];	
	char szTitle[1024];
	ZERO(szTitleID);
	ZERO(szTitle);

	int ret = psxProcessISO((char*)szISO, nSystem, (char*)szTitleID, (char*)szTitle, bPatch);

	if(ret == 0) {
		printf("Error: ISO file \"%s\" could not be located, please verify the path. \n", szISO);
	}
	if(ret == -1) {
		if(!bPSISOTool_verbose) {
			printf("Error: ISO file \"%s\" is not valid or there were problems processing it. Use --verbose or --v flag to display detailed info. \n", szISO);
		}
		return 1;
	}

	printf(SEP_LINE_2);

	if(!szTitleID[0]) {
		printf("error: szTitleID[0] == NULL\n");
	} else {
		printf("TITLE ID: ( %s ) \n", szTitleID);		
		if(!szTitle[0]) {
			printf("error: szTitle[0] == NULL\n");
		} else {
			printf("TITLE: ( %s ) \n", szTitle);
		}
	}	
	printf(SEP_LINE_2);

	return 0;
}

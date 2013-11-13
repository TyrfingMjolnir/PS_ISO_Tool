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

#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__) || defined(_MSC_VER)
#define WIN
#endif

#ifdef _MSC_VER
#include <direct.h>
// disable annoying warnings ...
#pragma warning(disable:4514)		// [*] : unreferenced inline function has been removed
#pragma warning(disable:4711)		// function [*] selected for automatic inline expansion
#pragma warning(disable:4996)		// [*] : This function or variable may be unsafe. Consider using [*] instead
#endif

#ifdef WIN
// ---------------------------------------------------------------------------------------------------------------

#if defined(__CYGWIN32__) || defined(__MINGW32__)
#include <fcntl.h>
#include <unistd.h>
#endif

// On non-ps3 build we use the current working directory to look 
// for the PS1/PS2 title & id database files (for now)
#define GetCurrentDir _getcwd

/*
-----------------------------------------------------------------------------------
For PS3 build, just add this flags to your makefile
-----------------------------------------------------------------------------------
-DPSISOTOOL_PS3BUILD
-DAPP_DIR="SISO00123"	<-------- with your app TITLE ID
-----------------------------------------------------------------------------------
*/
#ifdef PSISOTOOL_PS3BUILD

// On ps3 build we use the application directory to look for 
// the PS1/PS2 title & id database files (for now)
#define PS3_GAME_CWD	"/dev_hdd0/game/"APP_DIR"/USRDIR"

#else

#endif

// ---------------------------------------------------------------------------------------------------------------
#else
#include <fcntl.h>
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

extern bool bPSISOTool_verbose; // info display control

// This should work on any compiler that is not Microsoft Visual C++...
#ifndef _MSC_VER
/*
-----------------------------------------------------------------------------------
This will integrate the PS ISO Tool modules with the PS3 NTFS library made by Estwald.
Just add these flags to your makefile...
-----------------------------------------------------------------------------------
-DNTFS_IO_DEFS -DPSISOTOOL_PS3BUILD
-----------------------------------------------------------------------------------
*/

#define _open		open
#define _close		close
#define _write		write
#define _read		read
#define _seek		seek

#define _lseek64	lseek //64 for DARWIN lseek is 64 bit already
/*
The reason you can just use lseek is because off_t is a 64bit wide value.

http://stackoverflow.com/questions/19649634/clang-error-unsupported-option-static-libgcc-on-mac-osx-mavericks

*/

#define _fstat		fstat
#define _stat		stat
#define _link		link
#define _unlink		unlink
#define _chdir		chdir
#define _rename		rename
#define _mkdir		mkdir

#ifdef PSISOTOOL_PS3BUILD
	#ifdef NTFS_IO_DEFS
		#define _open		ps3ntfs_open
		#define _close		ps3ntfs_close
		#define _write		ps3ntfs_write
		#define _read		ps3ntfs_read
		#define _seek		ps3ntfs_seek
		#define _lseek64	ps3ntfs_seek64
		#define _fstat		ps3ntfs_fstat
		#define _stat		ps3ntfs_stat
		#define _link		ps3ntfs_link
		#define _unlink		ps3ntfs_unlink
		#define _chdir		ps3ntfs_chdir
		#define _rename		ps3ntfs_rename
		#define _mkdir		ps3ntfs_mkdir
	#endif
#endif

#endif

#define SAFE_FREE(x) \
	if(x) { free(x); *&x = NULL; }

#ifndef WIN 
#define SAFE_CLOSE(x) \
	if(x) { _close(x); *&x = NULL; }
#endif

#define SAFE_FCLOSE(x) \
	if(x) { fclose(x); *&x = NULL; }

#define ZERO(x) \
	memset(&x, 0, sizeof(x));

// ---------------------------------------------------------------------------------------------------------
// TODO: Create a structure array included with source to avoid this, plus it will allow adding more fields
// like "Year", "Manufacturer", "Max Players", etc...(of course, if someone wants to expand this)

#ifdef PSISOTOOL_PS3BUILD

#define PS1_TITLE_DB	PS3_GAME_CWD "/db/ps1titles_us_eu_jp.txt"	// by CaptainCPS-X based on (http://sonyindex.com/)
#define PS2_TITLE_DB	PS3_GAME_CWD "/db/ps2titleid.txt"			// by aldostools [?]

#else

#define PS1_TITLE_DB "db/ps1titles_us_eu_jp.txt" // by CaptainCPS-X based on (http://sonyindex.com/)
#define PS2_TITLE_DB "db/ps2titleid.txt"		 // by aldostools [?]

#endif
// ---------------------------------------------------------------------------------------------------------


#define ISO_SYSTEM_PS1	0	// ---ISO9660 / MODE1 / 2048--- or ---ISO9660 / MODE2 / 2352---
#define ISO_SYSTEM_PS2	1	// ---ISO9660 / MODE1 / 2048--- or ---ISO9660 / MODE2 / 2352---
#define ISO_SYSTEM_PS3	2	// ---ISO9660 / MODE1 / 2048 / Joliet (ONLY)---
#define ISO_SYSTEM_PSP	3	// ---ISO9660 / MODE1 / 2048 (ONLY)---

#define PS1_TITLE_ID_LEN	11					// EX. SCUS_941.65
#define PS2_TITLE_ID_LEN	PS1_TITLE_ID_LEN

// ------------------------------------------------------------------------------------------------
// PS ISO Processing module (by CaptainCPS-X, 2013)
/* ------------------------------------------------------------------------------------------------
(in)	szISO			- Path to ISO
(in)	nSystem			- One of the following: (ISO_SYSTEM_PS1) (ISO_SYSTEM_PS2) (ISO_SYSTEM_PS3) (ISO_SYSTEM_PSP) 
(out)	szTitleID		- String buffer to store Game Title ID (Ex. BLUS-00123)
(out)	szTitle			- String buffer to store Game Title (Ex. The Last of Us)
(in)	bPatchPS3ISO	- Flag to specify if ISO header should be patched to be PS3 compliant (Ex. If ISO was done with PowerISO, ImgBurn, etc...) 

(out)	return			- Will return 1 for success and 0 for failure.  

------------------------------------------------------------------------------------------------- 
*/
int psxProcessISO(char* szISO, int nSystem, char* szTitleID, char* szTitle, bool bPatchPS3ISO);

// -----------------------------------------------------------------------------------------------
// PARAM.SFO Processing module (by CaptainCPS-X, 2013)
/* -----------------------------------------------------------------------------------------------
(in)	fp				- FILE* pointer to and actual open handle of the file containing the PARAM.SFO (like an ISO or the actual PARAM.SFO)
(in)	nOffset			- Offset address to the location where PARAM.SFO data is located (pass 0 if the file is the actual PARAM.SFO)
(in)	nLen			- Length in bytes of the PARAM.SFO file data
(in)	szEntry			- Variable field name being requested (Can be PS3 field or PSP) (Ex1. TITLE_ID) (Ex2. DISC_ID)
(out)	szOut			- String buffer to store the field data requested, if the data is text.
						- If you're requesting a numeric field value then, this should be NULL.

(out)	return			- Will return the numeric field value, when a numeric field is requested. 
						- Will return 0 if the requested field value is text.

-------------------------------------------------------------------------------------------------
*/
#ifdef WIN
extern uint64_t ParseSFO(FILE* fp, uint64_t nOffset, size_t nLen, char* szEntry, char* szOut);
#else
uint64_t ParseSFO(int fd, uint64_t nOffset, size_t nLen, char* szEntry, char* szOut);
#endif

// -----------------------------------------------------------------------------------------------
// Utility modules
// -----------------------------------------------------------------------------------------------
void utf8_to_ansi(char *utf8, char *ansi, int len);
void swap16_data(uint8_t* data);
void swap8_data(uint8_t* data);
uint32_t data_to_u16(uint8_t* data);
uint32_t data_to_u8(uint8_t* data);

#define SEP_LINE_1 "=========================================================================\n"
#define SEP_LINE_2 "-------------------------------------------------------------------------\n"

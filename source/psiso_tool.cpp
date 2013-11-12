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

// ------------------------------------------------------------------------------
char cCurrentPath[FILENAME_MAX];
const char szISOSystem[][64] = {{"PS1"},{"PS2"},{"PS3"}, {"PSP"}};

#define MAX_MSG_SZ      1024

// info display control
bool bPSISOTool_verbose = false;
#define _verbose_printf if(bPSISOTool_verbose)printf

// ------------------------------------------------------------------------------

#ifndef WIN
 
bool isWhitespace(char c)
{
    switch (c)
    {
        case '\r':
        case '\n':
        case ' ':
        case '\0':
            return true;
        default:
            return false;
    }
}

// Strip off whitespace characters from the end of the line
void chomp(char *line)
{
    int len = strlen(line);    
	while (isWhitespace(line[len])) {
        line[len--] = '\0';
    }
}

char * GetLine(int fds)
{
    char tline[MAX_MSG_SZ];
    char *line;
    
    int messagesize = 0;
    int amtread = 0;
    
	while((amtread = _read(fds, tline + messagesize, 1)) < MAX_MSG_SZ)
    {
        if (amtread > 0)
            messagesize += amtread;
        else
        {
            // error...
			return NULL;
        }
        if (tline[messagesize - 1] == '\n')
            break;
    }
    tline[messagesize] = '\0';
    
	chomp(tline);
    
	line = (char *)malloc((strlen(tline) + 1) * sizeof(char));
    
	strcpy(line, tline);
    
	return line;
}
#endif

int GetTitle(char *_szTitleID, char* szDatabase, char* szTitle, int nSystem)
{
	char szTitleID[32];
	ZERO(szTitleID);

	strcpy(szTitleID, _szTitleID);

	if(nSystem == ISO_SYSTEM_PS1) 
	{
		// SLUS_012.34 -> SLUS-01234
		if(szTitleID[4] == '_') 
		{
			szTitleID[4] = '-';
			szTitleID[8] = szTitleID[9];
			szTitleID[9] = szTitleID[10];
			szTitleID[10] = 0;
			szTitleID[11] = 0;
		}
	}

	if(nSystem == ISO_SYSTEM_PS2) 
	{
		// SLUS_012.34 -> SLUS01234
		if(szTitleID[4] == '_') 
		{
			szTitleID[4] = szTitleID[5];
			szTitleID[5] = szTitleID[6];
			szTitleID[6] = szTitleID[7];
			szTitleID[7] = szTitleID[9];
			szTitleID[8] = szTitleID[10];
			szTitleID[9] = szTitleID[11];
			szTitleID[10] = 0;
			szTitleID[11] = 0;
		}
	}

	if(nSystem == ISO_SYSTEM_PS3) 
	{
		// BLUS-01234 -> BLUS01234
		if(szTitleID[4] == '-') 
		{
			szTitleID[4] = szTitleID[5];
			szTitleID[5] = szTitleID[6];
			szTitleID[6] = szTitleID[7];
			szTitleID[7] = szTitleID[8];
			szTitleID[8] = szTitleID[9];
			szTitleID[9] = szTitleID[10];
			szTitleID[10] = 0;
		}
	}

	if(nSystem == ISO_SYSTEM_PSP)
	{
		// BLUS-01234 -> BLUS01234
		if(szTitleID[4] != '-') 
		{
			char szTmp1[5];
			ZERO(szTmp1);
			char szTmp2[7];
			ZERO(szTmp2);
			strncpy(szTmp1, szTitleID, 4);
			sprintf(szTmp2, "%s-%s", szTmp1, szTitleID+4);
			memset(szTitleID, 0, 10);
			strcpy(szTitleID, szTmp2);
		}
	}

	_verbose_printf("Getting title for: %s\n", szTitleID);

	bool bFoundTitle = false;

	char szFullDatabasePath[2048];
	ZERO(szFullDatabasePath);

#ifndef PSISOTOOL_PS3BUILD
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) {
		// error...
		return 0;
	}
	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';

	sprintf(szFullDatabasePath, "%s/%s", cCurrentPath, szDatabase); 
#else
	strcpy(szFullDatabasePath, szDatabase);
#endif

#ifdef WIN
	FILE *fp = NULL;
	fp = fopen(szFullDatabasePath, "r");
	if(fp)
	{
#else
	int fd = _open(szFullDatabasePath, O_RDONLY);
	if(fd != -1) 
	{
#endif
		char *buffer = NULL;
		buffer = (char*)malloc(MAX_MSG_SZ);
		memset(buffer, 0, MAX_MSG_SZ);

#ifdef WIN
		while(fgets(buffer , MAX_MSG_SZ, fp))
		{
#else
		while((buffer = GetLine(fd))) 
		{
#endif
			char szLine[MAX_MSG_SZ];
			ZERO(szLine);
			strcpy(szLine, buffer);

			if(strncmp(szLine, "//", 2)==0) continue;
			if(strlen(szLine) < 11) continue;

			char *p1 = strchr(szLine, ' ');
			uint32_t nSpcPos = (uint32_t)(p1-szLine);
				
			char _szTitle[256];
			ZERO(_szTitle);
			strcpy(_szTitle, szLine + nSpcPos + 1);

			if(_szTitle[strlen(_szTitle)-1] == '\n' || _szTitle[strlen(_szTitle)-1] == '\r') {
				_szTitle[strlen(_szTitle)-1] = 0; // remove new line
			}
			szLine[p1-szLine] = 0;

			char szFinalTitleID[32]; 
			ZERO(szFinalTitleID)
			strncpy(szFinalTitleID, szLine, strlen(szTitleID));

			if(strcmp(szFinalTitleID, szTitleID) == 0 && strlen(szTitleID) == strlen(szFinalTitleID))
			{
				strncpy(szTitle, _szTitle, strlen(_szTitle));

				SAFE_FREE(buffer);
#ifdef WIN
				SAFE_FCLOSE(fp);
#else
				SAFE_CLOSE(fd);
#endif
				bFoundTitle = true;
				return 1;
			}
		}
#ifdef WIN
		SAFE_FCLOSE(fp);
#else
		SAFE_CLOSE(fd);
#endif
	}

	return 0;
}

// UTF-8 modules were Imported from Iris Manager (utils.c), thanks to Iris devs.
void utf8_truncate(char *utf8, char *utf8_trunc, int len)
{
	uint8_t *ch= (uint8_t *) utf8;
	*utf8_trunc = 0;

	while(*ch!=0 && len>0)
	{
		// 3, 4 bytes utf-8 code 
		if(((*ch & 0xF1)==0xF0 || (*ch & 0xF0)==0xe0) && (*(ch+1) & 0xc0)==0x80)
		{
			//*utf8_trunc++=' '; // ignore
			memcpy(utf8_trunc, &ch, (size_t)(3+1*((*ch & 0xF1)==0xF0)) );
			utf8_trunc+= 3+1*((*ch & 0xF1)==0xF0);
			len--;
			ch+=2+1*((*ch & 0xF1)==0xF0);
		} else {

			// 2 bytes utf-8 code	
			if((*ch & 0xE0)==0xc0 && (*(ch+1) & 0xc0)==0x80)
			{
				memcpy(utf8_trunc, &ch, 2);
				utf8_trunc+=2;
				len--;
				ch++;
			} else {
				if(*ch<32) *ch=32;
				*utf8_trunc++=(char)*ch;
				len--;
			}
		}
		ch++;
	}

	while(len > 0) {
		*utf8_trunc++=0;
		len--;
	}
}

void utf8_to_ansi(char *utf8, char *ansi, int len)
{
	uint8_t *ch= (uint8_t *) utf8;
	uint8_t c;
	*ansi = 0;

	while(*ch!=0 && len>0)
	{
		// 3, 4 bytes utf-8 code 
		if(((*ch & 0xF1)==0xF0 || (*ch & 0xF0)==0xe0) && (*(ch+1) & 0xc0)==0x80)
		{
			*ansi++=' '; // ignore
			len--;
			ch+=2+1*((*ch & 0xF1)==0xF0);
		} else {

			// 2 bytes utf-8 code	
			if((*ch & 0xE0)==0xc0 && (*(ch+1) & 0xc0)==0x80)
			{
				c= (uint8_t)(((*ch & 3)<<6) | (*(ch+1) & 63));

				if(c>=0xC0 && c<=0xC5) c='A';
				else if(c==0xc7) c='C';
				else if(c>=0xc8 && c<=0xcb) c='E';
				else if(c>=0xcc && c<=0xcf) c='I';
				else if(c==0xd1) c='N';
				else if(c>=0xd2 && c<=0xd6) c='O';
				else if(c>=0xd9 && c<=0xdc) c='U';
				else if(c==0xdd) c='Y';
				else if(c>=0xe0 && c<=0xe5) c='a';
				else if(c==0xe7) c='c';
				else if(c>=0xe8 && c<=0xeb) c='e';
				else if(c>=0xec && c<=0xef) c='i';
				else if(c==0xf1) c='n';
				else if(c>=0xf2 && c<=0xf6) c='o';
				else if(c>=0xf9 && c<=0xfc) c='u';
				else if(c==0xfd || c==0xff) c='y';
				else if(c>127) c=*(++ch+1); //' ';

				*ansi++=(char)c;
				len--;
				ch++;

			} else {
				if(*ch<32) *ch=32;
				*ansi++=(char)*ch;
				len--;
			}
		}
		ch++;
	}

	while(len > 0) {
		*ansi++=0;
		len--;
	}
}
//

void swap16_data(uint8_t* data)
{
	uint8_t* temp = NULL;
	temp = (uint8_t*)malloc(4);
	memset(temp, 0, 4);
	memcpy(temp, data, 4);
	
	data[0] = temp[3];
	data[1] = temp[2];
	data[2] = temp[1];
	data[3] = temp[0];

	if(temp) free(temp);	
}

void swap8_data(uint8_t* data)
{
	uint8_t* temp = NULL;
	temp = (uint8_t*)malloc(2);
	memset(temp, 0, 2);
	memcpy(temp, data, 2);
	
	data[0] = temp[1];
	data[1] = temp[0];

	if(temp) free(temp);
}

uint32_t data_to_u16(uint8_t* data)
{
	char szTemp[9];
	memset(&szTemp, 0, 9);
	sprintf(szTemp, "%02X%02X%02X%02X", data[0], data[1], data[2], data[3]);
	uint32_t ret = (uint32_t)strtol(szTemp, NULL, 16);
	return ret;
}

uint32_t data_to_u8(uint8_t* data)
{
	char szTemp[5];
	memset(&szTemp, 0, 5);
	sprintf(szTemp, "%02X%02X", data[0], data[1]);
	uint32_t ret = (uint32_t)strtol(szTemp, NULL, 16);
	return ret;
}

bool bSFOInfoDisplayed = false;

// New function coded from scratch to properly parse PARAM.SFO
#ifdef WIN
uint64_t ParseSFO(FILE* fp, uint64_t nOffset, size_t nLen, char* szEntry, char* szOut)
{
#else
uint64_t ParseSFO(int fd, uint64_t nOffset, size_t nLen, char* szEntry, char* szOut)
{
#endif
	(void)nLen;
	
	if(!bSFOInfoDisplayed) 
	{
		_verbose_printf(SEP_LINE_2);
		_verbose_printf( "Preparing to process PARAM.SFO \n");
		_verbose_printf(SEP_LINE_2);
	}

#ifdef WIN
	if(!fp) {
#else
	if(fd == -1) {
#endif
		_verbose_printf("Fatal error: File cannot be found / accessed. \n");
		return 0;
	}

#ifdef WIN
	fseek(fp, (long)nOffset, SEEK_SET);
#else
	_lseek64(fd, nOffset, SEEK_SET);
#endif

	struct sfo_header_data
	{
		char type;
		uint8_t id[3];
		uint8_t version[4];
		uint8_t variable_table_offset[4];
		uint8_t data_table_offset[4];
		uint8_t total_variables[4];
	};

	sfo_header_data header_data;
	memset(&header_data, 0, sizeof(sfo_header_data));

#ifdef WIN
	fread(&header_data, 1, sizeof(sfo_header_data), fp);
#else
	_read(fd, &header_data, sizeof(sfo_header_data));
#endif

	struct sfo_header
	{		
		uint32_t nType;
		char szId[4];
		uint32_t nVersion;		
		uint32_t nVarNameTableOffset;
		uint32_t nDataTableOffset;		
		uint32_t nTotalVariables;
	};

	sfo_header header;
	memset(&header, 0, sizeof(sfo_header));
	
	// TYPE (always 0 ?)
	header.nType = (uint32_t)strtol(&header_data.type, NULL, 16);
	
	// ID (should be PSF)
	memcpy(header.szId, header_data.id, sizeof(header_data.id));

	// VARIABLES TABLE OFFSET
	swap16_data((uint8_t*)header_data.variable_table_offset);
	header.nVarNameTableOffset = data_to_u16((uint8_t*)header_data.variable_table_offset);
	
	// DATA TABLE OFFSET
	swap16_data((uint8_t*)header_data.data_table_offset);
	header.nDataTableOffset = data_to_u16((uint8_t*)header_data.data_table_offset);

	// TOTAL VARIABLES
	swap16_data((uint8_t*)header_data.total_variables);
	header.nTotalVariables = data_to_u16((uint8_t*)header_data.total_variables);

	if(!bSFOInfoDisplayed) 
	{
		_verbose_printf("SFO Type: 0x%02X \n"					, header.nType);
		_verbose_printf("SFO Identifier: %s \n"					, header.szId);
		_verbose_printf("SFO Variable Name Table Offset: 0x%08X \n"	, (uint32_t)header.nVarNameTableOffset);
		_verbose_printf("SFO Data Table Offset: 0x%08X \n"			, (uint32_t)header.nDataTableOffset);
		_verbose_printf("SFO Total Variables: %d \n"					, header.nTotalVariables);
	}

	struct sfo_vartbl_entry_data
	{
		uint8_t name_offset[2];
		uint8_t type[2];
		uint8_t data_size[4];
		uint8_t data_block_size[4];
		uint8_t data_offset[4];
	};

	struct sfo_vartbl_entry
	{
		uint32_t nNameOffset;
		uint32_t nType;
		uint32_t nDataSize;
		uint32_t nDataBlockSize;
		uint32_t nDataOffset;
		
		uint32_t nNumData;			// This will hold numeric values when needed		
		char	 szName[32];		// Variable names should not surpass this...LOL
		char	 szTxtData[1024];	// 1024 should be enough for any possible text data entry
		
	};

	// Obtain variable table entries
	size_t nVarTableDataLen = header.nTotalVariables * sizeof(sfo_vartbl_entry_data);
	sfo_vartbl_entry_data* var_table_entries_data = NULL;
	var_table_entries_data = (sfo_vartbl_entry_data*)malloc( nVarTableDataLen);
	memset(var_table_entries_data, 0, nVarTableDataLen);

	uint32_t nVarTableOffset = 0x14;

#ifdef WIN
	fseek(fp, (long)(nOffset + nVarTableOffset), SEEK_SET);
	fread(var_table_entries_data, 1, nVarTableDataLen, fp);
#else
	_lseek64(fd, nOffset + nVarTableOffset, SEEK_SET);
	_read(fd, var_table_entries_data, nVarTableDataLen);
#endif

	size_t nVarTableLen = header.nTotalVariables * sizeof(sfo_vartbl_entry);	
	sfo_vartbl_entry* var_table_entries = NULL;
	var_table_entries = (sfo_vartbl_entry*)malloc( nVarTableLen );
	memset(var_table_entries, 0, nVarTableLen);

	if(!bSFOInfoDisplayed) 
	{
		_verbose_printf(SEP_LINE_2);
		_verbose_printf( "SFO Variable Table Entries: \n");
		_verbose_printf(SEP_LINE_2);
	}

	uint32_t i;
	for(i = 0; i < header.nTotalVariables; i++) 
	{		
		// NAME OFFSET
		swap8_data((uint8_t*)var_table_entries_data[i].name_offset);
		var_table_entries[i].nNameOffset = data_to_u8((uint8_t*)var_table_entries_data[i].name_offset);

		// TYPE
		swap8_data((uint8_t*)var_table_entries_data[i].type);
		var_table_entries[i].nType = data_to_u8((uint8_t*)var_table_entries_data[i].type);
		
		// DATA SIZE
		swap16_data((uint8_t*)var_table_entries_data[i].data_size);
		var_table_entries[i].nDataSize = data_to_u16((uint8_t*)var_table_entries_data[i].data_size);

		// DATA BLOCK SIZE
		swap16_data((uint8_t*)var_table_entries_data[i].data_block_size);
		var_table_entries[i].nDataBlockSize = data_to_u16((uint8_t*)var_table_entries_data[i].data_block_size);

		// DATA OFFSET
		swap16_data((uint8_t*)var_table_entries_data[i].data_offset);
		var_table_entries[i].nDataOffset = data_to_u16((uint8_t*)var_table_entries_data[i].data_offset);

		// ====

		memset(var_table_entries[i].szName, 0, 32);
		memset(var_table_entries[i].szTxtData, 0, 1024);
		*&var_table_entries[i].nNumData = 0;

#ifdef WIN
		fseek(fp, (long)(nOffset + header.nVarNameTableOffset + var_table_entries[i].nNameOffset), SEEK_SET);
		fread(var_table_entries[i].szName, 1, 32, fp);
#else
		_lseek64(fd, nOffset + header.nVarNameTableOffset + var_table_entries[i].nNameOffset, SEEK_SET);
		_read(fd, var_table_entries[i].szName, 32);
#endif
		// CHECK FOR NUMERIC or TEXT DATA
		if(var_table_entries[i].nType == 0x0204)
		{
			// text
#ifdef WIN
			fseek(fp, (long)(nOffset + header.nDataTableOffset + var_table_entries[i].nDataOffset), SEEK_SET);
			fread(var_table_entries[i].szTxtData, 1, var_table_entries[i].nDataSize, fp);
#else
			_lseek64(fd, nOffset + header.nDataTableOffset + var_table_entries[i].nDataOffset, SEEK_SET);
			_read(fd, var_table_entries[i].szTxtData, var_table_entries[i].nDataSize);
#endif
			if(!bSFOInfoDisplayed) 
			{
				_verbose_printf(" >> %s: %s \n", var_table_entries[i].szName, var_table_entries[i].szTxtData);
			}
		} else if(var_table_entries[i].nType == 0x0404) {
			
			// numeric
			if(var_table_entries[i].nDataBlockSize == 0x04)
			{
				uint8_t temp[4];
				memset(&temp, 0, 4);
#ifdef WIN
				fseek(fp, (long)(nOffset + header.nDataTableOffset + var_table_entries[i].nDataOffset), SEEK_SET);
				fread(temp, 1, 4, fp);
#else
				_lseek64(fd, nOffset + header.nDataTableOffset + var_table_entries[i].nDataOffset, SEEK_SET);
				_read(fd, temp, 4);
#endif								
				swap16_data((uint8_t*)temp);
				var_table_entries[i].nNumData = data_to_u16((uint8_t*)temp);
				
				if(!bSFOInfoDisplayed) 
				{
					_verbose_printf(" >> %s: 0x%04X \n", var_table_entries[i].szName, var_table_entries[i].nNumData);
				}
			} 
			else if(var_table_entries[i].nDataBlockSize == 0x02) 
			{
				uint8_t temp[2];
				memset(&temp, 0, 2);
#ifdef WIN
				fseek(fp, (long)(nOffset + header.nDataTableOffset + var_table_entries[i].nDataOffset), SEEK_SET);
				fread(temp, 1, 2, fp);
#else
				_lseek64(fd, nOffset + header.nDataTableOffset + var_table_entries[i].nDataOffset, SEEK_SET);
				_read(fd, temp, 2);
#endif								
				swap8_data((uint8_t*)temp);
				var_table_entries[i].nNumData = data_to_u8((uint8_t*)temp);

				if(!bSFOInfoDisplayed) 
				{
					_verbose_printf(" >> %s: 0x%02X \n", var_table_entries[i].szName, var_table_entries[i].nNumData);
				}
			}
		}
	}
	
	if(!bSFOInfoDisplayed) 
	{
		_verbose_printf(SEP_LINE_2);
	}

	// If a variable name entry is specified then look for it...
	if(szEntry != NULL) 
	{		
		_verbose_printf("Searching variable data for [ %s ] \n", szEntry);

		for(i = 0; i < header.nTotalVariables; i++) 
		{	
			// CHECK FOR NUMERIC or TEXT DATA
			if(var_table_entries[i].nType == 0x0204)
			{
				// text
				if(memcmp(szEntry, var_table_entries[i].szName, strlen(szEntry))==0)
				{
					_verbose_printf("Found variable data for [ %s ]... [ %s ]\n", szEntry, var_table_entries[i].szTxtData);
					strcpy(szOut, var_table_entries[i].szTxtData);
					SAFE_FREE(var_table_entries);
					SAFE_FREE(var_table_entries_data);
#ifdef WIN
					fseek(fp, (long)nOffset, SEEK_SET);
#else
					_lseek64(fd, nOffset, SEEK_SET);
#endif
					bSFOInfoDisplayed = true;
					return 0;
				}

			} else if(var_table_entries[i].nType == 0x0404) {
			
				// numeric
				if(strcmp(szEntry, var_table_entries[i].szName)==0)
				{
					if(var_table_entries[i].nDataBlockSize == 0x04) {
						_verbose_printf("Found variable data for [ %s ]... [ 0x%04X ] \n", szEntry, var_table_entries[i].nNumData);
					} else if(var_table_entries[i].nDataBlockSize == 0x02) 
					{
						_verbose_printf("Found variable data for [ %s]... [ 0x%02X ] \n", szEntry, var_table_entries[i].nNumData);
					}

					uint64_t ret = var_table_entries[i].nNumData;
					
					SAFE_FREE(var_table_entries);
					SAFE_FREE(var_table_entries_data);
#ifdef WIN
					fseek(fp, (long)nOffset, SEEK_SET);
#else
					_lseek64(fd, nOffset, SEEK_SET);
#endif
					bSFOInfoDisplayed = true;
					return ret;
				}
			}	
		}
		_verbose_printf("Error: Variable data \"%s\" not found on SFO. \n", szEntry);
	}
	bSFOInfoDisplayed = true;
	return 0;
}

#ifdef WIN
int PatchPS3ISO(FILE* fp, char* szTitleID, uint8_t* vol_size)
{
	_verbose_printf("Preparing to patch PS3 ISO (%s)... \n", szTitleID);

	if(!fp) return 0; // wth?... xD
#else
int PatchPS3ISO(int fd, char* szTitleID, uint8_t* vol_size)
{
	if(fd == -1) return 0; // wth?... xD
#endif

	// Check for PS3 Disc header at first sector
	uint64_t nHdrPS3DiscIdOffset = 0x800;
#ifdef WIN
	fseek(fp, (long)nHdrPS3DiscIdOffset, SEEK_SET);
#else
	_lseek64(fd, nHdrPS3DiscIdOffset, SEEK_SET);
#endif

	uint8_t* ps3_disc_id = NULL;
	ps3_disc_id = (uint8_t*)malloc(0xC);
	memset(ps3_disc_id, 0, 0xC);

#ifdef WIN
	fread(ps3_disc_id, 1, 0xC, fp);
#else
	_read(fd, ps3_disc_id, 0xC);
#endif

	uint8_t _ps3_disc_id[] = { 'P', 'l', 'a', 'y', 'S', 't', 'a', 't', 'i', 'o', 'n', '3'};
	if(memcmp(_ps3_disc_id, ps3_disc_id, 0xC)==0)
	{
		// patched
		printf("PS3 ISO has proper disc header. No patching will be done. \n");
		return 1;
	} else {
		printf("PS3 ISO does not have a valid disc header, it will be patched now... \n");
	}

	uint8_t _ps3_hdr_p1[32] = {
		0x00, 0x00, 0x00, 0x02,								// unknown (always 0x02)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		vol_size[0], vol_size[1], vol_size[2], vol_size[3],	// total volume sectors (TOT_BYTES = TOT_VOL_SEC * 0x800)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	
	uint8_t _ps3_hdr_p2[64] = 
	{
		// PlayStation3
		0x50, 0x6C, 0x61, 0x79, 0x53, 0x74, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x33,
		// zeros
		0x00, 0x00, 0x00, 0x00,
		// title id (Ex. BLUS-00000)
		(uint8_t)szTitleID[0], (uint8_t)szTitleID[1], (uint8_t)szTitleID[2], (uint8_t)szTitleID[3], (uint8_t)'-', (uint8_t)szTitleID[4], (uint8_t)szTitleID[5], (uint8_t)szTitleID[6], (uint8_t)szTitleID[7], (uint8_t)szTitleID[8],
		// blank spaces
		0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
		0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
		0x20, 0x20,
		// zeros
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00
	};

#ifdef WIN
	fseek(fp, 0, SEEK_SET);
	fwrite(_ps3_hdr_p1, 1, sizeof(_ps3_hdr_p1), fp);

	fseek(fp, 0x800, SEEK_SET);
	fwrite(_ps3_hdr_p2, 1, sizeof(_ps3_hdr_p2), fp);
#else
	_lseek64(fd, 0, SEEK_SET);
	_write(fd, _ps3_hdr_p1, sizeof(_ps3_hdr_p1));

	_lseek64(fd, 0x800, SEEK_SET);
	_write(fd, _ps3_hdr_p2, sizeof(_ps3_hdr_p2)); 
#endif
	
	printf("PS3 ISO patching done! \n");

	return 1;
}

int psxProcessISO(char *szISO, int nSystem, char* szTitleID, char* szTitle, bool bPatchPS3ISO)
{
	// always display file name
	printf("ISO file: %s \n", szISO);

#ifdef WIN
	FILE* fp = NULL;
	fp = fopen(szISO, "r+b");
	if(fp) 
	{
#else
	int fd = _open(szISO, O_RDWR);
	if(fd != -1) 
	{
#endif	
		uint64_t nSectorSize	= 0x800;
		uint64_t nSectorHeader	= 0;
		uint64_t nOffset		= ((nSectorSize * 16) + nSectorHeader);

		// CD001
		uint64_t nStdIDOffset = 1;
#ifdef WIN
		fseek(fp, (long)(nOffset + nStdIDOffset), SEEK_SET);
#else
		_lseek64(fd, nOffset + nStdIDOffset, SEEK_SET);
#endif
		unsigned char* std_id = NULL;
		std_id = (unsigned char*)malloc(5);
		memset(std_id, 0, 5);

#ifdef WIN
		fread(std_id, 1, 5, fp);
#else
		_read(fd, std_id, 5);
#endif
		unsigned char _std_id[5] = {'C','D','0','0','1'};

		bool bSupportedISO = false;

		int nMode = 0;

		if(memcmp(std_id, _std_id, 5) == 0) 
		{
			_verbose_printf("Supported %s ISO (ISO9660/MODE1/2048) \n", szISOSystem[nSystem]);
			bSupportedISO = true;
			nMode = 1;
		} else {
		
			if(nSystem != ISO_SYSTEM_PS3)
			{
				// Try 0x930 sector size
				nSectorSize = 0x930;
				nSectorHeader = 0x18;
				nOffset = ((nSectorSize * 16) + nSectorHeader);
#ifdef WIN
				fseek(fp, (long)(nOffset + nStdIDOffset), SEEK_SET);
				fread(std_id, 1, 5, fp);
#else
				_lseek64(fd, nOffset + nStdIDOffset, SEEK_SET);
				_read(fd, std_id, 5);
#endif
				if(memcmp(std_id, _std_id, 5) == 0) {
					_verbose_printf("Supported %s ISO (ISO9660/MODE2/FORM1/2352) \n", szISOSystem[nSystem]);
					bSupportedISO = true;
					nMode = 2;
				}
			}
		}
		
		if(!bSupportedISO) {
			_verbose_printf("Error: The %s disc image is not supported / valid \n", szISOSystem[nSystem]);
			SAFE_FREE(std_id);
			return -1;
		}

		// VOLUME SIZE
		uint64_t nVolSize = 0;
		uint64_t nVolSizeOffset = nOffset + 0x50 + 4; // BE

#ifdef WIN
		fseek(fp, (long)nVolSizeOffset, SEEK_SET);
#else
		_lseek64(fd, nVolSizeOffset, SEEK_SET);
#endif

		uint8_t* vol_size = NULL;
		vol_size = (uint8_t*)malloc(4);
		memset(vol_size, 0, 4);

#ifdef WIN
		fread(vol_size, 1, 4, fp);
#else
		_read(fd, vol_size, 4);
#endif

		char szVolSize[32]; // BE
		ZERO(szVolSize);
		sprintf(szVolSize, "%02X%02X%02X%02X", vol_size[0], vol_size[1], vol_size[2], vol_size[3]);

#ifdef WIN
		nVolSize = strtoul(szVolSize, NULL, 16);
#else
		nVolSize = strtoull(szVolSize, NULL, 16);
#endif
		uint64_t nTotalVolSize = (nVolSize * 0x800);
		_verbose_printf("Volume Size: (0x%s sectors) (%lu bytes)\n", szVolSize, (unsigned long)nTotalVolSize);

		// ROOT DR
		uint64_t nRootDRLocOffset = 0x9E;
#ifdef WIN
		fseek(fp, (long)(nOffset + nRootDRLocOffset), SEEK_SET);
#else
		_lseek64(fd, nOffset + nRootDRLocOffset, SEEK_SET);
#endif		
		unsigned char *root_dr_sector = NULL;
		root_dr_sector = (unsigned char*)malloc(8);
		memset(root_dr_sector, 0, 8);		
#ifdef WIN
		fread(root_dr_sector, 1, 8, fp);
#else		
		_read(fd, root_dr_sector, 8);
#endif
		char szRootDROffset[64]; // BE
		ZERO(szRootDROffset);
		sprintf(szRootDROffset, "%02X%02X%02X%02X", root_dr_sector[4], root_dr_sector[5], root_dr_sector[6], root_dr_sector[7]);

		uint64_t nRootDROffset = 0;
#ifdef WIN
		nRootDROffset = strtoul(szRootDROffset, NULL, 16);
#else
		nRootDROffset = strtoull(szRootDROffset, NULL, 16);
#endif
		nRootDROffset = nRootDROffset * nSectorSize;

		_verbose_printf("Root Directory Record Offset: 0x%08X \n", (uint32_t)nRootDROffset);
		
#ifdef WIN
		fseek(fp, (long)nRootDROffset, SEEK_SET);
#else
		_lseek64(fd, nRootDROffset, SEEK_SET);
#endif
		// ======================================================
		// FIND SYSTEM.CNF (used for both PS1 and PS2 ISO)
		// ======================================================

		if(nSystem == ISO_SYSTEM_PS1 || nSystem == ISO_SYSTEM_PS2)
		{
			// ...

			unsigned char _SYSTEM_CNF[]	= { 'S','Y','S','T','E','M','.','C','N','F' };		
			unsigned char *SYSTEM_CNF	= NULL;
			size_t nLen = sizeof(_SYSTEM_CNF);
			SYSTEM_CNF = (unsigned char*)malloc(nLen);

			bool bFoundTitleIDFile = false;		
			uint64_t nPos = 0;

			while(nPos < nSectorSize) 
			{
				memset(SYSTEM_CNF, 0, nLen);
#ifdef WIN
				fread(SYSTEM_CNF, 1, nLen, fp);
#else
				_read(fd, SYSTEM_CNF, nLen);
#endif
				if(memcmp(SYSTEM_CNF, _SYSTEM_CNF, nLen) == 0)
				{
					_verbose_printf("SYSTEM.CNF file record found at pos: 0x%03X \n", (uint32_t)nPos);
					bFoundTitleIDFile = true;
					break;
				}
				nPos++;
#ifdef WIN
				fseek(fp, (long)(nRootDROffset + nPos), SEEK_SET);
#else
				_lseek64(fd, nRootDROffset + nPos, SEEK_SET);
#endif
			}

			if(!bFoundTitleIDFile) 
			{
				// Corrupted ISO, this should be present...
				_verbose_printf("Error: Couldn't find SYSTEM.CNF entry on the specified sector.\n");		
				SAFE_FREE(root_dr_sector);
				SAFE_FREE(SYSTEM_CNF);
#ifdef WIN
				SAFE_FCLOSE(fp);
#else
				SAFE_CLOSE(fd);
#endif
			} else {
				// SYSTEM.CNF Extent Location (Data location)
				uint64_t nDataOffset = (nRootDROffset + nPos) - 0x1F;

				unsigned char *extent_loc = NULL;
				extent_loc = (unsigned char*)malloc(8);
				memset(extent_loc, 0, 8);
#ifdef WIN
				fseek(fp, (long)nDataOffset, SEEK_SET);
				fread(extent_loc, 1, 8, fp);
#else
				_lseek64(fd, nDataOffset, SEEK_SET);
				_read(fd, extent_loc, 8);
#endif
				char szExtentOffset[64]; // BE
				ZERO(szExtentOffset);
				sprintf(szExtentOffset, "%02X%02X%02X%02X", extent_loc[4], extent_loc[5], extent_loc[6], extent_loc[7]);

				uint64_t nExtentOffset = 0;
#ifdef WIN
				nExtentOffset = strtoul(szExtentOffset, NULL, 16);
#else
				nExtentOffset = strtoull(szExtentOffset, NULL, 16);
#endif
				nExtentOffset = nExtentOffset * nSectorSize;
				_verbose_printf("SYSTEM.CNF Extent (data) Offset: 0x%08X \n", (uint32_t)nExtentOffset);

				// Data length(size)
				uint64_t nDataLenOffset = (nRootDROffset + nPos) - 0x17;

				unsigned char *data_len = NULL;
				data_len = (unsigned char*)malloc(8);
				memset(data_len, 0, 8);
#ifdef WIN
				fseek(fp, (long)nDataLenOffset, SEEK_SET);
				fread(data_len, 1, 8, fp);
#else
				_lseek64(fd, nDataLenOffset, SEEK_SET);
				_read(fd, data_len, 8);
#endif
				char szDataLen[64]; // BE
				ZERO(szDataLen);
				sprintf(szDataLen, "%02X%02X%02X%02X", data_len[4], data_len[5], data_len[6], data_len[7]);

				size_t nDataLen = 0;
#ifdef WIN
				nDataLen = strtoul(szDataLen, NULL, 16);
#else
				nDataLen = strtoull(szDataLen, NULL, 16);
#endif		
				_verbose_printf("SYSTEM.CNF Data Length: 0x%08X \n", nDataLen);

				char *title_id_file_extent_data = NULL;
				title_id_file_extent_data = (char*)malloc(nDataLen+1);
				memset(title_id_file_extent_data, 0, nDataLen+1);
#ifdef WIN
				fseek(fp, (long)(nExtentOffset + nSectorHeader), SEEK_SET);
				fread(title_id_file_extent_data, 1, nDataLen, fp);
#else
				_lseek64(fd, nExtentOffset + nSectorHeader, SEEK_SET);
				_read(fd, title_id_file_extent_data, nDataLen);
#endif						
				int nCount = 0;
				while(nCount < 30) 
				{
					nCount++;
					
					if(nSystem == ISO_SYSTEM_PS1) 
					{
						char* check = NULL;
						check = (char*)malloc(PS1_TITLE_ID_LEN);
						memset(check, 0, PS1_TITLE_ID_LEN);
						strncpy(check, title_id_file_extent_data+nCount, PS1_TITLE_ID_LEN);

						char _check_ps1[] = {'c','d','r','o','m',':','\\'};

						if(strncmp(check, _check_ps1, sizeof(_check_ps1))==0)
						{
							// 
							memcpy(szTitleID, title_id_file_extent_data+nCount+sizeof(_check_ps1), PS1_TITLE_ID_LEN);
							SAFE_FREE(check);
							break;
						}
						SAFE_FREE(check);
					}
					if(nSystem == ISO_SYSTEM_PS2) 
					{
						char* check = NULL;
						check = (char*)malloc(PS2_TITLE_ID_LEN);
						memset(check, 0, PS2_TITLE_ID_LEN);
						strncpy(check, title_id_file_extent_data+nCount, PS2_TITLE_ID_LEN);

						char _check_ps2[] = {'c','d','r','o','m','0',':','\\'};

						if(strncmp(check, _check_ps2, sizeof(_check_ps2))==0)
						{
							// 
							memcpy(szTitleID, title_id_file_extent_data+nCount+sizeof(_check_ps2), PS2_TITLE_ID_LEN);
							SAFE_FREE(check);
							break;
						}
						SAFE_FREE(check);
					}
				}
 
				if(nSystem == ISO_SYSTEM_PS1) {
					GetTitle(szTitleID, (char*)PS1_TITLE_DB, szTitle, ISO_SYSTEM_PS1);
				}
				if(nSystem == ISO_SYSTEM_PS2) {
					GetTitle(szTitleID, (char*)PS2_TITLE_DB, szTitle, ISO_SYSTEM_PS2);
				}

				SAFE_FREE(root_dr_sector);
				SAFE_FREE(SYSTEM_CNF);
				SAFE_FREE(extent_loc);
				SAFE_FREE(data_len);
				SAFE_FREE(title_id_file_extent_data);
#ifdef WIN
				SAFE_FCLOSE(fp);
#else
				SAFE_CLOSE(fd);
#endif
				return 1;
			}
		}

		// ======================================================
		// FIND PARAM.SFO (used for both PS3 and PSP ISOs)
		// ======================================================

		if(nSystem == ISO_SYSTEM_PS3 || nSystem == ISO_SYSTEM_PSP) 
		{
			uint64_t nPos = 0;
			uint64_t nParamOffset = 0;

			char szPS3_SYSTEM_FILE[] = { "PARAM.SFO" };
			unsigned char _PS3_SYSTEM_FILE[]	= { 'P','A','R','A','M','.','S','F','O' };

			unsigned char _PS3_GAME[] = { 'P','S','3','_','G','A','M','E'};

			if(nSystem == ISO_SYSTEM_PSP) {
				_PS3_GAME[2] = 'P';
			}

			unsigned char *PS3_GAME	= NULL;
			PS3_GAME = (unsigned char*)malloc(sizeof(_PS3_GAME));
			
			bool bFoundPS3GameDir = false;

			while(nPos < nSectorSize) 
			{
				memset(PS3_GAME, 0, sizeof(_PS3_GAME));
#ifdef WIN
				fread(PS3_GAME, 1, sizeof(_PS3_GAME), fp);
#else
				_read(fd, PS3_GAME, sizeof(_PS3_GAME));
#endif
				if(memcmp(PS3_GAME, _PS3_GAME, sizeof(_PS3_GAME)) == 0)
				{
					if(nSystem == ISO_SYSTEM_PS3) {
						_verbose_printf("PS3_GAME file record found at pos: 0x%03X \n", (uint32_t)nPos);
					}
					if(nSystem == ISO_SYSTEM_PSP) {
						_verbose_printf("PSP_GAME file record found at pos: 0x%03X \n", (uint32_t)nPos);
					}
					bFoundPS3GameDir = true;
					break;
				}
				nPos++;
#ifdef WIN
				fseek(fp, (long)(nRootDROffset + nPos), SEEK_SET);
#else
				_lseek64(fd, nRootDROffset + nPos, SEEK_SET);
#endif
			}

			if(!bFoundPS3GameDir) 
			{
				// Corrupted ISO, this should be present...
				if(nSystem == ISO_SYSTEM_PS3) {
					_verbose_printf("Error: Couldn't find PS3_GAME entry on the specified sector. ISO is invalid.\n");
				}

				if(nSystem == ISO_SYSTEM_PSP) {
					_verbose_printf("Error: Couldn't find PSP_GAME entry on the specified sector. ISO is invalid.\n");
				}

				SAFE_FREE(root_dr_sector);
				SAFE_FREE(PS3_GAME);
#ifdef WIN
				SAFE_FCLOSE(fp);
#else
				SAFE_CLOSE(fd);
#endif
				return -1;
			} else {
				// PS3_GAME Extent Location (Data location)
				uint64_t nDataOffset = (nRootDROffset + nPos) - 0x1F;

				unsigned char *extent_loc = NULL;
				extent_loc = (unsigned char*)malloc(8);
				memset(extent_loc, 0, 8);
#ifdef WIN
				fseek(fp, (long)nDataOffset, SEEK_SET);
				fread(extent_loc, 1, 8, fp);
#else
				_lseek64(fd, nDataOffset, SEEK_SET);
				_read(fd, extent_loc, 8);
#endif
				char szExtentOffset[64]; // BE
				ZERO(szExtentOffset);
				sprintf(szExtentOffset, "%02X%02X%02X%02X", extent_loc[4], extent_loc[5], extent_loc[6], extent_loc[7]);

				uint64_t nExtentOffset = 0;
#ifdef WIN
				nExtentOffset = strtoul(szExtentOffset, NULL, 16);
#else
				nExtentOffset = strtoull(szExtentOffset, NULL, 16);
#endif
				nExtentOffset = nExtentOffset * nSectorSize;

				if(nSystem == ISO_SYSTEM_PS3) {
					_verbose_printf("PS3_GAME Extent (data) Offset: 0x%08X \n", (uint32_t)nExtentOffset);
				}

				if(nSystem == ISO_SYSTEM_PSP) {
					_verbose_printf("PSP_GAME Extent (data) Offset: 0x%08X \n", (uint32_t)nExtentOffset);
				}

				nParamOffset = nExtentOffset;

				SAFE_FREE(PS3_GAME);
			}
			unsigned char *PS3_SYSTEM_FILE	= NULL;
			size_t nLen = sizeof(_PS3_SYSTEM_FILE);
			PS3_SYSTEM_FILE = (unsigned char*)malloc(nLen);

			bool bFoundTitleIDFile = false;		
			nPos = 0;

#ifdef WIN
			fseek(fp, (long)(nParamOffset + nPos), SEEK_SET);
#else
			_lseek64(fd, nParamOffset + nPos, SEEK_SET);
#endif
			while(nPos < nSectorSize) 
			{
				memset(PS3_SYSTEM_FILE, 0, nLen);
#ifdef WIN
				fread(PS3_SYSTEM_FILE, 1, nLen, fp);
#else
				_read(fd, PS3_SYSTEM_FILE, nLen);
#endif
				if(memcmp(PS3_SYSTEM_FILE, _PS3_SYSTEM_FILE, nLen) == 0)
				{
					_verbose_printf("%s file record found at pos: 0x%03X \n", szPS3_SYSTEM_FILE, (uint32_t)nPos);
					bFoundTitleIDFile = true;
					break;
				}
				nPos++;
#ifdef WIN
				fseek(fp, (long)(nParamOffset + nPos), SEEK_SET);
#else
				_lseek64(fd, nParamOffset + nPos, SEEK_SET);
#endif
			}
			
			if(!bFoundTitleIDFile) 
			{
				// Corrupted ISO, this should be present...
				_verbose_printf("Error: Couldn't find %s entry on the specified sector.\n", szPS3_SYSTEM_FILE);		
				SAFE_FREE(root_dr_sector);
				SAFE_FREE(PS3_SYSTEM_FILE);
#ifdef WIN
				SAFE_FCLOSE(fp);
#else
				SAFE_CLOSE(fd);
#endif
				return -1;
			} else {
				// PARAM.SFO Extent Location (Data location)
				uint64_t nDataOffset = (nParamOffset + nPos) - 0x1F;

				unsigned char *extent_loc = NULL;
				extent_loc = (unsigned char*)malloc(8);
				memset(extent_loc, 0, 8);
#ifdef WIN
				fseek(fp, (long)nDataOffset, SEEK_SET);
				fread(extent_loc, 1, 8, fp);
#else
				_lseek64(fd, nDataOffset, SEEK_SET);
				_read(fd, extent_loc, 8);
#endif
				char szExtentOffset[64]; // BE
				ZERO(szExtentOffset);
				sprintf(szExtentOffset, "%02X%02X%02X%02X", extent_loc[4], extent_loc[5], extent_loc[6], extent_loc[7]);

				uint64_t nExtentOffset = 0;
#ifdef WIN
				nExtentOffset = strtoul(szExtentOffset, NULL, 16);
#else
				nExtentOffset = strtoull(szExtentOffset, NULL, 16);
#endif
				nExtentOffset = nExtentOffset * nSectorSize;
				_verbose_printf("%s Extent (data) Offset: 0x%08X \n", szPS3_SYSTEM_FILE, (uint32_t)nExtentOffset);

				// Data length(size)
				uint64_t nDataLenOffset = (nParamOffset + nPos) - 0x17;

				unsigned char *data_len = NULL;
				data_len = (unsigned char*)malloc(8);
				memset(data_len, 0, 8);
#ifdef WIN
				fseek(fp, (long)nDataLenOffset, SEEK_SET);
				fread(data_len, 1, 8, fp);
#else
				_lseek64(fd, nDataLenOffset, SEEK_SET);
				_read(fd, data_len, 8);
#endif
				char szDataLen[64]; // BE
				ZERO(szDataLen);
				sprintf(szDataLen, "%02X%02X%02X%02X", data_len[4], data_len[5], data_len[6], data_len[7]);

				size_t nDataLen = 0;
#ifdef WIN
				nDataLen = strtoul(szDataLen, NULL, 16);
#else
				nDataLen = strtoull(szDataLen, NULL, 16);
#endif
				_verbose_printf("%s Data Length: 0x%08X \n", szPS3_SYSTEM_FILE, nDataLen);

#ifdef WIN
				fseek(fp, (long)(nExtentOffset + nSectorHeader), SEEK_SET);
#else
				_lseek64(fd, nExtentOffset + nSectorHeader, SEEK_SET);
#endif
				if(nSystem == ISO_SYSTEM_PS3) {
#ifdef WIN
					ParseSFO(fp, nExtentOffset + nSectorHeader, nDataLen, (char*)"TITLE_ID", szTitleID);
#else
					ParseSFO(fd, nExtentOffset + nSectorHeader, nDataLen, (char*)"TITLE_ID", szTitleID);
#endif
				} 
				if(nSystem == ISO_SYSTEM_PSP) {
#ifdef WIN
					ParseSFO(fp, nExtentOffset + nSectorHeader, nDataLen, (char*)"DISC_ID", szTitleID);
				}
				ParseSFO(fp, nExtentOffset + nSectorHeader, nDataLen, (char*)"TITLE", szTitle);
#else
					ParseSFO(fd, nExtentOffset + nSectorHeader, nDataLen, (char*)"DISC_ID", szTitleID);
				}
				ParseSFO(fd, nExtentOffset + nSectorHeader, nDataLen, (char*)"TITLE", szTitle);
#endif
				char szTmp[256];
				ZERO(szTmp);
				strcpy(szTmp, szTitle);
				utf8_to_ansi(szTmp, szTitle, (int)strlen(szTitle));

				// Patch PS3 ISO if needed
				if(nSystem == ISO_SYSTEM_PS3) 
				{
					if(bPatchPS3ISO == true) {
						_verbose_printf(SEP_LINE_2);
#ifdef WIN
						PatchPS3ISO(fp, szTitleID, vol_size); // this function assumes that the ISO was validated previously, so it will not do any extensive tests.
#else
						PatchPS3ISO(fd, szTitleID, vol_size); // this function assumes that the ISO was validated previously, so it will not do any extensive tests.
#endif
					} else {
						_verbose_printf(SEP_LINE_2);
						_verbose_printf("No PS3 ISO patching option flag detected (no patching done). \n");
					}
				}

				SAFE_FREE(root_dr_sector);
				SAFE_FREE(PS3_SYSTEM_FILE);
				SAFE_FREE(extent_loc);
				SAFE_FREE(data_len);
#ifdef WIN
				SAFE_FCLOSE(fp);
#else
				SAFE_CLOSE(fd);
#endif
				return 1;
			}
		}
	}
	return 0; // error: file not found
}


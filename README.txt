================================================================================
 PS ISO Tool (Supports PS1/PS2/PS3/PSP disc images) (CaptainCPS-X, 2013)
================================================================================

https://github.com/CaptainCPS/PS_ISO_Tool

 This tool will parse any of the supported PlayStation ISO / BIN. The module will 
 also check for valid (ISO9660 / MODE1 / 2048) or (MODE2 / 2352).

 When used as library for an application this can:

 - [PS1 / PS2] Get game Title ID from SYSTEM.CNF and obtain Title from a text database.
 - [PS3 / PSP] Get game Title and ID from the PARAM.SFO inside the ISO (no need for text database).
 - Titles gets automatically converted from UTF-8 to ASCII.
 - Provide a function to patch PS3 ISOs created with different applications than 
   GenPS3iso (Ex. ImgBurn, PowerISO).

 When used as application:

 - [Windows Only] Users can generate a ISO that will instantly be compatible with the PS3
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

Changelog:

v1.03 (November 11, 2013)

- GitHub created for project (https://github.com/CaptainCPS/PS_ISO_Tool)
- [source] Initial portability changes done and made Makefiles for both MSys/MinGW and MS Visual C++.
- [source] Sorted out compilations warnings and fixed possible memory leaks.

NOTE: This release still don't have support or checking 
for split files on directories when creating ISOs. Support for Split files
will eventually come on future versions.

v1.02 (November 11, 2013)

- Added feature to create ISOs using ImgBurn and patch them to comply with the PlayStation 3 disc header format.
- Did a couple of cosmetic changes.

v1.01 (November 9, 2013)

- Fixed typo, now all references to "vervose" are correctly named "verbose".
- Removed accidental duplicate macro definition on "psiso_tool.h".
 
v1.00 (November 9, 2013)

- Initial Release.


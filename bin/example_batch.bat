@echo off

psiso_tool --ps1 --verbose "I:\PSXISO\FINALFANTASY7_CD3.BIN"
@echo .
psiso_tool --ps2 --verbose "I:\PS2ISO\Capcom vs. SNK 2 - Mark of the Millennium 2001 (USA).iso"
@echo .
psiso_tool --ps3 --patch --verbose "I:\PS3ISO\BCUS98174-[The Last of Us].iso" 
@echo .
psiso_tool --psp --verbose "I:\PSPISO\b-ff4u.iso"
@echo .

pause
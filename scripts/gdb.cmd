@echo off
rem gdb.cmd -- Windows counterpart of gdb.sh: starts riscv32-esp-elf-gdb from
rem the ESP-IDF installation (path C, handout H1) with the given arguments.
rem The VS Code debug configuration (.vscode/launch.json) uses it as its
rem debugger on Windows. Prints nothing on stdout, which the debugger
rem extension reads as GDB/MI.
rem
rem The ESP-IDF Tools Installer sets IDF_TOOLS_PATH (default C:\Espressif);
rem GDB is in tools\riscv32-esp-elf-gdb\<version>\riscv32-esp-elf-gdb\bin.
setlocal
set "TOOLS=%IDF_TOOLS_PATH%"
if not defined TOOLS set "TOOLS=C:\Espressif"
set "GDB="
for /d %%V in ("%TOOLS%\tools\riscv32-esp-elf-gdb\*") do (
  if exist "%%V\riscv32-esp-elf-gdb\bin\riscv32-esp-elf-gdb.exe" set "GDB=%%V\riscv32-esp-elf-gdb\bin\riscv32-esp-elf-gdb.exe"
)
if not defined GDB (
  echo gdb.cmd: riscv32-esp-elf-gdb not found under %TOOLS%\tools -- see handout H1, path C 1>&2
  exit /b 1
)
"%GDB%" %*

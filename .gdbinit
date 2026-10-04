# Tell GDB to target 64-bit architectures natively
set architecture i386:x86-64

# Point GDB to your compiled ELF64 kernel binary to load your C++ function symbol tables
file Build/LunarOS

# Connect straight to the open QEMU socket session
target remote :1234

# Set a breakpoint right at your C++ entry point function
b kmain

# Continue execution up until your breakpoint is triggered
c

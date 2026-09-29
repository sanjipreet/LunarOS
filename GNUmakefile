OUTPUT := LunarOS
ISO_OUTPUT := $(OUTPUT).iso

# Compiler and Linker tools
CC := cc
LD := ld
NASM := nasm

# Kernel specific compilation flags
CFLAGS := -g -O2 -pipe -Wall -Wextra -std=gnu11 -ffreestanding \
          -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC \
          -ffunction-sections -fdata-sections -m64 -march=x86-64 \
          -mabi=sysv -mno-80387 -mno-mmx -mno-sse -mno-sse2 \
          -mno-red-zone -mcmodel=kernel

# Include search paths (pointing to your new Headers directory and root)
CPPFLAGS := -I Headers -I . -I limine-binary -MMD -MP

# Assembler flags
NASMFLAGS := -f elf64 -g -F dwarf -i Kernel/ -Wall
LDFLAGS   := -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000 \
             -z noexecstack --gc-sections -T Configs/linker.lds -no-pie

# Automatically locate source files inside your new Kernel/ folder
CFILES    := $(shell find Kernel -type f -name "*.c")
ASFILES   := $(shell find Kernel -type f -name "*.S")
NASMFILES := $(shell find Kernel -type f -name "*.asm")

# Map source files to temporary object paths
OBJ := $(patsubst Kernel/%,obj/%,$(CFILES:.c=.o) $(ASFILES:.S=.o) $(NASMFILES:.asm=.o))
DEPS := $(OBJ:.o=.d)

.PHONY: all clean iso

all: iso

# Pull in dependency rules (.d files) if they exist
-include $(DEPS)

# Download and compile Limine binaries if the directory doesn't exist
limine-binary:
	@echo "Downloading and extracting latest Limine binary release..."
	curl -fL -o limine-binary.tar.gz https://github.com/Limine-Bootloader/Limine/releases/latest/download/limine-binary.tar.gz
	gunzip < limine-binary.tar.gz | tar -xf -
	$(MAKE) -C limine-binary

# Link the kernel binary using the path inside Configs/
bin/$(OUTPUT): $(OBJ) Configs/linker.lds
	@mkdir -p bin
	$(LD) $(LDFLAGS) $(OBJ) -o $@
	@echo "Link successful: $@ created."

# Build the final bootable ISO image using the path inside Configs/
iso: bin/$(OUTPUT) limine-binary Configs/limine.conf
	@echo "Staging files for ISO building..."
	mkdir -p iso_root/boot/limine
	mkdir -p iso_root/EFI/BOOT
	cp -v bin/$(OUTPUT) iso_root/boot/
	cp -v Configs/limine.conf limine-binary/limine-bios.sys limine-binary/limine-bios-cd.bin \
	      limine-binary/limine-uefi-cd.bin iso_root/boot/limine/
	cp -v limine-binary/BOOTX64.EFI iso_root/EFI/BOOT/
	cp -v limine-binary/BOOTIA32.EFI iso_root/EFI/BOOT/
	@echo "Generating bootable ISO..."
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
	        -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
	        -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
	        -efi-boot-part --efi-boot-image --protective-msdos-label \
	        iso_root -o $(ISO_OUTPUT)
	@echo "Installing legacy BIOS boot sectors..."
	./limine-binary/limine bios-install $(ISO_OUTPUT)
	@echo "ISO generation successful: $(ISO_OUTPUT) created."
	@echo "Removing temporary object and staging folders..."
	rm -rf obj iso_root

# Compile C source files from Kernel/ folder
obj/%.o: Kernel/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Compile Assembly (.S) source files from Kernel/ folder
obj/%.o: Kernel/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Compile NASM (.asm) source files from Kernel/ folder
obj/%.o: Kernel/%.asm
	@mkdir -p $(dir $@)
	$(NASM) $(NASMFLAGS) -MD $(@:.o=.d) -MP $< -o $@

clean:
	rm -rf bin obj iso_root limine-binary limine-binary.tar.gz $(ISO_OUTPUT)

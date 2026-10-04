CFLAGS = -g -O2 -pipe -Wall -Wextra -std=c++20 -ffreestanding \
          -fno-stack-protector -fno-stack-check -fno-lto -fno-PIC \
          -ffunction-sections -fdata-sections -m64 -march=x86-64 \
          -mabi=sysv -mno-80387 -mno-mmx -mno-sse -mno-sse2 \
          -mno-red-zone -mcmodel=kernel -I ./Headers -I .

NASMFLAGS = -f elf64 -g -F dwarf -i Kernel/ -Wall

LDFLAGS = 	-m elf_x86_64 -nostdlib -static -z max-page-size=0x1000 \
        	-z noexecstack --gc-sections -T Configs/linker.lds -no-pie

OBJ = Build/gdtFlush.o Build/gdt.o Build/printf.o Build/port.o Build/idtStubs.o Build/pic.o Build/idt.o Build/kernel.o

Limine:
	curl -fL -o limine-binary.tar.gz https://github.com/Limine-Bootloader/Limine/releases/latest/download/limine-binary.tar.gz
	gunzip < limine-binary.tar.gz | tar -xf -
	$(MAKE) -C limine-binary

Build/LunarOS: $(OBJ)
	ld $(LDFLAGS) $(OBJ) -o $@

Build/%.o: Assembly/%.s 
	as --64 -o $@ $<

Build/%.o: Kernel/%.cpp
	g++ $(CFLAGS) -c $< -o $@

Build/%.o: C++/%.cpp
	g++ $(CFLAGS) -c $< -o $@

LunarOS.iso: Limine Build/LunarOS 
		mkdir -p iso_root/boot/limine
	mkdir -p iso_root/EFI/BOOT
	cp -v Build/LunarOS iso_root/boot/
	cp -v Configs/limine.conf limine-binary/limine-bios.sys limine-binary/limine-bios-cd.bin \
	      limine-binary/limine-uefi-cd.bin iso_root/boot/limine/
	cp -v limine-binary/BOOTX64.EFI iso_root/EFI/BOOT/
	cp -v limine-binary/BOOTIA32.EFI iso_root/EFI/BOOT/
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
	        -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
	        -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
	        -efi-boot-part --efi-boot-image --protective-msdos-label \
	        iso_root -o $@
	./limine-binary/limine bios-install $@
	rm -rf limine-binary iso_root limine*

run: LunarOS.iso 
	qemu-system-x86_64 -bios /usr/share/ovmf/x64/OVMF.4m.fd $<

debug: LunarOS.iso
	qemu-system-x86_64 -s -S -bios /usr/share/ovmf/x64/OVMF.4m.fd $<

.PHONY: clean 

clean:
	rm -rf LunarOS.iso Build 
	mkdir Build 


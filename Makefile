CC = x86_64-elf-gcc
LD = x86_64-elf-ld
AS = nasm

CFLAGS = -ffreestanding -nostdlib -lgcc -m32
INCLUDES = -Isrc/arch/ -Isrc/libs -Isrc/libs/libc/include

x86_asm_source_files := $(shell find src/ -name *.asm) 
x86_asm_object_files := $(patsubst src/%.asm, build/%.o, $(x86_asm_source_files)) 

x86_c_source_files := $(shell find src/ -name *.c) 
x86_c_object_files := $(patsubst src/%.c, build/%.obj, $(x86_c_source_files)) 

x86_c_headers_files := $(shell find src/ -name *.h) 

all: build-x86

build/%.o : src/%.asm
	mkdir -p $(dir $@) && \
	$(AS) -f elf32 $(patsubst build/%.o, src/%.asm, $@) -o $@

build/kernel/trap.obj : src/kernel/trap.c
	mkdir -p $(dir $@) && \
	$(CC) -c -g $(CFLAGS) $(INCLUDES) -mno-red-zone -mgeneral-regs-only $(patsubst build/%.obj, src/%.c, $@) -o $@

build/%.obj : src/%.c
	mkdir -p $(dir $@) && \
	$(CC) -c -g $(CFLAGS) $(INCLUDES) $(patsubst build/%.obj, src/%.c, $@) -o $@

.PHONY: build-x86
build-x86: $(x86_asm_object_files) $(x86_c_object_files)
	mkdir -p dist/ && \
	$(LD) -m elf_i386 -n -o dist/x86/kernel.bin -T targets/x86/linker.ld $(x86_c_object_files) $(x86_asm_object_files) && \
	cp dist/x86/kernel.bin targets/x86/iso/boot/kernel.bin && \
	grub-mkrescue /usr/lib/grub/i386-pc -o dist/x86/kernel.iso targets/x86/iso

clean:
	rm $(x86_c_object_files) $(x86_asm_object_files)

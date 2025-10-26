%define MBH_PAGE_ALIGN               (1 << 0)
%define MBH_MEM_INFO                 (1 << 1)
%define MBH_VIDEO_MODE               (1 << 2)
%define MBH_ADDR_FIELDS              (1 << 16)
%define MBH_MASK                     (MBH_MEM_INFO | MBH_VIDEO_MODE)

%define MBI_MEM                      (1 << 0)
%define MBI_BOOT_DEVICE              (1 << 1)
%define MBI_CMDLINE                  (1 << 2)
%define MBI_MODS                     (1 << 3)
%define MBI_AOUT_SYMS                (1 << 4)
%define MBI_ELF_SHDR                 (1 << 5)
%define MBI_MMAP                     (1 << 6)
%define MBI_DRIVES                   (1 << 7)
%define MBI_CONFIG_TABLE             (1 << 8)
%define MBI_BOOT_LOADER              (1 << 9)
%define MBI_APM_TABLE                (1 << 10)
%define MBI_VBE                      (1 << 11)
%define MBI_FRAMEBUFFER              (1 << 12)

%macro MB_SETFLAG 1
	push eax
    mov eax, [multiboot_info.flags]
    or eax, %1
    mov [multiboot_info.flags], eax
    pop eax
%endmacro
%macro MB_TESTFLAG 1
	push eax
    mov eax, [multiboot_header.flags]
    test eax, %1
    pop eax
%endmacro

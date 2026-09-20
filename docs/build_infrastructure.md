[Click here](../README.md) to view the README.

## Build infrastructure

This document describes the full build pipeline: how the version and build-number parameters are threaded through the build system, how each PPCA linker script selects the correct CODE SRAM slot at link time, and how the post-build Signer-Combiner tool assembles the final combined hex image.

### Build parameters

All build-number and version parameters are declared once in `common.mk` and propagated automatically to every sub-project:

Parameter       | Variable        | Description
:-------------- | :-------------- | :----------
`IMG_TYPE`      | `BOOT` / `UPDATE` | Controls the flash base address used by the Signer-Combiner (main bank `0x3200_0000` for BOOT, alternate bank `0x3280_0000` for UPDATE)
`IMG_VER_MAJOR` | 0–255           | MCUboot image version major field
`IMG_VER_MINOR` | 0–255           | MCUboot image version minor field
`IMG_REVISION`  | 0–65535         | MCUboot image version revision field
`IMG_BUILD_NO`  | 0–65535         | Build number; also acts as the dual-bank counter and selects the PPCA CODE SRAM slot (even → Slot A, odd → Slot B)

<br>

The `IMG_BUILD_NO` value is passed to every project in two ways:

- **Compiler define** – each sub-project Makefile adds `IMAGE_BUILD_NUM=$(IMG_BUILD_NO)` to `DEFINES` so that C source code can query the build number at compile time (see `PPCA_IS_EVEN_BUILD()` macro in `main_cm33_s/ppca_init.c`).
- **Linker define** – the PPCA sub-project Makefiles pass `BUILD_NUM` to the linker using a toolchain-specific flag so the linker script can select the correct CODE SRAM slot:

```makefile
# ppca_cm33_0/Makefile  (same pattern in ppca_cm33_1/Makefile)
ifeq ($(TOOLCHAIN),IAR)
LDFLAGS+=--config_def BUILD_NUM=$(IMG_BUILD_NO)
else ifeq ($(TOOLCHAIN),GCC_ARM)
LDFLAGS+=-Wl,--defsym=BUILD_NUM=$(IMG_BUILD_NO)
else ifeq ($(TOOLCHAIN),ARM)
LDFLAGS+=--predefine="-DBUILD_NUM=$(IMG_BUILD_NO)"
endif
```

### Pre-build step

The pre-build step is executed by `main_cm33_s/Makefile` before the main CM33 project is compiled:

```makefile
PREBUILD=$(PYTHON) ../configs/prebuild.py \
    --maj $(IMG_VER_MAJOR) --min $(IMG_VER_MINOR) \
    --rev $(IMG_REVISION)  --bn  $(IMG_BUILD_NO)  \
    --t ../configs/symbol_template.json \
    -o ../bsps/TARGET_$(TARGET)/symbols.json
```

`configs/prebuild.py` reads `configs/symbol_template.json` (which contains the placeholder `MAJ.MIN.REV+BN`) and writes `bsps/TARGET_APP_KIT_PSC3M8_EVK/symbols.json` with the resolved image-version string. The build number is stored with a device-specific mask (`0x5A3C0000 | IMG_BUILD_NO`) so it is also valid as the dual-bank counter value that BootROM uses to select the active flash bank.

The generated `symbols.json` is consumed by the post-build Signer-Combiner step as the source of the `{{ IMAGE_VERSION_STRING }}` template variable.

### PPCA linker slot selection

Each PPCA core has two equal CODE SRAM half-regions (Slot A and Slot B). The active slot is chosen at link time based on the parity of `BUILD_NUM`:

| Build number parity | Active slot | PPCA0 CODE SRAM range | PPCA1 CODE SRAM range |
|:-------------------|:------------|:----------------------|:----------------------|
| Even (0, 2, 4, …)  | Slot A      | `0x0000_0000 – 0x0000_3FFB` | `0x0000_0000 – 0x0000_3FFB` |
| Odd  (1, 3, 5, …)  | Slot B      | `0x0000_4000 – 0x0000_7FFB` | `0x0000_4000 – 0x0000_7FFB` |

The mechanism differs by toolchain but the logic is identical:

**GCC (`TOOLCHAIN_GCC_ARM/ppca*_linker_ns_ram.ld`)**

The GNU ld script includes the Device Configurator-generated memory map (`cymem_gnu_CM33_1.ld` / `cymem_gnu_CM33_2.ld`) which defines the `ppca0_code_A`, `ppca0_code_B`, `ppca0_crc_A`, and `ppca0_crc_B` MEMORY regions. The linker script then resolves the active start address and size as plain symbols using a conditional expression and places sections at those computed addresses directly — no dynamic MEMORY region is created, which avoids the GNU ld early-evaluation limitation:

```ld
INCLUDE cymem_gnu_CM33_1.ld

BUILD_NUM = DEFINED(BUILD_NUM) ? BUILD_NUM : 0;

ppca0_code_start = ((BUILD_NUM % 2) == 0) ? ORIGIN(ppca0_code_A) : ORIGIN(ppca0_code_B);
ppca0_crc_start  = ((BUILD_NUM % 2) == 0) ? ORIGIN(ppca0_crc_A)  : ORIGIN(ppca0_crc_B);
ppca0_code_size  = ((BUILD_NUM % 2) == 0) ? LENGTH(ppca0_code_A) : LENGTH(ppca0_code_B);
ppca0_crc_size   = ((BUILD_NUM % 2) == 0) ? LENGTH(ppca0_crc_A)  : LENGTH(ppca0_crc_B);

.text ppca0_code_start : AT(ppca0_code_start) { ... }
.crc_section ppca0_crc_start : { KEEP(*(.crc_section)) }
```

**IAR (`TOOLCHAIN_IAR/ppca*_linker_ns_ram.icf`)**

IAR's ICF language supports native `if/else` control flow and `isdefinedsymbol()`, so slot selection is straightforward:

```icf
include "../config/GeneratedSource/cymem_ilinkarm_CM33_1.icf";

if (!isdefinedsymbol(BUILD_NUM)) { define symbol BUILD_NUM = 0; }

if ((BUILD_NUM % 2) == 0) {
  define symbol PPCA0_CODE_START = CYMEM_CM33_1_ppca0_code_A_START;
  define symbol PPCA0_CODE_SIZE  = CYMEM_CM33_1_ppca0_code_A_SIZE;
  define symbol PPCA0_CRC_START  = CYMEM_CM33_1_ppca0_crc_A_START;
} else {
  define symbol PPCA0_CODE_START = CYMEM_CM33_1_ppca0_code_B_START;
  define symbol PPCA0_CODE_SIZE  = CYMEM_CM33_1_ppca0_code_B_SIZE;
  define symbol PPCA0_CRC_START  = CYMEM_CM33_1_ppca0_crc_B_START;
}
```

**ARM Compiler (`TOOLCHAIN_ARM/ppca*_linker_ns_ram.sct`)**

The scatter file starts with an `armclang -E` shebang so the ARM linker runs the file through the C preprocessor before linking. This allows standard C `#if`/`#define` directives:

```c
#! armclang -E --target=arm-arm-none-eabi -x c -mcpu=cortex-m33
#include "../config/GeneratedSource/cymem_armlink_CM33_1.sct"

#ifndef BUILD_NUM
#define BUILD_NUM 0
#endif

#if ((BUILD_NUM % 2) == 0)
#define PPCA0_CODE_START  CYMEM_CM33_1_ppca0_code_A_START
#define PPCA0_CODE_SIZE   CYMEM_CM33_1_ppca0_code_A_SIZE
#define PPCA0_CRC_START   CYMEM_CM33_1_ppca0_crc_A_START
#define PPCA0_CRC_SIZE    CYMEM_CM33_1_ppca0_crc_A_SIZE
#else
#define PPCA0_CODE_START  CYMEM_CM33_1_ppca0_code_B_START
...
#endif
```

### Compile-time size validation

`main_cm33_s/ppca_init.c` enforces at compile time that the A and B slot sizes are equal for each core, and that both PPCA cores have the same code region size. This guarantees the DMA copy logic can use a single `PPCA_IMAGE_SIZE` constant regardless of which slot is active:

```c
#define PPCA_IS_EVEN_BUILD(bn)  (((bn) % 2u) == 0u)

#if ((CYMEM_CM33_0_S_ppca0_code_A_SIZE != CYMEM_CM33_0_S_ppca0_code_B_SIZE) || \
     (CYMEM_CM33_0_S_ppca1_code_A_SIZE != CYMEM_CM33_0_S_ppca1_code_B_SIZE) || \
     (CYMEM_CM33_0_S_ppca0_code_A_SIZE != CYMEM_CM33_0_S_ppca1_code_A_SIZE))
#error "PPCA code sizes must match: A==B for each core and ppca0_A_SIZE==ppca1_A_SIZE"
#endif
```

### Post-build step — Signer-Combiner

After all three sub-projects have been compiled and linked, the main_cm33_s post-build step invokes the ModusToolbox Signer-Combiner tool with one of four JSON configuration files. The file is chosen automatically in `common.mk` based on `IMG_TYPE` and `IMG_BUILD_NO` parity:

```makefile
# common.mk
ifeq ($(filter %1 %3 %5 %7 %9, $(strip $(IMG_BUILD_NO))),)
  # Even build
  ifeq ($(IMG_TYPE),BOOT)
    COMBINE_SIGN_JSON?=configs/boot_even_build.json
  else
    COMBINE_SIGN_JSON?=configs/update_even_build.json
  endif
else
  # Odd build
  ifeq ($(IMG_TYPE),BOOT)
    COMBINE_SIGN_JSON?=configs/boot_odd_build.json
  else
    COMBINE_SIGN_JSON?=configs/update_odd_build.json
  endif
endif
```

Each JSON file drives the Signer-Combiner through three sequential stages:

**Stage 1 — Hex-relocate (×3)**

The linker produces per-project hex files whose addresses reflect the core's local address space. Relocation maps those addresses to their physical flash positions before merging:

- `main_cm33_s.hex` — the main CM33 secure code region is an alias address; it is relocated from the C-bus alias (`CYMEM_CM33_0_S_m33s_nvm_C_S_START`) to the S-bus physical address (`CYMEM_CM33_0_S_m33s_nvm_S_START`), producing `main_cm33_s_shifted.hex`.
- `ppca_cm33_0.hex` — the PPCA0 code image is relocated from its CODE SRAM origin (Slot A offset `ppca0_code_A_OFFSET` for even builds, Slot B offset `ppca0_code_B_OFFSET` for odd builds) to its flash storage address (`CYMEM_CM33_0_S_ppca0_nvm_S_START`), producing `ppca_cm33_0_flash.hex`.
- `ppca_cm33_1.hex` — same relocation to flash for PPCA1, producing `ppca_cm33_1_flash.hex`.

The table below shows which slot offset is used for each scenario:

| Config file               | PPCA slot used | Slot offset symbol (ppca0 / ppca1)               |
|:--------------------------|:---------------|:-------------------------------------------------|
| `boot_even_build.json`    | Slot A         | `ppca0_code_A_OFFSET` / `ppca1_code_A_OFFSET`   |
| `boot_odd_build.json`     | Slot B         | `ppca0_code_B_OFFSET` / `ppca1_code_B_OFFSET`   |
| `update_even_build.json`  | Slot A         | `ppca0_code_A_OFFSET` / `ppca1_code_A_OFFSET`   |
| `update_odd_build.json`   | Slot B         | `ppca0_code_B_OFFSET` / `ppca1_code_B_OFFSET`   |

**Stage 2 — Merge**

The three relocated hex files are merged into a single intermediate file `build/project_hex/app_combined_intermediate.hex`. Overlapping regions (e.g., flash padding gaps) are handled with `"overlap": "ignore"`.

**Stage 3 — Sign (MCUboot metadata)**

The merged hex is signed with MCUboot metadata using the `sign` command:

| Parameter        | Value                               | Notes                                            |
|:-----------------|:------------------------------------|:-------------------------------------------------|
| `header-size`    | `0x400`                             | 1 KB MCUboot header                              |
| `slot-size`      | `0x40000`                           | 256 KB — full flash bank size                    |
| `min-erase-size` | `0x200`                             | 512 B flash erase page                           |
| `overwrite-only` | `true`                              | MCUboot overwrite-only upgrade mode              |
| `hex-address`    | `0x32000000` (BOOT) / `0x32800000` (UPDATE) | Main bank vs. alternate bank base address |
| `image-version`  | `{{ IMAGE_VERSION_STRING }}`        | Resolved from `symbols.json` by prebuild step    |

The output is `build/app_combined.hex` — the final image ready for programming (BOOT) or DFU transfer (UPDATE).

**Figure 1. Post-build pipeline**

```
┌──────────────┐  ┌───────────────┐  ┌───────────────┐
│ main_cm33_s  │  │  ppca_cm33_0  │  │  ppca_cm33_1  │
│     .hex     │  │     .hex      │  │     .hex      │
└──────┬───────┘  └───────┬───────┘  └───────┬───────┘
       │ hex-relocate     │ hex-relocate     │ hex-relocate
       │ (C→S bus alias)  │ (SRAM→flash)     │ (SRAM→flash)
       ▼                  ▼                  ▼
┌──────────────┐  ┌───────────────┐  ┌───────────────┐
│ m33s_shifted │  │ ppca0_flash   │  │ ppca1_flash   │
│     .hex     │  │     .hex      │  │     .hex      │
└──────┬───────┘  └───────┬───────┘  └───────┬───────┘
       └──────────────────┼──────────────────┘
                          │ merge
                          ▼
               ┌─────────────────────┐
               │ app_combined_       │
               │ intermediate.hex    │
               └──────────┬──────────┘
                          │ sign (MCUboot header + trailer)
                          ▼
               ┌─────────────────────┐
               │   app_combined.hex  │  ← final image
               └─────────────────────┘
```

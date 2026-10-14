# stm32-playground

FreeRTOS firmware for the **STM32F407 Discovery** board, written in C and runnable with no
hardware: it runs in the [Renode](https://renode.io) emulator. The toolchain, emulator and
test tools all live in one Docker image, so the host needs only `docker` and `make`.

## Quick start

```sh
git submodule update --init --depth 1   # FreeRTOS, CMSIS, Unity
make image                              # build the dev container (once)
make run                                # build firmware, boot it, attach to its UART
```

```
stm32-playground: FreeRTOS V11.3.1, core clock 16000000 Hz
type 'help' for commands
> led blue on
blue led on
```

Press `Ctrl-]` to quit.

## VS Code

Open the folder and choose **Reopen in Container** (Dev Containers extension). The dev
container is the same image `make` uses, with clangd for C language support. It mounts the
repo at its host path, and the Makefile runs directly inside it. clangd reads
`compile_commands.json`, which `make build`, `make test-host` and `make compile-db` regenerate.
Run `make compile-db` after adding source files. Outside the container clangd can't see the ARM
toolchain's headers, so expect errors there.

## Make targets

| Target | What it does |
|---|---|
| `make` / `make build` | Cross-compile → `build/f407/firmware/firmware.{elf,bin,map}` |
| `make run` | Boot the firmware in Renode with your terminal attached to USART2 |
| `make test-host` | Build `lib/` natively and run the Unity unit tests (with ASan/UBSan) |
| `make test-emu` | Run the Robot Framework tests in `tests/renode/` against the emulated board |
| `make test` | Both of the above |
| `make debug` | Boot halted with a GDB server and open `gdb-multiarch`; `continue` to run |
| `make format` | Format our C sources with `.clang-format` |
| `make hooks` | Install git hooks: pre-commit on commit, `pre-commit-pr-run` on push (once per clone) |
| `make pre-commit-run` | Run the pre-commit hooks on staged changes |
| `make pre-commit-pr-run` | Run them on everything changed on this branch (`PR_BASE..HEAD`, default `origin/master`), plus the manual-stage hooks |
| `make shell` | Shell inside the dev container |
| `make image` | (Re)build the dev container |
| `make clean` | Remove `build/` |

Each target re-runs itself inside the container, so `make shell` followed by `make build`
works too. Pass extra gdb arguments with e.g. `make debug GDB_FLAGS="-ex 'break main'"`.
Renode's log (including UART output in debug mode) goes to `build/renode.log`; test
reports go to `build/renode-results/`.

## Layout

```
firmware/            the platform: startup, clock, drivers, FreeRTOS config, main()
  bsp/               board API apps use: LEDs (PD12-15), user button (PA0), console init
  drivers/           register-level GPIO and UART (USART2 on PA2/PA3, IRQ-driven RX)
apps/                applications, each a FreeRTOS task registered in firmware/main.c
  blinky/            blinks the green LED at 1 Hz
  shell/             UART command shell (help, led, button, uptime, tasks)
lib/                 hardware-independent code, shared by firmware and host tests
tests/host/          Unity unit tests for lib/
tests/renode/        Renode board description, machine script, and Robot tests
third_party/         git submodules: FreeRTOS-Kernel, CMSIS_5, cmsis_device_f4, Unity
docker/Dockerfile    antmicro/renode + arm-none-eabi-gcc, cmake, ninja, gdb-multiarch, lint tools
```

## Adding an app

1. Create `apps/<name>/<name>.c` with a `<name>_start()` that calls `xTaskCreate(...)`.
2. Declare `<name>_start` in `apps/apps.h` and add it to the `apps[]` table in `firmware/main.c`.
3. Add the source file to `apps/CMakeLists.txt`.

Apps should talk to hardware through `bsp.h` and `uart.h`, not registers. Logic with no
hardware dependency belongs in `lib/`, where it can be unit-tested on the host (add the file to
`corelib` in the top-level `CMakeLists.txt` and a test under `tests/host/`).

## Emulation notes

- The firmware runs from the 16 MHz internal oscillator (the reset default). The Renode
  board (`tests/renode/f407-disco.repl`) sets SysTick to 16 MHz to match, so FreeRTOS
  timing in the emulator matches real hardware. If you configure the PLL, update both.
- Renode's peripheral models are functional rather than cycle-accurate; anything
  timing-critical, analog, or using peripherals Renode doesn't model needs real hardware.
- On first run Renode downloads the STM32F40x SVD file; it is cached in `.cache/home`.

## On real hardware

The same `firmware.elf` runs on a physical STM32F407 Discovery, e.g. with
`openocd -f board/stm32f4discovery.cfg -c "program build/f407/firmware/firmware.elf verify reset exit"`.
The console is USART2 (PA2 TX, PA3 RX) at 115200 8N1, so you'll need a USB-serial adapter on
those pins.

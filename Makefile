# All commands run inside the dev container. On the host, each target re-invokes
# itself through `docker run`; inside the container (IN_CONTAINER=1) it runs directly.

IMAGE   ?= stm32-playground:dev
ELF     := build/f407/firmware/firmware.elf
GDBPORT ?= 3333

# Targets that run inside the container; included makefiles add to this list.
CONTAINER_TARGETS := build test-host run test-emu test debug format compile-db clean
.DEFAULT_GOAL := build

include Makefile.pre-commit

.PHONY: image shell $(CONTAINER_TARGETS)

ifneq ($(IN_CONTAINER),1)

TTY := $(shell [ -t 0 ] && echo -t)
# The repo is mounted at the same path so compile_commands.json and debug info work
# on the host too. HOME persists Renode's and pre-commit's caches between runs. The other
# -e flags forward variables the container targets use (GIT_INDEX_FILE is set by git hooks).
DOCKER_RUN = docker run --rm -i $(TTY) -u $(shell id -u):$(shell id -g) \
             -v $(CURDIR):$(CURDIR) -w $(CURDIR) -e HOME=$(CURDIR)/.cache/home \
             -e GDB_FLAGS -e GDBPORT -e PR_BASE -e GIT_INDEX_FILE $(DOCKER_FLAGS) $(IMAGE)

image:
	docker build -t $(IMAGE) docker

shell:
	@mkdir -p .cache/home
	$(DOCKER_RUN) bash

debug: DOCKER_FLAGS = -p $(GDBPORT):$(GDBPORT)

$(CONTAINER_TARGETS):
	@mkdir -p .cache/home
	@$(DOCKER_RUN) make $@

else

# clangd reads ./compile_commands.json: the firmware and host-test databases merged, so
# every file gets the flags of the build that actually compiles it.
MERGE_COMPILE_DB = @python3 -c 'import json, sys; json.dump([e for f in sys.argv[1:] for e in json.load(open(f))], open("compile_commands.json", "w"), indent=1)' \
                   $(wildcard build/f407/compile_commands.json build/host/compile_commands.json)

build:
	cmake --preset f407
	cmake --build --preset f407
	$(MERGE_COMPILE_DB)

test-host:
	cmake --preset host
	cmake --build --preset host
	ctest --preset host
	$(MERGE_COMPILE_DB)

# Configures both builds (no compiling) just to produce compile_commands.json.
compile-db:
	cmake --preset f407 >/dev/null
	cmake --preset host >/dev/null
	$(MERGE_COMPILE_DB)

# Renode runs in the background (setsid keeps Ctrl-C away from it); its log goes to build/.
RENODE   := setsid renode --disable-xwt --plain
UART_PTY := $(CURDIR)/build/uart
LOAD     := include @tests/renode/f407.resc

# Connects the terminal to the emulated board's UART. Ctrl-] quits.
run: build
	@rm -f $(UART_PTY)
	@$(RENODE) -e '$(LOAD); emulation CreateUartPtyTerminal "uart" "$(UART_PTY)" true' \
	    -e 'connector Connect sysbus.usart2 uart; start' > build/renode.log 2>&1 & \
	trap 'kill $$! 2>/dev/null' EXIT; \
	while [ ! -e $(UART_PTY) ]; do sleep 0.2; done; \
	echo "--- board UART (Renode log: build/renode.log). Ctrl-] to quit ---"; \
	socat -,raw,echo=0,escape=0x1d $(UART_PTY),raw,echo=0

test-emu: build
	renode-test --results-dir build/renode-results tests/renode/smoke.robot

test: test-host test-emu

# Starts the board halted with a GDB server; `continue` in gdb runs it. UART output
# goes to build/renode.log. Host tools (e.g. an IDE) can attach to localhost:$(GDBPORT).
debug: build
	@$(RENODE) -e '$(LOAD); showAnalyzer sysbus.usart2; machine StartGdbServer $(GDBPORT) true' \
	    > build/renode.log 2>&1 & \
	trap 'kill $$! 2>/dev/null' EXIT; \
	until grep -q "GDB server" build/renode.log 2>/dev/null; do sleep 0.2; done; \
	gdb-multiarch -q $(ELF) -ex 'target remote :$(GDBPORT)' $(GDB_FLAGS)

# Formats our C sources in place with .clang-format (third_party/ is left alone).
format:
	clang-format -i $(shell find apps firmware lib tests -name '*.[ch]')

clean:
	rm -rf build

endif

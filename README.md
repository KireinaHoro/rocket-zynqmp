## Rocket Chip on ZynqMP

This repo serves about the same purpose as the [Chipyard](https://github.com/ucb-bar/chipyard) project, as the root project for compatible generators to create a functional FPGA-based system for Vivado processing.  Besides, the repo also hosts the bootloader and a bootrom for convenience.

## Purposes for different branches

Each of the branches of this repo serves a different project.  Working with `git worktree` is strongly recommended to avoid chaos.

- `midgard`: work on the Midgard project.
	- Accelerators: No
	- Board Target: Baidu EdgeBoard FZ3
	- BootROM: No (Direct memory load from ARM)
	- OpenSBI payloads (checked = tested working):
		- [x] Baremetal test for PWM interrupt
		- [x] seL4
	- RAM: shared DRAM from PS
- `nvdla`: accelerators exploration project.
	- Accelerators: Gemmini, NVDLA (won't fit together!)
	- Board Target: ZCU102
	- BootROM: No (Direct memory load from ARM)
	- OpenSBI payloads (checked = tested working):
		- [x] Baremetal Zynq Monitor (Gemmini, for use with TVM)
		- [x] Linux
		- [ ] Baremetal test for SDP programming
	- RAM: DDR4 MIG
	- **Note**: DLA Chisel wrapper requires Linux build environment

## Simulation with Verilator

\[Documentation TODO\]

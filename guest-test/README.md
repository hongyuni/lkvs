# KVM/QEMU based Guest VM auto test framework

## Description
A simple auto guest VM test framework for several types of VM launched by KVM/QEMU.
Types of VM supported includes: legacy, tdx, tdxio,
may extend in future for specific type of VMs, 
in fact, the main differences among above VM types are QEMU config parameters.

As QEMU config parameters may vary from version to version, 
current implementation is QEMU version 7.2.0 based with tdx, tdxio special feature support.

The qemu.config.json and common test framework code need to change along with new QEMU update

## Limitaion
Each test execution will launch a new VM and run specific test scripts/binaries in guest VM,
test log will be captured from QEMU launching VM to test scripts/binaries execution in guest VM,
untill VM launched being shutdown properly or pkilled on purpose in abnormal test status.

No multi-VMs test scenarios covered/supported.

In any case of issue debugging, please refer to above test log with VM QEMU config info 
and launch VM and debug issues manually running test scripts/binaries.

A prepared Guest OS Image (qcow2 or raw image format) is requried with preset root account and password,
several values/parameters in qemu.config.json highly depend on Guest OS Image, please accomodate accordingly.

## Usage
### qemu.config.json description
grouped in 4 by 1st-level-keys: "common", "vm", "tdx", "tdxio"

group "common" includes all configurable values to be passed to group "vm", "tdx", "tdxio"
2nd-level-keys info:
"kernel_img": /abs/path/to/vmlinuz file or bzImage file of target VM guest kernel
"initrd_img": /abs/path/to/initrd file or initramfs file of target VM guest kernel
"bios_img": /abs/path/to/ovmf file or other bios file of target VM guest bios
"qemu_img": /abs/path/to/qemu-kvm or qemu-system-x86_64 file to boot target VM guest
"guest_img": /abs/path/to/VM guest OS image with qcow2 format or raw image format
"guest_img_format": value range in [qcow2/raw], guest os image file type qcow2 or raw image
"boot_pattern": Guest OS booting pattern shows bootup completed, depends on Guest OS image provided
"guest_root_passwd": Guest OS root account password
"vm_type": value range in [legacy/tdx/tdxio], VM type to test includes legacy vm, tdx vm or tdxio vm
"pmu": value range in [on/off], qemu config -cpu pmu=on or -cpu pmu=off
"cpus": value range in [1 ~ maximum vcpu number], qemu config -smp cpus=$VCPU
"sockets": value range in [1 ~ maximum sockets number], qemu config -smp sockets=$SOCKETS
"mem": value range in [1 ~ maximum mem size in GB], qemu config -m memory size in GB
"cmdline": value range in [guest kernel paramter extra string], qemu config -append extra command line parameters to pass to VM guest kernel
"debug": value range in [on/off], qemu config -object tdx-guest,debug=on or -object tdx-guest,debug=off

group "vm" includes all legacy vm 

## How to add new feature test

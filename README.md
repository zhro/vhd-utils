# vhd-utils

Simple command-line utilities to manipulate Virtual Hard Disks (VHD/VHDX).

This project is based on the original work by [neozeed](https://github.com/neozeed) in the [VHD-Utilities repository](https://github.com/neozeed/VHD-Utilities). The source code has been modified and extended with additional command-line options, size suffix support, and VHD geometry handling improvements.

**Version:** 1.1

## System Requirements

* **Windows 7 / Server 2008 R2** or newer is required for basic operations.
* **Windows 8 / Server 2012** or newer is required for in-place resizing (`resizevhd`) and VHDX support.

## Usage

### Create a VHD

```text
makevhd [-d] [-c] [-s] vhd size [source]

  -d      Use dynamic allocation for the virtual disk.
  -c      Use Windows-default CHS geometry behavior.
  -s      Preserve the requested size and use maximum CHS geometry.
  vhd     Path of the VHD file to be created.
  size    Maximum size of the VHD; must be a multiple of 512.
          Suffixes: b=bytes (default), k=KiB, m=MiB, g=GiB, t=TiB.
          If set to 0, the size of the source disk will be used.
  source  Path of an optional source whose data will populate the new VHD.
```

Create a differencing VHD:

```text
makevhd -f vhd parent

  -f      Create a differencing VHD.
  vhd     Path of the differencing VHD file to be created.
  parent  Path of an existing disk to be associated as the parent.
```

Resize a VHD in-place:

```text
resizevhd [-f] vhd size

  -f      Forcibly truncate the data stored within the VHD, if necessary.
  vhd     Path of the VHD file to be resized.
  size    Maximum size, in bytes, of the VHD; must be a multiple of 512.
          If set to 0, reduce the VHD to the smallest possible size.
          
Note: In-place resizing can only enlarge .vhd files, whereas .vhdx files can
  be both enlarged or shrunken. To shrink a .vhd  file or resize a disk, use
  makevhd to make a copy instead.
```

## Usage Examples

### Create a fixed-size VHD

By default, the requested capacity is rounded up when necessary to accommodate the CHS geometry stored in the VHD footer for legacy system compatibility

For example:

```text
makevhd disk.vhd 4m
```

Specify the size directly in bytes:

```text
makevhd disk.vhd 10485760
```

By default, a size request may result in a slightly larger virtual capacity, so that the reported CHS geometry can represent the disk.

Size suffixes are case-insensitive. The supported suffixes are `b` (bytes), `k` (KiB), `m` (MiB), `g` (GiB), and `t` (TiB). Unsuffixed values are interpreted as bytes.

### Preserve the exact requested size

Use `-s` to preserve the requested virtual disk size rather than rounding it up to a CHS-representable capacity to achieve a virtual capacity of exactly 4 MiB:

```text
makevhd -s disk.vhd 4m
```

This option sets the VHD footer to the maximum CHS values (`65535/16/255`) (representing a maximum capacity of 127.5 GiB) as a signal for modern cloud platforms and hypervisors to use exact byte lengths and LBA addressing.

Using exact sizing is critical for modern storage backends and cloud platforms (such as Xen and XenServer/XCP-ng) for several reasons:

* **Storage Allocation and Block Alignment:** Storage pools like LVM or ZFS manage data in clean binary increments. CHS padding introduces non-standard trailing sector counts that disrupt block alignment.

* **Migration and Conversion Tools:** Automated tooling often validates disk sizes against strict metadata limits, and CHS padding can cause import validation errors or replication failures.

* **Partition Integrity:** Modern guest OS partitioning utilities rely on exact byte lengths to calculate GPT backup header locations at the end of the disk.

If older software does not support maximum CHS values as a signal for LBA addressing, use the `-c` option instead to fall back to Windows-default CHS geometry.

### Create a dynamically allocated VHD

Use `-d` to create a dynamically allocated VHD whose backing file grows as data is written:

```text
makevhd -d disk.vhd 2g
```

This creates a virtual disk with a maximum capacity of 2 GiB without immediately allocating the entire capacity on the host filesystem.

### Use Windows-compatible sizing behavior

Use `-c` to retain the Windows-default VHD sizing and geometry behavior:

```text
makevhd -c disk.vhd 4m
```

Unlike the default QEMU-compatible sizing behavior, this option does not round the requested size up using the CHS geometry calculation.

If both `-s` and `-c` are specified, `-s` takes precedence and a warning is issued:

```text
makevhd -c -s disk.vhd 4m
```

### Create a VHD from a source disk or file

Provide an optional source to populate the new virtual disk:

```text
makevhd disk.vhd 2g source.img
```

The source argument supplies the data used to populate the new VHD. Ensure that the requested capacity is sufficient for the source data.

### Specify a size using different units

Equivalent ways to specify a 1 GiB capacity:

```text
makevhd disk.vhd 1073741824
makevhd disk.vhd 1048576k
makevhd disk.vhd 1024m
makevhd disk.vhd 1g
```

### Create a differencing VHD

Use `-f` to create a differencing VHD associated with an existing parent disk:

```text
makevhd -f child.vhd parent.vhd
```

The new `child.vhd` stores changes relative to `parent.vhd`. Keep the parent disk available and unchanged so the differencing disk can access its original data.

### Merging a differencing VHD with its parent into a single combined dynamic VHD:

```text
  makevhd -d merged.vhd 0 diff.vhd
```

### Size constraints

The minimum supported VHD size is 3,168,256 bytes (6,188 sectors). Disk sizes must be multiples of 512 bytes. Invalid or non-aligned sizes will result in errors or warnings.

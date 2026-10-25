# vhd-utils
These are simple utilities to manipulate Virtual Hard Disks (VHD/VHDX) from the command line.

VHD Utilities
Version 1.1

### Attribution

This project is based on the original work by [neozeed](https://github.com/neozeed) in the [VHD-Utilities repository](https://github.com/neozeed/VHD-Utilities). The original source code has been modified and extended with additional command-line options, size suffix support, and VHD geometry handling improvements.

## Introduction

These are simple utilities to manipulate Virtual Hard Disks (VHD/VHDX) from the
command line.


## System Requirements

Windows 7 / Server 2008 R2 or newer is required. Windows 8 / Server 2012 or
newer is required for in-place resizing (resizevhd) and VHDX support.


## Usage

Create a VHD:
makevhd [-d] [-c] [-s] vhd size [source]

  -d      Use dynamic allocation for the virtual disk.
  -c      Use Windows-default CHS geometry behavior.
  -s      Preserve the requested size and use maximum CHS geometry.
  vhd     Path of the VHD file to be created.
  size    Maximum size of the VHD; must be a multiple of 512.
          Suffixes: b=bytes (default), k=KiB, m=MiB, g=GiB, t=TiB.
          If set to 0, the size of the source disk, if available, will be used.
  source  Path of an optional source whose data will populate the new VHD.

Create a differencing VHD:
makevhd -f vhd parent

  -f      Create a differencing VHD.
  vhd     Path of the differencing VHD file to be created.
  parent  Path of an existing disk to be associated as the parent.

Resize a VHD in-place:
resizevhd [-f] vhd size

  -f      Forcibly truncate the data stored within the VHD, if necessary.
  vhd     Path of the VHD file to be resized.
  size    Maximum size, in bytes, of the VHD; must be a multiple of 512.
          Alternatively, DVD or BD can be specified for preset sizes.
          If set to 0, reduce the VHD to the smallest possible size.

Notes:
  In-place resizing of a virtual disk (using resizevhd) can only be used to
  enlarge .vhd files; .vhdx files can be either enlarged or shrunken using
  resizevhd. To shrink a .vhd file or to resize a disk on Windows 7, use
  makevhd instead (see Usage Examples below).


## Usage Examples

### Create a fixed-size VHD

Create a 10 MiB VHD with the default CHS geometry handling:

```cmd
makevhd disk.vhd 10m
```

Specify the size directly in bytes:

```cmd
makevhd disk.vhd 10485760
```

Size suffixes are case-insensitive. The supported suffixes are `b` (bytes), `k` (KiB), `m` (MiB), `g` (GiB), and `t` (TiB). Unsuffixed values are interpreted as bytes.

### Create a dynamically allocated VHD

Use `-d` to create a dynamically allocated VHD whose backing file grows as data is written:

```cmd
makevhd -d disk.vhd 2g
```

This creates a virtual disk with a maximum capacity of 2 GiB without immediately allocating the entire capacity on the host filesystem.

### Use QEMU-compatible CHS sizing

By default, the requested capacity is rounded up when necessary to accommodate the CHS geometry stored in the VHD footer.

For example:

```cmd
makevhd disk.vhd 4m
```

A 4 MiB request may result in a slightly larger virtual capacity so that the reported CHS geometry can represent the disk.

### Preserve the exact requested size

Use `-s` to preserve the requested virtual disk size rather than rounding it up to a CHS-representable capacity:

```cmd
makevhd -s disk.vhd 4m
```

This option sets the VHD footer geometry to the maximum CHS values (`65535/16/255`) and updates the footer checksum. The resulting virtual capacity remains exactly 4 MiB.

Use this option when exact capacity is more important than providing conventional CHS geometry metadata.

### Use Windows-compatible sizing behavior

Use `-c` to retain the Windows-default VHD sizing and geometry behavior:

```cmd
makevhd -c disk.vhd 4m
```

Unlike the default QEMU-compatible sizing behavior, this option does not round the requested size up using the CHS geometry calculation.

If both `-s` and `-c` are specified, `-s` takes precedence and a warning is issued:

```cmd
makevhd -c -s disk.vhd 4m
```

### Create a VHD from a source disk or file

Provide an optional source to populate the new virtual disk:

```cmd
makevhd disk.vhd 2g source.img
```

The source argument supplies the data used to populate the new VHD. Ensure that the requested capacity is sufficient for the source data.

### Create a differencing VHD

Use `-f` to create a differencing VHD associated with an existing parent disk:

```cmd
makevhd -f child.vhd parent.vhd
```

The new `child.vhd` stores changes relative to `parent.vhd`. Keep the parent disk available and unchanged so the differencing disk can access its original data.

### Specify a size using different units

The following commands illustrate equivalent ways to specify a 1 GiB capacity:

```cmd
makevhd disk.vhd 1073741824
makevhd disk.vhd 1048576k
makevhd disk.vhd 1024m
makevhd disk.vhd 1g
```

### Size requirements

The minimum supported VHD size is **3,168,256 bytes (6,188 sectors)**. Disk sizes must be multiples of 512 bytes. A size that is too small or invalid may be rejected, and a size that is not sector-aligned may produce an error or warning depending on the outcome of VHD creation.

Merging a differencing VHD with its parent into a single combined dynamic VHD:
  makevhd -d merged.vhd 0 diff.vhd

Merging into a fully-allocated VHD that can be backed up onto a DVD-R:
  makevhd merged.vhd DVD diff.vhd

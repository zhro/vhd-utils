# Building vhd-utils

This document describes how to compile and package the `vhd-utils` project from source.

> **Note:** This project has been built and tested with **Visual Studio 2015 only**.

## Prerequisites

Open the appropriate **Developer Command Prompt for Visual Studio** (e.g., *VS2015 x64 Native Tools Command Prompt* or *VS2015 x86 Native Tools Command Prompt*) to ensure all required environment variables are set.

---

## Common Build Commands

### standard `make` Commands

* **Default / Full Build:**
  ```text
  make
  ```
  *(or `make all` — specifying `all` is optional)*  
  Compiles all sources and creates the target executables in the appropriate output directory (`build/bin/<ARCH>/`).

* **Clean Build Files:**
  ```text
  make clean
  ```
  Removes temporary build objects, binaries, and intermediate directories.

* **Package Release Distribution:**
  ```text
  make dist
  ```
  Builds the binaries and packages the release distribution into a `.zip` archive located in the `dist/<ARCH>` directory.

---

## Helper Scripts

The repository includes helper batch scripts (`make.bat` and `clean.bat`) for convenience.

### `make.bat` 

`make.bat` is a wrapper script around `nmake`.

```text
Usage: make.bat [CPPVER=version] [ARCH=architecture] [targets...]

Options:
  CPPVER=version    Visual Studio toolset version (e.g., 110, 120, 140, 150)
  ARCH=architecture Target architecture (x86 or x64)
  --help            Display this help message
```

### `clean.bat`

`clean.bat` removes build artifacts directly, without requiring `nmake` or Visual Studio environment dependencies.

```text
Usage: clean.bat [-x86 | -x64 | --help]
```

---

## Optional Build Parameters

You can customize the build configuration by passing variables to `make`:

* **`ARCH`** (`x86` | `x64`):  
  Specifies the target architecture.  
  * **Auto-detection:** If omitted, `ARCH` is automatically detected by inspecting your Visual Studio command prompt environment variables (such as `PLATFORM`, `VSCMD_ARG_TGT_ARCH`, or the `LIB` path).
  * **Manual override example:**
    ```text
    make ARCH=x64
    ```

* **`CPPVER`**:  
  Specifies the Visual Studio C++ compiler version code (e.g., `140` for VS 2015).  
  * **Auto-detection:** If omitted, `CPPVER` is automatically detected by reading the `VISUALSTUDIOVERSION` environment variable.
  * **Manual override example:**
    ```text
    make CPPVER=140
    ```

---

## Managing Versions & Release Notes

* **Updating Version:** To bump the project version, update the version string inside the `VERSION` text file.
* **README Warning Check:** When running `make dist`, the build script automatically verifies whether `README.md` has been updated alongside the version change. A warning will be displayed if you forget to update `README.md`.
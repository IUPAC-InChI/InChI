# Official home of the InChI

- <em>Version 1.07.0</em> (17/07/2024):
  - ["WarnOnEmptyStructure option problem / Issue #25](https://github.com/IUPAC-InChI/InChI/issues/25) has been fixed
  - Software unresponsiveness when options are applied without any input file(s) has been fixed
  - Addressing <code>Google<sup>&reg;</sup> oss-fuzz</code> issues
  - Minor bugs fixes
- <em>Version 1.07-Release Candidate 4 (RC4)</em> (27/06/2024):
  - Addressing <code>Google<sup>&reg;</sup> oss-fuzz</code> issues
  - Minor bugs fixes
- <em>Version 1.07-Release Candidate 3 (RC3)</em> (13/06/2024):
  - Issues with executables compiled with `GCC` versions `12.x/13.x/14.x` has been fixed. Starting from `InChI v.1.07-RC3`, any version of `GCC` compiler can be used.
  - Addressing <code>Google<sup>&reg;</sup> oss-fuzz</code> issues
  - Minor bugs fixes
- <em>Version 1.07-Release Candidate 2 (RC2)</em> (04/06/2024):
  - Addressing <code>Google<sup>&reg;</sup> oss-fuzz</code> issues
  - Minor bugs fixes
- <em>Version 1.07-Release Candidate 1 (RC1)</em> (27/05/2024):
  - Addressing <code>Google<sup>&reg;</sup> oss-fuzz</code> issues
  - Minor bugs fixes
- <em>Beta5</em> (23/04/2024):
  - Addressing <code>Google<sup>&reg;</sup> oss-fuzz</code> issues
  - Licence updated
- <em>Beta4</em> (24/03/2024):
  - ["/b-layer" issue / Issue #9](https://github.com/IUPAC-InChI/InChI/issues/9#issue-2151944482) has been fixed
  - [Incorrect output message regarding CPU architecture on ARM64 CPUs / Issue #10](https://github.com/IUPAC-InChI/InChI/issues/10#issue-2169630260) has been fixed
  - Further <code>Google<sup>&reg;</sup> oss-fuzz</code> issues have been addressed

## Using precompiled binaries

64-bit and 32-bit precompiled binaries (executable, `.dll/.so` and ELF files) are located in the following folders:
<br />
<table id="Win">
  <tr>
    <td colspan="3" align="center"><strong>Microsoft<sup>&reg;</sup> Windows</strong></td>
  </tr>
  <tr>
    <td>
      <strong>Files</strong> (given in compressed <code>.zip</code> format)
    </td>
    <td>
      <strong>Location(s)</strong>
    </td>
    <td>
      <strong>Compiler</strong>
    </td>
  </tr>
  <tc>
    <td rowspan="2">
      <code>inchi-1.exe</code>
    </td>
    <td>
      <em>64-bit</em>: <code>INCHI-1-BIN/windows/64bit</code>
    </td>
    <td>
      <code>Microsoft<sup>&reg;</sup> Visual Studio C++ (MSVC)</code>
    </td>
  </tr>
  <tr>
    <td>
      <em>32-bit</em>: <code>INCHI-1-BIN/windows/32bit</code>
    </td>
    <td>
      <code>MinGW-w64/GCC</code>
    </td>
  </tr>
  <tc>
    <td rowspan="2">
      <code>libinchi.dll</code><br /> + corresponding <code>inchi_main.exe</code>
    </td>
    <td>
      <em>64-bit</em>: <code>INCHI-1-BIN/windows/64bit/dll</code>
    </td>
    <td>
      <code>Microsoft<sup>&reg;</sup> Visual Studio C++ (MSVC)</code>
    </td>
  </tr>
  <tr>
    <td>
      <em>32-bit</em>: <code>INCHI-1-BIN/windows/32bit/dll</code>
    </td>
    <td>
      <code>MinGW-w64/GCC</code>
    </td>
  </tr>
</table>
<br />
<table id="OtherOS">
  <tr>
    <td colspan="3" align="center"><strong>UNIX-based OSs and MacOS<sup>&reg;</sup></strong></td>
  </tr>
  <tr>
    <td>
      <strong>Files</strong> (given in compressed <code>.gz</code> format)
    </td>
    <td>
      <strong>Location(s)</strong>
    </td>
    <td>
      <strong>Compiler</strong>
    </td>
  </tr>
  <tc>
    <td>
      <code>inchi-1</code> (ELF file)
    </td>
    <td>
      <em>64-bit</em>: <code>INCHI-1-BIN/linux/64bit/</code><br />
      <em>32-bit</em>: <code>INCHI-1-BIN/linux/32bit/</code>
    </td>
    <td>
      <code>GCC</code>
    </td>
  </tr>
  <tc>
    <td>
      <code>libinchi.so.1.07</code><br /> + corresponding <code>inchi_main</code> (ELF file)
    </td>
    <td>
      <em>64-bit</em>: <code>INCHI-1-BIN/linux/64bit/so/</code><br />
      <em>32-bit</em>: <code>INCHI-1-BIN/linux/32bit/so/</code>
    </td>
    <td>
      <code>GCC</code>
    </td>
  </tr>
</table>
<!--Please note that 32-bit binaries have to be compiled from the source, although the use of 64-bit versions is highly recommended.-->
<br />

## Compiling `InChI v.1.07` from source

Project files for Microsoft<sup>&reg;</sup> `Visual C++ (MSVC)/Clang/LLVM` users are provided for both command line and API versions of `InChI v.1.07`. The project files are located in the following folders:

- `INCHI-1-SRC/INCHI_EXE/inchi-1/vc14` (command line version)
- `INCHI-1-SRC/INCHI_API/demos/inchi_main/vc14` (API version consisting of `libinchi.dll` and its corresponding executable `inchi_main.exe`)
- `INCHI-1-SRC/INCHI_API/libinchi/vc14` (API version consisting only of `libinchi.dll`).

For other C compilers, `makefile/makefile32` files are provided in the following folders:

- `INCHI-1-SRC/INCHI_EXE/inchi-1/gcc` (command line version)
- `INCHI-1-SRC/INCHI_API/demos/inchi_main/gcc` (API version consisting of `libinchi.dll/libinchi.so.1.07` and its corresponding executable/ELF `inchi_main.exe/inchi_main`)
- `INCHI-1-SRC/INCHI_API/libinchi/gcc` (API version consisting only of `libinchi.dll/libinchi.so.1.07`).

<a id="MAKEFILE"></a>

`makefile/makefile32` files are configured to detect OSs automatically, so it is no longer needed to specify OS explicitly or run batch/bash script(s) before compiling. If both `GCC` and `Clang/LLVM` compilers are detected, `GCC` is used by default; setting `Clang/LLVM` as default compiler can be done simply by changing `CCN` parameter from `1` to `2` in `makefile/makefile32`.

If `makefile/makefile32` is used for compiling `libinchi` on Microsoft<sup>&reg;</sup> Windows, `libinchi.dll` is now generated instead of `libinchi.so.1.07`.

<a id="BCF"></a>

In order to further improve code security, [bounds checking functions](https://wiki.sei.cmu.edu/confluence/display/c/Scope) (see Annex K of [C11 standard](https://en.cppreference.com/w/c/11)) can be optionally used in `InChI v.1.07`. Since a number of C compilers (e.g. `GNU GCC`) do not support bounds checking functions, they can be installed using some of the third-party open-source libraries such as:

- [safec libc extension with all C11 Annex K functions](https://github.com/rurban/safeclib) -- actively maintained
- [Safe C lib](https://sourceforge.net/projects/safeclib/)
- [Implementation of C11 Annex K "Bounds-checking interfaces" ISO/IEC 9899:2011](https://github.com/sbaresearch/slibc)

The use of bounds checking functions in `InChI v.1.07` can be enabled/disabled in `bcf_s.h`.

If you wish to use [Intel<sup>&reg;</sup> oneAPI Threading Building Blocks (oneTBB)](https://github.com/oneapi-src/oneTBB), please follow the instructions given in header files `mode.h` and `tbbmalloc_proxy.h`. Please note that the [pre-compiled binaries](#using-precompiled-binaries) mentioned in the previous section do not use `oneTBB`.

### Known issues

<!--
1. In case `GCC` is used to compile `InChI v.1.07`, `GCC` version `11.x` is recommended, since compiling any `InChI` version (incl. `v.1.07` and `v.1.06`) using `GCC` versions `12.x` and `13.x` can produce very rare segmentation errors whilst processing the following [PubChem](https://pubchem.ncbi.nlm.nih.gov/) structures: [453841144](https://pubchem.ncbi.nlm.nih.gov/substance/453841144), [453979403](https://pubchem.ncbi.nlm.nih.gov/substance/453979403), [450031964](https://pubchem.ncbi.nlm.nih.gov/substance/450031964), [449987684](https://pubchem.ncbi.nlm.nih.gov/substance/449987684), [448775139](https://pubchem.ncbi.nlm.nih.gov/substance/448775139), [448753468](https://pubchem.ncbi.nlm.nih.gov/substance/448753468), [448623641](https://pubchem.ncbi.nlm.nih.gov/substance/448623641). This issue is expected to be fixed in the forthcoming update of `InChI v.1.07`.
-->
If API version (i.e. `libinchi.so.1.07` and `inchi_main` ELF file) is compiled using `Clang/LLVM` on `Linux` OS, and `libinchi.so.1.07` cannot be found by `inchi_main`, `LD_LIBRARY_PATH` should be set either temporarily or permanently before `inchi_main` ELF file is used.
It might be worth trying to change the value of `LINKER_CWD_PATH` to `-Wl,-R,"",-rpath,$(LIB_DIR)` (i.e. replacing `=` with `,`) in corresponding `makefile/makefile32`; however, please note that during our tests, this option failed to generate `libinchi.so.1.07` with `Clang/LLVM` on Linux.
More reliably, `LD_LIBRARY_PATH` can be set in several ways:

   - Temporarily:
  
     - by running a shell script `ldlp_fix.sh` (located in `/INCHI_API/bin/Linux`) with either of these two commands:
       - `. ldlp_fix.sh`
       - `source ldlp_fix.sh`;

        path to `libinchi.so.1.07` can be edited in `ldlp_fix.sh`
     - using command line interface:

       ```
       export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/libinchi.so.1.07
       ```

   - Permanently:
     - by adding the following line in `~/.bashrc`:

         ```
         LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/path/to/libinchi.so.1.07"
         ```

     - by adding the `libinchi.so.1.07` path to `ld.so.conf`, which means adding a file `/etc/ld.so.conf.d/local.conf` containing just one line:

         ```
         path/to/libinchi.so.1.07
         ```

         and then running `sudo ldconfig`.

    - Open-source utility [patchelf](https://github.com/NixOS/patchelf) can also be of use.
  
    If a similar issue occurs on MacOS<sup>&reg;</sup>, one of the above solutions should be applied for setting `DYLD_LIBRARY_PATH` and/or `DYLD_FALLBACK_LIBRARY_PATH` (which behave like `LD_LIBRARY_PATH`).
<br />

## Experimental features under development

Some of the experimental/engineering/hidden options featured in `InChI 1.07` which are known to be not fully functional are:

- In command line version:

  - <em>32-bit</em> <code>Microsoft<sup>&reg;</sup> Visual Studio C++ (MSVC) Win32</code> compiler-specific issue with the following options:

    - `AMI`         Allow multiple input files (wildcards supported)
    - `AMIOutStd`   Write output to stdout (in AMI mode)
    - `AMILogStd`   Write log to stderr (in AMI mode)
    - `AMIPrbNone`  Suppress creation of problem files (in AMI mode)
  
<!-- 
  -  `MERGE`       Use bMergeAllInputStructures
  -  `DSB`         Use REQ_MODE_NO_ALT_SBONDS
  -  `NOHDR`       Use bNoStructLabels
  -  `NOUUSB`      Use REQ_MODE_SB_IGN_ALL_UU
  -  `NOUUSC`      Use REQ_MODE_SC_IGN_ALL_UU
  -  `FixRad`      Set bFixAdjacentRad
  -  `DoneOnly`    Set bIgnoreUnchanged
  -  `DISCONSALT:0|1`     Set bDisconnectSalts
  -  `DISCONMETAL:0|1`    Set bDisconnectCoord
  -  `DISCONMETALCHKVAL:0|1` Set bDisconnectCoordChkVal
  -  `RECONMETAL:0|1`     Set bReconnectCoord
  -  `MERGESALTTG:0|1`    Set bMergeSaltTGroups
  -  `UNCHARGEDACIDS:0|1` Set bUnchargedAcidTaut
  -  `ACIDTAUT:0|1|2`     Set bAcidTautomerism
  -  `AUXINFO:0|1|2`      Set AuxInfo print options
  -  `SDFID`       ...
  -  `PLAINP`      ....
  -  `ANNPLAIN`    ....
-->
- In API/`.dll`/`.so` version:
  - `PT_22_00`    Account for PT_22_00 tautomerism (experimental)
  - `PT_16_00`    Account for PT_16_00 tautomerism (experimental)
  - `PT_06_00`    Account for PT_06_00 tautomerism (experimental)
  - `PT_39_00`    Account for PT_39_00 tautomerism (experimental)
  - `PT_13_00`    Account for PT_13_00 tautomerism (experimental)
  - `PT_18_00`    Account for PT_18_00 tautomerism (experimental)
  - `Polymers105` Allow processing of polymers (experimental, legacy mode of v. 1.05)
  - `NoEdits`     Disable polymer CRU frame shift and folding
  - `NPZz`        Allow non-polymer-related Zz atoms (pseudo element placeholders)
  - `SAtZz`       Allow stereo at atoms connected to Zz(default: disabled)
  - `InChI2Struct` Test mode: Mol/SDfile -> InChI -> Structure -> (InChI+AuxInfo) -- produces `Fatal Error (2)3` just like in `InChI v.1.06`
  - `InChI2InChI`  Convert  Convert InChI string(s) into InChI string(s) -- produces `Fatal Error(2)3` just like in `InChI v.1.06`

Please refrain from using the above mentioned options as they might not function properly, or will not be recognised. Regular updates with regard to their functionality will be posted on this page.

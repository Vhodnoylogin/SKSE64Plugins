# Native qualification build prerequisites

The `build/shared-string-storage-se-20261029` integration branch contains the
shared-string lifetime repair plus minimal native build prerequisites. The
upstream lifetime PR remains isolated on `fix/shared-string-storage-teardown`.

The integration branch fixes task-delegate namespace references, preserves SKSE's
global delay-functor RTTI names through consumer aliases, supplies the missing
legacy bool/fixed-string serialization specializations, constructs temporary tint
materials through the real engine factory and releases them on the matching
scrap heap. Unresolved linker symbols are fatal; `/FORCE:UNRESOLVED` is removed.

CommonLib is pinned to the official fork's
`a60e57173f9cfaf4e4408346490d0fedfe49c5ed`, based on the original gitlink
`7a60f4de794095d7b0f8928d1b930a52e9a7da83`. Its one-line change invokes the
latent return-type resolver instead of assigning the resolver object.

## Reproduce

Clone with submodules, use Windows/MSVC x64, bootstrap vcpkg at the manifest's
`ee12231b20c95013c6638d845d04c91559a1d1ff`, and configure the
`release-msvc-vcpkg-static` preset. Choose an external build/cache directory with
`cmake --preset release-msvc-vcpkg-static -B <cache>/native` and set
`VCPKG_INSTALLED_DIR`, `VCPKG_DOWNLOADS` and `VCPKG_BINARY_SOURCES` to directories
inside that cache. Build `skee64`; no install step is required.

The inherited target selects AE support (Skyrim SE's 1.6.x executable family),
with SE1.5.x and VR support disabled. Do not advertise this DLL for those runtimes.

Offline checks:

```text
python tools/test-string-table-lifetime.py --output <cache>/lifetime
python tools/test-native-contracts.py --output <cache>/native-contracts
```

The native-contract checks compile byte-identical production helper headers
against narrow engine/serialization boundaries. They check legacy wire bytes,
empty/boundary/truncated inputs and separate-DLL global RTTI/resume-slot lookup.
They load only the small test fixture DLL, never `skee64.dll` or a game.

## Qualification limits

A complete strict native build and the offline checks are not gameplay
qualification or proof of binary equivalence to the installed Nexus B6 DLL.
Installed binaries are not replaced by these commands.

The inherited `AttachMeshLatentFunctor`/`DetachMeshLatentFunctor` bodies still have
separate known co-save defects: inverted filter-count success checks, failed form
resolution continuing to lookup, invalid-type handling being overwritten and a
null-reference dereference in detach save. The inherited generic uint32 reader
also accepts positive partial reads. The helper/RTTI checks do not test those
functor bodies. This integration candidate does not claim those latent co-save
paths are qualified. Their repair is separate from the shutdown fix and bounded
single-actor transformation/reversal investigation.

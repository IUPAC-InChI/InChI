See https://matklad.github.io//2021/02/06/ARCHITECTURE.md.html

The `src/sdf_pipeline` package is meant to provide a common abstraction for testing molecular identifiers (and/or descriptors),
such as InChI, SMILES, or TUCAN against molecules from [SDF files](https://en.wikipedia.org/wiki/Chemical_table_file#SDF).
`src/sdf_pipeline` parses molfiles from SDFs and distributes those molfiles over parallel consumer processes.
Any code that's specific to the molecular identifier that is being tested does not belong in `src/sdf_pipeline`.
In other words, you should be able to vendor the `src/sdf_pipeline` package into another project (e.g., TUCAN).

In contrast, the test code that's specific to the InChI tests lives under `tests`.

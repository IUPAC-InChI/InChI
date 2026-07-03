<!--
Thank you for contributing to InChI!

PR titles MUST follow Conventional Commits — release-please uses them
to drive versioning and the CHANGELOG. Examples:
  feat: add support for new polymer normalization
  fix: NULL deref in ichister.c when stereo center has no neighbors
  feat!: change default behavior of metal implicit-H handling  (the `!` marks a breaking change)

For more details see:
  https://github.com/googleapis/release-please
  https://www.conventionalcommits.org/en/v1.0.0/

Allowed types: feat, fix, perf, revert, build, chore, ci, docs, refactor,
                style, test

Do NOT embed phase or task numbers in the type/scope
(write `feat: ...`, not `feat(05-04): ...`).
-->

## Summary

<!-- 1–3 bullets describing the change and the motivation. -->

## Type of change

- [ ] feat — new feature (minor bump)
- [ ] fix — bug fix (patch bump)
- [ ] perf — performance improvement (patch bump)
- [ ] BREAKING CHANGE — incompatible change (`!` in title or footer; major bump)
- [ ] refactor / docs / chore / build / ci / test (no release)

## Checklist

- [ ] PR title follows Conventional Commits
- [ ] Builds cleanly for both targets (`./INCHI-1-TEST/build_with_cmake.sh all`)
- [ ] Compiles without new warnings on GCC/Clang/MSVC where applicable
- [ ] Code follows `.editorconfig` (4-space indent, LF, no trailing whitespace)
- [ ] Unit tests pass (`ctest` in the `full_build` test dir)
- [ ] CLI tests pass (`pytest INCHI-1-TEST/tests/test_executable`)
- [ ] New behavior is covered by tests
- [ ] Public API / behavior changes are documented (Doxygen in headers + README/docs if applicable)

## Related issues

<!-- Closes #123, refs #456, etc. -->

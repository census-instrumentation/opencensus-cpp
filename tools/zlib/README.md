`prometheus-cpp` imports `zlib` as `@net_zlib_zlib`.

`grpc` imports `zlib` as `@com_github_madler_zlib`.

We use this directory to join the diamond by creating a
`@net_zlib_zlib` external repository that just maps to
`@com_github_madler_zlib`.

# Docker images for CI/CD

The Dockerfiles in this folder are used to build images for CI/CD pipelines.

### Building images locally

```bash
docker build -f <path-to-dockerfile> -t <image-name> .
# for example:
docker build -f ./ubuntu-aqtinstall-6 -t zjeffer:notes/ubuntu-aqtinstall-6.4.3 .
```

### Pushing images to Docker Hub

```bash
docker push <image-name>
```

### AppImage update artifacts

The AppImage build produces a matching `.AppImage` and `.AppImage.zsync` pair. CI
includes both files in the AppImage artifact. The build fails if either file is
missing or if the AppImage does not contain the configured update information.

By default, AppImageUpdate follows stable releases of `nuttyartist/notes`, with a
separate asset pattern for each Qt major version and architecture. For Qt 6 on
x86_64, the embedded update information is:

```text
gh-releases-zsync|nuttyartist|notes|latest|Notes_*-Qt6.*-x86_64.AppImage.zsync
```

To build for another repository or update channel, pass
`-e LDAI_UPDATE_INFORMATION='...'` to `docker run`. The legacy
`UPDATE_INFORMATION` variable is also accepted; `LDAI_UPDATE_INFORMATION` takes
precedence when both are set. For a local update test, use a `zsync|http://...`
URL served by a local HTTP server that supports range requests.

When publishing a stable release, attach both generated files directly to the
same GitHub release. Do not change the AppImage after generating its `.zsync`
file. CI uploads build artifacts only; it does not create or publish a release.
Existing AppImages without update information need a one-time manual upgrade to
an enabled build. This enables external AppImageUpdate tools; Notes' built-in
update checker is configured separately.

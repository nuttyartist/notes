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

# syntax=docker/dockerfile:1@sha256:4edf897a3ffa55b89f906fc8cc78afdb3f1834cc9c7083565e611a8a7d5fe99e

# Images are locked by digest; when bumping a tag, update its digest too
# (docker buildx imagetools inspect <image:tag>).

# Ubuntu 22.04's cppcheck (2.7) predates --check-level, so build a current one.
FROM ubuntu:22.04@sha256:5ec03bb3441e8b0bf3b4f9cd4629a1ae763010dc3035bb8da3ae6cf026486401 AS cppcheck
ARG CPPCHECK_VERSION=2.22.0
SHELL ["/bin/bash", "-o", "pipefail", "-c"]
# Distro package versions are pinned by the base image.
# hadolint ignore=DL3008
RUN <<EOF
    set -eux
    apt-get update
    apt-get install -y --no-install-recommends ca-certificates curl g++ cmake make
    rm -rf /var/lib/apt/lists/*
    curl -fsSL "https://github.com/danmar/cppcheck/archive/refs/tags/${CPPCHECK_VERSION}.tar.gz" | tar -xz -C /tmp
    cmake -S "/tmp/cppcheck-${CPPCHECK_VERSION}" -B /tmp/build \
        -DCMAKE_BUILD_TYPE=Release -DFILESDIR=/opt/cppcheck/share -DCMAKE_INSTALL_PREFIX=/opt/cppcheck
    cmake --build /tmp/build -j"$(nproc)"
    cmake --install /tmp/build
EOF

FROM antmicro/renode:1.16.1@sha256:fc2a8c1bad2296a6d7cbc852bbf5540b22b778bdeb0ad42a45b8c54ea1e6a24c
ARG GITLEAKS_VERSION=8.30.1
ARG HADOLINT_VERSION=2.15.1
ARG PRE_COMMIT_VERSION=4.6.2
ARG CLANG_FORMAT_VERSION=23.1.3
ARG CLANGD_VERSION=22.1.1

USER 0
SHELL ["/bin/bash", "-o", "pipefail", "-c"]

# hadolint ignore=DL3008
RUN <<EOF
    set -eux
    apt-get update
    apt-get install -y --no-install-recommends \
        gcc-arm-none-eabi \
        libnewlib-arm-none-eabi \
        binutils-arm-none-eabi \
        gdb-multiarch \
        build-essential \
        cmake \
        ninja-build \
        make \
        socat \
        git \
        curl \
        ca-certificates \
        python3-pip \
        shellcheck
    rm -rf /var/lib/apt/lists/*
EOF

# pre-commit hooks use `language: system`, so their tools live in the image, as does
# clangd for VS Code in the dev container (.devcontainer/).
COPY --from=cppcheck /opt/cppcheck /opt/cppcheck
RUN <<EOF
    set -eux
    ln -s /opt/cppcheck/bin/cppcheck /usr/local/bin/cppcheck
    pip3 install --no-cache-dir "pre-commit==${PRE_COMMIT_VERSION}" "clang-format==${CLANG_FORMAT_VERSION}" \
        "clangd==${CLANGD_VERSION}"
    curl -fsSL "https://github.com/gitleaks/gitleaks/releases/download/v${GITLEAKS_VERSION}/gitleaks_${GITLEAKS_VERSION}_linux_x64.tar.gz" \
        | tar -xz -C /usr/local/bin gitleaks
    curl -fsSL -o /usr/local/bin/hadolint "https://github.com/hadolint/hadolint/releases/download/v${HADOLINT_VERSION}/hadolint-linux-x86_64"
    chmod +x /usr/local/bin/hadolint
EOF

# The Makefile checks this to run targets directly instead of via docker.
ENV IN_CONTAINER=1

# Fallback; the Makefile points HOME at .cache/home so Renode's and pre-commit's caches persist.
ENV HOME=/tmp
WORKDIR /tmp

# The Makefile runs containers as the host UID; this is the base image's own user.
USER 1000
ENTRYPOINT []
CMD ["bash"]

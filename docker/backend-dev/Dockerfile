# Run from project-root directory

FROM ubuntu:24.04 AS backend-dev-builder

RUN apt-get update && apt-get install -y \
    build-essential \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    git \
    ninja-build \
    cmake \
    python3 \
    bison \
    flex \
    autoconf \
    && rm -rf /var/lib/apt/lists/*

# Installing vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git /vcpkg
WORKDIR /vcpkg
RUN ./bootstrap-vcpkg.sh

# Env variables for vcpkg root directory
ENV VCPKG_ROOT=/vcpkg
ENV PATH="${VCPKG_ROOT}:${PATH}"

WORKDIR /project

COPY vcpkg.json .

RUN vcpkg install --triplet x64-linux

# Clear unused build files
RUN rm -rf \
    /vcpkg/downloads \
    /vcpkg/packages \
    /vcpkg/buildtrees


FROM ubuntu:24.04 AS backend-dev

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake g++ curl clang-tidy clang-format ninja-build git \
    && rm -rf /var/lib/apt/lists/*

# Copy only needed
COPY --from=backend-dev-builder /vcpkg /vcpkg

# Env variables for vcpkg root directory
ENV VCPKG_ROOT=/vcpkg
ENV PATH="${VCPKG_ROOT}:${PATH}"

WORKDIR /project
COPY --from=backend-dev-builder /project /project

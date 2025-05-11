# Run from project-root directory

FROM ubuntu:24.04 AS android-dev-builder

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
    ca-certificates \
    google-android-platform-34-installer \
    google-android-build-tools-34.0.0-install \
    google-android-cmdline-tools-13.0-install \
    google-android-ndk-r25-installer \
    && rm -rf /var/lib/apt/lists/*

# Installing vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git /vcpkg
WORKDIR /vcpkg
RUN ./bootstrap-vcpkg.sh

# Env variables for vcpkg root directory
ENV ANDROID_NDK_HOME=/usr/lib/android-ndk/
ENV VCPKG_ROOT=/vcpkg
ENV PATH="${VCPKG_ROOT}:${PATH}"

# Remove disabling neon (build error)
RUN sed -i 's/-DANDROID_ARM_NEON=OFF//g' /vcpkg/triplets/community/arm-android.cmake

WORKDIR /project

COPY vcpkg.json .

RUN vcpkg install --triplet arm-android --x-feature android

# Clear unused build files
RUN rm -rf \
    /vcpkg/downloads \
    /vcpkg/packages \
    /vcpkg/buildtrees


FROM ubuntu:24.04 AS android-dev

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake g++ curl clang-tidy clang-format ninja-build git ca-certificates \
    google-android-platform-34-installer \
    google-android-build-tools-34.0.0-install \
    google-android-cmdline-tools-13.0-install \
    google-android-ndk-r25-installer \
    && rm -rf /var/lib/apt/lists/*

# Copy only needed
COPY --from=android-dev-builder /vcpkg /vcpkg

# Env variables for vcpkg root directory
ENV ANDROID_NDK_HOME=/usr/lib/android-ndk/
ENV VCPKG_ROOT=/vcpkg
ENV PATH="${VCPKG_ROOT}:${PATH}"

WORKDIR /project
COPY --from=android-dev-builder /project /project

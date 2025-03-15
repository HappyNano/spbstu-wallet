# Установка

## Установка пакетов, vcpkg, android-sdk

```bash
sudo apt update
sudo apt install cmake g++ clang-format clang-tidy python3-pip pkg-config curl zip unzip tar -y

# Installing vcpkg
wget -O vcpkg.tar.gz https://github.com/microsoft/vcpkg/archive/refs/tags/2025.02.14.tar.gz
mkdir ~/vcpkg
tar xf vcpkg.tar.gz --strip-components=1 -C ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh
sudo ln -s ~/vcpkg/vcpkg /usr/local/bin/vcpkg
rm -rf vcpkg.tar.gz

# Installing android-sdk ndk
sudo apt install google-android-platform-34-installer -y
sudo apt install google-android-build-tools-34.0.0-installer -y
sudo apt install google-android-cmdline-tools-13.0-installer -y
sudo apt install google-android-ndk-r25-installer -y
```

Gradle неоходим версии 8+

## Установка переменных среды

```bash
export ANDROID_NDK_HOME=/usr/lib/android-ndk/
export VCPKG_ROOT=$HOME/vcpkg
export PATH=$PATH:$VCPKG_ROOT
```

## Установка pre-commit hook

```bash
cp scripts/pre-commit/pre-commit .git/hooks/
```

### pre-commit hook:
- Проверяет staged файлы на clang-format
- Проверяет наличие новой строки в конце файла

# Сборка

## CMD

```bash
mkdir build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
```

## VSCode

Для VSCode указан CMakePresets.json файл для сборки с помощью расширения Cmake-tools

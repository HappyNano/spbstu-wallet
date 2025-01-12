# Установка

## Установка пакетов

```bash
sudo apt update
sudo apt install cmake clang clang-format -y
```

## Установка pre-commit hook

```bash
cp scripts/pre-commit/pre-commit .git/hooks/
```

### pre-commit hook:
- Проверяет staged файлы на clang-format
- Проверяет наличие новой строки в конце файла

## Установка conan:

```bash
pip3 install conan
conan profile detect --force
```

# Сборка

## CMD

```bash
mkdir build
cmake -B build -S . -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=cmake/conan_install.cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=yes
cd build
cmake --build .
ctest
```

## VSCode

Для VSCode указан CMakePresets.json файл для сборки с помощью расширения Cmake-tools

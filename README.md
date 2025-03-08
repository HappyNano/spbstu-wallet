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

# Сборка

## CMD

```bash
mkdir build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
```

## VSCode

Для VSCode указан CMakePresets.json файл для сборки с помощью расширения Cmake-tools

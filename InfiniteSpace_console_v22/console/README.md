# Console engine (PC backend)

Сборка (Linux/macOS, нужен SDL2: `apt install libsdl2-dev` / `brew install sdl2`):

    cmake -S . -B build && cmake --build build -j
    ./build/demo

Windows: vcpkg (`vcpkg install sdl2`) + `-DCMAKE_TOOLCHAIN_FILE=.../vcpkg.cmake`.

Управление: стрелки, Z = A, X = B, Enter = Start, Backspace/RShift = Select.
Enter+Backspace — выход в лаунчер (на ПК закрывает окно). Esc — закрыть.

Папка `sdcard/` имитирует SD (другой путь: переменная окружения CONSOLE_SD).

Новая игра = новый файл с классом Game и `Game* create_game()`, плюс `add_executable` в CMakeLists.

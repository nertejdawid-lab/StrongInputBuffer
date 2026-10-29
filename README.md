# Strong Input Buffer — mod Geode do Geometry Dash

**Wersja:** 1.0.0 | **Cel:** Windows x64, Geometry Dash **2.2081**, Geode **5.11.2**.

Ten projekt zawiera rzeczywisty kod C++ moda Geode, pliki projektu, testy logiki bufora oraz gotowy workflow do wygenerowania instalowalnego pliku `.geode` na GitHub Actions.

> **Status:** logika bufora została przetestowana testami C++ (bez gry). **Nie skompilowano tutaj pliku `.geode` i nie przetestowano moda w uruchomionym Geometry Dash**, ponieważ to środowisko nie ma zainstalowanego Geode SDK ani gry. Nie traktuj ZIP-a ze źródłami jak moda do bezpośredniej instalacji.

## Jak działa

- Gra dostaje **każdy oryginalny klik od razu** — nie wprowadzamy sztucznego opóźnienia.
- Gdy wykonasz szybki klik **przed lądowaniem** w trybie cube, ball, robot lub spider i puścisz przycisk, mod zapamiętuje go na wybrane okno czasowe.
- Jeśli w tym czasie postać dotknie podłoża lub orba, mod wyśle dodatkowe naciśnięcie skoku (pulse), a następnie je zwolni po jednym kroku fizyki.
- Jeżeli **trzymasz przycisk**, gra obsługuje go normalnie; mod nie robi wtedy podwójnego skoku.
- Rozdzielne stany P1 i P2, w tym dual w trybie jednoosobowym.
- Zgon, restart poziomu i załadowanie checkpointu czyszczą zapisane kliknięcia.
- W trybie ship, UFO, wave i swing sterowanie pozostaje oryginalne: **nie ma tam dodatkowego buforowania**.

## Ustawienia w Geode

W liście modów Geode wybierz **Strong Input Buffer** -> **Settings**:

| Opcja | Znaczenie |
| --- | --- |
| Enable Input Buffer | Włącza/wyłącza mod |
| Buffer Strength (milliseconds) | Okno na wczesne kliknięcie, **0–250 ms**, domyślnie **100 ms** |
| Buffer Orbs / Rings | Pozwala użyć zapisanego kliku przy orbie (domyślnie ON) |

**Przykłady:** 40 ms = łagodnie, 100 ms = mocno, 160–250 ms = bardzo mocno. Bardzo duże wartości mogą powodować niezamierzone skoki.

## Jak uzyskać instalowalny plik `.geode` bez programowania

1. Załóż puste repozytorium na GitHub, np. `StrongInputBuffer`, bez automatycznego tworzenia README.
2. Rozpakuj ZIP z tym projektem i **wgraj zawartość folderu** do katalogu głównego repozytorium. Ważne: zachowaj folder `.github/workflows/` (to ukryty folder — w niektórych menedżerach plików trzeba włączyć wyświetlanie ukrytych plików). Możesz użyć `git`, jeżeli zwykłe przesyłanie plików na stronie GitHub nie zachowa ukrytego folderu.
3. Po wysłaniu plików otwórz zakładkę **Actions**, workflow **Build Strong Input Buffer (Windows)**. Uruchom go przyciskiem **Run workflow**, jeśli nie uruchomi się automatycznie.
4. Po zakończonej kompilacji kliknij ostatni udany przebieg i pobierz artefakt **StrongInputBuffer-Windows**. To ZIP zawierający **kompilowany plik `.geode`**.
5. Rozpakuj artefakt i skopiuj plik `community.strong-input-buffer.geode` do `Geometry Dash/geode/mods/`. Uruchom grę ponownie.

Jeśli w Actions pojawi się czerwony błąd, sprawdź log konkretnego kroku. Projekt jest punktem wyjścia do realnej kompilacji, **nie gwarantuje kompatybilności z nowszymi aktualizacjami GD/Geode**.

## Kompilacja lokalna (Windows)

Zainstaluj Geode CLI, Geode SDK v5.11.2, zależności Geode SDK, Clang/Visual Studio, CMake i Ninja według [oficjalnej instrukcji](https://docs.geode-sdk.org/getting-started/).

```powershell
geode sdk install
geode sdk update v5.11.2
geode sdk install-binaries
geode build
```

Gdy wszystko jest skonfigurowane, w `build/` będzie plik `.geode`. Samo wykonanie `cmake` bez SDK nie stworzy moda.

## Testy logiki bez gry

Na komputerze z kompilatorem C++ i CMake:

```sh
cmake -S . -B build-tests -DSTRONG_BUFFER_TEST_ONLY=ON
cmake --build build-tests
ctest --test-dir build-tests --output-on-failure
```

Testy sprawdzają m.in.: szybki wczesny klik, trzymanie przycisku, wygaśnięcie bufora, orby, P1/P2, reset i ponowny fizyczny klik podczas syntetycznego pulsu. **Testy logiki nie zastępują testów w Geometry Dash.**

## Kompatybilność / ostrożność

- Projekt celuje w **Geometry Dash 2.2081 na Windows x64**; może nie działać z **2.209** i nowszymi wersjami gry bez aktualizacji bindów.
- Może konfliktować z innymi modami, które modyfikują tę samą metodę `GJBaseGameLayer::handleButton` lub `processCommands`.
- Bufor ułatwia wciskanie przycisków, dlatego nie powinien być traktowany jako wynik bez asysty w rankingach lub konkursach.
- Jeśli kontrola zacznie działać dziwnie, wyłącz **Enable Input Buffer**, a potem spróbuj bez innych modów wejściowych.

## Pliki

- `src/main.cpp` — haki Geode i podłączenie do stanu gry.
- `src/JumpBuffer.hpp` — niezależna, testowalna logika bufora.
- `mod.json` — metadane i ustawienia w grze.
- `.github/workflows/build.yml` — automatyczna kompilacja `.geode` na Windows.
- `tests/engine_tests.cpp` — testy logiki.

Własny projekt demonstracyjny na licencji MIT, bez kopiowania kodu innych modów.

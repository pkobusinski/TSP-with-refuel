#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 5
int tsp[N][N];
int gas_stations[N];
int start_fuel;

void generate_random_data() {
    // Inicjalizacja generatora liczb losowych
    srand(time(NULL));

    // Wypełnienie macierzy odległości losowymi wartościami
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (i == j) {
                tsp[i][j] = 0;  // Odległość z miasta do siebie samego wynosi 0
            }
            else {
                tsp[i][j] = 1 + rand() % 100;  // Losowa odległość między miastami (1-100)
            }
        }
    }

    // Wypełnienie tablicy stacji benzynowych losowymi ilościami paliwa
    for (int i = 0; i < N; i++) {
        gas_stations[i] = 10 + rand() % 40;  // Losowa ilość paliwa na stacji (10-49)
    }

    // Ustawienie początkowej ilości paliwa na losową wartość
    start_fuel = 50 + rand() % 100;  // Losowa ilość początkowego paliwa (50-149)
}


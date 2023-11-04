#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include "cJSON.h"


#define N 5  // Liczba miast
#define IT 10

int tsp[N][N];
int gas_stations[N];
int start_fuel = 40;


void shuffle(int array[N], int size) {              // funkcja potrzebna do random_greedy_tsp 
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

void copy_array(int source[], int destination[], int length) {
    for (int i = 0; i < length; i++) {
        destination[i] = source[i];
    }

}

int can_reach_gas_station(int current_fuel, int distance, int station_fuel) {
    return current_fuel >= distance ;
}

int greedy_tsp(int path[N+1], int fuel) {

    int unvisited_cities[N];
    int current_city = 0;
    int current_fuel = fuel;
    int path_length = 0;

    for (int i = 0; i < N; i++) {
        unvisited_cities[i] = 1;               
    }

    unvisited_cities[current_city] = 0; 
    path[0] = current_city;

    for (int i = 1; i < N; i++) {
        int min_cost = -1;
        int next_city = -1;

        for (int j = 0; j < N; j++) {
            if (unvisited_cities[j]) {
                int cost = tsp[current_city][j];
                if (min_cost == -1 || cost < min_cost) {
                    if (can_reach_gas_station(current_fuel, cost, gas_stations[j])) {
                        min_cost = cost;
                        next_city = j;
                    }
                }
            }
        }

        if (next_city == -1) {
            // brak dostępnych miast, powrót do miasta początkowego
            next_city = 0;
            min_cost = tsp[current_city][0];
        }

        path_length += min_cost;
        current_city = next_city;
        current_fuel -= min_cost;
        current_fuel += gas_stations[current_city];
        path[i] = current_city;
        unvisited_cities[current_city] = 0;
    }

    path[N] = 0;
    path_length += tsp[current_city][0];

    for (int i = 0; i < N; i++) {
        if (unvisited_cities[i] == 1)
            return 0;
    }

    return path_length;
}

void fill_array_with_indices(int array[], int length) {

    for (int i = 1; i < length - 1; i++) {
        array[i] = i;
    }

    array[0] = 0;
    array[length - 1] = 0;
}

int random_greedy_tsp(int path[N+1], int fuel) {
    int unvisited_cities[N];
    int error_path[N + 1];
    fill_array_with_indices(error_path, N + 1);
    int current_city = 0;
    int current_fuel = fuel;
    int path_length = 0;

    for (int i = 0; i < N; i++) {
        unvisited_cities[i] = 1; 
    }

    unvisited_cities[current_city] = 0; 
    path[0] = current_city;

    for (int i = 1; i < N; i++) {
        int available_cities[N];
        int available_count = 0;

        for (int j = 0; j < N; j++) {
            if (unvisited_cities[j] && can_reach_gas_station(current_fuel, tsp[current_city][j], gas_stations[j])) {
                available_cities[available_count] = j;
                available_count++;
            }
        }

        if (available_count == 0) {
            // Brak dostępnych miast, powrót do miasta początkowego
            available_cities[0] = 0;
        }

        shuffle(available_cities, available_count);

        int next_city = available_cities[0];
        path[i] = next_city;
        unvisited_cities[next_city] = 0;
        path_length += tsp[current_city][next_city];
        current_fuel -= tsp[current_city][next_city];
        current_fuel += gas_stations[next_city];
        current_city = next_city;
    }

    path[N] = 0;
    path_length += tsp[current_city][0];

    if (!can_reach_gas_station(current_fuel, tsp[current_city][0], gas_stations[0])) {
        copy_array(error_path, path, N + 1);
        return 0;
    }

    for (int i = 0; i < N; i++) {
        if (unvisited_cities[i] == 1 || current_fuel < 0) {
            copy_array(error_path, path, N + 1);
            return 0;
        }
    }

    return path_length;
}

void generatePermutations(int step, int length, int path[N], int unvisited[N], int* min_path_length, int best_path[N + 1], int* fuel, int tsp[N][N], int gas_stations[N]) {
    if (step == N) {
        length += tsp[path[N - 1]][0];
        if (length < *min_path_length) {
            // Sprawdź, czy paliwo pozwala na powrót
            int required_fuel = tsp[path[N - 1]][0];
            if (*fuel >= required_fuel) {
                *min_path_length = length;
                for (int i = 0; i < N; i++) {
                    best_path[i] = path[i];
                }
                best_path[N] = 0;
            }
        }
        return;
    }

    for (int i = 1; i < N; i++) {
        if (unvisited[i] && can_reach_gas_station(*fuel, tsp[path[step-1]][i], gas_stations[i])) {//tsp[path[step - 1]][i] <= *fuel) {
            path[step] = i;
            unvisited[i] = false;
            int prev_fuel = *fuel; // Zapamiętaj ilość paliwa przed przejazdem
            *fuel -= tsp[path[step - 1]][i]; // Aktualizacja ilości paliwa
            *fuel += gas_stations[i];

            generatePermutations(step + 1, length + tsp[path[step - 1]][i], path, unvisited, min_path_length, best_path, fuel, tsp, gas_stations);

            unvisited[i] = true;
            *fuel = prev_fuel; // Przywróć ilość paliwa po powrocie
        }
    }
}

int bruteforce_tsp(int path[N + 1], int fuel) {
    int local_path[N];
    int unvisited[N];
    int best_path[N + 1];
    int min_path_length = INT_MAX;
    int current_fuel = fuel;

    for (int i = 0; i < N; i++) {
        unvisited[i] = true;
    }

    local_path[0] = 0;
    unvisited[0] = false;

    generatePermutations(1, 0, local_path, unvisited, &min_path_length, best_path, &current_fuel, tsp, gas_stations);

    for (int i = 0; i < N + 1; i++) {
        path[i] = best_path[i];
    }

    for (int i = 1; i < N + 1; i++)
    {
        if (path[i] == path[i - 1]) {

            return 0;
        }
    }

    return min_path_length;
}

bool check_if_route_ok(int path[N + 1],int gas_stations[N], int fuel) {
    int current_fuel = fuel;

    for (int i = 0; i < N; i++) {
        if (current_fuel < tsp[path[i]][path[i + 1]]) {
            return false;  // Brak wystarczającej ilości paliwa na odcinku między miastami
        }
        current_fuel -= tsp[path[i]][path[i + 1]];  // Zużywanie paliwa na danym odcinku

        current_fuel += gas_stations[path[i + 1]];  // Dodanie paliwa z bieżącej stacji
    }

    // Sprawdzamy, czy można wrócić do miasta początkowego
    if (current_fuel < tsp[path[N]][path[0]]) {
        return false;  // Brak wystarczającej ilości paliwa na ostatnim odcinku
    }
    return true;  // Trasa jest wykonalna z dostępnym paliwem
}

int calculate_path_length(int path[N + 1],int gas_stations[N], int fuel) {
    
    int length = 0;
    for (int i = 1; i < N + 1; i++) {
        length += tsp[path[i]][path[i - 1]];
        if (path[i] == path[i - 1])
            return INT_MAX;
    }
    if (!check_if_route_ok(path, gas_stations, fuel))
        return INT_MAX;
    else
        return length;
}

void change_path(int source[], int destination[], int length) {
    // Inicjalizacja tablicy destination tak samo jak source
    for (int i = 0; i < length; i++) {
        destination[i] = source[i];
    }

    // Wybieranie dwóch losowych indeksów do zamiany
    int index1, index2;
    do {
        index1 = 1 + rand() % (length - 2);  // Wybieramy losowy indeks (z wyjątkiem pierwszego i ostatniego)
        index2 = 1 + rand() % (length - 2);
    } while (index1 == index2);

    // Zamiana miejscami dwóch wybranych indeksów
    int temp = destination[index1];
    destination[index1] = destination[index2];
    destination[index2] = temp;
}

int meta_local_search_tsp(int path[N + 1], int fuel) {
    int current_path[N + 1]; 
    int alternate_path[N + 1];
    copy_array(path, current_path, N + 1);
    int length = calculate_path_length(current_path, gas_stations, fuel);

    for (int i = 0; i < IT; i++) {
        change_path(current_path, alternate_path, N + 1);
        //print_array(alternate_path, N + 1);
        if (check_if_route_ok(alternate_path, gas_stations, fuel)) {
            int alternate_length = calculate_path_length(alternate_path,gas_stations, fuel);
            //printf(" : %d\n", alternate_length);
            if (alternate_length < length) {
                length = alternate_length;
                copy_array(alternate_path, current_path, N + 1);
            }
        }
    }


    copy_array(current_path, path, N+1);

    if (length == INT_MAX)
        return 0;

    return length;
}

int main() {
        
    srand(time(NULL)); 

    int graph[N][N] = {
        {0, 10, 15, 20, 25},
        {10, 0, 35, 25, 30},
        {15, 35, 0, 30, 20},
        {20, 25, 30, 0, 10},
        {25, 30, 20, 10, 0}
    };


    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            tsp[i][j] = graph[i][j];
        }
    }

    for (int i = 0; i < N; i++) {
        gas_stations[i] = 0;
    }

    gas_stations[3] = 50;
    gas_stations[2] = 20;

    int path[N + 1]; // +1 na powrót do miasta początkowego
    int r_greedy_path[N + 1];

    int path_length = greedy_tsp(path, start_fuel);
    printf("Najkrótsza ścieżka GREEDY to: ");
    for (int i = 0; i < N + 1; i++) {
        printf("%d -> ", path[i]);
    }
    printf("\nDługość: %d\n", path_length);

    path_length = random_greedy_tsp(path, start_fuel);
    printf("Najkrótsza ścieżka GREEDY MIXED to: ");
    for (int i = 0; i < N + 1; i++) {
        printf("%d -> ", path[i]);
        r_greedy_path[i] = path[i];
    }
    printf("\nDługość: %d\n", path_length);

    path_length = bruteforce_tsp(path, start_fuel);
    printf("Najkrótsza ścieżka BRUTEFORCE to: ");
    for (int i = 0; i < N + 1; i++) {
        printf("%d -> ", path[i]);
    }
    printf("\nDługość: %d\n", path_length);

    path_length = meta_local_search_tsp(r_greedy_path, start_fuel);
    printf("Najkrótsza ścieżka META to: ");
    for (int i = 0; i < N + 1; i++) {
        printf("%d -> ", r_greedy_path[i]);
    }
    printf("\nDługość: %d\n", path_length);


    return 0;
}
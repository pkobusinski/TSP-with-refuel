#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include "cJSON.h"


int N, IT;
int** tsp;
int* gas_stations;
int start_fuel;
cJSON* root; // Assuming you have a global cJSON root variable

void initialize() {
    // Read data from a JSON file
    const char* jsonFilename = "data.json";
    FILE* jsonFile;
    errno_t err;

    if ((err = fopen_s(&jsonFile, jsonFilename, "r")) != 0) {
        printf("Failed to open JSON file\n");
        exit(1);
    }

    char buffer[1024];
    size_t length;
    length = fread(buffer, 1, sizeof(buffer), jsonFile);

    fclose(jsonFile);

    root = cJSON_Parse(buffer);
    if (!root) {
        printf("Error parsing JSON.\n");
        exit(1);
    }

    // Read N, IT, and start_fuel from the JSON file
    N = cJSON_GetObjectItem(root, "N")->valueint;
    IT = cJSON_GetObjectItem(root, "IT")->valueint;
    start_fuel = cJSON_GetObjectItem(root, "start_fuel")->valueint;

    // Dynamic memory allocation for tsp and gas_stations
    tsp = (int**)malloc(N * sizeof(int*));
    gas_stations = (int*)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) {
        tsp[i] = (int*)malloc(N * sizeof(int));
    }

    cJSON* tspArray = cJSON_GetObjectItem(root, "tsp");
    cJSON* gasStationsArray = cJSON_GetObjectItem(root, "gas_stations");

    for (int i = 0; i < N; i++) {
        cJSON* row = cJSON_GetArrayItem(tspArray, i); // Pobierz i-ty wiersz z tablicy tsp
        for (int j = 0; j < N; j++) {
            tsp[i][j] = cJSON_GetArrayItem(row, j)->valueint; // Pobierz j-tą wartość z i-tego wiersza
        }
        gas_stations[i] = cJSON_GetArrayItem(gasStationsArray, i)->valueint;
    }
}

void cleanup() {
    // Free allocated memory
    for (int i = 0; i < N; i++) {
        free(tsp[i]);
    }
    free(tsp);
    free(gas_stations);
}

void shuffle(int* array, int size, int keep_from_start, int keep_from_end) {
    if (keep_from_start + keep_from_end >= size) {
        return;
    }

    for (int i = keep_from_start; i < size - keep_from_end - 1; i++) {
        int j = keep_from_start + rand() % (size - keep_from_end - keep_from_start - 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

void copy_array(int* source, int* destination, int length) {
    for (int i = 0; i < length; i++) {
        destination[i] = source[i];
    }
}

int can_reach_gas_station(int current_fuel, int distance, int station_fuel) {
    return current_fuel >= distance ;
}

int greedy_tsp(int* path, int fuel) {
    int* unvisited_cities = (int*)malloc(N * sizeof(int));
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
        if (unvisited_cities[i] == 1) {
            free(unvisited_cities); // Zwolnij dynamicznie alokowaną tablicę
            return 0;
        }
    }

    free(unvisited_cities);
    return path_length;
}

void fill_array_with_indices(int array[], int length) {

    for (int i = 1; i < length - 1; i++) {
        array[i] = i;
    }

    array[0] = 0;
    array[length - 1] = 0;
}

int random_greedy_tsp(int* path, int fuel) {
    int* unvisited_cities = (int*)malloc(N * sizeof(int));
    int* error_path = (int*)malloc((N + 1) * sizeof(int));
    fill_array_with_indices(error_path, N + 1);
    shuffle(error_path, N + 1, 1, 1);
    int current_city = 0;
    int current_fuel = fuel;
    int path_length = 0;

    for (int i = 0; i < N; i++) {
        unvisited_cities[i] = 1;
    }

    unvisited_cities[current_city] = 0;
    path[0] = current_city;

    for (int i = 1; i < N; i++) {
        int* available_cities = (int*)malloc(N * sizeof(int)); // Dynamicznie alokowana tablica
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

        shuffle(available_cities, available_count, 0, 0);

        int next_city = available_cities[0];
        path[i] = next_city;
        unvisited_cities[next_city] = 0;
        path_length += tsp[current_city][next_city];
        current_fuel -= tsp[current_city][next_city];
        current_fuel += gas_stations[next_city];
        current_city = next_city;

        free(available_cities); // Zwolnienie dynamicznie alokowanej tablicy
    }

    path[N] = 0;
    path_length += tsp[current_city][0];

    if (!can_reach_gas_station(current_fuel, tsp[current_city][0], gas_stations[0])) {
        copy_array(error_path, path, N + 1);
        free(unvisited_cities);
        free(error_path);
        return 0;
    }

    for (int i = 0; i < N; i++) {
        if (unvisited_cities[i] == 1 || current_fuel < 0) {
            copy_array(error_path, path, N + 1);
            free(unvisited_cities);
            free(error_path);
            return 0;
        }
    }

    free(unvisited_cities);
    free(error_path);
    return path_length;
}

void generatePermutations(int step, int length, int* path, int* unvisited, int* min_path_length, int* best_path, int* fuel, int** tsp, int* gas_stations) {
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
        if (unvisited[i] && can_reach_gas_station(*fuel, tsp[path[step - 1]][i], gas_stations[i])) {
            path[step] = i;
            unvisited[i] = 0;
            int prev_fuel = *fuel; // Zapamiętaj ilość paliwa przed przejazdem
            *fuel -= tsp[path[step - 1]][i]; // Aktualizacja ilości paliwa
            *fuel += gas_stations[i];

            generatePermutations(step + 1, length + tsp[path[step - 1]][i], path, unvisited, min_path_length, best_path, fuel, tsp, gas_stations);

            unvisited[i] = 1;
            *fuel = prev_fuel; // Przywróć ilość paliwa po powrocie
        }
    }
}

int bruteforce_tsp(int* path, int fuel) {
    int* local_path = (int*)malloc(N * sizeof(int));
    int* unvisited = (int*)malloc(N * sizeof(int));
    int* best_path = (int*)malloc((N + 1) * sizeof(int));
    int min_path_length = INT_MAX;
    int current_fuel = fuel;

    for (int i = 0; i < N; i++) {
        unvisited[i] = 1;
    }

    local_path[0] = 0;
    unvisited[0] = 0;

    generatePermutations(1, 0, local_path, unvisited, &min_path_length, best_path, &current_fuel, tsp, gas_stations);

    for (int i = 0; i < N + 1; i++) {
        path[i] = best_path[i];
    }

    for (int i = 1; i < N + 1; i++) {
        if (path[i] == path[i - 1]) {
            free(local_path);
            free(unvisited);
            free(best_path);
            return 0;
        }
    }

    free(local_path);
    free(unvisited);
    free(best_path);
    return min_path_length;
}

bool check_if_route_ok(int* path,int* gas_stations, int fuel) {
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

int calculate_path_length(int* path,int* gas_stations, int fuel) {
    
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

int meta_local_search_tsp(int* path, int fuel) {
    int* current_path = (int*)malloc((N + 1) * sizeof(int));
    int* alternate_path = (int*)malloc((N + 1) * sizeof(int));
    copy_array(path, current_path, N + 1);
    int length = calculate_path_length(current_path, gas_stations, fuel);

    for (int i = 0; i < IT; i++) {
        change_path(current_path, alternate_path, N + 1);
        //print_array(alternate_path, N + 1);
        if (check_if_route_ok(alternate_path, gas_stations, fuel)) {
            int alternate_length = calculate_path_length(alternate_path, gas_stations, fuel);
            //printf(" : %d\n", alternate_length);
            if (alternate_length < length) {
                length = alternate_length;
                copy_array(alternate_path, current_path, N + 1);
            }
        }
    }

    copy_array(current_path, path, N + 1);

    if (length == INT_MAX) {
        free(alternate_path);
        free(current_path);
        return 0;
    }

    free(alternate_path);
    free(current_path);
    return length;
}

void printData(int** tsp, int* gas_stations, int N, int IT, int start_fuel) {
    printf("N = %d\n", N);
    printf("IT = %d\n", IT);
    printf("start_fuel = %d\n", start_fuel);

    printf("tsp = {\n");
    for (int i = 0; i < N; i++) {
        printf("    {");
        for (int j = 0; j < N; j++) {
            printf("%d", tsp[i][j]);
            if (j < N - 1) {
                printf(", ");
            }
        }
        printf("}");
        if (i < N - 1) {
            printf(",");
        }
        printf("\n");
    }
    printf("}\n");

    printf("gas_stations = {");
    for (int i = 0; i < N; i++) {
        printf("%d", gas_stations[i]);
        if (i < N - 1) {
            printf(", ");
        }
    }
    printf("}\n");
}

int main() {
    srand(time(NULL));

    initialize(); // Initialize the program with dynamic memory allocation

    int* path = (int*)malloc((N + 1) * sizeof(int)); // +1 for the return to the starting city
    int* r_greedy_path = (int*)malloc((N + 1) * sizeof(int));
    //printData(tsp, gas_stations, N, IT, start_fuel);
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

    // Clean up dynamically allocated memory
    free(path);
    free(r_greedy_path);
    cleanup(); // Free memory allocated in the initialize function

    return 0;
}
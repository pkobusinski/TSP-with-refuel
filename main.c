#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include "zen_timer.h"

#include "cJSON.h"
#include "generator.h"


int N, IT;
int** tsp;
int* gas_stations;
int start_fuel;
cJSON* root; 


void initialize() {
    
    const char* jsonFilename = "data.json";
    FILE* jsonFile;
    errno_t err;

    if ((err = fopen_s(&jsonFile, jsonFilename, "r")) != 0) {
        printf("Failed to open JSON file\n");
        exit(1);
    }

    fseek(jsonFile, 0, SEEK_END);
    long file_size = ftell(jsonFile);
    rewind(jsonFile);

    char* buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        printf("Failed to allocate memory for JSON buffer\n");
        exit(1);
    }

    size_t length = fread(buffer, 1, file_size, jsonFile);
    buffer[length] = '\0';

    fclose(jsonFile);

    root = cJSON_Parse(buffer);
    if (!root) {
        printf("Error parsing JSON.\n");
        exit(1);
    }

    N = cJSON_GetObjectItem(root, "N")->valueint;
    IT = cJSON_GetObjectItem(root, "IT")->valueint;
    start_fuel = cJSON_GetObjectItem(root, "start_fuel")->valueint;

    tsp = (int**)malloc(N * sizeof(int*));
    gas_stations = (int*)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) {
        tsp[i] = (int*)malloc(N * sizeof(int));
    }

    cJSON* tspArray = cJSON_GetObjectItem(root, "tsp");
    cJSON* gasStationsArray = cJSON_GetObjectItem(root, "gas_stations");

    for (int i = 0; i < N; i++) {
        cJSON* row = cJSON_GetArrayItem(tspArray, i); 
        for (int j = 0; j < N; j++) {
            tsp[i][j] = cJSON_GetArrayItem(row, j)->valueint; 
        }
        gas_stations[i] = cJSON_GetArrayItem(gasStationsArray, i)->valueint;
    }

    free(buffer); 
}

void cleanup() {
    for (int i = 0; i < N; i++) {
        free(tsp[i]);
    }
    free(tsp);
    free(gas_stations);
}

void print_data(int** tsp, int* gas_stations, int N, int IT, int start_fuel) {
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

void shuffle(int* array, int size, int keep_from_start, int keep_from_end) {
    if (keep_from_start + keep_from_end >= size) 
        return;
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

void fill_array_with_indices(int array[], int length) {

    for (int i = 1; i < length - 1; i++) {
        array[i] = i;
    }

    array[0] = 0;
    array[length - 1] = 0;
}

bool check_if_route_ok(int* path, int* gas_stations, int fuel) {
    int current_fuel = fuel;

    for (int i = 0; i < N; i++) {
        if (current_fuel < tsp[path[i]][path[i + 1]]) {
            return false;
        }
        current_fuel -= tsp[path[i]][path[i + 1]];
        current_fuel += gas_stations[path[i + 1]];
    }
    if (current_fuel < tsp[path[N]][path[0]]) {
        return false;
    }
    return true;
}

int calculate_path_length(int* path, int* gas_stations, int fuel) {
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
    for (int i = 0; i < length; i++) {
        destination[i] = source[i];
    }
    int index1, index2;
    do {
        index1 = 1 + rand() % (length - 2);
        index2 = 1 + rand() % (length - 2);
    } while (index1 == index2);
    int temp = destination[index1];
    destination[index1] = destination[index2];
    destination[index2] = temp;
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

int random_greedy_tsp(int* path, int fuel) {
    int* unvisited_cities = (int*)malloc(N * sizeof(int));
    int* error_path = (int*)malloc((N + 1) * sizeof(int));
    int* test_path = (int*)malloc((N + 1) * sizeof(int));

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
        int* available_cities = (int*)malloc(N * sizeof(int)); 
        int available_count = 0;

        for (int j = 0; j < N; j++) {
            if (unvisited_cities[j] && can_reach_gas_station(current_fuel, tsp[current_city][j], gas_stations[j])) {
                available_cities[available_count] = j;
                available_count++;
            }
        }

        if (available_count == 0) {
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

        free(available_cities); 
    }

    path[N] = 0;
    path_length += tsp[current_city][0];

    //copy_array(path, test_path, N+1);

    //for (int i = 0; i < N+1; i++)
    //{
    //    printf("%d,",path[i]);
    //}
    //printf("\n");

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
    //copy_array(test_path, path, N + 1);
   // printf("\n");
    free(unvisited_cities);
    free(error_path);
    return path_length;
}

void generatePermutations(int step, int length, int* path, int* unvisited, int* min_path_length, int* best_path, int* fuel, int** tsp, int* gas_stations) {
    if (step == N) {
        length += tsp[path[N - 1]][0];
        if (length < *min_path_length) {
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
            int prev_fuel = *fuel;
            *fuel -= tsp[path[step - 1]][i]; 
            *fuel += gas_stations[i];
            generatePermutations(step + 1, length + tsp[path[step - 1]][i], path, unvisited, min_path_length, best_path, fuel, tsp, gas_stations);
            unvisited[i] = 1;
            *fuel = prev_fuel; 
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

int meta_local_search_tsp(int* path, int fuel) {
    int* current_path = (int*)malloc((N + 1) * sizeof(int));
    int* alternate_path = (int*)malloc((N + 1) * sizeof(int));
    copy_array(path, current_path, N + 1);
    int length = calculate_path_length(current_path, gas_stations, fuel);

    for (int i = 0; i < IT; i++) {
        change_path(current_path, alternate_path, N + 1);
        if (check_if_route_ok(alternate_path, gas_stations, fuel)) {
            int alternate_length = calculate_path_length(alternate_path, gas_stations, fuel);
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

void saveResultToFile(const char* algorithm, int* path, int path_length, int N, int64_t time) {
    cJSON* results = cJSON_CreateObject();
    cJSON_AddStringToObject(results, "algorithm", algorithm);
    cJSON* pathArray = cJSON_CreateArray();
    for (int i = 0; i < N; i++) {
        cJSON_AddItemToArray(pathArray, cJSON_CreateNumber(path[i]));
    }
    cJSON_AddItemToObject(results, "path", pathArray);
    cJSON_AddNumberToObject(results, "path_length", path_length);
    cJSON_AddNumberToObject(results, "N", N-1);   
    
 
    cJSON_AddNumberToObject(results, "time", time);
   
    char* json_str = cJSON_Print(results);

    FILE* file;
    if (fopen_s(&file, "results.json", "a") == 0) { 
        fputs(json_str, file);
        fputs("\n", file); 
        fclose(file);
        printf("Zapisano wynik dla algorytmu %s.\n", algorithm);
    }
    else {
        fprintf(stderr, "Błąd podczas zapisywania pliku results.json.\n");
    }

    cJSON_Delete(results); 
    free(json_str);
}


void solve_and_save_results() {

    FILE* clear_file;
    if (fopen_s(&clear_file, "results.json", "w") == 0) {
        printf("Plik results.json został wygenerowany.\n");
        fclose(clear_file);
    }
    else {
        fprintf(stderr, "Błąd podczas czyszczenia pliku results.json.\n");
    }

    int* path = (int*)malloc((N + 1) * sizeof(int)); 
    int* r_greedy_path = (int*)malloc((N + 1) * sizeof(int));
       
    zen_timer_t timer = ZenTimer_Start();
    int path_length = greedy_tsp(path, start_fuel);
    int64_t time = ZenTimer_End(&timer);
    saveResultToFile("GreedyTSP", path, path_length, N+1, time);

    zen_timer_t timer_2 = ZenTimer_Start();
    path_length = random_greedy_tsp(path, start_fuel);
    int64_t time_2 = ZenTimer_End(&timer_2);
    for (int i = 0; i < N + 1; i++) {
        r_greedy_path[i] = path[i];
    }

    saveResultToFile("RandomGreedyTSP", path, path_length, N+1, time_2);

    zen_timer_t timer_3 = ZenTimer_Start();
    path_length = meta_local_search_tsp(r_greedy_path, start_fuel);
    int64_t time_3 = ZenTimer_End(&timer_3);
    saveResultToFile("MetaTSP", r_greedy_path, path_length, N+1, time_3);

    zen_timer_t timer_4 = ZenTimer_Start();
    path_length = bruteforce_tsp(path, start_fuel);
    int64_t time_4 = ZenTimer_End(&timer_4);
    saveResultToFile("BruteForceTSP", path, path_length, N + 1, time_4);

    free(path);
    free(r_greedy_path);
}

int main() {
    srand(time(NULL));
    generator();
    initialize(); 
    //print_data(tsp, gas_stations, N, IT, start_fuel);
    solve_and_save_results();
    cleanup(); 
    return 0;
}
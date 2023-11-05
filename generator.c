#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cJSON.h"

int generator() {
    srand(time(NULL)); 

    int N = rand() % 9 + 5;     // losowa liczba miast od 5 do 14
    int min_distance = INT_MAX;

    cJSON* root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "N", N);
    cJSON_AddNumberToObject(root, "IT", 100);

    
    cJSON* tsp = cJSON_AddArrayToObject(root, "tsp");
    for (int i = 0; i < N; i++) {
        cJSON* row = cJSON_CreateArray();
        cJSON_AddItemToArray(tsp, row);
        for (int j = 0; j < N; j++) {
            if (i == j) {
                cJSON_AddItemToArray(row, cJSON_CreateNumber(0));
            }
            else {
                int distance = rand() % 351 + 50; // Losowa odległość od 50 do 400
                if (distance < min_distance)
                    min_distance = distance;
                cJSON_AddItemToArray(row, cJSON_CreateNumber(distance));
            }
        }
    }

    int fuel_for_one_trip = min_distance * 2;
    int start_fuel = fuel_for_one_trip + rand() % (250 - fuel_for_one_trip + 1); // Losowa ilość paliwa od minimalnej do 250

    cJSON_AddNumberToObject(root, "start_fuel", start_fuel);

    cJSON* gas_stations = cJSON_AddArrayToObject(root, "gas_stations");
    for (int i = 0; i < N; i++) {
        int gas = start_fuel + rand() % (251 - start_fuel); // Losowa ilość paliwa większa lub równa początkowej ilości paliwa
        cJSON_AddItemToArray(gas_stations, cJSON_CreateNumber(gas));
    }

    char* json_str = cJSON_Print(root);

    FILE* file;
    if (fopen_s(&file, "data.json", "w") == 0) {
        fputs(json_str, file);
        fclose(file);
        printf("Plik data.json został wygenerowany.\n");
    }
    else {
        fprintf(stderr, "Błąd podczas tworzenia pliku data.json.\n");
    }

    cJSON_Delete(root); 
    free(json_str);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cJSON.h"

int generator( int number) {
   
    int N = number;
    int min_distance = INT_MAX;
    int max_distance = INT_MIN;
    int bigger_distance = INT_MIN;

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

    //for (int i = 0; i < N; i++) {                             // rozwiązanie optymalne zakodowane w generowany graf
    //    cJSON* row = cJSON_CreateArray();
    //    cJSON_AddItemToArray(tsp, row);
    //    for (int j = 0; j < N; j++) {
    //        if (i == j) {
    //            cJSON_AddItemToArray(row, cJSON_CreateNumber(0));
    //        }
    //        else if(j == i+1){
    //            int optimal_distance = rand() % 101 + 50;
    //            if (optimal_distance > max_distance)
    //                max_distance = optimal_distance;
    //            cJSON_AddItemToArray(row, cJSON_CreateNumber(optimal_distance));
    //        }
    //        else if (j > i+1 ){
    //            int distance = rand() % 150 + max_distance; // Losowa odległość od najlepszej optymalnej długości do 
    //            if (distance > bigger_distance)
    //                bigger_distance = distance;
    //            cJSON_AddItemToArray(row, cJSON_CreateNumber(distance));
    //        }
    //        else {
    //            int distance = rand() % 400 + bigger_distance; // Losowa odległość od najlepszej optymalnej długości do 
    //            if (distance < min_distance)
    //                min_distance = distance;
    //            cJSON_AddItemToArray(row, cJSON_CreateNumber(distance));
    //        }
    //    }
    //}

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
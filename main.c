#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <pthread.h>
int stations_start[5] = {500, 0, 0, 0, 0};
int stations_final[5] = {0, 50, 100, 250, 100};  
pthread_mutex_t stations_mutex[5];
pthread_mutex_t stdout_mutex;

typedef struct {
    int id;
    int capacity;
    int current;
} Train;

static void passenger_offload() {
    sleep(1);
}

static void station_pickup(Train *train) {
    pthread_mutex_lock(&stations_mutex[0]);
    int space = train->capacity - train->current;
    int to_load = (stations_start[0] < space) ? stations_start[0] : space;
    pthread_mutex_lock(&stdout_mutex);
    printf("Train %d ENTERS Station 0\n", train->id);
    printf("Station 0 has %d passengers left to %s\n", stations_start[0], (stations_start[0] > 0) ? "pick up." : "arrive.");
    printf("Train %d is at Station 0 and has %d %d passengers\n", train->id, train->current, train->capacity);
    pthread_mutex_unlock(&stdout_mutex);
    if (to_load > 0) {
        pthread_mutex_lock(&stdout_mutex);
        printf("Loading passengers...\n");
        pthread_mutex_unlock(&stdout_mutex);
        passenger_offload();
        train->current += to_load;
        stations_start[0] -= to_load;
    } else {
        pthread_mutex_lock(&stdout_mutex);
        printf("<Nothing more to do>...\n");
        pthread_mutex_unlock(&stdout_mutex);
    }
    pthread_mutex_lock(&stdout_mutex);
    printf(" Train %d is at Station 0 and has %d/%d passengers\n", train->id, train->current, train->capacity);
    printf(" Station 0 has %d passengers left to %s\n",stations_start[0], (stations_start[0] > 0) ? "pick up" : "arrive");
    printf(" Train %d LEAVES Station 0\n", train->id);
    pthread_mutex_unlock(&stdout_mutex);
    pthread_mutex_unlock(&stations_mutex[0]);
}

static void arrival_stations(Train *train, int station) {
    pthread_mutex_lock(&stations_mutex[station]);
    int to_drop = (to_drop > stations_final[station]) ? train->current : stations_final[station];
    pthread_mutex_lock(&stdout_mutex);
    printf("Train %d ENTERS Station %d\n", train->id, station);
    printf("Station %d has %d passengers left to arrive\n", station, stations_final[station]);
    printf("Train %d is at Station %d and has %d %d passengers\n", train->id, station, train->current, train->capacity);
    pthread_mutex_unlock(&stdout_mutex);
    if (to_drop > 0) {
        pthread_mutex_lock(&stdout_mutex);
        printf("Unloading passengers...\n");
        pthread_mutex_unlock(&stdout_mutex);
        passenger_offload();
        train->current -= to_drop;
        stations_final[station] -= to_drop;
    } else {
        pthread_mutex_lock(&stdout_mutex);
        printf("<Nothing more to do>...\n");
        pthread_mutex_unlock(&stdout_mutex);
    }
    pthread_mutex_lock(&stdout_mutex);
    printf("Train %d is at Station %d and has %d %d passengers\n",
           train->id, station, train->current, train->capacity);
    printf("Station %d has %d passengers left to arrive\n", station, stations_final[station]);
    printf("Train %d leaves Station %d\n", train->id, station);
    pthread_mutex_unlock(&stdout_mutex);
 
    pthread_mutex_unlock(&stations_final[station]);
}
void *trains_thread(void *arg) {
    Train *train = (Train *)arg;
 
    while (true) {
        visit_station0(train);
        if (train->current == 0) {
            break;
        }   
        int last_station_visited = 0;  
        for (int num_of_station = 1; num_of_station <= 4; num_of_station++) {
            passenger_offload();
            arrival_stations(train, num_of_station);
            last_station_visited = num_of_station;
            if (train->current == 0)      
                break;
        }
        for (int num = last_station_visited - 1; num >= 1; num--) {
            passenger_offload();               
            visit_station(train, num);
        }
       passenger_offload(); 
    }
}

int main(int argc, char *argv[]) {
    for (int station = 0; station < 5; station++)
        pthread_mutex_init(&stations_mutex[station], NULL);
        pthread_mutex_init(&stdout_mutex, NULL);
        Train trains[2] = {
            { .id = 0, .capacity = 100, .current = 0 },
            { .id = 1, .capacity = 50,  .current = 0 }
    };
    pthread_t tid[2];
    pthread_create(&tid[0], NULL, trains_thread, &trains[0]);
    pthread_create(&tid[1], NULL, trains_thread, &trains[1]);
 
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
 
    for (int station = 0; station < 5; station++)
        pthread_mutex_destroy(&stations_mutex[station]);
    pthread_mutex_destroy(&stdout_mutex);
 
    return 0;
}
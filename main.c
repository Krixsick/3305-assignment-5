#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <pthread.h>
//Defining variables
int stations_start[5] = {500, 0, 0, 0, 0};
int stations_final[5] = {0, 50, 100, 250, 100};  
pthread_mutex_t stations_mutex[5];
pthread_mutex_t stdout_mutex;
//Creating our struct Train
typedef struct {
    int id;
    int capacity;
    int current;
} Train;
//Sleeping function to simulate time between loading and unpacking
static void passenger_offload() {
    sleep(1);
}

static void station_pickup(Train *train) {
    //locking the station
    pthread_mutex_lock(&stations_mutex[0]);
    //calculating the loads
    int space = train->capacity - train->current;
    int to_load = (stations_start[0] < space) ? stations_start[0] : space;
    pthread_mutex_lock(&stdout_mutex);
    //printing out the which train and how many passengers left
    printf("Train %d ENTERS Station 0\n", train->id);
    printf("Station 0 has %d passengers left to %s\n", stations_start[0], (stations_start[0] > 0) ? "pick up." : "arrive.");
    printf("Train %d is at Station 0 and has %d/%d passengers\n", train->id, train->current, train->capacity);
    //unlocking
    pthread_mutex_unlock(&stdout_mutex);
    //checking if the to_load is > 0 bec then we have people to load
    if (to_load > 0) {
        pthread_mutex_lock(&stdout_mutex);
        printf("Loading passengers...\n");
        pthread_mutex_unlock(&stdout_mutex);
        //calling the sleep function
        passenger_offload();
        train->current += to_load;
        stations_start[0] -= to_load;
    } else {
        //we run this code if there's no people to load 
        pthread_mutex_lock(&stdout_mutex);
        printf("<Nothing more to do>...\n");
        pthread_mutex_unlock(&stdout_mutex);
    }
    //locks
    pthread_mutex_lock(&stdout_mutex);
    //prints out the train and how many passengers it has and which train leaves at a station (0) 
    printf(" Train %d is at Station 0 and has %d/%d passengers\n", train->id, train->current, train->capacity);
    printf(" Station 0 has %d passengers left to %s\n",stations_start[0], (stations_start[0] > 0) ? "pick up" : "arrive");
    printf(" Train %d LEAVES Station 0\n", train->id);
    pthread_mutex_unlock(&stdout_mutex);
    pthread_mutex_unlock(&stations_mutex[0]);
}

static void arrival_stations(Train *train, int station) {
    //locks
    pthread_mutex_lock(&stations_mutex[station]);
    //calculating the drop off 
    int to_drop = (train->current > stations_final[station]) ? stations_final[station] : train->current;
    pthread_mutex_lock(&stdout_mutex);
    //printing out which train enters which station and how many passengers are left to arrive
    printf("Train %d ENTERS Station %d\n", train->id, station);
    printf("Station %d has %d passengers left to arrive\n", station, stations_final[station]);
    printf("Train %d is at Station %d and has %d/%d passengers\n", train->id, station, train->current, train->capacity);
    pthread_mutex_unlock(&stdout_mutex);
    //Checks if the people we have to drop is greater than 0
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
    //locks
    pthread_mutex_lock(&stdout_mutex);
    //prints out which station and which train has passengers and how many passengers are left to arrive
    printf("Train %d is at Station %d and has %d/%d passengers\n",
           train->id, station, train->current, train->capacity);
    printf("Station %d has %d passengers left to arrive\n", station, stations_final[station]);
    printf("Train %d leaves Station %d\n", train->id, station);
    pthread_mutex_unlock(&stdout_mutex);
    //unlocks at that station
    pthread_mutex_unlock(&stations_mutex[station]);
}
void *trains_thread(void *arg) {
    //thread initialization
    Train *train = (Train *)arg;
    
    while (true) {
        //visits station 0 to pick to load passengers
        station_pickup(train);
        if (train->current == 0) {
            break;
        }   
        //it starts heading down to track stations 1-4
        int last_station_visited = 0;  
        for (int num_of_station = 1; num_of_station <= 4; num_of_station++) {
            //sleeping function to simulate delay
            passenger_offload();
            arrival_stations(train, num_of_station);
            last_station_visited = num_of_station;
            if (train->current == 0)      
                break;
        }
        //once we hit the end (station 4) we go back to station 0
        for (int num = last_station_visited - 1; num >= 1; num--) {
            //sleeping function to simulate delay
            passenger_offload();               
            arrival_stations(train, num);
        }
       passenger_offload(); 
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    //initializing our locks
    for (int station = 0; station < 5; station++) { 
        pthread_mutex_init(&stations_mutex[station], NULL);
    };
    //preparing thread data
    pthread_mutex_init(&stdout_mutex, NULL);
    Train trains[2] = { { .id = 0, .capacity = 100, .current = 0 },
            { .id = 1, .capacity = 50,  .current = 0 }
    };
    //creating our threads
    pthread_t tid[2];
    pthread_create(&tid[0], NULL, trains_thread, &trains[0]);
    pthread_create(&tid[1], NULL, trains_thread, &trains[1]);
    //makes our main threads pause and wait
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    //creating our stations and once everything is done we destroy to free up our resources
    for (int station = 0; station < 5; station++) {
        pthread_mutex_destroy(&stations_mutex[station]);
    }
    pthread_mutex_destroy(&stdout_mutex);
 
    return 0;
}
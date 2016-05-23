
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
//#include <string.h>
//#include <stdbool.h>

#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 3
//#define FALSE 0
//#define TRUE !(FALSE)
#define SLEEP_TIME 1

//using namespace std;

//int resources = 0;
//int processes = 0;
int counter = 0;
int i, j, k;

pthread_mutex_t mutex;

void *printer(void *params);
int safety(int process);
int request_resources(int customer_num, int request[]);
int release_resources(int customer_num, int release[]);
    
//declare each variable based on the defined number of number of resources and processes declared.
int available[NUMBER_OF_RESOURCES];
int max[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int request[NUMBER_OF_RESOURCES];
int release[NUMBER_OF_RESOURCES];


int main(int argumentCount, char* argumentArray[]) {
    
    //allow input to be parsed into three arguments and store in the array.
    //printf("Argument count - %d\n", argumentCount);
    
    printf("Available\n");
    
    //split each token into each index of the argument array.
    for(i=1; i<argumentCount; i++) {
        printf("%d\t", atoi(argumentArray[i]));
    }
    
    printf("\n");
    printf("Allocation\t");
    printf("Max\t");
    printf("Need\n");
    
    //generate a random number between 0 and the number of types of resources.
    //at the beginning of the process these values will be assigned to the max matrix.
    //the allocation array is kept with all values at 0.
    for(i=0; i<NUMBER_OF_CUSTOMERS; i++) {
        printf("P%d\t", i);
        for(j=0; j<NUMBER_OF_RESOURCES; j++) {
            allocation[i][j] = 0;
            max[i][j] = rand() % (argumentCount+1);
            need[i][j] = max[i][j];
            printf("%d\t", allocation[i][j]);
            printf("%d\t", max[i][j]);
            printf("%d\t", need[i][j]);
        }
        
        printf("\n");
    }
    
    //create a thread process for each customer process.
    pthread_t threadIDS[NUMBER_OF_CUSTOMERS];
    pthread_attr_t attr;
    
    //initialize each processor thread.
    pthread_attr_init(&attr);
    
    //initialize each mutex lock.
    pthread_mutex_init(&mutex, NULL);
    
    //create each individual processor thread.
    for(i=0; i<NUMBER_OF_CUSTOMERS; i++) {
        pthread_create(&threadIDS[i], &attr, printer, NULL);
    }
    
    //wait for each processor thread for each loop iteration.
    for(i=0; i<NUMBER_OF_CUSTOMERS; i++) {
        pthread_join(threadIDS[i], NULL);
    }
    
    //end the thread here.
    pthread_mutex_destroy(&mutex);
}

void *printer(void *params) {
    pthread_mutex_lock(&mutex);
    
    //critical section - bankers algorithm code goes here.
    
    //loop through each of the five threads 20 times.
    for(i=0; i<20; i++) {
    
        counter++;
        
        //generate a random number between 0 and the number of types of resources.
        //number was inputted by the user and represents the upper bound.
        for(j=0; j<NUMBER_OF_RESOURCES; j++) {
            request[j] = rand() % (max[i][j]+1);
        }
        
      
        k = rand() % NUMBER_OF_CUSTOMERS;
        i = k;
        
        request_resources(i, request);
        
        //let the process halt for a second.
        sleep(SLEEP_TIME);
        
        for(j=0; j<NUMBER_OF_RESOURCES; j++) {
            release[j] = rand() % (max[i][j]+1);
        }
        
        //the process was able to successfully give the resources it needs.
        release_resources(i, release);
        
        pthread_mutex_unlock(&mutex);
        pthread_exit(0);
    }
}

//grants a request if it satisfies all safety preconditions. 
//returns a 0 successful, else returns unsuccessful.
int request_resources(int customer_num, int request[]) {
    
    printf("P%d is requesting: [%d, %d, %d]\n", customer_num, request[0], request[1], request[2]);
    
    for(i=0; i<NUMBER_OF_RESOURCES; i++) {
        //checks to see if the requested resources does not exceed the amount needed.
        if(request[i] <= need[i]) {
            //checks to see if the requested resources does not exceed the amount available.
            if(request[i] <= available[i]) {
                
                available[i] = available[i] - request[i];
                
                //checks to see if each customer process is in a safe state.
                int condition = safety(customer_num);
                
                //change the allocation to represent the amount requested.
                allocation[customer_num][i] = allocation[customer_num][i] + request[i];
                
                //the amount needed by the process is updated with the number of requested resources removed by the array.
                need[customer_num][i] = need[customer_num][i] - request[i];
                
                //continue if there is a safety sequence.
                if(condition==0) {
                    
                    printf("The request has been granted\n");
                    
                    printf("\n");
                    printf("Allocation\t");
                    printf("Max\t");
                    printf("Need\n");
                    
                    for(i=0; i<NUMBER_OF_CUSTOMERS; i++) {
                        printf("P%d\t", i);
                        for(j=0; j<NUMBER_OF_RESOURCES; j++) {
                            printf("%d\t", allocation[i][j]);
                            printf("%d\t", max[i][j]);
                            printf("%d\t", need[i][j]);
                        }
                        
                        printf("\n");
                    }
                
                    return 0; //successfully allocated a certain number of resources.
                
                }
                
                //perform the rollback if found that system is in an unsafe state.
                else {
                    available[i] = available[i] + request[i];
                
                    //change the allocation to represent the amount requested.
                    allocation[customer_num][i] = allocation[customer_num][i] - request[i];
                
                    //the amount needed by the process is updated with the number of requested resources removed by the array.
                    need[customer_num][i] = need[customer_num][i] + request[i];
                    
                    printf("The system is in an unsafe state.\n");
                    return -1;
                }
    
            }
            
            //make the procss wait if the number of resources requested for a type exceeds the amount available for the process.
            else if(request[i] > available[i]) {
                printf("The process must wait, since the resources are not available.\n");
                return -1;
            }
        }
        
        //if the amount of a type of resource requested exceeds the amount needed by the customer process, do not allow the resources to be allocated.
        else if(request[i] > need[i]) {
            printf("The process has exceeded its maximum claim.\n");
            return -1;
        }
        
    }
}

//determines whether each process has entered the safe state or not.
//returns 0 if it is in a successful/safe state, otherwise -1 to show that it is in an unsafe state.
int safety(int process) {
    
    int work[process];
    int finish[process];
    
    work[process] = available;
    finish[process] = -1;
    
    for(i=0; i<NUMBER_OF_CUSTOMERS; i++) {
        //loop through the number of types of each resources for each customer process.
        for(j=0; j<NUMBER_OF_RESOURCES; j++) {
        
            //checks if the customer process has been completed, and the amount of resources needed does not exceeded the work done.
            if(finish[i]==-1 && need[i][j] <= work[i]) {
                //the system is an unsafe state.
                return -1;
            }
        
            else {
            
            //if(i = NUMBER_OF_RESOURCES-1) {
                //system is in a safe state for all i.
            //    finish[i] = TRUE;
                //return a successful state.
            //}
            
                work[i] = work[i] + allocation[i][j];
                finish[i] = 0;
            }
        }
    }
    
    return 0;
}

//return the resources to the bank after the processes have been done using them.
//returns a 0 successful, else returns unsuccessful.
int release_resources(int customer_num, int release[]) {
    
    //return the types of resources back to the available vector.
    for(i=0; i<NUMBER_OF_RESOURCES; i++) {
        available[i] = available[i] + request[i];
        
        //the amount requested should be removed by what is allocated.
        allocation[customer_num][i] = allocation[customer_num][i] - request[i];
        
        //update what is needed by the request, and start the process over again.
        need[customer_num][i] = need[customer_num][i] + request[i];   
    }
    
    printf("\n");
    printf("Allocation\t");
    printf("Max\t");
    printf("Need\n");
    
    for(i=0; i<NUMBER_OF_CUSTOMERS; i++) {
        printf("P%d\t", i);
        for(j=0; j<NUMBER_OF_RESOURCES; j++) {
            printf("%d\t", allocation[i][j]);
            printf("%d\t", max[i][j]);
            printf("%d\t", need[i][j]);
        }
                        
        printf("\n");
    }
    
    //the resources have successfully been returned.
    return 0;
}


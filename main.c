/*
 *  Pthreads example (CS 425 Project 2)
 *  Copyright © 2017 Trevor D. Brown. All rights reserved.
 *  This program was written in fulfillment of requirements for Project 2, in CS 425-001 (Fall 2017).
*/

// Include statements
#include <stdio.h>              // Standard I/O library
#include <stdlib.h>             // Standard C library
#include <pthread.h>            // Pthreads library
#include <semaphore.h>          // Semaphore library
#include <unistd.h>             // Standard symbolic constants and types

// Preprocessor macros
#define BUFFER_SIZE 24         // Predefined buffer size
#define THREAD_COUNT 4         // Predefined thread count

// Global Variables
int n = 0, in = 0, out = 0, running_count = 0;    // n, in, out, and running (integers)
int buffer[BUFFER_SIZE];                    // buffer (integer array, size BUFFER_SIZE)
sem_t empty, full;                          // empty and full (semaphores)
pthread_mutex_t mutex;                      // mutex (pthread_mutex_t)

// Producer thread function
void *producer(void *param){

    // Declare for loop (from 0 to n)
    // This loop processes the producer actions.
    int i = 0;
    while(i <= n){

        // Wait for empty buffer.
        sem_wait(&empty);

        // Lock for buffer protection.
        pthread_mutex_lock(&mutex);

        // ------------------------ Start Critical Section ------------------------ //

        // Store value in buffer
        buffer[in] = i;

        // Set "in" flag
        in = (in + 1) % BUFFER_SIZE;

        // ------------------------ End Critical Section ------------------------ //

        // Release the buffer protection.
        pthread_mutex_unlock(&mutex);

        // Signal full buffer.
        sem_post(&full);

        // Increment i
        i++;
    }

    // Decrement running
    pthread_mutex_lock(&mutex);
    running_count--;
    pthread_mutex_unlock(&mutex);

    // Return thread.
    pthread_exit(0);
}

// Consumer thread function
void *consumer(void *param){

    // Declare for loop (from 0 to n)
    // This loop processes the consumer actions.
    int i = 0;
    while (i <= n){

        // Wait for buffer to fill.
        sem_wait(&full);

        // Lock for buffer protection.
//        sem_wait(&mutex);
        pthread_mutex_lock(&mutex);

        // ------------------------ Start Critical Section ------------------------ //

        // Print the result.
        printf("Thread %i says: %i \n", (int)pthread_self(), buffer[out]);

        // Set "out" flag
        out = (out + 1) % BUFFER_SIZE;

        // ------------------------ End Critical Section ------------------------ //

        // Release the buffer protection.
//        sem_post(&mutex);
        pthread_mutex_unlock(&mutex);

        // Signal empty buffer.
        sem_post(&empty);

        // Increment i
        i++;
    }

    // Decrement running counter
    pthread_mutex_lock(&mutex);
    running_count--;
    pthread_mutex_unlock(&mutex);

    // Return thread.
    pthread_exit(0);
}

// Main function
int main(int argc, const char *argv[]) {

    // Create pthread_t arrays for our producer and consumer threads
    pthread_t producer_threads[THREAD_COUNT]; // producer_threads (pthread_t array, size THREAD_COUNT)
    pthread_t consumer_threads[THREAD_COUNT]; // consumer_threads (pthread_t array, size THREAD_COUNT)

    // Declare for loop (from 0 to THREAD_COUNT)
    // This loop creates producer and consumer threads for our use.
    int i = 0;
    for (i = 0; i < THREAD_COUNT; i++){
        pthread_t producer_thread = NULL, consumer_thread = NULL;   // Temporary threads
        producer_threads[i] = producer_thread;                      // Add to producer threads
        consumer_threads[i] = consumer_thread;                      // Add to consumer threads
    }

    // If the argument count is not 2, report error and exit.
    if (argc != 2){
        fprintf(stderr, "Error: parameter mismatch. Program usage: ./<program name> <positive integer>\n");
        return -1;
    }

    // Convert the user input (string) to an integer using atoi function.
    n = atoi(argv[1]);

    // Print message to user.
    printf("\nInteger printing using POSIX Threads (pthreads)\n");
    printf("By Trevor D. Brown - CS 425\n");
    printf("Buffer size: %i\n", BUFFER_SIZE);
    printf("Thread count: %i\n\n", THREAD_COUNT);
    printf("Printing up to %i:\n", n);

    // Get default attributes for semaphores.
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // Initialize the semaphores

    /*
     NOTE:
     sem_init has been deprecated in macOS High Sierra (my development platform). Therefore, I opted to use sem_open, which is equivalent, and available, in all POSIX-compliant systems.

     */

    // Empty semaphore (set to BUFFER_SIZE)
    // sem_open((char *)&empty, O_CREAT, O_RDWR, BUFFER_SIZE);
    sem_init(&empty, 0, BUFFER_SIZE);
    // Full semaphore (set to 0)
    // sem_open((char *)&full, O_CREAT, O_RDWR, 0);
    sem_init(&full, 0, 0);
    // Mutex semaphore (Binary semaphore, set to 0)
    pthread_mutex_init(&mutex, 0);

    // Create the producer and consumer threads
    for (i = 0; i < THREAD_COUNT; i++){
        // Increment the running counter.
        pthread_mutex_lock(&mutex);
        running_count++;
        pthread_mutex_unlock(&mutex);
        // Create the producer thread
        pthread_create(&producer_threads[i], &attr, producer, (void *)argv[1]);

        // Increment the running counter.
        pthread_mutex_lock(&mutex);
        running_count++;
        pthread_mutex_unlock(&mutex);
        // Create the producer thread
        pthread_create(&consumer_threads[i], &attr, consumer, (void *)argv[1]);
    }

    // While threads are still running, sleep for a second and join threads, when possible.
    while (running_count > 0){
        sleep(1);
        // Join all producer and consumer threads
        for (i = 0; i < THREAD_COUNT; i++){
            pthread_join(producer_threads[i], NULL);
            pthread_join(consumer_threads[i], NULL);
        }
    }

    // Unlink and close the semaphore(s)

    /*
     NOTE:
     sem_destroy has been deprecated in macOS High Sierra (my development platform). Therefore, I opted to use sem_close, which is equivalent, and available, in all POSIX-compliant systems.

     */

    // Mutex semaphore
    pthread_mutex_destroy(&mutex);

    // Full semaphore
    sem_close(&full);
    
    // Empty semaphore
    sem_close(&empty);

    // Print completion message.
    printf("\nGoodbye! - Trevor D. Brown\n");

    // Exit program.
    return 0;
}

#include "PCalc_T.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <math.h>



PCalc_T::PCalc_T(unsigned int n_array, int numberThreads) : PCalc(n_array) {
    //Set number of threads
    numThreads = numberThreads;
}

void PCalc_T::doWork(int p) {
    //Take the prime p and starting at it's square mark those numbers as non-prime and increment for p
    for (int i=p*p; i<=array_size(); i += p) {
        at(i) = false;
    }
}

void PCalc_T::markNonPrimes() {

    //Vectors to keep track of threads and mutex
    std::vector<int> threadWork(numThreads);
    std::vector<std::thread> threadPool(numThreads);

    //Initialize vector storing work for threads to 0 for each thread meaning they are able to accept work
    for (int i=0; i<numThreads; i++) {
        threadWork[i] = 0;
    }


    //Preload the primelist before spinning off threads to minimize redoing work
    at(0) = false;
    at(1) = false;
    switch (numThreads)
    {
    case 1:
        threadWork[0] = 2;
    case 2:
        threadWork[0] = 2;
        threadWork[1] = 3;
        break;
    case 3:
        threadWork[0] = 2;
        threadWork[1] = 3;
        threadWork[2] = 5;
        break;
    default:
        threadWork[0] = 2;
        threadWork[1] = 3;
        threadWork[2] = 5;
        threadWork[3] = 7;
        break;
    }


    //Lambda for the Threads
    auto f = [&](int i) {
        //Prime to do work on
        int work = 0;

        //While program isn't completed
        while (threadWork[i] != -1) {
            //Check the threadWork vector to see if this thread is assigned work
            if (threadWork[i] != 0) {

                //Read the available prime into local variable and break on -1 (terminate signal)
                arrayWrite.lock();

                work = threadWork[i];
                if (work == -1 ) { break; }

                //Set the vector to available to accept new work
                threadWork[i] = 0;

                arrayWrite.unlock();

                //Do the actual work of marking non-primes
                doWork(work);
            }
            //Sleep before checking for new work
            //std::this_thread::sleep_for(std::chrono::nanoseconds(1));
            sched_yield();
        }
    };

    //Make the threads and add them to our threadpool
    for (int i=0; i<numThreads; i++) {
        threadPool[i] = std::thread(f, i);
    }

    //Main control loop
    for (double i = 11; i <= sqrt(array_size()); i++) {
        //If we have a prime number start the marking process
        if (at(i) == true) {
            bool hasWork = true;

            //While you have a prime to assign out to threads
            while (hasWork) {
                //Loop over threadpool looking for a waiting thread
                for (int t = 0; t < numThreads; t++) {
                    //Found waiting thread?
                    if (threadWork[t] == 0) {
                        //Lock the array for reading and pass the prime number to the thread
                        arrayWrite.lock();
                        threadWork[t] = i;
                        arrayWrite.unlock();
                        //We no longer have work so set the boolean to false and break so we don't try and give the work to other threads
                        hasWork = false;
                        break;
                    }
                }
            //Give threads tiny second to calculate stuff
            //std::this_thread::sleep_for(std::chrono::nanoseconds(1));
            }
        }
    }

    bool threadsDone = false;
    int numDone = 0;

    //Wait on threads to finish before returning
    while (threadsDone == false) {
        //Look at each entry in vector or work
        for (int i = 0; i < numThreads; i++) {
            //If the thread is done working
            if (threadWork[i] == 0) {
                arrayWrite.lock();
                threadWork[i] = -1;
                arrayWrite.unlock();
            }
            //If we have already told the thread to exit
            if (threadWork[i] == -1) {
                //Count number of exited threads
                numDone++;
            }
        }
        //If all threads have exited
        if (numDone == numThreads) {
            threadsDone = true;
            break;
        }
        //Reset number done
        else { numDone = 0; }
    }

    for (auto i = threadPool.begin(); i != threadPool.end(); ++i) {
        i->join();
    }

}
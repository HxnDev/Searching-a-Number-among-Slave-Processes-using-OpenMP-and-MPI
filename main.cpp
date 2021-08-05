//##########################################################################################################################
//               Hassan Shahzad
//               18i-0441
//               CS-D
//               FAST-NUCES ISB
//               chhxnshah@gmail.com
//#########################################################################################################################

//#################################################### Libraries #########################################################

#include <unistd.h>
#include <omp.h>
#include <iostream>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define max_size 100000             // Defining a really large number to be used during communication
using namespace std;

int main(int argc, char** argv)
{

//########################################## Variable Declaration #########################################################

    int rank;                       // Will be used to identify the processes
    int size = 100;                 // Total numbers to be searched from
    int* data;                      // The data that needs to be distributed
    int dist;                       // Store the amount of data to be distributed among each process
    int nprocs;                     // Number of processes -> Passed as an argument
    int processes;                  // Number of processes
    int* send;                      // The data to send
    int* recv_slave;                // Will store the data recieved from the master process
    int recv_master;                // Will store the data recieved from the master process
    int size_of_recv;               // Will store the size of the data recieved
    int count = 0;                  // Will be used in distribution of data
    int search;                     // The number that needs to be searched
    int found = 111;                // Error code if the value is found
    int abort = 000;                // Error code to abort the search
    int not_found = 666;            // Error code if the value is not found

//####################################################################### Use of MPI ######################################################################################
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  
    processes = nprocs;
    
//################################################################## Master Process #######################################################################################

    if (rank == 0)                                                          // Master process with rank = 0
    {   
        MPI_Status status; 
        cout<<"Enter the number you want to search = ";
        cin>> search;
        cout<<endl;
        data = new int[size];                                               // Creating a dynamic array to make the code generic
        
        int x = 1;
        for (int i=0; i<size; i++)                                          // Initializing the array
        {
            data[i] = x;
            x++;
        }

        if ((size % (processes-1)) == 0)                                    // If number of data can be equally distributed among the slaves
        {
            dist = size / (processes-1);
            send = new int[size];   
        }

        for (int i =1; i< processes; i++)                                   // Data is being distributed
        {
            for (int j=count; j<(count+dist); j++)
            {
                send[j] = data[j];
            }

//##################################################################### Implementation of OpenMP ############################################################################

            #pragma omp critical                                                                // Critical Section starts
		    {
                MPI_Send(send+count,dist, MPI_INT, i, 6, MPI_COMM_WORLD);                       // Sending the data to each of the slave processes
                count+=dist;
                MPI_Send(&search,1, MPI_INT, i, 6, MPI_COMM_WORLD);                             // Sending the value to be searched

            }            
        }

        MPI_Recv (&recv_master, 1, MPI_INT,MPI_ANY_SOURCE,6, MPI_COMM_WORLD, &status);          // Recieving data from master process

        if (recv_master == found)                                                               // If master recieves a "FOUND" code
        {
            cout<<"MASTER: The value was found by SLAVE "<<status.MPI_SOURCE<<endl<<endl;
            
            for (int i=1; i<processes; i++)
            {
                cout<<"MASTER: SENDING ABORT SIGNALS TO SLAVE "<<i<<endl;
                MPI_Send(&abort,1, MPI_INT, i, 6, MPI_COMM_WORLD);                              // Sending abort signal to each of the slave processes
            }
            
        }
        //sleep(5);                                                                             // This sleep will force the scheduler to shift to slave processes		
   	}

//####################################################################### Slave Processses ####################################################################################

    else
    {
        int search1,abort1;
        bool found1 = false;
    	//sleep(5);                                                                             // For synchronization
        #pragma omp parallel num_threads(1)                                                     // This will create slave processes (i.e. 1 thread per proccess)
        {
            recv_slave = new int[size];                                                         // Allocating memory to the recieving buffer
            MPI_Status status;                                                                  // To store status of the message recieved
            #pragma omp critical
            {
                //sleep(5);
                MPI_Recv (recv_slave, max_size, MPI_INT,0,6, MPI_COMM_WORLD, &status);          // Recieving data from master process
                MPI_Get_count(&status, MPI_INT,&size_of_recv);                                  // Calculating the size of the message recieved
                cout<<"SLAVE "<<rank<<": ";  
                
                    for (int i =0; i<size_of_recv; i++)
                    {
                        cout<<recv_slave[i]<<" ";                                               // Displaying the local data for each process
                    }
                    cout<<endl<<endl<<endl;

                #pragma omp_barrier                                                             // For Synchronization
                {
                    int idx;  
                    MPI_Recv (&search1, 1, MPI_INT,0,6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);     // Recieving the value to be searched from master process
                    
                    
                    for (int i = 0; i < size_of_recv; i++)                                      // Searching the Value
                    {
                        if (recv_slave[i] == search1)                                           // If value is found
                        {
                            idx =  i;
                            found1 = true;
                        }
                    }

                    if (found1 == true)
                    {
                        cout<<"SLAVE "<<rank <<": Value found at index "<<idx<<endl<<endl;
                        cout<<"SLAVE "<<rank<<": Sending FOUND signal to the master..."<<endl;
                        MPI_Send(&found,1, MPI_INT, 0, 6, MPI_COMM_WORLD);                     // Sending the signal to master process that data has been found
                    }
                    else
                    {
                        sleep(2);
                        cout<<"SLAVE "<<rank<<": VALUE NOT FOUND!"<<endl;                      // Displaying message that value is not found
                    }
                }   
            }
            MPI_Recv (&abort1, 1, MPI_INT,0,6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);             // Recieving the value to be searched from master process
            
            if (abort1 == abort)
            {
                cout<< "SLAVE "<<rank<<": ABORT SIGNAL RECIEVED!"<<endl;
            }
        }       
    }
    MPI_Finalize();
    return 0;
}


//################################################################### Implementation Ended ##################################################################################
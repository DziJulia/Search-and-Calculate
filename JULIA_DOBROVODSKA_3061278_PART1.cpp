// Distributed Systems - 2023
// Assignment 1 Part 1 - template

#include <iostream>
#include <fstream>
#include <mpi.h>
#include <string>

//Global variables
const static int arraySize = 400;
float gradesArray[arraySize];
int rank;
int world_size;

/**
 * should print out an array to console in a single line. It should accept
 * two parameters: a pointer to the array and the size of the array.
 */
void printArray(float *arr, int size) {
    for(int i = 0; i < size; i++) {
         std::cout << arr[i] << ", ";
    }
    std::cout << "\n";
}

/**
 * takes in a reference to an array and an array size, and returns
 * the sum of all the values in that array.
 * @return {Float}
 */
float sum(float *arr, int size) {
    float mySum = 0;
    for(int i = 0; i < size; i++) {
        mySum += arr[i];
    }
    return mySum;
}

/**
 * takes in a float value for the sum and an array size, and
 * returns the mean
 * @return {Float}
 */
float getAvg(float sum, int size) {
    float myAvg = 0;
    if (size > 0) {
        myAvg = sum / size;
    }
    return myAvg;
}

/**
 * takes in a reference to an array and an array size, and
 * returns the highest value in that array.
 * @return {Float}
 */
float getMax(float *arr, int size) {
    float myMax = arr[0];
    for(int i = 0; i < size; i++) {
        if(arr[i] > myMax){
            myMax = arr[i];
        }
    }
    return myMax;
}

/**
 * takes in a reference to an array and an array size,
 * and returns the lowest value in that array.
 * @param
 * @return {Float}
 */
float getMin(float *arr, int size) {
    float myMin = arr[0];
    for(int i = 0; i < size; i++) {
        if(arr[i] < myMin){
            myMin = arr[i];
        }
    }
    return myMin;
}


//You do not need to modify this method
void createData(std::string filename) {
    /*
    Populates an array with the data read from file.
    */
    std::ifstream file;
    file.open(filename, std::ios::in);
    if (!file) {
        std::cout << "Error reading file: " << filename << std::endl;
        std::cout << "Make sure file is saved in the same directory as your executable for this project." << std::endl;
    }
    else {
        std::cout << "Reading file: " << filename << std::endl;
        std::string line;
        //use a while loop with getLine() to read file line by line and store into array
        int i = 0;
        while (getline(file, line, ' '))
        {
            // add text from file into array
            gradesArray[i] = std::stof(line);
            // std::cout << std::to_string(gradesArray[i]) << std::endl; //debug
            i++;
        }
        file.close();
    }
}


//Main method - work to be done here
int main(int argc, char** argv) {

    // variables for the methods
    //float nodeSum = ()
    // initialise the MPI library
    MPI_Init(NULL, NULL);

    // Starting time
    //double start_time = MPI_Wtime();

    // determine the world size
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // determine our rank in the world
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // arrays that we need for communicating data
    int partitionSize = arraySize / world_size;
    float* partition = new float[partitionSize];
    float total_sum = 0;
    float overall_avg = 0;
    float max_grade = 0;
    float min_grade = 0;

    // check if we diving the work evenly if the modulus doesn't equal 0
    // meand we don't spread work evenly
    if (arraySize % world_size != 0) {
        if (rank == 0) {
            std::cerr << "Error: The data size -> " << arraySize 
                    << " cannot be divided evenly among the processes -> " << world_size 
                    << ". Please use a different data size or number of processes." << std::endl;
        }
        MPI_Finalize();
        return 0;
    }
    // Check if the number of processes is valid (at least 2)
    if (world_size <= 2) {
        // If not, print an error message and finalize MPI
        if (rank == 0) {
            std::cout << "Error: At least 3 processes are required to run this program." << std::endl;
        }
        MPI_Finalize();
        return 0;
    }

    /**
     * Node0 will read the provided moduleGrades.txt file and store the data into an
     * array by calling createData(). Node0 will then partition the data and give all
     * of the other nodes (including itself), a piece (or partition) of the data
    */
    if(rank == 0){
        createData("moduleGrades.txt");
       // printArray(gradesArray, arraySize);
    }

    MPI_Scatter(
        gradesArray,
        partitionSize,
        MPI_FLOAT,
        partition,
        partitionSize,
        MPI_FLOAT,
        0,
        MPI_COMM_WORLD
    );

    /**
     * Each node will take its given chunk and use the completed helper
     * methods to perform the following calculations:
     * - Calculate the sum of grades, stored in variable nodeSum
     * - Calculate the average grade, stored in variable nodeAvg
     * - Evaluate the highest grade, stored in variable nodeMax
     * - Evaluate the lowest grade, stored in variable nodeMin
    */
    float nodeSum = sum(partition,partitionSize) ;
    float nodeAvg = getAvg(nodeSum,partitionSize);
    float nodeMax = getMax(partition,partitionSize);
    float nodeMin = getMin(partition,partitionSize);

    /**
     * Each node will output its calculated values to console in the format:
     * Node [rank]: highest grade: [nodeMax], lowest grade: [nodeMin], average:
     * [nodeAvg] Format your output for decimal numbers such that they print
     * to 2 decimal points.
     */
    std::cout << std::fixed << std::setprecision(2)
    << "Node: " << rank
    << " Highest grade: " << nodeMax
    << ", Lowest grade: " << nodeMin
    << ", Average: " << nodeAvg
    << std::endl; 

    /**
     * Node 1 will collect each of the averages from all nodes and add these
     * together. The result subsequently placed into a new variable, called
     * total_sum. Calculate the total average and save in a variable named
     * overall_avg. Each node will then print its rank and the value of
     * overall_avg to console.
    */
    MPI_Reduce(
        &nodeAvg,
        &total_sum,
        1,
        MPI_FLOAT,
        MPI_SUM,
        1,
        MPI_COMM_WORLD
    );

    // Each node will then print its rank and the value of overall_avg to console.
    overall_avg = total_sum / world_size;
    std::cout << std::fixed << std::setprecision(2)
        << "Node: " << rank
        << " Over all average: " << overall_avg
        << std::endl;

    /**
     * Node 2 will collect each of the highest grades from all nodes and
     * save the overall highest value into a new variable, called max_grade.
     * Node 2 will also collect each of the lowest grades from all nodes and
     * save the lowest value into a new variable, called min_grade.
     */
    MPI_Reduce(
        &nodeMax,
        &max_grade,
        1,
        MPI_FLOAT,
        MPI_MAX,
        2,
        MPI_COMM_WORLD
    );

    MPI_Reduce(
        &nodeMin,
        &min_grade,
        1,
        MPI_FLOAT,
        MPI_MIN,
        2,
        MPI_COMM_WORLD
    );

    // Only node 2 should output these values to console.
    if(rank == 2){
        std::cout << std::fixed << std::setprecision(2)
        << "Node: " << rank
        << " Highest Grade: " << max_grade
        << ", Minimum Grade : " << min_grade
        << std::endl;
    }

    /**
     * Ensure all nodes have finished the previous tasks before moving
     * on to the next task, by invoking an MPI method that will force the
     * nodes to synchronise. Afterward, Node 1 should broadcast overall_avg
     * to all other nodes. Each node will now print its rank and overall_avg
     * once again to console.
    */
    MPI_Barrier(MPI_COMM_WORLD);

    // Node 1 broadcasts the overall average to all other nodes
    MPI_Bcast(
        &overall_avg,
        1,
        MPI_FLOAT,
        1,
        MPI_COMM_WORLD
    );

    // Each node will then print its rank and the value of overall_avg to console.
    std::cout << std::fixed << std::setprecision(2)
        << "Node: " << rank
        << " Printing again over all average: " << overall_avg
        << std::endl;

    // Finish time and print out the message 
    // double end_time = MPI_Wtime();

    // if (rank == 0) {
    //     printf("Time taken for %d nodes with %d grades: %lf seconds\n", world_size, arraySize, end_time - start_time);
    // }

    // finalise the MPI library
    MPI_Finalize();

    // clean up
    delete[] partition;

   // standard C/C++ thing to do at the end of main
   return 0;
}

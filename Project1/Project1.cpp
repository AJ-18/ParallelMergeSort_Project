// Project1.cpp: Arjun Suresh  - Parallel Merge Sort 
// Numbers, then uppercase, then lowercase

#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <semaphore>
#include <atomic>

using namespace std;

// Implements sorting rules
// Rules are: numbers first, then uppercase letters, then lowercase letters
bool compareMerge(char a, char b)
{
    // Check if either character is a digit
    bool aIsDigit = (a >= '0' && a <= '9');
    bool bIsDigit = (b >= '0' && b <= '9');

    // If both are digits, compare them normally
    if (aIsDigit && bIsDigit) 
    {
        return a < b;
    }

    // If only a is a digit, it should come first
    if (aIsDigit)
    {
        return true;
    }

    // If only b is a digit, it should come first
    if (bIsDigit)
    {
        return false;
    }

    // Check if characters are uppercase (A-Z)
    bool aIsUpper = (a >= 'A' && a <= 'Z');
    bool bIsUpper = (b >= 'A' && b <= 'Z');

    // If both are uppercase, compare them normally
    if (aIsUpper && bIsUpper) 
    {
        return a < b;
    }

    // If only a is uppercase, it should come first
    if (aIsUpper)
    {
        return true;
    }

    // If only b is uppercase, it should come first
    if (bIsUpper)
    {
        return false;
    }

    // If we reach here, both have to be lowercase
    return a < b;
}

// Timer class
struct ThreadTimer
{
    // Counter for threads
    static atomic<uint64_t> globalTimer;

    // Get the CURRENT time
    static uint64_t getTime()
    {
        return globalTimer.load();
    }

    // Increment the timer
    static void incremementTimer()
    {
        globalTimer++;
    }
};

// Static timer initialized
atomic<uint64_t> ThreadTimer::globalTimer(0);

void merge(vector<char>& arr, int left, int mid, int right)
{
    // To gather time for the merge operation:
    uint64_t startTime = ThreadTimer::getTime();

    // Temp array to store merged result
    vector<char> temp(right - left + 1);
    
    // index for the first half
    int i = left;
    
    // index for the second half
    int j = mid + 1;

    // index for the temp array
    int k = 0;

    // Compare and merge from both halves
    while (i <= mid && j <= right)
    {
        // Compare elements using the custom comparison
        if (compareMerge(arr[i], arr[j]))
        {
            // Take from first half into temp array
            temp[k++] = arr[i++];
        }
        else
        {
            // Take from second half into temp array
            temp[k++] = arr[j++];
        }
    }

    // Copy remaining elements from first half
    while (i <= mid)
    {
        temp[k++] = arr[i++];
    }
    
    // Copy remaining elements from second half
    while (j <= right)
    {
        temp[k++] = arr[j++];
    }
    
    // Copy back the merged results to the original array
    for (i = 0; i < k; i++)
    {
        arr[left + i] = temp[i];
    }

    // Print out the time taken
    uint64_t endTime = ThreadTimer::getTime();
    cout << "Thread " << this_thread::get_id()
         << " merge time for segment [" << left << "," << right << "]: "
         << (endTime - startTime) << " units\n";

}

uint64_t getCurrentTime() 
{
    auto now = this_thread::get_id();

    // Convert to int for the timing
    size_t time_val = hash<thread::id>()(now);

    // Convert and return time value in milliseconds
    return time_val % 1000000;
}

// Non-parallel merge sort
void regularMergeSort(vector<char>& arr, int left, int right) 
{
    // If there are two elements or more, sort.
    if (left < right)
    {
        // Find middle
        int mid = left + (right - left) / 2;

        // Sort first & second halves (recursively)
        regularMergeSort(arr, left, mid);
        regularMergeSort(arr, mid + 1, right);

        // Merge the sorted halves
        merge(arr, left, mid, right);
    }
}


// Parallel merge sort
void parallelMergeSort(vector<char>& arr, int left, int right, int depth)
{
    uint64_t startTime = ThreadTimer::getTime();
    // Use the regularMergeSort if we have used all the thread depth
    if (depth <= 0) 
    {
        regularMergeSort(arr, left, right);
        return;
    }
   

    // Find middle
    int mid = left + (right - left) / 2;

    // Creating thread to handle the left part
    thread leftThread([&arr, left, mid, depth]()
        {
            // Start time of thread
            uint64_t threadStart = ThreadTimer::getTime();
            
            // Recursively calling the function while reducing the thread depth
            parallelMergeSort(arr, left, mid, depth - 1);

            // End time of thread
            uint64_t threadEnd = ThreadTimer::getTime();

            // Printing out time of left thread
            cout << "Thread " << this_thread::get_id()
                 << " time: " << (threadEnd - threadStart) << " units\n";
        });
    
    // Use current thread for right half
    uint64_t rightStart = ThreadTimer::getTime();
    parallelMergeSort(arr, mid + 1, right, depth - 1);
    uint64_t rightEnd = ThreadTimer::getTime();

    // Printing out time of right thread
    cout << "Thread " << this_thread::get_id()
         << " right half time: " << (rightEnd - rightStart) << " units\n";

    // Join the left thread to complete
    leftThread.join();

    // Merge the sorted halves
    merge(arr, left, mid, right);
}

// Main function:
// - Will process command line arguments
// - Reads and filters input file
// - Background timer thread
// - Initiates Parallel sorting
// - Write results and timing information
int main(int argc, char* argv[])
{
    // Check command line arguments
    // Checks for exactly 4 arguments
    // Prints usage instructions if incorrect number of arguments
    if (argc != 4)
    {
        cerr << "Usage: " << argv[0] << " <input_file> <output_file> <thread_depth>\n";
        cerr << "thread_depth: 0 for regular, 1 for 2 threads, 2 for 4 threads, etc.\n";
        return 1;
    }

    // Read input file
    // Opens input file in binary mode using name from first argument
    ifstream inFile(argv[1], ios::binary);

    // Check if file is opened successfully
    if (!inFile)
    {
        cerr << "Error opening input file\n";
        return 1;
    }

    // Get file size for reporting, get file size by moving to end and checking position
    // Move to end of file
    inFile.seekg(0, ios::end);

    // Get current position (File size)
    size_t fileSize = inFile.tellg();

    // Move back to start for file reading
    inFile.seekg(0, ios::beg);

    // Read entire file into memory
    vector<char> data;
    
    // Pre allocate to avoid resizing
    data.reserve(fileSize); 
    char c;

    // Reads file character by character and filters
    while (inFile.get(c))
    {
        // Only include valid characters (0-9, A-Z, a-z)
        if ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z'))
        {
            data.push_back(c);
        }
    }
    // Close file
    inFile.close();

    // Check to see if any valid characters are found
    if (data.empty())
    {
        cerr << "No valid characters found in the input file, remember only numbers, upper & lowercase letters are allowed!\n";
        return 1;
    }

    // Get thread depth from command line
    // Converts third argument into integer for thread depth, makes sure thread depth is not negative
    // stoi converts string to int
    int threadDepth = stoi(argv[3]);
    if (threadDepth < 0)
    {
        cerr << "Thread depth must be non-negative\n";
        return 1;
    }

    // Print initial info
    // Print sorting parameters, input size, and thread configuration
    cout << "Starting sort with parameters:\n"
         << "Input size: " << data.size() << " characters\n"
         << "Thread depth: " << threadDepth << "\n"
         << "Maximum possible threads: " << (1 << threadDepth) << "\n\n";

    // Get start time
    uint64_t startTime = ThreadTimer::getTime();

    // Start a timer thread
    // Background thread for timing
    thread timerThread([]()
        {
            while (true)
            {
                // Thread continuously increments timer
                ThreadTimer::incremementTimer();

                // Sleep to prevent excessive CPU usage
                this_thread::yield();
            }
        });
    // Let it run independantly 
    timerThread.detach();

    // Sort the data
    parallelMergeSort(data, 0, data.size() - 1, threadDepth);

    // Records end time
    uint64_t endTime = ThreadTimer::getTime();

    // Write to output file
    ofstream outFile(argv[2]);

    // Check if file opened successfully
    if (!outFile) 
    {
        cerr << "Error opening output file\n";
        return 1;
    }

    // Writes sorted characters to file
    for (char c : data)
    {
        outFile << c;
    }

    // Closes output file
    outFile.close();

    // Print performance results
    // Total time taken
    // Processing speed
    cout << "\n Overall Performance:\n"
        << "Total time: " << (endTime - startTime) << " units\n"
        << "Characters processed: " << data.size() << "\n"
        << "Processing speed: "
        << (data.size() * 1.0 / (endTime - startTime))
        << " characters per unit time\n";

    return 0;

}

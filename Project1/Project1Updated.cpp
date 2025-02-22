// Project1.cpp: Arjun Suresh  - Parallel Merge Sort 
// Numbers, then uppercase, then lowercase

#include <iostream>
#include <fstream>
#include <thread>
//Mainly used for timer
#include <atomic>
#include <semaphore>
#include <mutex>

using namespace std;

struct ThreadTimer
{
	// Static atomic counter that is shared across thread
	static atomic<unsigned long long> globalTimer;

	// Returns the current timer value by loading atomic counter
	static unsigned long long getTime()
	{
		// Reads the current value atomically
		return globalTimer.load();
	}

	// Incremements the timer by one unit
	static void incrementTimer()
	{
		globalTimer++;
	}
 };

// Initialize timer variable to 0
atomic<unsigned long long> ThreadTimer::globalTimer(0);


// Custom comparison function that implements our rules
// Numbers first, then uppercase letters, then lowercase letters. 
bool compareMerge(char a, char b)
{
	// Check if the first character is a digit
	bool aIsDigit = (a >= '0' && a <= '9');
    bool bIsDigit = (b >= '0' && b <= '9');

    // If both are digits, compare them using normal character order
    if (aIsDigit && bIsDigit)
        return a < b;

    // If only a is a digit, it should come before b
    if (aIsDigit)
        return true;

    // If only b is a digit, a should come after b
    if (bIsDigit)
        return false;

    // Check if a is an uppercase letter (A-Z)
    bool aIsUpper = (a >= 'A' && a <= 'Z');
    // Check if b is an uppercase letter (A-Z)
    bool bIsUpper = (b >= 'A' && b <= 'Z');

    // If both are uppercase letters, compare them normally
    if (aIsUpper && bIsUpper)
        return a < b;

    // If only a is uppercase, it should come before b
    if (aIsUpper)
        return true;
    // If only b is uppercase, then a should come after b
    if (bIsUpper)
        return false;

    // At this point, both characters must be lowercase
    return a < b; 
}

// Merge function that merges the two sorted subarrays of arr into a single sorted array
void merge(char* arr, int left, int mid, int right)
{
    // Record the start time for the merge
    unsigned long long startTime = ThreadTimer::getTime();

    // Find number of elements to merge
    int n = right - left + 1;

    // Dynamically allocate a temporary array to hold merged elements
    char* temp = new char[n];

    // Initialize the indices for the left and right subarrays & the temp array
    
    // Starting index of left subarray
    int i = left;
    
    // Starting index of right subarray
    int j = mid + 1;

    // Index for temproary array
    int k = 0;

    // Merge the two subarrays into temp for now
    while (i <= mid && j <= right)
    {
        // Use compareMerge to decide which element comes first
        if (compareMerge(arr[i], arr[j]))
        {
            // Take element from left subarray
            temp[k++] = arr[i++];
        }
        else
        {
            // Take element from right subarray
            temp[k++] = arr[j++];
        }
    }

    // Copy any remaining elements from the left subarray
    while (i <= mid)
    {
        temp[k++] = arr[i++];
    }

    // Copy any remaining elements from the right subarray
    while (j <= right)
    {
        temp[k++] = arr[j++];
    }

    // Copy the merged elements back into the original array
    for (int m = 0; m < n; m++)
    {
        arr[left + m] = temp[m];
    }

    // Free the temporary array to avoid memory leaks
    delete[] temp;

    // Record the end time & print the time taken for merge
    unsigned long long endTime = ThreadTimer::getTime();
    cout << "Thread " << this_thread::get_id()
        << " merge time for segment [" << left << "," << right << "]: "
        << (endTime - startTime) << " units\n";
}

// Regular merge sort - non parallel
// Recursively divides the array and then merges sorted halves
void regularMergeSort(char* arr, int left, int right)
{
    // Check if the current segment has more than one element
    if (left < right)
    {
        // Find the middle of the index  of current segment
        int mid = left + (right - left) / 2;

        // Recursively sort the left half
        regularMergeSort(arr, left, mid);

        // Recursively sort the right half
        regularMergeSort(arr, mid + 1, right);

        // Merge the two sorted halves
        merge(arr, left, mid, right);
    }
}

// Parallel merge sort - merge sort using threads
// When thread depth is less than 0, falls back to regualrMergeSort
// 'depth' controls max recursion level that spawns new threads
void parallelMergeSort(char* arr, int left, int right, int depth)
{
    // Proceed if current segment has more than one element
    if (left < right)
    {
        // If the thread depth is 0, switch to the regular merge sort
        if (depth <= 0)
        {
            regularMergeSort(arr, left, right);
            return;
        }
    


    // Find the middle of the  index for current segment
    int mid = left + (right - left) / 2;

    // Create new threat to sort left half of array
    thread leftThread([arr, left, mid, depth]() {
        // Record the start time for the left segment
        unsigned long long threadStart = ThreadTimer::getTime();

        // Recursively call parallelmErgesort on left half with reduced depth
        parallelMergeSort(arr, left, mid, depth - 1);

        // Record the end time for the left segment
        unsigned long long threadEnd = ThreadTimer::getTime();

        // Output the time taken by this thread to process the left half
        cout << "Thread " << this_thread::get_id()
            << " left half time: " << (threadEnd - threadStart) << " units\n";
        });

    // Current thread, sort right half
    // Record the start time for the right segment
    unsigned long long rightStart = ThreadTimer::getTime();

    // Recursively call parallelmErgesort on right half with reduced depth
    parallelMergeSort(arr, mid + 1, right, depth - 1);

    // Record the end time for the right segment
    unsigned long long rightEnd = ThreadTimer::getTime();

    // Output the time taken by this thread to process the right half
    cout << "Thread " << this_thread::get_id()
        << " right half time: " << (rightEnd - rightStart) << " units\n";

    }
}

// Converts c-string into a non-negative int
// Returns -1 if conversion fails, like if the string contains non digit characters 
int toInt(const char* s) 
{
    // Variable to store the number
    int num = 0;

    // Check if the string is a null pointer (pointer doesnt point to anyt valid location)
    if (!s)
    {
        return -1;
    }

    // Iterate over each character in string
    for (int i = 0; s[i] != '\0'; i++)
    {
        char c = s[i];
        
        // If the character is not a digit, return error
        if (c < '0' || c > '9')
        {
            return -1;
        }

        // Build num by multiplying current number by and adding new digit
        // The reason for this is if you want to convert string of digits to int
        // You need to multiply by 10 and add the digit to shift the number over to the current
        // spot in the base 10 decimal system. It moves over, making room in the ones place for the next digit.
        num = num * 10 + (c - '0');
    }
    // Return the converted int
    return num;
}

// Checks if a character is valid for sorting
bool isValidChar(char c)
{
    return ((c >= '0' && c <= '9') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z'));
}

// Command line arguments, reads input files, sorts data, writes sorted output to file
// Starts background timer
int main(int argc, char* argv[]) 
{
    // 4 command line arguments have to be provided
    // Program name, input file name, output file name, thread depth (how many threads)
    if (argc != 4)
    {
        // Print usage instruction
        cerr << "Usage: " << argv[0] << " <input_file> <output_file> <thread_depth>\n";
        cerr << "thread_depth: 0 for regular sort, 1 for 2 threads, 2 for 4 threads, etc.\n";
        return 1; 
    }

    // Open the input file in binary mode
    ifstream inFile(argv[1], ios::binary);
    
    // Check to see if the file opened successfuly
    if (!inFile)
    {
        cerr < "error opening input file\n";
        return 1;
    }

    // Determine the size of the file by seeking to the end
    inFile.seekg(0, ios::end);

    // Get position, filesize in bytes
    size_t fileSize = inFile.tellg();

    // Return to beggining of file
    inFile.seekg(0, ios::beg);

    // Dynamically allocate a buffer to hold entire file content
    char* buffer = new char[fileSize];

    // Counter for valid characters
    size_t validCount = 0;

    // Variable to store each character that is read
    char c;

    // Read the file character by charactyer
    while (inFile.get(c)) 
    {
        // If the character is valid, store it in the buffer
        if (isValidChar(c)) {
            buffer[validCount++] = c;
        }
    }
    // Close the input file
    inFile.close(); 

    // Check if there were any valid characters in the file
    if (validCount == 0) {
        cerr << "No valid characters found in the input file. Only digits, uppercase and lowercase letters are allowed.\n";
        // Free the allocated memory
        delete[] buffer; 
        return 1;
    }

    // Allocate an array that is exactly the size needed for valid characters
    char* data = new char[validCount];

    // Copy valid characters from the buffer into the data array
    for (size_t i = 0; i < validCount; i++) {
        data[i] = buffer[i];
    }
    delete[] buffer; // Free the buffer as it is no longer needed

    // Convert the thread depth argument to an int
    int threadDepth = toInt(argv[3]);

    // If conversion fails or negative value is given
    if (threadDepth < 0) { 
        cerr << "Thread depth must be a non-negative integer.\n";

        // Free the allocated memory
        delete[] data; 

        return 1;
    }

    // Print initial information about the sorting operation.
    cout << "Starting sort with parameters:\n";
    cout << "Valid characters (approx. memory consumption): " << validCount << " bytes\n";
    cout << "Thread depth: " << threadDepth << "\n";
    cout << "Maximum possible threads: " << (1 << threadDepth) << "\n\n";

    // Start a background timer thread that continuously increments the global timer
    thread timerThread([]() {
        while (true) {
            // Increment timer continuously.
            ThreadTimer::incrementTimer(); 

            // Yield to allow other threads to run - so it doesn't get in the way
            this_thread::yield();           
        }
        });
    // Detach the timer thread so it runs independently
    timerThread.detach(); 

    // Record the starting time using timer
    unsigned long long startTime = ThreadTimer::getTime();

    // Perform parallel merge sort on the data array
    parallelMergeSort(data, 0, validCount - 1, threadDepth);

    // Record the ending time
    unsigned long long endTime = ThreadTimer::getTime();

    // Open the output file to write the sorted data
    ofstream outFile(argv[2]);

    // Check if the file opened successfully
    if (!outFile) { 
        cerr << "Error opening output file\n";
        // Free the allocated memory
        delete[] data;
        return 1; 
    }
    // Write each sorted character to the output file
    for (size_t i = 0; i < validCount; i++) {
        outFile << data[i];
    }

    // Close the output file
    outFile.close(); 

    // Print overall performance metrics
    cout << "\nOverall Performance:\n";
    cout << "Total time: " << (endTime - startTime) << " units\n";
    cout << "Characters processed: " << validCount << "\n";
    cout << "Processing speed: "
        << (double(validCount) / (endTime - startTime))
        << " characters per unit time\n";

    // Free the allocated memory for the data array
    delete[] data;

    // 0 is for success, 1 & -1 are error codes
    return 0; 
}
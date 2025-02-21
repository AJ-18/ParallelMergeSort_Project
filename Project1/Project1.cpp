// Project1.cpp: Arjun Suresh - Parallel Merge Sort 

#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>

using namespace std;

// Only want one thread to access the file at a time.
mutex file_mutex;

// Function to read the file and store the values in a vector
// Read each character, store it in vector<char>
vector<char> readFile(const string& filename)
{
    // Open file using ifstream
    ifstream file(filename, ios::in);
    vector<char> data;

	// Check if file is open
    if (!file) 
    {
        cerr << "Error opening the file: " << filename << endl;
        return data;
    }

    // Temporarily holds each character from the file
    char ch;

    // Loops through file to read one character at a time from the file to ch.
    while (file.get(ch)) 
    {
        // Adds character stored in ch to end of data vector
        data.push_back(ch);
    }

    // After loop is terminated, means all characters have been read, close file and return data
    file.close();
    return data;

}

// Actual merge sort happens here:
// Merge sort works by dividing an arrayh into two halves, sorting them, and then merging them back together.
// The function will merge two sorted sorted section, using a temp array to store sorted values which then copies them back into arr.
void merge(vector<char>& arr, int left, int mid, int right) 
{
    vector<char> temp(right - left + 1);
    int i = left, j = mid + 1, k = 0;

    while (i <= mid && j <= right)
    {
        if (arr[i] <= arr[j])
        {
            temp[k++] = arr[j++];
        }
    }
}



int main()
{
    
}

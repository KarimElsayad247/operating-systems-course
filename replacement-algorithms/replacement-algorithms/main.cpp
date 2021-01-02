#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <list>
#include <algorithm>

using namespace std;


/*
 * Utility funcitons that handle printing
*/

void printHeader(string policy)
{
    cout << "Replacement Policy = " << policy << endl;
    cout << "-------------------------------------" << endl;
    cout << "Page   Content of Frames" << endl;
    cout << "----   -----------------" << endl;
}

void printFooter(int nPageMisses) {
    cout << "-------------------------------------" << endl;
    cout << "Number of page faults = " << nPageMisses << endl;
}


template <typename Iter>
void printLine(int page, bool hit, Iter bufferBegin, Iter bufferEnd)
{
    // print current page
    cout << setfill('0') << setw(2) << page;

    // print an F if it's a miss
    if (hit) {
        cout << "     ";
    }
    else {
        cout << " F   ";
    }

    // print contents of buffer
    while (bufferBegin != bufferEnd) {
        cout << setfill('0') << setw(2) << *bufferBegin  << " ";
        bufferBegin++;
    }
    cout << endl;
}

// unneeded implementation of std::find
//template <typename Iter>
//bool existsInBuffer(int page, Iter begin, Iter end)
//{
//    while (begin != end) {
//        if (page == *begin)
//            return true;
//        begin++;
//    }
//    return false;
//}

// Used by LRU, this function takes a page and pushes it to the front
void pushElementToFront(list<int>& usedList, int page)
{
    // for optimizaiton, check last entry for the page
    // following principle of locality, the referenced page might be the MRU
    if (usedList.back() != page) {
        // 1- find in linked list
        auto locationToPushFront = find(usedList.begin(), usedList.end(), page);

        // 2- get contents
        int temp = *locationToPushFront;

        // 3- remove and put in front
        usedList.erase(locationToPushFront);
        usedList.push_back(temp);
    }
}

/*
 * FUNCTIONS
 * each of the subsequent functions takes as arguments an array of page references,
 * and the size of the buffer. each functions handles printing its results.
*/

int doFIFO(vector<int>& pageReferences, vector<int>::size_type nPages)
{

    // construct buffer to hold pages;
    vector<int> buffer;

    // maintain a reference to the first element to be inserted
    int first = 0;

    // hold number of misses to be printed later
    int nPageMisses = 0;

    printHeader("FIFO");

    for (vector<int>::iterator iter = pageReferences.begin(); iter != pageReferences.end(); ++iter) {

        // if buffer still has space, add elemetn
        if (buffer.size() < nPages) {

            // check first if elemetn already in buffer, if it's not add it, otherwise not
            if (find(buffer.begin(), buffer.end(), *iter) == buffer.end()) {
                buffer.push_back(*iter);
            }

            // line will be printed regardless
            printLine(*iter, true, buffer.begin(), buffer.end());
        }
        else { // if buffer is full, find page to replace if target not in buffer figure out which page to replace

            if (find(buffer.begin(), buffer.end(), *iter) != buffer.end()) {
                printLine(*iter, true, buffer.begin(), buffer.end());
                continue;
            }

            // if not in buffer, replace the firs to be added
            buffer[(first) % nPages] = *iter;
            printLine(*iter, false, buffer.begin(), buffer.end());
            first++;
            nPageMisses++;
        }

    }

    printFooter(nPageMisses);

    return 0;
}

int doLRU(vector<int>& pageReferences, vector<int>::size_type nPages)
{
    // construct buffer to hold pages
    vector<int> buffer;

    // construct list to hold order of frame usages
    list<int> usedList;

    // hold number of misses to be printed later
    int nPageMisses = 0;

    printHeader("LRU");

    for (vector<int>::iterator iter = pageReferences.begin(); iter != pageReferences.end(); ++iter) {

        // look for elemetn in page buffer
        auto found = find(buffer.begin(), buffer.end(), *iter);

        // if buffer still has space, add elemetn
        if (buffer.size() < nPages) {

            // check first if elemetn already in buffer, if it's not add it, otherwise not
            if (found == buffer.end()) {
                buffer.push_back(*iter);
                usedList.push_back(*iter);
            }
            else {
                // handle reference to found page
                pushElementToFront(usedList, *iter);
            }
            printLine(*iter, true, buffer.begin(), buffer.end());
        }
        else {
            // buffer is full but page is found
            if (found != buffer.end()) {

                // handle reference to found page
                pushElementToFront(usedList, *iter);

                printLine(*iter, true, buffer.begin(), buffer.end());
                continue;
            }

            // if not found, remove LRU element
            // 1- get least used
            int leastUsed = usedList.front();
            usedList.pop_front();

            // 2- find in buffer (get index of least used page)
            int index = find(buffer.begin(), buffer.end(), leastUsed) - buffer.begin();

            // 3- replace page and add to list
            buffer[index] = *iter;
            usedList.push_back(*iter);

            printLine(*iter, false, buffer.begin(), buffer.end());
            nPageMisses++;
        }
    }

    printFooter(nPageMisses);
    return 0;
}

int doOPTIMAL(vector<int>& pageReferences, vector<int>::size_type nPages)
{
    // replacement: sub iter from find
    // construct buffer to hold pages;
    vector<int> buffer;

    // hold number of misses to be printed later
    int nPageMisses = 0;

    printHeader("OPTIMAL");

    for (vector<int>::iterator iter = pageReferences.begin(); iter != pageReferences.end(); ++iter) {

        // if buffer still has space, add elemetn
        if (buffer.size() < nPages) {

            // check first if elemetn already in buffer, if it's not add it, otherwise not
            if (find(buffer.begin(), buffer.end(), *iter) == buffer.end()) {
                buffer.push_back(*iter);
            }

            // line will be printed regardless
            printLine(*iter, true, buffer.begin(), buffer.end());
        }
        else { 
            // if buffer is full, find page to replace if target not in buffer figure out which page to replace
            if (find(buffer.begin(), buffer.end(), *iter) != buffer.end()) {
                printLine(*iter, true, buffer.begin(), buffer.end());
                continue;
            }

            // for all pages in buffer, find the next occurence of each
            // find the index for which the score is highest
            int maxScore = 0;
            int maxScoreIndex = 0;
            for (size_t i = 0; i < buffer.size(); ++i) {
                int score = find(iter + 1, pageReferences.end(), buffer[i]) - iter; 
                if (score > maxScore) {
                    maxScoreIndex = i;
                    maxScore = score;
                }                
            }

            // select highest and replace
            buffer[maxScoreIndex] = *iter;

            printLine(*iter, false, buffer.begin(), buffer.end());
            nPageMisses++;
        }
    }    
    printFooter(nPageMisses);
    return 0;
}

int doCLOCK(vector<int>& pageReferences, vector<int>::size_type nPages)
{
    // construct buffer to hold pages;
    vector<int> buffer;

    // the additional bit to keep track of
    vector<int> bufferBit;

    // pointer is set to indicate the next frame in the buffer after the one just updated after replacement
    vector<int>::size_type associatedPointer = 0;

    // hold number of misses to be printed later
    int nPageMisses = 0;

    printHeader("CLOCK");

    for (vector<int>::iterator iter = pageReferences.begin(); iter != pageReferences.end(); ++iter) {
        
         // if buffer still has space, add elemetn
        if (buffer.size() < nPages) {

            // check first if elemetn already in buffer, if it's not add it, otherwise not
            if (find(buffer.begin(), buffer.end(), *iter) == buffer.end()) {
                buffer.push_back(*iter);
                bufferBit.push_back(1);
            }

            // line will be printed regardless
            printLine(*iter, true, buffer.begin(), buffer.end());
        }
        else {
            
            // try finding page in buffer
            vector<int>::iterator found = find(buffer.begin(), buffer.end(), *iter);

            // if it is in buffer, set its bit to one
            if (found != buffer.end()) {
                vector<int>::size_type index = found - buffer.begin();
                bufferBit[index] = 1;
                printLine(*iter, true, buffer.begin(), buffer.end());
                continue;
            }

            // // keep track of the first page in loop to check if we did one full loop
            // vector<int>::size_type startingPage = associatedPointer;

            // if not in buffer, look for an page to replace. pointer needs to get back to original point 
            // in case all page bits were 1, so 1 more loop, hence + 1. not using <= for clarity.
            for (vector<int>::size_type index = associatedPointer; index < buffer.size() + 1; ++index) {
                
                // just to make typing shorter
                vector<int>::size_type i = index % buffer.size();

                // replace if bit is 0
                if (bufferBit[i] == 0) {
                    buffer[i] = *iter;
                    bufferBit[i] = 1;
                    printLine(*iter, false, buffer.begin(), buffer.end());
                    nPageMisses++;

                    // pointer will be set to next page
                    associatedPointer = (i + 1) % buffer.size();
                    break;
                }
                // but if bit is one, set to zero
                else if (bufferBit[i] == 1) {
                    bufferBit[i] = 0;
                }
            }
        }       
    }
    printFooter(nPageMisses);
    return 0;
}

int main()
{

    // get amount of pages allocatec to process
    int nPages;
    cin >> nPages;

    // get algorithm type [FIFO, OPTIMAL, LRU, CLOCK]
    string sAlgorithm;
    cin >> sAlgorithm;

    // the sequence of pages which the program requests
    vector<int> pageReferences;
    int input;
    while (cin >> input) {

        if (input == -1)
            break;

        pageReferences.push_back(input);
    }

    // invoke appropriate function, each function handles when to print what
    if (sAlgorithm.compare("FIFO") == 0) {
        doFIFO(pageReferences, nPages);
    }
    else if (sAlgorithm.compare("LRU") == 0) {
        doLRU(pageReferences, nPages);
    }
    else if (sAlgorithm.compare("OPTIMAL") == 0) {
        doOPTIMAL(pageReferences, nPages);
    }
    else if (sAlgorithm.compare("CLOCK") == 0) {
        doCLOCK(pageReferences, nPages);
    }
    else {
        cout << "Algorithm not recognised!" << endl;
    }
    return 0;
}

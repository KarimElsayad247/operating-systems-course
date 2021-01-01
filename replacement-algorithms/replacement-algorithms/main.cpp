#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <list>
#include <algorithm>

using namespace std;

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
void printLine(int page, bool hit, Iter begin, Iter end)
{
    cout << setfill('0') << setw(2) << page;

    if (hit) {
        cout << "     ";
    }
    else {
        cout << " F   ";
    }

    while (begin != end) {
        cout << setfill('0') << setw(2) << *begin  << " ";
        begin++;
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

void pushElementToFront(list<int>& usedList, int page)
{
    // 1- find in linked list
    auto locationToPushFront = find(usedList.begin(), usedList.end(), page);

    // 2- get contents
    int temp = *locationToPushFront;

    // 3- remove and put in front
    usedList.erase(locationToPushFront);
    usedList.push_back(temp);
}


// first, implement fifo
// you need a buffer of some size nPages
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

    // vector to keep track of the furthes page
    // i.e. the one that will be requestd the latest
    vector<int> scores(nPages, 0);

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
                scores[i] = find(iter + 1, pageReferences.end(), buffer[i]) - iter; 
                if (scores[i] > maxScore) {
                    maxScoreIndex = i;
                    maxScore = scores[i];
                }                
                // maxScoreIndex = (scores[i] > maxScore) ? i : maxScoreIndex;
                // maxScore = max(maxScore, scores[i]);
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

    // invoke appropriate function
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

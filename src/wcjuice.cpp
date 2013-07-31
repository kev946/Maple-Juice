#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

using namespace std;

/*
 * Word count Juice task example.  This is only an example; other ways
 * of implementing this are possible, but you must keep all the distributed
 * functionality separate from Maple/Juice tasks.
 *
 * NOTE: assumes input is already sorted by key!
 *
 * Usage: wcjuice <input file> <output file>
 */
int main(int argc, char **argv)
{
    // open input and output files
    if (argc != 3) {
        cerr << "usage: wcjuice <input file> <output file>\n";
        return 1;
    }
    ifstream input(argv[1]);
		ofstream output(argv[2], ofstream::app);

    // get tuples from input file, count, write tuples to output file
    map <string, int> keycount;
    string tmp, key;
    while(input >> tmp >> key >> tmp >> tmp >> tmp) {
        keycount[key]++;
    }
    map<string, int>::const_iterator itr;
    for(itr = keycount.begin(); itr != keycount.end(); ++itr) {
        output << "( " << (*itr).first << " , " << (*itr).second << " )" << endl;
    }

    return 0;
}

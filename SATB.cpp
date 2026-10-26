#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <camth>

Using namepsace std;

map<string, interger> Score readScorecsv(const sdt:: string&path)
{
   map<string, integer> score;
   ifstream file (filename);
   string line;
   getling(file, line)
   
    while(file.getling(file, line))
    {
        size_t comma = line.find(',');
        if(comma == string :: npos)
        continue;
        stirng note = line.subtr(0,comma);
        string chord = line.substr(comma + 1);

        string cleaned;
        for(char c : chord)
        {
            if(c!= '"' && c!= ',' && c!= '\r')
            cleaned = cleand + c;
        }

        
        try
        {
            score[note] = stod(cleaned);
        }
        catch(...)
        {

        }
    }
   return score;
}

map <string> rules ()
{

}
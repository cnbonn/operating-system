#include <iostream>
#include <string.h>
#include <string>
#include <sstream>
#include <locale>

#include "scheduling.cpp"
#include "page.cpp"
using namespace std;


 
int main()
{

  //arguments
  string line =""; 
  locale loc;

  while(1)
  {
    cout << endl;
    cout << "*********************************" << endl;
    cout << "*           MAIN MENU           *" << endl;
    cout << "*********************************" << endl;

    cout << "1) Process Scheduler Simulation (PSS)" << endl;
    cout << "2) Memory Management Simulation (MMS)" << endl;
    cout << "3) Page Replacement Simulation  (PRS)" << endl;
    cout << "4) Exit                         (EXIT)\n" << endl;
    cout << endl << "CMD> ";


    getline(cin, line);
    
    //convert to uppercase
    for(int i = 0; i < line.length() ; i++ )
      line[i] =toupper( line.at(i) );

    //exit program
    if(line.compare("EXIT") == 0 )
    {
      break;
    }
    
    else if(line.compare("PSS") == 0)
    {
      p_scheduling();
    }

    else if(line.compare("MMS") == 0)
    {
     
    }
    
    else if(line.compare("PRS") == 0)
    {
      page_sim();
    }

    else 
    {
      cout << "Please use a valid command" << endl;
    }
    //reset string
    line.clear();
  }

  return 0;
}

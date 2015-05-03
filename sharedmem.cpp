#include <iostream>
#include <string>
#include <dirent.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>


int main()
{
  DIR *dir; // directory pointer 
  bool exit = false;
  string command; 
  string command2; 
  string command3;


  // while not exit run the loop
  while(exit == false)
  {
   
   
    cout << "dash>";
    cin >> command;

    if(command.compare("mboxinit") == 0)
    {
      cin >> command2;
    }
    else if(command.compare("mboxdel") == 0)
    {
      cin >> command2;
    }
    else if(command.compare("mboxwrite") == 0)
    {
    }
    else if(command.compare("mboxread") == 0)
    {
    }
    else if(command.compare("mboxcopy") == 0)
    {
    }
    else if(command.compare("exit") == 0)
    {
      exit = true;
    }
    else
	exit = false;
  }

 
 
  return 0;
}

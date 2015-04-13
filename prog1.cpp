#include <iostream>
#include <string>
#include <dirent.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>

using namespace std;
/***************************************
 *
 *  function pid
 *  input command
 *
 *  this function sarches system files looking for 
 *  commands that are active pid
 *
 */////////////////////////////////////

void pid(string command)
{
  DIR *dir;
  struct dirent *ent;
  struct stat data;
  ifstream info;
  char temp[1025];
  bool empty = true;


  cout << endl << "PID with command: " << command << endl;

  // if you can open the directory
  if((dir = opendir("/proc")) != NULL) // open main dir
  {
    //while you can open a dir
    while((ent = readdir (dir)) !=NULL ) 
    {
      // if the name fits
      if( ent->d_name[0] != '.' && isdigit(ent->d_name[0]) == 1)
      {
       //  printf("\n%s",ent->d_name);
       //  create file name
         string path = string("/proc") + "/" + string(ent->d_name) + string("/cmdline");

         //ut << "path: " << path.c_str() << endl;
         info.open(path.c_str());
         // while you can read the file
         if(info.getline(temp,1025 ))
         { 
           string tempstr(temp);
           if(tempstr.find(command) != string::npos)
           {
              cout << "path: " << path.c_str() << endl;
              cout << temp;
            // while(info.getline(temp,1025));
            // {
               cout <<  path;
               empty = false;
           //  }
           }
         } 
         // if pid is empty
         if(empty == true)
         {
           cout << "No pid found matching command:" << command ;
         }
	 info.close(); 
      }
                       
    }
  closedir(dir);
  }
 
  else 
  {
    // could not open directory
    cout << "could not open dir" << endl;
    return;
  }
  cout << endl;
 }


/*************************
 * function: cmdnm
 * input: pid
 *
 * this function searches system folders to look for the process
 * number and displays the commands that started it
 *****************************************/




void cmdnm(string pid)
{
  DIR *dir;
  struct dirent *ent;
  struct stat data;
  ifstream info;
  char temp[1025];
  bool empty = true;


  cout << endl << "Commands under PID# :" << pid << endl;

  // if you can open the directory
  if((dir = opendir("/proc")) != NULL) // open main dir
  {
    //while you can open a dir
    while((ent = readdir (dir)) !=NULL ) 
    {
      // if the name fits
      if( ent->d_name[0] != '.' && isdigit(ent->d_name[0]) == 1 && ent->d_name == pid)
      {
       //  printf("\n%s",ent->d_name);
       //  create file name
         string path = string("/proc") + "/" + string(ent->d_name) + string("/cmdline");

         cout << "path: " << path.c_str() << endl;
         info.open(path.c_str());
         // while you can read the file
         if(info.getline(temp,1025 ))
         {
           cout << temp;
           while(info.getline(temp,1025));
           {
             cout << temp ;
             empty = false;
           }
         } 
         // if pid is empty
         if(empty == true)
         {
           cout << "Pid is empty" ;
         }
	 info.close(); 
         cout << endl;       
      }
                       
    }
  closedir(dir);
  }
 
  else 
  {
    // could not open directory
    cout << "could not open dir" << endl;
    return;
  }
  cout << endl;
 }





/***********************************88
 * function systat
 * purpose: to read out system information.
 *
 * prints system info by accessing the system 
 * files and retreving the data.
 *
 *
 * *****************************************/


void systat()
{
  ifstream info;
  char temp[1025];


  // open file
  info.open("/proc/version");

  

  cout << endl << "*****System Info******" << endl << endl;
  //read file and output file
  while( info >> temp )
  {
    cout << temp; 
  }

  cout << endl << endl;
  //close file
  info.close();


  //up time
  info.open("/proc/uptime");
  cout << "*****Uptime*****" << endl << endl;
  //read file and output file
  while( info.getline(temp,1025) )
  {
    cout << temp << endl;
  }
  cout << endl;
  info.close();

  //memory useage
  info.open("/proc/meminfo");
  cout << "*****Memory info*****" << endl << endl;
  while( info.getline(temp,1025) )
  {
    cout << temp << endl;
  }
  cout << endl;
  info.close();

  //cpu usage
  info.open("/proc/cpuinfo");
  cout << "*****CPU info*****" << endl << endl;
  info.getline(temp,256);
  for(int i = 0; i < 8; i++)
  {
    info.getline(temp,256);
    cout << temp << endl;;
  }
  cout << endl;
  info.close();
}

int main()
{
  DIR *dir; // directory pointer 
  bool exit = false;
  string command; 
  string command2; 



  // while not exit run the loop
  while(exit == false)
  {
   
   
    cout << "dash>";
    cin >> command;

    if(command.compare("cmdnm") == 0)
    {
      cin >> command2;
      cmdnm(command2);
    }
    else if(command.compare("pid") == 0)
    {
      cin >> command2;
      pid(command2);
    }
    else if(command.compare("systat") == 0)
    {
      systat();
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

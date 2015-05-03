#include <iostream>
#include <time.h>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <cstdio>
#include <algorithm>

using namespace std;



//Structure for processes.
struct process
{
  int id;
  int start_time;
  int burst;
  int time_left;
  int priority;
  bool done;
};

//function definitions
int create_processes(vector<process> &proc_list, int &processes);
void assign_priority(int processes, vector<process> &proc_list);
void reset_time(int processes, vector<process> &proc_list);
int get_quantum();
void RR(vector<process> &proc_list, int quantum, int processes);
void priority(vector<process> &proc_list, int quantum);
void SJF(vector<process> &proc_list, int quantum);
void sortbystart(vector<process> &proc_list, int processes);
void print_proc(vector<process> &proc_list, int processes);
int get_total_time(vector<process> &proc_list, int processes);
void print_sim(vector<process> &proc_list, int curr_proc);
bool valid_proc(vector<process> &proc_list, int curr_time, int curr_proc);

/******************************************
 * Function: p_scheduling
 *
 * input
 *
 * purpose: menu for selecting what process
 * scheduling simulation to run and calls the 
 * proper function to simulate
 * *******************************************/
int p_scheduling()
{
  string line = "";
  vector<process> proc_list;  
  bool exit = false; 
  int quantum = 1;
  int processes = 0;

  do
  {
    cout << "********************************" << endl;
    cout << "* Process Scheduling Simulator *" << endl;
    cout << "********************************" << endl;
    cout << "1) Round Robin        (RR)" << endl;
    cout << "2) Priority           (P)" << endl;
    cout << "3) Shortest Job First (SJF)" << endl;
    cout << "4) Exit to Main       (EXIT)" << endl << endl;
    cout << "PSS> ";
    cin >> line;

    for(int i = 0; i < line.length(); i++)
      line[i] = toupper(line.at(i));

    if(line.compare("EXIT") == 0)
    {
      exit = true;
      break;  
    }
    else if(line.compare("RR") == 0)
    {
      create_processes(proc_list, processes);
      quantum = get_quantum();
      RR(proc_list, quantum, processes);
    }
    else if(line.compare("P") == 0)
    {
      create_processes(proc_list, processes);
      quantum = get_quantum();
      priority(proc_list, quantum);
    }
    else if(line.compare("SJF") == 0)
    {
      create_processes(proc_list, processes);
      quantum = get_quantum();
      SJF(proc_list, quantum);
    }
    else
    {
      cout << "Please Enter a Valid Command" << endl;
    }

    line="";
  }while(exit == false);

}
/************************************************
 * function create_processes
 *
 * input: proc_list - list of processes
 *
 * function: create processes based of user input
 * ***********************************************/
int create_processes(vector<process> &proc_list, int &processes)
{
  //varables
  string choice;
  bool valid = false;
  process temp;
  int start = 0;
  int burst = 0;
  int priority = 0;

 
  if( !proc_list.empty())
  {
    cout << "Would you like to use the same processes? (y/n)" << endl;
    cout << "CMD>: ";
    cin >> choice;

    if(choice.compare("y") == 0)
    {
      reset_time(processes, proc_list);
 	return 2;
    }
    //clear list if new list
    proc_list.erase(proc_list.begin(), proc_list.end());
  }

  cout << endl;
  cout << " How many processes would you like to create?" << endl;
  //check for valid input. 
  while(!valid)
  {
    cout << " Processes: " ;
    cin >> processes;
    if(!cin.fail() && (cin.peek()==EOF || cin.peek()=='\n'))
	valid = true;
    else
    {
      cin.clear();
      cin.ignore(256,'\n');
      cout <<"Please enter a number" << endl;
    }
  }


  cout << "Creating " << processes << " processes" << endl;

  //initilize random seed
  srand(time(NULL));
 
  //create processes
  for(int i = 0; i < processes; i++)
  {
    //create process and assign id
    proc_list.push_back(process());
    proc_list[i].id = i;
   
    //assign random information
    proc_list[i].start_time = rand() % 10 + 1;
    proc_list[i].burst = rand() % 10 + 3;
    proc_list[i].time_left = proc_list[i].burst;
    proc_list[i].done = false;
  }

  // assign random start time to zero
  proc_list[ rand() % processes ].start_time = 0;

  assign_priority(processes, proc_list);
  reset_time(processes, proc_list);

  //print_proc(proc_list, processes);
  sortbystart(proc_list, processes); 

  print_proc(proc_list, processes);
  return 1;  
}
/***************************************************8
 * function: assign priority
 *
 * input: processes - number of processes
 *        proc_list - vector of process stucts
 *
 * function: assign a unique priority to each process
 * *****************************************************/


void assign_priority(int processes, vector<process> &proc_list)
{
  int * used;
  used = new int [processes];
  int temp;
  bool unique = true;
  // create a used array
  for(int i = 0; i < processes ; i++)
  {
    used[i] = 1;
    proc_list[i].priority = 0;
  }
  //check to see if each random value assigned in unique
  for(int i = 0; i < processes ; i++)
  {
    unique = false;
    while(unique == false)
    {
      temp = ((rand() % processes) + 1); //set random temp value

      for(int j = 0; j < processes; j ++)
      {
        if(proc_list[j].priority == temp)
        {
          unique = false;
        }
	else
          unique = true;
      }
      if(unique == true)
      {
        proc_list[i].priority = temp;
      }
    }
  }
  delete [] used; // delete temp array 
}
/*****************************************************
 * function: reset_time
 *
 * input: proc_list - list of processes
 *
 * function: reset the durration time of the processes
 *****************************************************/
void reset_time( int processes, vector<process> &proc_list)
{
  for(int i = 0; i < processes; i++ )
  {
    proc_list[i].time_left = proc_list[i].burst;
    proc_list[i].done = false;
  }
}
 
/*********************************************8
 * Function RR
 *
 * input:
 *
 * Purpose: This function impliments the round
 * robin process scheduling function functionality
 * using a que system
 *
 * *********************************************/

void RR(vector<process> &proc_list, int quantum, int processes)
{
  int time = 0;
  int qtime;
  int total_time;
  int curr_proc = 0;
  int fail_safe;

  // get times
  total_time = get_total_time(proc_list, processes);

  //print statements
  cout << endl;
  cout << " Round Robin Proccess Scheduling Simulation" << endl;
  cout << "     Quantum : " << quantum << endl << endl;

  cout << " total time: " << total_time << endl; 
  //go until all jobs finish duration
  for(int curr_time = 0; curr_time < total_time; curr_time++)
  {
    curr_proc++;
    while(!valid_proc(proc_list, curr_time, curr_proc))
    {
      curr_proc++;
      //reset curr proc for wrap around
      if(curr_proc > processes)
      {
        curr_proc = 0;
      }
    }    

    for(qtime = 0; qtime < quantum ; qtime++ )
    {

      print_sim(proc_list, curr_proc);
 
      proc_list[curr_proc].time_left--;
      
      if(proc_list[curr_proc].time_left == 1 )
      {
        qtime = quantum; // gets out of quantum time loop/.
        proc_list[curr_proc].done = true;
        break;
       
      }
    }
  }
  
  return;

}

/***********************************************
 * function: priorit
 *
 * input:
 *
 * Purpose: this function impliments the priority
 * process scheduling functionalit
 * **********************************************/
void priority(vector<process> &proc_list, int quantum)
{
  int time = 0;
  //print statement
  cout << endl;
  cout << " Priority Process Scheduling Simulation " << endl;
  cout << "     Quantum : "<< quantum << endl << endl;
}
/***********************************************
 * Function: SJF
 *
 * input:
 *
 * Purpose: This function impliments the priority
 * process scheduling functionality
 * *********************************************/
void SJF(vector<process> &proc_list , int quantum)
{
  int time = 0;

  //print statement
  cout << endl;
  cout << " Shortest Job First Scheduling Simulation " << endl;
  cout << "      Quantum : " << quantum << endl << endl;
}

int get_quantum()
{
  bool valid = false;
  int quantum;
 
  cout << endl;
  cout << "What is the quantum of the simulation?" << endl;
  //check for valid input. 
  while(!valid)
  {
    cout << " Quantum: " ;
    cin >> quantum;
    if(!cin.fail() && (cin.peek()==EOF || cin.peek()=='\n'))
	valid = true;
    else
    {
      cin.clear();
      cin.ignore(256,'\n');
      cout <<"Please enter a number" << endl;
    }
  }
   

  return quantum;
}

void sortbystart(vector<process> &proc_list, int processes)
{
  int i; int j;
  
  for(i = 0; i < proc_list.size() ; i++)
  {
    for( j = 0; j < (proc_list.size() - 1); j++)
    {  
      if( proc_list[j].start_time > proc_list[j+1].start_time )
      {
        swap(proc_list[j], proc_list[j+1]);  
      }
    }
  }
  return;
}

void print_proc(vector<process> &proc_list, int processes)
{
  for(int i = 0; i < processes; i++)
  {
    cout << "process " << i << " assigned to: " << proc_list[i].id << endl;
    cout << "Start time: " << proc_list[i].start_time << endl;
    cout << "Burst time: " << proc_list[i].burst << endl;
    cout << "Priorty: " << proc_list[i].priority << endl;
    cout << "Time: " << proc_list[i].time_left << endl;
   } 
}

int get_total_time(vector<process> &proc_list, int processes)
{
  int total_time = 0;
  for(int i = 0; i < processes; i++)
    total_time += proc_list[i].burst;

  return total_time;
}

void print_sim(vector<process> &proc_list, int curr_proc)
{
    int curr; 
    cout << "current process: [ " << proc_list[curr_proc].id << " | "
                    << proc_list[curr_proc].time_left   << " ]" << endl;
   
    if(curr_proc == proc_list.size())
    {
      curr = 0;
    }
    cout << "inque process  : ";
   
    cout << "[ " << " | " << " ]";
    cout << endl;

   cin.ignore();
}

bool valid_proc(vector<process> &proc_list, int curr_time, int curr_proc)
{
  
  if(proc_list[curr_proc].done == true)
  {
    return false;
  }
  
  if(proc_list[curr_proc].time_left == 0)
  {
    return false; 
  }
  if(proc_list[curr_proc].start_time > curr_time)
  {
    return false;
  } 

  return true;
}

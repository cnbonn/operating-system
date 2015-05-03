#include <iostream>
#include <time.h>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <cstdio>
#include <algorithm>
#include <queue>

#include "checkfunctions.cpp"

using namespace std;



//function definitions
int page_sim();
void create_page(vector<int> &page_list, int &pages, int **&page_table, int &table_size);
void create_page_table(int **&page_table, int &table_size);
void reset_table(int **&page_table, int table_size);
void delete_page_table(int **&page_table, int table_size);
void print_page(vector<int> &page_list, int pages);
void print_table(int **&page_table, int table_size, int spot);
void fifo_sim(vector<int> &page_list, int pages, int **&page_table, int table_size);
void o_sim(vector<int> &page_list, int pages, int **&page_table, int table_size);
int o_find_next(vector<int> &page_list, int pages, int current);
void LRU_sim(vector<int> &page_list, int pages, int **&page_table, int table_size);
void LFU_sim(vector<int> &page_list, int pages, int **&page_table, int table_size);
void SC_sim(vector<int> &page_list, int pages);
void c_sim(vector<int> &page_list, int pages);



/******************************************
 * Function: p_scheduling
 *
 * input
 *
 * purpose: menu for selecting what process
 * scheduling simulation to run and calls the 
 * proper function to simulate
 * *******************************************/
int page_sim()
{
  string line = "";
  bool exit = false;
  vector<int> page_list; 
  int pages;
  int ** page_table = NULL;
  int table_size;

  do
  {
    cout << "********************************" << endl;
    cout << "*  Page Replacement Simulator  *" << endl;
    cout << "********************************" << endl;
    cout << "1) FIFO            (FIFO) " << endl;
    cout << "2) Optimal         (O)" << endl;
    cout << "3) LRU             (LRU) " << endl;
    cout << "4) LFU             (LFU)" << endl;
    cout << "5) Second Chance   (SC)" << endl;
    cout << "6) Clock           (C)" << endl;
    cout << "7) Exit            (EXIT)" << endl << endl;
    cout << " PRS> ";
    cin >> line;

    for(int i = 0; i < line.length(); i++)
      line[i] = toupper(line.at(i));

    if(line.compare("EXIT") == 0)
    {
      exit = true;
      delete_page_table(page_table, table_size);
      break;  
    }
    else if(line.compare("FIFO") == 0)
    {
      create_page(page_list, pages, page_table, table_size);
      fifo_sim(page_list, pages, page_table, table_size);
    }
    else if(line.compare("O") == 0)
    {
      create_page(page_list, pages, page_table, table_size);
      o_sim(page_list, pages, page_table, table_size);
    }
    else if(line.compare("LRU") == 0)
    {
      create_page(page_list, pages, page_table, table_size);
      LRU_sim(page_list, pages, page_table, table_size); 
    }
    else if(line.compare("LFU") == 0)
    {
      create_page(page_list, pages, page_table, table_size);
      LFU_sim(page_list, pages, page_table, table_size);
    }
    else if(line.compare("SC") == 0)
    {
     // create_page(page_list, pages);
      SC_sim(page_list, pages);
    }
    else if(line.compare("C") == 0)
    {
     // create_page(page_list, pages);
      SC_sim(page_list, pages);
    }
    else
    {
      cout << "Please Enter a Valid Command" << endl;
    }

    line="";

  }while(exit == false);

}
/**********************************************
 *function: create_page
 * 
 * input: page_list - list of pages
 *        pages - number of pages
 *
 *  purpose: create pages 
 *
 *  ****************************************/
void create_page(vector<int> &page_list, int &pages, int **&page_table, int &table_size)
{
  bool reuse;
  //check to see if pages exist
  if(!page_list.empty())
  {
    reuse = reuse_list(3); // 3 for page
    reset_table(page_table, table_size);
    if( reuse == true )
    {
      print_page(page_list, pages);
      return;
    } 
    page_list.erase(page_list.begin(), page_list.end()); //erase previous pages
    delete_page_table(page_table, table_size);
  }

  cout << endl << "  How many pages would you like to simulate? " << endl;; 

  pages = valid_num(); // check for valid number

  cout << " Creating " << pages << " pages" << endl;

  srand(time(NULL));  // random seed

  for(int i = 0; i < pages ; i++)
  {
    page_list.push_back(rand() % 10 +1);
  }
 
  //create page table
  create_page_table(page_table, table_size); 

  print_page(page_list, pages);
}
/**********************************************
 * function: print pages
 *
 * input: page_list - list of pages
 *        pages - number of pages
 *
 * purpose: print the pages in the list
 *
 * **********************************************/
void print_page(vector<int> &page_list, int pages)
{
  cout << " Pages: |" ;
  for(int i = 0 ; i < pages ; i++)
  {
    cout << page_list[i] << "|" ;
  }
  cout << endl;
}
/*****************************************************
 * function: prin_table
 *
 * input: page_table - page table
 *        table_size - size of the page table
 *
 * purpose: print the page
 * ***************************************************/
void print_table(int **&page_table, int table_size, int spot)
{
     
    //print 
    cout << "table " << spot+1 << " : |";
    for(int j = 0 ; j < table_size ; j++)
    {
      cout << page_table[j][0] << "|";
    }
    cout << endl;

 
}
/************************************************
 * function: create page table
 *
 * input: *page_table
 *
 * purpose: to create the page table
 * *********************************************/
void create_page_table(int **&page_table, int &table_size)
{
  
  cout << " What is the size of the page table? " << endl;
  table_size = valid_num();

  page_table = new(nothrow) int* [table_size];
  if(page_table == NULL)
  {
    cout << " ERROR: MEMORY COULD NOT BE ALLOCATED " << endl;
    return;
  }

  //create second row
  for(int i = 0; i < table_size; i++)
  {
    page_table[i] = new int [2];
  }

  reset_table(page_table, table_size);
  
}

/************************************************
 * function: delete_page_table
 *
 * input: *page_table
 *
 * purpose delete page table
 * *********************************************/
void delete_page_table(int **&page_table, int table_size)
{
  for(int i = 0; i < table_size ; i++)
    delete [] page_table[i];

  delete [] page_table;
}
/************************************************
 * function: reset_table
 *
 * input page_table - the page table
 *       table_size - the size of the table
 *
 * function: pad page_Table with zeros
 *
 * ***********************************************/
void reset_table(int **&page_table, int table_size)
{
  for(int i = 0; i < table_size; i++)
  {
    for(int j = 0; j < 2; j++)
    {
      page_table[i][j] = 0;
    }
  }
}
/************************************************
 * function fifo_sim
 *
 * input: page_list - list of pages
 *        pages - number of pages
 *
 * purpose: fifo sim
 * **********************************************/
void fifo_sim(vector<int> &page_list, int pages, int **&page_table, int table_size)
{
  int table_spot = 0;
  int fault_count = 0; 
  bool fault;
  //run through list 
  for(int i = 0; i < pages ; i++)
  {
    if( table_spot >= 3 )
       table_spot = 0;

    fault = true;
   //check to see if in array
    for(int j = 0; j < table_size; j++)
    {
      if(page_list[i] == page_table[j][0] )
      {
        fault = false;
      }
    }

    // if not in array
    if(fault == true)
    {
      page_table[table_spot][0] = page_list[i];
      table_spot++;
      fault_count ++;
    }
    
    //print table
    print_table(page_table, table_size, i);
   
  }
  cout << "Fault count: " << fault_count << endl;

}
/**********************************************8
 * function o_sim
 *
 * input: page_list - list of pages
 *        pages - number of pages
 *        page_table - page table
 *        table_size - size of the page table
 *
 * purpose: optimal simulation
 * *********************************************/
void o_sim(vector<int> &page_list, int pages, int **&page_table, int table_size)
{ 
  int table_spot = 0;
  int fault_count = 0; 
  bool fault;
  int l_distance;
  int l_spot = 0;
  int dis;

  //initilize high values
  for(int i = 0; i < table_size ; i++)
  {
    page_table[i][1] = pages;
  }


  //run through list 
  for(int i = 0; i < pages ; i++)
  {
   if( table_spot >= 3 )
       table_spot = 0;

    //adjust distances
    for(int j = 0 ; j < table_size ; j++)
    {
      page_table[j][1] -= 1;
    }

    fault = true;
   //check to see if in array
    for(int j = 0; j < table_size; j++)
    {
      if(page_list[i] == page_table[j][0] )
      {
        fault = false;
      }
    }

    // if not in array
    if(fault == true)
    {

      l_distance = page_table[0][1];
      if( i > 2)
      {
        for(int k = 0; k < 3; k++)
        {
          if( page_table[k][1] > l_distance)
          { 
            l_distance = page_table[k][1];
            table_spot = k;
          }
          else
           table_spot = 0;
        }
      }
 

 
    
      //find longest period of time
      page_table[table_spot][0] = page_list[i];
      dis = o_find_next(page_list, pages, i);
      page_table[table_spot][1] = dis;
      

      //increnment
      table_spot++;
      fault_count ++;
   }
     
    //print 
    print_table(page_table, table_size, i );
  }
 
 cout << "Fault count: " << fault_count << endl;


}
/******************************************
 * function o_find_time
 *
 * input page_list - list of pages
 *       pages - number of pages
 *       current - current spot
 *
 * purpose: find next distance
 *
 * out - distance
 * ****************************************/
int o_find_next(vector<int> &page_list, int pages, int current)
{
  int time = (pages * 2) - ( 2 * current );
  int count = 0;

  // loop though and cound distance
  for(int i = current+1 ; i < pages; i++)
  {
    count ++;
    if(page_list[i] == page_list[current])
    {
     return count;
    }
  } 
  return time;
}
/*****************************************
 * function: LRU
 *
 * input: page_list - list of pages
 *        pages - number of pages
 *
 * purpose: simulate the least recentuly used
 *          pages
 * *******************************************/
void LRU_sim(vector<int> &page_list, int pages, int **&page_table, int table_size)
{
  int table_spot = 0;
  int fault_count = 0; 
  bool fault;
  int lru;

  // initilise page table
  for(int i = 0 ; i < table_size ; i++ )
  {
    page_table[i][1] = 1;
  }
  //run through list 
  for(int i = 0; i < pages ; i++)
  {
    fault = true;
   //check to see if in array
    for(int j = 0; j < table_size; j++)
    {
      if(page_list[i] == page_table[j][0] )
      {
        page_table[j][1] = 0; // set back to zero
        fault = false;
      }
    }
 
    // if not in array
    if(fault == true)
    {
      if(i > 2)
      {
       //find longest 
        lru = page_table[0][1];
        for(int k = 0; k < table_size ; k++)
        {
          if(page_table[k][1] >= lru )
          {
            table_spot = k;
          }
           
        }
      } 
      page_table[table_spot][0] = page_list[i];
      page_table[table_spot][1] = 0;
      table_spot++;
      fault_count ++;
    }
  
    //increment all 
    for(int j = 0 ; j < table_size ; j++)
    {
      page_table[j][1] += 1;
    } 
     
    //print table
    print_table(page_table, table_size, i);
   
  }
  cout << "Fault count: " << fault_count << endl;


}
/************************************************888
 * function LFU_sim
 *
 * input page_list -  list of pages
 *       pages - number of pages
 *       page_table - table of pages
 *       table_size - the size of the table
 *
 * purpose: to simulate lfu simulation
 *
 * **********************************************/
void LFU_sim(vector<int> &page_list, int pages, int **&page_table, int table_size)
{
  int table_spot = 0;
  int fault_count = 0; 
  bool fault;
  int lfu;

  // initilise page table
  for(int i = 0 ; i < table_size ; i++ )
  {
    page_table[i][1] = 1;
  }
  //run through list 
  for(int i = 0; i < pages ; i++)
  {
    fault = true;
   //check to see if in array
    
    lfu = i;
    for(int j = 0; j < table_size; j++)
    {
      if(page_list[i] == page_table[j][0] )
      {
        fault = false;
      }
    }
 
    // if not in array
    if(fault == true)
    {
      if(i > 2)
      {
       //find longest 
        lfu = i;
        for(int k = 0; k < table_size ; k++)
        {
          if(page_table[k][1] <= lfu )
          {
            table_spot = k;
          }
           
        }
      } 
      page_table[table_spot][0] = page_list[i];
      page_table[table_spot][1] = 0;
      table_spot++;
      fault_count ++;
    }
  
    //increment all 
    for(int j = 0 ; j < table_size ; j++)
    {
      page_table[j][1] += 1;
    } 
     
    //print table
    print_table(page_table, table_size, i);
   
  }
  cout << "Fault count: " << fault_count << endl;



}

void SC_sim(vector<int> &page_list, int pages)
{
  cout << "Second chance does not work" << endl;
}

void c_sim(vector<int> &page_list, int pages)
{
  cout << "clock simulator does not work" << endl;
}

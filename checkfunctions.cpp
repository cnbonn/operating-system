#include <iostream>
#include <string>

//fucntion prototypes
//int valid_num();
bool reuse_list(int type);

int valid_num()
{
  int num;
  bool valid = false;

  while(!valid)
  {
    cout << " > ";
    cin >> num;

    if(!cin.fail() && (cin.peek()==EOF || cin.peek()=='\n'))
    {
      valid = true;
    }
    else
    {
      cin.clear();
      cin.ignore(256,'\n');
      cout << "Please enter a valid number" << endl;
    }
  } 

  return num;
}
/***********************************************
 * function: reuse_list
 *
 * input: type
 *
 * purpose: check if reuse previous
 *
 * output: false - use new 
 *         true - use same
 *         **********************************/

bool reuse_list(int type)
{
  string choice;
  cout << "Would you like to use the previous ";

  // choice type
  if( type == 1 )
  {
    cout << "Processes? (y/n)" << endl;
  }
  if( type == 2 )
  {
    
  }
  if( type == 3 )
  {
    cout << "page list? (y/n)" << endl;
  }
 
  // stay tell valid choice
  while(1)
  {
    cout << " Choice: ";
    cin >> choice;
    choice[0] = tolower(choice[0]);

    if(choice.compare("y") == 0)
    {
      return true;
    }
    else if(choice.compare("n") == 0)
    {
      return false;
    } 
    else
      cout << "Please enter (y/n)" << endl;
  }
  
}


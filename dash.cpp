//*********************************dash2*************************************//
//Program:     dash2                                                         //
//Author:      Erik S. Lunde                                                 //
//Instructor:  C. Karlsson                                                   //
//Course:      CSC456 Operating Systems                                      //
//Description: Runs specific commands for retrieving system information and  //
//             process information. (See below).                             //
//             -help                                                         //
//                Displays help menu.                                        //
//             -cmdmn <PID#>                                                 //
//                Finds the command string for a given process ID number.    //
//             -PID <command string/substring>                               //
//                Finds all running processes using the supplied string in   //
//                their command line prompt.                                 //
//             -systat:                                                      //
//                Displays System information including: Process Information,//
//                Linux version information, system uptime, storage          //
//                information, CPU information.                              //
//             -exit:                                                        //
//                Exits the program.                                         //
//                                                                           //
//New Features in 2:                                                         //
//             -cd <path>                                                    //
//               Change current working directory. Displays current and new. //
//             -<Unix Command>                                               //
//                Attempts to issue unix command, default command if no      //
//                recognized command is found.                               //
//             -<Unix Command> > <filename>                                  //
//                Issue Unix command and read given file name as user input. //
//             -<Unix Command> < <filename>                                  //
//                Issue Unix command and write output to given file name.    //
//             -signal <signal_num> <pid>                                    //
//                Sends a signal of signal_num to pid. Process will have to  //
//                catch and display all received signals.                    //
//PIPELINES - Unimplemented.
//
//New Features in 3:
//
//                                                                           //
//Author Notes: The algorithm library could easily be removed. Is only used  //
//  for the transform function. A lazy way to convert input to lower case    //
//  quickly without the use of a loop and tolower() function calls.          //
//                                                                           //  
//Planned Features:                                                          //
//Command Line Arguments: The program currently ignores command line args.   //
//  all commands must be entered while program is running.                   //
//Known Bugs:
//***************************************************************************//

//Pre-processor declarations//
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
//Version 2 Libraries
#include <unistd.h>
#include <sys/resource.h>
#include <sys/time.h>

using namespace std;

//Global Variable Declaration
const int shmkey = 1066;
//Class Structures
struct id{ int shm; int sem; };

//*********************//
//Function Declarations//
//*********************//
int main_loop( int argc , char* argv[], fstream &f );

void help_menu();

//Parse Functions
int parse( string &input );
bool parse_pid( string &input , fstream &f );
bool parse_cmdmn( int cmdmn , fstream &f );
int parse_system( string input );

//Systat Function
int systat();

//Shared Memory [External File]
extern id create_shm( int shmkey , int mail_count , int mail_size ,
                      void* &address );
extern void del_shm( id id , void *addr );
extern istream & write_shm( istream &in , void *address , int box , int size );
extern ostream & read_shm( ostream &out , void *address , int box , int size );
extern void copy_shm( void *address , int source , int dest , int size );

void mbox_help();


//*********************************MAIN**************************************//
//main( int argc, char* argv[] )                                             //
//Purpose:   Begins the program. Calls main loop and terminates when finished//
//Arguments: int argc: Number of command line arguments at run time.         //
//           char* argv[]: Command line arguments specified at run time.     //
//Returns:   0: No errors                                                    //
//           1: Parsing of user input failed, SHOULD NOT HAPPEN.             //
//           2: Main Loop function broke, SHOULD NOT HAPPEN.                 //
//           3: Unable to open log file. Program terminated.                 //
//           4: Unable to open system files. Program terminated.             //
//***************************************************************************//
int main( int argc, char* argv[] )
{
    //Local Variable Declaration
    int error_code = 0;         //Error code generated
    ofstream create;            //Create log file if it does not exist
    fstream f;                  //File for log reading/writing

    //Create log file if it does not already exist
    create.open( "dash_log.txt" );
    create.close();

    //Open log file for reading/writing
    f.open( "dash_log.txt" , ios::in|ios::out );


    //Check if opened correctly
    if( !f.is_open() )
    {
        cout << "Unable to open log files. Terminating." << endl;
        return 3;
    }

    //parse command line - Never implemented

    //Run Main Loop
    error_code = main_loop( argc , argv , f );
    f.close();

    return error_code;   
}

//***************************************************************************//
//main_loop( int argc , char* argv[] )                                       //
//Purpose:   Runs the main program loop. Receives user commands until told to//
//           exit program.                                                   //
//Arguments: int argc: Number of command line arguments at run time.         //
//           char* argv[]: Command line arguments specified at run time.     //
//           fstream &f: Log file stream stores system data and last run data//
//Returns:   Will return 0 if no errors detected. See error listing below.   //
//Error Codes:                                                               //
//  1:Input parse function failed or command variable corrupted.             //
//  2:Loop broke without exit command. Should never occur.                   //
//Author Notes: Both arguments are not currently used as program is not      //
//  currently parsing command line arguments, only accepts input at runtime. //
//***************************************************************************//
int main_loop( int argc , char* argv[], fstream &f )
{
    //Local Variable Declaration
    int command = -5;           //Numeric value for user issued command, 1-13
    string input = "";          //String used for reading input
    string buffer = "";         //Buffer string for System calls
    int CMDMN = -1;             //CMDMN requested Process ID number
    string pid = "";            //PID requested string
    int pid_count = -1;         //Number of process matching PID supplied name
    char cwd[256];              //Pointer to Current Working Directory
    int pos = 0;                //Index into input when parsing system commands
    rusage usage;               //Struct to store usage
    rusage old_usage;           //Struct to store previous usage data
                                //Previous children existed.
    //Shared Memory Variable Declaration
    id id;                      //Holds shmid and semid(Shared Mem & Semaphore)
    id.shm = 0; id.sem = 0;     
    int mail_count = 0;         //# of mailboxes to set up (user specified)
    int mail_size = 0;          //Size of each mailbox in bytes(user specified)
    void *addr = NULL;          //Pointer to shared memory address
    int box = 0;                //User specified box to read/write in
    int i = 0;                  //Loop counter for shmid creation
    int source = 0 , dest = 0;  //Copy Mailbox variables
    
    

    //Initiate Program Loop
    do
    {
        cout << "dash>";
        //Read User Input
        cin >> noskipws >> input >> skipws;

        //Parse input to base command.
        command = parse( input );

        switch( command )
        {
            case 0: //All white space command specified
                cin.ignore( 1024 , '\n' );
                cin.clear();
                input.clear();
                break;

            case 1: //Exit Command
                cout << "Exit command received. Terminating." << endl;
                if( addr )
                {
                    del_shm( id , addr );
                }
                return 0;
                break;

            case 2: //PID
                //cout << "PID command received." << endl; //DEBUG
                system( "ps aux --sort pid > dash_log.txt");
                cin >> input;
                if( !parse_pid( input , f ) )
                {
                    cout << "Unable to locate PID for command string: "
                         << input << endl;
                }//If match found, parse_pid will output results
                break;

            case 3: //systat
                if( systat() )
                {
                    cout << "Unable to access System Files for systat call."
                         << " Terminating." << endl;
                    return 4; 
                }
                break;

            case 4: //CMDMN
                //cout << "CMDMN command received!" << endl; //DEBUG
                system( "ps aux --sort pid > dash_log.txt");
                if( !( cin >> CMDMN ) ) //check user entered valid integer
                {
                    cout << "Please use only numbers with CMDMN command."
                         << endl << "CMDMN command canceled." << endl;
                }

                //Search for indicated PID
                if( !parse_cmdmn( CMDMN , f ) ) //No matching PID found
                {
                    cout << "Unable to locate command for PID: " << CMDMN
                         << endl;
                }//If match found, parse_cmdmn will output results
                break;

            case 5: //Help Command
                help_menu();
                break;

//*******************************VERSION 2!**********************************//
            case 6: //signal <signal_num> <pid> Command
                //signal
                cout << endl << "This is a signal command test." << endl <<
                "Not yet implemented." << endl;
                break;

            case 7: //cd Command
                getcwd( cwd , 256 );
                cout << "Current: " << cwd << endl;
                cin >> input;
                chdir( &input[0] );
                getcwd( cwd , 256 );
                cout << "New:     " << cwd << endl;
                break;

            case 8: //Piped System Command
                //Get rest of system command
                getline( cin , buffer );
                input = input + ' ' + buffer;
                system( &input[0] );
                
                /*
                //Check for File redirect or regular command
                pos = parse_system( input );

                //Create Child Process
                create_child( input , pos );
                old_usage = usage;
                getrusage( RUSAGE_CHILDREN , &usage );
                cout << "Time: " << old_usage*/

                break;

//*******************************VERSION 3!**********************************//
            case 9:  //mailbox initialize
                cin >> input;
                mail_count = atoi( input.c_str() );
                cin >> input;
                mail_size = atoi( input.c_str() );

                //User entered non-integer value for # of boxes or size, BREAK
                if( mail_count == 0 || mail_size == 0 )
                {
                    cout << "Invalid Mailbox parameters." << endl
                         << "mboxinit <# of boxes> <# of kilobytes per box>"
                         << endl;
                    break;
                }

                //Only one shared memory may be created at a time.
                /*if( id.shm )
                {
                    cout << "Mailboxes already initialized. Command Ignored." 
                         << endl;
                    break;
                }*/

                //Create shared memory based on current process ID, increment
                //until valid shared space is created.
                /* Set shared memory key 1066 used instead, DEPRECATED
                for( i = 0;
                     create_shm( getpid()+i , mail_size*(mail_count+1) , addr );
                     i++ )
                { cout <<"loop test" << endl; }
                shmid = getpid()+i;
                */
                id = create_shm( shmkey , mail_count , mail_size , addr );
                break;

            case 10: //mailbox delete
                //Delete Semaphores
                //Delete Shared Memory
                if( id.shm != 0 )
                    del_shm( id , addr );
                else
                    cout << "No Shared Memory has been set up yet!" << endl;
                id.shm = 0;
                addr = NULL;
                break;

            case 11: //mailbox write
                cout << "Mailbox write command received. TESTING" << endl;
                cin >> input;
                box = atoi( input.c_str() );

                if ( id.shm == 0 )
                {
                    cout << "No Shared Memory has been set up yet!" << endl;
                    break;
                }
                if( box && box <= mail_count )
                    write_shm( cin , addr , box-1 , mail_size );
                else
                    cout << "Mailbox number specified is invalid." << endl;
                box = 0;
                break;

            case 12: //mailbox read
                cin >> input;
                box = atoi( input.c_str() );

                if ( id.shm == 0 )
                {
                    cout << "No Shared Memory has been set up yet!" << endl;
                    break;
                }
                if( box && box <= mail_count )
                    read_shm( cout , addr , box-1 , mail_size );
                else
                    cout << "Mailbox number specified is invalid." << endl;
                break;

            case 13: //mailbox copy
                cout << "Mailbox copy command received. TESTING" << endl;
                cin >> input;
                source = atoi( input.c_str() );
                cin >> input;
                dest = atoi( input.c_str() );

                /*if( !addr )
                {
                    addr = shmat( id.shm , 0 , 0 );
                }*/
                if( source && dest &&
                    source <= mail_count && dest <= mail_count )
                    copy_shm( addr , source-1 , dest-1 , mail_size );
                else
                    cout << "One or more mailboxes specified is invalid."
                         << endl;
                source = 0; dest = 0;
                break;

            case 14: //mailbox help
                mbox_help();
                break;
//***************************************************************************//


            default:
                cerr << "UNEXPECTED INPUT PARSE ERROR. ERROR CODE 1." << endl;
                return 1;
                break;

        }

        //Clear input stream and ignore extra keystrokes after command executed
        cin.ignore( 1024 , '\n' );
        cin.clear();
        input = "";

    }while( command != 1 ); //While command is not 'exit' continue looping

    //Should never occur, Error Code 2
    cout << "UNEXPECTED ERROR. MAIN LOOP BROKE. EXITING, ERROR CODE 2." 
         << endl;
    return 2;
}

//***************************************************************************//
//parse( string &input )                                                     //
//Purpose:   Parse user input string to one of four commands.                //
//Arguments: string &input: User input string to parse for valid command.    //
//Returns:  0: Command specified was all whitespace, ignore, redisplay prompt//
//          1: Exit command received. Terminate program on return.           //
//          2: PID command received. Parse for name on return.               //
//          3: systat command received. Output data on return.               //
//          4: CMDMN command received. Parse for ID on return.               //
//          5: help command received. Output help menu on return.            //
//          6: User entered another command, run as system command in pipe.  //
//Author Notes: Will only accept commands at the start of the input string   //
//  which match exactly to specific commands. Does not parse additional args.//
//***************************************************************************//
int parse( string &input )
{
    //Convert input to lower case
    transform( input.begin() , input.end() , input.begin() , ::tolower );

    //Determine if valid command
    if( input == "" )
    {
        return 0; //All-Whitespace command entered
    }
    else if( !input.compare( 0 , 4 , "exit" ) )
    {
        return 1; //Exit Command
    }
    else if( !input.compare( 0 , 4 , "pid" ) )
    {
        return 2; //PID command
    }
    else if( !input.compare( 0 , 6 , "systat" ) )
    {
        return 3; //systat command
    }
    else if( !input.compare( 0 , 6 , "cmdmn" ) )
    {
        return 4; //CMDMN command
    }
    else if( !input.compare( 0 , 4 , "help" ) )
    {
        return 5; //Help command
    }

    //Version 2 Commands
    else if( !input.compare( 0 , 6 , "signal" ) )
    {
        return 6; //signal command
    }
    else if( !input.compare( 0 , 2 , "cd" ) )
    {
        return 7; //CD command
    }
    else if( false ) //PIPING NOT IMPLEMENTED, still executes system command
        return 8; //Pipe system command

    //Version 3 Commands
    else if( !input.compare( 0 , 4 , "mbox" ) )
    {
        if( !input.compare( 4 , 4 , "init" ) )
            return 9;
        else if( !input.compare( 4 , 3 , "del" ) )
            return 10;
        else if( !input.compare( 4 , 5 , "write" ) )
            return 11;
        else if( !input.compare( 4 , 4 , "read" ) )
            return 12;
        else if( !input.compare( 4 , 4 , "copy" ) )
            return 13;
        else
            return 14;
    }

    //No Valid command found, SHOULD NOT OCCUR
    return 256;
}

//***************************************************************************//
//bool parse_cmdmn( int CMDMN , fstream &f )                                 //
//Purpose:   Search for a command matching the specific process ID number.   //
//Arguments: int CMDMN: Process ID to find command string of.                //
//           fstream &f: File to parse looking for active process ID.        //
//Returns:   True: Matching process ID found.                                //
//           False: No Process ID matches.                                   //
//***************************************************************************//
bool parse_cmdmn( int CMDMN , fstream &f )
{
    //Local Variable Declaration
    string seek;
    int file_end;

    //Reset position in file containing PID's
    f.seekg( 0 , f.end );
    file_end = f.tellg();
    f.seekg( 0 , f.beg );

    //Search PS system command output
    do{
        f >> seek >> seek; //Read to second entry, PID #
        if( CMDMN == atoi( seek.c_str() ) )  //Correct PID located
        {
            for( int i = 0; i <= 7; i++ )
            {
                f >> seek;
            }
            getline( f , seek );        //Getline used due to whitespace
                                        //found in command string
            cout << "Command String: " << seek << endl;
            return true;
        }
        for( int i = 0; i <= 7; i++ )//loop through lest of current process
        {
            f >> seek;
        }
        getline( f , seek );
    }while( f.tellg() < file_end  ); //default false, only true if match found
    
    return false;
}

//***************************************************************************//
//bool parse_pid( string &input , fstream &f )                               //
//Purpose:   Parse a PID request into all applicabable commands with the     //
//           string, or sub-string in PID command.                           //
//Arguments: string &input: string to locate in command.                     //
//           fstream &f: file to log file to search in.                      //
//Returns:   True: Matching command string located.                          //
//           False: No matching command string located.                      //
//***************************************************************************//
bool parse_pid( string &input , fstream &f )
{
    //Local Variable Declaration
    string seek;
    int file_end;
    int pids[1000] = { 0 };
    int temp = -1;
    bool located = false;

    //Reset position in file containing PID's
    f.seekg( 0 , f.end );
    file_end = f.tellg();
    f.seekg( 0 , f.beg );

    //Search PS system command output
    do{
        f >> seek >> seek; //Read to second entry, PID #, save PID#
        temp = atoi( seek.c_str() );
        
        for( int i = 0; i <= 7; i++ ) //Loop through data
        {
            f >> seek;
        }
        getline( f , seek );            //Getline used due to whitespace
                                        //found in command string
        //seek should now contain the command string we want to check

        if( seek.find( input , 0 ) != -1 )  //Correct PID located, get command
        {
            cout << "PID# matching command string " << input
                 << ": " << temp << endl;
            located = true;
        }
    }while( f.tellg() < file_end  ); //While not end of file, continue

    return located; //default false, only true if match found
}

//***************************************************************************//
//int parse_system( string input )                                           //
//Purpose:   Parse a user entered system command input for file redirect or  //
//           standard execution.                                             //
//Arguments: string &input: string to parse for <, > and file name if present//
//Returns:   0: No file redirect, presumed standard Unix Command, signals the//
//              calling function to attempt a normal system call.            //
//           Non-Zero: Position in input string of <, or > character         //
//              indicating a file redirect is required.                      //
//***************************************************************************//
int parse_system( string input )
{
    //Local Variable Declaration
    int pos = -1;               //Temporary Variable for return value

    pos = input.find_first_of( "<>" );
    if( pos == -1 )
        return 0;
    else
        return pos;
}

//***************************************************************************//
//int create_child
//Purpose:   
//Arguments: 
//Returns:   
//***************************************************************************//
/*
create_child()
{
}*/

//***************************************************************************//
//int systat()                                                               //
//Purpose:   Output system info including OS version, CPU, and memory info.  //
//Arguments: None.                                                           //
//Returns:   0: No errors.                                                   //
//           1: Failed to open version file. Terminate program.              //
//           2: Failed to open cpuinfo file. Terminate program.              //
//           3: Failed to open meminfo file. Terminate program.              //
//***************************************************************************//
int systat()
{
    //Local Variable Declaration
    ifstream f;
    string input;

    //Open version file for reading
    f.open( "/proc/version" );

    //Check if opened correctly
    if( !f.is_open() )
    {
        cout << "Unable to open version files. Terminating." << endl;
        return 1;
    }

    //Get version info and output to console
    getline( f , input );
    cout << endl << input << endl;

    //Close version file.
    f.close();

    //Open cpuinfo file for reading
    f.open( "/proc/cpuinfo" );

    //Check if opened correctly
    if( !f.is_open() )
    {
        cout << "Unable to open cpuinfo files. Terminating." << endl;
        return 2;
    }

    //Get CPU info and output to console
    cout << endl << "CPU Info: " << endl;
    for( int i = 0; i <= 8; i++ )
    {
        getline( f , input );
        cout << input << endl;
    }

    //Close CPU info file
    f.close();

    //Open cpuinfo file for reading
    f.open( "/proc/meminfo" );

    //Check if opened correctly
    if( !f.is_open() )
    {
        cout << "Unable to open meminfo files. Terminating." << endl;
        return 3;
    }

    //Get Memory info info and output to console
    cout << endl << "Memory Info: " << endl;
    for( int i = 0; i <= 6; i++ )
    {
        getline( f , input );
        cout << input << endl;
    }

    //Close Memory info file
    f.close();

    return 0;
}

//***************************************************************************//
//help_menu()                                                                //
//Purpose:   Prints out a help menu to the user upon command: "help"         //
//Arguments: None.                                                           //
//Returns:   Void.                                                           //
//***************************************************************************//
void help_menu()
{
    //Output help menu
    cout << endl;
    cout << "List of commands: " << endl;
    cout << "cmdmn <pid>" << endl <<
    "    Return the command string (name) that started the process for PID."
    << endl;
    cout << "pid <name>" << endl <<
    "    Return the process ids for given command string (matches substrings)"
    << endl;
    cout << "systat" << endl <<
    "    Prints out some process information." << endl;
    cout << "exit" << endl <<
    "    Exit the program. Good bye!" << endl;
    cout << endl << "List of Version 2 commands: " << endl;
    cout << "<Unix Command>" << endl <<
    "    Attempts to issue the unix command provided in pipe. Unrecognized " <<
    endl << "input defaults to this command." << endl;
    cout << "<Unix Command> < <Filename>" << endl <<
    "    Issue the unix command specified in pipe, redirecting output to file"
    << endl;
    cout << "<Unix Command> > <Filename>" << endl <<
    "    Issue the unix command specified in pipe, redirecting input from file"
    << endl;
    cout << "cd <path>" << endl <<
    "    Attempts to change the current working directory." << endl;
    cout << "signal <signal_num> <pid>" << endl <<
    "    Sends signal of <signal_num>, to process ID <pid>" << endl;

    //Other Help Sections
    mbox_help();
}

//***************************************************************************//
//mbox_help()                                                                //
//Purpose:   
//Arguments: None.                                                           //
//Returns:   Void.                                                           //
//***************************************************************************//
void mbox_help()
{
    cout << "FILL THIS MENU IN!" << endl;
}

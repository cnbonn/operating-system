//Pre-processor Directives
#include <iostream>
#include <sys/ipc.h>     
#include <sys/shm.h>
#include <errno.h>

#include <sys/sem.h>
#include <sys/types.h>

//Class Structures
union semun
{
    int val;                   //value for SETVAL
    struct semid_ds *buf;      //buffer for IPC_STAT, IPC_SET
    unsigned short int *array; //array for GETALL, SETALL
    struct seminfo *__buf;     //buffer for IPC_INFO
};

struct id{ int shm; int sem; };

//Accessing Standard Namespace
using namespace std;

//Function Prototypes
id create_shm( int shmkey , int mail_count , int mail_size , void* &address );
void del_shm( id id , void *addr );
istream & write_shm( istream &in , void *address , int box , int size );
ostream & read_shm( ostream &out , void *address , int box , int size );
void copy_shm( void *address , int source , int dest , int size );

//Semaphores
int sema_create( int boxes );
int sema_del( int smid );
/*
int sema_state();
int sema_set();
int sema_clear();
*/

//***************************************************************************//
//create_shm                                                                 //
//Purpose:   
//Arguments: 
//Returns:   
//***************************************************************************//
id create_shm( int shmkey , int mail_count , int mail_size , void* &address )
{
    //Local Variable Declaration
    id id;

    id.sem = sema_create( mail_count );

    if( id.sem == 0 )
    {
        cout << "Semaphores did not create correctly." << endl;
        id.shm = 0; id.sem = 0;
        return id;
    }
    

    //Using SHMKEY, create one shared memory region with permissions 0666:
    //Returns a size in kilobytes
    id.shm = shmget( shmkey , mail_count*mail_size * 1024 ,
                    IPC_CREAT|IPC_EXCL|0666);

    //Shared Memory creation failed, return 1 for failure
    if( id.shm < 0 )
    {
        cout << "Shared Memory segment failed to instantiate with key 1066."
             << endl;
        id.shm = 0;
        return id;
    }

    cout << "Shared Memory Created." << endl;    
    cout << "Shared Memory ID: " << id.shm << endl;
    cout << "Shared Memory Size: " << mail_count*mail_size * 1024
         << " bytes" << endl;

    //Attaching shared memory to calling process
    address =  shmat( id.shm , 0 , 0 );
    cout << "Address: 0x" << address << endl;
    
    //Return Success
    return id;
}

//***************************************************************************//
//del_shm                                                                    //
//Purpose:   
//Arguments: 
//Returns:   
//***************************************************************************//
void del_shm( id id , void *addr )
{
    shmdt( addr );
    shmctl( id.shm , IPC_RMID , 0 );
    sema_del( id.sem );
}

//***************************************************************************//
//write_shm                                                                  //
//Purpose:   Write data from input stream to a previously established shared //
//      memory location. Memory is in the form of a void pointer to start of //
//      location, and is seperated into mail boxes of size * kilobytes.      // 
//Arguments: istream &in: Input stream to read data from.                    //
//           void *addr: Void pointer to start of shared memory address.     //
//           int box: The mail box number in shared memory we are writing to.//
//           int size: The size (in kilobytes) of each mailbox.              //
//Returns: ifstream &: Returns the input file stream for continued reading if//
//      it is desired.                                                       //   
//***************************************************************************//
istream & write_shm( istream &in , void *address , int box , int size )
{
    //Local Variable Declaration
    string buffer;
    string data;
    bool trunc = false;
    size = size * 1024;
    int i = 0;
    char *addr = (char*)address;

    //Read into buffer until end of input or capacity reached
    while( getline( in , buffer ) )
    {
        if( in.eof() )
        {
            break;
        }
        data = data+buffer+'\n';

        if( data.length() >= size )
        {
            cout << "Mailbox limit reached. Truncating data." << endl;
            trunc = true;
            break;  //Write limit reached, break out and write buffer to memory
        }
    }

    //Remove End of File Flags for the input stream
    in.clear();

    //CHECK SEMAPHORES
    //Write the data to shared memory.
    for( i = 0; i < data.length(); i++ )
    {
        addr[ (box*size) + i ] = data[i];   
    }

    //NULL terminate data if we did not reach mailbox capacity
    if( !trunc )
        addr[ (box*size) + i+1 ] = '\0';

    //Return input stream for continued reading if desired
    return in;
}

//***************************************************************************//
//read_shm
//Purpose:   
//Arguments: 
//Returns:   
//***************************************************************************//
ostream & read_shm( ostream &out , void *address , int box , int size )
{
    //Local Variable Declaration
    int i = 0;
    size = size * 1024;
    char *addr = (char*)address;

    //Output contents of desired mailbox to specified output stream until end
    //of mailbox or null terminator hit.
    while( i < size && ( addr[(box*size) + i] != '\0' ) )
    {
        out << addr[((box)*size) + i];
        i++;
    }
    cout << endl;

    return out;
}

//***************************************************************************//
//copy_shm                                                                   //
//Purpose:   Copies the entire contents of one mailbox to another.           //  
//Arguments: void *addr: Pointer to start of shared memory.                  //
//           int source: Source (copying) mail box number.                   //
//           int dest:   Destination (copying to) mailbox number.            //
//           int size:   The size of each mailbox (in kilobytes).            //
//Returns:   Void. Function assumes caller gave it valid indexes for boxes   //
//           and correct size.                                               //
//***************************************************************************//
void copy_shm( void *address , int source , int dest , int size )
{
    //Local Variable Declaration
    char *addr = (char*)address;

    for( int i = 0; i < size * 1024; i++ )
    {
        addr[(dest)*1024 + i] = addr[(source)*1024 + i];
    }
}

//***************************************************************************//
//
//Purpose:   
//Arguments: 
//Returns:   
//***************************************************************************//
int sema_create( int boxes )
{
    //Local Variable Declaration
    int semid;
    int i;
    int opid;
    union semun options;

    //Semget using IPC_Private allows computer to pick a key to use to give
    //us the desired number (=boxes) of semaphores. Using flags CREATE
    //and EXCLUSIVE and permissions 0666
    semid = semget( IPC_PRIVATE , boxes , IPC_CREAT|IPC_EXCL|0666 );

    if( semid == -1 )
    {
        cout << "Unable to create semaphores. Critical Error. Exiting."
             << endl;
        return 0;
    }

    // Initialize the semaphore at index 0
    options.val = 1;
    for( int k = 0; k < boxes; k++ )
    {
        if( semctl( semid , k , GETVAL , options ) )
        {
            cout << "Cannot lock semaphore." << endl;
            return 0;
        }
        cout << "Value of semaphore at index " << k
             << " is: " << semctl( semid , k , GETVAL , 0 ) << endl;
    }

    return semid;
}

int sema_del( int semid )
{
    semctl( semid, 0, IPC_RMID, 0);
}
/*int sema_state()
{
}
int sema_set( int type )
{
}
int sema_clear()
{
}*/


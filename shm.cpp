//Pre-processor Directives
#include <iostream>
#include <sys/ipc.h>     
#include <sys/shm.h>
#include <errno.h>

#include <sys/sem.h>
#include <sys/types.h>

//Class Structures
union semun //Used for Semaphores
{
    int val;                   //value for SETVAL
    struct semid_ds *buf;      //buffer for IPC_STAT, IPC_SET
    unsigned short int *array; //array for GETALL, SETALL
    struct seminfo *__buf;     //buffer for IPC_INFO
};

struct id{ int shm; int sem; }; //Tracks shared memory and Semaphore ID's

//Accessing Standard Namespace
using namespace std;

//Function Prototypes//
//Shared Memory
id create_shm( int shmkey , int mail_count , int mail_size , void* &address );
void del_shm( id id , void *addr );
istream & write_shm( istream &in , void *address , int box ,
                     int size , id &id );
ostream & read_shm( ostream &out , void *address , int box ,
                    int size , id &id );
void copy_shm( void *address , int source , int dest , int size , id &id );
int attach_nonlocalshm( int shmkey , void* &address , id &id ,
                        int &mail_count , int &mail_size );

//Semaphores
int sema_create( int shmkey , int boxes );
int sema_clear( id &id , int box);
int sema_set( id &id , int box);
void sema_del( int smid );

//***************************************************************************//
//create_shm                                                                 //
//Purpose:   Sets up the shared memory segment based on set parameters.      //
//  Allocates a space of mail_count(#of boxes) * mail_size(size of each box) //
//  The size of each box is in KB. May be passed a shmkey to use for set up. //
//  Currently dash.cpp always passes shmkey = 1066                           // 
//Arguments: int shmkey: Can be anything, currently only ever passed 1066    //
//           int mail_count: # of mailboxes to create                        //
//           int mail_size: Size of each mailbox in KB                       //
//           void& &address: Void Point to start of shared memory location.  //
//Returns:   id: struct containing:                                          //
//              id.shm: Shared Memory ID        AND     id.sem: Semaphore ID //
//References: void* &address: Passed back to calling function for later.     //
//Known Errors: None.                                                        //
//***************************************************************************//
id create_shm( int shmkey , int mail_count , int mail_size , void* &address )
{
    //Local Variable Declaration
    id id;                      //Stores Semaphore and Shared Memory ID #s

    //Set up Semaphores first
    id.sem = sema_create( shmkey , mail_count );

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
//Purpose:   Marks the shared memory segment and semaphores for desctruction.//  
//Arguments: id id: Structure containing the ID for shared memory and semas. //
//           void *addr: The void pointer to the start of the shared memory. //
//Returns:   VOID.                                                           //
//***************************************************************************//
void del_shm( id &id , void *addr )
{
    shmdt( addr );                   //Detach the current process from shared
    shmctl( id.shm , IPC_RMID , 0 ); //Marks shared memory segment for deletion
    sema_del( id.sem );              //Deletes the Semaphores
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
//           id &id: struct containing shared memory and semaphore IDs.      //
//Returns: ifstream &: Returns the input file stream for continued reading if//
//      it is desired.                                                       //   
//***************************************************************************//
istream & write_shm( istream &in , void *address , int box , int size , id &id )
{
    //Local Variable Declaration
    string buffer;
    string data;
    bool trunc = false;
    size = size * 1024;
    int i = 0;
    char *addr = (char*)address;

    //Check the state of the semaphore
    if ( semctl( id.sem , box , GETVAL , 0 ) ) 
    {
        cout << "ERROR: Could not lock Semaphore " << box 
             << " for mailbox write command. Write Terminated" << endl;
        return in;
    }

    //Set semaphore
    if( sema_set( id , box ) )
    {
        cout << "Failed to set semaphore for box: " << box << endl
             << "Write Terminated." << endl;
    }

    //Clear the first enter for command from input stream
    in.ignore( 1 );

    //Read into buffer until end of input or capacity reached
    while( in.good() )
    {
        getline( in , buffer );
        if( buffer.length() == 0 )
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

    //Write the data to shared memory.
    for( i = 0; i < data.length(); i++ )
    {
        addr[ (box*size) + i ] = data[i];   
    }

    //NULL terminate data if we did not reach mailbox capacity
    if( !trunc )
        addr[ (box*size) + i+1 ] = '\0';
    if( data[data.length()-1] == '\n' )
        addr[ (box*size)+data.length()-1 ] = '\0';

    //Clear Semaphore
    if( sema_clear( id , box ) )
    {
        cout << "Failed to clear Semaphore for box: " << box << endl
             << "Critical Error." << endl;
        return in;
    }

    //Return input stream for continued reading if desired
    return in;
}

//***************************************************************************//
//read_shm                                                                   //
//Purpose:   Read the specified shared memory box until NULL/EOF/End of box. //
//Arguments: ostream &out: Output stream to display box contents to.         //
//           void *address: Pointer to start of shared memory location.      //
//           int box: Mailbox number to read.                                //
//           int size: Size of each mailbox in KB.                           //
//           id &id: struct containing shared memory and semaphore IDs.      //
//Returns:   ostream &: For continued use of out if desired.                 //
//***************************************************************************//
ostream & read_shm( ostream &out , void *address , int box , int size , id &id )
{
    //Local Variable Declaration
    int i = 0;
    size = size * 1024;
    char *addr = (char*)address;

    //Check the state of the semaphore
    if ( semctl( id.sem , box , GETVAL , 0 ) ) 
    {
        cout << "ERROR: Semaphore not unlocked for box: " << box
             << " Read Terminated." << endl;
        return out;
    }

    //Set semaphore
    if( sema_set( id , box ) )
    {
        cout << "Failed to lock semaphore for box: " << box << endl
             << "Read Terminated." << endl;
    }

    //Output contents of desired mailbox to specified output stream until end
    //of mailbox or null terminator hit.
    while( i < size && ( addr[(box*size) + i] != '\0' ) )
    {
        out << addr[((box)*size) + i];
        i++;
    }
    cout << endl;

    //Clear Semaphore
    if( sema_clear( id , box ) )
    {
        cout << "Failed to clear Semaphore for box: " << box << endl
             << "Critical Error." << endl;
        return out;
    }

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
void copy_shm( void *address , int source , int dest , int size , id &id )
{
    //Local Variable Declaration
    char *addr = (char*)address;

    //Check the state of the source semaphore
    if ( semctl( id.sem , source , GETVAL , 0 ) ) 
    {
        cout << "ERROR: Semaphore not unlocked for box: " << source << endl
             << " Copy Terminated" << endl;
        return;
    }

    //Set source semaphore
    if( sema_set( id , source ) )
    {
        cout << "Failed to lock semaphore for box: " << source << endl
             << "Copy Terminated." << endl;
        return;
    }

    //Check the state of the destination semaphore
    if ( semctl( id.sem , dest , GETVAL , 0 ) ) 
    {
        cout << "ERROR: Semaphore not unlocked for box: " << dest << endl
             << "Copy Terminated." << endl;
        return;
    }

    //Set destination semaphore
    if( sema_set( id , dest ) )
    {
        if( sema_clear( id , source ) )
        {
            cout << "Failed to clear Semaphore for box(source): "
                 << source << endl << "Critical Error." << endl;
        }
        cout << "Failed to lock semaphore for box: " << dest << endl
             << "Copy Terminated." << endl;
        return;
    }

    //Perform copy Operation
    for( int i = 0; i < size * 1024; i++ )
    {
        addr[(dest)*size*1024 + i] = addr[(source)*size*1024 + i];
    }

    //Clear Source Semaphore
    if( sema_clear( id , source ) )
    {
        cout << "Failed to clear Semaphore for box(source): " << source << endl
             << "Critical Error." << endl;
    }
    //Clear Destination Semaphore
    if( sema_clear( id , dest ) )
    {
        cout << "Failed to clear Semaphore for box(dest): " << dest << endl
             << "Critical Error." << endl;
    }
    return;
}

//***************************************************************************//
//attach_nonlocalshm                                                         //
//Purpose:   Attach to a shared memory segment with shmkey that this process //
//      did not personally create.                                           //
//Arguments: int shmkey: The key to use to search for a shared memory segment//
//           void * &address: Address of shared memory if found.             //
//           id &id: Struct containing shared mem and semaphore IDs if found.//
//           int &mail_count: Pass back the number of mailboxes found in seg //
//           int &mail_size: Pass back the size of mailboxes found in seg    //
//Returns:   int: -1: FAILURE, no shared memory segment or semaphores found. //
//                 0: SUCCESS, accessed and attached.                        //
//***************************************************************************//
int attach_nonlocalshm( int shmkey , void* &address , id &id ,
                        int &mail_count , int &mail_size )
{
    id.shm = shmget( shmkey , 0 , 0666);//Find memory segment
    if( id.shm == -1 )//No shared memory segment found
    {
        id.shm = 0; id.sem = 0;
        return -1;
    }
    address = shmat( id.shm , 0 , 0 );//Attach to segment
    mail_count = ((int*)address)[0];
    mail_size = ((int*)address)[1];

    id.sem = semget( shmkey , 0 , 0666 );//Find semaphores
    if( id.sem == -1 )
    {
        id.shm = 0; id.sem = 0;
        return -1;
    }

    return 0;
}

//***************************************************************************//
//sema_create                                                                //
//Purpose:   Create semaphores for all mailboxes in shared memory.           //
//Arguments: int shmkey: The shared memory key we are creating semaphore for.//
//           int boxes: The number of mailboxes that need a semaphore.       //
//Returns:   int: 0: Failed to create semaphores.                            //
//                #: The semaphore ID for created semaphores.                //
//***************************************************************************//
int sema_create( int shmkey , int boxes )
{
    //Local Variable Declaration
    int semid;
    int i;
    int opid;
    union semun options;

    //Semget using IPC_Private allows computer to pick a key to use to give
    //us the desired number (=boxes) of semaphores. Using flags CREATE
    //and EXCLUSIVE and permissions 0666
    semid = semget( shmkey , boxes , IPC_CREAT|IPC_EXCL|0666 );

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
            cout << "Cannot initialize semaphore." << endl;
            return 0;
        }
    }

    return semid;
}

//***************************************************************************//
//sema_del                                                                   //
//Purpose:   Mark the created semaphores for destruction.                    //
//Arguments: int semid: The semaphore ID to mark for destruction.            //
//Returns:   VOID.                                                           //
//***************************************************************************//
void sema_del( int semid )
{
    semctl( semid, 0, IPC_RMID, 0);
}

//***************************************************************************//
//sema_set                                                                   //
//Purpose:   Set the semaphore for the given box # so no one may access it.  //
//Arguments: id &id: Struct containing the shared memory and semaphore IDs.  //
//           int box: The semaphore for the box to set.                      //
//Returns:   int -1: FAILED! Could not lock semaphore.                       //
//           int 0:  SUCCESS, semaphore locked.                              //
//***************************************************************************//
int sema_set( id &id , int box )
{
    struct sembuf lock;
    int sem_value = 0;

    //Lock Semaphore
    lock.sem_num = box;         //semaphore index
    lock.sem_op = 1;            //the operation
    lock.sem_flg = IPC_NOWAIT;  //operation flags

    sem_value = semop( id.sem , &lock , 1 );//Lock

    if( sem_value == -1 )//Check success
    {
        cout << "Cannot lock Semaphore" << endl;
        return -1;
    }
    return 0;
}

//***************************************************************************//
//sema_clear                                                                 //
//Purpose:   Clear the semaphore for the given box # so it may be accessed.  //
//Arguments: id &id: Struct containing the shared memory and semaphore IDs.  //
//           int box: The semaphore for the box to clear.                    //
//Returns:   int -1: FAILED! Could not unlock semaphore.                     //
//           int 0:  SUCCESS, semaphore unlocked.                            //
//***************************************************************************//
int sema_clear( id &id , int box )
{
    struct sembuf lock; 
    int sem_value = 0;

    //Unlock Semaphore
    lock.sem_num = box;         //semaphore index
    lock.sem_op = -1;           //the operation
    lock.sem_flg = IPC_NOWAIT;  //operation flags

    sem_value = semop( id.sem , &lock , 1 );//Unlock

    if( sem_value == -1 )//Check Success
    {
        cout << "Cannot unlock Semaphore" << endl;
        return -1;
    }
    return 0;
}


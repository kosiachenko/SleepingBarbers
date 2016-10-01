#include <iostream>    // cout
#include <sys/time.h> // gettimeofday
#include <unistd.h> //usleep
#include "Shop.h"

using namespace std;

// function prototype
void *barber( void * );    // the prototype of the barber thread function
void *customer( void * );  // the prototype of the customer thread function

// a set of parameters to be passed to each thread
class ThreadParam {
public:
  ThreadParam( Shop *shop, int id, int serviceTime ) :
    shop( shop ), id( id ), serviceTime( serviceTime ) { };
  Shop *shop;               // a pointer to the Shop object
  int id;                   // a thread identifier
  int serviceTime;          // service time (in usec) to a barber, whereas 0 to a customer
};

int main( int argc, char *argv[] ) {

  // validate the arguments
  if ( argc != 5 ) {
    cerr << "usage: sleepingBarber nBaerbers nChairs nCustomers serviceTime" << endl;
    return -1;
  }
  int nBarbers = atoi( argv[1] );      // # barbers working in the barbershop
  int nChairs = atoi( argv[2] );       // # chairs available for customers to wait on
  int nCustomers = atoi( argv[3] );    // # customers who need a haircut service
  int serviceTime = atoi( argv[4] );   // each barber's service time ( in u seconds)

  pthread_t barber_thread[nBarbers];
  pthread_t customer_threads[nCustomers];
  Shop shop( nBarbers, nChairs );      // instantiate a barbershop
  
  for ( int i = 0; i < nBarbers; i++ ) {   // instantiate barbers
    ThreadParam *param = new ThreadParam( &shop, i, serviceTime );
    pthread_create( &barber_thread[i], NULL, barber, (void *)param );
  }
  for ( int i = 0; i < nCustomers; i++ ) { // instantiate customers
    usleep( rand( ) % 1000 );              // with random interval
    ThreadParam *param = new ThreadParam( &shop, i + 1, 0 );
    pthread_create( &customer_threads[i], NULL, customer, (void *)param );
  }

  for ( int i = 0; i < nCustomers; i++ )   // wait until all the customers are served
    pthread_join( customer_threads[i], NULL );

  for ( int i = 0; i < nBarbers; i++ )     // terminate all the barbers
    pthread_cancel( barber_thread[i] );
  cout << "# customers who didn't receive a service = " << shop.nDropsOff
       << endl;

  return 0;
}

// the barber thread function
void *barber( void *arg ) {

  // extract parameters
  ThreadParam &param = *(ThreadParam *)arg;
  Shop &shop = *(param.shop);
  int id = param.id;
  int serviceTime = param.serviceTime;
  delete &param;

  // keep working until being terminated by the main
  while( true ) {
    shop.helloCustomer( id );  // pick up a new customer
    usleep( serviceTime );     // spend a service time
    shop.byeCustomer( id );    // release the customer
  }
  return NULL;
}

// the customer thread function
void *customer( void *arg ) {

  // extract parameters
  ThreadParam &param = *(ThreadParam *)arg;
  Shop &shop = *(param.shop);
  int id = param.id;
  delete &param;

  int barber = -1;
  if ( ( barber = shop.visitShop( id ) ) != -1 ) // am I assigned to barber i or no barber (-1)?
    shop.leaveShop( id, barber );                // wait until my service is finished

  return NULL;
}

#include "Shop.h"

Shop::Shop( int nBarbers, int nChairs ) // initialize a Shop object with nBarbers and nChairs
{
    this -> nBarbers = nBarbers;
    this -> nChairs = nChairs;

    pthread_mutex_init(&mutex1, NULL);

    barbers = new Barber[nBarbers];
    for (int i = 0; i < nBarbers; i++)
    {
        barbers[i].id = i;
        pthread_cond_init(&barbers[i].barberCond, NULL);
    }
}

Shop::Shop( ) // initialize a Shop object with 1 barber and 3 chairs
{
    this -> nBarbers = DEFAULT_BARBERS;
    this -> nChairs = DEFAULT_CHAIRS;

    pthread_mutex_init(&mutex1, NULL);

    barbers = new Barber[nBarbers];
    for (int i = 0; i < nBarbers; i++)
    {
        barbers[i].id = i;
        pthread_cond_init(&barbers[i].barberCond, NULL);
    }
}

int Shop::visitShop( int customerId ) // return a non-negative number only when a customer got a service
{
    pthread_mutex_lock(&mutex1);
    if (((int)waitingCustomers.size() == nChairs)&&(sleepingBarbers.empty())) {

        printf("customer[%i]: leaves the shop because of no available waiting chairs. \n", customerId);
        nDropsOff++;
        pthread_mutex_unlock(&mutex1);
        return -1;
    }

    customers[customerId] = Customer();
    customers[customerId].id = customerId;
    pthread_cond_init(&customers[customerId].customerCond, NULL);
    int barberId;

    if (sleepingBarbers.empty()) {
        waitingCustomers.push(customerId);

        printf("customer[%i]]: takes a waiting chair. # waiting seats available = %i \n", 
                customerId, (int)(nChairs-waitingCustomers.size()));

        while (customers[customerId].myBarber == -1)
        {
            pthread_cond_wait(&customers[customerId].customerCond, &mutex1);
        }
        barberId = customers[customerId].myBarber;
    }
    else {    //there are sleeping barbers in a queue
        barberId = sleepingBarbers.front();
        sleepingBarbers.pop();
        customers[customerId].myBarber = barberId;
        getBarber(barberId) -> myCustomer = customerId;
    }

    printf("customer[%i]: moves to a service chair[%i], # waiting seats available = %i \n", 
            customerId, barberId, (int)(nChairs-waitingCustomers.size()));

    customers[customerId].state = CHAIR;
    pthread_cond_signal(&(getBarber(barberId) -> barberCond));
    pthread_mutex_unlock(&mutex1);
    return barberId;
}

void Shop::leaveShop( int customerId, int barberId )
{
    pthread_mutex_lock(&mutex1);
    printf("customer[%i]: wait for barber[%i] to be done with hair-cut.\n", customerId, barberId);

    while (customers[customerId].myBarber != -1)
    {
        pthread_cond_wait(&customers[customerId].customerCond, &mutex1);
    }

    printf("customer[%i]: says good-bye to barber[%i]\n", customerId, barberId);

    customers[customerId].state = LEAVING;
    pthread_cond_signal(&(getBarber(barberId) -> barberCond));

    pthread_mutex_unlock(&mutex1);
}

void Shop::helloCustomer( int barberId )
{
    pthread_mutex_lock(&mutex1);

    if (getBarber(barberId) -> myCustomer == -1)  {
        printf("barber  [%i]: sleeps because of no customers.\n", barberId);
        sleepingBarbers.push(barberId);
        while (getBarber(barberId) -> myCustomer == -1)
        {
            pthread_cond_wait(&(getBarber(barberId) -> barberCond), &mutex1);
        }
    }

    while (customers[getBarber(barberId) -> myCustomer].state != CHAIR) //synchronization with customer thread
    {
        pthread_cond_wait(&(getBarber(barberId) -> barberCond), &mutex1);
    }

    printf("barber  [%i]: starts a hair-cut service for customer[%i]\n", barberId, getBarber(barberId) -> myCustomer);

    pthread_mutex_unlock(&mutex1);
}

void Shop::byeCustomer( int barberId )
{
    pthread_mutex_lock(&mutex1);
    printf("barber  [%i]: says he's done with a hair-cut service for customer[%i]\n", barberId, getBarber(barberId) -> myCustomer);

    customers[getBarber(barberId) -> myCustomer].myBarber = -1;
    pthread_cond_signal(&customers[getBarber(barberId) -> myCustomer].customerCond);
    while (customers[getBarber(barberId) -> myCustomer].state != LEAVING) //synchronization with customer thread
    {
        pthread_cond_wait(&(getBarber(barberId) -> barberCond), &mutex1);
    }
    getBarber(barberId) -> myCustomer = -1;

    printf("barber  [%i]: calls in another customer.\n", barberId);
    if (!waitingCustomers.empty())
    {
        int customerId = waitingCustomers.front();
        waitingCustomers.pop();
        getBarber(barberId) -> myCustomer = customerId;
        customers[customerId].myBarber = barberId; //?
        pthread_cond_signal(&customers[customerId].customerCond);
    }

    pthread_mutex_unlock(&mutex1);
}

Shop::Barber* Shop::getBarber(int barberId)
{
    for (int i = 0; i< nBarbers; i++)
    {
        if (barbers[i].id == barberId)
        {
            return &barbers[i];
        }
    }
    return NULL;
}
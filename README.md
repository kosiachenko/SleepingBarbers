# SleepingBarbers
Multi-threaded C++ program simulating the work of a barbershop with multiple barbers and customers 
(separate thread for each barber and each customer).

<b>Purpose</b>

In this program, we extend the original so-called sleeping-barber problem to a
multiple sleeping barbers problem where many customers visit a barbershop and receive a haircut service
from any one available among barbers in the shop.

<b>Sleeping-Barbers Problem</b>

A barbershop consists of a waiting room with n chairs and a barber room with m barber chairs. If there are
no customers to be served, all the barbers go to sleep. If a customer enters the barbershop and all chairs
are occupied, then the customer leaves the shop. If all the barbers are busy but chairs are available, then
the customer sits in one of the free chairs. If the barbers are asleep, the customer wakes up one of the
barbers.

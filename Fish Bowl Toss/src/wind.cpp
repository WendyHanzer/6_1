// Includes
#include "wind.h"

// Constructor
Wind::Wind(int a, int b, int c, btVector3 d)
   {
	minStr = a;
	maxStr = b;
	roc = c;
	dir = d; // (x,y,z) where -1< x,y,z < 1
	srand(time(NULL));
	force = btVector3(0,0,0);
   }

// Destructor
Wind::~Wind()
   { 

   }

void Wind::calcWind()
   {
	//Randomly Alter Wind Power
	if((rand() % roc) == 0)
	   {
		//Calculate New Wind
		float newX = rand() % (maxStr-minStr) + minStr;
		float newY = rand() % (maxStr-minStr) + minStr;
		float newZ = rand() % (maxStr-minStr) + minStr;
		force = btVector3(newX * dir.x() ,newY * dir.y(), newZ * dir.z());
		std::cout << "Wind(x,y,z) = "<< newX * dir.x() << " " << newY * dir.y() << " " << newZ * dir.z() << std::endl;
	   }
   }
void Wind::set(int a, int b, int c, btVector3 d)
   {
	minStr = a;
	maxStr = b;
	roc = c;
	dir = d; // (x,y,z) where -1< x,y,z < 1
	srand(time(NULL));
	force = btVector3(0,0,0);
   }
btVector3 Wind::getForce()
   {
	return force;
   }

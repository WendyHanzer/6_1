// 
// Wind Class to control the physics Wind
//
// Headers
#include <iostream>

// Bullet
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#define BIT(x) (1<<(x))

class Wind
   {
	public: 
		Wind(int a, int b, int c, btVector3 d);
		~Wind();
		void calcWind();
		void set(int a, int b, int c, btVector3 d);
		btVector3 getForce();

	private:
		int minStr, maxStr; //speed&direction attributes
		int roc; //rate of change where roc > 1. Lower numbers are
		btVector3 force, dir;
   };

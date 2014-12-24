// Includes
#include "camera.h"

// Constructor
Camera::Camera()
   {
	// Assign the initial camer locations
	//0,5,12 << Previous Default Values

	x_view_update = y_view_update = z_view_update = 0;
	x_view_next = x_view = 20.0;
	y_view_next = y_view = 30.0;
	z_view_next = z_view = 70.0;
	x_view = -40.0;
	y_view = -30.0;
	z_view = 20.0;
	x_focus_next = x_focus = 10.0;
	y_focus_next = y_focus = 1.0;
	z_focus_next = z_focus = -5.0;


	mode = 0;
	defaultT_f = threshold_f = 400;
	defaultT_v = threshold_v = 100;
	objX = 0;
	objY = 0;
	objZ = 0;
   }

// Destructor
Camera::~Camera()
   {

   }

// Return the updated view of the game
glm::mat4 Camera::getView()
   {
   	//Check if focus should be changed
   	if(!inFocus())
   	   {
   	   	slowChange(x_focus, x_focus_next, threshold_f);
   	   	slowChange(y_focus, y_focus_next, threshold_f);
   	   	slowChange(z_focus, z_focus_next, threshold_f);
   	   }

   	// Update the view
   	slowChange(x_view, x_view_next, threshold_v);
   	slowChange(y_view, y_view_next, threshold_v);
   	slowChange(z_view, z_view_next, threshold_v);

   	// Return the new view
   	return glm::lookAt(
   					   glm::vec3((x_view + x_view_update), (y_view + y_view_update), (z_view + z_view_update)),
   					   glm::vec3(x_focus, y_focus, z_focus),
   					   glm::vec3(0.0, 1.0, 0.0)
   					  );
   }

// Move the camera to a preset location
void Camera::viewUpdate()
   {
   	// Switch on a mode to get the camera view
	switch(mode)
	   {
		case 0:
			x_view_update = y_view_update = z_view_update = 0;
			x_view_next = 20.0;
			y_view_next = 30.0;
			z_view_next = 70.0;
			x_focus_next = x_focus = 10.0;
			y_focus_next = y_focus = 1.0;
			z_focus_next = z_focus = -5.0;
				break;
		case 1:
			x_focus = 0.0;
			y_focus = 1.0;
			z_focus =  0.0;
			x_view_update = 0.0;
			y_view_update = 0.0;
			z_view_update = 0.0;
			x_view = 0.0;
			y_view = 45.0;
			z_view = 0.0;
			break;
		case 2:
			x_view_update = x_view_next = x_view = 0.0;
			y_view_update = y_view_next = y_view = 5.0;
			z_view_update = z_view_next = z_view = 12.0;
			x_focus_next = x_focus = 0.0;
			y_focus_next = y_focus = 0.0;
			z_focus_next = z_focus = 0.0;
			break;
		case 3: //follow
			x_focus_next = objX;
			y_focus_next = objY;
			z_focus_next = objZ;
			x_view_update = 0.0;
			y_view_update = 0.0;
			z_view_update = 0.0;
			x_view_next = objX - 15;
			y_view_next = objY + 45;
			z_view_next = objZ + 15;
			break;

		case 4: //follow
			x_focus_next = 0;
			y_focus_next = 0;
			z_focus_next = 0;
			x_view_update = 0.0;
			y_view_update = 0.0;
			z_view_update = 0.0;
			x_view_next = objX;
			y_view_next = objY;
			z_view_next = objZ;
			break;
	   }
   }

void Camera::setMode(int newMode)
	{
		mode = newMode;
	}

void Camera::setObj(float x, float y, float z)
{
	objX = x;
	objY = y;
	objZ = z;
}

void Camera::setThresh(float vw, float foc)
{
	threshold_v = vw;
	threshold_f = foc;
}

// Take a keyboard hit and update the view
void Camera::cameraMove(unsigned char key)
   {
	//Move the camera
	//X-Locations
	if(key == 'q')
	   {
		if(x_view_update > -60.0)
		   {
			x_view_update -= 1.0;
		   }
	   }
	if(key == 'w')
	   {
		if(x_view_update < 60.0)
		   {
			x_view_update += 1.0;
		   }
	   }
	//Y-Locations
	if(key == 'e')
	   {
		if(y_view_update > -60.0)
		   {
			y_view_update -= 1.0;
		   }
	   }
	if(key == 'r')
	   {
		if(y_view_update < 60.0)
		   {
			y_view_update += 1.0;
		   }
	   }
	//Z-Locations
	if(key == 't')
	   {
		if(z_view_update > -60.0)
		   {
			z_view_update -= 1.0;
		   }
	   }
	if(key == 'y')
	   {
		if(z_view_update < 60.0)
		   {
			z_view_update += 1.0;
		   }
	   }

	//Change the Focus
	//X-Focus
	if(key == 'a')
	   {
		x_focus_next -= 1.0;
	   }
	if(key == 's')
	   {
		x_focus_next += 1.0;
	   }
	//Y-Focus
	if(key == 'd')
	   {
		y_focus_next -= 1.0;
	   }
	if(key == 'f')
	   {
		y_focus_next += 1.0;
	   }
	//Z-Focus
	if(key == 'g')
	   {
		z_focus_next -= 1.0;
	   }
	if(key == 'h')
	   {
		z_focus_next += 1.0;
	   }
   }

// Slowly moves the camera
void Camera::slowChange(float &current, float next, float divisor)
   {
	float change = ((next-current)/divisor);
	current+=change;
   }

// Checks if the camera is focused
bool Camera::inFocus()
   {
   	//Check X
	if((x_focus_next - x_focus) != 0)
	   {
	   	return false;
	   }

   	// Check Y
	if((y_focus_next - y_focus) != 0)
	   {
	   	return false;
	   }

   	// Check Z
	if((z_focus_next - z_focus) != 0)
	   {
	   	return false;
	   }

   	// No errors return true
   	return true;
   }

float Camera::x()
	{ return x_view + x_view_update; }
float Camera::y()
	{ return y_view + y_view_update; }
float Camera::z()
	{ return z_view + z_view_update; }

float Camera::nextX()
	{ return x_view_next; }
float Camera::nextY()
	{ return y_view_next; }
float Camera::nextZ()
	{ return z_view_next; }

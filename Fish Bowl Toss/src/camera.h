//
// Class to camera in the game
// 
// Includes
#include <iostream>

// GLM
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class Camera
   {
    public:
    	Camera();
    	~Camera();
    	glm::mat4 getView();
		void viewUpdate();
		void cameraMove(unsigned char key);
		void setMode(int newMode);
		void setObj(float x, float y, float z);
		void setThresh(float vw, float foc);
		float x();
		float y();
		float z();
		float nextX();
		float nextY();
		float nextZ();
		float objX;
		float objY;
		float objZ;
    private:
    	// Functions
    	void slowChange(float &current, float next, float divisor);
    	bool inFocus();

    	// Camera Positions
		float x_view,
			  y_view,
			  z_view,
			  x_view_next,
			  y_view_next,
			  z_view_next,
			  x_view_update,
			  y_view_update,
			  z_view_update,
			  x_focus,
			  y_focus,
			  z_focus,
			  x_focus_next,
			  y_focus_next,
			  z_focus_next;
		int mode;
		float threshold_f,
			  threshold_v,
			  defaultT_f,
			  defaultT_v;
   };

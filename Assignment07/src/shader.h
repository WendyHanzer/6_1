//
// Shader Class
//
class Shader
   {
    public:
        //Needed Functions
        Shader();
        Shader(Shader &src);
        ~Shader();
      
        //Working functions
        bool readFile(char *fileLoc);
        char* getShaderSource();

    private:
        char* buffer;
        long bufferSize;
   };

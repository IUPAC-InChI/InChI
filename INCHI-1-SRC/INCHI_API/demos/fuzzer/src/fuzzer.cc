/*
    Mimics Google AutoFuzz
*/

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>  // for free()
#include <string.h>  // for strdup()
#include <string>

#include "../../../../INCHI_BASE/src/inchi_api.h"

#include "fuzzer.h"

int(*getstructfrominchi) (inchi_InputINCHI*, inchi_OutputStruct*);
int(*getstructfrominchiex)(inchi_InputINCHI*, inchi_OutputStructEx*);
void(*freestructfrominchi) (inchi_OutputStruct*);
void(*freestructfrominchiex) (inchi_OutputStructEx*);
int(*getinchifrominchi)(inchi_InputINCHI*, inchi_Output*);
void (*freeinchi)(inchi_Output*);
int(*getinchikeyfrominchi)(const char*, const int, const int,char*, char*, char*);


HANDLE_INSTANCE hLIBINCHI;
char *szOptions = _strdup("");

extern "C" int FuzzerTestOneInput(const uint8_t *data, size_t size) 
{
  char *x =
      _strdup(std::string(reinterpret_cast<const char *>(data), size).c_str());

  char inchiKey[29], xtra1[65], xtra2[65];
  
  getinchikeyfrominchi(x, 0, 0, inchiKey, xtra1, xtra2);
  /*printf("i2k");*/

  inchi_InputINCHI i;
  i.szInChI = x;
  i.szOptions = szOptions;

  if (0)
  {
      inchi_Output o;
      getinchifrominchi(&i, &o);
      /*printf(" i2i");*/
      /*if (o.szLog)
      {
          fprintf(stderr, "\n*** AFTER GetINCHIfromINCHI() szlog={%-s} ***\n", o.szLog);
      }*/
      freeinchi(&o);
  }

  if (0)
  {
      inchi_OutputStruct os;
      getstructfrominchi(&i, &os);
      /*printf(" i2s");
      if (os.szLog)
      {
          fprintf(stderr, "\n*** AFTER GetStructFromINCHI() szlog={%-s} ***\n", os.szLog);
      }*/
      freestructfrominchi(&os);
      free(x);
  }
  if (1)
  {
      inchi_OutputStructEx os;
      getstructfrominchiex(&i, &os);
      /*printf(" i2s");
      if (os.szLog)
      {
      fprintf(stderr, "\n*** AFTER GetStructFromINCHI() szlog={%-s} ***\n", os.szLog);
      }*/
      freestructfrominchiex(&os);
      free(x);
  }

  return 0;
}

/****************************************************************************/
int main( int argc, char *argv[] )
{
    uint8_t buf[65536];
    FILE *f=NULL;
    size_t nbytes, bufsize;
    int result;
    long int ntodo, ndone=0;

#ifdef _DEBUG
    ntodo = 1000;
#else
    ntodo = 20000;
#endif


    if (argc < 2)
    {
        fprintf(stderr, "Usage:\tfuzzer inchi_input_file\n");
        exit( 1 );
    }
    else
    {
        f = fopen( argv[1], "rb" );
        if (!f)
        {
            fprintf(stderr, "Could not open input file\n" );
            fprintf(stderr, "Usage:\nfuzzer inchi_input_file\n");
            exit( 2 );
        }
    }

    if ( !load_inchi_library())
    { 
        fprintf(stderr, "Could not load %-s\n", LIBINCHINAME);
    }

    bufsize = sizeof( buf );
    memset( buf, 0, bufsize );
    
    nbytes = fread( buf, 1, bufsize , f );
    
    if (nbytes!= bufsize)
    {
        if (feof( f ))
        {
            int i;

            try
            {
                printf("\n\nFile: %-s\n", argv[1]);
                /* repeat calc many times trying to get crash or any other problem */
                for (i = 1; i<=ntodo; i++)
                {
                    /* Use pair of calloc/free of variable size to add some memory bifurcation */
                    size_t tmpsize = ( size_t ) ( 1048577.0 * ( rand( ) / ( RAND_MAX + 1.0 ) ) );
                    char *tmp = (char*) calloc( tmpsize, sizeof( char ) );

                    result = FuzzerTestOneInput( buf, nbytes );

                    if (result == 0)
                    {
                        ndone++;
                        printf( "%-d ", i);  //printf( "\r%-d %d", i, tmpsize );
                    }
                    else
                    {
                        printf( "\n* %d * ! Unknown problem * %-s\n", i, argv[1] );
                    }           

                    free( tmp );
                }
            }
            catch (...)
            {
                printf( "\n* %d * !!! Unknown problem * %-s\n", i, argv[1] );
                exit( 99 );
            }
        }
        else
        {
            printf( "\nFile read error\n" );
            exit( 3 );
        }

    }
    else
    {
        printf( "\nSomething went wrong\n" );
        exit( 4 );
    }

    printf( "\nPassed: %-d of %-d\n", ndone, ntodo);

    fclose( f );

    return 0;
}


int load_inchi_library(void) 
{
#ifdef _WIN32
    // I:\Dropbox\work\INCHI\devel\INCHI-DEV\INCHI-1-SRC\INCHI_API\bin2\Windows\x64\Debug\
    //if ((hLIBINCHI = LoadLibraryA("libinchi.dll")) == 0)
    if ((hLIBINCHI = LoadLibraryA("I:/Dropbox/work/INCHI/devel/INCHI-DEV/INCHI-1-SRC/INCHI_API/bin2/Windows/x64/Debug/libinchi.dll")) == 0)
        return 0;
#else
    if ((hLIBINCHI = dlopen("libinchi.so.1", RTLD_NOW)) == 0)
        return 0;
#endif


    getstructfrominchi = (int(*)(inchi_InputINCHI *inpInChI, inchi_OutputStruct *outStruct))
        GET_DLSYM(hLIBINCHI, "GetStructFromINCHI");

    getstructfrominchiex = (int(*)(inchi_InputINCHI*, inchi_OutputStructEx*))
        GET_DLSYM(hLIBINCHI, "GetStructFromINCHIEx");

    freestructfrominchi = (void(*)(inchi_OutputStruct *out))
        GET_DLSYM(hLIBINCHI, "FreeStructFromINCHI");

    freestructfrominchiex = (void(*) (inchi_OutputStructEx *out))
        GET_DLSYM(hLIBINCHI, "FreeStructFromINCHIEx");

    getinchifrominchi = (int(*)(inchi_InputINCHI*, inchi_Output*))
        GET_DLSYM(hLIBINCHI, "GetINCHIfromINCHI");

    freeinchi = (void(*) (inchi_Output *out))
        GET_DLSYM(hLIBINCHI, "FreeINCHI");

    getinchikeyfrominchi = (int(*)(const char*, const int, const int, char*, char*, char*))
        GET_DLSYM(hLIBINCHI, "GetINCHIKeyFromINCHI"); 

    return 1;
}

void unload_inchi_library(void)
{
#ifdef _WIN32
    FreeLibrary(hLIBINCHI);
#else
    dlclose(hLIBINCHI);
#endif
}


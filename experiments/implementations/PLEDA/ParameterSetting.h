//
//  ParameterSettings.h
//  CDRVNSforLOP
//
//  Created by Josu Ceberio Uribe on 21/09/17.
//  Copyright © 2017 Collaboration Santucci - Ceberio. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef ParameterSetting_h
#define ParameterSetting_h

#define RUNNING_ON_CLUSTER 1 //when commented the code is run locally.
//#define VERBOSE 1



// Name of the file where the instances is stored.
char INSTANCE_FILENAME[50];

// The seed asigned to the process
int SEED;

/*
 * Help command output.
 */
void usage(char *progname)
{
    printf("Usage: PlackettLuceEDA -i <instance_name> -s <seed> \n");
    printf("   -i File name of the instance.\n");
    printf("   -s Seed used for pseudo-random numbers generator.\n");
}

/*
 * Get next command line option and parameter
 */
int GetOption (int argc, char** argv, char* pszValidOpts, char** ppszParam)
{
    
    static int iArg = 1;
    char chOpt;
    char* psz = NULL;
    char* pszParam = NULL;
    
    if (iArg < argc)
    {
        psz = &(argv[iArg][0]);
        
        if (*psz == '-' || *psz == '/')
        {
            // we have an option specifier
            chOpt = argv[iArg][1];
            
            if (isalnum(chOpt) || ispunct(chOpt))
            {
                // we have an option character
                psz = strchr(pszValidOpts, chOpt);
                
                if (psz != NULL)
                {
                    // option is valid, we want to return chOpt
                    if (psz[1] == ':')
                    {
                        // option can have a parameter
                        psz = &(argv[iArg][2]);
                        if (*psz == '\0')
                        {
                            // must look at next argv for param
                            if (iArg+1 < argc)
                            {
                                psz = &(argv[iArg+1][0]);
                                if (*psz == '-' || *psz == '/')
                                {
                                    // next argv is a new option, so param
                                    // not given for current option
                                }
                                else
                                {
                                    // next argv is the param
                                    iArg++;
                                    pszParam = psz;
                                }
                            }
                            else
                            {
                                // reached end of args looking for param
                            }
                            
                        }
                        else
                        {
                            // param is attached to option
                            pszParam = psz;
                        }
                    }
                    else
                    {
                        // option is alone, has no parameter
                    }
                }
                else
                {
                    // option specified is not in list of valid options
                    chOpt = -1;
                    pszParam = &(argv[iArg][0]);
                }
            }
            else
            {
                // though option specifier was given, option character
                // is not alpha or was was not specified
                chOpt = -1;
                pszParam = &(argv[iArg][0]);
            }
        }
        else
        {
            // standalone arg given with no option specifier
            chOpt = 1;
            pszParam = &(argv[iArg][0]);
        }
    }
    else
    {
        // end of argument list
        chOpt = 0;
    }
    
    iArg++;
    
    *ppszParam = pszParam;
    return (chOpt);
}


/*
 * Obtaint the execution parameters from the command line.
 */
bool GetParameters(int argc,char * argv[])
{
    char c;
    if(argc==1)
    {
        usage(argv[0]);
        return false;
    }
    char** optarg;
    optarg = new char*[argc];
    while ((c = GetOption (argc, argv,":h:s:i:",optarg)) != '\0')
    {
        switch (c)
        {
            case 'h' :
                usage(argv[0]);
                return false;
                break;
                
            case 's' :
                SEED = atoi(*optarg);
                srand(SEED);
                break;
                            
            case 'i':
                strcpy(INSTANCE_FILENAME, *optarg);
                break;
                
                
            default:
                printf("Wrong parameter specification...\n");
                exit(1);
        }
    }
    
    delete [] optarg;
    return true;
}

#endif /* ParameterSetting_h */

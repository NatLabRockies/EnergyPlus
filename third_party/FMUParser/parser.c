///////////////////////////////////////////////////////
/// \file   parser.c
///
/// \brief  Methods for master program that interacts
///         with an FMU for co-simulation
///
/// \author Wangda Zuo, Thierry S. Nouidui, Michael Wetter
///         Simulation Research Group,
///         LBNL,
///         WZuo@lbl.gov
///
/// \date   2011-11-02
///
/// This file parse the FMU for co-simulation and translate related information
/// into Energyplus input file (.idf)
///
/// Copyright (c) 2012, The Regents of the University of California,
/// through Lawrence Berkeley National Laboratory
/// subject to receipt of any required approvals from the U.S. Dept. of Energy.
/// All rights reserved.
///////////////////////////////////////////////////////
#include "parser.h"
#include "string.h"
#include "util.h"

#ifndef _MSC_VER
#include <libgen.h>
#endif

#define XML_FILE "modelDescription.xml"

FMU fmu; // FMU to parse

///////////////////////////////////////////////////////////////////////////
/// Call the parser and save the information in fmu.modelDescription
///
///\param fmuFilNam The name of fmu file
///\param tmpPat Temporary path of the fmu file
///\return 0 if no error occurred
///////////////////////////////////////////////////////////////////////////
int callparser(const char *fmuFilNam, const char *tmpPat)
{
    size_t length;
    char *xmlPat;
    char *filNam, *tmp, *ext;

    // Get the model description
    xmlPat = (char *)calloc(strlen(fmuFilNam) + strlen(XML_FILE) + strlen(PATH_SEP) + 1, sizeof(char));
    printfDebug("fmuFilNam is \"%s\"\n", fmuFilNam);
    sprintf(xmlPat, "%s%s%s", tmpPat, PATH_SEP, XML_FILE);
    printfDebug("Start to parse model description file: \"%s\".\n", xmlPat);

    fmu.modelDescription = parse(xmlPat);
    printfDebug("Parsed model description file: \"%s\"\n", xmlPat);

    if (fmu.modelDescription == NULL) {
        printfError("Fail to parse xml file \"%s\".\n", xmlPat);
        free(xmlPat);
        return -1;
    }

    free(xmlPat);

    length = strlen(fmuFilNam);
    tmp = (char *)malloc((length+1) * sizeof(char));
    strcpy(tmp, fmuFilNam);

#ifdef _MSC_VER // Command in windows
    filNam = (char *)malloc(length * sizeof(char));
    if (filNam == NULL) {
        printError("Can not allocate memory for filName\n");
        free(tmp);
        return -1;
    }

    ext = (char *)malloc(length * sizeof(char));
    if (ext == NULL) {
        printError("Can not allocate memory for ext\n");
        free(filNam);
        free(tmp);
        return -1;
    }

    _splitpath(tmp, NULL, NULL, filNam, ext);
    filNam = strcat(filNam, ext);
#else // Command in linux
    filNam = basename(tmp);
#endif
    printidf(filNam, fmu.modelDescription);
    printfDebug("***** Model description is \"%s\".\n", getModelIdentifier(fmu.modelDescription));

    // Clean up memory
    // free(xmlPat); // Done above
    free(tmp);
#ifdef _MSC_VER
    // https://man7.org/linux/man-pages/man3/basename.3.html
    // Both dirname() and basename() return pointers to null-terminated strings.  (Do not pass these pointers to free(3).)
    free(filNam);
    free(ext);
#endif
    return 0;
}

//////////////////////////////////////////////////////////////////////
/// Print help information
/////////////////////////////////////////////////////////////////////
void help(void)
{

    printf("NAME\n");
    printf("\tparser - parse xml file\n\n");
    printf("SYNOPSIS\n");
    printf("\tparser OPTION...FMUFILE...[-name objNam]\n\n");
    printf("DESCRIPION\n");
    printf("\tThe parser parses informaiton in modelDescription.xml file in functional mock-up unit for co-simulation.\n\n");
    printf("OPTION\n");
    printf("\t-d, --delete\n");
    printf("\t\tdelete the unpacked fmu folder\n");
    printf("\t-h, --help\n");
    printf("\t\tprint help information\n");
    printf("\t-p, --printidf\n");
    printf("\t\tprint temporary IDF file for Energyplus\n");
    printf("\t-u, --unpack\n");
    printf("\t\tunpack the FMU\n");
    printf("\t-v, --verbose\n");
    printf("\t\tprint debug information. It is the only option that can combine with other options.\n");
    printf("OTHERS\n");
    printf("\t-n, --name\n");
    printf("\t\textract the fmu in folder objNam. The default folder name is fmu's name without the \".fmu\".\n\n");
    printf("EXAMPLE\n");
    printf("\tTo generate a temporary IDF file from a fmu named havc.fmu in Linux\n");
    printf("\t\tparser -p hvac.fmu\n\n");
    printf("\tTo unpack the fmu havc.fmu in a folder called MyFMU in linux\n");
    printf("\t\tparser -u havc.fmu -n MyFMU\n");
    printf("AUTHOR\n");
    printf("\tWangda Zuo, Thierry S. Nouidui, Michael Wetter @ Lawrence Berkeley National Laboratory\n");
    printf("LICENSE\n");
    printf("\tCopyright (c) 2012, The Regents of the University of California, through Lawrence Berkeley National Laboratory subject to receipt of "
           "any required approvals from the U.S. Dept. of Energy. All rights reserved.\n");
}

//////////////////////////////////////////////////////////////////////////////////
/// Main routine of parser
///
///\param argc Number of arguments
///\param argv Arguments
///\return 0 if no error occurred
//////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    int i;
    const char *fmuFilNam;
    char *tmpPat = NULL;
    char *objNam = NULL;
    option opt;
    size_t length = 0;
    int nam = 0;
    int optNum = 0;

    printDebug("New Line:\n");

    // Read command line arguments
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--delete") == 0 || strcmp(argv[i], "-d") == 0) {
            opt = opt_delete;
            optNum++;
            printDebug("Get option: delete");
        } else if (strcmp(argv[i], "--printidf") == 0 || strcmp(argv[i], "-p") == 0) {
            opt = opt_printidf;
            optNum++;
            printDebug("Get option: printidf");
        } else if (strcmp(argv[i], "--unpack") == 0 || strcmp(argv[i], "-u") == 0) {
            opt = opt_unpack;
            optNum++;
            printDebug("Get option: unpack");
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            opt = opt_help;
            optNum++;
            printDebug("Get option: help");
        } else if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            setDebug();
            printDebug("Get option: verbose");
        } else if (strcmp(argv[i], "--name") == 0 || strcmp(argv[i], "-n") == 0) {
            i++;
            nam = 1;
            if (argv[i] != NULL)
                objNam = argv[i];
            else {
                printError("with option -n, objNam is needed.");
                return -1;
            }
            printDebug("Get option: name");
        } else if (strstr(argv[i], ".fmu") != NULL)
            fmuFilNam = argv[i];
        else
            printf("Warning: Ingore unrecognized input \"%s\"\n", argv[i]);
    }

    if (optNum == 0) {
        if (IS_WINDOWS) {
            printError("Missing option: parser.exe option [argument]");
            printf("For help, use: parser.exe -h \n");
        } else {
            printError("Missing option: parser option [argument]");
            printf("For help, use: parser -h \n");
        }
        return -1;
    }

    if (optNum > 1) {
        printError("Can not use more than 1 option at the same time except \"-u\".");
        if (IS_WINDOWS) {
            printf("For help, use: parser.exe -h \n");
        } else {
            printf("For help, use: parser -h \n");
        }
        return -1;
    }

    if (fmuFilNam == NULL && opt != opt_help) {
        switch (opt) {
        case opt_printidf:
        case opt_unpack:
            printError("No FMU file name is given.\n");
            break;
        case opt_delete:
            printError("No FMU folder is given.\n");
            break;
        case opt_help:
            // we know we cannot hit this block
            break;
        }

        if (IS_WINDOWS) {
            printf("Correct usage: parser.exe option [argument] \n");
            printf("For help information, use: parser.exe -h \n");
        } else {
            printf("Correct usage: parser option [argument] \n");
            printf("For help information, use: parser -h \n");
        }

        return -1;
    } else {
        printfDebug("FMU file/folder name is \"%s\".\n", fmuFilNam);
    }

    // Check command line arguments
    switch (opt) {
    case opt_printidf:
    case opt_unpack:
    case opt_delete:
        if (nam == 0) {
            length = strlen(fmuFilNam) - 4;
            if (length < 0) {
                printError("Cannot get correct length of fmu file name");
                return -1;
            }

            objNam = (char *)calloc((length + 1), sizeof(char));
            // Copy the fmufilNam without extension as objNam
            if (memcpy(objNam, fmuFilNam, length) == NULL) {
                printError("Can not get name \"objNam\" for new folder.\n");
                free(objNam);
                return -1;
            }

            // This is pretty useless as calloc will initialize to 0 (which is in effect the null-terminator)
            if (strcat(objNam, "\0") == NULL) {
                printError("Can not add ending for objNam.\n");
                free(objNam);
                return -1;
            }
            printfDebug("objNam = \"%s\"\n", objNam);
        } else
            length = strlen(objNam);
        break;
    case opt_help:
        help();
        return 0;
    default:
        printError("Missing options: --delete | -d | --printidf | -p | --unpack | -u| --verbose | -v \n");
        return -1;
    }

    printDebug("Get length of file name");
    // Define the temporary folder
    tmpPat = getTmpPath(objNam, length);
    if (tmpPat == NULL) {
        printError("Fail to allocate memory for temp dir.\n");
        return -1;
    }

    printfDebug("Received tmpPat \"%s\".\n", tmpPat);

    // Act according to the option
    switch (opt) {
    case opt_delete:
        if (delete (objNam) != 0) {
            printfError("Fail to delete \"%s\".\n", objNam);
            return -1;
        } else
            printfDebug("Successfully deleted the temporary folder \"%s\".\n", objNam);
        break;
    case opt_unpack:
    case opt_printidf:
        // Unpack: Common part for unpack and printidf
        if (unpackminizip(fmuFilNam, tmpPat) != 0) {
            printfError("Fail to unpack \"%s\".\n", fmuFilNam);
            return -1;
        } else
            printDebug("Successfully unpacked the fmu.\n");

        // Parser: Additional part for printidf
        if (opt == opt_printidf) {
            if (callparser(fmuFilNam, tmpPat) != 0) {
                printfError("Fail to print the idf file from \"%s\".\n", fmuFilNam);
                return -1;
            } else
                printDebug("Successfully printed the idf file.\n");
        }
        break;
    case opt_help:
        // Nothing to do here
        break;
    }

    free(objNam);
    free(tmpPat);
    return 0;
}

/*******************************************************************************

     The MIT License (MIT)

    Copyright (c) 2017 Gikoskos <georgekoskerid@outlook.com>

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

 *********************************************************************************/


/**********
* xf2bf.c
 *************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>



/* Creates a string that stores the filename for the binary
 * by appending an .out extension (e.g. if fname_in == "test.txt"
 * then *fname_out == "text.txt.out").
 * Returns false if both the arguments given are NULL. */
bool makeNewFilename(char *fname_in, char **fname_out)
{
    if (fname_in && fname_out) {
        //get the character length of the input filename
        size_t len = strlen(fname_in);

        //calculate the size of the newly allocated string
        //by adding 5 (for ".out" to fit in the new string)
        len += 5;

        *fname_out = malloc(len);
        assert(*fname_out);

        snprintf(*fname_out, len, "%s.out", fname_in);

        return true;

    }

    return false;
}

/* Reads the next word from a file.
 * Returns a pointer to the word, or NULL in case EOF is reached. */
char *readNextWordFromFile(FILE *fd)
{
    //constant value to use as a standard allocation size
    static size_t const alloc_size = 127;

    //buffer to store the word that was read. The buffer's
    //size is dynamically allocated (and reallocated if neccessary)
    //to fit the total characters of each word
    char *buff = NULL;

    //points to the current index in the buff array
    size_t idx = 0;

    //stores the current size of the buff array
    size_t curr_size = 0;

    //temporary byte to store the current character read from the file
    char c;

    //skip all initial whitespace until we hit a word character or EOF
    while ( (c = fgetc(fd)) != EOF && isspace(c));

    //if we haven't hit EOF and we hit a valid word character instead
    if (!feof(fd)) {

        //first we allocate size equal to alloc_size for our buff array
        curr_size = alloc_size;
        buff = malloc(curr_size);
        assert(buff != NULL);

        //loop through each character of the word, until we hit whitespace or EOF
        //this is a do/while loop so that we won't skip the last character that
        //was read in the previous while() loop
        do {

            //if the current index is bigger than the current size
            //it means that we have read more characters than they could fit into
            //the array
            if (idx >= curr_size) {

                //so we reallocate the array into a bigger size
                curr_size += alloc_size;
                buff = realloc(buff, curr_size);
                assert(buff != NULL);

            }

            //store the character to the array and increment our index
            buff[idx++] = c;

        } while ((c = fgetc(fd)) != EOF && !isspace(c));

        //in case we read as many characters as the array can fit,
        //we need to reallocate, to fit the null-terminating
        //character into the array
        if (idx >= curr_size) {

            buff = realloc(buff, curr_size + 1);
            assert(buff != NULL);

        }

        buff[idx] = '\0';

    }

    return buff;
}

/* Takes a null-terminated string as an argument.
 * Returns true only if the string was in this format "0xab", "0x12" etc
 * or in this format "ab", "12" etc */
bool isSingleHexByteString(char *str)
{
    bool retval = false;

    if (str) {
        size_t len = strlen(str);

        //if the length of the string is 4 characters, check for this format "0xdf" etc
        if (len == 4) {

            if (str[0] == '0' && str[1] == 'x') {

                if (isxdigit(str[2]) && isxdigit(str[3]))
                    retval = true;

            }
        //else if it's 2 characters check for this format "c3" etc
        } else if (len == 2) {

            if (isxdigit(str[0]) && isxdigit(str[1]))
                retval = true;

        }
    }

    return retval;
}

/* Takes as argument a filename that points to a file containing 
 * hex strings (0xab 0xfd 0x9d etc) and creates a new binary 
 * file with its bytes as the values of each of those strings. 
 * Returns errno error code from any failed function, or 0 on success. */
int createBinaryFromByteArrayFile(char *fname)
{
//Convenience macro that saves its argument  as the current error
//code and jumps to CLEANUP
#define CLEANUP_RETURN(n) \
{\
    err_code = n;\
    goto CLEANUP;\
}
    //integer to store the return error code
    int err_code = 0;

    //this variable stores each byte to be written to the file
    //every time. Its value is the result of the conversion of
    //a hex string to int.
    char byte_to_write;

    //the filename of the new file to be created
    char *new_fname = NULL;

    //each word as it's read by the input file
    char *word;

    //the file to read the hex strings from
    FILE *in_fd = NULL;

    //the file to write the converted bytes to
    FILE *out_fd = NULL;

    //try to open the file with the given filename, in reading mode
    in_fd = fopen(fname, "r");
    if (!in_fd) CLEANUP_RETURN(errno);

    //create a filename for the new binary
    if (!makeNewFilename(fname, &new_fname)) CLEANUP_RETURN(errno);

    //try to create the new file and open it in writing mode
    out_fd = fopen(new_fname, "w");
    if (!out_fd) CLEANUP_RETURN(errno);


    //parse every word from the input file until there are no more words
    while ( (word = readNextWordFromFile(in_fd)) != NULL ) {

        //if the string that was read from the input file is a single hex byte string
        if (isSingleHexByteString(word)) {
            //try to convert the string that was parsed, to a single byte character
            errno = 0;
            byte_to_write = (char)strtol(word, NULL, 16);
            if (errno) CLEANUP_RETURN(errno);

            //try to write the single byte to the binary file
            errno = 0;
            fwrite(&byte_to_write, 1, 1, out_fd);
            if (errno) CLEANUP_RETURN(errno);
        }

        //free the dynamically allocated string to avoid mem. leaks
        free(word);
    }

//We don't need the macro after this point.
#undef CLEANUP_RETURN
CLEANUP:
    //cleanup
    if (new_fname) free(new_fname);
    if (out_fd) fclose(out_fd);
    if (in_fd) fclose(in_fd);
    return err_code;
}


int main(int argc, char *argv[])
{
    //If at least one argument is entered by the user (other than the executable's name)
    if (argc > 1) {

        //process each file with the function createBinaryFromByteArrayFile,
        //starting from the last one entered in the command line, and save the returned error code.
        //Print error message if there was an error (err == 0 means success).
        int err;
        while (argc > 1)
            if ( (err = createBinaryFromByteArrayFile(argv[--argc])) )
                fprintf(stderr, "File \"%s\" failed with error: %s!\n", argv[argc], strerror(err));

    } else {

        fprintf(stderr, "No filename arguments given.\n");

    }

    return 0;
}

//
// Created by Cata on 4/19/2021.
//

/**
 * Encrypt352 is the main source of the file that process and runs simultaneous threads
 * reading characters from an input file, encrypting them and writing to an output file.
 * This file is also the source for reset and output of information on the current status
 * of the encryption.
 *
 * Unfortunately this file fails to deliver upon most of what it needs to do sadly.
 */


#include "encrypt-module.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


/**
 * This method should do all the printing of th currently read input and output.
 */
void reset_requested(){
    //This should print out all the current details
    //Problem is unsure if this is where I would lock threads to print that info
    //Need to play with this more.
    printf("Reset Requested\n");
    printf("Total input count with current key is %d\n", get_input_total_count());
    char c;
    int count = 0;
    for(c = 'A'; c < 'Z'; c++) printf("%c:%d ", c, get_input_count(count++));
    printf("Z:%d\n", get_input_count(count));
    printf("Total output count with current key is %d\n", get_output_total_count());
    count = 0;
    for(c = 'A'; c < 'Z'; c++) printf("%c:%d ", c, get_output_count(count++));
    printf("Z:%d\n", get_output_count(count));
}

void reset_finished(){
    //not exactly sure what reset_finished does. I know it prints the statement
    //Need to figure out if read_input is correct or what needs to be done
    //Ideally I think this should print and release holds on the threads?
    //Then go back to reading?
    printf("Reset Finished.\n");
    read_input();
}


int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Number of arguments are wrong. Should be 3 but there was %d. The expected inputs should be encrypt352 with an input file and output file text\n", argc);
        exit(1);
    }
    int input_buffer_size, output_buffer_size;
    printf("Enter input buffer size (N) as an integer: ");
    scanf("%d", &input_buffer_size);
    printf("Enter output buffer size (M) as an integer: ");
    scanf("%d", &output_buffer_size);
    if (input_buffer_size <= 1) {
        fprintf(stderr, "Input buffer size is %d, needs to be greater than 1", input_buffer_size);
        exit(1);
    }
    if (output_buffer_size <=1){
        fprintf(stderr, "Output buffer size is %d, needs to be greater than 1", output_buffer_size);
        exit(1);
    }
    
    //need to figure out buffers
    //to to understand adding and manipulating the threads
    //Testing the single thread to get an idea of how this all works.
    init(argv[1], argv[2]);
    int c;
    while (1) {
        c = read_input();
        if (EOF == c)
            break;
        count_input(c);
        int encrypted = caesar_encrypt(c);
        count_output(encrypted);
        write_output(encrypted);
    }
}
/*
 * bf2c.c
 * 
 * Copyright 2017 Nigel Nquande <nigelnquande@yahoo.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license.
 */


#include <stdio.h>
#include <stdlib.h>
// #include <sys/unistd.h>
#include <string.h>

#define DEBUG 1  /* Change this to 1 to see debug output */
/* number of characters to reserve per instruction. 
 * was 10; increased as the reserved array was too short and caused a segmentation fault. 
 * Increase this if you encounter segfaults. 
 */
#define TRANSLATION_EXPANDER 15 

/* Copy chars from src to dest, returning the number of chars copied */
static unsigned int str_copy(const char *src, char *dst) {
	unsigned int i = 0;
	while (src[i] != '\0') {
		dst[i] = src[i];
		i++;
	}
	return i;
}

/* Translate a BF character instruction (token) to the C equivalent */
static const char * codeGen(char token) {
	switch (token) {
		case '+': return "\to[i]++;\n";
		case '-': return "\to[i]--;\n";
		case '>': return  "\ti++;\n";
		case '<': return "\ti--;\n";
		case '.': return "\tputchar (o[i]);\n";
		case ',': return "\to[i] = getchar();\n";
		case '[': return "\n\twhile (o[i] > 0) {\n\t";
		case ']': return "\n\t}\n\n";
		default:  return "";
	}
}

/* Given BF input, print C output */
void parseBrainfuck(char * input, unsigned long inputLength) {
  printf("\n/* BF Source:\n%s\n*/", input);
  unsigned long translated_length = inputLength * TRANSLATION_EXPANDER;
  char translated[translated_length];
  char inChar;
  unsigned long input_ind = 0, translated_ind = 0;
  if (DEBUG) printf("/*Parsing BF source code of length %lu bytes (%lu bytes reserved):\n%s*/\n", inputLength, translated_length, input);
	translated_ind += str_copy(
		"\n#include <stdio.h>\n#include <stdlib.h>\n\nint main (void) {\n\tchar o[30000];\n\tint i; for (i = 0; i < 30000; i++) o[i] = 0;\n\ti = 0;\n", 
		translated + translated_ind
	);
	for (input_ind = 0; input_ind < inputLength; input_ind++) {
		inChar = input[input_ind];
		if (inChar == '\0') continue;
		if (DEBUG) printf("In Char: %c\t", inChar);
		translated_ind += str_copy(codeGen(inChar), translated + translated_ind);
		if (DEBUG) printf("Translation Index:\t%03lu\n", translated_ind);
	}
	translated_ind += str_copy("\n\treturn 0;\n}\n", translated + translated_ind);
	printf("%s", translated);
}

unsigned long str_len(char * in) {
  unsigned long len = 0;
  while (in[len] != '\0') { 
    len++;
    asm("nop"); // requires `-funroll-all-loops` passed to GCC
  }
  len++;
  
  if (DEBUG) printf("String length: %lu\n", len);
  
  return len;
}

int main(int argc, char **argv) {
	if (argc < 3 || argv[1][0] != '-') {
		fprintf(stderr, "No Brainfuck input!\n");
		fprintf(stderr, "Usage:\t%s [-e|-i] brainfuck_input\n-e\tevaluate input string\n-i\tevaluate input file\n", argv[0]);
		return EXIT_FAILURE;
	} else if (argv[1][1] == 'e') {
		char instream[str_len(argv[2])];
		strcpy(instream, argv[2]);
		if (DEBUG) printf("/* Received Input:\t%s */\n", instream);
		// pass the contents of instream to parseBrainfuck();
		parseBrainfuck(instream, str_len(argv[2]));
	} else if (argv[1][1] == 'i') {
		// open the specified file and copy its contents into instream
		char * buffer = 0;
		unsigned long length = 0;
		size_t bytes_read = 0;
		if (DEBUG) printf("/* File to read: %s */\n", argv[2]);
		FILE * f = fopen(argv[2], "rb");
		if (f) {
			fseek(f, 0, SEEK_END); // go to end of file
			length = ftell(f); // get the length of the file (in bytes)
			if (DEBUG) printf("/*File size: %ld bytes*/\n", length);
			buffer = malloc(length);
			fseek(f, 0, SEEK_SET); // go to start of file
			if (buffer) bytes_read = fread(buffer, 1, length, f);
			printf("/* %03lu bytes read from file. */\n", bytes_read);
			if (DEBUG) printf("/* File content:\n%s */\n", buffer);
			fclose(f);
		} else fprintf(stderr, "No input file specified! Did you mean to use -e instead?\n");
		if (buffer) parseBrainfuck(buffer, length);
		else fprintf(stderr, "No file input in buffer!");
		free(buffer);
	}
	printf("\n");
	
	return EXIT_SUCCESS;
}

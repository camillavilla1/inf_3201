/*"""
* Precode for assignment 3
* INF-3201, UIT
* Written by Frode Opdahl
* Based on Python precode by Edvard Pedersen
*/


#include <stdio.h>
#include <sys/time.h>
#include "md5.h"

#define ALPHABET_SIZE 100
static char alphabet[ALPHABET_SIZE];			// Alphabet containing all printable characters in the same order as pythons string.printable

unsigned int *encrypted;			// The encrypted data
const char *known_part;				// The known phrase that is in the decrypted data
unsigned int in_byte_size;			// Size of the encrypted data in bytes
unsigned int in_int_size;			// Size of the encrypted data in int
char password_solution[20];			// The final correct password


unsigned long long limit = 100000000;

// Variables used to calculate time
struct timeval _start_t, _mid_t, _end_t;
long dur;




/*
 * Function:  decipher 
 * --------------------
 * XTEA implementation in C, decryption
 *
 *	num_rounds: the number of iterations in the algorithm, 32 is reccomended
 *  v: 2 32bit ints, will be overwritten with deciphered value
 *  key: 128-bit key to use
 */
void decipher(unsigned int num_rounds, unsigned int v[2], unsigned int const key[4]) {
    unsigned int i;
    unsigned int v0=v[0], v1=v[1], delta=0x9E3779B9, sum=delta*num_rounds;
    for (i=0; i < num_rounds; i++) {
        v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum>>11) & 3]);
        sum -= delta;
        v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
    }
    v[0]=v0; v[1]=v1;
}

/*
 * Function:  decrypt_bytes 
 * --------------------
 * decrypts the data in the encrypted array, and stores it
 *
 *  decrypted: memory location of where to store the decrypted data
 *  key: 128-bit key to use
 */
void decrypt_bytes(unsigned int *decrypted, unsigned char *key)
{
	
	unsigned int deciphered[2];
	deciphered[0] = encrypted[0];
	deciphered[1] = encrypted[1];
	decipher(32, deciphered, (unsigned int*)key);
	decrypted[0] = deciphered[0] ^ (unsigned int)1;
	decrypted[1] = deciphered[1] ^ (unsigned int)2;
	int i = 2;
	while (i < in_int_size -1){
		deciphered[0] = encrypted[i];
		deciphered[1] = encrypted[i+1];
		decipher(32, deciphered, (unsigned int*)key);
		decrypted[i] = deciphered[0] ^ encrypted[i-2];
		decrypted[i+1] = deciphered[1] ^ encrypted[i-1];
		i += 2;
	}
}	

/*
 * Function:  reconstruct_secret 
 * --------------------
 * unshuffles the data in decrypted
 *  decrypted: pointer to the decrypted data
 *	result: pointer to where to store the unshuffled data
 */
// Reconstructs on cpu
void reconstruct_secret(unsigned int *result, unsigned int *decrypted)
{
	unsigned int i;
	for (i = 0; i < in_int_size; i++){
		unsigned int element = decrypted[i];
		result[(element >> 8) % in_int_size] = element & 0xff;
	}
}

/*
 * Function:  search_for_secret 
 * --------------------
 * searches for the known phrase in the given data
 *
 *  in_data: pointer to the decrypted and unshuffled data
 *
 *  returns: 1 if the known part is found in the data, 0 otherwise
 */
int search_for_secret(unsigned int *in_data)
{
	int j = 0;
	char *pch;
	while (j < in_int_size-1){
		int size_left = in_byte_size-j*sizeof(unsigned int);
		pch = (char *)memchr(&in_data[j], known_part[0], size_left);
		if (pch != NULL){
			int pos = (((unsigned int *)pch - &in_data[0]));
			int k;
			for (k = 0; k <= strlen(known_part); k++){
				if (k == strlen(known_part)){
					// Reconstruction is correct
					return 1; 
				}
				if (in_data[pos+k] == known_part[k]){
					continue;
				} 
				else{
					break;
				}
			}
			if (memcmp(pch, known_part, strlen(known_part)) == 0){

			} else {
				j = pos+1;
				continue;
			}
		} 
		else{ 
			break;
		}
	}
	return 0;
}
/*    """Check if a password is correct by decrypting, reconstructing, and looking for the known string.

    Arguments:
    in_data -- encrypted data
    guess -- password to try
    known_part -- known part of the plaintext

    returns -- True if the guess is correct, False otherwise
    """*/

/*
 * Function:  try_password 
 * --------------------
 * Check if a password is correct by decrypting, reconstructing, and looking for the known string.
 *
 *  decrypted: pointer to where the decrypted data will be stored
 *  reconstructed: pointer to where the reconstructed data will be stored
 *  guess: password to try
 *  returns: 1 if the guess is correct, 0 otherwise
 */
int try_password(unsigned int *decrypted, unsigned int *reconstructed, char *guess)
{
	// Decode key
	unsigned char digest[16];
	MD5_CTX md5;
	MD5_Init(&md5);
	MD5_Update(&md5, guess, strlen((char*)guess));
	MD5_Final(digest, &md5);

	decrypt_bytes(decrypted, (unsigned char*)digest);

	reconstruct_secret(reconstructed, decrypted);

	//Check if known part is contained in the reconstructed_h secret
	if (search_for_secret(reconstructed)){
		strcpy(password_solution, guess);		// Store correct password
		return 1;
	} 
	else {
		// Reconstruction is wrong
		return 0;
	}
}

/*
 * Function:  convert_val_to_string 
 * --------------------
 * generates a string and stores it in str, based on an integer
 * e.g. 0 = "0", 10 = a, 15="f", 4284394="4sH".
 * This is an easy way to split up the problem into sets of fewer problems if one
 * wants to parallelise over many threads. E.g. thread0 guess from 0 to 999, thread1
 * from 1000 to 1999 etc...
 *
 *  number: number that will be used to generate the string
 *  str: memory location where the strings value will be stored
 *
 */
inline void convert_val_to_string(unsigned int number, char *str){
	if (number == 0){
		memset(str, 0, 20);
		str[0] = alphabet[0];
		return;
	}
	unsigned long long x = (unsigned long long)number;
    char digits[20];
    memset(str, 0, 20);
    unsigned long long i, k;
    i = k = 0;
    while (x){
    	digits[i] = x % (unsigned int)ALPHABET_SIZE;
    	x = x / ALPHABET_SIZE;
    	i++;
    }
   	for (k = 0; k < i; k++) 
   		str[i-1-k] = alphabet[digits[k]];
	

   	// Prints status update
	if (number % 1000 == 0){
		int print = 1;
		for (k = 0; k < i; k++) {
   			if (str[k] < 14 && str[k] > 8 || str[k] == 92 || str[k] == 37 || str[k] == 32){
   				print = 0;
   			}
		}

		if (print){
			gettimeofday(&_mid_t, NULL);
			unsigned long long sec = (_mid_t.tv_sec) - (_start_t.tv_sec);
			if (sec != 0)
				printf("%s\t\t%uk\t\t%llus\t\t%llu", str, number/1000, sec, (unsigned long long)(number/sec));
			else
				printf("%s\t\t%uk\t\t%llus\t\t", str, number/1000, sec);
			printf("\t");
			printf("\b");//backspaces on char
			printf("\r");//return to beginning of line
			fflush(stdout);
		}
	}
	
}

/*
 * Function:  bruteforce_password 
 * --------------------
 * Iterative brute-force password guessing. Allocates necessary memory for future use.
 * Tries increasingly more complicated passwords.
 * Returns only if the password is found
 *
 *  returns: the number of passwords attempted before the correct one was found
 */
int bruteforce_password()
{
    unsigned long long i = 0;

	char *str = (char *)malloc(20);    

	unsigned int *decrypted;	 
	unsigned int *reconstructed;
	decrypted = (unsigned int*)malloc(in_int_size*sizeof(unsigned int));     
	reconstructed = (unsigned int*)malloc(in_int_size*sizeof(unsigned int));

	for (i = 0; i < limit; i++){
		convert_val_to_string(i, str);
        if(try_password(decrypted, reconstructed, str)){
        	break;
        }
    }

	free(decrypted);
	free(reconstructed);
    return i;
}

/*
 * Function:  create_alphabet 
 * --------------------
 * Creates an array of all characters, in the same order as pythons string.printable.
 * This is to be competable with testing the C implementation with the Python implementation.
 *
 *  ab: pointer to where the values will be stored
 *
 */

void create_alphabet(char *ab)
{
	int i;
	for (i = 0; i<10; i++){
		ab[i] = i+48;
	}
	for (i = 0; i<26; i++){
		ab[i+10] = i+97;
	}
	for (i = 0; i<26; i++){
		ab[i+36] = i+65;
	}
	for (i = 0; i<14; i++){
		ab[i+62] = i+33;
	}
	for (i = 0; i<7; i++){
		ab[i+78] = i+58;
	}
	for (i = 0; i<6; i++){
		ab[i+85] = i+91;
	}
	for (i = 0; i<4; i++){
		ab[i+91] = i+123;
	}
	ab[94] = 32;
	ab[95] = 9;
	ab[96] = 10;
	ab[97] = 13;
	ab[98] = 11;
	ab[99] = 12;
}

/*
 * Function:  read_encrypted_file 
 * --------------------
 * Reads the encrypted binary file into memory
 *
 *  filename: name of the file to be read
 *
 *  returns: returns 0 if the file is read successfully, 1 otherwise
 */
// Reads the encrypted binary file into memory
int read_encrypted_file(char *filename)
{
	FILE *fileptr;

	long filelen;
	printf("filename: %s\n", filename);
	fileptr = fopen(filename, "rb");  // Open the file in binary mode
	if (fileptr == NULL){
		printf("FILE NOT FOUND!\n");
		printf("Exiting...\n");
		exit(1);
	}
	fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
	filelen = ftell(fileptr);             // Get the current byte offset in the file
	rewind(fileptr);                      // Jump back to the beginning of the file
	encrypted = (unsigned int *)malloc((filelen+1)*sizeof(char)); // Enough memory for file + \0
	
	if(encrypted == NULL){
	    printf("\nERROR: Memory allocation did not complete successfully!"); 
	    return 1;
	} 

	in_byte_size = filelen;
	in_int_size = filelen/(sizeof(unsigned int)/sizeof(char));

	if (!fread(encrypted, filelen, 1, fileptr)){
		return 1;
	} // Read in the entire file
	fclose(fileptr); // Close the file
	return 0;
}

int main (int argc, char **argv)
{
	if (argc < 2){
        fprintf(stderr, "usage: %s filename [-known (default 'secret')]\n", argv[0]);
        exit(1);
    }

    known_part = "secret";
	int i;
    for (i = 1; i < argc; i++) { 				/* Skip argv[0] (program name). */
        if (strcmp(argv[i], "-known") == 0 && i+1 < argc) {	/* Process optional arguments. */
            known_part = argv[i+1];  			
            printf("known_part %s\n", known_part);
        }
    }
	// Create printable alphabet similar to pythons string.printable
	create_alphabet(alphabet);

	if (read_encrypted_file(argv[1]) == 1){
		printf("\nMemory for file not allocated");
		return 1;
	}

	printf("Bruteforcing password...\n");
	gettimeofday(&_start_t, NULL);

	// Search for the password 
	int result = bruteforce_password();


	// Output result
	gettimeofday(&_end_t, NULL);
	unsigned long long sec = _end_t.tv_sec - _start_t.tv_sec;
	
	dur = (_end_t.tv_sec * 1000000 + _end_t.tv_usec) - (_start_t.tv_sec * 1000000 + _start_t.tv_usec);
    if (result){
		printf("\nPassword found! Password is: %s\n", password_solution);
	    printf("Time used: %ld usec\t", dur);
	    printf("Passwords: searched ~%d\t", result);
	    if (sec != 0)
	    	printf("tested per second: %llu\n", (unsigned long long )(result/sec));

	   	printf("\n\n");

	   	fflush(stdout);
    }
	return 0;
}
#include <stdio.h>		// fprintf(), fileno()
#include <string.h>		// strncmp()
#include <fcntl.h>		// open()
#include <sys/stat.h>		// open()
#include <sys/types.h>		// open(), lseek()
#include <unistd.h>		// lseek()
#include <stdlib.h>		// strtoull(), exit(), malloc(), free()
#include <errno.h>		// errno

/**********************************************************/

void usage(char* program) {
	fprintf(stderr, "Usage: %s [options]\n", program);
	fprintf(stderr, "Copies a chunk of data as specified by the offset and length.\n");
	fprintf(stderr, "Hexadecimal values (prepended with 0x) are valid.\n");
	fprintf(stderr, "    Options:\n");
	fprintf(stderr, "        -p offset   : sets the offset position of the input (default is 0)\n");
	fprintf(stderr, "        -s seek     : sets the seek position of the output (default is 0)\n");
	fprintf(stderr, "        -l length   : sets maximum length to copy (default is until EOF)\n");
	fprintf(stderr, "        -f file_in  : sets the input file (default is STDIN)\n");
	fprintf(stderr, "        -o file_out : sets the output file (default is STDOUT)\n");
	fprintf(stderr, "        -b size     : sets the I/O buffer size (default is 262144)\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "        -?          : this help message\n");
	exit(0);
}

/**********************************************************/

int main(int argc, char* argv[]) {
	int fdin, fdout;
	ssize_t bytes_read;
	ssize_t bytes_written;
	size_t size;
	unsigned char* buf = NULL;

	// options
	char opt;
	size_t offset = 0;
	size_t seek = 0;
	size_t length = -1;	// EOF (unsigned maximum)
	char* file_in = NULL;
	char* file_out = NULL;
	size_t buf_size = 262144;

	// file descriptor flags
	int in_flags = O_RDONLY;
	int out_flags = O_WRONLY | O_CREAT | O_TRUNC;

	// check options
	opterr = 0;
	while ((opt = getopt(argc, argv, "p:s:l:f:o:b:")) != -1) {
		switch(opt) {

			case 'p':	// offset
				if (strncmp(optarg, "0x", 2) == 0) {
					offset = strtoull(optarg + 2, 0, 16);
				}
				else {
					offset = strtoull(optarg, 0, 10);
				}
				break;

			case 's':	// seek
				if (strncmp(optarg, "0x", 2) == 0) {
					seek = strtoull(optarg + 2, 0, 16);
				}
				else {
					seek = strtoull(optarg, 0, 10);
				}

				// disable truncating the output file
				if (out_flags & O_TRUNC) {
					out_flags ^= O_TRUNC;
				}

				break;

			case 'l':	// length
				if (strncmp(optarg, "0x", 2) == 0) {
					length = strtoull(optarg + 2, 0, 16);
				}
				else {
					length = strtoull(optarg, 0, 10);
				}
				break;

			case 'f':	// file_in
				file_in = optarg;
				break;

			case 'o':	// file_out
				file_out = optarg;
				break;

			case 'b':	// buffer size
				if (strncmp(optarg, "0x", 2) == 0) {
					buf_size = strtoull(optarg + 2, 0, 16);
				}
				else {
					buf_size = strtoull(optarg, 0, 10);
				}
				break;

			case '?':
			default:
				usage(argv[0]);
				break;
		}
	}

	// invalid arguments
	if (optind != argc) usage(argv[0]);

	// cannot seek with STDOUT
	if (file_out == NULL && seek > 0) {
		fprintf(stderr, "Cannot seek when STDOUT is output file\n");
		exit(EPERM);
	}

	// allocate memory
	buf = (unsigned char*)malloc((size_t)buf_size);
	if (buf == NULL) {
		fprintf(stderr, "Could not allocate memory\n");
		exit(errno);
	}

	// open input file, or STDIN if not defined
	if (file_in != NULL) {
		fdin = open(file_in, in_flags, 0666);
		if (fdin < 0) {
			fprintf(stderr, "Could not open input file\n");
			exit(errno);
		}
	}
	else {
		fdin = fileno(stdin);
	}

	// open output file, or STDOUT if not defined
	if (file_out != NULL) {
		fdout = open(file_out, out_flags, 0666);
		if (fdout < 0) {
			fprintf(stderr, "Could not open output file\n");
			exit(errno);
		}
	}
	else {
		fdout = fileno(stdout);
	}

	// seek to input position
	if (lseek(fdin, offset, SEEK_SET) == -1) {
		// lseek() doesn't work on STDIN
		size = 0;
		while(size < offset) {
			if (size + buf_size > offset) {
				bytes_read = read(fdin, buf, offset - size);
			}
			else {
				bytes_read = read(fdin, buf, buf_size);
			}

			// EOF, error
			if (bytes_read == 0 || bytes_read < 0) {
				close(fdin);
				close(fdout);
				exit(0);
			}
			size += bytes_read;
		}
	}

	// seek to output position
	if (seek > 0) {
		if (lseek(fdout, seek, SEEK_SET) == -1) {
			fprintf(stderr, "Could not seek output file\n");
			exit(errno);
		}
	}

	// copy data
	size = 0;
	do {
		// read
		if (size + buf_size > length) {
			bytes_read = read(fdin, buf, length - size);
		}
		else {
			bytes_read = read(fdin, buf, buf_size);
		}

		// write
		if (bytes_read > 0) {
			bytes_written = write(fdout, buf, bytes_read);
			size += bytes_written;
		}

	} while(bytes_read > 0 && size < length);

	// close files
	close(fdin);
	close(fdout);

	// free memory
	free(buf);

	return 0;
}

/**********************************************************/

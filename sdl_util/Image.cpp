#include "sdl_util/Image.h"
#include <png/png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>

#define PNG_BYTES_TO_CHECK 8

#define Z_FILTERED            1
#define ZLIB_VERSION "1.2.8"


#define _POSIX_SOURCE 1


#define STDERR stdout   /* For DOS */


/* Known chunks that exist in pngtest.png must be supported or pngtest will fail
 * simply as a result of re-ordering them.  This may be fixed in 1.7
 *
 * pngtest allocates a single row buffer for each row and overwrites it,
 * therefore if the write side doesn't support the writing of interlaced images
 * nothing can be done for an interlaced image (and the code below will fail
 * horribly trying to write extra data after writing garbage).
 */
#if defined PNG_READ_SUPPORTED && /* else nothing can be done */\
   defined PNG_READ_bKGD_SUPPORTED &&\
   defined PNG_READ_cHRM_SUPPORTED &&\
   defined PNG_READ_gAMA_SUPPORTED &&\
   defined PNG_READ_oFFs_SUPPORTED &&\
   defined PNG_READ_pCAL_SUPPORTED &&\
   defined PNG_READ_pHYs_SUPPORTED &&\
   defined PNG_READ_sBIT_SUPPORTED &&\
   defined PNG_READ_sCAL_SUPPORTED &&\
   defined PNG_READ_sRGB_SUPPORTED &&\
   defined PNG_READ_sPLT_SUPPORTED &&\
   defined PNG_READ_tEXt_SUPPORTED &&\
   defined PNG_READ_tIME_SUPPORTED &&\
   defined PNG_READ_zTXt_SUPPORTED &&\
   (defined PNG_WRITE_INTERLACING_SUPPORTED || PNG_LIBPNG_VER >= 10700)


 /* Copied from pngpriv.h but only used in error messages below. */
#ifndef PNG_ZBUF_SIZE
#  define PNG_ZBUF_SIZE 8192
#endif
#define FCLOSE(file) fclose(file)

#ifndef PNG_STDIO_SUPPORTED
typedef FILE                * png_FILE_p;
#endif

/* Makes pngtest verbose so we can find problems. */
#ifndef PNG_DEBUG
#  define PNG_DEBUG 0
#endif

#if PNG_DEBUG > 1
#  define pngtest_debug(m)        ((void)fprintf(stderr, m "\n"))
#  define pngtest_debug1(m,p1)    ((void)fprintf(stderr, m "\n", p1))
#  define pngtest_debug2(m,p1,p2) ((void)fprintf(stderr, m "\n", p1, p2))
#else
#  define pngtest_debug(m)        ((void)0)
#  define pngtest_debug1(m,p1)    ((void)0)
#  define pngtest_debug2(m,p1,p2) ((void)0)
#endif

#if !PNG_DEBUG
#  define SINGLE_ROWBUF_ALLOC  /* Makes buffer overruns easier to nail */
#endif

#ifndef PNG_UNUSED
#  define PNG_UNUSED(param) (void)param;
#endif

/* Turn on CPU timing
#define PNGTEST_TIMING
*/

#ifndef PNG_FLOATING_POINT_SUPPORTED
#undef PNGTEST_TIMING
#endif

#ifdef PNGTEST_TIMING
static float t_start, t_stop, t_decode, t_encode, t_misc;
#include <time.h>
#endif

#ifdef PNG_TIME_RFC1123_SUPPORTED
#define PNG_tIME_STRING_LENGTH 29
static int tIME_chunk_present = 0;
static char tIME_string[PNG_tIME_STRING_LENGTH] = "tIME chunk is not present";

#if PNG_LIBPNG_VER < 10619
#define png_convert_to_rfc1123_buffer(ts, t) tIME_to_str(read_ptr, ts, t)

static int
tIME_to_str(png_structp png_ptr, png_charp ts, png_const_timep t)
{
	png_const_charp str = png_convert_to_rfc1123(png_ptr, t);

	if (str == NULL)
		return 0;

	strcpy(ts, str);
	return 1;
}
#endif /* older libpng */
#endif

static int verbose = 0;
static int strict = 0;
static int relaxed = 0;
static int xfail = 0;
static int unsupported_chunks = 0; /* chunk unsupported by libpng in input */
static int error_count = 0; /* count calls to png_error */
static int warning_count = 0; /* count calls to png_warning */

/* Define png_jmpbuf() in case we are using a pre-1.0.6 version of libpng */
#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) png_ptr->jmpbuf
#endif

/* Defines for unknown chunk handling if required. */
#ifndef PNG_HANDLE_CHUNK_ALWAYS
#  define PNG_HANDLE_CHUNK_ALWAYS       3
#endif
#ifndef PNG_HANDLE_CHUNK_IF_SAFE
#  define PNG_HANDLE_CHUNK_IF_SAFE      2
#endif

/* Utility to save typing/errors, the argument must be a name */
#define MEMZERO(var) ((void)memset(&var, 0, sizeof var))

/* Example of using row callbacks to make a simple progress meter */
static int status_pass = 1;
static int status_dots_requested = 0;
static int status_dots = 1;

static void PNGCBAPI
read_row_callback(png_structp png_ptr, png_uint_32 row_number, int pass)
{
	if (png_ptr == NULL || row_number > PNG_UINT_31_MAX)
		return;

	if (status_pass != pass)
	{
		fprintf(stdout, "\n Pass %d: ", pass);
		status_pass = pass;
		status_dots = 31;
	}

	status_dots--;

	if (status_dots == 0)
	{
		fprintf(stdout, "\n         ");
		status_dots = 30;
	}

	fprintf(stdout, "r");
}

#ifdef PNG_WRITE_SUPPORTED
static void PNGCBAPI
write_row_callback(png_structp png_ptr, png_uint_32 row_number, int pass)
{
	if (png_ptr == NULL || row_number > PNG_UINT_31_MAX || pass > 7)
		return;

	fprintf(stdout, "w");
}
#endif


#ifdef PNG_READ_USER_TRANSFORM_SUPPORTED
/* Example of using a user transform callback (doesn't do anything at present).
 */
static void PNGCBAPI
read_user_callback(png_structp png_ptr, png_row_infop row_info, png_bytep data)
{
	PNG_UNUSED(png_ptr)
		PNG_UNUSED(row_info)
		PNG_UNUSED(data)
}
#endif

#ifdef PNG_WRITE_USER_TRANSFORM_SUPPORTED
/* Example of using user transform callback (we don't transform anything,
 * but merely count the zero samples)
 */

static png_uint_32 zero_samples;

static void PNGCBAPI
count_zero_samples(png_structp png_ptr, png_row_infop row_info, png_bytep data)
{
	png_bytep dp = data;
	if (png_ptr == NULL)
		return;

	/* Contents of row_info:
	 *  png_uint_32 width      width of row
	 *  png_uint_32 rowbytes   number of bytes in row
	 *  png_byte color_type    color type of pixels
	 *  png_byte bit_depth     bit depth of samples
	 *  png_byte channels      number of channels (1-4)
	 *  png_byte pixel_depth   bits per pixel (depth*channels)
	 */

	 /* Counts the number of zero samples (or zero pixels if color_type is 3 */

	if (row_info->color_type == 0 || row_info->color_type == 3)
	{
		int pos = 0;
		png_uint_32 n, nstop;

		for (n = 0, nstop = row_info->width; n < nstop; n++)
		{
			if (row_info->bit_depth == 1)
			{
				if (((*dp << pos++) & 0x80) == 0)
					zero_samples++;

				if (pos == 8)
				{
					pos = 0;
					dp++;
				}
			}

			if (row_info->bit_depth == 2)
			{
				if (((*dp << (pos += 2)) & 0xc0) == 0)
					zero_samples++;

				if (pos == 8)
				{
					pos = 0;
					dp++;
				}
			}

			if (row_info->bit_depth == 4)
			{
				if (((*dp << (pos += 4)) & 0xf0) == 0)
					zero_samples++;

				if (pos == 8)
				{
					pos = 0;
					dp++;
				}
			}

			if (row_info->bit_depth == 8)
				if (*dp++ == 0)
					zero_samples++;

			if (row_info->bit_depth == 16)
			{
				if ((*dp | *(dp + 1)) == 0)
					zero_samples++;
				dp += 2;
			}
		}
	}
	else /* Other color types */
	{
		png_uint_32 n, nstop;
		int channel;
		int color_channels = row_info->channels;
		if (row_info->color_type > 3)
			color_channels--;

		for (n = 0, nstop = row_info->width; n < nstop; n++)
		{
			for (channel = 0; channel < color_channels; channel++)
			{
				if (row_info->bit_depth == 8)
					if (*dp++ == 0)
						zero_samples++;

				if (row_info->bit_depth == 16)
				{
					if ((*dp | *(dp + 1)) == 0)
						zero_samples++;

					dp += 2;
				}
			}
			if (row_info->color_type > 3)
			{
				dp++;
				if (row_info->bit_depth == 16)
					dp++;
			}
		}
	}
}
#endif /* WRITE_USER_TRANSFORM */

#ifndef PNG_STDIO_SUPPORTED
/* START of code to validate stdio-free compilation */
/* These copies of the default read/write functions come from pngrio.c and
 * pngwio.c.  They allow "don't include stdio" testing of the library.
 * This is the function that does the actual reading of data.  If you are
 * not reading from a standard C stream, you should create a replacement
 * read_data function and use it at run time with png_set_read_fn(), rather
 * than changing the library.
 */

#ifdef PNG_IO_STATE_SUPPORTED
void
pngtest_check_io_state(png_structp png_ptr, size_t data_length,
	png_uint_32 io_op);
void
pngtest_check_io_state(png_structp png_ptr, size_t data_length,
	png_uint_32 io_op)
{
	png_uint_32 io_state = png_get_io_state(png_ptr);
	int err = 0;

	/* Check if the current operation (reading / writing) is as expected. */
	if ((io_state & PNG_IO_MASK_OP) != io_op)
		png_error(png_ptr, "Incorrect operation in I/O state");

	/* Check if the buffer size specific to the current location
	 * (file signature / header / data / crc) is as expected.
	 */
	switch (io_state & PNG_IO_MASK_LOC)
	{
	case PNG_IO_SIGNATURE:
		if (data_length > 8)
			err = 1;
		break;
	case PNG_IO_CHUNK_HDR:
		if (data_length != 8)
			err = 1;
		break;
	case PNG_IO_CHUNK_DATA:
		break;  /* no restrictions here */
	case PNG_IO_CHUNK_CRC:
		if (data_length != 4)
			err = 1;
		break;
	default:
		err = 1;  /* uninitialized */
	}
	if (err != 0)
		png_error(png_ptr, "Bad I/O state or buffer size");
}
#endif

static void PNGCBAPI
pngtest_read_data(png_structp png_ptr, png_bytep data, size_t length)
{
	size_t check = 0;
	png_voidp io_ptr;

	/* fread() returns 0 on error, so it is OK to store this in a size_t
	 * instead of an int, which is what fread() actually returns.
	 */
	io_ptr = png_get_io_ptr(png_ptr);
	if (io_ptr != NULL)
	{
		check = fread(data, 1, length, (png_FILE_p)io_ptr);
	}

	if (check != length)
	{
		png_error(png_ptr, "Read Error");
	}

#ifdef PNG_IO_STATE_SUPPORTED
	pngtest_check_io_state(png_ptr, length, PNG_IO_READING);
#endif
}

#ifdef PNG_WRITE_FLUSH_SUPPORTED
static void PNGCBAPI
pngtest_flush(png_structp png_ptr)
{
	/* Do nothing; fflush() is said to be just a waste of energy. */
	PNG_UNUSED(png_ptr)   /* Stifle compiler warning */
}
#endif

/* This is the function that does the actual writing of data.  If you are
 * not writing to a standard C stream, you should create a replacement
 * write_data function and use it at run time with png_set_write_fn(), rather
 * than changing the library.
 */
static void PNGCBAPI
pngtest_write_data(png_structp png_ptr, png_bytep data, size_t length)
{
	size_t check;

	check = fwrite(data, 1, length, (png_FILE_p)png_get_io_ptr(png_ptr));

	if (check != length)
	{
		png_error(png_ptr, "Write Error");
	}

#ifdef PNG_IO_STATE_SUPPORTED
	pngtest_check_io_state(png_ptr, length, PNG_IO_WRITING);
#endif
}
#endif /* !STDIO */

/* This function is called when there is a warning, but the library thinks
 * it can continue anyway.  Replacement functions don't have to do anything
 * here if you don't want to.  In the default configuration, png_ptr is
 * not used, but it is passed in case it may be useful.
 */
typedef struct
{
	const char *file_name;
}  pngtest_error_parameters;

static void PNGCBAPI
pngtest_warning(png_structp png_ptr, png_const_charp message)
{
	const char *name = "UNKNOWN (ERROR!)";
	pngtest_error_parameters *test =
		(pngtest_error_parameters*)png_get_error_ptr(png_ptr);

	++warning_count;

	if (test != NULL && test->file_name != NULL)
		name = test->file_name;

	fprintf(STDERR, "\n%s: libpng warning: %s\n", name, message);
}

/* This is the default error handling function.  Note that replacements for
 * this function MUST NOT RETURN, or the program will likely crash.  This
 * function is used by default, or if the program supplies NULL for the
 * error function pointer in png_set_error_fn().
 */
static void PNGCBAPI
pngtest_error(png_structp png_ptr, png_const_charp message)
{
	++error_count;

	pngtest_warning(png_ptr, message);
	/* We can return because png_error calls the default handler, which is
	 * actually OK in this case.
	 */
}

/* END of code to validate stdio-free compilation */

/* START of code to validate memory allocation and deallocation */
#if defined(PNG_USER_MEM_SUPPORTED) && PNG_DEBUG

/* Allocate memory.  For reasonable files, size should never exceed
 * 64K.  However, zlib may allocate more than 64K if you don't tell
 * it not to.  See zconf.h and png.h for more information.  zlib does
 * need to allocate exactly 64K, so whatever you call here must
 * have the ability to do that.
 *
 * This piece of code can be compiled to validate max 64K allocations
 * by setting MAXSEG_64K in zlib zconf.h *or* PNG_MAX_MALLOC_64K.
 */
typedef struct memory_information
{
	png_alloc_size_t          size;
	png_voidp                 pointer;
	struct memory_information *next;
} memory_information;
typedef memory_information *memory_infop;

static memory_infop pinformation = NULL;
static int current_allocation = 0;
static int maximum_allocation = 0;
static int total_allocation = 0;
static int num_allocations = 0;

png_voidp PNGCBAPI png_debug_malloc PNGARG((png_structp png_ptr,
	png_alloc_size_t size));
void PNGCBAPI png_debug_free PNGARG((png_structp png_ptr, png_voidp ptr));

png_voidp
PNGCBAPI png_debug_malloc(png_structp png_ptr, png_alloc_size_t size)
{

	/* png_malloc has already tested for NULL; png_create_struct calls
	 * png_debug_malloc directly, with png_ptr == NULL which is OK
	 */

	if (size == 0)
		return (NULL);

	/* This calls the library allocator twice, once to get the requested
	   buffer and once to get a new free list entry. */
	{
		/* Disable malloc_fn and free_fn */
		memory_infop pinfo;
		png_set_mem_fn(png_ptr, NULL, NULL, NULL);
		pinfo = (memory_infop)png_malloc(png_ptr,
			(sizeof *pinfo));
		pinfo->size = size;
		current_allocation += size;
		total_allocation += size;
		num_allocations++;

		if (current_allocation > maximum_allocation)
			maximum_allocation = current_allocation;

		pinfo->pointer = png_malloc(png_ptr, size);
		/* Restore malloc_fn and free_fn */

		png_set_mem_fn(png_ptr,
			NULL, png_debug_malloc, png_debug_free);

		if (size != 0 && pinfo->pointer == NULL)
		{
			current_allocation -= size;
			total_allocation -= size;
			png_error(png_ptr,
				"out of memory in pngtest->png_debug_malloc");
		}

		pinfo->next = pinformation;
		pinformation = pinfo;
		/* Make sure the caller isn't assuming zeroed memory. */
		memset(pinfo->pointer, 0xdd, pinfo->size);

		if (verbose != 0)
			printf("png_malloc %lu bytes at %p\n", (unsigned long)size,
				pinfo->pointer);

		return (png_voidp)(pinfo->pointer);
	}
}

/* Free a pointer.  It is removed from the list at the same time. */
void PNGCBAPI
png_debug_free(png_structp png_ptr, png_voidp ptr)
{
	if (png_ptr == NULL)
		fprintf(STDERR, "NULL pointer to png_debug_free.\n");

	if (ptr == 0)
	{
#if 0 /* This happens all the time. */
		fprintf(STDERR, "WARNING: freeing NULL pointer\n");
#endif
		return;
	}

	/* Unlink the element from the list. */
	if (pinformation != NULL)
	{
		memory_infop *ppinfo = &pinformation;

		for (;;)
		{
			memory_infop pinfo = *ppinfo;

			if (pinfo->pointer == ptr)
			{
				*ppinfo = pinfo->next;
				current_allocation -= pinfo->size;
				if (current_allocation < 0)
					fprintf(STDERR, "Duplicate free of memory\n");
				/* We must free the list element too, but first kill
				   the memory that is to be freed. */
				memset(ptr, 0x55, pinfo->size);
				free(pinfo);
				pinfo = NULL;
				break;
			}

			if (pinfo->next == NULL)
			{
				fprintf(STDERR, "Pointer %p not found\n", ptr);
				break;
			}

			ppinfo = &pinfo->next;
		}
	}

	/* Finally free the data. */
	if (verbose != 0)
		printf("Freeing %p\n", ptr);

	if (ptr != NULL)
		free(ptr);
	ptr = NULL;
}
#endif /* USER_MEM && DEBUG */
/* END of code to test memory allocation/deallocation */


#ifdef PNG_READ_USER_CHUNKS_SUPPORTED
/* Demonstration of user chunk support of the sTER and vpAg chunks */

/* (sTER is a public chunk not yet known by libpng.  vpAg is a private
chunk used in ImageMagick to store "virtual page" size).  */

static struct user_chunk_data
{
	png_const_infop info_ptr;
	png_uint_32     vpAg_width, vpAg_height;
	png_byte        vpAg_units;
	png_byte        sTER_mode;
	int             location[2];
}
user_chunk_data;

/* Used for location and order; zero means nothing. */
#define have_sTER   0x01
#define have_vpAg   0x02
#define before_PLTE 0x10
#define before_IDAT 0x20
#define after_IDAT  0x40

static void
init_callback_info(png_const_infop info_ptr)
{
	MEMZERO(user_chunk_data);
	user_chunk_data.info_ptr = info_ptr;
}

static int
set_location(png_structp png_ptr, struct user_chunk_data *data, int what)
{
	int location;

	if ((data->location[0] & what) != 0 || (data->location[1] & what) != 0)
		return 0; /* already have one of these */

	 /* Find where we are (the code below zeroes info_ptr to indicate that the
	  * chunks before the first IDAT have been read.)
	  */
	if (data->info_ptr == NULL) /* after IDAT */
		location = what | after_IDAT;

	else if (png_get_valid(png_ptr, data->info_ptr, PNG_INFO_PLTE) != 0)
		location = what | before_IDAT;

	else
		location = what | before_PLTE;

	if (data->location[0] == 0)
		data->location[0] = location;

	else
		data->location[1] = location;

	return 1; /* handled */
}

static int PNGCBAPI
read_user_chunk_callback(png_struct *png_ptr, png_unknown_chunkp chunk)
{
	struct user_chunk_data *my_user_chunk_data =
		(struct user_chunk_data*)png_get_user_chunk_ptr(png_ptr);

	if (my_user_chunk_data == NULL)
		png_error(png_ptr, "lost user chunk pointer");

	/* Return one of the following:
	 *    return (-n);  chunk had an error
	 *    return (0);  did not recognize
	 *    return (n);  success
	 *
	 * The unknown chunk structure contains the chunk data:
	 * png_byte name[5];
	 * png_byte *data;
	 * size_t size;
	 *
	 * Note that libpng has already taken care of the CRC handling.
	 */

	if (chunk->name[0] == 115 && chunk->name[1] == 84 &&     /* s  T */
		chunk->name[2] == 69 && chunk->name[3] == 82)       /* E  R */
	{
		/* Found sTER chunk */
		if (chunk->size != 1)
			return (-1); /* Error return */

		if (chunk->data[0] != 0 && chunk->data[0] != 1)
			return (-1);  /* Invalid mode */

		if (set_location(png_ptr, my_user_chunk_data, have_sTER) != 0)
		{
			my_user_chunk_data->sTER_mode = chunk->data[0];
			return (1);
		}

		else
			return (0); /* duplicate sTER - give it to libpng */
	}

	if (chunk->name[0] != 118 || chunk->name[1] != 112 ||    /* v  p */
		chunk->name[2] != 65 || chunk->name[3] != 103)      /* A  g */
		return (0); /* Did not recognize */

	 /* Found ImageMagick vpAg chunk */

	if (chunk->size != 9)
		return (-1); /* Error return */

	if (set_location(png_ptr, my_user_chunk_data, have_vpAg) == 0)
		return (0);  /* duplicate vpAg */

	my_user_chunk_data->vpAg_width = png_get_uint_31(png_ptr, chunk->data);
	my_user_chunk_data->vpAg_height = png_get_uint_31(png_ptr, chunk->data + 4);
	my_user_chunk_data->vpAg_units = chunk->data[8];

	return (1);
}

#ifdef PNG_WRITE_SUPPORTED
static void
write_sTER_chunk(png_structp write_ptr)
{
	png_byte sTER[5] = { 115,  84,  69,  82, '\0' };

	if (verbose != 0)
		fprintf(STDERR, "\n stereo mode = %d\n", user_chunk_data.sTER_mode);

	png_write_chunk(write_ptr, sTER, &user_chunk_data.sTER_mode, 1);
}

static void
write_vpAg_chunk(png_structp write_ptr)
{
	png_byte vpAg[5] = { 118, 112,  65, 103, '\0' };

	png_byte vpag_chunk_data[9];

	if (verbose != 0)
		fprintf(STDERR, " vpAg = %lu x %lu, units = %d\n",
		(unsigned long)user_chunk_data.vpAg_width,
			(unsigned long)user_chunk_data.vpAg_height,
			user_chunk_data.vpAg_units);

	png_save_uint_32(vpag_chunk_data, user_chunk_data.vpAg_width);
	png_save_uint_32(vpag_chunk_data + 4, user_chunk_data.vpAg_height);
	vpag_chunk_data[8] = user_chunk_data.vpAg_units;
	png_write_chunk(write_ptr, vpAg, vpag_chunk_data, 9);
}

static void
write_chunks(png_structp write_ptr, int location)
{
	int i;

	/* Notice that this preserves the original chunk order, however chunks
	 * intercepted by the callback will be written *after* chunks passed to
	 * libpng.  This will actually reverse a pair of sTER chunks or a pair of
	 * vpAg chunks, resulting in an error later.  This is not worth worrying
	 * about - the chunks should not be duplicated!
	 */
	for (i = 0; i < 2; ++i)
	{
		if (user_chunk_data.location[i] == (location | have_sTER))
			write_sTER_chunk(write_ptr);

		else if (user_chunk_data.location[i] == (location | have_vpAg))
			write_vpAg_chunk(write_ptr);
	}
}
#endif /* WRITE */
#else /* !READ_USER_CHUNKS */
#  define write_chunks(pp,loc) ((void)0)
#endif
/* END of code to demonstrate user chunk support */

/* START of code to check that libpng has the required text support; this only
 * checks for the write support because if read support is missing the chunk
 * will simply not be reported back to pngtest.
 */
#ifdef PNG_TEXT_SUPPORTED
static void
pngtest_check_text_support(png_structp png_ptr, png_textp text_ptr,
	int num_text)
{
	while (num_text > 0)
	{
		switch (text_ptr[--num_text].compression)
		{
		case PNG_TEXT_COMPRESSION_NONE:
			break;

		case PNG_TEXT_COMPRESSION_zTXt:
#           ifndef PNG_WRITE_zTXt_SUPPORTED
			++unsupported_chunks;
			/* In libpng 1.7 this now does an app-error, so stop it: */
			text_ptr[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
#           endif
			break;

		case PNG_ITXT_COMPRESSION_NONE:
		case PNG_ITXT_COMPRESSION_zTXt:
#           ifndef PNG_WRITE_iTXt_SUPPORTED
			++unsupported_chunks;
			text_ptr[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
#           endif
			break;

		default:
			/* This is an error */
			png_error(png_ptr, "invalid text chunk compression field");
			break;
		}
	}
}
#endif
/* END of code to check that libpng has the required text support */

/* Test one file */
int Image::test_one_file(const char *inname, const char *outname)
{
	static png_FILE_p fpin;
	static png_FILE_p fpout;  /* "static" prevents setjmp corruption */
	pngtest_error_parameters error_parameters;
	png_structp read_ptr;
	png_infop read_info_ptr, end_info_ptr;
#ifdef PNG_WRITE_SUPPORTED
	png_structp write_ptr;
	png_infop write_info_ptr;
	png_infop write_end_info_ptr;
#ifdef PNG_WRITE_FILTER_SUPPORTED
	int interlace_preserved = 1;
#endif /* WRITE_FILTER */
#else /* !WRITE */
	png_structp write_ptr = NULL;
	png_infop write_info_ptr = NULL;
	png_infop write_end_info_ptr = NULL;
#endif /* !WRITE */
	png_bytep row_buf;
	png_uint_32 y;
	png_uint_32 width, height;
	volatile int num_passes;
	int pass;
	int bit_depth, color_type;

	row_buf = NULL;
	error_parameters.file_name = inname;

	if ((fpin = fopen(inname, "rb")) == NULL)
	{
		fprintf(STDERR, "Could not find input file %s\n", inname);
		return (1);
	}

	if ((fpout = fopen(outname, "wb")) == NULL)
	{
		fprintf(STDERR, "Could not open output file %s\n", outname);
		FCLOSE(fpin);
		return (1);
	}

	pngtest_debug("Allocating read and write structures");
#if defined(PNG_USER_MEM_SUPPORTED) && PNG_DEBUG
	read_ptr =
		png_create_read_struct_2(PNG_LIBPNG_VER_STRING, NULL,
			NULL, NULL, NULL, png_debug_malloc, png_debug_free);
#else
	read_ptr =
		png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
#endif
	png_set_error_fn(read_ptr, &error_parameters, pngtest_error,
		pngtest_warning);

#ifdef PNG_WRITE_SUPPORTED
#if defined(PNG_USER_MEM_SUPPORTED) && PNG_DEBUG
	write_ptr =
		png_create_write_struct_2(PNG_LIBPNG_VER_STRING, NULL,
			NULL, NULL, NULL, png_debug_malloc, png_debug_free);
#else
	write_ptr =
		png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
#endif
	png_set_error_fn(write_ptr, &error_parameters, pngtest_error,
		pngtest_warning);
#endif
	pngtest_debug("Allocating read_info, write_info and end_info structures");
	read_info_ptr = png_create_info_struct(read_ptr);
	end_info_ptr = png_create_info_struct(read_ptr);
#ifdef PNG_WRITE_SUPPORTED
	write_info_ptr = png_create_info_struct(write_ptr);
	write_end_info_ptr = png_create_info_struct(write_ptr);
#endif

#ifdef PNG_READ_USER_CHUNKS_SUPPORTED
	init_callback_info(read_info_ptr);
	png_set_read_user_chunk_fn(read_ptr, &user_chunk_data,
		read_user_chunk_callback);
#endif

#ifdef PNG_SETJMP_SUPPORTED
	pngtest_debug("Setting jmpbuf for read struct");
	if (setjmp(png_jmpbuf(read_ptr)))
	{
		fprintf(STDERR, "%s -> %s: libpng read error\n", inname, outname);
		png_free(read_ptr, row_buf);
		row_buf = NULL;
		if (verbose != 0)
			fprintf(STDERR, "   destroy read structs\n");
		png_destroy_read_struct(&read_ptr, &read_info_ptr, &end_info_ptr);
#ifdef PNG_WRITE_SUPPORTED
		if (verbose != 0)
			fprintf(STDERR, "   destroy write structs\n");
		png_destroy_info_struct(write_ptr, &write_end_info_ptr);
		png_destroy_write_struct(&write_ptr, &write_info_ptr);
#endif
		FCLOSE(fpin);
		FCLOSE(fpout);
		return (1);
	}

#ifdef PNG_WRITE_SUPPORTED
	pngtest_debug("Setting jmpbuf for write struct");

	if (setjmp(png_jmpbuf(write_ptr)))
	{
		fprintf(STDERR, "%s -> %s: libpng write error\n", inname, outname);
		png_free(read_ptr, row_buf);
		row_buf = NULL;
		if (verbose != 0)
			fprintf(STDERR, "   destroying read structs\n");
		png_destroy_read_struct(&read_ptr, &read_info_ptr, &end_info_ptr);
		if (verbose != 0)
			fprintf(STDERR, "   destroying write structs\n");
		png_destroy_info_struct(write_ptr, &write_end_info_ptr);
		png_destroy_write_struct(&write_ptr, &write_info_ptr);
		FCLOSE(fpin);
		FCLOSE(fpout);
		return (1);
	}
#endif
#endif

#ifdef PNG_BENIGN_ERRORS_SUPPORTED
	if (strict != 0)
	{
		/* Treat png_benign_error() as errors on read */
		png_set_benign_errors(read_ptr, 0);

# ifdef PNG_WRITE_SUPPORTED
		/* Treat them as errors on write */
		png_set_benign_errors(write_ptr, 0);
# endif

		/* if strict is not set, then app warnings and errors are treated as
		 * warnings in release builds, but not in unstable builds; this can be
		 * changed with '--relaxed'.
		 */
	}

	else if (relaxed != 0)
	{
		/* Allow application (pngtest) errors and warnings to pass */
		png_set_benign_errors(read_ptr, 1);

		/* Turn off CRC checking while reading */
		png_set_crc_action(read_ptr, PNG_CRC_QUIET_USE, PNG_CRC_QUIET_USE);

#ifdef PNG_IGNORE_ADLER32
		/* Turn off ADLER32 checking while reading */
		png_set_option(read_ptr, PNG_IGNORE_ADLER32, PNG_OPTION_ON);
#endif

# ifdef PNG_WRITE_SUPPORTED
		png_set_benign_errors(write_ptr, 1);
# endif

	}
#endif /* BENIGN_ERRORS */

	pngtest_debug("Initializing input and output streams");
#ifdef PNG_STDIO_SUPPORTED
	png_init_io(read_ptr, fpin);
#  ifdef PNG_WRITE_SUPPORTED
	png_init_io(write_ptr, fpout);
#  endif
#else
	png_set_read_fn(read_ptr, (png_voidp)fpin, pngtest_read_data);
#  ifdef PNG_WRITE_SUPPORTED
	png_set_write_fn(write_ptr, (png_voidp)fpout, pngtest_write_data,
#    ifdef PNG_WRITE_FLUSH_SUPPORTED
		pngtest_flush);
#    else
		NULL);
#    endif
#  endif
#endif

	if (status_dots_requested == 1)
	{
#ifdef PNG_WRITE_SUPPORTED
		png_set_write_status_fn(write_ptr, write_row_callback);
#endif
		png_set_read_status_fn(read_ptr, read_row_callback);
	}
	else
	{
#ifdef PNG_WRITE_SUPPORTED
		png_set_write_status_fn(write_ptr, NULL);
#endif
		png_set_read_status_fn(read_ptr, NULL);
	}

#ifdef PNG_READ_USER_TRANSFORM_SUPPORTED
	png_set_read_user_transform_fn(read_ptr, read_user_callback);
#endif
#ifdef PNG_WRITE_USER_TRANSFORM_SUPPORTED
	zero_samples = 0;
	png_set_write_user_transform_fn(write_ptr, count_zero_samples);
#endif

#ifdef PNG_SET_UNKNOWN_CHUNKS_SUPPORTED
	/* Preserve all the unknown chunks, if possible.  If this is disabled then,
	 * even if the png_{get,set}_unknown_chunks stuff is enabled, we can't use
	 * libpng to *save* the unknown chunks on read (because we can't switch the
	 * save option on!)
	 *
	 * Notice that if SET_UNKNOWN_CHUNKS is *not* supported read will discard all
	 * unknown chunks and write will write them all.
	 */
#ifdef PNG_SAVE_UNKNOWN_CHUNKS_SUPPORTED
	png_set_keep_unknown_chunks(read_ptr, PNG_HANDLE_CHUNK_ALWAYS,
		NULL, 0);
#endif
#ifdef PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED
	png_set_keep_unknown_chunks(write_ptr, PNG_HANDLE_CHUNK_ALWAYS,
		NULL, 0);
#endif
#endif

	pngtest_debug("Reading info struct");
	png_read_info(read_ptr, read_info_ptr);

#ifdef PNG_READ_USER_CHUNKS_SUPPORTED
	/* This is a bit of a hack; there is no obvious way in the callback function
	 * to determine that the chunks before the first IDAT have been read, so
	 * remove the info_ptr (which is only used to determine position relative to
	 * PLTE) here to indicate that we are after the IDAT.
	 */
	user_chunk_data.info_ptr = NULL;
#endif

	pngtest_debug("Transferring info struct");
	{
		int interlace_type, compression_type, filter_type;

		if (png_get_IHDR(read_ptr, read_info_ptr, &width, &height, &bit_depth,
			&color_type, &interlace_type, &compression_type, &filter_type) != 0)
		{
			png_set_IHDR(write_ptr, write_info_ptr, width, height, bit_depth,
				color_type, interlace_type, compression_type, filter_type);
			/* num_passes may not be available below if interlace support is not
			 * provided by libpng for both read and write.
			 */
			switch (interlace_type)
			{
			case PNG_INTERLACE_NONE:
				num_passes = 1;
				break;

			case PNG_INTERLACE_ADAM7:
				num_passes = 7;
				break;

			default:
				png_error(read_ptr, "invalid interlace type");
				/*NOT REACHED*/
			}
		}
		else
			png_error(read_ptr, "png_get_IHDR failed");
	}
#ifdef PNG_FIXED_POINT_SUPPORTED
#ifdef PNG_cHRM_SUPPORTED
	{
		png_fixed_point white_x, white_y, red_x, red_y, green_x, green_y, blue_x,
			blue_y;

		if (png_get_cHRM_fixed(read_ptr, read_info_ptr, &white_x, &white_y,
			&red_x, &red_y, &green_x, &green_y, &blue_x, &blue_y) != 0)
		{
			png_set_cHRM_fixed(write_ptr, write_info_ptr, white_x, white_y, red_x,
				red_y, green_x, green_y, blue_x, blue_y);
		}
	}
#endif
#ifdef PNG_gAMA_SUPPORTED
	{
		png_fixed_point gamma;
		if (png_get_gAMA_fixed(read_ptr, read_info_ptr, &gamma) != 0)
			png_set_gAMA_fixed(write_ptr, write_info_ptr, gamma);
	}
#endif
#else /* Use floating point versions */
#ifdef PNG_FLOATING_POINT_SUPPORTED
#ifdef PNG_cHRM_SUPPORTED
	{
		double white_x, white_y, red_x, red_y, green_x, green_y, blue_x,
			blue_y;

		if (png_get_cHRM(read_ptr, read_info_ptr, &white_x, &white_y, &red_x,
			&red_y, &green_x, &green_y, &blue_x, &blue_y) != 0)
		{
			png_set_cHRM(write_ptr, write_info_ptr, white_x, white_y, red_x,
				red_y, green_x, green_y, blue_x, blue_y);
		}
	}
#endif
#ifdef PNG_gAMA_SUPPORTED
	{
		double gamma;

		if (png_get_gAMA(read_ptr, read_info_ptr, &gamma) != 0)
			png_set_gAMA(write_ptr, write_info_ptr, gamma);
	}
#endif
#endif /* Floating point */
#endif /* Fixed point */
#ifdef PNG_iCCP_SUPPORTED
	{
		png_charp name;
		png_bytep profile;
		png_uint_32 proflen;
		int compression_type;

		if (png_get_iCCP(read_ptr, read_info_ptr, &name, &compression_type,
			&profile, &proflen) != 0)
		{
			png_set_iCCP(write_ptr, write_info_ptr, name, compression_type,
				profile, proflen);
		}
	}
#endif
#ifdef PNG_sRGB_SUPPORTED
	{
		int intent;

		if (png_get_sRGB(read_ptr, read_info_ptr, &intent) != 0)
			png_set_sRGB(write_ptr, write_info_ptr, intent);
	}
#endif
	{
		png_colorp palette;
		int num_palette;

		if (png_get_PLTE(read_ptr, read_info_ptr, &palette, &num_palette) != 0)
			png_set_PLTE(write_ptr, write_info_ptr, palette, num_palette);
	}
#ifdef PNG_bKGD_SUPPORTED
	{
		png_color_16p background;

		if (png_get_bKGD(read_ptr, read_info_ptr, &background) != 0)
		{
			png_set_bKGD(write_ptr, write_info_ptr, background);
		}
	}
#endif
#ifdef PNG_READ_eXIf_SUPPORTED
	{
		png_bytep exif = NULL;
		png_uint_32 exif_length;

		if (png_get_eXIf_1(read_ptr, read_info_ptr, &exif_length, &exif) != 0)
		{
			if (exif_length > 1)
				fprintf(STDERR, " eXIf type %c%c, %lu bytes\n", exif[0], exif[1],
				(unsigned long)exif_length);
# ifdef PNG_WRITE_eXIf_SUPPORTED
			png_set_eXIf_1(write_ptr, write_info_ptr, exif_length, exif);
# endif
		}
	}
#endif
#ifdef PNG_hIST_SUPPORTED
	{
		png_uint_16p hist;

		if (png_get_hIST(read_ptr, read_info_ptr, &hist) != 0)
			png_set_hIST(write_ptr, write_info_ptr, hist);
	}
#endif
#ifdef PNG_oFFs_SUPPORTED
	{
		png_int_32 offset_x, offset_y;
		int unit_type;

		if (png_get_oFFs(read_ptr, read_info_ptr, &offset_x, &offset_y,
			&unit_type) != 0)
		{
			png_set_oFFs(write_ptr, write_info_ptr, offset_x, offset_y, unit_type);
		}
	}
#endif
#ifdef PNG_pCAL_SUPPORTED
	{
		png_charp purpose, units;
		png_charpp params;
		png_int_32 X0, X1;
		int type, nparams;

		if (png_get_pCAL(read_ptr, read_info_ptr, &purpose, &X0, &X1, &type,
			&nparams, &units, &params) != 0)
		{
			png_set_pCAL(write_ptr, write_info_ptr, purpose, X0, X1, type,
				nparams, units, params);
		}
	}
#endif
#ifdef PNG_pHYs_SUPPORTED
	{
		png_uint_32 res_x, res_y;
		int unit_type;

		if (png_get_pHYs(read_ptr, read_info_ptr, &res_x, &res_y,
			&unit_type) != 0)
			png_set_pHYs(write_ptr, write_info_ptr, res_x, res_y, unit_type);
	}
#endif
#ifdef PNG_sBIT_SUPPORTED
	{
		png_color_8p sig_bit;

		if (png_get_sBIT(read_ptr, read_info_ptr, &sig_bit) != 0)
			png_set_sBIT(write_ptr, write_info_ptr, sig_bit);
	}
#endif
#ifdef PNG_sCAL_SUPPORTED
#if defined(PNG_FLOATING_POINT_SUPPORTED) && \
   defined(PNG_FLOATING_ARITHMETIC_SUPPORTED)
	{
		int unit;
		double scal_width, scal_height;

		if (png_get_sCAL(read_ptr, read_info_ptr, &unit, &scal_width,
			&scal_height) != 0)
		{
			png_set_sCAL(write_ptr, write_info_ptr, unit, scal_width, scal_height);
		}
	}
#else
#ifdef PNG_FIXED_POINT_SUPPORTED
	{
		int unit;
		png_charp scal_width, scal_height;

		if (png_get_sCAL_s(read_ptr, read_info_ptr, &unit, &scal_width,
			&scal_height) != 0)
		{
			png_set_sCAL_s(write_ptr, write_info_ptr, unit, scal_width,
				scal_height);
		}
	}
#endif
#endif
#endif

#ifdef PNG_sPLT_SUPPORTED
	{
		png_sPLT_tp entries;

		int num_entries = (int)png_get_sPLT(read_ptr, read_info_ptr, &entries);
		if (num_entries)
		{
			png_set_sPLT(write_ptr, write_info_ptr, entries, num_entries);
		}
	}
#endif

#ifdef PNG_TEXT_SUPPORTED
	{
		png_textp text_ptr;
		int num_text;

		if (png_get_text(read_ptr, read_info_ptr, &text_ptr, &num_text) > 0)
		{
			pngtest_debug1("Handling %d iTXt/tEXt/zTXt chunks", num_text);

			pngtest_check_text_support(read_ptr, text_ptr, num_text);

			if (verbose != 0)
			{
				int i;

				fprintf(STDERR, "\n");
				for (i = 0; i < num_text; i++)
				{
					fprintf(STDERR, "   Text compression[%d]=%d\n",
						i, text_ptr[i].compression);
				}
			}

			png_set_text(write_ptr, write_info_ptr, text_ptr, num_text);
		}
	}
#endif
#ifdef PNG_tIME_SUPPORTED
	{
		png_timep mod_time;

		if (png_get_tIME(read_ptr, read_info_ptr, &mod_time) != 0)
		{
			png_set_tIME(write_ptr, write_info_ptr, mod_time);
#ifdef PNG_TIME_RFC1123_SUPPORTED
			if (png_convert_to_rfc1123_buffer(tIME_string, mod_time) != 0)
				tIME_string[(sizeof tIME_string) - 1] = '\0';
			else
			{
				strncpy(tIME_string, "*** invalid time ***", (sizeof tIME_string));
				tIME_string[(sizeof tIME_string) - 1] = '\0';
			}

			tIME_chunk_present++;
#endif /* TIME_RFC1123 */
		}
	}
#endif
#ifdef PNG_tRNS_SUPPORTED
	{
		png_bytep trans_alpha;
		int num_trans;
		png_color_16p trans_color;

		if (png_get_tRNS(read_ptr, read_info_ptr, &trans_alpha, &num_trans,
			&trans_color) != 0)
		{
			int sample_max = (1 << bit_depth);
			/* libpng doesn't reject a tRNS chunk with out-of-range samples */
			if (!((color_type == PNG_COLOR_TYPE_GRAY &&
				(int)trans_color->gray > sample_max) ||
				(color_type == PNG_COLOR_TYPE_RGB &&
				((int)trans_color->red > sample_max ||
					(int)trans_color->green > sample_max ||
					(int)trans_color->blue > sample_max))))
				png_set_tRNS(write_ptr, write_info_ptr, trans_alpha, num_trans,
					trans_color);
		}
	}
#endif
#ifdef PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED
	{
		png_unknown_chunkp unknowns;
		int num_unknowns = png_get_unknown_chunks(read_ptr, read_info_ptr,
			&unknowns);

		if (num_unknowns != 0)
		{
			png_set_unknown_chunks(write_ptr, write_info_ptr, unknowns,
				num_unknowns);
#if PNG_LIBPNG_VER < 10600
			/* Copy the locations from the read_info_ptr.  The automatically
			 * generated locations in write_end_info_ptr are wrong prior to 1.6.0
			 * because they are reset from the write pointer (removed in 1.6.0).
			 */
			{
				int i;
				for (i = 0; i < num_unknowns; i++)
					png_set_unknown_chunk_location(write_ptr, write_info_ptr, i,
						unknowns[i].location);
			}
#endif
		}
	}
#endif

#ifdef PNG_WRITE_SUPPORTED
	pngtest_debug("Writing info struct");

	/* Write the info in two steps so that if we write the 'unknown' chunks here
	 * they go to the correct place.
	 */
	png_write_info_before_PLTE(write_ptr, write_info_ptr);

	write_chunks(write_ptr, before_PLTE); /* before PLTE */

	png_write_info(write_ptr, write_info_ptr);

	write_chunks(write_ptr, before_IDAT); /* after PLTE */

	png_write_info(write_ptr, write_end_info_ptr);

	write_chunks(write_ptr, after_IDAT); /* after IDAT */

#ifdef PNG_COMPRESSION_COMPAT
   /* Test the 'compatibility' setting here, if it is available. */
	png_set_compression(write_ptr, PNG_COMPRESSION_COMPAT);
#endif
#endif

#ifdef SINGLE_ROWBUF_ALLOC
	pngtest_debug("Allocating row buffer...");
	row_buf = (png_bytep)png_malloc(read_ptr,
		png_get_rowbytes(read_ptr, read_info_ptr));

	pngtest_debug1("\t%p", row_buf);
#endif /* SINGLE_ROWBUF_ALLOC */
	pngtest_debug("Writing row data");

#if defined(PNG_READ_INTERLACING_SUPPORTED) &&\
   defined(PNG_WRITE_INTERLACING_SUPPORTED)
	/* Both must be defined for libpng to be able to handle the interlace,
	 * otherwise it gets handled below by simply reading and writing the passes
	 * directly.
	 */
	if (png_set_interlace_handling(read_ptr) != num_passes)
		png_error(write_ptr,
			"png_set_interlace_handling(read): wrong pass count ");
	if (png_set_interlace_handling(write_ptr) != num_passes)
		png_error(write_ptr,
			"png_set_interlace_handling(write): wrong pass count ");
#else /* png_set_interlace_handling not called on either read or write */
#  define calc_pass_height
#endif /* not using libpng interlace handling */

#ifdef PNGTEST_TIMING
	t_stop = (float)clock();
	t_misc += (t_stop - t_start);
	t_start = t_stop;
#endif
	for (pass = 0; pass < num_passes; pass++)
	{
#     ifdef calc_pass_height
		png_uint_32 pass_height;

		if (num_passes == 7) /* interlaced */
		{
			if (PNG_PASS_COLS(width, pass) > 0)
				pass_height = PNG_PASS_ROWS(height, pass);

			else
				pass_height = 0;
		}

		else /* not interlaced */
			pass_height = height;
#     else
#        define pass_height height
#     endif

		pngtest_debug1("Writing row data for pass %d", pass);
		for (y = 0; y < pass_height; y++)
		{
#ifndef SINGLE_ROWBUF_ALLOC
			pngtest_debug2("Allocating row buffer (pass %d, y = %u)...", pass, y);

			row_buf = (png_bytep)png_malloc(read_ptr,
				png_get_rowbytes(read_ptr, read_info_ptr));

			pngtest_debug2("\t%p (%lu bytes)", row_buf,
				(unsigned long)png_get_rowbytes(read_ptr, read_info_ptr));

#endif /* !SINGLE_ROWBUF_ALLOC */
			png_read_rows(read_ptr, (png_bytepp)&row_buf, NULL, 1);

#ifdef PNG_WRITE_SUPPORTED
#ifdef PNGTEST_TIMING
			t_stop = (float)clock();
			t_decode += (t_stop - t_start);
			t_start = t_stop;
#endif
			png_write_rows(write_ptr, (png_bytepp)&row_buf, 1);
#ifdef PNGTEST_TIMING
			t_stop = (float)clock();
			t_encode += (t_stop - t_start);
			t_start = t_stop;
#endif
#endif /* WRITE */

#ifndef SINGLE_ROWBUF_ALLOC
			pngtest_debug2("Freeing row buffer (pass %d, y = %u)", pass, y);
			png_free(read_ptr, row_buf);
			row_buf = NULL;
#endif /* !SINGLE_ROWBUF_ALLOC */
		}
	}

#ifdef PNG_STORE_UNKNOWN_CHUNKS_SUPPORTED
#  ifdef PNG_READ_UNKNOWN_CHUNKS_SUPPORTED
	png_free_data(read_ptr, read_info_ptr, PNG_FREE_UNKN, -1);
#  endif
#  ifdef PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED
	png_free_data(write_ptr, write_info_ptr, PNG_FREE_UNKN, -1);
#  endif
#endif

	pngtest_debug("Reading and writing end_info data");

	png_read_end(read_ptr, end_info_ptr);
#ifdef PNG_TEXT_SUPPORTED
	{
		png_textp text_ptr;
		int num_text;

		if (png_get_text(read_ptr, end_info_ptr, &text_ptr, &num_text) > 0)
		{
			pngtest_debug1("Handling %d iTXt/tEXt/zTXt chunks", num_text);

			pngtest_check_text_support(read_ptr, text_ptr, num_text);

			if (verbose != 0)
			{
				int i;

				fprintf(STDERR, "\n");
				for (i = 0; i < num_text; i++)
				{
					fprintf(STDERR, "   Text compression[%d]=%d\n",
						i, text_ptr[i].compression);
				}
			}

			png_set_text(write_ptr, write_end_info_ptr, text_ptr, num_text);
		}
	}
#endif
#ifdef PNG_READ_eXIf_SUPPORTED
	{
		png_bytep exif = NULL;
		png_uint_32 exif_length;

		if (png_get_eXIf_1(read_ptr, end_info_ptr, &exif_length, &exif) != 0)
		{
			if (exif_length > 1)
				fprintf(STDERR, " eXIf type %c%c, %lu bytes\n", exif[0], exif[1],
				(unsigned long)exif_length);
# ifdef PNG_WRITE_eXIf_SUPPORTED
			png_set_eXIf_1(write_ptr, write_end_info_ptr, exif_length, exif);
# endif
		}
	}
#endif
#ifdef PNG_tIME_SUPPORTED
	{
		png_timep mod_time;

		if (png_get_tIME(read_ptr, end_info_ptr, &mod_time) != 0)
		{
			png_set_tIME(write_ptr, write_end_info_ptr, mod_time);
#ifdef PNG_TIME_RFC1123_SUPPORTED
			if (png_convert_to_rfc1123_buffer(tIME_string, mod_time) != 0)
				tIME_string[(sizeof tIME_string) - 1] = '\0';

			else
			{
				strncpy(tIME_string, "*** invalid time ***", sizeof tIME_string);
				tIME_string[(sizeof tIME_string) - 1] = '\0';
			}

			tIME_chunk_present++;
#endif /* TIME_RFC1123 */
		}
	}
#endif
#ifdef PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED
	{
		png_unknown_chunkp unknowns;
		int num_unknowns = png_get_unknown_chunks(read_ptr, end_info_ptr,
			&unknowns);

		if (num_unknowns != 0)
		{
			png_set_unknown_chunks(write_ptr, write_end_info_ptr, unknowns,
				num_unknowns);
#if PNG_LIBPNG_VER < 10600
			/* Copy the locations from the read_info_ptr.  The automatically
			 * generated locations in write_end_info_ptr are wrong prior to 1.6.0
			 * because they are reset from the write pointer (removed in 1.6.0).
			 */
			{
				int i;
				for (i = 0; i < num_unknowns; i++)
					png_set_unknown_chunk_location(write_ptr, write_end_info_ptr, i,
						unknowns[i].location);
			}
#endif
		}
	}
#endif

#ifdef PNG_WRITE_SUPPORTED
#ifdef PNG_WRITE_CUSTOMIZE_ZTXT_COMPRESSION_SUPPORTED
	/* Normally one would use Z_DEFAULT_STRATEGY for text compression.
	 * This is here just to make pngtest replicate the results from libpng
	 * versions prior to 1.5.4, and to test this new API.
	 */
	png_set_text_compression_strategy(write_ptr, Z_FILTERED);
#endif

	/* When the unknown vpAg/sTER chunks are written by pngtest the only way to
	 * do it is to write them *before* calling png_write_end.  When unknown
	 * chunks are written by libpng, however, they are written just before IEND.
	 * There seems to be no way round this, however vpAg/sTER are not expected
	 * after IDAT.
	 */
	write_chunks(write_ptr, after_IDAT);

	png_write_end(write_ptr, write_end_info_ptr);
#endif

#ifdef PNG_EASY_ACCESS_SUPPORTED
	if (verbose != 0)
	{
		png_uint_32 iwidth, iheight;
		iwidth = png_get_image_width(write_ptr, write_info_ptr);
		iheight = png_get_image_height(write_ptr, write_info_ptr);
		fprintf(STDERR, "\n Image width = %lu, height = %lu\n",
			(unsigned long)iwidth, (unsigned long)iheight);
	}
#endif

	pngtest_debug("Destroying data structs");
	const int BLOCK_SIZE = 4;//(rgba 4 bytes)
	png_bytepp row_pointers = &row_buf;
	int file_size = width * height * BLOCK_SIZE;
	this->nWidth = width;
	this->nHeight = height;
	unsigned char **pData = &this->imgData;
	*pData = new unsigned char[file_size];
	memset(&pData[0][0],0, file_size * sizeof(unsigned char));
	int pos = 0;
	//int wirteResult = png_image_write_to_file(&image, (char *)*cbData, 0/*convert_to_8bit*/,buffer, 0/*row_stride*/, NULL/*colormap*/);
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width * BLOCK_SIZE; x += BLOCK_SIZE)
		{
			(*pData)[pos++] = row_pointers[y][x + 0];//R
			(*pData)[pos++] = row_pointers[y][x + 1];//G
			(*pData)[pos++] = row_pointers[y][x + 2];//B
			(*pData)[pos++] = row_pointers[y][x + 3];//A
		}
	}
#ifdef SINGLE_ROWBUF_ALLOC
	pngtest_debug("destroying row_buf for read_ptr");
	png_free(read_ptr, row_buf);
	row_buf = NULL;
#endif /* SINGLE_ROWBUF_ALLOC */
	pngtest_debug("destroying read_ptr, read_info_ptr, end_info_ptr");
	png_destroy_read_struct(&read_ptr, &read_info_ptr, &end_info_ptr);
#ifdef PNG_WRITE_SUPPORTED
	pngtest_debug("destroying write_end_info_ptr");
	png_destroy_info_struct(write_ptr, &write_end_info_ptr);
	pngtest_debug("destroying write_ptr, write_info_ptr");
	png_destroy_write_struct(&write_ptr, &write_info_ptr);
#endif
	pngtest_debug("Destruction complete.");

	FCLOSE(fpin);
	FCLOSE(fpout);

	/* Summarize any warnings or errors and in 'strict' mode fail the test.
	 * Unsupported chunks can result in warnings, in that case ignore the strict
	 * setting, otherwise fail the test on warnings as well as errors.
	 */
	if (error_count > 0)
	{
		/* We don't really expect to get here because of the setjmp handling
		 * above, but this is safe.
		 */
		fprintf(STDERR, "\n  %s: %d libpng errors found (%d warnings)",
			inname, error_count, warning_count);

		if (strict != 0)
			return (1);
	}

#  ifdef PNG_WRITE_SUPPORTED
	/* If there is no write support nothing was written! */
	else if (unsupported_chunks > 0)
	{
		fprintf(STDERR, "\n  %s: unsupported chunks (%d)%s",
			inname, unsupported_chunks, strict ? ": IGNORED --strict!" : "");
	}
#  endif

	else if (warning_count > 0)
	{
		fprintf(STDERR, "\n  %s: %d libpng warnings found",
			inname, warning_count);

		if (strict != 0)
			return (1);
	}

	pngtest_debug("Opening files for comparison");
	if ((fpin = fopen(inname, "rb")) == NULL)
	{
		fprintf(STDERR, "Could not find file %s\n", inname);
		return (1);
	}

	if ((fpout = fopen(outname, "rb")) == NULL)
	{
		fprintf(STDERR, "Could not find file %s\n", outname);
		FCLOSE(fpin);
		return (1);
	}

#if defined (PNG_WRITE_SUPPORTED) /* else nothing was written */ &&\
    defined (PNG_WRITE_FILTER_SUPPORTED)
	if (interlace_preserved != 0) /* else the files will be changed */
	{
		for (;;)
		{
			static int wrote_question = 0;
			size_t num_in, num_out;
			char inbuf[256], outbuf[256];

			num_in = fread(inbuf, 1, sizeof inbuf, fpin);
			num_out = fread(outbuf, 1, sizeof outbuf, fpout);

			if (num_in != num_out)
			{
				fprintf(STDERR, "\nFiles %s and %s are of a different size\n",
					inname, outname);

				if (wrote_question == 0 && unsupported_chunks == 0)
				{
					fprintf(STDERR,
						"   Was %s written with the same maximum IDAT"
						" chunk size (%d bytes),",
						inname, PNG_ZBUF_SIZE);
					fprintf(STDERR,
						"\n   filtering heuristic (libpng default), compression");
					fprintf(STDERR,
						" level (zlib default),\n   and zlib version (%s)?\n\n",
						ZLIB_VERSION);
					wrote_question = 1;
				}

				FCLOSE(fpin);
				FCLOSE(fpout);

				if (strict != 0 && unsupported_chunks == 0)
					return (1);

				else
					return (0);
			}

			if (num_in == 0)
				break;

			if (memcmp(inbuf, outbuf, num_in))
			{
				fprintf(STDERR, "\nFiles %s and %s are different\n", inname,
					outname);

				if (wrote_question == 0 && unsupported_chunks == 0)
				{
					fprintf(STDERR,
						"   Was %s written with the same maximum"
						" IDAT chunk size (%d bytes),",
						inname, PNG_ZBUF_SIZE);
					fprintf(STDERR,
						"\n   filtering heuristic (libpng default), compression");
					fprintf(STDERR,
						" level (zlib default),\n   and zlib version (%s)?\n\n",
						ZLIB_VERSION);
					wrote_question = 1;
				}

				FCLOSE(fpin);
				FCLOSE(fpout);

				/* NOTE: the unsupported_chunks escape is permitted here because
				 * unsupported text chunk compression will result in the compression
				 * mode being changed (to NONE) yet, in the test case, the result
				 * can be exactly the same size!
				 */
				if (strict != 0 && unsupported_chunks == 0)
					return (1);

				else
					return (0);
			}
		}
	}
#endif /* WRITE && WRITE_FILTER */

	FCLOSE(fpin);
	FCLOSE(fpout);

	return (0);
}

/* Input and output filenames */
#ifdef RISCOS
static const char *inname = "pngtest/png";
static const char *outname = "pngout/png";
#else
static const char *inname = "C:\\workspace\\libpng-1.6.37\\Faerie.png";
static const char *outname = "pngoutsprites.png";
#endif

int Image::readPngFile(const char *inname)
{
	int multiple = 0;
	int ierror = 0;

	png_structp dummy_ptr;

	if (multiple != 0)
	{
		int i;
#if defined(PNG_USER_MEM_SUPPORTED) && PNG_DEBUG
		int allocation_now = current_allocation;
#endif
		for (i = 2; i < 3; ++i)
		{
			int kerror;
			//fprintf(STDERR, "\n Testing %s:", argv[i]);
#if PNG_DEBUG > 0
			fprintf(STDERR, "\n");
#endif
			kerror = test_one_file(inname, "outputpng.png");
			if (kerror == 0)
			{
#ifdef PNG_WRITE_USER_TRANSFORM_SUPPORTED
				fprintf(STDERR, "\n PASS (%lu zero samples)\n",
					(unsigned long)zero_samples);
#else
				fprintf(STDERR, " PASS\n");
#endif
#ifdef PNG_TIME_RFC1123_SUPPORTED
				if (tIME_chunk_present != 0)
					fprintf(STDERR, " tIME = %s\n", tIME_string);

				tIME_chunk_present = 0;
#endif /* TIME_RFC1123 */
			}

			else
			{
				if (xfail)
					fprintf(STDERR, " XFAIL\n");
				else
				{
					fprintf(STDERR, " FAIL\n");
					ierror += kerror;
				}
			}
#if defined(PNG_USER_MEM_SUPPORTED) && PNG_DEBUG
			if (allocation_now != current_allocation)
				fprintf(STDERR, "MEMORY ERROR: %d bytes lost\n",
					current_allocation - allocation_now);

			if (current_allocation != 0)
			{
				memory_infop pinfo = pinformation;

				fprintf(STDERR, "MEMORY ERROR: %d bytes still allocated\n",
					current_allocation);

				while (pinfo != NULL)
				{
					fprintf(STDERR, " %lu bytes at %p\n",
						(unsigned long)pinfo->size,
						pinfo->pointer);
					pinfo = pinfo->next;
				}
			}
#endif
		}
#if defined(PNG_USER_MEM_SUPPORTED) && PNG_DEBUG
		fprintf(STDERR, " Current memory allocation: %10d bytes\n",
			current_allocation);
		fprintf(STDERR, " Maximum memory allocation: %10d bytes\n",
			maximum_allocation);
		fprintf(STDERR, " Total   memory allocation: %10d bytes\n",
			total_allocation);
		fprintf(STDERR, "     Number of allocations: %10d\n",
			num_allocations);
#endif
	}

	else
	{
		int i;
		for (i = 0; i < 3; ++i)
		{
			int kerror;
#if defined(PNG_USER_MEM_SUPPORTED) && PNG_DEBUG
			int allocation_now = current_allocation;
#endif
			if (i == 1)
				status_dots_requested = 1;

			else if (verbose == 0)
				status_dots_requested = 0;

			if (i == 0 || verbose == 1 || ierror != 0)
			{
				fprintf(STDERR, "\n Testing %s:", inname);
#if PNG_DEBUG > 0
				fprintf(STDERR, "\n");
#endif
			}

			kerror = test_one_file(inname, outname);

			if (kerror == 0)
			{
				if (verbose == 1 || i == 2)
				{
#ifdef PNG_WRITE_USER_TRANSFORM_SUPPORTED
					fprintf(STDERR, "\n PASS (%lu zero samples)\n",
						(unsigned long)zero_samples);
#else
					fprintf(STDERR, " PASS\n");
#endif
#ifdef PNG_TIME_RFC1123_SUPPORTED
					if (tIME_chunk_present != 0)
						fprintf(STDERR, " tIME = %s\n", tIME_string);
#endif /* TIME_RFC1123 */
				}
			}

			else
			{
				if (verbose == 0 && i != 2)
				{
					fprintf(STDERR, "\n Testing %s:", inname);
#if PNG_DEBUG > 0
					fprintf(STDERR, "\n");
#endif
				}

				if (xfail)
					fprintf(STDERR, " XFAIL\n");
				else
				{
					fprintf(STDERR, " FAIL\n");
					ierror += kerror;
				}
			}
#if defined(PNG_USER_MEM_SUPPORTED) && PNG_DEBUG
			if (allocation_now != current_allocation)
				fprintf(STDERR, "MEMORY ERROR: %d bytes lost\n",
					current_allocation - allocation_now);

			if (current_allocation != 0)
			{
				memory_infop pinfo = pinformation;

				fprintf(STDERR, "MEMORY ERROR: %d bytes still allocated\n",
					current_allocation);

				while (pinfo != NULL)
				{
					fprintf(STDERR, " %lu bytes at %p\n",
						(unsigned long)pinfo->size, pinfo->pointer);
					pinfo = pinfo->next;
				}
			}
#endif
		}
#if defined(PNG_USER_MEM_SUPPORTED) && PNG_DEBUG
		fprintf(STDERR, " Current memory allocation: %10d bytes\n",
			current_allocation);
		fprintf(STDERR, " Maximum memory allocation: %10d bytes\n",
			maximum_allocation);
		fprintf(STDERR, " Total   memory allocation: %10d bytes\n",
			total_allocation);
		fprintf(STDERR, "     Number of allocations: %10d\n",
			num_allocations);
#endif
	}

#ifdef PNGTEST_TIMING
	t_stop = (float)clock();
	t_misc += (t_stop - t_start);
	t_start = t_stop;
	fprintf(STDERR, " CPU time used = %.3f seconds",
		(t_misc + t_decode + t_encode) / (float)CLOCKS_PER_SEC);
	fprintf(STDERR, " (decoding %.3f,\n",
		t_decode / (float)CLOCKS_PER_SEC);
	fprintf(STDERR, "        encoding %.3f ,",
		t_encode / (float)CLOCKS_PER_SEC);
	fprintf(STDERR, " other %.3f seconds)\n\n",
		t_misc / (float)CLOCKS_PER_SEC);
#endif

	if (ierror == 0)
		fprintf(STDERR, " libpng passes test\n");
	else
		fprintf(STDERR, " libpng FAILS test\n");

	dummy_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	fprintf(STDERR, " Default limits:\n");
	fprintf(STDERR, "  width_max  = %lu\n",
		(unsigned long)png_get_user_width_max(dummy_ptr));
	fprintf(STDERR, "  height_max = %lu\n",
		(unsigned long)png_get_user_height_max(dummy_ptr));
	if (png_get_chunk_cache_max(dummy_ptr) == 0)
		fprintf(STDERR, "  cache_max  = unlimited\n");
	else
		fprintf(STDERR, "  cache_max  = %lu\n",
		(unsigned long)png_get_chunk_cache_max(dummy_ptr));
	if (png_get_chunk_malloc_max(dummy_ptr) == 0)
		fprintf(STDERR, "  malloc_max = unlimited\n");
	else
		fprintf(STDERR, "  malloc_max = %lu\n",
		(unsigned long)png_get_chunk_malloc_max(dummy_ptr));
	png_destroy_read_struct(&dummy_ptr, NULL, NULL);

	return (int)(ierror != 0);
}
#else
int
main(void)
{
	fprintf(STDERR,
		" test ignored because libpng was not built with read support\n");
	/* And skip this test */
	return PNG_LIBPNG_VER < 10600 ? 0 : 77;
}
#endif

/* Generate a compiler error if there is an old png.h in the search path. */
typedef png_libpng_version_1_6_37 Your_png_h_is_not_version_1_6_37;



Image::Image()
{

}

Image::~Image()
{

}

bool Image::loadImage(const char *filePath)
{
	int file_size = 0;
	FILE* fp = NULL;
	if (check_if_png(filePath, &fp))
	{
		if (NULL != fp) {
			this->read_png(fp, PNG_BYTES_TO_CHECK);
			fclose(fp);
			fp = NULL;
			return true;
		}
	}
	else {
		if (NULL != fp) {
			fclose(fp);
			fp = NULL;
		}
		return false;
	}
}

int Image::check_if_png(const char *filePath, FILE **fp)
{
	char buf[PNG_BYTES_TO_CHECK];
	/* Open the prospective PNG file. */
	if (NULL == (*fp = fopen(filePath, "rb")))
		return 0;
	/* Read in some of the signature bytes. */
	if (fread(buf, 1, PNG_BYTES_TO_CHECK, *fp) != PNG_BYTES_TO_CHECK)
		return 0;
	/* Compare the first PNG_BYTES_TO_CHECK bytes of the signature.
	 * Return nonzero (true) if they match.
	 */
	return(!png_sig_cmp((png_const_bytep)buf, 0, PNG_BYTES_TO_CHECK));
}

void Image::read_png(FILE *fp, int sig_read)
{//https://stackoverflow.com/questions/53185114/how-to-change-rgb-values-from-a-png-image-with-libpng-using-c
	//https://stackoverflow.com/questions/1861382/how-to-convert-a-png-image-to-a-svg
https://www.linuxidc.com/Linux/2013-08/88652.html
	/*
//set wanted color as red
	png_byte wanted_color[] = { 255,0,0 };

	for (unsigned int y = 0; y < height; y++) {
		png_bytep row = row_pointers[y];
		for (unsigned int x = 0; x < width; x++) {
			png_bytep px = &(row[x * 4]);

			//set pixel to wanted color
			for (int i = 0; i < 3; ++i)
				px[i] = wanted_color[i];
		}
	}
} */
	png_structp png_ptr;
	png_infop read_info_ptr, end_info_ptr;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;

	/* Create and initialize the png_struct with the desired error handler
	 * functions.  If you want to use the default stderr and longjump method,
	 * you can supply NULL for the last three parameters.  We also supply the
	 * the compiler header file version, so that we know if the application
	 * was compiled with a compatible version of the library.  REQUIRED.
	 */
	 //png_voidp user_error_ptr;
	 //png_error_ptr user_error_fn,user_warning_fn;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	//png_create_read_struct(PNG_LIBPNG_VER_STRING, user_error_ptr, user_error_fn, user_warning_fn);
	if (NULL == png_ptr) {
		return;
	}
	/* Allocate/initialize the memory for image information.  REQUIRED. */
	read_info_ptr = png_create_info_struct(png_ptr);
	end_info_ptr = png_create_info_struct(png_ptr);
	if (NULL == read_info_ptr) {
		png_destroy_read_struct(&png_ptr, &read_info_ptr, &end_info_ptr);
		return;
	}
	/* Set error handling if you are using the setjmp/longjmp method (this is
	 * the normal method of doing things with libpng).  REQUIRED unless you
	 * set up your own error handlers in the png_create_read_struct() earlier.
	 */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* Free all of the memory associated with the png_ptr and info_ptr. */
		png_destroy_read_struct(&png_ptr, &read_info_ptr, &end_info_ptr);
		/* If we get here, we had a problem reading the file. */
		return;
	}
	/* One of the following I/O initialization methods is REQUIRED. */
#ifdef streams /* PNG file I/O method 1 */
   /* Set up the input control if you are using standard C streams. */
	png_init_io(png_ptr, fp);

#else no_streams /* PNG file I/O method 2 */
   /* If you are using replacement read functions, instead of calling
	* png_init_io(), you would call:
	*/
	png_init_io(png_ptr, fp);// png_ptr �� �ļ���
	//png_set_read_fn(png_ptr, (png_voidp)user_io_ptr, user_read_fn);
	/* where user_io_ptr is a structure you want available to the callbacks. */
#endif no_streams /* Use only one I/O method! */

   /* If we have already read some of the signature */
	png_set_sig_bytes(png_ptr, sig_read);

#ifdef hilevel
	/* If you have enough memory to read in the entire image at once,
	 * and you need to specify only transforms that can be controlled
	 * with one of the PNG_TRANSFORM_* bits (this presently excludes
	 * quantizing, filling, setting background, and doing gamma
	 * adjustment), then you can read the entire image (including
	 * pixels) into the info structure with this call:
	 */
	png_read_png(png_ptr, read_info_ptr, png_transforms, NULL);
#else
	/* OK, you're doing it the hard way, with the lower-level functions. */

	/* The call to png_read_info() gives us all of the information from the
	 * PNG file before the first IDAT (image data chunk).  REQUIRED.
	 */
	png_read_info(png_ptr, read_info_ptr);
	png_get_IHDR(png_ptr, read_info_ptr, &width, &height, &bit_depth, &color_type,
		&interlace_type, NULL, NULL);

	/* Set up the data transformations you want.  Note that these are all
	 * optional.  Only call them if you want/need them.  Many of the
	 * transformations only work on specific types of images, and many
	 * are mutually exclusive.
	 */

	 /* Tell libpng to strip 16 bits/color files down to 8 bits/color.
	  * Use accurate scaling if it's available, otherwise just chop off the
	  * low byte.
	  */
#ifdef PNG_READ_SCALE_16_TO_8_SUPPORTED
	png_set_scale_16(png_ptr);
#else
	png_set_strip_16(png_ptr);
#endif
	/* Strip alpha bytes from the input data without combining with the
	 * background (not recommended).
	 */
	 //png_set_strip_alpha(png_ptr);

	 /* Extract multiple pixels with bit depths of 1, 2 or 4 from a single
	  * byte into separate bytes (useful for paletted and grayscale images).
	  */
	png_set_packing(png_ptr);

	/* Change the order of packed pixels to least significant bit first
	 * (not useful if you are using png_set_packing).
	 */
	png_set_packswap(png_ptr);

	/* Expand paletted colors into true RGB triplets. */
	//if (color_type == PNG_COLOR_TYPE_PALETTE)
		//png_set_palette_to_rgb(png_ptr);
	/* Expand grayscale images to the full 8 bits from 1, 2 or 4 bits/pixel. */
	//if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		//png_set_expand_gray_1_2_4_to_8(png_ptr);
	/* Expand paletted or RGB images with transparency to full alpha channels
	 * so the data will be available as RGBA quartets.
	 */
	 //if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) != 0)
		 //png_set_tRNS_to_alpha(png_ptr);
	 /* Set the background color to draw transparent and alpha images over.
	  * It is possible to set the red, green and blue components directly
	  * for paletted images, instead of supplying a palette index.  Note that,
	  * even if the PNG file supplies a background, you are not required to
	  * use it - you should use the (solid) application background if it has one.
	  */
	  /*png_color_16 my_background, *image_background;
	  if (png_get_bKGD(png_ptr, info_ptr, &image_background) != 0)
		  png_set_background(png_ptr, image_background,
			  PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
	  else
		  png_set_background(png_ptr, &my_background,
			  PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);*/
			  /* Some suggestions as to how to get a screen gamma value.
			   *
			   * Note that screen gamma is the display_exponent, which includes
			   * the CRT_exponent and any correction for viewing conditions.
			   */
			   //int screen_gamma;
			   //if (/* We have a user-defined screen gamma value */)
			   //	screen_gamma = user - defined screen_gamma;
			   ///* This is one way that applications share the same screen gamma value. */
			   //else if ((gamma_str = getenv("SCREEN_GAMMA")) != NULL)
			   //	screen_gamma = atof(gamma_str);
			   ///* If we don't have another value */
			   //else
			   //{
			   //	screen_gamma = PNG_DEFAULT_sRGB; /* A good guess for a PC monitor
			   //										in a dimly lit room */
			   //	//screen_gamma = PNG_GAMMA_MAC_18 or 1.0; /* Good guesses for Mac
			   //											   systems */
			   //}
			   /* Tell libpng to handle the gamma conversion for you.  The final call
				* is a good guess for PC generated images, but it should be configurable
				* by the user at run time.  Gamma correction support in your application
				* is strongly recommended.
				*/
	int intent;
	//if (png_get_sRGB(png_ptr, info_ptr, &intent) != 0)
	//{
		//png_set_gamma(png_ptr, screen_gamma, PNG_DEFAULT_sRGB);
	//}
	//else
	//{
		/*double image_gamma;
		if (png_get_gAMA(png_ptr, info_ptr, &image_gamma) != 0)
			png_set_gamma(png_ptr, screen_gamma, image_gamma);
		else
			png_set_gamma(png_ptr, screen_gamma, 0.45455);*/
			//}
#ifdef PNG_READ_QUANTIZE_SUPPORTED
		/* Quantize RGB files down to 8-bit palette, or reduce palettes
		 * to the number of colors available on your screen.
		 */
	if ((color_type & PNG_COLOR_MASK_COLOR) != 0)
	{
		int num_palette;
		png_colorp palette;
		/* This reduces the image to the application-supplied palette. */
		//if (/* We have our own palette */)
		//{
		//	/* An array of colors to which the image should be quantized. */
		//	png_color std_color_cube[MAX_SCREEN_COLORS];
		//	png_set_quantize(png_ptr, std_color_cube, MAX_SCREEN_COLORS,
		//		MAX_SCREEN_COLORS, NULL, 0);
		//}
		///* This reduces the image to the palette supplied in the file. */
		//else if (png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette) != 0)
		//{
		//	png_uint_16p histogram = NULL;
		//	png_get_hIST(png_ptr, info_ptr, &histogram);
		//	png_set_quantize(png_ptr, palette, num_palette,
		//		max_screen_colors, histogram, 0);
		//}
	}
#endif /* READ_QUANTIZE */

	/* Invert monochrome files to have 0 as white and 1 as black. */
	//png_set_invert_mono(png_ptr);
	/* If you want to shift the pixel values from the range [0,255] or
	 * [0,65535] to the original [0,7] or [0,31], or whatever range the
	 * colors were originally in:
	 */
	if (png_get_valid(png_ptr, read_info_ptr, PNG_INFO_sBIT) != 0)
	{
		//png_color_8p sig_bit_p;
		//png_get_sBIT(png_ptr, read_info_ptr, &sig_bit_p);
		//png_set_shift(png_ptr, sig_bit_p);
	}
	/* Flip the RGB pixels to BGR (or RGBA to BGRA). */
	//if ((color_type & PNG_COLOR_MASK_COLOR) != 0)
		//png_set_bgr(png_ptr);
	/* Swap the RGBA or GA data to ARGB or AG (or BGRA to ABGR). */
	//png_set_swap_alpha(png_ptr);
	/* Swap bytes of 16-bit files to least significant byte first. */
	//png_set_swap(png_ptr);
	/* Add filler (or alpha) byte (before/after each RGB triplet). */
	if (color_type == PNG_COLOR_TYPE_RGB ||
		color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	if (color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);
	int number_passes;
#ifdef PNG_READ_INTERLACING_SUPPORTED
	/* Turn on interlace handling.  REQUIRED if you are not using
	 * png_read_image().  To see how to handle interlacing passes,
	 * see the png_read_row() method below:
	 */
	number_passes = png_set_interlace_handling(png_ptr);
#else /* !READ_INTERLACING */
	number_passes = 1;
#endif /* READ_INTERLACING */
	/* Optional call to gamma correct and add the background to the palette
	 * and update info structure.  REQUIRED if you are expecting libpng to
	 * update the palette for you (i.e. you selected such a transform above).
	 */
	png_read_update_info(png_ptr, read_info_ptr);
	/* Allocate the memory to hold the image using the fields of info_ptr. */
	png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
	int row;
	memset(row_pointers, NULL, height);/* Clear the pointer array */
	for (row = 0; row < height; row++)
		row_pointers[row] = (png_bytep)png_malloc(png_ptr, png_get_rowbytes(png_ptr,
			read_info_ptr));
	/* Now it's time to read the image.  One of these methods is REQUIRED. */
#ifdef entire /* Read the entire image in one go */
	png_read_image(png_ptr, row_pointers);
#else no_entire /* Read the image one or more scanlines at a time */
   /* The other way to read images - deal with interlacing: */
	for (int pass = 0; pass < number_passes; pass++)
	{
#ifdef single /* Read the image a single row at a time */
		for (y = 0; y < height; y++)
			png_read_rows(png_ptr, &row_pointers[y], NULL, 1);

#else no_single /* Read the image several rows at a time */
		int number_of_rows = 1;
		for (int y = 0; y < height; y += number_of_rows)
		{
#ifdef sparkle /* Read the image using the "sparkle" effect. */
			png_read_rows(png_ptr, &row_pointers[y], NULL,
				number_of_rows);
#else no_sparkle /* Read the image using the "rectangle" effect */
			png_read_rows(png_ptr, NULL, &row_pointers[y],
				number_of_rows);
#endif no_sparkle /* Use only one of these two methods */
		}
		/* If you want to display the image after every pass, do so here. */
#endif no_single /* Use only one of these two methods */
	}
#endif no_entire /* Use only one of these two methods */
	/* Read rest of file, and get additional chunks in info_ptr.  REQUIRED. */
	png_read_end(png_ptr, read_info_ptr);
#endif hilevel
	/* At this point you have read the entire image. */

	/* Clean up after the read, and free any memory allocated.  REQUIRED. */
	png_destroy_read_struct(&png_ptr, &read_info_ptr, &end_info_ptr);

	const int BLOCK_SIZE = 4;//(rgba 4 bytes)
	int file_size = width * height * BLOCK_SIZE;
	this->nWidth = width;
	this->nHeight = height;
	unsigned char **pData = &this->imgData;
	*pData = new unsigned char[file_size];
	int pos = 0;
	//int wirteResult = png_image_write_to_file(&image, (char *)*cbData, 0/*convert_to_8bit*/,buffer, 0/*row_stride*/, NULL/*colormap*/);
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width * BLOCK_SIZE; x += BLOCK_SIZE)
		{
			(*pData)[pos++] = row_pointers[y][x + 0];//R
			(*pData)[pos++] = row_pointers[y][x + 1];//G
			(*pData)[pos++] = row_pointers[y][x + 2];//B
			(*pData)[pos++] = row_pointers[y][x + 3];//A
		}
	}
}

int Image::readPng(const char *inname)
{
	static png_FILE_p fpin;
	//static png_FILE_p fpout;  /* "static" prevents setjmp corruption */
	pngtest_error_parameters error_parameters;
	png_structp read_ptr;
	png_infop read_info_ptr, end_info_ptr;

	png_bytep row_buf;
	png_uint_32 y;
	png_uint_32 width, height;
	volatile int num_passes;
	int pass;
	int bit_depth, color_type;

	row_buf = NULL;
	error_parameters.file_name = inname;

	if ((fpin = fopen(inname, "rb")) == NULL)
	{
		//fprintf(STDERR, "Could not find input file %s\n", inname);
		return (1);
	}


	//pngtest_debug("Allocating read and write structures");
	read_ptr =
		png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_set_error_fn(read_ptr, &error_parameters, pngtest_error,
		pngtest_warning);

	//pngtest_debug("Allocating read_info, write_info and end_info structures");
	read_info_ptr = png_create_info_struct(read_ptr);
	end_info_ptr = png_create_info_struct(read_ptr);


#ifdef PNG_READ_USER_CHUNKS_SUPPORTED
	init_callback_info(read_info_ptr);
	png_set_read_user_chunk_fn(read_ptr, &user_chunk_data,
		read_user_chunk_callback);
#endif

#ifdef PNG_SETJMP_SUPPORTED
	//pngtest_debug("Setting jmpbuf for read struct");
	if (setjmp(png_jmpbuf(read_ptr)))
	{
		fprintf(STDERR, "%s -> %s: libpng read error\n", inname, "no write");
		png_free(read_ptr, row_buf);
		row_buf = NULL;
		if (verbose != 0)
			fprintf(STDERR, "   destroy read structs\n");
		png_destroy_read_struct(&read_ptr, &read_info_ptr, &end_info_ptr);
#ifdef PNG_WRITE_SUPPORTED
		if (verbose != 0)
			fprintf(STDERR, "   destroy write structs\n");
		//png_destroy_info_struct(write_ptr, &write_end_info_ptr);
		//png_destroy_write_struct(&write_ptr, &write_info_ptr);
#endif
		FCLOSE(fpin);
		//FCLOSE(fpout);
		return (1);
	}

#ifdef PNG_WRITE_SUPPORTED
	//pngtest_debug("Setting jmpbuf for write struct");

#endif
#endif

#ifdef PNG_BENIGN_ERRORS_SUPPORTED
	if (strict != 0)
	{
		/* Treat png_benign_error() as errors on read */
		png_set_benign_errors(read_ptr, 0);

# ifdef PNG_WRITE_SUPPORTED
		/* Treat them as errors on write */
		//png_set_benign_errors(write_ptr, 0);
# endif

		/* if strict is not set, then app warnings and errors are treated as
		 * warnings in release builds, but not in unstable builds; this can be
		 * changed with '--relaxed'.
		 */
	}

	else if (relaxed != 0)
	{
		/* Allow application (pngtest) errors and warnings to pass */
		png_set_benign_errors(read_ptr, 1);

		/* Turn off CRC checking while reading */
		png_set_crc_action(read_ptr, PNG_CRC_QUIET_USE, PNG_CRC_QUIET_USE);

#ifdef PNG_IGNORE_ADLER32
		/* Turn off ADLER32 checking while reading */
		png_set_option(read_ptr, PNG_IGNORE_ADLER32, PNG_OPTION_ON);
#endif

# ifdef PNG_WRITE_SUPPORTED
		//png_set_benign_errors(write_ptr, 1);
# endif

	}
#endif /* BENIGN_ERRORS */

	//pngtest_debug("Initializing input and output streams");
#ifdef PNG_STDIO_SUPPORTED
	png_init_io(read_ptr, fpin);
#  ifdef PNG_WRITE_SUPPORTED
	//png_init_io(write_ptr, fpout);
#  endif
#else
	png_set_read_fn(read_ptr, (png_voidp)fpin, pngtest_read_data);
#  ifdef PNG_WRITE_SUPPORTED
	png_set_write_fn(write_ptr, (png_voidp)fpout, pngtest_write_data,
#    ifdef PNG_WRITE_FLUSH_SUPPORTED
		pngtest_flush);
#    else
		NULL);
#    endif
#  endif
#endif

	if (status_dots_requested == 1)
	{
#ifdef PNG_WRITE_SUPPORTED
		//png_set_write_status_fn(write_ptr, write_row_callback);
#endif
		png_set_read_status_fn(read_ptr, read_row_callback);
	}
	else
	{
#ifdef PNG_WRITE_SUPPORTED
		//png_set_write_status_fn(write_ptr, NULL);
#endif
		png_set_read_status_fn(read_ptr, NULL);
	}

#ifdef PNG_READ_USER_TRANSFORM_SUPPORTED
	png_set_read_user_transform_fn(read_ptr, read_user_callback);
#endif
#ifdef PNG_WRITE_USER_TRANSFORM_SUPPORTED
	zero_samples = 0;
	//png_set_write_user_transform_fn(write_ptr, count_zero_samples);
#endif

#ifdef PNG_SET_UNKNOWN_CHUNKS_SUPPORTED
	/* Preserve all the unknown chunks, if possible.  If this is disabled then,
	 * even if the png_{get,set}_unknown_chunks stuff is enabled, we can't use
	 * libpng to *save* the unknown chunks on read (because we can't switch the
	 * save option on!)
	 *
	 * Notice that if SET_UNKNOWN_CHUNKS is *not* supported read will discard all
	 * unknown chunks and write will write them all.
	 */
#ifdef PNG_SAVE_UNKNOWN_CHUNKS_SUPPORTED
	png_set_keep_unknown_chunks(read_ptr, PNG_HANDLE_CHUNK_ALWAYS,
		NULL, 0);
#endif
#ifdef PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED
	//png_set_keep_unknown_chunks(write_ptr, PNG_HANDLE_CHUNK_ALWAYS,NULL, 0);
#endif
#endif

	//pngtest_debug("Reading info struct");
	png_read_info(read_ptr, read_info_ptr);

#ifdef PNG_READ_USER_CHUNKS_SUPPORTED
	/* This is a bit of a hack; there is no obvious way in the callback function
	 * to determine that the chunks before the first IDAT have been read, so
	 * remove the info_ptr (which is only used to determine position relative to
	 * PLTE) here to indicate that we are after the IDAT.
	 */
	user_chunk_data.info_ptr = NULL;
#endif

	//pngtest_debug("Transferring info struct");
	{
		int interlace_type, compression_type, filter_type;

		if (png_get_IHDR(read_ptr, read_info_ptr, &width, &height, &bit_depth,
			&color_type, &interlace_type, &compression_type, &filter_type) != 0)
		{
			//png_set_IHDR(write_ptr, write_info_ptr, width, height, bit_depth,color_type, interlace_type, compression_type, filter_type);
			/* num_passes may not be available below if interlace support is not
			 * provided by libpng for both read and write.
			 */
			switch (interlace_type)
			{
			case PNG_INTERLACE_NONE:
				num_passes = 1;
				break;

			case PNG_INTERLACE_ADAM7:
				num_passes = 7;
				break;

			default:
				png_error(read_ptr, "invalid interlace type");
				/*NOT REACHED*/
			}
		}
		else
			png_error(read_ptr, "png_get_IHDR failed");
	}

#ifdef PNG_cHRM_SUPPORTED
	{
		png_fixed_point white_x, white_y, red_x, red_y, green_x, green_y, blue_x,
			blue_y;

		if (png_get_cHRM_fixed(read_ptr, read_info_ptr, &white_x, &white_y,
			&red_x, &red_y, &green_x, &green_y, &blue_x, &blue_y) != 0)
		{
			//png_set_cHRM_fixed(write_ptr, write_info_ptr, white_x, white_y, red_x,
				//red_y, green_x, green_y, blue_x, blue_y);
		}
	}
#endif
#ifdef PNG_gAMA_SUPPORTED
	{
		png_fixed_point gamma;
		if (png_get_gAMA_fixed(read_ptr, read_info_ptr, &gamma) != 0) {
			//png_set_gAMA_fixed(write_ptr, write_info_ptr, gamma);
		}
	}
#endif

#ifdef PNG_iCCP_SUPPORTED
	{
		png_charp name;
		png_bytep profile;
		png_uint_32 proflen;
		int compression_type;

		if (png_get_iCCP(read_ptr, read_info_ptr, &name, &compression_type,
			&profile, &proflen) != 0)
		{
			//png_set_iCCP(write_ptr, write_info_ptr, name, compression_type,
				//profile, proflen);
		}
	}
#endif
#ifdef PNG_sRGB_SUPPORTED
	{
		int intent;

		if (png_get_sRGB(read_ptr, read_info_ptr, &intent) != 0) {
			//png_set_sRGB(write_ptr, write_info_ptr, intent);
		}
	}
#endif
	{
		png_colorp palette;
		int num_palette;

		if (png_get_PLTE(read_ptr, read_info_ptr, &palette, &num_palette) != 0) {
			//png_set_PLTE(write_ptr, write_info_ptr, palette, num_palette);
		}
	}
#ifdef PNG_bKGD_SUPPORTED
	{
		png_color_16p background;

		if (png_get_bKGD(read_ptr, read_info_ptr, &background) != 0)
		{
			//png_set_bKGD(write_ptr, write_info_ptr, background);
		}
	}
#endif
#ifdef PNG_READ_eXIf_SUPPORTED
	{
		png_bytep exif = NULL;
		png_uint_32 exif_length;

		if (png_get_eXIf_1(read_ptr, read_info_ptr, &exif_length, &exif) != 0)
		{
			if (exif_length > 1)
				fprintf(STDERR, " eXIf type %c%c, %lu bytes\n", exif[0], exif[1],
				(unsigned long)exif_length);
# ifdef PNG_WRITE_eXIf_SUPPORTED
			//png_set_eXIf_1(write_ptr, write_info_ptr, exif_length, exif);
# endif
		}
	}
#endif
#ifdef PNG_hIST_SUPPORTED
	{
		png_uint_16p hist;

		if (png_get_hIST(read_ptr, read_info_ptr, &hist) != 0) {
			//png_set_hIST(write_ptr, write_info_ptr, hist);
		}
			
	}
#endif
#ifdef PNG_oFFs_SUPPORTED
	{
		png_int_32 offset_x, offset_y;
		int unit_type;

		if (png_get_oFFs(read_ptr, read_info_ptr, &offset_x, &offset_y,
			&unit_type) != 0)
		{
			//png_set_oFFs(write_ptr, write_info_ptr, offset_x, offset_y, unit_type);
		}
	}
#endif
#ifdef PNG_pCAL_SUPPORTED
	{
		png_charp purpose, units;
		png_charpp params;
		png_int_32 X0, X1;
		int type, nparams;

		if (png_get_pCAL(read_ptr, read_info_ptr, &purpose, &X0, &X1, &type,
			&nparams, &units, &params) != 0)
		{
			//png_set_pCAL(write_ptr, write_info_ptr, purpose, X0, X1, type,
				//nparams, units, params);
		}
	}
#endif
#ifdef PNG_pHYs_SUPPORTED
	{
		png_uint_32 res_x, res_y;
		int unit_type;

		if (png_get_pHYs(read_ptr, read_info_ptr, &res_x, &res_y,
			&unit_type) != 0) {
			//png_set_pHYs(write_ptr, write_info_ptr, res_x, res_y, unit_type);
		}
		
	}
#endif
#ifdef PNG_sBIT_SUPPORTED
	{
		png_color_8p sig_bit;

		if (png_get_sBIT(read_ptr, read_info_ptr, &sig_bit) != 0) {
			//png_set_sBIT(write_ptr, write_info_ptr, sig_bit);
		}
			
	}
#endif
#ifdef PNG_sCAL_SUPPORTED
#if defined(PNG_FLOATING_POINT_SUPPORTED) && \
   defined(PNG_FLOATING_ARITHMETIC_SUPPORTED)
	{
		int unit;
		double scal_width, scal_height;

		if (png_get_sCAL(read_ptr, read_info_ptr, &unit, &scal_width,
			&scal_height) != 0)
		{
			//png_set_sCAL(write_ptr, write_info_ptr, unit, scal_width, scal_height);
		}
	}
#else
#ifdef PNG_FIXED_POINT_SUPPORTED
	{
		int unit;
		png_charp scal_width, scal_height;

		if (png_get_sCAL_s(read_ptr, read_info_ptr, &unit, &scal_width,
			&scal_height) != 0)
		{
			//png_set_sCAL_s(write_ptr, write_info_ptr, unit, scal_width,
				//scal_height);
		}
	}
#endif
#endif
#endif

#ifdef PNG_sPLT_SUPPORTED
	{
		png_sPLT_tp entries;

		int num_entries = (int)png_get_sPLT(read_ptr, read_info_ptr, &entries);
		if (num_entries)
		{
			//png_set_sPLT(write_ptr, write_info_ptr, entries, num_entries);
		}
	}
#endif

#ifdef PNG_TEXT_SUPPORTED
	{
		png_textp text_ptr;
		int num_text;

		if (png_get_text(read_ptr, read_info_ptr, &text_ptr, &num_text) > 0)
		{
			//pngtest_debug1("Handling %d iTXt/tEXt/zTXt chunks", num_text);

			pngtest_check_text_support(read_ptr, text_ptr, num_text);

			if (verbose != 0)
			{
				int i;

				fprintf(STDERR, "\n");
				for (i = 0; i < num_text; i++)
				{
					fprintf(STDERR, "   Text compression[%d]=%d\n",
						i, text_ptr[i].compression);
				}
			}

			//png_set_text(write_ptr, write_info_ptr, text_ptr, num_text);
		}
	}
#endif
#ifdef PNG_tIME_SUPPORTED
	{
		png_timep mod_time;

		if (png_get_tIME(read_ptr, read_info_ptr, &mod_time) != 0)
		{
			//png_set_tIME(write_ptr, write_info_ptr, mod_time);
#ifdef PNG_TIME_RFC1123_SUPPORTED
			if (png_convert_to_rfc1123_buffer(tIME_string, mod_time) != 0)
				tIME_string[(sizeof tIME_string) - 1] = '\0';
			else
			{
				strncpy(tIME_string, "*** invalid time ***", (sizeof tIME_string));
				tIME_string[(sizeof tIME_string) - 1] = '\0';
			}

			tIME_chunk_present++;
#endif /* TIME_RFC1123 */
		}
	}
#endif
#ifdef PNG_tRNS_SUPPORTED
	{
		png_bytep trans_alpha;
		int num_trans;
		png_color_16p trans_color;

		if (png_get_tRNS(read_ptr, read_info_ptr, &trans_alpha, &num_trans,
			&trans_color) != 0)
		{
			int sample_max = (1 << bit_depth);
			/* libpng doesn't reject a tRNS chunk with out-of-range samples */
			if (!((color_type == PNG_COLOR_TYPE_GRAY &&
				(int)trans_color->gray > sample_max) ||
				(color_type == PNG_COLOR_TYPE_RGB &&
				((int)trans_color->red > sample_max ||
					(int)trans_color->green > sample_max ||
					(int)trans_color->blue > sample_max)))) {
				//png_set_tRNS(write_ptr, write_info_ptr, trans_alpha, num_trans,
					//trans_color);
			}	
		}
	}
#endif
#ifdef PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED
	{
		png_unknown_chunkp unknowns;
		int num_unknowns = png_get_unknown_chunks(read_ptr, read_info_ptr,
			&unknowns);

		if (num_unknowns != 0)
		{
			//png_set_unknown_chunks(write_ptr, write_info_ptr, unknowns,num_unknowns);
#if PNG_LIBPNG_VER < 10600
			/* Copy the locations from the read_info_ptr.  The automatically
			 * generated locations in write_end_info_ptr are wrong prior to 1.6.0
			 * because they are reset from the write pointer (removed in 1.6.0).
			 */
			{
				int i;
				/*for (i = 0; i < num_unknowns; i++)
					png_set_unknown_chunk_location(write_ptr, write_info_ptr, i,
						unknowns[i].location);*/
			}
#endif
		}
	}
#endif

#ifdef SINGLE_ROWBUF_ALLOC
	//pngtest_debug("Allocating row buffer...");
	row_buf = (png_bytep)png_malloc(read_ptr,
		png_get_rowbytes(read_ptr, read_info_ptr));

	//pngtest_debug1("\t%p", row_buf);
#endif /* SINGLE_ROWBUF_ALLOC */
	//pngtest_debug("Writing row data");

#if defined(PNG_READ_INTERLACING_SUPPORTED) &&\
   defined(PNG_WRITE_INTERLACING_SUPPORTED)
	/* Both must be defined for libpng to be able to handle the interlace,
	 * otherwise it gets handled below by simply reading and writing the passes
	 * directly.
	 */
	if (png_set_interlace_handling(read_ptr) != num_passes) {
		//png_error(write_ptr, "png_set_interlace_handling(read): wrong pass count ");
	}
		
	/*if (png_set_interlace_handling(write_ptr) != num_passes)
		png_error(write_ptr,
			"png_set_interlace_handling(write): wrong pass count ");*/
#else /* png_set_interlace_handling not called on either read or write */
#  define calc_pass_height
#endif /* not using libpng interlace handling */

#ifdef PNGTEST_TIMING
	t_stop = (float)clock();
	t_misc += (t_stop - t_start);
	t_start = t_stop;
#endif
	for (pass = 0; pass < num_passes; pass++)
	{
#     ifdef calc_pass_height
		png_uint_32 pass_height;

		if (num_passes == 7) /* interlaced */
		{
			if (PNG_PASS_COLS(width, pass) > 0)
				pass_height = PNG_PASS_ROWS(height, pass);

			else
				pass_height = 0;
		}

		else /* not interlaced */
			pass_height = height;
#     else
#        define pass_height height
#     endif

		//pngtest_debug1("Writing row data for pass %d", pass);
		for (y = 0; y < pass_height; y++)
		{
#ifndef SINGLE_ROWBUF_ALLOC
			//pngtest_debug2("Allocating row buffer (pass %d, y = %u)...", pass, y);

			row_buf = (png_bytep)png_malloc(read_ptr,
				png_get_rowbytes(read_ptr, read_info_ptr));

			//pngtest_debug2("\t%p (%lu bytes)", row_buf,
			//(unsigned long)png_get_rowbytes(read_ptr, read_info_ptr));

#endif /* !SINGLE_ROWBUF_ALLOC */
			png_read_rows(read_ptr, (png_bytepp)&row_buf, NULL, 1);

#ifdef PNG_WRITE_SUPPORTED
#ifdef PNGTEST_TIMING
			t_stop = (float)clock();
			t_decode += (t_stop - t_start);
			t_start = t_stop;
#endif
			//png_write_rows(write_ptr, (png_bytepp)&row_buf, 1);
#ifdef PNGTEST_TIMING
			t_stop = (float)clock();
			t_encode += (t_stop - t_start);
			t_start = t_stop;
#endif
#endif /* WRITE */

#ifndef SINGLE_ROWBUF_ALLOC
			//pngtest_debug2("Freeing row buffer (pass %d, y = %u)", pass, y);
			png_free(read_ptr, row_buf);
			row_buf = NULL;
#endif /* !SINGLE_ROWBUF_ALLOC */
		}
	}

	png_free_data(read_ptr, read_info_ptr, PNG_FREE_UNKN, -1);

	//pngtest_debug("Reading and writing end_info data");

	png_read_end(read_ptr, end_info_ptr);
#ifdef PNG_TEXT_SUPPORTED
	{
		png_textp text_ptr;
		int num_text;

		if (png_get_text(read_ptr, end_info_ptr, &text_ptr, &num_text) > 0)
		{
			//pngtest_debug1("Handling %d iTXt/tEXt/zTXt chunks", num_text);

			pngtest_check_text_support(read_ptr, text_ptr, num_text);

			if (verbose != 0)
			{
				int i;

				fprintf(STDERR, "\n");
				for (i = 0; i < num_text; i++)
				{
					fprintf(STDERR, "   Text compression[%d]=%d\n",
						i, text_ptr[i].compression);
				}
			}

			//png_set_text(write_ptr, write_end_info_ptr, text_ptr, num_text);
		}
	}
#endif
#ifdef PNG_READ_eXIf_SUPPORTED
	{
		png_bytep exif = NULL;
		png_uint_32 exif_length;

		if (png_get_eXIf_1(read_ptr, end_info_ptr, &exif_length, &exif) != 0)
		{
			if (exif_length > 1)
				fprintf(STDERR, " eXIf type %c%c, %lu bytes\n", exif[0], exif[1],
				(unsigned long)exif_length);
# ifdef PNG_WRITE_eXIf_SUPPORTED
			//png_set_eXIf_1(write_ptr, write_end_info_ptr, exif_length, exif);
# endif
		}
	}
#endif
#ifdef PNG_tIME_SUPPORTED
	{
		png_timep mod_time;

		if (png_get_tIME(read_ptr, end_info_ptr, &mod_time) != 0)
		{
			//png_set_tIME(write_ptr, write_end_info_ptr, mod_time);
#ifdef PNG_TIME_RFC1123_SUPPORTED
			if (png_convert_to_rfc1123_buffer(tIME_string, mod_time) != 0)
				tIME_string[(sizeof tIME_string) - 1] = '\0';

			else
			{
				strncpy(tIME_string, "*** invalid time ***", sizeof tIME_string);
				tIME_string[(sizeof tIME_string) - 1] = '\0';
			}

			tIME_chunk_present++;
#endif /* TIME_RFC1123 */
		}
	}
#endif
#ifdef PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED
	{
		png_unknown_chunkp unknowns;
		int num_unknowns = png_get_unknown_chunks(read_ptr, end_info_ptr,
			&unknowns);
		int a = 0;
	}
#endif


#ifdef PNG_EASY_ACCESS_SUPPORTED
	if (verbose != 0)
	{
		png_uint_32 iwidth, iheight;
		//iwidth = png_get_image_width(write_ptr, write_info_ptr);
		//iheight = png_get_image_height(write_ptr, write_info_ptr);
		//fprintf(STDERR, "\n Image width = %lu, height = %lu\n",
			//(unsigned long)iwidth, (unsigned long)iheight);
	}
#endif

	//pngtest_debug("Destroying data structs");
#ifdef SINGLE_ROWBUF_ALLOC
	pngtest_debug("destroying row_buf for read_ptr");
	png_free(read_ptr, row_buf);
	row_buf = NULL;
#endif /* SINGLE_ROWBUF_ALLOC */
	//pngtest_debug("destroying read_ptr, read_info_ptr, end_info_ptr");
	png_destroy_read_struct(&read_ptr, &read_info_ptr, &end_info_ptr);
#ifdef PNG_WRITE_SUPPORTED
	//pngtest_debug("destroying write_end_info_ptr");
	//png_destroy_info_struct(write_ptr, &write_end_info_ptr);
	//pngtest_debug("destroying write_ptr, write_info_ptr");
	//png_destroy_write_struct(&write_ptr, &write_info_ptr);
#endif
	//pngtest_debug("Destruction complete.");

	FCLOSE(fpin);
	//FCLOSE(fpout);

	const int BLOCK_SIZE = 4;//(rgba 4 bytes)
	int file_size = width * height * BLOCK_SIZE;
	this->nWidth = width;
	this->nHeight = height;
	unsigned char **pData = &this->imgData;
	*pData = new unsigned char[file_size];
	int pos = 0;
	png_bytep *row_p = &row_buf;
	//int wirteResult = png_image_write_to_file(&image, (char *)*cbData, 0/*convert_to_8bit*/,buffer, 0/*row_stride*/, NULL/*colormap*/);
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width * BLOCK_SIZE; x += BLOCK_SIZE)
		{
			(*pData)[pos++] = row_p[y][x + 0];//R
			(*pData)[pos++] = row_p[y][x + 1];//G
			(*pData)[pos++] = row_p[y][x + 2];//B
			(*pData)[pos++] = row_p[y][x + 3];//A
		}
	}

	/* Summarize any warnings or errors and in 'strict' mode fail the test.
	 * Unsupported chunks can result in warnings, in that case ignore the strict
	 * setting, otherwise fail the test on warnings as well as errors.
	 */
	if (error_count > 0)
	{
		/* We don't really expect to get here because of the setjmp handling
		 * above, but this is safe.
		 */
		fprintf(STDERR, "\n  %s: %d libpng errors found (%d warnings)",
			inname, error_count, warning_count);

		if (strict != 0)
			return (1);
	}

#  ifdef PNG_WRITE_SUPPORTED
	/* If there is no write support nothing was written! */
	else if (unsupported_chunks > 0)
	{
		fprintf(STDERR, "\n  %s: unsupported chunks (%d)%s",
			inname, unsupported_chunks, strict ? ": IGNORED --strict!" : "");
	}
#  endif

	else if (warning_count > 0)
	{
		//fprintf(STDERR, "\n  %s: %d libpng warnings found",
			//inname, warning_count);

		if (strict != 0)
			return (1);
	}

	//pngtest_debug("Opening files for comparison");
	if ((fpin = fopen(inname, "rb")) == NULL)
	{
		fprintf(STDERR, "Could not find file %s\n", inname);
		return (1);
	}

	/*if ((fpout = fopen(outname, "rb")) == NULL)
	{
		fprintf(STDERR, "Could not find file %s\n", outname);
		FCLOSE(fpin);
		return (1);
	}*/


	FCLOSE(fpin);
	//FCLOSE(fpout);

	return (0);
}
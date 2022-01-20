//
//  main.cpp
//  mnist_inference
//
//  Created by Klein, Russell on 10/20/20.
//  Copyright � 2020 Siemens AG
//


#ifdef MAC
#define HOST
#define WEIGHT_MEMORY
#define FIXED_POINT
//#define SLAVE
#define PAR_IN 5
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef float host_type;
typedef float weight_t;

// defines for local compile, should be set on cc line
//
// #define WEIGHT_MEMORY  - use a memory region for weights, else use a C++ variable. Required if cross compiling for ARM.
// #define FIXED_POINT    - use ac_fixed for weights and features, else use floats
// #define HOST           - compiled to run on the HOST, else compiled to hardware.  Define HOST for running with C++ DUT.
// #define SLAVE          - uses slave interface on accelerator, else use master
// #define ARM            - cross compile for ARM
// #define MAC            - define if MAC is the host
//

#define HEIGHT        28
#define WIDTH         28
#define AREA          (HEIGHT * WIDTH)
#define FILTER_HEIGHT  3
#define FILTER_WIDTH   3
#define FILTER_AREA   (FILTER_HEIGHT * FILTER_WIDTH)

#ifdef ARM
int console_out(char *s, ...);
#define printf console_out
#ifdef SLAVE
#include "conv_par_in.h"
#else // not SLAVE
#include "conv_par_in.h"
#endif // else not SLAVE
#endif // ARM

#ifdef HOST
#ifdef FIXED_POINT
#ifdef MAC
#include "/Users/russk/catapult/include/ac_channel.h"
#include "/Users/russk/catapult/include/ac_fixed.h"
#include "/Users/russk/catapult/include/ac_int.h"
#else  // not MAC
#include "ac_channel.h"
#include "ac_fixed.h"
#include "ac_int.h"
#endif // else not MAC
#endif // FIXED_POINT
#endif // HOST

#ifdef WEIGHT_MEMORY
// #include "weights_embedded.h"
#else // not WEIGHT_MEMORY
#include "/Users/russk/python/mnist_cat/weights.h"
#endif // else not WEIGHT_MMEORY

#ifdef MAC
#include "/Users/russk/python/mnist_cat/zero.h"
#include "/Users/russk/python/mnist_cat/one.h"
#include "/Users/russk/python/mnist_cat/two.h"
#include "/Users/russk/python/mnist_cat/three.h"
#include "/Users/russk/python/mnist_cat/four.h"
#include "/Users/russk/python/mnist_cat/five.h"
#include "/Users/russk/python/mnist_cat/six.h"
#include "/Users/russk/python/mnist_cat/seven.h"
#include "/Users/russk/python/mnist_cat/eight.h"
#include "/Users/russk/python/mnist_cat/nine.h"
#else
#include "zero.h"
#include "one.h"
#include "two.h"
#include "three.h"
#include "four.h"
#include "five.h"
#include "six.h"
#include "seven.h"
#include "eight.h"
#include "nine.h"
#endif

#ifdef WEIGHT_MEMORY

#ifndef PAR_IN
#define PAR_IN  (1)
#endif // ndef PAR_IN

#define PAR_OUT (PAR_IN)
#define WEIGHT_MEMORY_SIZE (100000)

#ifdef FIXED_POINT

#ifdef MAC
#include "/Users/russk/python/mnist_cat/weights_embedded_1.h"
#else
#include "weights_embedded_1.h"
#endif

#if PAR_IN==1
#define WORD_SIZE 32
#define INTEGER_BITS 16
#endif // PAR_IN == 1

#if PAR_IN==2
#define WORD_SIZE 16
#define INTEGER_BITS 8
#endif // PAR_IN == 2

#if PAR_IN==3
#define WORD_SIZE 10
#define INTEGER_BITS 5
#endif // PAR_IN == 3

#if PAR_IN==4
#define WORD_SIZE 8
#define INTEGER_BITS 4
#endif // PAR_IN == 4

#if PAR_IN==5
#define WORD_SIZE 6
#define INTEGER_BITS 3
#endif // PAR_IN == 5

#define FRACTIONAL_BITS (WORD_SIZE-INTEGER_BITS)

// bus width must be power of 2, and represents number of 32 bit elements that can be passed on the bus
// BUS_WIDTH_BITS == 0 means 32 bit bus
// BUS_WIDTH_BITS == 1 means 64 bit bus
// BUS_WIDTH_BITS == 2 means 128 bit bus
// BUS_WIDTH_BITS == 3 means 256 bit bus

#ifdef SLAVE
 #ifdef M3
  #define BUS_WIDTH_BITS 0
#else // A53
  #define BUS_WIDTH_BITS 1
 #endif

#else // MASTER
 #ifdef M3
  #define BUS_WIDTH_BITS 0
 #else // A53
  #define BUS_WIDTH_BITS 2
 #endif
#endif

#define BUS_WIDTH (1<<BUS_WIDTH_BITS)
#define STRIDE (PAR_IN * BUS_WIDTH)

#ifdef ARM

typedef unsigned int cat_memory_type;
typedef unsigned int hw_cat_type;
typedef float        sw_cat_type;

#else // not ARM

typedef ac_fixed<WORD_SIZE, INTEGER_BITS, true, AC_RND, AC_SAT> hw_cat_type;
typedef hw_cat_type                               cat_memory_type[PAR_IN];
typedef struct {cat_memory_type buffer[BUS_WIDTH];}  bus_type;

typedef ac_int<32 * BUS_WIDTH, false>             raw_bus_type;

//typedef struct {hw_cat_type buffer[PAR_IN];}    cat_memory_type_struct;
//typedef struct {hw_cat_type buffer[STRIDE];}    internal_memory_line_type;

#endif // not ARM
#else // not FIXED_POINT

#include "weights_embedded_1.h"

typedef float hw_cat_type;
typedef hw_cat_type cat_memory_type[PAR_IN];

#endif // else not FIXED_POINT

#ifdef ARM
typedef unsigned long bus_type;
#ifdef M3
static bus_type *weight_memory = (bus_type *) 0x40000000;
#else  // A53
static bus_type *weight_memory = (bus_type *) 0x20000000;
#endif
#else // not ARM
static raw_bus_type         weight_memory[WEIGHT_MEMORY_SIZE];
#endif // not ARM

#endif // WEIGHT_MEMORY


#if defined(WEIGHT_MEMORY) & defined(FIXED_POINT)
#ifndef ARM

//#include "conv_par_in.hpp"
#include "mnist_par.hpp"

hw_cat_type read_cat_memory_as_fixed(int offset)
{
    raw_memory_line line;
    int major;
    int minor;
    
    major = offset/STRIDE;
    minor = offset%STRIDE;
    
    return get_bus_word(weight_memory[major], minor);
    
    //printf("read_cat_memory_as_fixed(offset=%d) returned %5.3f = %5.3f \n", offset, ((hw_cat_type *)weight_memory)[offset].to_double());
}

void write_cat_memory_as_fixed(int offset, hw_cat_type value)
{
    raw_memory_line line;
    int major;
    int minor;
    
    major = offset/STRIDE;
    minor = offset%STRIDE;
    
    line = weight_memory[major];
    set_memory_word(line, minor, value);
    weight_memory[major] = line;
    
    //printf("write_cat_memory_as_fixed(offset=%d, value=%5.3f) \n", offset, value.to_double());
}

#ifdef SLAVE
#ifdef HOST

void conv2d_hw(
               int image_offset,
               int weight_offset,
               int output_offset,
               int num_input_images,
               int num_output_images,
               int height,
               int width,
               int filter_height,
               int filter_width,
               int relu)
{
    ac_channel<bool> go;
    ac_channel<bool> done;
    pipe_type image_in_channel;
    pipe_type filter_in_channel;
    pipe_type image_out_channel;
    pipe_type dense_in_channel;
    pipe_type dense_weights_channel;
    pipe_type dense_out_channel;
    cat_memory_type debug_signal;
    
    raw_memory_line line;
    hw_cat_type in_buf[STRIDE];
    hw_cat_type value;
    
    int f, i, n, o, p;
    int num_filters = num_input_images * num_output_images;
    int filter_size = filter_height * filter_width;
    int count;
    int msg;
    
    const bool chatty = false;
    
    if (chatty) printf("writing feature data to image_in_channel \n");
    count = 0;
    msg = 0;
    for (i=0; i<num_input_images; i++) {
        for (n=0; n<(height * width + (STRIDE-1))/STRIDE; n++) {  // pad end of each image
            for (p=0; p<STRIDE; p++) {
                if ((n * STRIDE + p) < (height * width)) {
                    value = read_cat_memory_as_fixed(image_offset+i*HEIGHT*WIDTH+n*STRIDE+p);
                    if (chatty) {
                        if (count%28==0) printf("\n");
                        if (value < 0.001) printf("  -   "); else printf("%5.3f ", value.to_double());
                    }
                    count++;
                } else {
                    value = 0.0;
                }
                set_memory_word(line, p, value);
            }
            msg++;
            image_in_channel.write(line);
        }
    }
    if (chatty) printf("wrote: %d features in %d packets \n", count, msg);
    
    if (chatty) printf("Writing filter_data to filter_in_channel \n");
    count = 0;
    for (f=0; f<num_filters; f++) {
        for (n=0; n<(filter_size + (STRIDE-1))/STRIDE; n++) {
            for (p=0; p<STRIDE; p++) {
                if ((n * STRIDE + p) < filter_size) {
                    value = read_cat_memory_as_fixed(weight_offset+f*filter_size+n*STRIDE+p);
                    if (chatty) {
                        if (count%3==0) printf("\n");
                        printf("%5.3f ", value.to_double());
                    }
                    count++;
                } else {
                    value = 0.0;
                }
                set_memory_word(line, p, value);
            }
            msg++;
            filter_in_channel.write(line);
        }
    }
    if (chatty) printf("wrote: %d weights \n", count);
    
    go.write(1);
    
    conv_par_in(
                debug_signal,
                go,
                done,
                relu,
                true,
                false,
                image_in_channel,
                filter_in_channel,
                image_out_channel,
                dense_in_channel,
                dense_weights_channel,
                dense_out_channel,
                num_input_images,
                num_output_images);
    
    done.read();
    
    if (chatty) printf("Reading output images from image_out_channel \n");
    count = 0;
    for (o=0; o<num_output_images; o++) {
        for (n=0; n<(height * width + (STRIDE-1))/STRIDE; n++) {
            line = image_out_channel.read();
            for (p=0; p<STRIDE; p++) {
                value = get_memory_word(line, p);
                if ((n * STRIDE + p) < (height * width)) {
                    write_cat_memory_as_fixed(output_offset + o * height * width + n * STRIDE + p, value);
                    if (chatty) {
                        if (count%28==0) printf("\n");
                        if (value < 0.001) printf("  -   "); else printf("%5.3f ", value.to_double());
                    }
                    count++;
                }
            }
        }
    }
}

void dense_hw(
              int image_offset,
              int weight_offset,
              int output_offset,
              int num_input_images,
              int num_output_images)
{
    ac_channel<bool> go;
    ac_channel<bool> done;
    pipe_type image_in_channel;
    pipe_type filter_in_channel;
    pipe_type image_out_channel;
    pipe_type dense_in_channel;
    pipe_type dense_weights_channel;
    pipe_type dense_out_channel;
    cat_memory_type debug_signal;

    raw_memory_line line;
    hw_cat_type value;
    
    int i, o, n, p;
    int count;
    
    count = 0;
    
    for (n=0; n<(num_input_images + (STRIDE-1))/STRIDE; n++) {
        for (p=0; p<STRIDE; p++) {
            if ((n * STRIDE + p) < num_input_images) {
                value = read_cat_memory_as_fixed(image_offset + n * STRIDE + p);
                count++;
            } else {
                value = 0.0;
            }
            set_memory_word(line, p, value);
        }
        dense_in_channel.write(line);
    }
    
    for (o=0; o<num_output_images; o++) {
        for (n=0; n<(num_input_images + (STRIDE-1))/STRIDE; n++) {
            for (p=0; p<STRIDE; p++) {
                if ((n * STRIDE + p) < (num_input_images)) {
                    value = read_cat_memory_as_fixed(weight_offset + o * num_input_images + n * STRIDE + p);
                } else {
                    value = 0.0;
                }
                set_memory_word(line, p, value);
            }
            dense_weights_channel.write(line);
        }
    }
    
    go.write(1);
    
    conv_par_in(
                debug_signal,
                go,
                done,
                false,
                false,
                true,
                image_in_channel,
                filter_in_channel,
                image_out_channel,
                dense_in_channel,
                dense_weights_channel,
                dense_out_channel,
                num_input_images,
                num_output_images);
    
    for (i=0; i<(num_output_images + (STRIDE-1))/STRIDE; i++) {
        line = dense_out_channel.read();
        for (p=0; p<STRIDE; p++) {
            write_cat_memory_as_fixed(output_offset + i * STRIDE + p, get_memory_word(line,p));
        }
    }
    done.read();
}
#else // not HOST
void conv2d_hw(
               cat_memory_type memory_base[0x10000],
               int image_offset,
               int weight_offset,
               int output_offset,
               int num_input_images,
               int num_output_images,
               int height,
               int width,
               int filter_height,
               int filter_width,
               int relu)
{
    int i, o, p, n;

    NUM_INPUT_IMAGES = num_input_images;
    NUM_OUTPUT_IMAGES = num_output_images;
    CONVOLVE = 1;
    RELU = 1;
    FULLY_CONNECTED = 0;
    
    while (!GO_READY);
    GO = 1;
    
printf("started conv2d() \n");
    for (o=0; o<num_output_images; o++) {
        for (i=0; o<num_input_images; i++) {
            for (n=0; n<(height*width + (STRIDE-1))/STRIDE; n++) {
                while (!IMAGE_CHANNEL_IN_READY);
                IMAGE_IN_CHANNEL = memory_base[image_offset + height*width*i + n];
            }
printf("Image in \n");
            for (n=0; n<(filter_height*filter_width + (STRIDE-1))/STRIDE; n++) {
                while (!FILTER_IN_CHANNEL_READY);
                FILTER_IN_CHANNEL = memory_base[weight_offset + filter_height*filter_width*i + n];
            }
printf("Filter in \n");
        }
        for (n=0; n<(height*width + (STRIDE-1))/STRIDE; n++) {
            while (!IMAGE_OUT_CHANNEL_VALID);
            memory_base[output_offset + height*width*o + n] = IMAGE_OUT_CHANNEL;
        }
    }
    
    while (!DONE_VALID);
    x = DONE;
    /*
    for (o=0; o<num_output_images; o++) {
        for (n=0; n<(height*width + (STRIDE-1))/STRIDE; n++) {
            memory_base[output_offset + height*width*o + n] = IMAGE_OUT_CHANNEL;
        }
    }
    */
}

void dense_hw(
              cat_memory_type memory_base[0x10000],
              int image_offset,
              int weight_offset,
              int output_offset,
              int num_input_images,
              int num_output_images)
{
    volatile int x;
    int i, o, c, n, p;
    
    NUM_INPUT_IMAGES = num_input_images;
    NUM_OUTPUT_IMAGES = num_output_images;
    
    while (!GO_READY);
    GO = 1;
    
    for (int i=0; i<num_input_images;i++) {
        for (int c=0; c<(((image_height*image_width) + (STRIDE-1))/STRIDE); c++) {
            while (!DENSE_IN_CHANNEL_READY);
            DENSE_IN_CHANNEL = memory_base[image_offset + image_height*image_width*i + c];
        }
    }
    
    for (int i=0; i<num_input_images;i++) {
        for (int c=0; c<(((image_height*image_width) + (STRIDE-1))/STRIDE); c++) {
            while (!DENSE_WEIGHT_CHANNEL_READY);
            DENSE_WEIGHT_CHANNEL = memory_base[weight_offset + image_height*image_width*i + c];
        }
    }
    
    while (!DONE_VALID);
    x = DONE;
    
    for (o=0; o<(num_output_images + (STRIDE-1))/STRIDE; o++) {
        while (!DENSE_OUT_CHANNEL_DONE);
        memory_base[output_offset + o] = DENSE_OUT_CHANNEL;
    }
}

#endif // not HOST
#else // not SLAVE
void conv2d_hw(
               int image_offset,
               int weight_offset,
               int output_offset,
               int num_input_images,
               int num_output_images,
               int height,
               int width,
               int filter_height,
               int filter_width,
               int relu)
{
    ac_channel<bool> go;
    ac_channel<bool> done;
    cat_memory_type debug_signal;
    raw_bus_type *memory_base = weight_memory;
    
    go.write(1);
    
    conv_par_in(
                debug_signal,
                go,
                done,
                relu,
                true,
                false,
                memory_base,
                image_offset,
                weight_offset,
                output_offset,
                num_input_images,
                num_output_images);
    
    done.read();
}

void dense_hw(
              int image_offset,
              int weight_offset,
              int output_offset,
              int num_input_images,
              int num_output_images)
{
    ac_channel<bool> go;
    ac_channel<bool> done;
    cat_memory_type debug_signal;
    raw_bus_type *memory_base = weight_memory;
    
    go.write(1);
    
    conv_par_in(
                debug_signal,
                go,
                done,
                false,
                false,
                true,
                memory_base,
                image_offset,
                weight_offset,
                output_offset,
                num_input_images,
                num_output_images);
    
    done.read();
}
#endif // not SLAVE
#else // not ifndef ARM (i.e. not on the HOST)

#ifdef SLAVE

bus_type get_cat_value_as_fixed(int index)
{
    int major = index/PAR_IN;
    int minor = index%PAR_IN;

#if (WORD_SIZE == 32)
    return weight_memory[major];
#else
    return (weight_memory[major] >> (minor * WORD_SIZE)) & ((1 << WORD_SIZE) -1); 
#endif
}

void set_cat_value_as_fixed(int index, bus_type value)
{
    int major = index/PAR_IN;
    int minor = index%PAR_IN;
    bus_type mem_value;
    bus_type mask;

#if (WORD_SIZE == 32)
    weight_memory[major] = value;
#else
    mem_value = weight_memory[major];
    mask = ((1 << WORD_SIZE) -1) << (minor * WORD_SIZE);
    mem_value &= (~mask);
    mem_value |= mask & (value << (minor * WORD_SIZE));
    weight_memory[major] = mem_value; 
#endif
}

void conv2d_hw(
               int image_offset,
               int weight_offset,
               int output_offset,
               int num_input_images,
               int num_output_images,
               int height,
               int width,
               int filter_height,
               int filter_width,
               int relu)
{
    volatile int x;
    int i, o, p, n;
    bus_type outgoing_word;
    bus_type incoming_word;
    
    NUM_INPUT_IMAGES = num_input_images;
    NUM_OUTPUT_IMAGES = num_output_images;
    
    CONVOLVE = 1;
    RELU = 1;

    FULLY_CONNECTED = 0;

    while (!GO_READY);
    GO = 1;
    
printf("Started conv2d() \n");
    for (i=0; i<num_input_images; i++) {
        for (n=0; n<(height*width + (STRIDE-1))/STRIDE; n++) {
            outgoing_word = 0;
            for (p=0; p<STRIDE; p++) {
               outgoing_word = outgoing_word + (get_cat_value_as_fixed(image_offset + height*width*i + n*STRIDE + p) << (WORD_SIZE * p));
            }
            while (!IMAGE_IN_CHANNEL_READY);
            IMAGE_IN_CHANNEL = outgoing_word;
        }
printf("Image in\n");
    }
    for (o=0; o<num_output_images; o++) {
        for (i=0; i<num_input_images; i++) {
            for (n=0; n<(filter_height*filter_width + (STRIDE-1))/STRIDE; n++) {
                outgoing_word = 0;
                for (p=0; p<STRIDE; p++) {
                   outgoing_word = outgoing_word + (get_cat_value_as_fixed(weight_offset + (filter_height*filter_width)*(i+(o*num_input_images)) + n*STRIDE + p) << (WORD_SIZE * p));
                }
                while (!FILTER_IN_CHANNEL_READY);
                FILTER_IN_CHANNEL = outgoing_word;
            }
printf("Filter in \n");
        }

        for (n=0; n<(height*width + (STRIDE-1))/STRIDE; n++) {
            while (!IMAGE_OUT_CHANNEL_VALID);
            incoming_word = IMAGE_OUT_CHANNEL;
            for (p=0; p<STRIDE; p++) {
                set_cat_value_as_fixed(output_offset + height*width*o + n*STRIDE + p, (incoming_word >> (WORD_SIZE * p)) & ((1 << WORD_SIZE) -1));
            }
            //weight_memory[output_offset + height*width*o + n] = IMAGE_OUT_CHANNEL;
        }
printf("results out \n");
    }
    
    while (!DONE_VALID);
    x = DONE;

printf("done with conv2d_hw()\n");
}

void dense_hw(
              //cat_memory_type memory_base[0x10000],
              int image_offset,
              int weight_offset,
              int output_offset,
              int num_input_images,
              int num_output_images)
{
    volatile int x;
    int i, o, c, n, p;
    bus_type outgoing_word;
    bus_type incoming_word;
    
    CONVOLVE = 0;
    RELU = 0;

    FULLY_CONNECTED = 1;
    NUM_INPUT_IMAGES = num_input_images;
    NUM_OUTPUT_IMAGES = num_output_images;
    
    while (!GO_READY);
    GO = 1;
    
    for (int i=0; i<(num_input_images + (STRIDE-1))/STRIDE; i++) {
        outgoing_word = 0;
        for (p=0; p<STRIDE; p++) {
            outgoing_word = outgoing_word + (get_cat_value_as_fixed(image_offset + i*STRIDE + p) << (WORD_SIZE * p));
        }
        while (!DENSE_IN_CHANNEL_READY);
        DENSE_IN_CHANNEL = outgoing_word;
    }

    for (int o=0; o<num_output_images; o++) {
        for (int i=0; i<(num_input_images + (STRIDE-1))/STRIDE; i++) {
            outgoing_word = 0;
            for (p=0; p<STRIDE; p++) {
                outgoing_word = outgoing_word + (get_cat_value_as_fixed(weight_offset + num_input_images * o + i*STRIDE + p) << (WORD_SIZE * p));
            }
            while (!DENSE_WEIGHTS_CHANNEL_READY);
            DENSE_WEIGHTS_CHANNEL = outgoing_word;
        }
    }
    
    for (o=0; o<(num_output_images + (STRIDE-1))/STRIDE; o++) {
        while (!DENSE_OUT_CHANNEL_VALID);
        incoming_word = DENSE_OUT_CHANNEL;
        for (p=0; p<STRIDE; p++) {
            set_cat_value_as_fixed(output_offset + o*STRIDE +p, (incoming_word >> (WORD_SIZE * p)) & ((1 << WORD_SIZE) -1));
        }
        //weight_memory[output_offset + o] = DENSE_OUT_CHANNEL;
    }

    while (!DONE_VALID);
    x = DONE;
}

#else // not SLAVE

void conv2d_hw(
               int image_offset,
               int weight_offset,
               int output_offset,
               int num_input_images,
               int num_output_images,
               int height,
               int width,
               int filter_height,
               int filter_width,
               int relu)
{
    volatile int x;

    CONVOLVE = 1;
    RELU = relu;

    FULLY_CONNECTED = 0;
    NUM_INPUT_IMAGES = num_input_images;
    NUM_OUTPUT_IMAGES = num_output_images;

    IMAGE_OFFSET = image_offset;
    WEIGHT_OFFSET = weight_offset;
    OUTPUT_OFFSET = output_offset;
    NUM_INPUT_IMAGES = num_input_images;
    NUM_OUTPUT_IMAGES = num_output_images;
    
    while (!GO_READY);
    GO = 1;
    
    while (!DONE_VALID);
    x = DONE;
};


void dense_hw(
              int image_offset,
              int weight_offset,
              int output_offset,
              int num_input_images,
              int num_output_images)
{
    volatile int x;
    
    CONVOLVE = 0;
    RELU = 0;

    FULLY_CONNECTED = 1;
    IMAGE_OFFSET = image_offset;
    WEIGHT_OFFSET = weight_offset;
    OUTPUT_OFFSET = output_offset;
    NUM_INPUT_IMAGES = num_input_images;
    NUM_OUTPUT_IMAGES = num_output_images;
    
    while (!GO_READY);
    GO = 1;
    
    while (!DONE_VALID);
    x = DONE;
};

#endif // not SLAVE
#endif // else def ARM
#endif // defined WEIGHT_MEMORY and FIXED_POINT


#ifdef WEIGHT_MEMORY
#ifdef ARM

sw_cat_type get_cat_value(bus_type *m, int index)
{
    int major, minor;
    int n;
    float r;
    float f;
    float divisor;
    unsigned int mask;
    
    major = index/STRIDE;
    minor = index%STRIDE;
    
#if WORD_SIZE==32
    mask = 0xFFFFFFFF;
#else // WORD_SIZE != 32
    mask = ((1<<WORD_SIZE)-1);
#endif
    
    n = m[major] >> (WORD_SIZE * minor);
    n = n & mask;
#if WORD_SIZE!=32
    if (n & (1 << (WORD_SIZE - 1))) n = n * -1;
#endif
    
    divisor = (float) (1 << FRACTIONAL_BITS);
    f = (float) n;
    r = f/divisor;
    
    return r;
}

void set_cat_value(bus_type *m, int index, sw_cat_type value)
{
    int major, minor;
    int mask;
    int n;
    unsigned int temp;
    
    major = index/STRIDE;
    minor = index%STRIDE;
    
#if WORD_SIZE == 32
    mask = 0xFFFFFFFF;
#else // WORD_SIZE != 32
    mask = ((1<<WORD_SIZE)-1) << (WORD_SIZE * minor);
#endif
    
    n = m[major];
    n = n & (~mask);
    
    temp = (int) (value * (1 << FRACTIONAL_BITS));
    n = n | (((temp) << (WORD_SIZE * minor)) & mask);
    
    m[major] = n;
}

void copy_to_cat(int offset, host_type *source, int n)
{
    int i;
    
    for (i=0; i<n; i++) {
        set_cat_value(weight_memory, offset+i, source[i]);
    }
}

void copy_from_cat(host_type *dest, int offset, int n)
{
    int i;
    
    for (i=0; i<n; i++) {
        dest[i] = get_cat_value(weight_memory, offset + i);
    }
}

#else

hw_cat_type get_cat_value(raw_bus_type *m, int offset)
{
    /*
    int major, minor;
    hw_cat_type *mm = (hw_cat_type *) m;
    
    major = index/STRIDE;
    minor = index%STRIDE;
    
    return mm[index];
    */
    
    bool chatty = false;
    hw_cat_type t;
    int slice_offset = (offset % STRIDE) * WORD_SIZE;
    int index = offset / STRIDE;
    
    t.set_slc(0, weight_memory[index].slc<WORD_SIZE>(slice_offset));
    if (chatty) printf("Read %8.5f from %d \n", t.to_double(), offset);
    return t;

}

void set_cat_value(raw_bus_type *m, int offset, hw_cat_type value)
{
    /*
    int major, minor;
    cat_memory_type *mm = (raw_cat_memory_type *) m;
    
    major = index/STRIDE;
    minor = index%STRIDE;
    
    mm[major][minor] = value;
    */
    
    bool chatty = false;
    ac_int<WORD_SIZE, true> t_int;
    
    t_int.set_slc(0, value.slc<WORD_SIZE>(0));
    
    int slice_offset = offset % STRIDE;
    int index = offset / STRIDE;
    
    m[index].set_slc(slice_offset * WORD_SIZE, t_int);
    if (chatty) printf("Wrote %8.5f to %d \n", value.to_double(), offset);

}


void write_cat_memory(int offset, host_type value)
{
    /*
    int p;
    int n;
    int major;
    int minor;
    const int chatty = 0;
    
    p = offset % STRIDE;
    n = offset / STRIDE;
    major = n >> BUS_WIDTH_BITS;
    minor = n & (BUS_WIDTH -1);
    
    weight_memory[major].buffer[minor][p] = value;
    
    */
    
    int chatty = 0;
    hw_cat_type t;
    ac_int<WORD_SIZE, true> t_int;
    
    t = value;
    t_int.set_slc(0, t.slc<WORD_SIZE>(0));
    
    int slice_offset = (offset % STRIDE) * WORD_SIZE;
    int index = offset / STRIDE;

    weight_memory[index].set_slc(slice_offset, t_int);
    if (chatty) printf("Wrote %8.5f to %d \n", value, offset);
}

host_type read_cat_memory(int offset)
{
    /*
    int p;
    int n;
    int major;
    int minor;
    host_type value;
    const int chatty = 0;
    hw_cat_type t;
    
    p = offset % STRIDE;
    n = offset / STRIDE;
    major = n >> BUS_WIDTH_BITS;
    minor = n & (BUS_WIDTH -1);
    
    value = weight_memory[major].buffer[minor][p].to_double();
    */
    
    int chatty = 0;
    host_type value;
    hw_cat_type t;
    int slice_offset = (offset % STRIDE) * WORD_SIZE;
    int index = offset / STRIDE;
    
    t.set_slc(0, weight_memory[index].slc<WORD_SIZE>(slice_offset));
    value = t.to_double();
    if (chatty) printf("Read %8.5f from %d \n", value, offset);
    return value;
}


void copy_to_cat(int offset, host_type *source, int n)
{
    int i;
    
    for (i=0; i<n; i++) {
        write_cat_memory(offset+i, source[i]);
    }
}

void copy_from_cat(host_type *dest, int offset, int n)
{
    int i;
    
    for (i=0; i<n; i++) {
        dest[i] = read_cat_memory(offset + i);
    }
}

#endif // else not ARM

#ifdef ARM

void print_image(int offset, int n)
{
    int r, c, i;
    float value;
    
    for (i=0; i<n; i++){
        printf("image offset: %d \n", offset + AREA * i);
        for (r=0; r<HEIGHT; r++) {
            for (c=0; c<WIDTH; c++) {
                //value = read_cat_memory(offset + i * HEIGHT * WIDTH + r * WIDTH + c);
                value = get_cat_value(weight_memory, offset + i * HEIGHT * WIDTH + r * WIDTH + c);
                if (value > 0.001) {
                    printf("%4d ", (int)(value * 100.));
                } else {
                    printf("  -  ");
                }
            }
            printf("\n");
        }
        printf("\n");
    }
}

void print_weight(int offset, int n)
{
    int i, r, c;
    float value;
    
    for (i=0; i<n; i++) {
        printf("\n");
        printf("weight offset: %d \n", offset);
        for (r=0; r<FILTER_HEIGHT; r++) {
            for (c=0; c<FILTER_WIDTH; c++) {
                value = get_cat_value(weight_memory, offset + i * FILTER_HEIGHT * FILTER_WIDTH + r * FILTER_WIDTH + c);
                printf("%d ", (int)(value * 100.0));
            }
            printf("\n");
        }
        printf("\n");
    }
}

#else // not ARM

void print_image(int offset, int n)
{
    int r, c, i;
    hw_cat_type value;
    
    for (i=0; i<n; i++){
        fprintf(stdout, "image offset: %d \n", offset + AREA * i);
        for (r=0; r<HEIGHT; r++) {
            for (c=0; c<WIDTH; c++) {
                //value = read_cat_memory(offset + i * HEIGHT * WIDTH + r * WIDTH + c);
                value = get_cat_value(weight_memory, offset + i * HEIGHT * WIDTH + r * WIDTH + c);
                if (value > 0.001) {
#ifdef HOST
                    fprintf(stdout, "%5.3f ", value.to_double());
#else // not HOST
                    fprintf(stdout, "%5.3f ", value);
#endif // else not HOST
                } else {
                    fprintf(stdout, "  -   ");
                }
            }
            fprintf(stdout, "\n");
        }
        fprintf(stdout, "\n");
    }
}

void print_weight(int offset, int n)
{
    int i, r, c;
    hw_cat_type value;
    
    for (i=0; i<n; i++) {
        fprintf(stdout, "\n");
        fprintf(stdout, "weight offset: %d \n", offset);
        for (r=0; r<FILTER_HEIGHT; r++) {
            for (c=0; c<FILTER_WIDTH; c++) {
                //value = read_cat_memory(offset + i * FILTER_HEIGHT * FILTER_WIDTH + r * FILTER_WIDTH + c);
                value = get_cat_value(weight_memory, offset + i * FILTER_HEIGHT * FILTER_WIDTH + r * FILTER_WIDTH + c);
#ifdef HOST
                fprintf(stdout, "%5.3f ", value.to_double());
#else // not HOST
                fprintf(stdout, "%5.3f ", value);
#endif // else not HOST
            }
            fprintf(stdout, "\n");
        }
        fprintf(stdout, "\n");
    }
}
#endif // else not ARM
#else // not WEIGHT_MEMORY

void print_image(host_type *d, int n)
{
    int r, c, i;
    
    for (i=0; i<n; i++){
        for (r=0; r<HEIGHT; r++) {
            for (c=0; c<WIDTH; c++) {
                if (d[i * HEIGHT * WIDTH + r * WIDTH + c] > 0.001) {
                    printf("%5.3f, ", d[i * HEIGHT * WIDTH + r * WIDTH + c]);
                } else {
                    printf("  -    ");
                }
            }
            printf("\n");
        }
        printf("\n");
    }
}

#endif // else not WEIGHT_MEMORY

int in_bounds(
              int r,
              int c,
              int height,
              int width)
{
    if (r < 0)        return 0;
    if (r >= height)  return 0;
    if (c < 0)        return 0;
    if (c >= width)   return 0;
    return 1;
}


int compute_index(
                  int o,
                  int i,
                  int r,
                  int c,
                  int num_i,
                  int num_r,
                  int num_c)
{
    return
    o * num_i * num_r * num_c +
    i * num_r * num_c +
    r * num_c +
    c;
}

#define count(array_name) (sizeof(array_name)/sizeof(host_type))

#if defined(WEIGHT_MEMORY) & defined(FIXED_POINT)

int compute_par_index(
                      int o,
                      int i,
                      int r,
                      int c,
                      int num_i,
                      int num_r,
                      int num_c)
{
    int size = ((num_r * num_c) + (STRIDE - 1))/STRIDE;
    int alignment_offset = (size * STRIDE) - (num_r * num_c);
    int number_of_elements = o * num_i + i;
    
    return
    o * num_i * num_r * num_c +
    i * num_r * num_c +
    r * num_c +
    c + alignment_offset * number_of_elements;
}


#define WIDX(out_image, in_image, row, col) \
compute_index(out_image, in_image, row, col, num_input_images, filter_height, filter_width)
//compute_par_index(out_image, in_image, row, col, num_input_images, filter_height, filter_width)

#define IDX(image, row, col)  \
compute_index(0, image, row, col, 1, height, width)
//compute_par_index(0, image, row, col, 1, height, width)

#else // not defined(WEIGHT_MEMORY) & defined(FIXED_POINT)

#define WIDX(out_image, in_image, row, col) \
compute_index(out_image, in_image, row, col, num_input_images, filter_height, filter_width)

#define IDX(image, row, col)  \
compute_index(0, image, row, col, 1, height, width)

#endif // else defined(WEIGHT_MEMORY) & defined(FIXED_POINT)

#ifdef WEIGHT_MEMORY

void conv2d_sw(
               int image_offset,
               int weight_offset,
               int output_image_offset,
               int num_input_images,
               int num_output_images,
               int height,
               int width,
               int filter_height,
               int filter_width,
               int relu)
{
    int  o, i, fr, fc, r, c, rr, cc;
    hw_cat_type sum;
    hw_cat_type n;
    hw_cat_type image_value;
    hw_cat_type weight_value;
    int image_index;
    int weight_index;
    int output_index;

    const int chatty = 0;
    
    for (o=0; o<num_output_images; o++) {
        for (i=0; i<num_input_images; i++) {
            for (r=0; r<height; r++) {
                for (c=0; c<width; c++) {
                    sum = 0.0;
                    for (fr=0; fr<filter_height; fr++) {
                        for (fc=0; fc<filter_width; fc++) {
                            rr = r + fr - (filter_height -1)/2;
                            cc = c + fc - (filter_width -1)/2;
                            if (in_bounds(rr, cc, height, width)) {
                                image_index = IDX(i, rr, cc);
                                weight_index = WIDX(o, i, fr, fc);
                                image_value = get_cat_value(weight_memory, image_offset + image_index);
                                weight_value = get_cat_value(weight_memory, weight_offset + weight_index);
#ifdef ARM
                                if (chatty) printf("image_index: %d weight_index: %d image_value: %5.2f weight_value: %5.2f \n",
                                                       image_index, weight_index, image_value, weight_value);
#else
                                if (chatty) printf("image_index: %d weight_index: %d image_value: %5.2f weight_value: %5.2f \n",
                                                       image_index, weight_index, image_value.to_double(), weight_value.to_double());
#endif
                                sum += image_value * weight_value;
                            }
                        }
                    }
                    output_index = IDX(o, r, c);
                    if (i==0) n = sum; else n = sum + get_cat_value(weight_memory, output_image_offset + output_index);
                    set_cat_value(weight_memory, output_image_offset + output_index, n);
                }
            }
        }
        if (relu) {
            for (r=0; r<height; r++) {
                for (c=0; c<width; c++) {
                    output_index = IDX(o, r, c);
                    n = get_cat_value(weight_memory, output_image_offset + output_index);
                    if (n<0) set_cat_value(weight_memory, output_image_offset + output_index, 0.0);
                }
            }
        }
    }
}

#else // not WEIGHT_MEMORY   24752, '53, '54

void conv2d_sw(
               host_type *images,
               host_type *weights,
               host_type *output_images,
               int num_input_images,
               int num_output_images,
               int height,
               int width,
               int filter_height,
               int filter_width,
               int relu)
{
    int  o, i, fr, fc, r, c, rr, cc;
    host_type sum;
    host_type n;
    host_type image_value;
    host_type weight_value;
    int image_index;
    int weight_index;
    int output_index;
    static float max_val = 0.0;
    static float min_val = 0.0;
    
    for (o=0; o<num_output_images; o++) {
        for (i=0; i<num_input_images; i++) {
            for (r=0; r<height; r++) {
                for (c=0; c<width; c++) {
                    sum = 0.0;
                    for (fr=0; fr<filter_height; fr++) {
                        for (fc=0; fc<filter_width; fc++) {
                            rr = r + fr - (filter_height -1)/2;
                            cc = c + fc - (filter_width -1)/2;
                            if (in_bounds(rr, cc, height, width)) {
                                image_index = IDX(i, rr, cc);
                                weight_index = WIDX(o, i, fr, fc);
                                image_value = images[image_index];
                                weight_value = weights[weight_index];
                                sum += image_value * weight_value;
                                if ((image_value * weight_value) > max_val) max_val = image_value * weight_value;
                                if ((image_value * weight_value) < min_val) min_val = image_value * weight_value;
                            }
                        }
                    }
                    output_index = IDX(o, r, c);
                    if (i==0) n = sum; else n = sum + output_images[output_index];
                    output_images[output_index] = n;
                }
            }
        }
        if (relu) {
            for (r=0; r<height; r++) {
                for (c=0; c<width; c++) {
                    output_index = IDX(o, r, c);
                    n = output_images[output_index];
                    if (n<0) output_images[output_index] = 0.0;
                }
            }
        }
    }
    printf("max_val: %5.3d min_val= %5.3d \n", max_val, min_val);
}

#endif // else not WEIGHT_MEMORY

#ifdef WEIGHT_MEMORY

void dense_sw(
              int input_image_offset,
              int weight_offset,
              int output_image_offset,
              int num_units,
              int unit_count,
              int output_image_elements)
{

    int i, n, c;
    hw_cat_type sum;
    int chatty = 0;

    for (i=0; i<output_image_elements; i++) {
        sum = 0.0;
        for (n=0; n<num_units; n++) {
            for (c=0; c<unit_count; c++) {
                sum += get_cat_value(weight_memory, input_image_offset + n * unit_count + c) * get_cat_value(weight_memory, weight_offset + (i*num_units*unit_count)+n*unit_count+c);
                if (chatty) {
#ifdef FIXED_POINT
                    if ((i<10) && (n<10) && (c<10))
#ifdef ARM
                        fprintf(stdout, "sum[%d] = %8.3f  image_data[%d] = %8.3f  weights[%d] = %8.3f \n",
                               i, sum,
                               n * unit_count + c, get_cat_value(weight_memory, input_image_offset + n * unit_count + c),
                              (i*num_units*unit_count)+n*unit_count+c, get_cat_value(weight_memory, weight_offset + ((i*num_units*unit_count)+n*unit_count+c)));
#else
                        fprintf(stdout, "sum[%d] = %8.3f  image_data[%d] = %8.3f  weights[%d] = %8.3f \n",
                               i, sum.to_double(),
                               n * unit_count + c, get_cat_value(weight_memory, input_image_offset + n * unit_count + c).to_double(),
                              (i*num_units*unit_count)+n*unit_count+c, get_cat_value(weight_memory, weight_offset + ((i*num_units*unit_count)+n*unit_count+c)).to_double());
#endif
#else // not FIXED_POINT
                    fprintf(stdout, "sum = %8.3f  image_data = %8.3f  weights = %8.3f \n", sum, input_images[n], weights[(i*input_image_elements)+n]);
#endif // else not FIXED_POINT
                }
            }
        }
        set_cat_value(weight_memory, output_image_offset + i, sum);
    }
}

#else // not WEIGHT_MEMORY

void dense_sw(
              host_type *input_images,
              host_type *weights,
              host_type *output_images,
              int input_image_elements,
              int output_image_elements)
{
    int i, n;
    host_type sum;
    
    for (i=0; i<output_image_elements; i++) {
        sum = 0.0;
        for (n=0; n<input_image_elements; n++) {
            //printf("sum = %8.3f  image_data = %8.3f  weights = %8.3f \n", sum, input_images[n], weights[(i*input_image_elements)+n]);
            sum += input_images[n] * weights[(i*input_image_elements)+n];
        }
        output_images[i] = sum;
    }
}

#endif // else not WEIGHT_MEMORY

void softmax(
             host_type *predictions,
             host_type *probabilities,
             int count)
{
    int i;
    double sum;
    double f;
    
    sum = 0.0;
    
    for (i=0;i<count;i++) {
        f = predictions[i];
        sum += exp(f);
    }
    
    for (i=0;i<count;i++) {
        probabilities[i] = exp(predictions[i])/sum;
    };
}

#ifdef WEIGHT_MEMORY
void infer(int image_offset, float *probabilities)
{
    const int chatty = 1;
    
    int layer1_out_offset  = top_of_weights + (image_height * image_width);
    int layer2_out_offset  = layer1_out_offset + layer1_output_images * (image_height * image_width);
    int layer3_out_offset  = layer2_out_offset + layer2_output_images * (image_height * image_width);
    host_type host_layer3_out[10];
    
    if (chatty) printf("sw image in: \n");
    if (chatty) print_image(image_offset, 1);
    
    if (chatty) printf("Convolution layer #1 \n");
    conv2d_sw(image_offset, layer1_weight_offset, layer1_out_offset,
              layer1_input_images, layer1_output_images, image_height, image_width, 3, 3, 1);
    
    if (chatty) printf("sw image out layer #1: \n");
    if (chatty) print_image(layer1_out_offset, layer1_output_images);
    
    if (chatty) printf("Convolution layer #2 \n");
    conv2d_sw(layer1_out_offset, layer2_weight_offset, layer2_out_offset,
              layer2_input_images, layer2_output_images, image_height, image_width, 3, 3, 1);
    
    if (chatty) printf("sw image out layer #2: \n");
    if (chatty) print_image(layer2_out_offset, layer2_output_images);
    
    if (chatty) printf("dense layer #3 \n");
    dense_sw (layer2_out_offset, layer3_weight_offset, layer3_out_offset,
              layer3_weights_cols / (image_height * image_width), image_height * image_width, layer3_weights_rows);
    
    copy_from_cat(host_layer3_out, layer3_out_offset, layer3_weights_rows);
    
    if (chatty) printf("raw sw scores... \n");
    if (chatty) for (int i=0; i<10; i++) printf("raw scores[%d] = %f \n", i, host_layer3_out[i]);
    
    softmax(host_layer3_out, probabilities, layer3_weights_rows);
}

#else // not WEIGHT_MEMORY

void infer(host_type *image, float *probabilities)
{
    static host_type layer1_out[layer1_output_images * image_height * image_width];
    static host_type layer2_out[layer2_output_images * image_height * image_width];
    static host_type layer3_out[layer3_weights_rows];
    const bool chatty = false;

    if (chatty) printf("sw image in: \n");
    if (chatty) print_image(image, 1);
    
    conv2d_sw(image, (host_type *) layer1_weights, layer1_out,
              layer1_input_images, layer1_output_images, image_height, image_width, 3, 3, 1);
    
    if (chatty) printf("sw image out: \n");
    if (chatty) print_image(layer1_out, layer1_output_images);
    
    conv2d_sw(layer1_out, (host_type *) layer2_weights, layer2_out,
              layer2_input_images, layer2_output_images, image_height, image_width, 3, 3, 1);
    
    if (chatty) printf("sw image out: \n");
    if (chatty) print_image(layer2_out, layer2_output_images);
    
    dense_sw (layer2_out, (host_type *) layer3_weights, layer3_out,
              layer3_weights_cols, layer3_weights_rows);
    
    if (chatty) printf("raw sw scores... \n");
    if (chatty) for (int i=0; i<10; i++) printf("raw scores[%d] = %f \n", i, layer3_out[i]);
    
    softmax(layer3_out, probabilities, layer3_weights_rows);
}
#endif // else not WEIGHT_MEMORY


void scale(unsigned char *input_image, host_type *output_image, int count)
{
    int i;
    
    for (i=0; i<count; i++) {
        output_image[i] = ((float) input_image[i])/255.0;
    }
}

#ifdef WEIGHT_MEMORY
#ifdef HOST

void set_memory(char *s, int offset)
{
    unsigned long n = strtoul(s, NULL, 16);
    bool    negative = false;
    float   value;
    long    bits;
    int     p;

    for (p=0; p<STRIDE; p++) {
        bits = (n >> (p * WORD_SIZE)) & ((1ULL << WORD_SIZE) - 1);
        if (bits & (1<<(WORD_SIZE-1))) {
            // number is negative
            bits = ((~bits) & ((1ULL << WORD_SIZE)-1)) + 1;
            negative = true;
        }
        value = ((float) bits)/(float(1<<FRACTIONAL_BITS));
        if (negative) value = value * -1.0;
        write_cat_memory(offset, value);
    }
}

#endif // HOST

#ifdef HOST
#ifndef ARM

void clean(char *s) {
    int i;
    int d;
    
    // remove all characters that are not hex digits
    
    i = 0;
    d = 0;
    
    while (s[i]) {
        s[d] = s[i];
        if (isxdigit(s[i])) {
            d++;
        }
        i++;
    }
    s[d] = 0;
}

void load_memory(raw_bus_type *m, host_type *image)
{
    char *r;
    char char_buf[100];
    char word[9];
    FILE *mem_image;
    unsigned long bits;
    unsigned long pbits;
    bool negative;
    int p;
    int i;
    int offset;
    host_type value;
    
#ifdef FIXED_POINT
#ifdef MAC
    char mem_image_filename[] = "/Users/russk/python/mnist_cat/fixed_weights_1.mem";
#else
    char mem_image_filename[] = "../../../../python_mnist/fixed_weights_1.mem";
#endif
#else // not FIXED_POINT
#ifdef MAC
    char mem_image_filename[] = "/Users/russk/python/mnist_cat/weights.mem";
#else // not MAC
    char mem_image_filename[] = "weights.mem";
#endif // else not MAC
#endif // else not FIXED_POINT
    
    mem_image = fopen(mem_image_filename, "r");
    //printf("loading weights from file %s \n", mem_image_filename);

    if (mem_image==NULL) {
        fprintf(stderr, "unable to open file %s for reading \n", mem_image_filename);
        perror("oops");
        exit(2);
    }
    i = 0;
    offset = 0;
    
    while (!feof(mem_image)) {
        r = fgets(char_buf, (size_t) sizeof(char_buf), mem_image);
        clean(char_buf);
        while (strlen(char_buf)) {
            strcpy(word, char_buf + strlen(char_buf)-8);
            char_buf[strlen(char_buf)-8] = 0;
            bits = strtoul(word, NULL, 16);
            for (p=0; p<1; p++) {
                pbits = bits & 0xFFFFFFFF;
                negative = false;
                if (pbits & 0x80000000) {
                    pbits = ((~pbits) & 0xFFFFFFFF) + 1;
                    negative = true;
                }
                value = ((float) pbits)/(float(1<<16));
                if (negative) value = value * -1.0;
                write_cat_memory(offset, value);
                offset++;
                bits = bits >> WORD_SIZE;
            }
        }
    }

    fclose(mem_image);
}

#endif // not ARM
#endif // HOST

#endif // WEIGHT_MEMORY

void sw_inference(unsigned char *input_image, host_type *probabilities)
{
    host_type image[image_height * image_width];
    int i;
    unsigned int start, end;
    
    scale(input_image, image, image_height * image_width);
    
#ifdef WEIGHT_MEMORY
#ifndef ARM  // load weights into memory from file
    load_memory(weight_memory, image);
#endif // not ARM

#ifdef ARM
    start = *(volatile unsigned int *) 0x90000000;
#endif

    copy_to_cat(top_of_weights, image, image_height * image_width);
    infer(top_of_weights, probabilities);

#ifdef ARM
    end = *(volatile unsigned int *) 0x90000000;
    printf("elapsed cycles: %d \n", end-start);    
#endif

#else // not WEIGHT_MEMORY
    infer(image, probabilities);
#endif // else not WEIGHT_MEMORY

    printf("software probabilities: \n");
    for (i=0; i<10; i++) {
        printf(" %d: %8.6f \n", i, probabilities[i]);
    }
    printf("\n");
}


#if defined(WEIGHT_MEMORY) & defined(FIXED_POINT)

void hw_infer(signed int image_offset, host_type *probabilities)
{
    const int chatty = 0;
    
    unsigned int  image_size    = image_height * image_width;
    unsigned int  layer1_out    = 12 * AREA + 10 + top_of_weights + image_size;
    unsigned int  layer2_out    = 12 * AREA + 10 + layer1_out + layer1_output_images * image_size;
    unsigned int  layer3_out    = 12 * AREA + 10 + layer2_out + layer2_output_images * image_size;
    host_type     layer3_out_host[layer3_weights_rows];
    
    if (chatty) printf("hw input image: @image_offset: %d \n", image_offset);
    if (chatty) print_image(image_offset, 1);
    
    conv2d_hw(image_offset, layer1_weight_offset, layer1_out,
              layer1_input_images, layer1_output_images, image_height, image_width, 3, 3, 1);
    
    if (chatty) printf("hw image out layer #1: \n");
    if (chatty) print_image(layer1_out, layer1_output_images);
    
    conv2d_hw(layer1_out, layer2_weight_offset, layer2_out,
              layer2_input_images, layer2_output_images, image_height, image_width, 3, 3, 1);
    
    if (chatty) printf("hw image out layer #2: \n");
    if (chatty) print_image(layer2_out, layer2_output_images);
    
    dense_hw(layer2_out, layer3_weight_offset, layer3_out, layer3_weights_cols, layer3_weights_rows);

    copy_from_cat(layer3_out_host, layer3_out, layer3_weights_rows);
    
    if (chatty) printf("raw hw scores... \n");
    if (chatty) for (int i=0; i<10; i++) printf("raw scores[%d] = %f \n", i, layer3_out_host[i]);
    
    softmax(layer3_out_host, probabilities, layer3_weights_rows);
}


void hw_inference(unsigned char *input_image, host_type *probabilities)
{
    int i, j;
    int hw_offset = 12 * AREA + 10;
    
    host_type image[image_height * image_width];
    host_type *image_pointer;
    unsigned int start, end;
    
    //float foo[2];

    scale(input_image, image, image_height * image_width);
    
#ifdef HOST
    load_memory(weight_memory, image);
#endif // HOST
    
    //for (i=0; i<5; i++) {image[i] = i; image[i+5] = i*2;}
    //copy_to_cat(0, image, 10);
    //copy_to_cat(10, image, 5);

    //dense_hw(10, 0, 15, 5, 2);
    //copy_from_cat(foo, 15, 2);

    //printf("foo = %d %d \n", (int) foo[0], (int) foo[1]);
    //printf("done\n");
    
#ifdef ARM
    start = * (volatile unsigned int *) 0x90000000;
#endif

    image_pointer = (host_type *) (void *) top_of_weights;
    copy_to_cat(top_of_weights + hw_offset, image, image_height * image_width);

    //for (i=0; i<10; i++) image[i] = 0.0;
    //image[4] = 1.0;


    //copy_to_cat(0, image, 10);

    //conv2d_hw(top_of_weights+hw_offset, 0, top_of_weights + hw_offset + AREA, 1, 1, 28, 28, 3, 3, 1);
    //print_image(top_of_weights+hw_offset+AREA, 1);

    hw_infer(top_of_weights + hw_offset, probabilities); 

#ifdef ARM    
    end = * (volatile unsigned int *) 0x90000000;
    printf("Elapsed cycles: %d \n", end-start);
#endif

    printf("hardware probabilities: \n");
    for (i=0; i<10; i++) {
        printf(" %d: %8.6f \n", i, probabilities[i]);
    }
    printf("\n");
}
#else // not defined(WEIGHT_MEMORY) & defined(FIXED_POINT)
void hw_inference(unsigned char *input_image, host_type *probabilities)
{
    printf("hw inference not performed, only works with weight memory and fixed point \n");
}
#endif  // else not defined(WEIGHT_MEMORY) & defined(FIXED_POINT)

int max(float *p)
{
    int i;
    int maximum = 0;
    float biggest = p[0];
    
    for (i=0; i<10; i++) {
        if (p[i]>biggest) {
            biggest = p[i];
            maximum = i;
        }
    }
    return maximum;
}


#ifndef ARM

void sweep()
{
    FILE *f = fopen("/Users/russk/python/mnist_cat/testdata.bin", "r");
    
    host_type probabilities[10];
    host_type image[image_height * image_width];
    unsigned char raw_image[image_height][image_width];
    unsigned char answer;
    size_t n;
    int tests = 0;
    int correct = 0;
    
    if (f == NULL) {
        printf("Unable to open testdata.bin for reading \n");
        perror("oops");
        return;
    }
    while (!feof(f) && (tests < 1000)) {
        n = fread(&answer, 1, 1, f);
        if (n) {
            n = fread(&raw_image, image_height * image_width, 1, f);
            if (n) {
                sw_inference(&(raw_image[0][0]), probabilities);
                if (answer == max(probabilities)) correct++;
                // else show(answer, raw_image, probabilities);
                tests++;
            }
        }
    }
    fclose(f);
    printf("tests: %d correct: %d \n", tests, correct);
    
    return;
}

#endif

void conv_test()
{
    int i;
    int j;
    int k;

    host_type array_in[28*28];
    host_type array_out[2*28*28];
    host_type null_filter[2*3*3];

    printf("array_test_start \n");
    for (i=0; i<28; i++) for (j=0; j<28; j++) array_in[i*28+j] = i + j;
    for (i=0; i<18; i++)  null_filter[i*+j] = 0.0;
    null_filter[4] = 1.0;
    null_filter[13] = 2.0;

    copy_to_cat(0, null_filter, 18);
    copy_to_cat(18, array_in, 28*28);

    conv2d_hw(2*3*3, 0, 2*3*3 + 28*28, 1, 2, 28, 28, 3, 3, 0);

    copy_from_cat(array_out, 2*3*3 + 28 * 28, 2*28*28);

    for (k=0; k<2; k++) {
       for (i=0; i<28; i++) {
          for (j=0; j<28; j++) {
             printf("%3d ", ((int) array_out[k*28*28+i*28+j]));
          }
          printf("\n");
       }
       printf("\n\n");
    }

    printf("complete \n");
}


void dense_test()
{
   int i, j;

   host_type array_in[10];
   host_type array_out[100];
   host_type weights[100];

   for (i=0; i<10; i++) array_in[i] = i+1;
   for (i=0; i<10; i++) for (j=0; j<10; j++) weights[i*10+j] = 1 + i*10+j;

   copy_to_cat(0, array_in, 10);
   copy_to_cat(10, weights, 100);

   dense_hw(0, 10, 110, 10, 10);

   copy_from_cat(array_out, 110, 10);

   for (i=0; i<10; i++) {
      printf("%5d \n", (int) array_out[i]);
   }

}

int main()
{
    // possible values for *input_image are "zero" through "nine" //
    unsigned char *input_image = (unsigned char *) four;
    host_type sw_prob[10];
    host_type hw_prob[10];
    int errors = 0;
    int i;
    
    // sweep();
    //printf("test: \n");
    //conv_test();
    //printf("done. \n"); return 0;

    printf("start sw: \n");
    sw_inference(input_image, sw_prob);
    
    printf("start hw: \n");

    hw_inference(input_image, hw_prob);
    
    for (i=0; i<10; i++) {
        if (sw_prob[i] != hw_prob[i]) errors++;
    }
    
    if (errors) {
        printf("Test failed, hw does not match sw! \n");
        return 1;
    } else {
        printf("Test passed! \n");
        return 0;
    }
    
    return 0;
}

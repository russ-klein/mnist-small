//
//  conv_par_in.hpp
//  mnist_inference
//
//  Created by Klein, Russell on 10/22/20.
//

#ifndef conv_par_in_hpp
#define conv_par_in_hpp

#define HEIGHT        28
#define WIDTH         28
#define AREA          (HEIGHT * WIDTH)
#define FILTER_HEIGHT  3
#define FILTER_WIDTH   3
#define FILTER_AREA   (FILTER_HEIGHT * FILTER_WIDTH)

#define INDEX_BITS    18
#define FILTER_BITS    3
#define PAR_BITS      (PAR_IN + BUS_WIDTH)

#define MARGIN ((WIDTH * ((FILTER_HEIGHT - 1) / 2)) + ((FILTER_WIDTH - 1) / 2))
#define MARGIN_ROUND_UP ( STRIDE * (((MARGIN) + (STRIDE - 1)) / (STRIDE)))
#define TAIL   ((WIDTH * ((FILTER_HEIGHT - 1) / 2)) - ((FILTER_WIDTH - 1) / 2)) + FILTER_WIDTH + (STRIDE - 1)
#define TAIL_ROUND_UP  ( STRIDE * (((TAIL) + (STRIDE - 1)) / (STRIDE)))

#define SHIFT_REGISTER_SIZE  ((MARGIN) + (TAIL_ROUND_UP))


typedef ac_int<INDEX_BITS, true>    index_type;
typedef ac_int<FILTER_BITS, false>  filter_index_type;
typedef ac_int<PAR_BITS, false>     p_type;
typedef ac_int<STRIDE, false>       enables_type;

typedef struct {hw_cat_type   word[STRIDE];} memory_line;

typedef ac_int<BUS_WIDTH * PAR_IN * WORD_SIZE, false> raw_memory_line;

typedef ac_channel<raw_memory_line> pipe_type;

typedef memory_line   image_type[((HEIGHT * WIDTH) + (STRIDE - 1))/STRIDE];
typedef memory_line   filter_type[((FILTER_HEIGHT * FILTER_WIDTH) + (STRIDE - 1))/STRIDE];

// accessors for memory and bus line buffers

void set_bus_word(raw_bus_type &line, index_type index, hw_cat_type word)
{
    // isolate all the slicing crap
    line.set_slc(index * WORD_SIZE, word.slc<WORD_SIZE>(0));
}

hw_cat_type get_bus_word(raw_bus_type &line, index_type index)
{
    hw_cat_type t;
    
    t.set_slc(0, line.slc<WORD_SIZE>(index * WORD_SIZE));
    
    return t;
}

void set_memory_word(raw_memory_line &line, index_type index, hw_cat_type word)
{
    line.set_slc(index * WORD_SIZE, word.slc<WORD_SIZE>(0));
}

hw_cat_type get_memory_word(raw_memory_line &line, index_type index)
{
    hw_cat_type t;
    
    t.set_slc(0, line.slc<WORD_SIZE>(index * WORD_SIZE));
    
    return t;
}

//  debug functions

void print_image(image_type *image)
{
    int r, c;
    hw_cat_type value;
    hw_cat_type *p = (hw_cat_type *) image;
    for (r=0; r<HEIGHT; r++) {
        for (c=0; c<WIDTH; c++) {
            value = p[r*WIDTH+c];
            if ((-0.001 < value) && (value < 0.001)) printf("   -  ");
            else printf("%5.3f ", value.to_double());
        }
        printf("\n");
    }
    printf("\n");
}

void print_image(bus_type *m, int addr)
{
    int r, c;
    hw_cat_type value;
    hw_cat_type *p = (hw_cat_type *) m;
    for (r=0; r<HEIGHT; r++) {
        for (c=0; c<WIDTH; c++) {
            value = p[addr + r * WIDTH + c]; // m[addr/STRIDE].buffer[(addr%STRIDE)/PAR_IN][addr%PAR_IN];
            if ((-0.001 < value) && (value < 0.001)) printf("   -  ");
            else printf("%5.3f ", value.to_double());
        }
        printf("\n");
    }
    printf("\n");
}

void print_image(raw_bus_type *m, int addr)
{
    int r, c;
    int offset, index;
    hw_cat_type value;
    
    for (r=0; r<HEIGHT; r++) {
        for (c=0; c<WIDTH; c++) {
            offset = (addr + r * WIDTH + c) / STRIDE;
            index = (addr + r * WIDTH + c) % STRIDE;
            value = get_bus_word(*(m + offset), index);
            if ((-0.001 < value) && (value < 0.001)) printf("   -  ");
            else printf("%5.3f ", value.to_double());
        }
        printf("\n");
    }
    printf("\n");
}

void print_image(raw_memory_line *m)
{
    int r, c;
    int offset, index;
    hw_cat_type value;
    
    for (r=0; r<HEIGHT; r++) {
        for (c=0; c<WIDTH; c++) {
            offset = (r * WIDTH + c) / STRIDE;
            index =  (r * WIDTH + c) % STRIDE;
            value = get_memory_word(*(m + offset), index);
            if ((-0.001 < value) && (value < 0.001)) printf("   -  ");
            else printf("%5.3f ", value.to_double());
        }
        printf("\n");
    }
    printf("\n");
}


void print_filter(filter_type filter)
{
    int r, c;
    hw_cat_type value;
    hw_cat_type *p = (hw_cat_type *) filter;
    for (r=0; r<FILTER_HEIGHT; r++) {
        for (c=0; c<FILTER_WIDTH; c++) {
            value = p[r*FILTER_WIDTH+c];
            printf("%5.3f ", value.to_double());
        }
        printf("\n");
    }
    printf("\n");
}

void print_filter(bus_type *m, int addr)
{
    int r, c;
    hw_cat_type value;
    hw_cat_type *p = (hw_cat_type *) m;
    for (r=0; r<FILTER_HEIGHT; r++) {
        for (c=0; c<FILTER_WIDTH; c++) {
            value = p[addr + r * FILTER_WIDTH + c]; // m[addr/STRIDE].buffer[(addr%STRIDE)/PAR_IN][addr%PAR_IN];
            printf("%5.3f ", value.to_double());
        }
        printf("\n");
    }
    printf("\n");
}

void print_filter(raw_bus_type *m, int addr)
{
    int r, c;
    int offset, index;
    hw_cat_type value;

    for (r=0; r<FILTER_HEIGHT; r++) {
        for (c=0; c<FILTER_WIDTH; c++) {
            offset = (addr + r * FILTER_WIDTH + c) / STRIDE;
            index = (addr + r * FILTER_WIDTH + c) % STRIDE;
            value = get_bus_word(*(m + offset), index);
            printf("%5.3f ", value.to_double());
        }
        printf("\n");
    }
    printf("\n");
}


void print_shift_register(hw_cat_type *sr)
{
    int i;
    for (i=0;i<WIDTH;i++) printf("%5.3f ", sr[i].to_double()); printf ("\n");
    for (i=WIDTH;i<WIDTH*2;i++) printf("%5.3f ", sr[i].to_double()); printf("\n");
    for (i=WIDTH*2;i<WIDTH*2+FILTER_WIDTH+STRIDE-1;i++) printf("%5.3f ", sr[i].to_double()); printf("\n");
}

bool hw_in_bounds(
                  index_type r,
                  index_type c,
                  index_type height,
                  index_type width)
{
    if (r < 0)        return false;
    if (r >= height)  return false;
    if (c < 0)        return false;
    if (c >= width)   return false;
    return true;
}

void increment(index_type &row, index_type &col, index_type width)
{
    col++;
    if (col == width) {
        col = 0;
        row++;
    }
}

void add(index_type &row, index_type &col, index_type amount, index_type width)
{
    col += amount;
    while (col >= width) {
        col = col - width;
        row++;
    }
}

void compute_pr_pc(index_type row, index_type col, p_type p, index_type &pr, index_type &pc)
{
    pr = row;
    pc = p + col;
    if (pc >= WIDTH) {
        pr = row + 1;
        pc -= WIDTH;
    }
}

hw_cat_type relu_fn(hw_cat_type n)
{
    if (n<0) return 0;
    return n;
}


void read_line(hw_cat_type *data, index_type data_addr, raw_memory_line *array_memory, index_type array_addr, index_type size)
{
    raw_memory_line t;
    index_type d_offset;
    index_type mem_addr;
    index_type count;
    index_type diff;
    index_type col;
    index_type row;
    index_type min;
    index_type max;
    p_type p;
    
    static const index_type stride = STRIDE;
    
    row = array_addr / stride;
    col = array_addr % stride;
    
    min = array_addr;
    max = array_addr + size;
    
    d_offset = data_addr;
    mem_addr = row * stride;
        
    diff = min - mem_addr;
    
    count = 0;
   #pragma hls_pipeline_init_interval 1
    while (count < size) {
        t = array_memory[row];
        
#pragma hls_unroll
        for (p=0; p<stride; p++) {
            if ((min <= (mem_addr + p)) && ((mem_addr + p) < max)) {
                data[d_offset + p - diff] = get_memory_word(t, p); // test me
                //data[d_offset] = get_line_word(t, p);
                //data[d_offset].set_slc(0, t.slc<WORD_SIZE>(p*WORD_SIZE));
                //data[d_offset] = t.word[p];
                
                // d_offset++;
                //count++;
            }
        }
        d_offset += (stride - diff); // test me
        count += (stride - diff);    // test me
        diff = 0;
        mem_addr += stride;
        row++;
    }
}


void write_line(raw_memory_line *array_memory, index_type array_addr, hw_cat_type *data, index_type data_addr, index_type size)
{
    raw_memory_line t;
    hw_cat_type buffer[STRIDE];
    index_type d_offset;
    index_type mem_addr;
    index_type count;
    index_type diff;
    index_type col;
    index_type row;
    index_type min;
    index_type max;
    p_type p;
    
    static const index_type stride = STRIDE;
    
    row = array_addr / stride;
    col = array_addr % stride;
    
    min = array_addr;
    max = array_addr + size;
    
    d_offset = data_addr;
    mem_addr = row * stride;
    
    diff = min - mem_addr;
    
    count = 0;

    while (count < size) {
        if ((((col != 0) && (count == 0))) || ((size - count) < stride)) t = array_memory[row];
        
#pragma hls_unroll
        for (p=0; p<stride; p++) {
            buffer[p] = ((min <= (mem_addr + p)) && ((mem_addr + p) < max)) ? data[d_offset - diff + p] : get_memory_word(t, p);
            set_memory_word(t, p, buffer[p]);
                //-set_line_word(t, p, data[d_offset]);
                //t.set_slc(p * WORD_SIZE, data[d_offset].slc<WORD_SIZE>(d_offset * WORD_SIZE));
                //t.word[p] = data[d_offset];
                //-d_offset++;
                //-count++;
            //}
        }

        d_offset += stride - diff;
        count += stride - diff;
        diff = 0;
        
        array_memory[row] = t;

        mem_addr += stride;
        row++;
    }
}


void copy_to_regs(hw_cat_type *dst, index_type dst_offset, raw_memory_line *src, index_type src_offset, index_type size)
{
    // read out of internal memories to an array of registers
    // *should* make it easy for catapult to pipeline access to internal memories
    
    index_type count;
    index_type n;
    
    static const index_type stride = STRIDE;
    
    count = 0;
    while (count < size) {
        n = stride;
        if ((size - count) < stride) n = size - count; // mis-aligned at the end of transfer
        read_line(dst, dst_offset, src, src_offset, n);
        count += n;
        src_offset += n;
        dst_offset += n;
    }
}

void copy_from_regs(raw_memory_line *dst, index_type dst_offset, hw_cat_type *src, index_type src_offset, index_type size)
{
    // write into internal memories from an array of registers
    // *should* make it easy for catapult to pipeline access to internal memories
    
    index_type count;
    index_type n;
    
    static const index_type stride = STRIDE;
    
    count = 0;
    while (count < size) {
        n = stride;
        if ((size - count) < stride) n = size - count;
        write_line(dst, dst_offset, src, src_offset, n);
        count += n;
        src_offset += n;
        dst_offset += n;
    }
}


#ifdef SLAVE

void load_feature_map_slave(pipe_type *input_channel, raw_memory_line *input_image)
{
    index_type    i;
    
    static const index_type message_count = (AREA + (STRIDE-1))/STRIDE;
    
    for (i=0; i<message_count; i++) {
        input_image[i] = input_channel->read();
    }
}


void load_filter_slave(pipe_type *filter_channel, raw_memory_line *filter)
{
    index_type    i;
    
    static const index_type message_count = (FILTER_AREA + (STRIDE-1))/STRIDE;
    
    for (i=0; i<message_count; i++) {
        filter[i] = filter_channel->read();
    }
}


void write_output_image_slave(pipe_type *image_out_channel, raw_memory_line *output_image)
{
    raw_memory_line  line;
    index_type   i;
    
    const index_type message_count = (AREA + (STRIDE-1))/STRIDE;
    
    each_output_pixel:
    for (i=0; i<message_count; i++) {
        line = output_image[i];
        image_out_channel->write(line);
    }
}

void load_dense_features_slave(pipe_type *dense_in_channel, index_type num_input_elements, raw_memory_line *input_image)
{
    raw_memory_line  line;
    index_type   image_index;
    index_type   i;
    p_type       p;
    
    static const bool chatty = false;

    const index_type message_count = (num_input_elements + (STRIDE-1))/STRIDE;
    
    image_index = 0;

   #pragma hls_pipeline_init_interval 1
    for (i=0; i<message_count; i++) {
        input_image[i] = dense_in_channel->read();
        if (chatty) {
            for (p=0; p<STRIDE; p++) {
                printf("word[%d]: %5.3f \n", (i * STRIDE + p).to_int(), get_memory_word(input_image[i], p).to_double());
            }
        }
    }
}

hw_cat_type load_weights_and_multiply_slave(pipe_type *dense_in_channel, index_type num_input_elements, raw_memory_line *input_image)
{
    raw_memory_line  feature_line;
    raw_memory_line  weight_line;
    hw_cat_type  weight_value;
    hw_cat_type  image_value;
    hw_cat_type  sum;
    index_type   count;
    index_type   n;
    p_type       p;
    
    static const index_type stride = STRIDE;
    static const bool chatty = false;
    static int sum_no=0;
    
    const index_type message_count = (num_input_elements + (STRIDE-1))/ STRIDE;

    sum = 0.0;
    count = 0;

   #pragma hls_pipeline_init_interval 1
    for (n=0; n<message_count; n++) {
        weight_line = dense_in_channel->read();
        feature_line = input_image[n];
       #pragma hls_unroll
        for (p=0; p<STRIDE; p++) {
            weight_value = get_memory_word(weight_line, p);
            image_value = get_memory_word(feature_line, p);
            sum += ((count + p) < num_input_elements) ? weight_value * image_value : 0.0;
            if (chatty) printf("sum[%d] = %8.3f  image_data[%d] = %8.3f  weights[%d] = %8.3f \n",
                                   sum_no, sum.to_double(), (count+p).to_int(), image_value.to_double(), (n*STRIDE+p).to_int(), weight_value.to_double());
        }
        count += stride;
    }
    sum_no++;
    return sum;
}

void write_dense_out_slave(pipe_type *dense_out_channel, raw_memory_line *dense_out, index_type num_output_elements)
{
    raw_memory_line  line;
    hw_cat_type  value;
    index_type   count;
    index_type   o;
    p_type       p;
    
    static const index_type stride = STRIDE;

    const index_type message_count = (num_output_elements + (STRIDE-1))/STRIDE;
    
    count = 0;

    for (o=0; o<message_count; o++) {
       #pragma hls_unroll
        for (p=0; p<STRIDE; p++) {
            value = ((count + p) < num_output_elements) ? get_bus_word(dense_out[o], p) : 0.0;
            set_memory_word(line, p, value);
        }
        count += stride;
        dense_out_channel->write(line);
    }
}


#else

index_type load_input_buffer(
                             raw_bus_type input_buffer,
                             hw_cat_type *array_buffer,
                             
                             index_type   bus_min,
                             index_type   bus_max,
                             index_type  &bus_line_start,
                             index_type  &bus_line_end,
                             index_type   start_address,
                             index_type   end_address,
                             index_type   bus_alignment_offset
                             )
{
    // loads STRIDE words into the the array buffer
    hw_cat_type  t;
    index_type   bus_word_offset;
    index_type   current_address;
    index_type   bus_address;
    index_type   count;
    uint1        bus_word_valid;
    uint1        in_range;
    
    index_type   w;
    p_type       p;
    
    static const bool chatty = false;
    static const index_type stride = STRIDE;
    
    count = 0;
    current_address = 0;
    
    bus_address = bus_line_start;
    
#pragma hls_unroll
    for (p=0; p<STRIDE; p++) {
        bus_word_valid = ((bus_min <= bus_address) && (bus_address < bus_max)) ? 1 : 0;
        t = (bus_word_valid) ? get_bus_word(input_buffer, p) : 0.0;
        array_buffer[current_address] = t;
            
        current_address += (bus_word_valid) ? 1 : 0;
        count += (bus_word_valid) ? 1 : 0;
            
        if (bus_word_valid) {
            if (chatty) printf("loaded array_buffer[%d] = %5.3f \n", current_address.to_int(), t.to_double());
        }
        
        bus_address++;
    }

    bus_line_start += stride;
    bus_line_end += stride;
    
    return count;
}


void load_from_system_memory(raw_bus_type *memory, index_type offset, index_type size, raw_memory_line *input_array, index_type array_offset)
{
    raw_bus_type input_buffer;
    hw_cat_type  transfer_buffer[STRIDE];
    
    index_type   bus_min;
    index_type   bus_max;
    index_type   bus_read_address;
    index_type   bus_line_start;
    index_type   bus_line_end;
    index_type   bus_alignment_offset;
    
    index_type   array_min;
    index_type   array_max;
    index_type   array_address;
    index_type   array_line_start;
    index_type   array_line_end;
    index_type   array_alignment_offset;
    
    index_type   alignment_offset;
    uint1        array_unaligned;
    uint1        bus_unaligned;
    index_type   start_address;
    index_type   end_address;
    index_type   num;
    index_type   count;
    index_type   b;
    
    static const bool          chatty          = false;
    static const index_type    bus_width_bits  = BUS_WIDTH_BITS;
    static const p_type        par_in          = PAR_IN;
    static const index_type    stride          = STRIDE;
    
    if (chatty) printf("reading: %d values from memory at address: %d \n", size.to_int(), offset.to_int());
    bus_min = offset;
    bus_max = offset + size;
    
    bus_read_address = (offset / par_in) >> bus_width_bits;
    bus_line_start = bus_read_address * stride;
    bus_line_end = bus_line_start + stride;
    
    bus_alignment_offset = bus_min - bus_line_start;
    
    bus_unaligned   = (bus_alignment_offset != 0)   ? 1 : 0;
    
    start_address = array_offset;
    end_address = stride;
    array_max = array_offset + size;
    
    count = 0;
    
   #pragma hls_pipeline_init_interval 1
    while (count<size) {
        
        input_buffer = memory[bus_read_address];
        if (chatty) {
            printf("read line: %d at address: %d \n", bus_read_address.to_int(), bus_read_address.to_int() * STRIDE);
            for (int i=0; i<STRIDE; i++) {
                hw_cat_type t;
                t.set_slc(0, input_buffer.slc<WORD_SIZE>(i*WORD_SIZE));
                printf("%5.3f ", t.to_double());
            }
            printf("\n");
        }
        
        num = load_input_buffer(input_buffer, transfer_buffer, bus_min, bus_max, bus_line_start, bus_line_end,
                              start_address, end_address, bus_alignment_offset);
        count += num;
        bus_read_address++;
        
        copy_from_regs(input_array, start_address, transfer_buffer, 0, num);
        
        start_address += num;
        end_address += num;
    }
}

void load_input_array(
    raw_bus_type &output_buffer,
    hw_cat_type  *array_buffer,
    index_type    bus_min,
    index_type    bus_max,
    index_type    bus_line_start,
    index_type    bus_line_end,
    index_type    bus_alignment_offset)
{
    hw_cat_type   t;
    index_type    bus_address;
    uint1         bus_word_valid;
    index_type    word_offset;
    p_type        p;
    
    static const bool chatty = false;

   #pragma hls_unroll
    for (p=0; p<STRIDE; p++) {
        bus_address = bus_line_start + p;
        bus_word_valid = (((bus_min<=bus_address) && (bus_address<bus_max)) &&
                            ((bus_line_start<=bus_address) && (bus_address<bus_line_end)) &&
                            (bus_alignment_offset <= p))  ? 1 : 0;
        t = (bus_word_valid) ? array_buffer[p - bus_alignment_offset] : get_bus_word(output_buffer, p);
        set_bus_word(output_buffer, p, t);
        if (bus_word_valid) {
            if (chatty) printf("wrote memory[%d] = %5.3f \n", bus_address.to_int(), t.to_double());
        }
    }
}


void store_into_system_memory(raw_memory_line *output_array, index_type array_offset, index_type size,  raw_bus_type *memory, index_type offset)
{
    raw_bus_type output_buffer;
    index_type   line_count;
    hw_cat_type  array_buffer[STRIDE];
    
    index_type   bus_min;
    index_type   bus_max;
    index_type   bus_address;
    index_type   bus_write_address;
    index_type   bus_line_start;
    index_type   bus_line_end;
    index_type   bus_alignment_offset;
    index_type   bus_word_offset;
    uint1        bus_word_valid;
    
    index_type   array_min;
    index_type   array_max;
    index_type   array_size;
    index_type   array_address;
    index_type   array_line_start;
    index_type   array_line_end;
    index_type   array_alignment_offset;
    
    index_type   remaining_words;
    uint1        array_unaligned;
    uint1        bus_unaligned;
    uint1        first;
    uint1        last;
    index_type   start_address;
    index_type   end_address;
    index_type   count;
    index_type   w;
    index_type   b;
    p_type       p;
    
    static const bool       chatty         = false;
    static const index_type bus_width_bits = BUS_WIDTH_BITS;
    static const p_type     par_in         = PAR_IN;
    static const index_type bus_line_width = STRIDE;
    static const index_type stride         = STRIDE;

    first = 1;
    
    bus_min = offset;
    bus_max = offset + size;
    
    bus_write_address = (offset / par_in) >> bus_width_bits;
    bus_line_start = bus_write_address * bus_line_width;
    bus_line_end = bus_line_start + bus_line_width;
    
    array_min = array_offset;
    array_max = array_offset + size;
    
    array_line_start = (array_min / par_in) * par_in;
    array_line_end = array_line_start + stride;
    
    bus_alignment_offset = bus_min - bus_line_start;
    array_alignment_offset = array_offset % par_in;
    
    array_unaligned = (array_alignment_offset != 0) ? 1 : 0;
    bus_unaligned   = (bus_alignment_offset != 0)   ? 1 : 0;
    
    start_address = array_offset;
    end_address = array_offset + bus_line_width;
    
    line_count = 0;
    
    count = 0;
    while (count<size) {
        
        array_size = stride;
        if ((start_address + stride) > array_max) array_size = array_max - start_address;
        copy_to_regs(array_buffer, 0, output_array, start_address, array_size);
        
        if (first) start_address += stride - bus_alignment_offset;
        else start_address += stride;
        
        if (first && ((size < bus_line_width) || (bus_min != bus_line_start))) {
            output_buffer= memory[bus_write_address];
            if (chatty) printf("read/modify/write at start \n");
        }
        
        else if ((size > bus_line_width) && ((bus_line_start + bus_line_width) > bus_max)) { // last bus line processed
            output_buffer = memory[bus_write_address+1];
            if (chatty) printf("read/modify/write at end \n");
        }

        load_input_array(output_buffer, array_buffer, bus_min, bus_max, bus_line_start, bus_line_end, bus_alignment_offset);

        if (first) count += bus_line_width - bus_alignment_offset;
        else count += bus_line_width;
        
        memory[bus_write_address] = output_buffer;

        bus_line_start += bus_line_width;
        bus_line_end += bus_line_width;
        
        remaining_words = size - count;
        
        bus_write_address++;
        bus_alignment_offset = 0;
        first = 0;
    }
}

hw_cat_type load_weights_and_multiply_master(raw_bus_type *memory, index_type offset, index_type num_input_elements, raw_memory_line *input_image)
{
    raw_memory_line input_buffer;
    hw_cat_type     values[STRIDE];
    hw_cat_type     weights[STRIDE];
    hw_cat_type     sum;
    index_type      count;
    index_type      i;
    
    static const bool chatty = false;
    static const index_type stride = STRIDE;
    
    count = 0;
    sum = 0.0;
    
    while (count < num_input_elements) {
        load_from_system_memory(memory, offset + count, stride, &input_buffer, 0);
        copy_to_regs(values, 0, input_image, count, stride);
        copy_to_regs(weights, 0, &input_buffer, 0, stride);
       #pragma hls_unroll
        for (i=0; i<STRIDE; i++) {
            sum += (count < num_input_elements) ? values[i] * weights[i] : 0.0;
            count += (count < num_input_elements) ? 1 : 0;
            if (count < num_input_elements) {
                if (chatty) printf("sum: %5.3f = image value: %5.3f * weight: %5.3f \n",
                                   sum.to_double(), values[i].to_double(), weights[i].to_double());
            }
        }
    }
    return sum;
}
        
#endif

void compute_row_col(index_type n, index_type &r, index_type &c)
{
    static const index_type width = WIDTH;
    
    r = n / width;
    c = n % width;
    if (c<0) {
        c += width;
        r--;
    }
}

void shift_by_stride(hw_cat_type *shift_register, hw_cat_type *input_image)
{
    index_type sr;
    
    static const bool chatty = false;
    
    if (chatty) {
        printf("Shifting in: ");
        for (int i=0; i<STRIDE; i++) printf("%5.3f ", input_image[i].to_double());
        printf("\n");
    }
    
   #pragma hls_unroll
    for (sr=0; sr<SHIFT_REGISTER_SIZE-STRIDE; sr++) {
        shift_register[sr] = shift_register[sr+STRIDE];
    }
    
   #pragma hls_unroll
    for (sr=0; sr<STRIDE; sr++) {
        shift_register[SHIFT_REGISTER_SIZE-STRIDE+sr] = input_image[sr];
    }
}

void get_shift_in_values(hw_cat_type *values, raw_memory_line *image, index_type n, index_type num_words)
{
    hw_cat_type line[STRIDE];
    index_type size;
    p_type p;

    static const index_type area = AREA;
    static const bool chatty = false;
    
    size = num_words;
    
    if (n < 0) {
        size = n + size;
        if (size > 0) {
            copy_to_regs(line, -n, image, 0, size);
        }
    } else {
        if ((n + size) > area) size = area - n;
        if (size > 0) {
            copy_to_regs(line, 0, image, n, size);
        }
    }
    
   #pragma hls_unroll
    for (p=0; p<STRIDE; p++) {
        values[p] = (((n + p) < 0) || ((n + p) >= area)) ? 0.0 : line[p];
    }
    if (chatty) {
        printf("shift in values: ");
        for (int i=0; i<STRIDE; i++) printf("%5.3f ", values[i].to_double());
        printf("\n");
    }
}

void perform_convolution(
                         raw_memory_line   *input_image,
                         raw_memory_line   *filter,
                         raw_memory_line   *output_image,
                         index_type         input_image_number)
{
    hw_cat_type partial_sum_buffer[STRIDE];
    hw_cat_type products[STRIDE][FILTER_HEIGHT * FILTER_WIDTH];
    hw_cat_type sums;
    hw_cat_type feature_load[STRIDE];
    static hw_cat_type shift_register[SHIFT_REGISTER_SIZE];

    // registers for computations
    hw_cat_type filter_regs[FILTER_HEIGHT * FILTER_WIDTH];
    hw_cat_type input_regs[STRIDE];
    hw_cat_type output_regs[STRIDE];
    
    filter_index_type fr;
    filter_index_type fc;
    index_type output_index;
    index_type loop_entry;
    index_type image_index;
    index_type target_pixel;
    index_type tail_pixel;
    index_type lead_pixel;
    index_type shift_offset;
    index_type p_lead_pixel;
    index_type p_image_index;
    index_type p_target_pixel;
    index_type f_index;
    index_type p_index;
    index_type num;
    index_type row;
    index_type col;
    index_type pr;
    index_type pc;
    index_type rr;
    index_type cc;
    p_type p;
    
    static const index_type tail_round_up = TAIL_ROUND_UP - STRIDE;
    static const index_type margin_round_up = MARGIN_ROUND_UP;
    static const index_type area = AREA;
    static const index_type mid_point_height = (FILTER_HEIGHT - 1) / 2;
    static const index_type mid_point_width  = (FILTER_WIDTH - 1) / 2;
    static const index_type stride = STRIDE;
    static const index_type pixels_to_shift = AREA + SHIFT_REGISTER_SIZE;
    static const bool chatty = false;
    
    // lead_pixel = the number of the pixel at the start of the shift_register
    // target_pixel = the number of the pixel at the center of the convolution kernel (lead_pixel + margin)
    // tail_pixel = the last pixel in the shift register (lead_pixel + shift_register_size)
    // total pixels needed to be shifted through is AREA + SHIFT_REGISTER_SIZE - (STRIDE -1)
    
    copy_to_regs(filter_regs, 0, filter, 0, FILTER_AREA);
    
   #pragma hls_pipeline_init_interval 1
main_convolve_loop:
    for (tail_pixel = 0; tail_pixel < pixels_to_shift; tail_pixel += stride) {

        target_pixel = tail_pixel - margin_round_up;
        lead_pixel = target_pixel - tail_round_up;
        
        compute_row_col(lead_pixel, row, col);

        get_shift_in_values(feature_load, input_image, target_pixel, stride);
        
        shift_by_stride(shift_register, feature_load);
        
        if ((target_pixel  < 0) || (target_pixel > area) || (input_image_number == 0)) {
           #pragma hls_unroll
            for (p=0; p<STRIDE; p++) {
                partial_sum_buffer[p] = 0.0;
            }
        } else {
            copy_to_regs(partial_sum_buffer, 0, output_image, lead_pixel, stride);
        }
        
       #pragma hls_unroll
        for (p=0; p<STRIDE; p++) {
            p_target_pixel = target_pixel + p;
            p_lead_pixel = lead_pixel + p;
            compute_row_col(p_lead_pixel, pr, pc);
            
            sums = 0;
            
            if ((0 <= p_lead_pixel) && (p_lead_pixel < area)) {
                
               #pragma hls_unroll
            conv_outer_loop:
                for (fr=0; fr<FILTER_HEIGHT; fr++) {
                    
                   #pragma hls_unroll
                conv_inner_loop:
                    for (fc=0; fc<FILTER_WIDTH; fc++) {
                        
                        rr = pr + fr - mid_point_height;
                        cc = pc + fc - mid_point_width;
                        shift_offset = fr * WIDTH + fc + p;
                        f_index = fr * FILTER_WIDTH + fc;
                        
                        products[p][f_index] = (hw_in_bounds(rr, cc, HEIGHT, WIDTH)) ? filter_regs[f_index] * shift_register[shift_offset] : 0.0;
                        
                        if (chatty) {
                            if (hw_in_bounds(rr, cc, HEIGHT, WIDTH)) {
                                printf("image_value[%d][%d]: %5.3f weight_value: %5.3f \n", rr.to_int(), cc.to_int(), shift_register[shift_offset].to_double(), filter_regs[f_index].to_double());
                            }
                        }

                        sums += products[p][f_index];
                    }
                }
                
                if (chatty) printf("sum[%d][%d] = %5.3f \n", pr.to_int(), pc.to_int(), sums.to_double());
                
                partial_sum_buffer[p] += sums;
                if (chatty) {
                    if ((output_index % WIDTH)==0) printf("\n");
                    if (sums <0.001) printf("  -   ");
                    else printf("%5.2f ", sums.to_double());
                }
            }
        }
        if ((0 <= lead_pixel) && (lead_pixel < area)) {
            num = stride;
            if ((area - lead_pixel) < stride) num = area - lead_pixel;
            copy_from_regs(output_image, lead_pixel, partial_sum_buffer, 0, num);
        }
    }
}


void perform_relu(bool relu, raw_memory_line *image_out, raw_memory_line *image_in)
{
    hw_cat_type values[STRIDE];
    index_type count;
    index_type i;
    
    static const index_type stride = STRIDE;
    static const index_type area = AREA;
    
   #pragma hls_pipeline_init_interval 1
    for (count = 0; count < area; count += stride) {
        copy_to_regs(values, 0, image_in, count, stride);
       #pragma hls_unroll
        for (i=0; i<STRIDE; i++) {
            values[i] = (relu) ? relu_fn(values[i]) : values[i];
        }
        copy_from_regs(image_out, count, values, 0, stride);
    }
}

hw_cat_type get_cat_value(raw_bus_type *m, int offset);


#pragma hls_design top
void conv_par_in(
                 cat_memory_type &debug_signal,
                 ac_channel<bool> &go,
                 ac_channel<bool> &done,
                 bool relu,
                 bool convolve,
                 bool fully_connected,
#ifdef SLAVE
                 pipe_type &image_in_channel,
                 pipe_type &filter_in_channel,
                 pipe_type &image_out_channel,
                 pipe_type &dense_in_channel,
                 pipe_type &dense_weights_channel,
                 pipe_type &dense_out_channel,
#else // MASTER
                 raw_bus_type memory    [0x100000],
                 index_type image_offset,
                 index_type weight_offset,
                 index_type output_offset,
#endif
                 index_type num_input_images,
                 index_type num_output_images)
{
    index_type   i;
    index_type   o;
    index_type   image_pointer;
    index_type   weight_pointer;
    index_type   output_pointer;
    
#ifdef SLAVE
    
    p_type       p;
    hw_cat_type  value;
    raw_memory_line line;
    
    raw_memory_line output_image_pr_mem[((HEIGHT * WIDTH) + (STRIDE - 1))/STRIDE];
    raw_memory_line output_image_mem[((HEIGHT * WIDTH) + (STRIDE - 1))/STRIDE];
    raw_memory_line input_image_mem[8][((HEIGHT * WIDTH) + (STRIDE - 1))/STRIDE];
    raw_memory_line filter_mem[((FILTER_HEIGHT * FILTER_WIDTH) + (STRIDE - 1))/STRIDE];
    raw_memory_line dense_in_mem[((3*AREA) + (STRIDE -1))/STRIDE];
    raw_memory_line dense_out_mem[(10 + (STRIDE -1))/STRIDE];

#else // MASTER
    
    p_type       p;
    index_type   count;
    hw_cat_type  dense_out_buffer[STRIDE];

    index_type   out_pointer;
    typedef memory_line   image_type[((HEIGHT * WIDTH) + (STRIDE - 1))/STRIDE];
    typedef memory_line   filter_type[((FILTER_HEIGHT * FILTER_WIDTH) + (STRIDE - 1))/STRIDE];

    raw_memory_line output_image_pr_mem[((HEIGHT * WIDTH) + (STRIDE - 1))/STRIDE];
    raw_memory_line output_image_mem[((HEIGHT * WIDTH) + (STRIDE - 1))/STRIDE];
    raw_memory_line input_image_mem[8][((HEIGHT * WIDTH) + (STRIDE - 1))/STRIDE];
    raw_memory_line filter_mem[((FILTER_HEIGHT * FILTER_WIDTH) + (STRIDE - 1))/STRIDE];
    raw_memory_line dense_in_mem[((3*AREA) + (STRIDE -1))/STRIDE];
    raw_memory_line dense_out_mem[(10 + (STRIDE -1))/STRIDE];

    static const index_type filter_size = FILTER_AREA;
    static const index_type image_size  = AREA;
    static const index_type stride = STRIDE;

#endif
    
    go.read();
    
    if (convolve) {
        
#ifdef SLAVE
        
        for (i=0; i<num_input_images; i++) {
            load_feature_map_slave(&image_in_channel, input_image_mem[i]);
        }
        
        for (o=0; o<num_output_images; o++) {
            for (i=0; i<num_input_images; i++) {
                load_filter_slave(&filter_in_channel, filter_mem);
                perform_convolution(input_image_mem[i], filter_mem, output_image_pr_mem, i);
            }
            perform_relu(relu, output_image_mem, output_image_pr_mem);
            write_output_image_slave(&image_out_channel, output_image_mem);
        }
        
#else // MASTER
        
        image_pointer = image_offset;
        for (i=0; i<num_input_images; i++) {
            // load feature map from external memory into internal memory
            load_from_system_memory(memory, image_pointer, image_size, input_image_mem[i], 0);
            image_pointer += image_size;
        }
        
        output_pointer = output_offset;
        weight_pointer = weight_offset;
        for (o=0; o<num_output_images; o++) {
            for (i=0; i<num_input_images; i++) {
                // load filter from external memory into internal memory
                load_from_system_memory(memory, weight_pointer, filter_size, filter_mem, 0);
                perform_convolution(input_image_mem[i], filter_mem, output_image_pr_mem, i);
                weight_pointer += filter_size;
            }
            perform_relu(relu, output_image_mem, output_image_pr_mem);
            store_into_system_memory(output_image_mem, 0, HEIGHT*WIDTH, memory, output_pointer);
            output_pointer += image_size;
        }
        
#endif
    }
    
    else if (fully_connected) {
#ifdef SLAVE
        load_dense_features_slave(&dense_in_channel, num_input_images, dense_in_mem);
        
        for (o=0; o<(num_output_images + (STRIDE-1))/STRIDE; o++) {
            for (p=0; p<STRIDE; p++) {
                if ((o * STRIDE + p) < num_output_images) {
                    value = load_weights_and_multiply_slave(&dense_weights_channel, num_input_images, dense_in_mem);
                } else {
                    value = 0.0;
                }
                set_memory_word(line, p, value);
            }
            dense_out_mem[o] = line;
        }
        write_dense_out_slave(&dense_out_channel, dense_out_mem, num_output_images);
#else
        load_from_system_memory(memory, image_offset, num_input_images, dense_in_mem, 0);
        
        count = 0;
        out_pointer = 0;
        while (count<num_output_images) {
            for (p=0; p<stride; p++) {
                if ((out_pointer + p) < num_output_images) {
                    dense_out_buffer[p] = load_weights_and_multiply_master(memory, weight_offset + num_input_images * count, num_input_images, dense_in_mem);
                    count++;
                } else {
                    dense_out_buffer[p] = 0.0;
                }
            }
            copy_from_regs(dense_out_mem, out_pointer, dense_out_buffer, 0, stride);
            out_pointer += stride;
        }

        store_into_system_memory(dense_out_mem, 0, num_output_images, memory, output_offset);
#endif
    }
    done.write(1);
}
#endif /* conv_par_in_hpp */

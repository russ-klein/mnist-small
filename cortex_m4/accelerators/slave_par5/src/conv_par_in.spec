# name,  bitwidth, type, connection (wire|channel)

#debug_signal,        32, unsigned, output, wire
go,                   1, unsigned, input,  channel
done,                 1, unsigned, output, channel
relu,                 1, unsigned, input, wire
convolve,             1, unsigned, input, wire
fully_connected,      1, unsigned, input, wire
image_in_channel,    30, unsigned, input, channel
filter_in_channel,   30, unsigned, input, channel
image_out_channel,   30, unsigned, output, channel
dense_in_channel,    30, unsigned, input, channel
dense_weights_channel, 30, unsigned, input, channel
dense_out_channel,   30, unsigned, output, channel
num_input_images,    18, unsigned, input, wire
num_output_images,   18, unsigned, input, wire


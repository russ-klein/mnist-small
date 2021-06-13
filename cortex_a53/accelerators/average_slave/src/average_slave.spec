# name,  bitwidth, type, connection (wire|channel)

#debug_signal,        32, unsigned, output, wire
go,                   1, unsigned, input,  channel
done,                 1, unsigned, output, channel
values,              23, unsigned, input,  channel
answer,              23, unsigned, output,  wire
count,                9, unsigned, input,  wire


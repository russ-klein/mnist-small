import mnist_conv as m
import write_image as w
w.write_header_file(m.model.weights, 28, 28)
w.write_memory_image_file(m.model.weights, 28, 28)


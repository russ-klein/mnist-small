# Baseline MLP for MNIST dataset
from keras.datasets import mnist
from keras.models import Sequential
from keras.layers import Dense, Conv2D, Flatten
from keras.utils import np_utils

# load data
(X_train, y_train), (X_test, y_test) = mnist.load_data()

# flatten 28*28 images to a 784 vector for each image
num_pixels = X_train.shape[1] * X_train.shape[2]

X_train = X_train.reshape((X_train.shape[0], X_train.shape[1], X_train.shape[2], 1)).astype('int32');
X_test = X_test.reshape((X_test.shape[0], X_test.shape[1], X_test.shape[2], 1)).astype('int32');

# normalize inputs from 0-255 to 0-1
X_train = X_train / 255
X_test = X_test / 255

# one hot encode outputs
y_train = np_utils.to_categorical(y_train)
y_test = np_utils.to_categorical(y_test)

num_classes = y_test.shape[1]

# define baseline model
def baseline_model(num_conv_out):
	# create model
	model = Sequential()
	model.add(Conv2D(num_conv_out, (3,3), use_bias=False, padding="same", activation="relu", input_shape=(28,28,1)))
	model.add(Conv2D(3, (3,3), use_bias=False, padding="same", activation="relu"))
	model.add(Flatten())
	model.add(Dense(num_classes, use_bias=False, kernel_initializer='normal', activation='softmax'))
	# Compile model
	model.compile(loss='categorical_crossentropy', optimizer='adam', metrics=['accuracy'])
	return model

n=8
model = baseline_model(n)
model.fit(X_train, y_train, validation_data=(X_test, y_test), epochs=20, batch_size=200, verbose=1)
scores = model.evaluate(X_test, y_test, verbose=0)
print(n, "Baseline Error: %.2f%%" % (100-scores[1]*100))


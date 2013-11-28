#include <armadillo>
#include <algorithm>
#include <random>

template<typename T, int x_size, int y_size>
struct Array2D {
  std::array<T, x_size * y_size> data;
  T& operator() (int x, int y) {
    return data.at(y + y_size*x);
  }
  decltype(data.begin()) row(int x) {
    return data.begin()+x*y_size;
  }
};

struct Squared_Error {
  static inline float error(float target, float result) {
    return 1/2.0f * (target - result) * (target - result);
  }
  static inline float error_dir(float target, float result) {
    return (target - result);
  }
};

struct Logistic {
  static inline float activation(float k) {
    return 1 / (1 + exp(-k));
  }

  static inline float activation_dir(float k) {
    return k * (1 - k);
  }
};

template <int input_size, int hidden_size, int output_size,
         typename activation = Logistic, typename error = Squared_Error>
struct FeedForward_Network{
  float learning_rate;
  FeedForward_Network(float learning_rate = 0.8f) : learning_rate(learning_rate) {};
  Array2D<float, input_size, hidden_size> weights_inputToHidden;
  Array2D<float, hidden_size, output_size> weights_hiddenToOutput;

  std::array<float, input_size> activation_input;
  std::array<float, hidden_size> activation_hidden;
  std::array<float, output_size> activation_output;
};

template <int input_size, int hidden_size, int output_size,
         typename activation = Logistic, typename error = Squared_Error>
void train(FeedForward_Network<input_size, hidden_size, output_size, activation, error>& network,
    std::array<float, input_size> input, std::array<float, output_size> target) {
    calculate_activation(network, input);
    backprop(network, target);
}

template <int input_size, int hidden_size, int output_size, int point_count,
         typename activation = Logistic, typename error = Squared_Error>
void train(FeedForward_Network<input_size, hidden_size, output_size, activation, error>& network,
    Array2D<float, point_count, input_size> inputs, Array2D<float, point_count, output_size> targets) {
    for (int i = 0; i < point_count; ++i) {
      calculate_activation(network, inputs.row(i));
      backprop(network, targets.row(i));
    }
}

template <int input_size, int hidden_size, int output_size,
         typename activation = Logistic, typename error = Squared_Error>
void randomize(FeedForward_Network<input_size, hidden_size, output_size, activation, error>& network) {
  std::default_random_engine generator;
  std::normal_distribution<float> distribution(0, .1);

  for (int i=0; i < input_size; ++i) {
    for (int j=0; j < hidden_size; ++j) {
      network.weights_inputToHidden(i,j) = distribution(generator);
    }
  }

  for (int i=0; i < hidden_size; ++i) {
    for (int j=0; j < output_size; ++j) {
      network.weights_hiddenToOutput(i,j) = distribution(generator);
    }
  }
}

template <int input_size, int hidden_size, int output_size,
         typename activation = Logistic, typename error = Squared_Error>
void backprop(FeedForward_Network<input_size, hidden_size, output_size, activation, error> &network,
    std::array<float, output_size> target) {
  backprop(network, target.data());
}

template <int input_size, int hidden_size, int output_size,
         typename activation = Logistic, typename error = Squared_Error>
void backprop(FeedForward_Network<input_size, hidden_size, output_size, activation, error> &network,
    float* target) {
  //Calculate deltas
  std::array<float, output_size> output_deltas;
  for (int i=0; i < output_size; ++i) {
    output_deltas[i] = error::error_dir(target[i], network.activation_output[i]) * activation::activation_dir(network.activation_output[i]);
  }
  std::array<float, hidden_size> hidden_deltas;
  for (int i=0; i < hidden_size; ++i) {
    float error_sum= 0;
    for (int k=0; k < output_size; ++k) {
      error_sum += output_deltas[k] * network.weights_hiddenToOutput(i, k);
    }
    hidden_deltas[i] = error_sum * activation::activation_dir(network.activation_hidden[i]);
  }

  //Update network weights
  for (int k=0; k < hidden_size; ++k) {
    for (int i=0; i < output_size; ++i) {
      network.weights_hiddenToOutput(k,i) += network.learning_rate * output_deltas[i] * network.activation_hidden[k];
    }
  }

  for (int k=0; k < input_size; ++k) {
    for (int i=0; i < hidden_size; ++i) {
      network.weights_inputToHidden(k,i) += network.learning_rate * hidden_deltas[i] * network.activation_input[k];
    }
  }
}

template <int input_size, int hidden_size, int output_size,
         typename activation = Logistic, typename error = Squared_Error>
void calculate_activation(FeedForward_Network<input_size, hidden_size, output_size, activation, error>& network,
    std::array<float, input_size> input) {
  calculate_activation(network, input.data());
}

template <int input_size, int hidden_size, int output_size,
         typename activation = Logistic, typename error = Squared_Error>
void calculate_activation(FeedForward_Network<input_size, hidden_size, output_size, activation, error>& network,
    float * input) {
  for (int i=0; i < input_size; ++i) {
    network.activation_input[i] = input[i];
  }
  for (int i = 0; i < hidden_size; ++i) {
    float temp = 0;
    for (int j = 0; j < input_size; j++) {
      temp += input[j] * network.weights_inputToHidden(j, i);
    }
    network.activation_hidden[i] = activation::activation(temp);
  }
  for (int i = 0; i < output_size; ++i) {
    float temp = 0;
    for (int j = 0; j < hidden_size; j++) {
      temp += network.activation_hidden[j] * network.weights_hiddenToOutput(j, i);
    }
    network.activation_output[i] = activation::activation(temp);
  }
}

template <int input_size, int hidden_size, int output_size,
         typename activation = Logistic, typename error = Squared_Error>
std::array<float, output_size> predict(FeedForward_Network<input_size, hidden_size, output_size, activation, error>& network,
    std::array<float, input_size> input) {
  calculate_activation(network, input);
  return network.activation_output;
}

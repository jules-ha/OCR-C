#include "xorNN.h"

//number of neurons in the network
int input_neurons = 2;
int hidden_neurons = 2;
int output_neurons = 1;

//input values for the neural network
double input[4][2] = {{0,0},{0,1},{1,0},{1,1}};

//expected outputs for above inputs
double y[4] = {0, 1, 1, 0};

double learning_rate = 3.0;

int epoch = 10000;

//biases and weights
double w1[2][2];
double w2[1][2];
double b1[2][1];
double b2[1][1];

double a2[2][1]; //output of hidden layer
double a3[1][1]; //output of output layer

//forward propagation
void forward_propagation(double *input, double w1[2][2], double w2[1][2], double b1[2][1], double b2[1][1])
{
    double z2[2][1];
    z2[0][0] = w1[0][0] * input[0] + w1[0][1] * input[1] + b1[0][0];
    z2[1][0] = w1[1][0] * input[0] + w1[1][1] * input[1] + b1[1][0];
    a2[0][0] = sigmoid(z2[0][0]);
    a2[1][0] = sigmoid(z2[1][0]);

    double z3[1][1];
    z3[0][0] = w2[0][0] * a2[0][0] + w2[0][1] * a2[1][0] + b2[0][0];
    a3[0][0] = sigmoid(z3[0][0]);
}

//train the neural network
//using forward and backward propagation
void train_network()
{
    for (int i = 0; i <= epoch; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            forward_propagation(input[j], w1, w2, b1, b2);
            // below this line : backpropagation
            double dz3 = a3[0][0] - y[j];
            double dw2 = a2[0][0] * dz3 * sigmoid_d(a3[0][0]);
            double dw3 = a2[1][0] * dz3 * sigmoid_d(a3[0][0]);
            double db2 = dz3 * sigmoid_d(a3[0][0]);

            double dz2_1 = w2[0][0] * dz3 * sigmoid_d(a3[0][0]);
            double dz2_2 = w2[0][1] * dz3 * sigmoid_d(a3[0][0]);

            double dw1_1 = input[j][0] * dz2_1 * sigmoid_d(a2[0][0]);
            double dw1_2 = input[j][1] * dz2_1 * sigmoid_d(a2[0][0]);
            double dw1_3 = input[j][0] * dz2_2 * sigmoid_d(a2[1][0]);
            double dw1_4 = input[j][1] * dz2_2 * sigmoid_d(a2[1][0]);

            double db1_1 = dz2_1 * sigmoid_d(a2[0][0]);
            double db1_2 = dz2_2 * sigmoid_d(a2[1][0]);

            w2[0][0] -= learning_rate * dw2;
            w2[0][1] -= learning_rate * dw3;
            b2[0][0] -= learning_rate * db2;

            w1[0][0] -= learning_rate * dw1_1;
            w1[0][1] -= learning_rate * dw1_2;
            w1[1][0] -= learning_rate * dw1_3;
            w1[1][1] -= learning_rate * dw1_4;
            b1[0][0] -= learning_rate * db1_1;
            b1[1][0] -= learning_rate * db1_2;
        }
    }
}

//save the weights and biases of the network
//to the file specified in filename
void saveNetwork(char *filename)
{
    FILE *file = fopen(filename, "w");
    fprintf(file, "%f %f\n", w1[0][0], w1[0][1]); //weights from input to first hidden neuron
    fprintf(file, "%f %f\n", w1[1][0], w1[1][1]); //weights from input to second hidden neuron
    fprintf(file, "%f %f\n", w2[0][0], w2[0][1]); //weigts from hidden to output neuron
    fprintf(file, "%f\n", b1[0][0]); //bias of first hidden neuron
    fprintf(file, "%f\n", b1[1][0]); //bias of second hidden neuron
    fprintf(file, "%f\n", b2[0][0]); //bias of output neuron
    fclose(file);
}

//load the weights and biases of the network
//from the file specified in filename
void loadNetwork(char *filename)
{
    FILE *file = fopen(filename, "r");
    fscanf(file, "%lf %lf", &w1[0][0], &w1[0][1]);
    fscanf(file, "%lf %lf", &w1[1][0], &w1[1][1]);
    fscanf(file, "%lf %lf", &w2[0][0], &w2[0][1]);
    fscanf(file, "%lf", &b1[0][0]);
    fscanf(file, "%lf", &b1[1][0]);
    fscanf(file, "%lf", &b2[0][0]);
    fclose(file);
}

//creates initial random weights and biases
//then begins training
void initializeNetwork()
{
    //initialize biases and weights
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            w1[i][j] = GaussRandom();
        }
    }

    for (int i = 0; i < 2; i++)
    {
        w2[0][i] = GaussRandom();
    }

    for (int i = 0; i < 2; i++)
    {
        b1[i][0] = GaussRandom();
    }
    
    b2[0][0] = GaussRandom();

    printf("Network training with %d epochs and learning rate of %f\n", epoch, learning_rate);
    train_network();
}

int main(int argc, char** argv)
{
    int loaded = 0;
    if (argc < 2 ||strcmp("--load",argv[1]) != 0)
    {
        initializeNetwork();
    }
    else
    {
        loadNetwork(argv[2]);
        loaded = 1;
    }
    //test the network
    for (int i = 0; i < 4; i++)
    {
        forward_propagation(input[i], w1, w2, b1, b2);
        printf("XOR(%f, %f) --> output = %f, expected %f\n", input[i][0], input[i][1], a3[0][0], y[i]);
    }
    if (loaded == 1)
    {
        return 0;
    }
    printf("Save network? (y/n): ");
    char c;
    scanf("%c", &c);
    if (c != 'y')
    {
        return 0;
    }
    printf("Enter filename path: ");
    char filename[100];
    scanf("%s", filename);
    saveNetwork(filename);
    return 0;
}
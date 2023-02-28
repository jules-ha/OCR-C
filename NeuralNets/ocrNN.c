#include "ocrNN.h"
#include "tools.h"

//iterate through the images's pixels
//and convert them to a matrix
void imageToMatrix(SDL_Surface* image, int* matrix)
{
    Uint32* pixels = (Uint32*)image->pixels;
    SDL_PixelFormat* format = image->format;
    SDL_LockSurface(image);
    for (int i = 0; i < image->h; i++)
    {
        for (int j = 0; j < image->w; j++)
        {
            Uint32 pixel = pixels[i * image->w + j];
            Uint8 r, g, b;
            SDL_GetRGB(pixel, format, &r, &g, &b);
            if ((r + g + b) / 3 < 128)
            {
                matrix[i * image->w + j] = 0;
            }
            else
            {
                matrix[i * image->w + j] = 1;
            }
        }
    }
    SDL_UnlockSurface(image);
}

void initNetwork(struct nn* network, int epoch, int nodes)
{
    network->input_neurons = 28*28;
    network->hidden_neurons = nodes;
    network->output_neurons = 10;
    network->learning_rate = 0.9;
    network->epochs = epoch;
    network->decay = 1;

    //initialize weights and biases
    for (int i = 0; i < network->hidden_neurons; i++)
    {
        network->biases_h[i] = GaussRandom();
        for (int j = 0; j < network->input_neurons; j++)
        {
            network->weights_ih[i][j] = GaussRandom();
        }
    }
    for (int i = 0; i < network->output_neurons; i++)
    {
        network->biases_o[i] = GaussRandom();
        for (int j = 0; j < network->hidden_neurons; j++)
        {
            network->weights_ho[i][j] = GaussRandom();
        }
    }
}

//matrix is a 28*28 matrix represented in a 1D array

//softmax activation function
void softmax(double* matrix, int size)
{
    double sum = 0;
    for (int i = 0; i < size; i++)
    {
        sum += exp(matrix[i]);
    }
    for (int i = 0; i < size; i++)
    {
        matrix[i] = exp(matrix[i]) / sum;
    }
}

//feedforward
//use softmax activation function for the output layer
void forward_propagation(struct nn* network, int* matrix)
{
    for (int i = 0; i < network->hidden_neurons; i++)
    {
        double sum = 0;
        for (int j = 0; j < network->input_neurons; j++)
        {
            sum += matrix[j] * network->weights_ih[i][j];
        }
        network->hidden_outputs[i] = sigmoid(sum + network->biases_h[i]);
    }
    for (int i = 1; i < network->output_neurons; i++)
    {
        double sum = 0;
        for (int j = 0; j < network->hidden_neurons; j++)
        {
            sum += network->hidden_outputs[j] * network->weights_ho[i][j];
        }
        network->outputs[i] = sum + network->biases_o[i];
    }
    softmax(network->outputs, network->output_neurons);
}

//matrix is the input matrix
//target is the expected output
void back_propagation(struct nn* network, int *matrix, int *target)
{
    double output_errors[10];
    double hidden_errors[network->hidden_neurons];
    for (int i = 1; i < network->output_neurons; i++)
    {
        output_errors[i] = (target[i] - network->outputs[i]);
    }
    for (int i = 0; i < network->hidden_neurons; i++)
    {
        double sum = 0;
        for (int j = 1; j < network->output_neurons; j++)
        {
            sum += output_errors[j] * network->weights_ho[j][i];
        }
        hidden_errors[i] = sum * sigmoid_d(network->hidden_outputs[i]);
    }
    for (int i = 1; i < network->output_neurons; i++)
    {
        for (int j = 0; j < network->hidden_neurons; j++)
        {
            network->weights_ho[i][j] += network->learning_rate * output_errors[i] * network->hidden_outputs[j];
        }
    }
    for (int i = 0; i < network->hidden_neurons; i++)
    {
        for (int j = 0; j < network->input_neurons; j++)
        {
            network->weights_ih[i][j] += network->learning_rate * hidden_errors[i] * matrix[j];
        }
    }
    for (int i = 1; i < network->output_neurons; i++)
    {
        network->biases_o[i] += network->learning_rate * output_errors[i];
    }
    for (int i = 0; i < network->hidden_neurons; i++)
    {
        network->biases_h[i] += network->learning_rate * hidden_errors[i];
    }
}

//load all the matrixes from the images in assets folder
//using imagetoMatrix function
//then feed random matrixes to the network
void train(struct nn *network, char* datasetPath)
{
    int folderSizes[10]; //number of images in each folder (0-9)
    int totalImages = 0;
 //matrixes[folder][image][pixel]
    for (int i = 1; i < 10; i++)
    {
        char path[100];
        sprintf(path, "%s/%d/",datasetPath, i);
        folderSizes[i] = count_files(path);
        totalImages += folderSizes[i];
    }
    int ***matrixes = malloc(10 * sizeof(int**));
    for (int i = 0; i < 10; i++)
    {
        matrixes[i] = malloc(folderSizes[i] * sizeof(int*));
        for (int j = 0; j < folderSizes[i]; j++)
        {
            matrixes[i][j] = malloc(28*28 * sizeof(int));
        }
    }
    //load matrixes
    for (int i = 1; i < 10; i++)
    {
        char path[100];
        DIR *dir;
        struct dirent *ent;
        sprintf(path, "%s/%d/",datasetPath, i);
        dir = opendir(path);
        if (dir)
        {
            int j = 0;
            while ((ent = readdir(dir)) != NULL)
            {
                if (ent->d_type == 8)
                {
                    char path2[300];
                    sprintf(path2, "%s/%d/%s",datasetPath,  i, ent->d_name);
                    SDL_Surface* image = load_image(path2);
                    imageToMatrix(image, matrixes[i][j]);
                    SDL_FreeSurface(image);
                    j++;
                }
            }
        }
        closedir(dir);
    }
    //train
    for (int i = 0; i < network->epochs; i++)
    {
        for (int j = 0; j < totalImages; j++)
        {
            int folder = (rand() % 9) + 1;
            int image = rand() % folderSizes[folder];
            int target[10];
            for (int k = 1; k < 10; k++)
            {
                if (k == folder)
                {
                    target[k] = 1;
                }
                else
                {
                    target[k] = 0;
                }
            }
            forward_propagation(network, matrixes[folder][image]);
            back_propagation(network, matrixes[folder][image], target);
        }
        if (i % 100 == 0)
        {
            printf("Epoch %i\n", i);
        }
        network->learning_rate *= network->decay;
    }
    //free memory
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < folderSizes[i]; j++)
        {
            free(matrixes[i][j]);
        }
        free(matrixes[i]);
    }
    free(matrixes);
}

int finalOutput(struct nn* network)
{
    int max = 0;
    for (int i = 0; i < network->output_neurons; i++)
    {
        if (network->outputs[i] > network->outputs[max])
        {
            max = i;
        }
    }
    return max;
}

void testNetwork(struct nn* network)
{
    char path[100];
    DIR *dir;
    struct dirent *ent;
    int correct = 0;
    int total = 0;
    int corrects[10] = {0};
    int totals[10] = {0};
    printf("Testing for epoch=%i and learning_rate=%lf\n", network->epochs, network->learning_rate);
    for (int i = 1; i < 10; i++)
    {
        sprintf(path, "./datset123/test/%d/", i);
        dir = opendir(path);
        if (dir)
        {
            while ((ent = readdir(dir)) != NULL)
            {
                if (ent->d_type == 8)
                    {
                        //printf("%s\n", ent->d_name);
                        char path2[300];
                        sprintf(path2, "./datset123/test/%d/%s", i, ent->d_name);
                        SDL_Surface* image = load_image(path2);
                        int matrix[28*28];
                        imageToMatrix(image, matrix);
                        forward_propagation(network, matrix);
                        int output = finalOutput(network);
                        if (output == i)
                        {
                            correct++;
                            corrects[i]++;
                        }
                        total++;
                        totals[i]++;
                        SDL_FreeSurface(image);
                    }
            }
        }
        closedir(dir);
    }
    printf("Correct : %i/%i == %i%%\n", correct, total, correct*100/total);
    for (int i = 1; i < 10; i++)
    {
        printf("Correct %i : %i/%i\n", i, corrects[i], totals[i]);
    }
}

void printFullOutput(struct nn* network)
{
    for (int i = 1; i < network->output_neurons; i++)
    {
        printf("P(X = %d) = %lf\n",i ,  network->outputs[i]);
    }
}


// main function to recognize the digit
// digit : 28*28px image of the digit to recognize
//         (must be a white digit on a black background)
// path : path to the saved neural network
// return the recognized digit

int readDigit(SDL_Surface* digit, char* path)
{
    struct nn* network = malloc(sizeof(struct nn));
    initNetwork(network, 500, 150);
    loadNet(network, path);
    int matrix[28*28];
    imageToMatrix(digit, matrix);
    forward_propagation(network, matrix);
    int output = finalOutput(network);
    free(network);
    return output;
}

/*
int main(int argc, char* argv[])
{
    struct nn* network = malloc(sizeof(struct nn));
    initNetwork(network);
    if (argc > 1 && strcmp(argv[1], "--load") == 0)
    {
        if (argc < 3)
        {
            errx(1, "Usage: %s --load <file>", argv[0]);
        }
        loadNet(network, argv[2]);
    }
    else
    {
        train(network);
    }
    if (argc > 1 && strcmp(argv[1], "--save") == 0)
    {
        if (argc < 3)
        {
            errx(1, "Usage: %s --save <file>", argv[0]);
        }
        saveNet(network, argv[2]);
    }
    testNetwork(network);
    int matrix[28*28];
    SDL_Surface* image = load_image("13CON.bmp");
    imageToMatrix(image, matrix);
    forward_propagation(network, matrix);
    printFullOutput(network);
    SDL_FreeSurface(image);
    free(network);
    return 0;
}
*/


//à faire:
//continuer a jouer avec le nombre de neuronnes
//diminuer la probabilité d'entrainer le réseau avec des 0
//fusionner les datasets ?
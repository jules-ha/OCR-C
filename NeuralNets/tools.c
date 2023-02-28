#include "tools.h"
#define M_PI 3.14159265358979323846

//generates a random number using the Gaussian law of distribution
//using Box-Muller transform
double GaussRandom()
{
    double r1 = (double)rand() / (double)RAND_MAX;
    double r2 = (double)rand() / (double)RAND_MAX;
    return (sqrt(-2*log(r1))* sin(2 * M_PI * r2));
}

//sigmoid function
double sigmoid(double i)
{
    return (1 / (1 + exp(-i)));
}

//derivative of the sigmoid function
double sigmoid_d(double i)
{
    return (i * (1 - i));
}

///////////////////////////
// All functions below are used for the OCR
///////////////////////////


SDL_Surface* load_image(const char* path)
{
    SDL_Surface* tmp = IMG_Load(path);
    SDL_Surface* res = SDL_ConvertSurfaceFormat(tmp, SDL_PIXELFORMAT_RGB888, 0);
    SDL_FreeSurface(tmp);
    return res;
}


//function to count the number of .jpeg or .jpg files in a directory
int count_files(char* path)
{
    DIR* dir = opendir(path);
    struct dirent* ent;
    int count = 0;
    while ((ent = readdir(dir)) != NULL)
    {
        if (strstr(ent->d_name, ".jpeg") || strstr(ent->d_name, ".jpg") || 
        strstr(ent->d_name, ".png"))
        {
            count++;
        }
        {
            count++;
        }
    }
    closedir(dir);
    return count;
}

//print the matrix with the '1' in red
//and the '0' in white
//used for debugging
void printMatrix(int *matrix)
{
    for (int i = 0; i < 28; i++)
    {
        for (int j = 0; j < 28; j++)
        {
            if (matrix[i*28 + j] == 1)
            {
                printf("\033[1;31m%d\033[0m", matrix[i*28 + j]);
            }
            else
            {
                printf("%d", matrix[i*28 + j]);
            }
        }
        printf("\n");
    }
}

void saveNet(struct nn* network, char* filename)
{
    FILE *f = fopen(filename, "w");
    if (f == NULL)
    {
        printf("Error opening file");
        exit(1);
    }
    fprintf(f, "%i %i %i %lf %lf\n", network->epochs, network->hidden_neurons,
    network->output_neurons, network->learning_rate, network->decay);
    for (int i = 0; i < network->hidden_neurons; i++)
    {
        for (int j = 0; j < network->input_neurons; j++)
        {
            fprintf(f, "%lf\n", network->weights_ih[i][j]);
        }
    }
    for (int i = 0; i < network->output_neurons; i++)
    {
        for (int j = 0; j < network->hidden_neurons; j++)
        {
            fprintf(f, "%lf\n", network->weights_ho[i][j]);
        }
    }
    for (int i = 0; i < network->hidden_neurons; i++)
    {
        fprintf(f, "%lf\n", network->biases_h[i]);
    }
    for (int i = 0; i < network->output_neurons; i++)
    {
        fprintf(f, "%lf\n", network->biases_o[i]);
    }
    fclose(f);
}

void loadNet(struct nn* network, char* filename)
{
    FILE *f = fopen(filename, "r");
    if (f == NULL)
    {
        printf("Error : %s file not found\n", filename);
        exit(1);
    }
    fscanf(f, "%i %i %i %lf %lf\n", &network->epochs, &network->hidden_neurons,
    &network->output_neurons, &network->learning_rate, &network->decay);
    for (int i = 0; i < network->hidden_neurons; i++)
    {
        for (int j = 0; j < network->input_neurons; j++)
        {
            fscanf(f, "%lf\n", &network->weights_ih[i][j]);
        }
    }
    for (int i = 0; i < network->output_neurons; i++)
    {
        for (int j = 0; j < network->hidden_neurons; j++)
        {
            fscanf(f, "%lf\n", &network->weights_ho[i][j]);
        }
    }
    for (int i = 0; i < network->hidden_neurons; i++)
    {
        fscanf(f, "%lf\n", &network->biases_h[i]);
    }
    for (int i = 0; i < network->output_neurons; i++)
    {
        fscanf(f, "%lf\n", &network->biases_o[i]);
    }
    fclose(f);
}